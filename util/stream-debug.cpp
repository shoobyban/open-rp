#include <iostream>
#include <sstream>

#include <unistd.h>
#include <string.h>
#include <errno.h>

#include "orp.h"
#include "base64.h"

extern int errno;

using namespace std;

class orpDecrypter : public OpenRemotePlay
{
public:
	orpDecrypter(struct orpConfig_t *config);
	int DecryptStream(FILE *stream, const char *nonce);
};

int main(int argc, char *argv[])
{
	if (argc != 5) {
		cerr << argv[0] << " <config> <profile> <nonce> <stream>\n";
		return 1;
	}

	FILE *stream = NULL;
	if (!(stream = fopen(argv[4], "r"))) {
		cerr << "Stream open error: %s";
		cerr << argv[4] << endl;
		return 1;
	}

	struct orpConfigCtx_t config_ctx;
	if (orpConfigOpen(&config_ctx, argv[1]) < 0) {
		cerr << "Configuration open error: %s";
		cerr << argv[1] << endl;
		return 1;
	}

	struct orpConfigRecord_t record;
	memset(&record, 0, sizeof(struct orpConfigRecord_t));
	strncpy((char *)&record.ps3_nickname, argv[2], ORP_NICKNAME_LEN);

	if (!orpConfigFind(&config_ctx, &record)) {
		cerr << "Configuration profile not found: ";
		cerr << "\"" << argv[2] << "\"" << endl;
		return 1;
	}

	struct orpConfig_t config;
	memset(&config, 0, sizeof(struct orpConfig_t));
	orpConfigGetKey(&config_ctx, orpID_KEY_0, config.key.skey0);
	orpConfigGetKey(&config_ctx, orpID_KEY_1, config.key.skey1);
	orpConfigGetKey(&config_ctx, orpID_KEY_2, config.key.skey2);

	memcpy(config.ps3_mac, record.ps3_mac, ORP_MAC_LEN);
	memcpy(config.psp_mac, record.psp_mac, ORP_MAC_LEN);
	memcpy(config.key.psp_id, record.psp_id, ORP_KEY_LEN);
	memcpy(config.key.pkey, record.pkey, ORP_KEY_LEN);

	orpDecrypter decrypter(&config);
	int rc = decrypter.DecryptStream(stream, argv[3]);
	fclose(stream);

	return rc;
}

orpDecrypter::orpDecrypter(struct orpConfig_t *config)
	: OpenRemotePlay(config) { }

int orpDecrypter::DecryptStream(FILE *stream, const char *nonce)
{
	FILE *output_stream = fopen("stream.dat", "w+");
	if (!output_stream) {
		cerr << "Error opening output stream file.\n";
		return 1;
	}
	FILE *output_header = fopen("header.dat", "w+");
	if (!output_header) {
		cerr << "Error opening output header file.\n";
		return 1;
	}

	if (!CreateKeys(nonce)) return 1;
	AES_KEY aes_key;
	AES_set_decrypt_key(config.key.xor_pkey, ORP_KEY_LEN * 8,
		&aes_key);

	rewind(stream);

	Uint8 buffer[getpagesize()];
	if (fread(buffer, 1, 4, stream) != 4) {
		cerr << "Short read.\n";
		return 1;
	}

	if (!strncasecmp("HTTP", (const char *)buffer, 4)) {
		if (fread(buffer, 1, 61, stream) != 61) {
			cerr << "Short read.\n";
			return 1;
		}
	} else rewind(stream);

	int dump = 0;
	Uint8 *keyframe_header = NULL;

	while (!feof(stream)) {
		if (fread(buffer, 1, 8, stream) != 8) {
			cerr << "Short read.\n";
			return 1;
		}

		Uint32 chunk_len = 0;
		if (sscanf((const char *)buffer, "%06x", &chunk_len) != 1) {
			fprintf(stderr, "Missing chunk length at position: %x\n",
				ftell(stream));
			return 1;
		}

		if (!chunk_len) {
			cerr << "Zero length chunk.\n";
			return 1;
		}

		Uint8 *chunk = new Uint8[chunk_len];
		if (!chunk) {
			cerr << "Out of memory allocating " << chunk_len << " bytes.\n";
			return 1;
		}

		if (fread(chunk, 1, chunk_len, stream) != chunk_len) {
			cerr << "Error reading " << chunk_len << " byte chunk.\n";
			return 1;
		}

		struct orpStreamPacketHeader_t *header;
		header = (struct orpStreamPacketHeader_t *)chunk;
		Uint16 header_len = SDL_Swap16(header->len);
		Uint8 *data = chunk + sizeof(struct orpStreamPacketHeader_t);
		fprintf(stderr, "%02x%02x: %c%c ",
			header->magic[0], header->magic[1],
			(chunk_len - sizeof(struct orpStreamPacketHeader_t) != header_len) ? 's' : '-',
			(data[0] != 0x00) ? 'e' : '-');
		Sint32 i;
		for (i = 0; i < 32; i++) fprintf(stderr, "%02x ", data[i]);
		fprintf(stderr, "\n");
//		fprintf(stderr, "         ");
//		for (i = 128; i < 160; i++) fprintf(stderr, "%02x ", data[i]);
//		fprintf(stderr, "\n");

		if (data[0] != 0x00) {
			Sint32 bad_byte = -1;
			Uint8 expected = 0, found = 0;
			if (!keyframe_header) {
				keyframe_header = new Uint8[header_len];
				memcpy(keyframe_header, data, header_len);
			}
			else {
				for (i = 0; i < header_len; i++) {
					if (data[i] == keyframe_header[i]) continue;
					bad_byte = i;
					expected = keyframe_header[i];
					found = data[i];
					break;
				}
			}

			if (bad_byte > 0) {
//				fprintf(stderr, "         bad_byte: 0x%02x, expected: 0x%02x, found: 0x%02x\n",
//					bad_byte, expected, found);
			}
#if 0
			fprintf(stderr, "%-9x", bad_byte);
			for (i = 0; i < 32; i++)
				fprintf(stderr, "%02x%c", data[i],
				(bad_byte == i) ? '<' : ' ');
			fprintf(stderr, "\n");
#endif
			memcpy(config.key.iv1,
				config.key.xor_nonce, ORP_KEY_LEN);
			AES_cbc_encrypt(data, data,
				header_len - (header_len % ORP_KEY_LEN),
				&aes_key, config.key.iv1, AES_DECRYPT);

			if (bad_byte > 0 && bad_byte != 0x60) {
				fprintf(stderr, "%04d %02x   ", dump, bad_byte);
				for (i = 0; i < 32; i++)
					fprintf(stderr, "%02x%c", data[i],
					(bad_byte == i) ? '<' : ' ');
				fprintf(stderr, "\n");
			}
//			if (dump < 4) {
				char filename[32];
				sprintf(filename, "dump-%02d.dat", dump);
				FILE *h_dump = fopen(filename, "w+");
				if (h_dump) {
					fwrite(data, 1, header_len, h_dump);
					fclose(h_dump);
				}
				dump++;
//			}
//			if (bad_byte != -1 && bad_byte != 96) {
//				fprintf(stderr, "Expectd: %02x\n", expected);
//				break;
//			}
			fwrite(data + (0x50 - 8), 1, 0x10 + 16, output_stream);
		}

		fwrite(header, 1, sizeof(struct orpStreamPacketHeader_t), output_header);
		//fwrite(data, 1, header_len, output_stream);

		delete [] chunk;
		fseek(stream, 2, SEEK_CUR);
	}

	fclose(output_header);
	fclose(output_stream);
	return 0;
}

// vi: ts=4
