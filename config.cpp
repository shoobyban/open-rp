//////////////////////////////////////////////////////////////////////////////
//
// Open Remote Play
// http://ps3-hacks.com
//
//////////////////////////////////////////////////////////////////////////////
//
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the
// Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful, but 
// WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
// or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
// for more details.
//
// You should have received a copy of the GNU General Public License along
// with this program; if not, write to the Free Software Foundation, Inc.,
// 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//
//////////////////////////////////////////////////////////////////////////////

#include "config.h"

int orpConfigOpen(struct orpConfigCtx_t *ctx, const char *filename)
{
	FILE *h_file;
	struct orpConfigHeader_t header;

	memset(ctx, 0, sizeof(struct orpConfigCtx_t));

	if (!(h_file = fopen(filename, "a+b"))) return -1;
	if (fseek(h_file, 0, SEEK_END) < 0) {
		fclose(h_file);
		return -1;
	}
	if (ftell(h_file) == 0) {
		memset(&header, 0, sizeof(struct orpConfigHeader_t));
		header.magic[0] = 'O';
		header.magic[1] = 'R';
		header.magic[2] = 'P';
		header.version = ORP_CONFIG_VER;
		if (fwrite(&header, 1, sizeof(struct orpConfigHeader_t),
			h_file) != sizeof(struct orpConfigHeader_t)) {
			fclose(h_file);
			return -1;
		}
	}
	fclose(h_file);
	if (!(h_file = fopen(filename, "r+b"))) return -1;
	if (fread((void *)&header, 1, sizeof(struct orpConfigHeader_t),
		h_file) != sizeof(struct orpConfigHeader_t)) {
		fclose(h_file);
		return -1;
	}
	if (header.magic[0] != 'O' ||
		header.magic[1] != 'R' || header.magic[2] != 'P') {
		fclose(h_file);
		return -1;
	}
	if (header.version > ORP_CONFIG_VER) {
		fclose(h_file);
		return -1;
	}
	ctx->h_file = h_file;
	ctx->filename = strdup(filename);
	memcpy(&ctx->header, &header, sizeof(struct orpConfigHeader_t));

	if (header.version < ORP_CONFIG_VER) {
		if (orpConfigUpgrade(ctx) < 0) {
			orpConfigClose(ctx);
			return -1;
		}
	}

	return 0;
}

int orpConfigUpgrade(struct orpConfigCtx_t *ctx)
{
	if (ctx->header.version == 1 && ORP_CONFIG_VER == 2) {
		if (fseek(ctx->h_file, 0, SEEK_END) < 0) return -1;
		long records = 0;
		if ((ftell(ctx->h_file) - sizeof(struct orpConfigHeader_t)) > 0) {
			records = (ftell(ctx->h_file) - sizeof(struct orpConfigHeader_t)) /
				sizeof(struct orpConfigRecord_v1_t);
		}
		struct orpConfigRecord_v1_t *v1 = NULL;
		if (records) v1 = (struct orpConfigRecord_v1_t *)calloc(records,
			sizeof(struct orpConfigRecord_v1_t));
		orpConfigRewind(ctx);
		long i;
		for (i = 0; i < records && v1 && !feof(ctx->h_file); i++) {
			if (fread(&v1[i], 1, sizeof(struct orpConfigRecord_v1_t),
				ctx->h_file) != sizeof(struct orpConfigRecord_v1_t)) {
					free(v1);
					return -1;
			}
		}
		orpConfigRewind(ctx);
		struct orpConfigRecord_v2_t v2;
		for (i = 0; i < records && v1; i++) {
			memset(&v2, 0, sizeof(struct orpConfigRecord_v2_t));
			memcpy(&v2, &v1[i], sizeof(struct orpConfigRecord_v1_t));
			if (fwrite(&v2, 1, sizeof(struct orpConfigRecord_v2_t),
				ctx->h_file) != sizeof(struct orpConfigRecord_v2_t)) {
					free(v1);
					return -1;
			}
		}
		if (v1) free(v1);
		rewind(ctx->h_file);
		ctx->header.version = ORP_CONFIG_VER;
		if (fwrite(&ctx->header, 1, sizeof(struct orpConfigHeader_t),
			ctx->h_file) != sizeof(struct orpConfigHeader_t)) return -1;
		orpConfigRewind(ctx);
		fflush(ctx->h_file);
		return 0;
	}

	return -1;
}

int orpConfigClose(struct orpConfigCtx_t *ctx)
{
	if (ctx->h_file) fclose(ctx->h_file);
	if (ctx->filename) free(ctx->filename);
	memset(ctx, 0, sizeof(struct orpConfigCtx_t));

	return 0;
}

int orpConfigRewind(struct orpConfigCtx_t *ctx)
{
	if (!ctx->h_file) return -1;
	if (fseek(ctx->h_file,
		sizeof(struct orpConfigHeader_t), SEEK_SET) < 0) return -1;
	return 0;
}

int orpConfigRead(struct orpConfigCtx_t *ctx, struct orpConfigRecord_t *rec)
{
	if (!ctx->h_file) return 0;
	struct orpConfigRecord_t _rec;
	memset(&_rec, 0, sizeof(struct orpConfigRecord_t));
	for ( ;; ) {
		if (fread((void *)&_rec, 1, sizeof(struct orpConfigRecord_t),
			ctx->h_file) != sizeof(struct orpConfigRecord_t))
			return 0;
		if (!(_rec.flags & ORP_CONFIG_DELETED)) break;
	}
	memcpy(rec, &_rec, sizeof(struct orpConfigRecord_t));
	return 1;
}

long orpConfigFind(struct orpConfigCtx_t *ctx, struct orpConfigRecord_t *rec)
{
	if (!ctx->h_file) return 0;
	long offset = 0, pos = ftell(ctx->h_file);
	if (orpConfigRewind(ctx) == 0) {
		struct orpConfigRecord_t _rec;
		while (orpConfigRead(ctx, &_rec)) {
			if (_rec.flags & ORP_CONFIG_DELETED) continue;
			if (strncmp((const char *)_rec.ps3_nickname,
				(const char *)rec->ps3_nickname,
				ORP_NICKNAME_LEN)) continue;
			offset = ftell(ctx->h_file) - sizeof(struct orpConfigRecord_t);
			memcpy(rec, &_rec, sizeof(struct orpConfigRecord_t));
			fseek(ctx->h_file, pos, SEEK_SET);
			break;
		}
	}
	fseek(ctx->h_file, pos, SEEK_SET);
	return offset;
}

int orpConfigDelete(struct orpConfigCtx_t *ctx, const char *nickname)
{
	if (!ctx->h_file) return -1;
	struct orpConfigRecord_t rec;
	strcpy((char *)rec.ps3_nickname, nickname);
	if (!orpConfigFind(ctx, &rec)) return -1;
	rec.flags |= ORP_CONFIG_DELETED;
	return orpConfigSave(ctx, &rec);
}

int orpConfigSave(struct orpConfigCtx_t *ctx, struct orpConfigRecord_t *rec)
{
	if (!ctx->h_file) return -1;
	struct orpConfigRecord_t _rec;
	strcpy((char *)_rec.ps3_nickname, (char *)rec->ps3_nickname);
	long pos = ftell(ctx->h_file);
	long offset = orpConfigFind(ctx, &_rec);
	if (!offset) {
		if (orpConfigRewind(ctx) < 0) return -1;
		for ( ;; ) {
			if (fread(&_rec, 1, sizeof(struct orpConfigRecord_t),
				ctx->h_file) != sizeof(struct orpConfigRecord_t)) break;
			if (_rec.flags & ORP_CONFIG_DELETED) {
				offset = ftell(ctx->h_file) - sizeof(struct orpConfigRecord_t);
				break;
			}
		}
	}
	if (!offset) {
		fseek(ctx->h_file, 0, SEEK_END);
		offset = ftell(ctx->h_file);
	}
	if (fseek(ctx->h_file, offset, SEEK_SET) < 0) {
		fseek(ctx->h_file, pos, SEEK_SET);
		return -1;
	}
	if (fwrite(rec, 1, sizeof(struct orpConfigRecord_t),
		ctx->h_file) != sizeof(struct orpConfigRecord_t)) {
		fseek(ctx->h_file, pos, SEEK_SET);
		return -1;
	}
	fflush(ctx->h_file);
	fseek(ctx->h_file, pos, SEEK_SET);
	return 0;
}

int orpConfigGetKey(struct orpConfigCtx_t *ctx, enum orpID_KEY which, Uint8 *dst)
{
	if (!ctx->h_file) return -1;
	Uint8 *src = NULL;
	switch (which) {
	case orpID_KEY_0:
		src = ctx->header.skey0;
		break;
	case orpID_KEY_1:
		src = ctx->header.skey1;
		break;
	case orpID_KEY_2:
		src = ctx->header.skey2;
		break;
	}
	if (src == NULL) return -1;
	memcpy(dst, src, ORP_KEY_LEN);
	return 0;
}

int orpConfigSetKey(struct orpConfigCtx_t *ctx, enum orpID_KEY which, Uint8 *src)
{
	if (!ctx->h_file) return -1;
	Uint8 *dst = NULL;
	switch (which) {
	case orpID_KEY_0:
		dst = ctx->header.skey0;
		break;
	case orpID_KEY_1:
		dst = ctx->header.skey1;
		break;
	case orpID_KEY_2:
		dst = ctx->header.skey2;
		break;
	}
	if (dst == NULL) return -1;
	memcpy(dst, src, ORP_KEY_LEN);
	long pos = ftell(ctx->h_file);
	fseek(ctx->h_file, 0, SEEK_SET);
	if (fwrite(&ctx->header, 1, sizeof(struct orpConfigHeader_t),
		ctx->h_file) != sizeof(struct orpConfigHeader_t)) {
		fseek(ctx->h_file, pos, SEEK_SET);
		return -1;
	}
	fflush(ctx->h_file);
	fseek(ctx->h_file, pos, SEEK_SET);
	return 0;
}

#ifdef ORP_CONFIG_DEBUG
enum orpConfigValueType {
	CVT_KEY,
	CVT_MAC,
	CVT_FLAGS
};

static void orpConfigPrint(enum orpConfigValueType type, FILE *output, Uint8 *value)
{
	Sint32 i, len;
	Uint32 flags;
	switch (type) {
	case CVT_KEY:
		len = ORP_KEY_LEN;
		for (i = 0; i < len; i++) fprintf(output, "%02x", value[i]);
		return;
	case CVT_MAC:
		len = ORP_MAC_LEN - 1;
		for (i = 0; i < len; i++) fprintf(output, "%02x:", value[i]);
		fprintf(output, "%02x", value[len]);
		return;
	case CVT_FLAGS:
		memcpy(&flags, (Uint32 *)value, sizeof(Uint32));
		fprintf(output, "%c", flags & ORP_CONFIG_DELETED ? 'd' : '-');
		fprintf(output, "%c", flags & ORP_CONFIG_EXPORT ? 'e' : '-');
		fprintf(output, "%c", flags & ORP_CONFIG_KEYS ? 'k' : '-');
		fprintf(output, "%c", flags & ORP_CONFIG_NOSRCH ? 'n' : '-');
		fprintf(output, "%c", flags & ORP_CONFIG_WOLR ? 'w' : '-');
		fprintf(output, "%c", flags & ORP_CONFIG_BR256 ? '2' : '-');
		fprintf(output, "%c", flags & ORP_CONFIG_BR384 ? '3' : '-');
		fprintf(output, "%c", flags & ORP_CONFIG_BR512 ? '5' : '-');
		fprintf(output, "%c", flags & ORP_CONFIG_BR768 ? '7' : '-');
		fprintf(output, "%c", flags & ORP_CONFIG_BR1024 ? '1' : '-');
		fprintf(output, "%c", flags & ORP_CONFIG_PRIVATE ? 'p' : '-');
		return;
	}
}

void orpConfigDebug(struct orpConfigCtx_t *ctx, FILE *output)
{
	if (!ctx->h_file) {
		fprintf(output, "Invalid context.\n");
		return;
	}

	fprintf(output, "%12s: %c%c%c\n", "magic",
		ctx->header.magic[0], ctx->header.magic[1], ctx->header.magic[2]);
	fprintf(output, "%12s: %hhd\n", "version", ctx->header.version);
	fprintf(output, "%12s: ", "flags");
	orpConfigPrint(CVT_FLAGS, output, (Uint8 *)&ctx->header.flags);
	fprintf(output, "\n");
	fprintf(output, "%12s: ", "skey0");
	orpConfigPrint(CVT_KEY, output, ctx->header.skey0);
	fprintf(output, "\n");
	fprintf(output, "%12s: ", "skey1");
	orpConfigPrint(CVT_KEY, output, ctx->header.skey1);
	fprintf(output, "\n");
	fprintf(output, "%12s: ", "skey2");
	orpConfigPrint(CVT_KEY, output, ctx->header.skey2);
	fprintf(output, "\n\n");

	long cpos = ftell(ctx->h_file);
	fseek(ctx->h_file, sizeof(struct orpConfigHeader_t), SEEK_SET);
	struct orpConfigRecord_t rec;

	while (!feof(ctx->h_file)) {
		if (fread(&rec, 1, sizeof(orpConfigRecord_t), ctx->h_file) !=
			sizeof(orpConfigRecord_t)) break;
		fprintf(output, "%12s: ", "flags");
		orpConfigPrint(CVT_FLAGS, output, (Uint8 *)&rec.flags);
		fprintf(output, "\n");
		fprintf(output, "%12s: %hd\n", "ps3_port", rec.ps3_port);
		fprintf(output, "%12s: \"%s\"\n", "ps3_hostname",
			(const char *)rec.ps3_hostname);
		fprintf(output, "%12s: \"%s\"\n", "ps3_nickname",
			(const char *)rec.ps3_nickname);
		fprintf(output, "%12s: ", "ps3_mac");
		orpConfigPrint(CVT_MAC, output, rec.ps3_mac);
		fprintf(output, "\n");
		fprintf(output, "%12s: ", "psp_mac");
		orpConfigPrint(CVT_MAC, output, rec.psp_mac);
		fprintf(output, "\n");
		fprintf(output, "%12s: ", "psp_id");
		orpConfigPrint(CVT_KEY, output, rec.psp_id);
		fprintf(output, "\n");
		fprintf(output, "%12s: \"%s\"\n", "psp_owner",
			(const char *)rec.psp_owner);
		fprintf(output, "%12s: ", "pkey");
		orpConfigPrint(CVT_KEY, output, rec.pkey);
		fprintf(output, "\n");
		fprintf(output, "%12s: \"%s\"\n\n", "psn_login",
			(const char *)rec.psn_login);
	}

orpConfigDebug_Return:
	fseek(ctx->h_file, cpos, SEEK_SET);
}
#endif

// vi: ts=4
