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

#include <iostream>
#include <sstream>
#include <vector>
#include <queue>

#include "orp.h"
#include "base64.h"
#include "images.h"

static struct orpHeader_t orpHeaderList[] = {
	{ HEADER_AUDIO_BITRATE, "PREMO-Audio-Bitrate" },
	{ HEADER_AUDIO_CHANNELS, "PREMO-Audio-Channels" },
	{ HEADER_AUDIO_CLOCKFREQ, "PREMO-Audio-ClockFrequency" },
	{ HEADER_AUDIO_CODEC, "PREMO-Audio-Codec" },
	{ HEADER_AUDIO_SAMPLERATE, "PREMO-Audio-SamplingRate" },
	{ HEADER_AUTH, "PREMO-Auth" },
	{ HEADER_CTRL_BITRATE, "Bitrate" },
	{ HEADER_CTRL_MAXBITRATE, "Max-Bitrate" },
	{ HEADER_CTRL_MODE, "Mode" },
	{ HEADER_EXEC_MODE, "PREMO-Exec-Mode" },
	{ HEADER_MODE, "PREMO-Mode" },
	{ HEADER_NONCE, "PREMO-Nonce" },
	{ HEADER_PAD_ASSIGN, "PREMO-Pad-Assign" },
	{ HEADER_PAD_COMPLETE, "PREMO-Pad-Complete" },
	{ HEADER_PAD_INFO, "PREMO-Pad-Info" },
	{ HEADER_PLATFORM_INFO, "PREMO-Platform-Info" },
	{ HEADER_POWER_CONTROL, "PREMO-Power-Control" },
	{ HEADER_PS3_NICKNAME, "PREMO-PS3-Nickname" },
	{ HEADER_PSPID, "PREMO-PSPID" },
	{ HEADER_TRANS, "PREMO-Trans" },
	{ HEADER_TRANS_MODE, "PREMO-Trans-Mode" },
	{ HEADER_SESSIONID, "SessionID" },
	{ HEADER_USERNAME, "PREMO-UserName" },
	{ HEADER_VERSION, "PREMO-Version" },
	{ HEADER_VIDEO_BITRATE, "PREMO-Video-Bitrate" },
	{ HEADER_VIDEO_CLOCKFREQ, "PREMO-Video-ClockFrequency" },
	{ HEADER_VIDEO_CODEC, "PREMO-Video-Codec" },
	{ HEADER_VIDEO_FRAMERATE, "PREMO-Video-Framerate" },
	{ HEADER_VIDEO_RESOLUTION, "PREMO-Video-Resolution" },

	{ HEADER_NULL, "null" },
};

static const char *orpGetHeader(enum orpHeader header)
{
	Uint32 i;
	for (i = 0; orpHeaderList[i].header != HEADER_NULL; i++) {
		if (orpHeaderList[i].header != header) continue;
		return orpHeaderList[i].name.c_str();
	}
	return orpHeaderList[HEADER_NULL].name.c_str();
}

static const char *orpGetHeaderValue(enum orpHeader header, vector<struct orpHeaderValue_t *> &headerList)
{
	Uint32 i;
	for (i = 0; headerList[i]->header != HEADER_NULL; i++) {
		if (headerList[i]->header != header) continue;
		return headerList[i]->value.c_str();
	}
	return NULL;
}

static struct orpCtrlMode_t orpCtrlList[] = {
	{ CTRL_CHANGE_BITRATE, "change-bitrate" },
	{ CTRL_SESSION_TERM, "session-term" },

	{ CTRL_NULL, "null" }
};

static void orpPostEvent(enum orpEvent id)
{
	SDL_Event event;

	event.type = SDL_USEREVENT;
	event.user.code = id;
	event.user.data1 = NULL;
	event.user.data2 = NULL;

	SDL_PushEvent(&event);
}

static size_t orpParseResponseHeader(void *ptr, size_t size, size_t nmemb, void *stream)
{
	Uint8 *data = (Uint8 *)ptr;
	Uint32 len = (Uint32)(size * nmemb);
	vector<struct orpHeaderValue_t *> *headerList = (vector<struct orpHeaderValue_t *> *)stream;

	Uint32 i;
	ostringstream h, v;
	for (i = 0; i < len; i++) {
		if (data[i] == (Uint8)' ' || data[i] == (Uint8)'\t' ||
			data[i] == (Uint8)'\n' || data[i] == (Uint8)'\r') continue;
		else if (data[i] == (Uint8)':') break;
		h << (char)data[i];
	}

	// Headers received...
	if (h.str().length() == 0) return (size_t)len;

	for (++i; i < len; i++) {
		if (data[i] == (Uint8)' ' || data[i] == (Uint8)'\t' ||
			data[i] == (Uint8)'\n' || data[i] == (Uint8)'\r') continue;
		v << (char)data[i];
	}

	for (i = 0; orpHeaderList[i].header != HEADER_NULL; i++) {
		if (strcasecmp(orpHeaderList[i].name.c_str(), h.str().c_str())) continue;
		struct orpHeaderValue_t *header = new struct orpHeaderValue_t;
		header->header = orpHeaderList[i].header;
		header->value = v.str();
		headerList->push_back(header);
	}

	return (size_t)len;
}

static size_t orpParseStreamData(void *ptr, size_t size, size_t nmemb, void *stream)
{
	Uint8 *data = (Uint8 *)ptr;
	Uint32 len = (Uint32)(size * nmemb);
	struct orpConfigStream_t *_config = (struct orpConfigStream_t *)stream;
	struct orpStreamData_t *streamData = _config->stream;

	// Allocate frame buffer
	Uint8 *buffer = new Uint8[streamData->len + len];
	if (!buffer) return 0;
	Uint8 *bp = buffer;

	// Prepend stored chunk if present
	if (streamData->len) {
		memcpy(bp, streamData->data, streamData->len);
		memcpy(bp + streamData->len, data, len);
		len += streamData->len;
		delete [] streamData->data;
		streamData->len = 0;
	} else memcpy(bp, data, len);

	// Eat 0x0d 0x0a prefix if present
	if (len >= 1 && bp[0] == 0x0d) {
		len--; bp++;
	}
	if (len >= 1 && bp[0] == 0x0a) {
		len--; bp++;
	}

	// Need at least 8 bytes (XXXXXX\n\r) to continue
	if (len < 8) {
		streamData->len = len;
		streamData->data = new Uint8[len];
		if (!streamData->data) {
			delete [] buffer;
			return 0;
		}
		memcpy(streamData->data, bp, len);
		delete [] buffer;
		return (size * nmemb);
	}

	// Expected trailing 0xd 0xa not found!
	if (bp[6] != 0x0d || bp[7] != 0x0a) {
		delete [] buffer;
		return 0;
	}

	// Zero out first byte after chunk length (for sscanf)
	bp[6] = 0x00;
	Uint32 chunk_len = 0;
//	printf("%02x %02x %02x %02x %02x %02x\n",
//		bp[0], bp[1], bp[2], bp[3], bp[4], bp[5], bp[6]);

	// Extract chunk length
	if (sscanf((const char *)bp, "%06x", &chunk_len) != 1) {
		delete [] buffer;
		return 0;
	}

	// Remove chunk length; advance buffer, decrement length
	len -= 8; bp += 8;

	// Available data less than what we need?
	if (len < chunk_len) {
		// Undo buffer and length adjustments for next packet
		len += 8; bp -= 8;
		bp[6] = 0x0d;

		// Allocate memory to hold partial packet
		streamData->len = len;
		streamData->data = new Uint8[len];
		if (!streamData->data) {
			delete [] buffer;
			return 0;
		}
		memcpy(streamData->data, bp, len);
		delete [] buffer;

		// Next...
		return (size * nmemb);

	} else if (len > chunk_len && len - chunk_len == 2 &&
		*(bp + chunk_len) == 0x0d && *(bp + chunk_len + 1) == 0x0a) {
		// Don't stash trailing 0xd 0xa...

	// More data available than we need?
	} else if (len > chunk_len) {
		// Save the overflow for next call...
		streamData->len = len - chunk_len;
		streamData->data = new Uint8[streamData->len];
		if (!streamData->data) {
			delete [] buffer;
			return 0;
		}
		memcpy(streamData->data, bp + chunk_len, streamData->len);
	}

	// We have a full audio/video packet (frame)
	if (bp[0] != 0x80) {
		cerr << "invalid " << _config->name << " packet header" << endl;
		delete [] buffer;
		return 0;
	}

	// Asked to restore audio/video stream?
	if (bp[1] == 0xfd) {
		cerr << _config->name << " restore (reset)" << endl;
		orpPostEvent(EVENT_RESTORE);
		delete [] buffer;
		return (size * nmemb);
	}

	// Audio/video magic of 0x8081 seems to mean PS3 shutdown
	if (bp[1] == 0x81) {
		cerr << _config->name << " system shutdown" << endl;
		orpPostEvent(EVENT_SHUTDOWN);
		delete [] buffer;
		return (size * nmemb);
	}

	// Audio or video header?
	if (bp[1] != 0x80 && bp[1] != 0xff && bp[1] != 0xfb && bp[1] != 0xfc) {
		printf("invalid %s magic: 0x%02x%02x\n", _config->name.c_str(),
			bp[0], bp[1]);
		delete [] buffer;
		// Discard...
		return (size * nmemb);
	}

	// Allocate new packet
	struct orpStreamPacket_t *packet = new struct orpStreamPacket_t;
	if (!packet) {
		delete [] buffer;
		return 0;
	}

	// Copy header, set length
	memcpy(&packet->header, bp, sizeof(struct orpStreamPacketHeader_t));
	packet->len = chunk_len - sizeof(struct orpStreamPacketHeader_t);

	// Header size must match packet length
	if (packet->len != SDL_Swap16(packet->header.len)) {
//		FILE *fh = fopen("bad-header.dat", "w+");
//		if (fh) {
//			fwrite(&packet->header, 1, sizeof(orpStreamPacketHeader_t), fh);
//			fclose(fh);
//		}
//		cerr << _config->name;
//		cerr << " packet length mis-match: " << packet->len;
//		cerr << ", expected: " << SDL_Swap16(packet->header.len);
//		cerr << endl;

		// TODO: For now we just accept the header length over the
		// chunk size.  This only seems to happen with MPEG4 video, ex:
		// PixelJunk Monsers/Eden
		packet->len = SDL_Swap16(packet->header.len);
	}

	// Allocate payload
	packet->data = new Uint8[packet->len];
	if (!packet->data) {
		delete [] packet;
		delete [] buffer;
		return 0;
	}

	// Copy payload
	bp += sizeof(struct orpStreamPacketHeader_t);
	memcpy(packet->data, bp, packet->len);
	delete [] buffer;

	// Decrypt video key-frames and encrypted audio frames
	if (((packet->header.magic[1] == 0x80 ||
		packet->header.magic[1] == 0xfc) && packet->header.unk8)
		|| packet->header.unk6 == 0x0401 || packet->header.unk6 == 0x0001) {
		memcpy(_config->key->iv1,
			_config->key->xor_nonce, ORP_KEY_LEN);
		AES_cbc_encrypt(packet->data, packet->data,
			packet->len - (packet->len % ORP_KEY_LEN),
			&_config->aes_key, _config->key->iv1, AES_DECRYPT);
	}

	// Push stream packet
	SDL_LockMutex(streamData->packetLock);
	streamData->packetList.push(packet);
	SDL_CondSignal(streamData->packetCond);
	SDL_UnlockMutex(streamData->packetLock);

	return (size * nmemb);
}

static bool orpDecodeKey(Uint8 *dst, const string &src)
{
	Base64 base64;
	Uint8 *result = base64.Decode((const Uint8 *)src.c_str());
	if (!result) return false;
	memcpy(dst, result, ORP_KEY_LEN);
	delete [] result;
	return true;
}

static SDL_mutex *orpAVMutex = NULL;

static Sint32 orpThreadVideoDecode(void *config)
{
	struct orpThreadDecode_t *_config = (struct orpThreadDecode_t *)config;
	struct orpStreamPacket_t *packet;

	SDL_LockMutex(orpAVMutex);
	AVCodecContext *context = avcodec_alloc_context();
	if (avcodec_open(context, _config->codec) < 0) {
		SDL_UnlockMutex(orpAVMutex);
		return -1;
	}
	SDL_UnlockMutex(orpAVMutex);

	AVFrame *frame;
	frame = avcodec_alloc_frame();

	struct SwsContext *sws_normal;
	struct SwsContext *sws_medium;
	struct SwsContext *sws_large;
	sws_normal = sws_getContext(ORP_FRAME_WIDTH, ORP_FRAME_HEIGHT,
		context->pix_fmt, ORP_FRAME_WIDTH, ORP_FRAME_HEIGHT,
		PIX_FMT_YUV420P, SWS_BICUBIC, NULL, NULL, NULL);
	sws_medium = sws_getContext(ORP_FRAME_WIDTH, ORP_FRAME_HEIGHT,
		context->pix_fmt,
			(int)(ORP_FRAME_WIDTH * 1.5), (int)(ORP_FRAME_HEIGHT * 1.5),
		PIX_FMT_YUV420P, SWS_BICUBIC, NULL, NULL, NULL);
	sws_large = sws_getContext(ORP_FRAME_WIDTH, ORP_FRAME_HEIGHT,
		context->pix_fmt, ORP_FRAME_WIDTH * 2, ORP_FRAME_HEIGHT * 2,
		PIX_FMT_YUV420P, SWS_BICUBIC, NULL, NULL, NULL);

	Sint32 bytes_decoded, frame_done = 0;
#ifdef ORP_DUMP_VIDEO_HEADER
	FILE *h_header = fopen("video-header.dat", "w+");
#endif
#ifdef ORP_DUMP_VIDEO_STREAM
	FILE *h_stream = fopen("video-stream.dat", "w+");
#endif
	while (!_config->terminate) {
		SDL_LockMutex(_config->stream->packetLock);
		Uint32 packets = _config->stream->packetList.size();
		if (packets == 0)
			SDL_CondWait(_config->stream->packetCond, _config->stream->packetLock);
		if (_config->terminate) {
			SDL_UnlockMutex(_config->stream->packetLock);
			break;
		}

		if (packets) {
			packet = _config->stream->packetList.front();
			_config->stream->packetList.pop();
		} else packet = NULL;
		SDL_UnlockMutex(_config->stream->packetLock);

		if (!packet) continue;
#ifdef ORP_DUMP_VIDEO_HEADER
		fwrite(&packet->header,
			1, sizeof(struct orpStreamPacketHeader_t), h_header);
#endif
#ifdef ORP_DUMP_VIDEO_STREAM
		fwrite(packet->data, 1, packet->len, h_stream);
#endif
		bytes_decoded = avcodec_decode_video(context,
			frame, &frame_done, packet->data, packet->len);

		if (bytes_decoded != -1 && frame_done) {
			SDL_LockMutex(_config->view->viewLock);
			SDL_LockYUVOverlay(_config->view->overlay);

			AVPicture p;
			p.data[0] = _config->view->overlay->pixels[0];
			p.data[1] = _config->view->overlay->pixels[2];
			p.data[2] = _config->view->overlay->pixels[1];

			p.linesize[0] = _config->view->overlay->pitches[0];
			p.linesize[1] = _config->view->overlay->pitches[2];
			p.linesize[2] = _config->view->overlay->pitches[1];

			if (_config->view->size == VIEW_FULLSCREEN ||
				_config->view->size == VIEW_NORMAL) {
				sws_scale(sws_normal,
					frame->data, frame->linesize, 0, context->height,
					p.data, p.linesize);
			} else if (_config->view->size == VIEW_MEDIUM) {
				sws_scale(sws_medium,
					frame->data, frame->linesize, 0, context->height,
					p.data, p.linesize);
			} else if (_config->view->size == VIEW_LARGE) {
				sws_scale(sws_large,
					frame->data, frame->linesize, 0, context->height,
					p.data, p.linesize);
			}

			SDL_UnlockYUVOverlay(_config->view->overlay);

			SDL_Rect rect;
			rect.x = 0;
			rect.y = 0;
			rect.w = _config->view->scale.w;
			rect.h = _config->view->scale.h;

			SDL_DisplayYUVOverlay(_config->view->overlay, &rect);
			SDL_UnlockMutex(_config->view->viewLock);
		}
//		else if (bytes_decoded == -1) cerr << "frame decode failed" << endl;

		delete [] packet->data;
		delete packet;
	}
#ifdef ORP_DUMP_VIDEO_HEADER
	fclose(h_header);
#endif
#ifdef ORP_DUMP_VIDEO_STREAM
	fclose(h_stream);
#endif
	av_free(frame);
	SDL_LockMutex(orpAVMutex);
	avcodec_close(context);
	SDL_UnlockMutex(orpAVMutex);
	sws_freeContext(sws_normal);
	sws_freeContext(sws_medium);
	sws_freeContext(sws_large);
	return 0;
}

static Sint32 orpThreadVideoConnection(void *config)
{
	struct orpConfigStream_t *_config = (struct orpConfigStream_t *)config;
	_config->name = "video";

	Base64 base64;
	Uint8 *encoded = base64.Encode(_config->key->akey, ORP_KEY_LEN);
	if (!encoded) return -1;

	string authkey = (const char *)encoded;
	delete [] encoded;

	CURL *curl = curl_easy_init();

	curl_easy_setopt(curl, CURLOPT_URL, _config->url.c_str());
	curl_easy_setopt(curl, CURLOPT_VERBOSE, 1);
	curl_easy_setopt(curl, CURLOPT_USERAGENT, ORP_USER_AGENT);

	vector<struct orpHeaderValue_t *> headerList;
	curl_easy_setopt(curl, CURLOPT_HEADERDATA, (void *)&headerList);
	curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, orpParseResponseHeader);

	curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)_config);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, orpParseStreamData);

	curl_easy_setopt(curl, CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_1_1);
	curl_easy_setopt(curl, CURLOPT_HTTP_CONTENT_DECODING, 0);
	curl_easy_setopt(curl, CURLOPT_HTTP_TRANSFER_DECODING, 0);

	struct curl_slist *headers = NULL;

	ostringstream os;
	os << "Accept: */*;q=0.01";
	headers = curl_slist_append(headers, os.str().c_str());

	os.str("");
	os << "Accept-Charset: iso-8859-1;q=0.01";
	headers = curl_slist_append(headers, os.str().c_str());

	os.str("");
	os << orpGetHeader(HEADER_VIDEO_CODEC);
	os << ": " << _config->codec;
	headers = curl_slist_append(headers, os.str().c_str());

	os.str("");
	os << orpGetHeader(HEADER_VIDEO_RESOLUTION);
	os << ": " << ORP_FRAME_WIDTH << "x" << ORP_FRAME_HEIGHT;
	headers = curl_slist_append(headers, os.str().c_str());

	os.str("");
	os << orpGetHeader(HEADER_AUTH);
	os << ": " << authkey;
	headers = curl_slist_append(headers, os.str().c_str());

	os.str("");
	os << orpGetHeader(HEADER_SESSIONID);
	os << ": " << _config->session_id;
	headers = curl_slist_append(headers, os.str().c_str());

	os.str("");
	os << "Connection: Keep-Alive";
	headers = curl_slist_append(headers, os.str().c_str());

	curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

	CURLcode cc;
		
	cc = curl_easy_perform(curl);
	curl_slist_free_all(headers);
	curl_easy_cleanup(curl);

	return 0;
}

static void orpAudioFeed(void *config, Uint8 *stream, int len)
{
	struct orpConfigAudioFeed_t *_config =
		(struct orpConfigAudioFeed_t *)config;

	SDL_LockMutex(_config->feedLock);
	if (_config->frameList.size()) {
		struct orpAudioFrame_t *frame;
		frame = _config->frameList.front();
		_config->frameList.pop();
		memcpy(stream, frame->data, len);
		delete [] frame->data;
		delete frame;
	} else memset(stream, 0, len);
	SDL_UnlockMutex(_config->feedLock);
}

static AVCodecContext *orpInitAudioCodec(AVCodec *codec, Sint32 channels, Sint32 sample_rate, Sint32 bit_rate)
{
	SDL_LockMutex(orpAVMutex);
	AVCodecContext *context = avcodec_alloc_context();
	context->channels = channels;
	context->sample_rate = sample_rate;
	if (codec->id == CODEC_ID_ATRAC3) {
		struct atrac3_config_t {
			Uint16 unk0;	// 2
			Uint32 unk1;	// 6
			Uint16 unk2;	// 8
			Uint16 unk3;	// 10
			Uint16 unk4;	// 12
			Uint16 unk5;	// 14
		};
		struct atrac3_config_t *at3_config;
		at3_config = (struct atrac3_config_t *)av_malloc(sizeof(struct atrac3_config_t));
		memset(at3_config, 0, sizeof(struct atrac3_config_t));
		at3_config->unk0 = 1;
		at3_config->unk1 = 1024;
		at3_config->unk2 = 1;
		at3_config->unk3 = 1;
		at3_config->unk4 = 1;
		at3_config->unk5 = 0;
		context->extradata = (Uint8 *)at3_config;
		context->extradata_size = 14;
		//context->block_align = 96 * 2;
		//context->block_align = 152 * 2;
		context->block_align = 192 * channels;
		context->bit_rate = bit_rate;
	}

	if (avcodec_open(context, codec) < 0) {
		if (codec->id == CODEC_ID_ATRAC3) av_free(context->extradata);
		av_free(context);
		SDL_UnlockMutex(orpAVMutex);
		cerr << "codec context initialization failed.\n";
		return NULL;
	}
	SDL_UnlockMutex(orpAVMutex);

	return context;
}

static Sint32 orpThreadAudioDecode(void *config)
{
	struct orpThreadDecode_t *_config = (struct orpThreadDecode_t *)config;
	struct orpStreamPacket_t *packet;

	if (!_config->codec) return -1;

	// TODO: These values must come from the session headers
	// rather than be hard coded here...
	Sint32 channels = 2, sample_rate = 48000, bit_rate = 128000;

	AVCodecContext *context;
	if (!(context = orpInitAudioCodec(_config->codec,
		channels, sample_rate, bit_rate))) return -1;

	Sint32 bytes_decoded, frame_size;
	Uint8 buffer[(AVCODEC_MAX_AUDIO_FRAME_SIZE * 3) / 2];

	struct orpConfigAudioFeed_t feed;
	feed.feedLock = SDL_CreateMutex();

	SDL_AudioSpec audioSpec, requestedSpec;
	requestedSpec.freq = sample_rate;
	requestedSpec.format = AUDIO_S16SYS;
	requestedSpec.channels = channels;
	requestedSpec.silence = 0;
	requestedSpec.samples = 1024;
	requestedSpec.callback = orpAudioFeed;
	requestedSpec.userdata = (void *)&feed;

	if(SDL_OpenAudio(&requestedSpec, &audioSpec) == -1) {
		cerr << SDL_GetError();
		return -1;
	}

	Sint32 decode_errors = 0;
	SDL_PauseAudio(0);
#ifdef ORP_DUMP_AUDIO_HEADER
	FILE *h_header = fopen("audio-header.dat", "w+");
#endif
#ifdef ORP_DUMP_AUDIO_STREAM
	FILE *h_stream = fopen("audio-stream.dat", "w+");
#endif
	while (!_config->terminate) {
		SDL_LockMutex(_config->stream->packetLock);
		Uint32 packets = _config->stream->packetList.size();
		if (packets == 0)
			SDL_CondWait(_config->stream->packetCond, _config->stream->packetLock);
		if (_config->terminate) {
			SDL_UnlockMutex(_config->stream->packetLock);
			break;
		}

		if (_config->stream->packetList.size()) {
			packet = _config->stream->packetList.front();
			_config->stream->packetList.pop();
		} else packet = NULL;
		SDL_UnlockMutex(_config->stream->packetLock);

		if (!packet) continue;
#ifdef ORP_DUMP_AUDIO_HEADER
		fwrite(&packet->header,
			1, sizeof(struct orpStreamPacketHeader_t), h_header);
#endif
#ifdef ORP_DUMP_AUDIO_STREAM
		fwrite(packet->data, 1, packet->len, h_stream);
#endif
		frame_size = sizeof(buffer);
		bytes_decoded = avcodec_decode_audio2(context,
			(Sint16 *)buffer, &frame_size, packet->data, packet->len);
		if (bytes_decoded != -1 && frame_size) {
			struct orpAudioFrame_t *audioFrame =
				new struct orpAudioFrame_t;
		
			audioFrame->len = (Uint32)frame_size;
			audioFrame->data = new Uint8[audioFrame->len];
			memcpy(audioFrame->data, buffer, audioFrame->len);

			SDL_LockMutex(feed.feedLock);
			feed.frameList.push(audioFrame);
			SDL_UnlockMutex(feed.feedLock);
			decode_errors = 0;
		} else if (decode_errors > 5) {
			SDL_LockMutex(orpAVMutex);
			if (_config->codec->id == CODEC_ID_ATRAC3)
				av_free(context->extradata);
			avcodec_close(context);
			SDL_UnlockMutex(orpAVMutex);
			if (!(context = orpInitAudioCodec(_config->codec,
				channels, sample_rate, bit_rate))) return -1;
			decode_errors = 0;
		} else decode_errors++;

		delete [] packet->data;
		delete packet;
	}
	
	SDL_PauseAudio(1);
	SDL_CloseAudio();
	SDL_DestroyMutex(feed.feedLock);
#ifdef ORP_DUMP_AUDIO_HEADER
	fclose(h_header);
#endif
#ifdef ORP_DUMP_AUDIO_STREAM
	fclose(h_stream);
#endif
	SDL_LockMutex(orpAVMutex);
	if (_config->codec->id == CODEC_ID_ATRAC3) av_free(context->extradata);
	avcodec_close(context);
	SDL_UnlockMutex(orpAVMutex);
	return 0;
}

static Sint32 orpThreadAudioConnection(void *config)
{
	struct orpConfigStream_t *_config = (struct orpConfigStream_t *)config;
	_config->name = "audio";

	Base64 base64;
	Uint8 *encoded = base64.Encode(_config->key->akey, ORP_KEY_LEN);
	if (!encoded) return -1;

	string authkey = (const char *)encoded;
	delete [] encoded;

	CURL *curl = curl_easy_init();

	curl_easy_setopt(curl, CURLOPT_URL, _config->url.c_str());
	curl_easy_setopt(curl, CURLOPT_VERBOSE, 1);
	curl_easy_setopt(curl, CURLOPT_USERAGENT, ORP_USER_AGENT);

	vector<struct orpHeaderValue_t *> headerList;
	curl_easy_setopt(curl, CURLOPT_HEADERDATA, (void *)&headerList);
	curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, orpParseResponseHeader);

	curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)_config);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, orpParseStreamData);

	curl_easy_setopt(curl, CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_1_1);
	curl_easy_setopt(curl, CURLOPT_HTTP_CONTENT_DECODING, 0);
	curl_easy_setopt(curl, CURLOPT_HTTP_TRANSFER_DECODING, 0);

	struct curl_slist *headers = NULL;

	ostringstream os;
	os << "Accept: */*;q=0.01";
	headers = curl_slist_append(headers, os.str().c_str());

	os.str("");
	os << "Accept-Charset: iso-8859-1;q=0.01";
	headers = curl_slist_append(headers, os.str().c_str());

	os.str("");
	os << orpGetHeader(HEADER_AUDIO_CODEC);
	os << ": " << _config->codec;
	headers = curl_slist_append(headers, os.str().c_str());

	os.str("");
	os << orpGetHeader(HEADER_AUDIO_BITRATE);
	os << ": " << "128000";
	headers = curl_slist_append(headers, os.str().c_str());

	os.str("");
	os << orpGetHeader(HEADER_AUTH);
	os << ": " << authkey;
	headers = curl_slist_append(headers, os.str().c_str());

	os.str("");
	os << orpGetHeader(HEADER_SESSIONID);
	os << ": " << _config->session_id;
	headers = curl_slist_append(headers, os.str().c_str());

	os.str("");
	os << "Connection: Keep-Alive";
	headers = curl_slist_append(headers, os.str().c_str());

	curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

	CURLcode cc;
		
	cc = curl_easy_perform(curl);
	curl_slist_free_all(headers);
	curl_easy_cleanup(curl);

	return 0;
}

OpenRemotePlay::OpenRemotePlay(struct orpConfig_t *config)
	: ps3_nickname(NULL),
	thread_video_connection(NULL), thread_video_decode(NULL),
	thread_audio_connection(NULL), thread_audio_decode(NULL)
{
	// Copy config
	memcpy(&this->config, config, sizeof(struct orpConfig_t));

	// Init libcurl
	curl_global_init(CURL_GLOBAL_WIN32);

	// Init libavcodec, load all codecs
	avcodec_init();
	avcodec_register_all();

	// Initialize the audio/video decoders we need
	struct orpCodec_t *oc;
	AVCodec *codec;

	codec = avcodec_find_decoder(CODEC_ID_H264);
	if (!codec) {
		cerr << "Required codec not found: CODEC_ID_H264\n";
		throw -1;
	}
	oc = new struct orpCodec_t;
	oc->name = "AVC";
	oc->codec = codec;
	this->codec.push_back(oc);

	codec = avcodec_find_decoder(CODEC_ID_MPEG4);
	if (!codec) {
		cerr << "Required codec not found: CODEC_ID_MPEG4\n";
		throw -1;
	}
	oc = new struct orpCodec_t;
	oc->name = "M4V";
	oc->codec = codec;
	this->codec.push_back(oc);

	codec = avcodec_find_decoder(CODEC_ID_AAC);
	if (!codec) {
		cerr << "Required codec not found: CODEC_ID_AAC\n";
		throw -1;
	}
	oc = new struct orpCodec_t;
	oc->name = "M4A";
	oc->codec = codec;
	this->codec.push_back(oc);

	codec = avcodec_find_decoder(CODEC_ID_ATRAC3);
	if (!codec) {
		cerr << "Required codec not found: CODEC_ID_ATRAC3\n";
		throw -1;
	}
	oc = new struct orpCodec_t;
	oc->name = "AT3";
	oc->codec = codec;
	this->codec.push_back(oc);

	view.view = NULL;
	view.overlay = NULL;
	view.size = VIEW_NORMAL;
	view.scale.x = view.scale.y = 0;
	view.scale.w = ORP_FRAME_WIDTH;
	view.scale.h = ORP_FRAME_HEIGHT;
	view.viewLock = SDL_CreateMutex();
	orpAVMutex = SDL_CreateMutex();
}

OpenRemotePlay::~OpenRemotePlay()
{
	SessionDestroy();
	SDL_DestroyMutex(orpAVMutex);
}

bool OpenRemotePlay::SessionCreate(void)
{
	// Initialize audio and video
	if (SDL_Init(SDL_INIT_AUDIO | SDL_INIT_VIDEO)) {
		cerr << SDL_GetError();
		return false;
	}

	// Create window
	if (!view.view) { if (!CreateView()) return false; }

	// Set window caption and display splash logo
	SDL_WM_SetCaption("Open Remote Play", NULL);
	SDL_RWops *rw;
	if ((rw = SDL_RWFromConstMem(splash_png, splash_png_len))) {
		SDL_Surface *splash = IMG_Load_RW(rw, 0);

		SDL_Rect rect;

		rect.x = 0;
		rect.y = 0;
		rect.w = view.scale.w;
		rect.h = view.scale.h;

		SDL_BlitSurface(splash, NULL, view.view, &rect);
		SDL_UpdateRect(view.view, rect.x, rect.y, rect.w, rect.h);
		SDL_FreeRW(rw);
	}

	// Initialize event thread
	SDL_InitSubSystem(SDL_INIT_EVENTTHREAD);

	IPaddress addr;
	if (SDLNet_ResolveHost(&addr,
		config.ps3_addr, config.ps3_port) != 0) {
		cerr << "Error resolving address.\n";
		return false;
	}
	//UDPsocket skt = SDLNet_UDP_Open(0);
	UDPsocket skt = SDLNet_UDP_Open(ORP_PORT);
	Sint32 channel;
	if ((channel = SDLNet_UDP_Bind(skt, -1, &addr)) == -1) {
		cerr << "Error binding socket.\n";
		return false;
	}

	struct PktAnnounceSrch_t srch;
	CreatePktAnnounceSrch(srch);
	UDPpacket *pkt_srch = SDLNet_AllocPacket(sizeof(struct PktAnnounceSrch_t));
	pkt_srch->len = sizeof(struct PktAnnounceSrch_t);
	memcpy(pkt_srch->data, &srch, sizeof(struct PktAnnounceSrch_t));
	UDPpacket *pkt_resp = SDLNet_AllocPacket(sizeof(struct PktAnnounceResp_t));

	// Fire it up!
	Sint32 i, replies = 0, first = 1;
	for (i = 0; i < 30; i++) {
		if (SDLNet_UDP_Send(skt, channel, pkt_srch) == 0) {
			cerr << "Error sending packet.\n";
			break;
		}

		Sint32 result;
		if ((result = SDLNet_UDP_Recv(skt, pkt_resp)) == -1) {
			cerr << "Error receiving packet.\n";
			break;
		}

		if (result == 0) {
			cerr << "No reply, trying again..." << endl;
			replies = 0;
			SDL_Delay(1000);
			continue;
		}

		replies++;
		cerr << "Reply of " << pkt_resp->len << " bytes." << endl;

		if (!first && replies < 5) {
			SDL_Delay(1000);
			continue;
		}

		if (SessionPerform() != EVENT_RESTORE) break;
		first = i = replies = 0;
	}

	SDLNet_FreePacket(pkt_srch);
	SDLNet_FreePacket(pkt_resp);
	SDLNet_UDP_Close(skt);

	return true;
}

void OpenRemotePlay::SessionDestroy(void)
{
	if (view.view && view.size == VIEW_FULLSCREEN)
		SDL_SetVideoMode(view.scale.w, view.scale.h, 0, 0);
	view.view = NULL;
	view.overlay = NULL;
	SDL_Quit();
}

bool OpenRemotePlay::CreateView(void)
{
	Uint32 flags = 0;
	switch (view.size)
	{
	case VIEW_FULLSCREEN:
		flags |= SDL_FULLSCREEN;
	case VIEW_NORMAL:
		view.scale.w = ORP_FRAME_WIDTH;
		view.scale.h = ORP_FRAME_HEIGHT;
		break;
	case VIEW_MEDIUM:
		view.scale.w = (int)(ORP_FRAME_WIDTH * 1.5);
		view.scale.h = (int)(ORP_FRAME_HEIGHT * 1.5);
		break;
	case VIEW_LARGE:
		view.scale.w = ORP_FRAME_WIDTH * 2;
		view.scale.h = ORP_FRAME_HEIGHT * 2;
		break;
	}
	if (view.overlay) SDL_FreeYUVOverlay(view.overlay);
	if (!(view.view = SDL_SetVideoMode(
		view.scale.w, view.scale.h, 0, flags))) {
		cerr << SDL_GetError();
		return false;
	}
	if (!(view.overlay = SDL_CreateYUVOverlay(
		view.scale.w, view.scale.h,
		SDL_YV12_OVERLAY, view.view))) {
		cerr << SDL_GetError();
		return false;
	}
	SDL_ShowCursor(!(view.size == VIEW_FULLSCREEN));
	return true;
}

bool OpenRemotePlay::CreateKeys(const string &nonce)
{
	if (!orpDecodeKey(config.key.nonce, nonce)) return false;

	memcpy(config.key.xor_pkey, config.key.pkey, ORP_KEY_LEN);
	memcpy(config.key.xor_nonce, config.key.nonce, ORP_KEY_LEN);

	Sint32 i;
	for (i = 0; i < ORP_KEY_LEN; i++)
		config.key.xor_pkey[i] ^= config.key.skey0[i];
	for (i = 0; i < ORP_KEY_LEN; i++)
		config.key.xor_nonce[i] ^= config.key.skey2[i];

	memcpy(config.key.iv1, config.key.xor_nonce, ORP_MAC_LEN);

	memset(config.key.akey, 0, ORP_KEY_LEN);
	memcpy(config.key.akey, config.psp_mac, ORP_MAC_LEN);

	AES_KEY aes_key_encrypt;
	AES_set_encrypt_key(config.key.xor_pkey,
		ORP_KEY_LEN * 8, &aes_key_encrypt);
	AES_cbc_encrypt(config.key.akey, config.key.akey, ORP_KEY_LEN,
		&aes_key_encrypt, config.key.iv1, AES_ENCRYPT);

	return true;
}

AVCodec *OpenRemotePlay::GetCodec(const string &name)
{
	Uint32 i;
	for (i = 0; i < codec.size(); i++) {
		if (strcasecmp(name.c_str(), codec[i]->name.c_str())) continue;
		return codec[i]->codec;
	}
}

Sint32 OpenRemotePlay::ControlPerform(CURL *curl, struct orpCtrlMode_t *mode)
{
	struct curl_slist *headers = NULL;

	ostringstream os;
	os << "Accept: */*;q=0.01";
	headers = curl_slist_append(headers, os.str().c_str());

	os.str("");
	os << "Accept-Charset: iso-8859-1;q=0.01";
	headers = curl_slist_append(headers, os.str().c_str());

	if (mode->mode == CTRL_CHANGE_BITRATE) {
		os.str("");
		os << orpGetHeader(HEADER_CTRL_MODE);
		os << ": " << "change-bitrate";
		headers = curl_slist_append(headers, os.str().c_str());

		os.str("");
		os << orpGetHeader(HEADER_CTRL_BITRATE);
		// TODO: Support these bit-rates
		//os << ": " << "384000";
		//os << ": " << "768000";
		//os << ": " << "1024000";
		os << ": " << mode->param1;
		headers = curl_slist_append(headers, os.str().c_str());

		os.str("");
		os << orpGetHeader(HEADER_CTRL_MAXBITRATE);
		//os << ": " << "768000";
		//os << ": " << "1024000";
		os << ": " << mode->param2;
		headers = curl_slist_append(headers, os.str().c_str());
	} else if (mode->mode == CTRL_SESSION_TERM) {
		os.str("");
		os << orpGetHeader(HEADER_CTRL_MODE);
		os << ": " << "session-term";
		headers = curl_slist_append(headers, os.str().c_str());
	}

	os.str("");
	os << orpGetHeader(HEADER_SESSIONID);
	os << ": " << session_id;
	headers = curl_slist_append(headers, os.str().c_str());

	os.str("");
	os << "Connection: Keep-Alive";
	headers = curl_slist_append(headers, os.str().c_str());

	curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

	CURLcode cc;
	cc = curl_easy_perform(curl);
	curl_slist_free_all(headers);
	headers = NULL;

	if (cc != 0) return -1;

	Uint32 code;
	curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &code);

	return (Sint32)code;
}

static void orpDumpPadState(Uint8 *state)
{
	bool toggle = false;
	Uint32 x, y, i = 0;
	for (y = 0; y < 4; y++) {
		for (x = 0; x < 32; x++) {
			printf("%02x", state[i]);
			if (toggle) {
				printf(" ");
				toggle = false;
			} else toggle = true;
			i++;
		}
		printf("\n");
	}
}

Sint32 OpenRemotePlay::SessionControl(void)
{
	CURL *curl = curl_easy_init();

	ostringstream os;
	os << "http://";
	os << config.ps3_addr << ":" << config.ps3_port;
	os << ORP_GET_CTRL;

	curl_easy_setopt(curl, CURLOPT_URL, os.str().c_str());
	curl_easy_setopt(curl, CURLOPT_VERBOSE, 1);
	curl_easy_setopt(curl, CURLOPT_USERAGENT, ORP_USER_AGENT);

	curl_easy_setopt(curl, CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_1_1);
	curl_easy_setopt(curl, CURLOPT_HTTP_CONTENT_DECODING, 0);
	curl_easy_setopt(curl, CURLOPT_HTTP_TRANSFER_DECODING, 0);

	IPaddress ip;
	if (SDLNet_ResolveHost(&ip, config.ps3_addr, config.ps3_port) != 0) {
		cerr << "Error resolving address.\n";
		return -1;
	}

	TCPsocket skt;
	if ((skt = SDLNet_TCP_Open(&ip)) == NULL) {
		cerr << "Input connection failure.\n";
		return -1;
	}

	vector<string> headers;

	os.str("");
	os << "POST " << ORP_POST_PAD << " HTTP/1.1\r\n";
	headers.push_back(os.str());

	os.str("");
	os << "User-Agent: " <<  ORP_USER_AGENT << "\r\n";
	headers.push_back(os.str());

	os.str("");
	os << "Accept: */*;q=0.01\r\n";
	headers.push_back(os.str());

	os.str("");
	os << "Accept-Encoding:\r\n";
	headers.push_back(os.str());

	os.str("");
	os << "Accept-Charset: iso-8859-1;q=0.01\r\n";
	headers.push_back(os.str());

	os.str("");
	os << "Host: " << config.ps3_addr << ":" << config.ps3_port << "\r\n";
	headers.push_back(os.str());

	os.str("");
	os << orpGetHeader(HEADER_SESSIONID);
	os << ": " << session_id << "\r\n";
	headers.push_back(os.str());

	os.str("");
	os << "Connection: Keep-Alive\r\n";
	headers.push_back(os.str());

	os.str("");
	os << "Content-Length: " << (ORP_PADSTATE_MAX * ORP_PADSTATE_LEN);
	os << "\r\n";
	headers.push_back(os.str());

	headers.push_back("\r\n");

	Uint32 i = 0;
	for (i = 0; i < headers.size(); i++)
		SDLNet_TCP_Send(skt, headers[i].c_str(), strlen(headers[i].c_str()));

	Uint8 statePad[ORP_PADSTATE_LEN] = {
		0x00, 0x00, 0x00, 0x74, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x80, 0x00, 0x80, 0x00, 0x80, 0x00, 0x80,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
	};

	Uint8 statePadHome[ORP_PADSTATE_LEN] = {
		0x00, 0x01, 0x00, 0x74, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x80, 0x00, 0x80, 0x00, 0x80, 0x00, 0x80,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
	};

	struct orpCtrlMode_t mode;

	Uint32 count = 0;
	Uint32 id = 0, be_id;
	Uint32 timestamp, be_timestamp;
	Uint32 ticks = SDL_GetTicks();

	SDL_Event event;

	// Flush any queued events...
	while (SDL_PollEvent(&event) > 0) cerr << "Flushing event...\n";

	// Main event loop
	for ( ;; ) {
		Uint32 key = 0;
		if (SDL_WaitEvent(&event) == 0) {
			cerr << SDL_GetError();
			return -1;
		}
		switch (event.type) {
		case SDL_QUIT:
			mode.mode = CTRL_SESSION_TERM;
			ControlPerform(curl, &mode);
			return 0;
		case SDL_USEREVENT:
			switch (event.user.code) {
			case EVENT_ERROR:
				cerr << "Error event\n";
				break;
			case EVENT_RESTORE:
				cerr << "Restore event\n";
				SDLNet_TCP_Close(skt);
				curl_easy_cleanup(curl);
				return EVENT_RESTORE;
			case EVENT_SHUTDOWN:
				mode.mode = CTRL_SESSION_TERM;
				ControlPerform(curl, &mode);
				return 0;
			}
			break;
		case SDL_KEYUP:
			switch (event.key.keysym.sym) {
			case SDLK_UP:
				key = ORP_PAD_KEYUP | ORP_PAD_PSP_DPUP;
				break;
			case SDLK_RIGHT:
				key = ORP_PAD_KEYUP | ORP_PAD_PSP_DPRIGHT;
				break;
			case SDLK_DOWN:
				key = ORP_PAD_KEYUP | ORP_PAD_PSP_DPDOWN;
				break;
			case SDLK_LEFT:
				key = ORP_PAD_KEYUP | ORP_PAD_PSP_DPLEFT;
				break;
			case SDLK_RETURN:
				key = ORP_PAD_KEYUP | ORP_PAD_PSP_X;
				break;
			case SDLK_F1:
				key = ORP_PAD_KEYUP | ORP_PAD_PSP_TRI;
				break;
			case SDLK_F2:
				key = ORP_PAD_KEYUP | ORP_PAD_PSP_SQUARE;
				break;
			case SDLK_PAGEUP:
				key = ORP_PAD_KEYUP | ORP_PAD_PSP_L1;
				break;
			case SDLK_PAGEDOWN:
				key = ORP_PAD_KEYUP | ORP_PAD_PSP_R1;
				break;
			case SDLK_ESCAPE:
				key = ORP_PAD_KEYUP | ORP_PAD_PSP_CIRCLE;
				break;
			// TODO: START/SELECT
			}
			break;
		case SDL_KEYDOWN:
			switch (event.key.keysym.sym) {
			case SDLK_q:
				if (event.key.keysym.mod == KMOD_LCTRL ||
					event.key.keysym.mod == KMOD_RCTRL) {
					mode.mode = CTRL_SESSION_TERM;
					ControlPerform(curl, &mode);
					return 0;
				}
				break;
			case SDLK_d:
				if ((event.key.keysym.mod == KMOD_LCTRL ||
					event.key.keysym.mod == KMOD_RCTRL) &&
					view.size != VIEW_FULLSCREEN) {
					SDL_LockMutex(view.viewLock);
					if (view.size == VIEW_NORMAL)
						view.size = VIEW_MEDIUM;
					else if (view.size == VIEW_MEDIUM)
						view.size = VIEW_LARGE;
					else if (view.size == VIEW_LARGE)
						view.size = VIEW_NORMAL;
					CreateView();
					SDL_UnlockMutex(view.viewLock);
				}
				break;
			case SDLK_f:
				if (event.key.keysym.mod == KMOD_LCTRL ||
					event.key.keysym.mod == KMOD_RCTRL) {
					SDL_LockMutex(view.viewLock);
					if (view.size == VIEW_FULLSCREEN)
						view.size = view.prev;
					else {
						view.prev = view.size;
						view.size = VIEW_FULLSCREEN;
					}
					CreateView();
					SDL_UnlockMutex(view.viewLock);
				}
				break;
			case SDLK_UP:
				key = ORP_PAD_KEYDOWN | ORP_PAD_PSP_DPUP;
				break;
			case SDLK_RIGHT:
				key = ORP_PAD_KEYDOWN | ORP_PAD_PSP_DPRIGHT;
				break;
			case SDLK_DOWN:
				key = ORP_PAD_KEYDOWN | ORP_PAD_PSP_DPDOWN;
				break;
			case SDLK_LEFT:
				key = ORP_PAD_KEYDOWN | ORP_PAD_PSP_DPLEFT;
				break;
			case SDLK_RETURN:
				key = ORP_PAD_KEYDOWN | ORP_PAD_PSP_X;
				break;
			case SDLK_ESCAPE:
				key = ORP_PAD_KEYDOWN | ORP_PAD_PSP_CIRCLE;
				break;
			case SDLK_F1:
				key = ORP_PAD_KEYDOWN | ORP_PAD_PSP_TRI;
				break;
			case SDLK_F2:
				key = ORP_PAD_KEYDOWN | ORP_PAD_PSP_SQUARE;
				break;
			case SDLK_PAGEUP:
				key = ORP_PAD_KEYDOWN | ORP_PAD_PSP_L1;
				break;
			case SDLK_PAGEDOWN:
				key = ORP_PAD_KEYDOWN | ORP_PAD_PSP_R1;
				break;
			case SDLK_HOME:
				key = ORP_PAD_KEYDOWN | ORP_PAD_PSP_HOME;
				break;
			// TODO: START/SELECT
			}
			break;
		}

		if (key) {
			id++;
			count++;
			Uint16 value = (Uint16)(key & 0x0000ffff);
			if (value == ORP_PAD_PSP_HOME) {
				//orpDumpPadState(statePadHome);
				SDLNet_TCP_Send(skt, statePadHome, ORP_PADSTATE_LEN);
				continue;
			}

			if (key & ORP_PAD_KEYUP) {
				statePad[((value & 0xff00) >> 8)] &=
					~((Uint8)(value & 0x00ff));
			} else {
				statePad[((value & 0xff00) >> 8)] |=
					((Uint8)(value & 0x00ff));
			}

			// TODO: This calculation is very approximate, needs to be fixed!
			timestamp = (SDL_GetTicks() - ticks) / 16;	

			be_id = SDL_Swap32(id);
			be_timestamp = SDL_Swap32(timestamp);
			memcpy(statePad + ORP_PAD_EVENTID, &be_id, 4);
			memcpy(statePad + ORP_PAD_TIMESTAMP, &be_timestamp, 4);
			//orpDumpPadState(statePad);

			SDLNet_TCP_Send(skt, statePad, ORP_PADSTATE_LEN);

			if (count == ORP_PADSTATE_MAX) {
				// TODO: Yes this is lazy, and not right...
				Uint8 reply[80];
				if (SDLNet_TCP_Recv(skt, reply, 80) != 80) {
					cerr << "Error receiving reply.\n";
					return -1;
				}
				count = 0;
				for (i = 0; i < headers.size(); i++) {
					SDLNet_TCP_Send(skt,
						headers[i].c_str(), strlen(headers[i].c_str()));
				}
			}
		}
	}

	// Never reached...
	return 0;
}

Sint32 OpenRemotePlay::SessionPerform(void)
{
	CURL *curl = curl_easy_init();

	ostringstream os;
	os << "http://";
	os << config.ps3_addr << ":" << config.ps3_port;
	os << ORP_GET_SESSION;

	curl_easy_setopt(curl, CURLOPT_URL, os.str().c_str());
	curl_easy_setopt(curl, CURLOPT_VERBOSE, 1);
	curl_easy_setopt(curl, CURLOPT_USERAGENT, ORP_USER_AGENT);

	vector<struct orpHeaderValue_t *> headerList;
	curl_easy_setopt(curl, CURLOPT_HEADERDATA, (void *)&headerList);
	curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, orpParseResponseHeader);

	curl_easy_setopt(curl, CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_1_1);
	curl_easy_setopt(curl, CURLOPT_HTTP_CONTENT_DECODING, 0);
	curl_easy_setopt(curl, CURLOPT_HTTP_TRANSFER_DECODING, 0);

	struct curl_slist *headers = NULL;

	Base64 base64;
	Uint8 *encoded;

	try {
		os.str("");
		os << "Accept: */*;q=0.01";
		headers = curl_slist_append(headers, os.str().c_str());

		os.str("");
		os << "Accept-Charset: iso-8859-1;q=0.01";
		headers = curl_slist_append(headers, os.str().c_str());

		encoded = base64.Encode(config.key.psp_id, ORP_KEY_LEN);
		if (!encoded) throw -1;
		os.str("");
		os << orpGetHeader(HEADER_PSPID);
		os << ": " << encoded;
		delete [] encoded;
		headers = curl_slist_append(headers, os.str().c_str());

		os.str("");
		os << orpGetHeader(HEADER_VERSION);
		os << ": " << ORP_PREMO_VMAJOR << "." << ORP_PREMO_VMINOR;
		headers = curl_slist_append(headers, os.str().c_str());

		os.str("");
		os << orpGetHeader(HEADER_MODE);
		os << ": " << "PREMO";
		headers = curl_slist_append(headers, os.str().c_str());

		os.str("");
		os << orpGetHeader(HEADER_PLATFORM_INFO);
		os << ": " << "PSP";
		headers = curl_slist_append(headers, os.str().c_str());

		os.str("");
		os << orpGetHeader(HEADER_PAD_INFO);
		os << ": " << "PSP-Pad";
		headers = curl_slist_append(headers, os.str().c_str());

		if (!(encoded = base64.Encode((const Uint8 *)config.psp_owner)))
			throw -1;

		os.str("");
		os << orpGetHeader(HEADER_USERNAME);
		os << ": " << encoded;
		delete [] encoded;
		headers = curl_slist_append(headers, os.str().c_str());

		os.str("");
		os << orpGetHeader(HEADER_TRANS);
		os << ": " << "capable";
		headers = curl_slist_append(headers, os.str().c_str());

		os.str("");
		os << "Connection: Keep-Alive";
		headers = curl_slist_append(headers, os.str().c_str());

		curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
	} catch (Sint32 e) {
		curl_slist_free_all(headers);
		return e;
	}

	CURLcode cc;
	cc = curl_easy_perform(curl);

	Uint32 code;
	curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &code);

	curl_slist_free_all(headers);
	curl_easy_cleanup(curl);

	if (cc != 0 || code != 200) return -1;
	if (!CreateKeys(orpGetHeaderValue(HEADER_NONCE, headerList)))
		return -1;

	AVCodec *videoCodec = GetCodec(
		orpGetHeaderValue(HEADER_VIDEO_CODEC, headerList));
	if (!videoCodec) {
		cerr << "Required video codec not found: ";
		cerr << orpGetHeaderValue(HEADER_VIDEO_CODEC, headerList) << endl;
		return -1;
	}
	AVCodec *audioCodec = GetCodec(
		orpGetHeaderValue(HEADER_AUDIO_CODEC, headerList));
	if (!audioCodec) {
		cerr << "Required audio codec not found: ";
		cerr << orpGetHeaderValue(HEADER_AUDIO_CODEC, headerList) << endl;
		return -1;
	}

	session_id = orpGetHeaderValue(HEADER_SESSIONID, headerList);

	ps3_nickname = (char *)base64.Decode((const Uint8 *)
		orpGetHeaderValue(HEADER_PS3_NICKNAME, headerList));
	if (ps3_nickname)
		SDL_WM_SetCaption((const char *)ps3_nickname, NULL);

	struct orpConfigStream_t *videoConfig = new struct orpConfigStream_t;
	os.str("");
	os << "http://";
	os << config.ps3_addr << ":" << config.ps3_port;
	os << ORP_GET_VIDEO;
	videoConfig->url = os.str();

	videoConfig->key = &config.key;
	AES_set_decrypt_key(config.key.xor_pkey, ORP_KEY_LEN * 8,
		&videoConfig->aes_key);
	videoConfig->codec = orpGetHeaderValue(HEADER_VIDEO_CODEC, headerList);
	videoConfig->session_id = session_id;

	videoConfig->stream = new struct orpStreamData_t;
	videoConfig->stream->data = NULL;
	videoConfig->stream->len = videoConfig->stream->pos = 0;
	videoConfig->stream->frames = 0;
	videoConfig->stream->packetLock = SDL_CreateMutex();
	videoConfig->stream->packetCond = SDL_CreateCond();

	struct orpConfigStream_t *audioConfig = new struct orpConfigStream_t;
	os.str("");
	os << "http://";
	os << config.ps3_addr << ":" << config.ps3_port;
	os << ORP_GET_AUDIO;
	audioConfig->url = os.str();

	audioConfig->key = &config.key;
	AES_set_decrypt_key(config.key.xor_pkey, ORP_KEY_LEN * 8,
		&audioConfig->aes_key);
	audioConfig->codec = orpGetHeaderValue(HEADER_AUDIO_CODEC, headerList);
	audioConfig->session_id = session_id;

	audioConfig->stream = new struct orpStreamData_t;
	audioConfig->stream->data = NULL;
	audioConfig->stream->len = audioConfig->stream->pos = 0;
	audioConfig->stream->frames = 0;
	audioConfig->stream->packetLock = SDL_CreateMutex();
	audioConfig->stream->packetCond = SDL_CreateCond();

	if (!(thread_video_connection = SDL_CreateThread(orpThreadVideoConnection,
		videoConfig))) return -1;

	if (!(thread_audio_connection = SDL_CreateThread(orpThreadAudioConnection,
		audioConfig))) return -1;

	struct orpThreadDecode_t *videoDecode = new struct orpThreadDecode_t;
	videoDecode->terminate = false;
	videoDecode->view = &view;
	videoDecode->codec = videoCodec;
	videoDecode->stream = videoConfig->stream;

	if (!(thread_video_decode = SDL_CreateThread(orpThreadVideoDecode,
		videoDecode))) return -1;

	struct orpThreadDecode_t *audioDecode = new struct orpThreadDecode_t;
	audioDecode->terminate = false;
	audioDecode->codec = audioCodec;
	audioDecode->stream = audioConfig->stream;

	if (!(thread_audio_decode = SDL_CreateThread(orpThreadAudioDecode,
		audioDecode))) return -1;

	// Hang-out here until something happens...
	Sint32 result = SessionControl();

	// Shutdown...
	videoDecode->terminate = audioDecode->terminate = true;

	Sint32 thread_result;
	SDL_CondBroadcast(videoDecode->stream->packetCond);
	SDL_CondBroadcast(audioDecode->stream->packetCond);

	SDL_WaitThread(thread_video_connection, &thread_result);
	SDL_WaitThread(thread_audio_connection, &thread_result);

	SDL_WaitThread(thread_video_decode, &thread_result);
	SDL_WaitThread(thread_audio_decode, &thread_result);

	// TODO: Free, clean-up everything, being really lazy here...
	if (result == EVENT_RESTORE)
		cerr << "Session restore." << endl;
	else
		cerr << "Session terminated." << endl;

	thread_video_connection = thread_video_decode = NULL;
	thread_audio_connection = thread_audio_decode = NULL;

	return result;
}

// vi: ts=4
