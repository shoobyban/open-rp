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
#include "launch.h"

static void orpOutput(const char *format, va_list ap)
{
	static SDL_mutex *lock = NULL;
	if (lock == NULL) lock = SDL_CreateMutex();
	SDL_LockMutex(lock);
	static FILE *log;
	if (!log) {
		log = fopen("orp.log", "w");
		if (log) fprintf(log, "Open Remote Play v%s\n", ORP_VERSION);
	}
	if (log) {
		fprintf(log, "%10u: ", SDL_GetTicks());
		vfprintf(log, format, ap);
		fflush(log);
	}
#ifndef _WIN32
	vfprintf(stderr, format, ap);
#endif
	SDL_UnlockMutex(lock);
}

static void orpPrintf(const char *format, ...)
{
	static SDL_mutex *lock = NULL;
	if (lock == NULL) lock = SDL_CreateMutex();
	SDL_LockMutex(lock);
	va_list ap;
	va_start(ap, format);
	orpOutput(format, ap);
	va_end(ap);
	SDL_UnlockMutex(lock);
}

static Sint32 orpCurlDebug(CURL *curl, curl_infotype type, char *text, size_t text_len, void *data)
{
	static SDL_mutex *lock = NULL;
	if (lock == NULL) lock = SDL_CreateMutex();
	SDL_LockMutex(lock);
	ostringstream os;
	char *url = NULL;
	curl_easy_getinfo(curl, CURLINFO_EFFECTIVE_URL, &url);
	if (url != NULL) os << url << ": ";
	else os << "<Unknown>: ";
	switch (type) {
	case CURLINFO_TEXT:
		break;
	case CURLINFO_HEADER_IN:
		os << ">> ";
		break;
	case CURLINFO_HEADER_OUT:
		os << "<< Sending headers:" << endl;
		break;
	case CURLINFO_DATA_IN:
		SDL_UnlockMutex(lock);
		return 0;
	case CURLINFO_DATA_OUT:
		SDL_UnlockMutex(lock);
		return 0;
	}
	char *buffer = new char[text_len + 1];
	memset(buffer, 0, text_len + 1);
	memcpy(buffer, text, text_len);
	if (buffer[text_len -1] == '\n')
		os << buffer;
	else
		os << buffer << endl;
	delete [] buffer;
	orpPrintf(os.str().c_str());
	SDL_UnlockMutex(lock);
	return 0;
}

static void orpAVDebug(void *ptr, Sint32 level, const char *format, va_list ap)
{
	orpOutput(format, ap);
}

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

#ifdef ORP_CLOCK_DEBUG
static Uint32 orpClockTimer(Uint32 interval, void *param)
{
	struct orpClock_t *clock = (struct orpClock_t *)param;
	SDL_LockMutex(clock->lock);
	Sint32 delta = clock->video - clock->audio;
	Uint32 decode = clock->decode;
	Sint32 s = (Sint32)((double)delta / (double)90000 * (double)1000);
	double a = (double)clock->audio / (double)90000;
	double v = (double)clock->video / (double)90000;
	SDL_UnlockMutex(clock->lock);
	orpPrintf("clock delta: %d, decode: %ums, drift: %dms, a/v: %.04fs %.04fs\n",
		delta, decode, s, a, v);
	return interval;
}
#endif

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
//	orpPrintf("%02x %02x %02x %02x %02x %02x\n",
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
		orpPrintf("invalid %s packet header\n", _config->name.c_str());
		delete [] buffer;
		return 0;
	}

	// Asked to restore audio/video stream?
	if (bp[1] == 0xfd) {
		orpPrintf("%s restore (reset)\n", _config->name.c_str());
		orpPostEvent(EVENT_RESTORE);
		delete [] buffer;
		return (size * nmemb);
	}

	// Audio/video magic of 0x8081 seems to mean PS3 shutdown
	if (bp[1] == 0x81) {
		orpPrintf("%s system power-off\n", _config->name.c_str());
		orpPostEvent(EVENT_SHUTDOWN);
		delete [] buffer;
		return (size * nmemb);
	}

	// Audio or video header?
	if (bp[1] != 0x80 && bp[1] != 0xff && bp[1] != 0xfb && bp[1] != 0xfc) {
		orpPrintf("invalid %s magic: 0x%02x%02x\n", _config->name.c_str(),
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
	//packet->len = chunk_len - sizeof(struct orpStreamPacketHeader_t);
	memset(&packet->pkt, 0, sizeof(AVPacket));
	packet->pkt.size = chunk_len - sizeof(struct orpStreamPacketHeader_t);

	// Header size must match packet length
	if (packet->pkt.size != SDL_Swap16(packet->header.len)) {
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
		packet->pkt.size = SDL_Swap16(packet->header.len);
	}

	// Allocate payload
	packet->pkt.data = new Uint8[packet->pkt.size +
		FF_INPUT_BUFFER_PADDING_SIZE];
	if (!packet->pkt.data) {
		delete [] packet;
		delete [] buffer;
		return 0;
	}
	memset(packet->pkt.data + packet->pkt.size, 0,
		FF_INPUT_BUFFER_PADDING_SIZE);

	// Copy payload
	bp += sizeof(struct orpStreamPacketHeader_t);
	memcpy(packet->pkt.data, bp, packet->pkt.size);
	delete [] buffer;

	// Decrypt video key-frames and encrypted audio frames
	if (((packet->header.magic[1] == 0x80 ||
		packet->header.magic[1] == 0xfc) && packet->header.unk8)
		|| packet->header.unk6 == 0x0401 || packet->header.unk6 == 0x0001) {
		memcpy(_config->key->iv1,
			_config->key->xor_nonce, ORP_KEY_LEN);
		AES_cbc_encrypt(packet->pkt.data, packet->pkt.data,
			packet->pkt.size - (packet->pkt.size % ORP_KEY_LEN),
			&_config->aes_key, _config->key->iv1, AES_DECRYPT);
	}

	// Push stream packet
	bool buffer_fill = false;
	SDL_LockMutex(streamData->lock);
#if 0
	if (!strcmp(_config->name.c_str(), "video")) {
		SDL_LockMutex(streamData->buffer->lock);
		if (!streamData->pkt.size()) {
			streamData->buffer->fill = true;
			// TODO: Post event...
			cerr << "Filling buffer...\n";
		} else if (streamData->buffer->fill &&
			streamData->pkt.size() < 120) {
			cerr << "Buffering...\n";
		} else {
			streamData->buffer->fill = false;
			//cerr << "Buffer full.\n";
		}
		buffer_fill = streamData->buffer->fill;
		SDL_UnlockMutex(streamData->buffer->lock);
	} else {
		SDL_LockMutex(streamData->buffer->lock);
		buffer_fill = streamData->buffer->fill;
		SDL_UnlockMutex(streamData->buffer->lock);
	}
#endif
	streamData->pkt.push(packet);
	if (!buffer_fill) SDL_CondSignal(streamData->cond);
	SDL_UnlockMutex(streamData->lock);

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

	// TODO: Frame rate must come from session headers...
	Uint32 fps = (Uint32)(1000 / 30);

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
	struct SwsContext *sws_fullscreen;
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
	sws_fullscreen = sws_getContext(ORP_FRAME_WIDTH, ORP_FRAME_HEIGHT,
		context->pix_fmt, _config->view->fs.w, _config->view->fs.h,
		PIX_FMT_YUV420P, SWS_BICUBIC, NULL, NULL, NULL);

	Sint32 bytes_decoded, frame_done = 0;
#ifdef ORP_DUMP_VIDEO_HEADER
	FILE *h_header = fopen("video-header.dat", "w+");
#endif
#ifdef ORP_DUMP_VIDEO_STREAM
	FILE *h_stream = fopen("video-stream.dat", "w+");
#endif
	while (!_config->terminate) {
		Uint32 ticks = SDL_GetTicks();
		SDL_LockMutex(_config->stream->lock);
		Uint32 packets = _config->stream->pkt.size();
		if (packets == 0)
			SDL_CondWait(_config->stream->cond, _config->stream->lock);
		if (_config->terminate) {
			SDL_UnlockMutex(_config->stream->lock);
			break;
		}

		if (packets) {
			packet = _config->stream->pkt.front();
			_config->stream->pkt.pop();
		} else packet = NULL;
		SDL_UnlockMutex(_config->stream->lock);

		if (!packet) continue;
#ifdef ORP_DUMP_VIDEO_HEADER
		fwrite(&packet->header,
			1, sizeof(struct orpStreamPacketHeader_t), h_header);
#endif
#ifdef ORP_DUMP_VIDEO_STREAM
		fwrite(packet->data, 1, packet->len, h_stream);
#endif
		SDL_LockMutex(_config->clock->lock);
		_config->clock->video = SDL_Swap32(packet->header.clock);
		SDL_UnlockMutex(_config->clock->lock);

		bytes_decoded = avcodec_decode_video2(context,
			frame, &frame_done, &packet->pkt);

		if (bytes_decoded != -1 && frame_done) {
			SDL_LockMutex(_config->view->lock);
			SDL_LockYUVOverlay(_config->view->overlay);

			AVPicture p;
			p.data[0] = _config->view->overlay->pixels[0];
			p.data[1] = _config->view->overlay->pixels[2];
			p.data[2] = _config->view->overlay->pixels[1];

			p.linesize[0] = _config->view->overlay->pitches[0];
			p.linesize[1] = _config->view->overlay->pitches[2];
			p.linesize[2] = _config->view->overlay->pitches[1];

			if (_config->view->size == VIEW_NORMAL) {
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
			} else if (_config->view->size == VIEW_FULLSCREEN) {
				sws_scale(sws_fullscreen,
					frame->data, frame->linesize, 0, context->height,
					p.data, p.linesize);
			}

			SDL_UnlockYUVOverlay(_config->view->overlay);

			SDL_Rect rect;
			rect.x = 0;
			if (_config->view->size == VIEW_FULLSCREEN) {
				rect.y = _config->view->fs.y;
				rect.w = _config->view->fs.w;
				rect.h = _config->view->fs.h;
			} else {
				rect.y = 0;
				rect.w = _config->view->scale.w;
				rect.h = _config->view->scale.h;
			}

			SDL_DisplayYUVOverlay(_config->view->overlay, &rect);
			SDL_UnlockMutex(_config->view->lock);

			SDL_LockMutex(_config->clock->lock);
			Sint32 delta = (Sint32)(_config->clock->video - _config->clock->audio);
			Uint32 decode = _config->clock->decode = SDL_GetTicks() - ticks;
			SDL_UnlockMutex(_config->clock->lock);

			Sint32 drift = 0;
			if (delta > 0)
				drift = (Sint32)((double)delta / (double)90000 * (double)1000);
			if (drift > 0) {
				Uint32 delay = decode < drift ? drift - decode : drift;
				if (delay > 1000)
					orpPrintf("Delay too large: %u\n", delay);
				else SDL_Delay(delay);
			}
		}

		delete [] packet->pkt.data;
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
	sws_freeContext(sws_fullscreen);
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
	curl_easy_setopt(curl, CURLOPT_DEBUGFUNCTION, orpCurlDebug);
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

	SDL_LockMutex(_config->lock);
	if (_config->frame.size()) {
		struct orpAudioFrame_t *frame;
		frame = _config->frame.front();
		_config->frame.pop();
		memcpy(stream, frame->data, len);
		if (_config->clock) {
			SDL_LockMutex(_config->clock->lock);
			_config->clock->audio = frame->clock;
			SDL_UnlockMutex(_config->clock->lock);
		}
		delete [] frame->data;
		delete frame;
	} else memset(stream, 0, len);
	SDL_UnlockMutex(_config->lock);
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
		context->block_align = 192 * channels;
		context->bit_rate = bit_rate;
	}

	if (avcodec_open(context, codec) < 0) {
		if (codec->id == CODEC_ID_ATRAC3) av_free(context->extradata);
		av_free(context);
		SDL_UnlockMutex(orpAVMutex);
		orpPrintf("codec context initialization failed.\n");
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
	feed.lock = SDL_CreateMutex();
	feed.clock = _config->clock;

	SDL_AudioSpec audioSpec, requestedSpec;
	requestedSpec.freq = sample_rate;
	requestedSpec.format = AUDIO_S16SYS;
	requestedSpec.channels = channels;
	requestedSpec.silence = 0;
	requestedSpec.samples = 1024;
	requestedSpec.callback = orpAudioFeed;
	requestedSpec.userdata = (void *)&feed;

	if(SDL_OpenAudio(&requestedSpec, &audioSpec) == -1) {
		orpPrintf("SDL_OpenAudio: %s\n", SDL_GetError());
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
		SDL_LockMutex(_config->stream->lock);
		Uint32 packets = _config->stream->pkt.size();
		if (packets == 0)
			SDL_CondWait(_config->stream->cond, _config->stream->lock);
		if (_config->terminate) {
			SDL_UnlockMutex(_config->stream->lock);
			break;
		}

		if (_config->stream->pkt.size()) {
			packet = _config->stream->pkt.front();
			_config->stream->pkt.pop();
		} else packet = NULL;
		SDL_UnlockMutex(_config->stream->lock);

		if (!packet) continue;
#ifdef ORP_DUMP_AUDIO_HEADER
		fwrite(&packet->header,
			1, sizeof(struct orpStreamPacketHeader_t), h_header);
#endif
#ifdef ORP_DUMP_AUDIO_STREAM
		fwrite(packet->data, 1, packet->len, h_stream);
#endif
		frame_size = sizeof(buffer);
		bytes_decoded = avcodec_decode_audio3(context,
			(Sint16 *)buffer, &frame_size, &packet->pkt);
		if (bytes_decoded != -1 && frame_size) {
			struct orpAudioFrame_t *audioFrame =
				new struct orpAudioFrame_t;
		
			audioFrame->len = (Uint32)frame_size;
			audioFrame->data = new Uint8[audioFrame->len];
			audioFrame->clock = SDL_Swap32(packet->header.clock);
			memcpy(audioFrame->data, buffer, audioFrame->len);

			SDL_LockMutex(feed.lock);
			feed.frame.push(audioFrame);
			if (feed.frame.size() > 10) {
				orpPrintf("feed queue: %u\n", feed.frame.size());
			}
			SDL_UnlockMutex(feed.lock);
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

		delete [] packet->pkt.data;
		delete packet;
	}
	
	SDL_PauseAudio(1);
	SDL_CloseAudio();
	SDL_DestroyMutex(feed.lock);
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

static Sint32 orpPlaySound(Uint8 *data, Uint32 len)
{
	Sint32 channels = 2, sample_rate = 48000;

	struct orpConfigAudioFeed_t feed;
	feed.clock = NULL;
	feed.lock = SDL_CreateMutex();

	SDL_AudioSpec audioSpec, requestedSpec;
	requestedSpec.freq = sample_rate;
	requestedSpec.format = AUDIO_S16SYS;
	requestedSpec.channels = channels;
	requestedSpec.silence = 0;
	requestedSpec.samples = 1024;
	requestedSpec.callback = orpAudioFeed;
	requestedSpec.userdata = (void *)&feed;

	if(SDL_OpenAudio(&requestedSpec, &audioSpec) == -1) {
		orpPrintf("SDL_OpenAudio: %s\n", SDL_GetError());
		SDL_DestroyMutex(feed.lock);
		return -1;
	}

	SDL_PauseAudio(0);

	Uint32 i, frame_size = 4096;
	for (i = 0; len != 0; i += frame_size) {
		struct orpAudioFrame_t *audioFrame =
			new struct orpAudioFrame_t;

		audioFrame->len = frame_size;
		audioFrame->data = new Uint8[frame_size];
		memset(audioFrame->data, 0, frame_size);
		memcpy(audioFrame->data, data + i,
			(len > frame_size) ? frame_size : len);
		len -= (len > frame_size ? frame_size : len);

		SDL_LockMutex(feed.lock);
		feed.frame.push(audioFrame);
		SDL_UnlockMutex(feed.lock);
	}

	for ( ;; ) {
		SDL_LockMutex(feed.lock);
		if (feed.frame.size() == 0) break;
		SDL_UnlockMutex(feed.lock);
		SDL_Delay(100);
	}

	SDL_PauseAudio(1);
	SDL_CloseAudio();
	SDL_DestroyMutex(feed.lock);

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
	curl_easy_setopt(curl, CURLOPT_DEBUGFUNCTION, orpCurlDebug);
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
	: ps3_nickname(NULL), js(NULL),
#ifdef ORP_CLOCK_DEBUG
	timer(0),
#endif
	thread_video_connection(NULL), thread_video_decode(NULL),
	thread_audio_connection(NULL), thread_audio_decode(NULL)
{
	view.view = NULL;
	view.overlay = NULL;
	view.size = VIEW_NORMAL;
	view.scale.x = view.scale.y = 0;
	view.scale.w = ORP_FRAME_WIDTH;
	view.scale.h = ORP_FRAME_HEIGHT;
	view.lock = SDL_CreateMutex();
	orpAVMutex = SDL_CreateMutex();

	// Copy config
	memcpy(&this->config, config, sizeof(struct orpConfig_t));

	// Init libcurl
	curl_global_init(CURL_GLOBAL_WIN32);

	// Init libavcodec, load all codecs
	avcodec_init();
	avcodec_register_all();
	av_log_set_callback(orpAVDebug);

	// Initialize the audio/video decoders we need
	struct orpCodec_t *oc;
	AVCodec *codec;

	codec = avcodec_find_decoder(CODEC_ID_H264);
	if (!codec) {
		orpPrintf("Required codec not found: %s\n", "CODEC_ID_H264");
		throw -1;
	}
	oc = new struct orpCodec_t;
	oc->name = "AVC";
	oc->codec = codec;
	this->codec.push_back(oc);

	codec = avcodec_find_decoder(CODEC_ID_MPEG4);
	if (!codec) {
		orpPrintf("Required codec not found: %s\n", "CODEC_ID_MPEG4");
		throw -1;
	}
	oc = new struct orpCodec_t;
	oc->name = "M4V";
	oc->codec = codec;
	this->codec.push_back(oc);

	codec = avcodec_find_decoder_by_name("libfaad");
	if (!codec) {
		orpPrintf("Required codec not found: %s\n", "CODEC_ID_AAC (libfaad)");
		throw -1;
	}
	oc = new struct orpCodec_t;
	oc->name = "M4A";
	oc->codec = codec;
	this->codec.push_back(oc);

	codec = avcodec_find_decoder(CODEC_ID_ATRAC3);
	if (!codec) {
		orpPrintf("Required codec not found: %s\n", "CODEC_ID_ATRAC3");
		throw -1;
	}
	oc = new struct orpCodec_t;
	oc->name = "AT3";
	oc->codec = codec;
	this->codec.push_back(oc);

	clock.lock = SDL_CreateMutex();
	clock.audio = clock.video = clock.decode = 0;

	video_buffer.fill = true;
	video_buffer.count = 0;
	video_buffer.lock = SDL_CreateMutex();
}

OpenRemotePlay::~OpenRemotePlay()
{
	SessionDestroy();
	Sint32 i;
	for (i = 0; i < codec.size(); i++) delete codec[i];
	if (orpAVMutex) SDL_DestroyMutex(orpAVMutex);
	if (view.lock) SDL_DestroyMutex(view.lock);
}

bool OpenRemotePlay::SessionCreate(void)
{
	Sint32 i;

	// Initialize audio, video, and timer
	if (SDL_Init(SDL_INIT_AUDIO | SDL_INIT_VIDEO | SDL_INIT_TIMER)) {
		orpPrintf("SDL_Init: %s\n", SDL_GetError());
		return false;
	}

	// Set window icon, must be done before setting video mode
	SDL_RWops *rw;
#ifdef _MACOSX_
	if ((rw = SDL_RWFromConstMem(icon_osx_bmp, icon_osx_bmp_len))) {
#else
	if ((rw = SDL_RWFromConstMem(icon_bmp, icon_bmp_len))) {
#endif
		SDL_Surface *icon = IMG_Load_RW(rw, 0);
		if (icon) {
			SDL_WM_SetIcon(icon, NULL);
			SDL_FreeSurface(icon);
		}
		SDL_FreeRW(rw);
	}

	// Determine desktop video resolution
	const SDL_VideoInfo *info = SDL_GetVideoInfo();
	if (info) {
		view.fs.x = 0;
		view.fs.w = info->current_w;
		view.fs.h = (Sint16)(info->current_w * ORP_FRAME_HEIGHT /
			ORP_FRAME_WIDTH);
		if (view.fs.h < info->current_h)
			view.fs.y = (info->current_h - view.fs.h) / 2;
		else view.fs.y = 0;
		view.desktop.w = info->current_w;
		view.desktop.h = info->current_h;
	} else {
		view.desktop.w = ORP_FRAME_WIDTH;
		view.desktop.h = ORP_FRAME_HEIGHT;
	}

	// Create window (set video mode)
	if (!view.view) { if (!CreateView()) return false; }

	// Initialize event thread
	SDL_InitSubSystem(SDL_INIT_EVENTTHREAD);

	// Set caption and display splash logo
	SetCaption("Open Remote Play");

	if ((rw = SDL_RWFromConstMem(splash_png, splash_png_len))) {
		SDL_Surface *splash = IMG_Load_RW(rw, 0);
		if (splash) {
			SDL_Rect rect;
			rect.x = rect.y = 0;
			rect.w = view.scale.w;
			rect.h = view.scale.h;
			SDL_BlitSurface(splash, NULL, view.view, &rect);

			SDL_Surface *surface;
			Uint32 rmask, gmask, bmask, amask;

#if SDL_BYTEORDER == SDL_BIG_ENDIAN
			rmask = 0xff000000;
			gmask = 0x00ff0000;
			bmask = 0x0000ff00;
			amask = 0x000000ff;
#else
			rmask = 0x000000ff;
			gmask = 0x0000ff00;
			bmask = 0x00ff0000;
			amask = 0xff000000;
#endif
			surface = SDL_CreateRGBSurface(SDL_SWSURFACE,
				ORP_FRAME_WIDTH, ORP_FRAME_HEIGHT, 32,
				rmask, gmask, bmask, amask);

			Sint32 i;
			for (i = 256; i > 0; i -= 16) {
				SDL_BlitSurface(splash, NULL, view.view, &rect);
				SDL_FillRect(surface, &rect, SDL_MapRGBA(surface->format, 0, 0, 0, (Uint8)i - 1));
				SDL_BlitSurface(surface, NULL, view.view, &rect);
				SDL_UpdateRect(view.view, rect.x, rect.y, rect.w, rect.h);
			}
			SDL_FreeSurface(splash);
			SDL_FreeSurface(surface);
		}
		SDL_FreeRW(rw);
	}

	// Initialize joystick(s)
	if (SDL_InitSubSystem(SDL_INIT_JOYSTICK) == 0 &&
		SDL_NumJoysticks() > 0) {
		if ((!strncasecmp("sony", SDL_JoystickName(0), 4) ||
			!strncasecmp("playstation", SDL_JoystickName(0), 11) ||
			!strncasecmp("www.", SDL_JoystickName(0), 4)) &&
			(js = SDL_JoystickOpen(0))) {
			orpPrintf("Joystick opened: %s\n", SDL_JoystickName(0));
		}
	}

	// Create and bind UDP socket
	IPaddress addr;
	if (SDLNet_ResolveHost(&addr,
		config.ps3_addr, config.ps3_port) != 0) {
		orpPrintf("Error resolving address: %s:%d: %s\n",
			config.ps3_addr, config.ps3_port, SDLNet_GetError());
		return false;
	}
	UDPsocket skt = SDLNet_UDP_Open(0);
	Sint32 channel;
	if ((channel = SDLNet_UDP_Bind(skt, -1, &addr)) == -1) {
		orpPrintf("Error binding socket: %s\n", SDLNet_GetError());
		return false;
	}

	// Send WoL packet
	UDPpacket *pkt_wol = SDLNet_AllocPacket(ORP_WOLPKT_LEN);
	pkt_wol->len = ORP_WOLPKT_LEN;
	memset(pkt_wol->data, 0, ORP_WOLPKT_LEN);
	for (i = 0; i < ORP_MAC_LEN; i++) pkt_wol->data[i] = 0xff;
	for (i = 0; i < 16; i++) {
		memcpy(pkt_wol->data + ORP_MAC_LEN + (i * ORP_MAC_LEN),
			config.ps3_mac, ORP_MAC_LEN);
	}

	for (i = 0; i < 2; i++) {
		if (SDLNet_UDP_Send(skt, channel, pkt_wol) == 0) {
			orpPrintf("Error sending WoL packet: %s\n", SDLNet_GetError());
		}
		SDL_Delay(200);
	}

	SDLNet_FreePacket(pkt_wol);

	// Direct TCP connection?
	if (!config.ps3_search) {
		SDLNet_UDP_Close(skt);
		return SessionPerform();
	}

	// UDP search for and/or wait on the PS3...
	struct PktAnnounceSrch_t srch;
	CreatePktAnnounceSrch(srch);
	UDPpacket *pkt_srch = SDLNet_AllocPacket(sizeof(struct PktAnnounceSrch_t));
	pkt_srch->len = sizeof(struct PktAnnounceSrch_t);
	memcpy(pkt_srch->data, &srch, sizeof(struct PktAnnounceSrch_t));
	UDPpacket *pkt_resp = SDLNet_AllocPacket(sizeof(struct PktAnnounceResp_t));

	ostringstream os;
	Sint32 reply = 0, first = 1;
	for (i = 0; i < ORP_SRCH_TIMEOUT; i++) {
		os.str("");
		os << "Searching... ";
		os << (i + 1) << "/" << ORP_SRCH_TIMEOUT;
		SetCaption(os.str().c_str());

		if (SDLNet_UDP_Send(skt, channel, pkt_srch) == 0) {
			orpPrintf("Error sending packet: %s\n", SDLNet_GetError());
			break;
		}

		Sint32 result;
		if ((result = SDLNet_UDP_Recv(skt, pkt_resp)) == -1) {
			orpPrintf("Error receiving packet: %s\n", SDLNet_GetError());
			break;
		}

		if (result == 0) {
			orpPrintf("No reply, retry %d of %d...\n",
				i + 1, ORP_SRCH_TIMEOUT);
			reply = 0;
			SDL_Delay(1000);
			continue;
		}

		reply++;
		orpPrintf("Reply of %d bytes.\n", pkt_resp->len);

		if (!first && reply < ORP_SRCH_REPLIES) {
			SDL_Delay(1000);
			continue;
		}

		// Fire it up!
		result = SessionPerform();
#ifdef ORP_CLOCK_DEBUG
		SDL_RemoveTimer(timer);
#endif
		if (result != EVENT_RESTORE) break;
		i = reply = first = 0;
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
	if (view.overlay) SDL_FreeYUVOverlay(view.overlay);
	view.overlay = NULL;
	// TODO: This *always* crashes, not sure why...
	//if (js && SDL_JoystickOpened(0)) SDL_JoystickClose(js);
	SDL_Quit();
}

bool OpenRemotePlay::CreateView(void)
{
	SDL_Rect *r = NULL, fs;
	Uint32 flags = 0;
	switch (view.size)
	{
	case VIEW_FULLSCREEN:
		flags |= SDL_FULLSCREEN;
		fs.x = fs.y = 0;
		fs.w = view.desktop.w;
		fs.h = view.desktop.h;
		r = &fs;
		break;
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
	if (r == NULL) r = &view.scale;
	if (view.overlay) SDL_FreeYUVOverlay(view.overlay);
	if (!(view.view = SDL_SetVideoMode(r->w, r->h, 0, flags))) {
		orpPrintf("SDL_SetVideoMode: %s\n", SDL_GetError());
		return false;
	}
	if (view.size == VIEW_FULLSCREEN) r->h = view.fs.h;
	if (!(view.overlay = SDL_CreateYUVOverlay(
		r->w, r->h, SDL_YV12_OVERLAY, view.view))) {
		orpPrintf("SDL_CreateYUVOverlay: %s\n", SDL_GetError());
		return false;
	}
	SDL_ShowCursor(!(view.size == VIEW_FULLSCREEN));
	SDL_WM_GrabInput((view.size == VIEW_FULLSCREEN)
		? SDL_GRAB_ON : SDL_GRAB_OFF);
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

	memcpy(config.key.iv1, config.key.xor_nonce, ORP_KEY_LEN);

	memset(config.key.akey, 0, ORP_KEY_LEN);
	memcpy(config.key.akey, config.psp_mac, ORP_MAC_LEN);

	AES_KEY aes_key_encrypt;
	AES_set_encrypt_key(config.key.xor_pkey,
		ORP_KEY_LEN * 8, &aes_key_encrypt);
	AES_cbc_encrypt(config.key.akey, config.key.akey, ORP_KEY_LEN,
		&aes_key_encrypt, config.key.iv1, AES_ENCRYPT);

	return true;
}

void OpenRemotePlay::SetCaption(const char *caption)
{
	SDL_WM_SetCaption(caption, NULL);
	SDL_Event event;
	while (SDL_PollEvent(&event) > 0);
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
			orpPrintf("%02x", state[i]);
			if (toggle) {
				orpPrintf(" ");
				toggle = false;
			} else toggle = true;
			i++;
		}
		orpPrintf("\n");
	}
}

static Sint32 orpSendPadState(TCPsocket skt, Uint8 *pad, Uint32 id, Uint32 &count, Uint32 timestamp, vector<string> &headers)
{
	if (id != 0) {
		Uint32 be_id = SDL_Swap32(id);
		Uint32 be_timestamp = SDL_Swap32(timestamp);
		memcpy(pad + ORP_PAD_EVENTID, &be_id, 4);
		memcpy(pad + ORP_PAD_TIMESTAMP, &be_timestamp, 4);
	}
	//orpDumpPadState(statePad);

	SDLNet_TCP_Send(skt, pad, ORP_PADSTATE_LEN);

	if (count == ORP_PADSTATE_MAX) {
		// TODO: Yes this is lazy, and not right...
		Uint8 reply[80];
		if (SDLNet_TCP_Recv(skt, reply, 80) != 80) {
			orpPrintf("Error receiving reply: %s\n", SDLNet_GetError());
			return -1;
		}
		count = 0;
		Uint32 i;
		for (i = 0; i < headers.size(); i++) {
			SDLNet_TCP_Send(skt,
				headers[i].c_str(), strlen(headers[i].c_str()));
		}
	}

	return 0;
}

static struct orpKeyboardMap_t orpKeyboardMap[ORP_KBMAP_LEN] = {
	// 1 2 3 4 5 6 7 8 9 0
	{ SDLK_1, KMOD_NONE, 0, 0 },
	{ SDLK_2, KMOD_NONE, 1, 0 },
	{ SDLK_3, KMOD_NONE, 2, 0 },
	{ SDLK_4, KMOD_NONE, 3, 0 },
	{ SDLK_5, KMOD_NONE, 4, 0 },
	{ SDLK_6, KMOD_NONE, 5, 0 },
	{ SDLK_7, KMOD_NONE, 6, 0 },
	{ SDLK_8, KMOD_NONE, 7, 0 },
	{ SDLK_9, KMOD_NONE, 8, 0 },
	{ SDLK_0, KMOD_NONE, 9, 0 },
	// q w e r t y u i o p
	{ SDLK_q, KMOD_NONE, 0, 1 },
	{ SDLK_w, KMOD_NONE, 1, 1 },
	{ SDLK_e, KMOD_NONE, 2, 1 },
	{ SDLK_r, KMOD_NONE, 3, 1 },
	{ SDLK_t, KMOD_NONE, 4, 1 },
	{ SDLK_y, KMOD_NONE, 5, 1 },
	{ SDLK_u, KMOD_NONE, 6, 1 },
	{ SDLK_i, KMOD_NONE, 7, 1 },
	{ SDLK_o, KMOD_NONE, 8, 1 },
	{ SDLK_p, KMOD_NONE, 9, 1 },
	// a s d f g h j k l '
	{ SDLK_a, KMOD_NONE, 0, 2 },
	{ SDLK_s, KMOD_NONE, 1, 2 },
	{ SDLK_d, KMOD_NONE, 2, 2 },
	{ SDLK_f, KMOD_NONE, 3, 2 },
	{ SDLK_g, KMOD_NONE, 4, 2 },
	{ SDLK_h, KMOD_NONE, 5, 2 },
	{ SDLK_j, KMOD_NONE, 6, 2 },
	{ SDLK_k, KMOD_NONE, 7, 2 },
	{ SDLK_l, KMOD_NONE, 8, 2 },
	{ SDLK_QUOTE, KMOD_NONE, 9, 2 },
	// z x c v b n m , . ?
	{ SDLK_z, KMOD_NONE, 0, 3 },
	{ SDLK_x, KMOD_NONE, 1, 3 },
	{ SDLK_c, KMOD_NONE, 2, 3 },
	{ SDLK_v, KMOD_NONE, 3, 3 },
	{ SDLK_b, KMOD_NONE, 4, 3 },
	{ SDLK_n, KMOD_NONE, 5, 3 },
	{ SDLK_m, KMOD_NONE, 6, 3 },
	{ SDLK_COMMA, KMOD_NONE, 7, 3 },
	{ SDLK_PERIOD, KMOD_NONE, 8, 3 },
	{ SDLK_SLASH, KMOD_SHIFT , 9, 3 },
	// symbols, shift, extra-char, etc...
	{ SDLK_UNKNOWN, KMOD_NONE, 0, 4 },
	{ SDLK_UNKNOWN, KMOD_NONE, 1, 4 },
	{ SDLK_UNKNOWN, KMOD_NONE, 2, 4 },
	{ SDLK_UNKNOWN, KMOD_NONE, 3, 4 },
	{ SDLK_UNKNOWN, KMOD_NONE, 4, 4 },
	{ SDLK_UNKNOWN, KMOD_NONE, 4, 4 },
	{ SDLK_UNKNOWN, KMOD_NONE, 6, 4 },
	{ SDLK_UNKNOWN, KMOD_NONE, 6, 4 },
	{ SDLK_RETURN, KMOD_NONE, 9, 4 },
	{ SDLK_UNKNOWN, KMOD_NONE, 9, 4 },
	// arrow buttons, input mode, enter, etc...
/*	{ SDLK_UP, KMOD_NONE, 0, 5 },
	{ SDLK_DOWN, KMOD_NONE, 1, 5 }, */
	{ SDLK_UNKNOWN, KMOD_NONE, 0, 5 },
	{ SDLK_UNKNOWN, KMOD_NONE, 1, 5 },
	{ SDLK_UNKNOWN, KMOD_NONE, 2, 5 },
	{ SDLK_UNKNOWN, KMOD_NONE, 3, 5 },
	{ SDLK_UNKNOWN, KMOD_NONE, 4, 5 },
	{ SDLK_UNKNOWN, KMOD_NONE, 5, 5 },
	{ SDLK_UNKNOWN, KMOD_NONE, 6, 5 },
	{ SDLK_UNKNOWN, KMOD_NONE, 6, 5 },
	{ SDLK_UNKNOWN, KMOD_NONE, 8, 5 },
	{ SDLK_UNKNOWN, KMOD_NONE, 8, 5 },
};

Sint32 OpenRemotePlay::SessionControl(void)
{
	CURL *curl = curl_easy_init();

	ostringstream os;
	os << "http://";
	os << config.ps3_addr << ":" << config.ps3_port;
	os << ORP_GET_CTRL;

	curl_easy_setopt(curl, CURLOPT_URL, os.str().c_str());
	curl_easy_setopt(curl, CURLOPT_VERBOSE, 1);
	curl_easy_setopt(curl, CURLOPT_DEBUGFUNCTION, orpCurlDebug);
	curl_easy_setopt(curl, CURLOPT_USERAGENT, ORP_USER_AGENT);

	curl_easy_setopt(curl, CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_1_1);
	curl_easy_setopt(curl, CURLOPT_HTTP_CONTENT_DECODING, 0);
	curl_easy_setopt(curl, CURLOPT_HTTP_TRANSFER_DECODING, 0);

	IPaddress ip;
	if (SDLNet_ResolveHost(&ip, config.ps3_addr, config.ps3_port) != 0) {
		orpPrintf("Error resolving address: %s:%d: %s\n",
			config.ps3_addr, config.ps3_port, SDLNet_GetError());
		return -1;
	}

	TCPsocket skt;
	if ((skt = SDLNet_TCP_Open(&ip)) == NULL) {
		orpPrintf("Input connection failure: %s\n", SDLNet_GetError());
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
	mode.mode = CTRL_CHANGE_BITRATE;
	mode.param1 = "1024000";
	mode.param2 = "1024000";
	ControlPerform(curl, &mode);

	bool kbmap_mode = false;
	Sint8 kbmap_cx = ORP_KBMAP_SX;
	Sint8 kbmap_cy = ORP_KBMAP_SY;
	vector<Uint32> kbmap_queue;

	Uint32 count = 0;
	Uint32 id = 0, be_id;
	Uint32 timestamp, be_timestamp;
	Uint32 ticks = SDL_GetTicks();
	Sint16 jsr_xaxis = 0x80, jsr_yaxis = 0x80;
	Sint16 jsl_xaxis = 0x80, jsl_yaxis = 0x80;
	Uint8 *keystate = SDL_GetKeyState(NULL);;
	SDL_Event event;

	// Flush any queued events...
	while (SDL_PollEvent(&event) > 0) orpPrintf("Flushing event...\n");

	// Main event loop
	for ( ;; ) {
		Uint32 key = 0;
		if (SDL_WaitEvent(&event) == 0) {
			orpPrintf("SDL_WaitEvent: %s\n", SDL_GetError());
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
				orpPrintf("Error event\n");
				break;
			case EVENT_RESTORE:
				orpPrintf("Restore event\n");
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
				if (!kbmap_mode)
					key = ORP_PAD_KEYUP | ORP_PAD_PSP_DPUP;
				break;
			case SDLK_RIGHT:
				if (kbmap_mode)
					key = ORP_PAD_KEYUP | ORP_PAD_PSP_R1;
				else
					key = ORP_PAD_KEYUP | ORP_PAD_PSP_DPRIGHT;
				break;
			case SDLK_DOWN:
				if (!kbmap_mode)
					key = ORP_PAD_KEYUP | ORP_PAD_PSP_DPDOWN;
				break;
			case SDLK_LEFT:
				if (kbmap_mode)
					key = ORP_PAD_KEYUP | ORP_PAD_PSP_L1;
				else
					key = ORP_PAD_KEYUP | ORP_PAD_PSP_DPLEFT;
				break;
			case SDLK_RETURN:
				if (!(event.key.keysym.mod & (KMOD_LCTRL | KMOD_RCTRL))) {
//					if (kbmap_mode)
//						key = ORP_PAD_KEYUP | ORP_PAD_PSP_START;
//					else
					if (!kbmap_mode)
						key = ORP_PAD_KEYUP | ORP_PAD_PSP_X;
				}
				break;
			case SDLK_F1:
			case SDLK_SPACE:
				key = ORP_PAD_KEYUP | ORP_PAD_PSP_TRI;
				break;
			case SDLK_F2:
			case SDLK_BACKSPACE:
				key = ORP_PAD_KEYUP | ORP_PAD_PSP_SQUARE;
				break;
			case SDLK_F3:
				key = ORP_PAD_KEYUP | ORP_PAD_PSP_SELECT;
				break;
			case SDLK_F4:
				key = ORP_PAD_KEYUP | ORP_PAD_PSP_START;
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
			}
			break;

		case SDL_KEYDOWN:
			if (kbmap_mode) {
				bool found = false;
				Sint8 tx = 0, ty = 0;
				for (i = 0; i < ORP_KBMAP_LEN; i++) {
					if (orpKeyboardMap[i].sym != event.key.keysym.sym)
						continue;
					if (event.key.keysym.mod &&
						!(orpKeyboardMap[i].mod & event.key.keysym.mod))
						continue;
					tx = orpKeyboardMap[i].x;
					ty = orpKeyboardMap[i].y;
					found = true;
					break;
				}
				if (found) {
					Sint8 dx = (tx + 1) - (kbmap_cx + 1);
					Sint8 dy = (ty + 1) - (kbmap_cy + 1);
					if (dy > 0) {
						for (i = 0; i < abs(dx); i++) {
							if (dx > 0)
								kbmap_queue.push_back(ORP_PAD_PSP_DPRIGHT);
							else if (dx < 0)
								kbmap_queue.push_back(ORP_PAD_PSP_DPLEFT);
						}
						for (i = 0; i < abs(dy); i++) {
							if (dy > 0)
								kbmap_queue.push_back(ORP_PAD_PSP_DPDOWN);
							else if (dy < 0)
								kbmap_queue.push_back(ORP_PAD_PSP_DPUP);
						}
					} else {
						for (i = 0; i < abs(dy); i++) {
							if (dy > 0)
								kbmap_queue.push_back(ORP_PAD_PSP_DPDOWN);
							else if (dy < 0)
								kbmap_queue.push_back(ORP_PAD_PSP_DPUP);
						}
						for (i = 0; i < abs(dx); i++) {
							if (dx > 0)
								kbmap_queue.push_back(ORP_PAD_PSP_DPRIGHT);
							else if (dx < 0)
								kbmap_queue.push_back(ORP_PAD_PSP_DPLEFT);
						}
					}
					kbmap_queue.push_back(ORP_PAD_PSP_X);
					kbmap_cx = dx + kbmap_cx;
					kbmap_cy = dy + kbmap_cy;
					break;
				}
				if (event.key.keysym.sym == SDLK_SPACE) {
					kbmap_queue.push_back(ORP_PAD_PSP_TRI);
					break;
				} else if (event.key.keysym.sym == SDLK_BACKSPACE) {
					kbmap_queue.push_back(ORP_PAD_PSP_SQUARE);
					break;
				}
			}
			switch (event.key.keysym.sym) {
			case SDLK_q:
				if (event.key.keysym.mod & (KMOD_LCTRL | KMOD_RCTRL)) {
					mode.mode = CTRL_SESSION_TERM;
					ControlPerform(curl, &mode);
					return 0;
				}
				break;
			case SDLK_1:
				if (event.key.keysym.mod & (KMOD_LCTRL |  KMOD_RCTRL)) {
					mode.mode = CTRL_CHANGE_BITRATE;
					mode.param1 = "1024000";
					mode.param2 = "1024000";
					ControlPerform(curl, &mode);
				}
				break;
			case SDLK_2:
				if (event.key.keysym.mod & (KMOD_LCTRL |  KMOD_RCTRL)) {
					mode.mode = CTRL_CHANGE_BITRATE;
					mode.param1 = "768000";
					mode.param2 = "1024000";
					//mode.param2 = "768000";
					ControlPerform(curl, &mode);
				}
				break;
			case SDLK_3:
				if (event.key.keysym.mod & (KMOD_LCTRL |  KMOD_RCTRL)) {
					mode.mode = CTRL_CHANGE_BITRATE;
					mode.param1 = "384000";
					mode.param2 = "1024000";
					//mode.param2 = "384000";
					ControlPerform(curl, &mode);
				}
				break;
			case SDLK_d:
				if ((event.key.keysym.mod & (KMOD_LCTRL |  KMOD_RCTRL)) &&
					view.size != VIEW_FULLSCREEN) {
					SDL_LockMutex(view.lock);
					if (view.size == VIEW_NORMAL)
						view.size = VIEW_MEDIUM;
					else if (view.size == VIEW_MEDIUM)
						view.size = VIEW_LARGE;
					else if (view.size == VIEW_LARGE)
						view.size = VIEW_NORMAL;
					CreateView();
					SDL_UnlockMutex(view.lock);
					while (SDL_PollEvent(&event) > 0);
				}
				break;
			case SDLK_f:
			case SDLK_F11:
				if ((event.key.keysym.mod & (KMOD_LCTRL | KMOD_RCTRL)) ||
					(event.key.keysym.sym == SDLK_F11 &&
						!(event.key.keysym.mod &
						(KMOD_CTRL | KMOD_SHIFT | KMOD_ALT)))) {
					SDL_LockMutex(view.lock);
					if (view.size == VIEW_FULLSCREEN)
						view.size = view.prev;
					else {
						view.prev = view.size;
						view.size = VIEW_FULLSCREEN;
					}
					CreateView();
					SDL_UnlockMutex(view.lock);
					while (SDL_PollEvent(&event) > 0);
				}
				break;
			case SDLK_UP:
				if (!kbmap_mode)
					key = ORP_PAD_KEYDOWN | ORP_PAD_PSP_DPUP;
				break;
			case SDLK_RIGHT:
				if (kbmap_mode)
					key = ORP_PAD_KEYDOWN | ORP_PAD_PSP_R1;
				else
					key = ORP_PAD_KEYDOWN | ORP_PAD_PSP_DPRIGHT;
				break;
			case SDLK_DOWN:
				if (!kbmap_mode)
					key = ORP_PAD_KEYDOWN | ORP_PAD_PSP_DPDOWN;
				break;
			case SDLK_LEFT:
				if (kbmap_mode)
					key = ORP_PAD_KEYDOWN | ORP_PAD_PSP_L1;
				else
					key = ORP_PAD_KEYDOWN | ORP_PAD_PSP_DPLEFT;
				break;
			case SDLK_RETURN:
				if (!(event.key.keysym.mod & (KMOD_LCTRL | KMOD_RCTRL))) {
//					if (kbmap_mode)
//						key = ORP_PAD_KEYDOWN | ORP_PAD_PSP_START;
//					else
					if (!kbmap_mode)
						key = ORP_PAD_KEYDOWN | ORP_PAD_PSP_X;
				} else {
					if (kbmap_mode == true) {
						SetCaption(ps3_nickname);
						kbmap_mode = false;
					} else {
						ostringstream os;
						os << ps3_nickname << " - Virtual Keyboard Enabled";
						SetCaption(os.str().c_str());
						kbmap_mode = true;
						kbmap_cx = ORP_KBMAP_SX;
						kbmap_cy = ORP_KBMAP_SY;
					}
				}
				break;
			case SDLK_ESCAPE:
				key = ORP_PAD_KEYDOWN | ORP_PAD_PSP_CIRCLE;
				break;
			case SDLK_F1:
			case SDLK_SPACE:
				key = ORP_PAD_KEYDOWN | ORP_PAD_PSP_TRI;
				break;
			case SDLK_F2:
			case SDLK_BACKSPACE:
				key = ORP_PAD_KEYDOWN | ORP_PAD_PSP_SQUARE;
				break;
			case SDLK_F3:
				key = ORP_PAD_KEYDOWN | ORP_PAD_PSP_SELECT;
				break;
			case SDLK_F4:
				key = ORP_PAD_KEYDOWN | ORP_PAD_PSP_START;
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
			}
			break;

		case SDL_MOUSEMOTION:
			if (SDL_WM_GrabInput(SDL_GRAB_QUERY) != SDL_GRAB_ON) break;
			if (!keystate[SDLK_RALT] && !keystate[SDLK_LALT]) break;
			if (abs(event.motion.xrel) < 3 && abs(event.motion.yrel) < 3) {
				key = ORP_PAD_KEYUP;
				key += (ORP_PAD_PSP_DPRIGHT | ORP_PAD_PSP_DPUP |
					ORP_PAD_PSP_DPLEFT | ORP_PAD_PSP_DPDOWN);
			} else if (abs(event.motion.xrel) > abs(event.motion.yrel)) {
				key = ORP_PAD_KEYDOWN;
				if (event.motion.xrel > 0)
					key += ORP_PAD_PSP_DPRIGHT;
				else
					key += ORP_PAD_PSP_DPLEFT;
			} else if (abs(event.motion.xrel) < abs(event.motion.yrel)) {
				key = ORP_PAD_KEYDOWN;
				if (event.motion.yrel > 0)
					key += ORP_PAD_PSP_DPDOWN;
				else
					key += ORP_PAD_PSP_DPUP;
			} else {
				key = ORP_PAD_KEYUP;
				key += (ORP_PAD_PSP_DPRIGHT | ORP_PAD_PSP_DPUP |
					ORP_PAD_PSP_DPLEFT | ORP_PAD_PSP_DPDOWN);
			}
			break;

		case SDL_JOYBUTTONUP:
			switch (event.jbutton.button) {
			case ORP_DS3_SELECT:
				key = ORP_PAD_KEYUP | ORP_PAD_PSP_SELECT;
				break;
			case ORP_DS3_L3:
				key = ORP_PAD_KEYUP | ORP_PAD_PSP_L3;
				break;
			case ORP_DS3_R3:
				key = ORP_PAD_KEYUP | ORP_PAD_PSP_R3;
				break;
			case ORP_DS3_START:
				key = ORP_PAD_KEYUP | ORP_PAD_PSP_START;
				break;
			case ORP_DS3_DPUP:
				key = ORP_PAD_KEYUP | ORP_PAD_PSP_DPUP;
				break;
			case ORP_DS3_DPRIGHT:
				key = ORP_PAD_KEYUP | ORP_PAD_PSP_DPRIGHT;
				break;
			case ORP_DS3_DPDOWN:
				key = ORP_PAD_KEYUP | ORP_PAD_PSP_DPDOWN;
				break;
			case ORP_DS3_DPLEFT:
				key = ORP_PAD_KEYUP | ORP_PAD_PSP_DPLEFT;
				break;
			case ORP_DS3_L2:
				key = ORP_PAD_KEYUP | ORP_PAD_PSP_L2;
				break;
			case ORP_DS3_R2:
				key = ORP_PAD_KEYUP | ORP_PAD_PSP_R2;
				break;
			case ORP_DS3_L1:
				key = ORP_PAD_KEYUP | ORP_PAD_PSP_L1;
				break;
			case ORP_DS3_R1:
				key = ORP_PAD_KEYUP | ORP_PAD_PSP_R1;
				break;
			case ORP_DS3_TRI:
				key = ORP_PAD_KEYUP | ORP_PAD_PSP_TRI;
				break;
			case ORP_DS3_CIRCLE:
				key = ORP_PAD_KEYUP | ORP_PAD_PSP_CIRCLE;
				break;
			case ORP_DS3_X:
				key = ORP_PAD_KEYUP | ORP_PAD_PSP_X;
				break;
			case ORP_DS3_SQUARE:
				key = ORP_PAD_KEYUP | ORP_PAD_PSP_SQUARE;
				break;
			}
			break;

		case SDL_JOYBUTTONDOWN:
			switch (event.jbutton.button) {
			case ORP_DS3_SELECT:
				key = ORP_PAD_KEYDOWN | ORP_PAD_PSP_SELECT;
				break;
			case ORP_DS3_L3:
				key = ORP_PAD_KEYDOWN | ORP_PAD_PSP_L3;
				break;
			case ORP_DS3_R3:
				key = ORP_PAD_KEYDOWN | ORP_PAD_PSP_R3;
				break;
			case ORP_DS3_START:
				key = ORP_PAD_KEYDOWN | ORP_PAD_PSP_START;
				break;
			case ORP_DS3_DPUP:
				key = ORP_PAD_KEYDOWN | ORP_PAD_PSP_DPUP;
				break;
			case ORP_DS3_DPRIGHT:
				key = ORP_PAD_KEYDOWN | ORP_PAD_PSP_DPRIGHT;
				break;
			case ORP_DS3_DPDOWN:
				key = ORP_PAD_KEYDOWN | ORP_PAD_PSP_DPDOWN;
				break;
			case ORP_DS3_DPLEFT:
				key = ORP_PAD_KEYDOWN | ORP_PAD_PSP_DPLEFT;
				break;
			case ORP_DS3_L2:
				key = ORP_PAD_KEYDOWN | ORP_PAD_PSP_L2;
				break;
			case ORP_DS3_R2:
				key = ORP_PAD_KEYDOWN | ORP_PAD_PSP_R2;
				break;
			case ORP_DS3_L1:
				key = ORP_PAD_KEYDOWN | ORP_PAD_PSP_L1;
				break;
			case ORP_DS3_R1:
				key = ORP_PAD_KEYDOWN | ORP_PAD_PSP_R1;
				break;
			case ORP_DS3_TRI:
				key = ORP_PAD_KEYDOWN | ORP_PAD_PSP_TRI;
				break;
			case ORP_DS3_CIRCLE:
				key = ORP_PAD_KEYDOWN | ORP_PAD_PSP_CIRCLE;
				break;
			case ORP_DS3_X:
				key = ORP_PAD_KEYDOWN | ORP_PAD_PSP_X;
				break;
			case ORP_DS3_SQUARE:
				key = ORP_PAD_KEYDOWN | ORP_PAD_PSP_SQUARE;
				break;
			case ORP_DS3_HOME:
				key = ORP_PAD_KEYDOWN | ORP_PAD_PSP_HOME;
				break;
			}
			break;

		case SDL_JOYAXISMOTION:
			switch (event.jaxis.axis) {
			case 0:
				key = ORP_PAD_PSP_LXAXIS;
				if (event.jaxis.value >= 0) {
					jsl_xaxis = (short)(event.jaxis.value *
						(short)SCHAR_MAX / (short)SHRT_MAX) + 0x80;
				} else {
					jsl_xaxis = (short)(event.jaxis.value *
						(short)SCHAR_MIN / (short)SHRT_MIN) + 0x80;
				}
				break;
			case 1:
				key = ORP_PAD_PSP_LYAXIS;
				if (event.jaxis.value >= 0) {
					jsl_yaxis = (short)(event.jaxis.value *
						(short)SCHAR_MAX / (short)SHRT_MAX) + 0x80;
				} else {
					jsl_yaxis = (short)(event.jaxis.value *
						(short)SCHAR_MIN / (short)SHRT_MIN) + 0x80;
				}
				break;
			case 2:
				key = ORP_PAD_PSP_RXAXIS;
				if (event.jaxis.value >= 0) {
					jsr_xaxis = (short)(event.jaxis.value *
						(short)SCHAR_MAX / (short)SHRT_MAX) + 0x80;
				} else {
					jsr_xaxis = (short)(event.jaxis.value *
						(short)SCHAR_MIN / (short)SHRT_MIN) + 0x80;
				}
				break;
			case 3:
				key = ORP_PAD_PSP_RYAXIS;
				if (event.jaxis.value >= 0) {
					jsr_yaxis = (short)(event.jaxis.value *
						(short)SCHAR_MAX / (short)SHRT_MAX) + 0x80;
				} else {
					jsr_yaxis = (short)(event.jaxis.value *
						(short)SCHAR_MIN / (short)SHRT_MIN) + 0x80;
				}
				break;
			}
			break;

		case SDL_MOUSEBUTTONUP:
			switch (event.button.button) {
			case SDL_BUTTON_LEFT:
				key = ORP_PAD_KEYUP | ORP_PAD_PSP_X;
				break;
			case SDL_BUTTON_RIGHT:
				key = ORP_PAD_KEYUP | ORP_PAD_PSP_TRI;
				break;
			case SDL_BUTTON_WHEELUP:
				key = ORP_PAD_KEYUP | ORP_PAD_PSP_DPUP;
				SDL_Delay(50);
				break;
			case SDL_BUTTON_WHEELDOWN:
				key = ORP_PAD_KEYUP | ORP_PAD_PSP_DPDOWN;
				SDL_Delay(50);
				break;
			}
			break;

		case SDL_MOUSEBUTTONDOWN:
			switch (event.button.button) {
			case SDL_BUTTON_LEFT:
				key = ORP_PAD_KEYDOWN | ORP_PAD_PSP_X;
				break;
			case SDL_BUTTON_MIDDLE:
				key = ORP_PAD_KEYDOWN | ORP_PAD_PSP_HOME;
				break;
			case SDL_BUTTON_RIGHT:
				key = ORP_PAD_KEYDOWN | ORP_PAD_PSP_TRI;
				break;
			case SDL_BUTTON_WHEELUP:
				key = ORP_PAD_KEYDOWN | ORP_PAD_PSP_DPUP;
				break;
			case SDL_BUTTON_WHEELDOWN:
				key = ORP_PAD_KEYDOWN | ORP_PAD_PSP_DPDOWN;
				break;
			}
			break;
		}

		if (key) {
			count++;
			Uint16 value = (Uint16)(key & 0x0000ffff);

			if (value == ORP_PAD_PSP_HOME) {
				//orpDumpPadState(statePadHome);
				//SDLNet_TCP_Send(skt, statePadHome, ORP_PADSTATE_LEN);
				orpSendPadState(skt,
					statePadHome, 0, count, 0, headers);
				continue;
			}

			if (key & ORP_PAD_KEYUP) {
				statePad[((value & 0xff00) >> 8)] &=
					~((Uint8)(value & 0x00ff));
			} else if (key & ORP_PAD_KEYDOWN) {
				statePad[((value & 0xff00) >> 8)] |=
					((Uint8)(value & 0x00ff));
			} else if (key == ORP_PAD_PSP_LXAXIS) {
				jsl_xaxis = SDL_Swap16(jsl_xaxis);
				memcpy(statePad + key, &jsl_xaxis, sizeof(Sint16));
			} else if (key == ORP_PAD_PSP_LYAXIS) {
				jsl_yaxis = SDL_Swap16(jsl_yaxis);
				memcpy(statePad + key, &jsl_yaxis, sizeof(Sint16));
			} else if (key == ORP_PAD_PSP_RXAXIS) {
				jsr_xaxis = SDL_Swap16(jsr_xaxis);
				memcpy(statePad + key, &jsr_xaxis, sizeof(Sint16));
			} else if (key == ORP_PAD_PSP_RYAXIS) {
				jsr_yaxis = SDL_Swap16(jsr_yaxis);
				memcpy(statePad + key, &jsr_yaxis, sizeof(Sint16));
			}

			id++;
			// TODO: This calculation is very approximate, needs to be fixed!
			timestamp = (SDL_GetTicks() - ticks) / 16;	

			orpSendPadState(skt,
				statePad, id, count, timestamp, headers);

		} else if (kbmap_queue.size()) {
			for (i = 0; i < kbmap_queue.size(); i++) {
				Uint16 value = (Uint16)kbmap_queue[i];

				// Send key down...
				id++;
				count++;
				timestamp = (SDL_GetTicks() - ticks) / 16;

				statePad[((value & 0xff00) >> 8)] |=
					((Uint8)(value & 0x00ff));

				orpSendPadState(skt,
					statePad, id, count, timestamp, headers);

				// Sleep...
				SDL_Delay(100);

				// Send key up...
				id++;
				count++;
				timestamp = (SDL_GetTicks() - ticks) / 16;	

				statePad[((value & 0xff00) >> 8)] &=
					~((Uint8)(value & 0x00ff));

				orpSendPadState(skt,
					statePad, id, count, timestamp, headers);

				// Sleep...
				SDL_Delay(50);
			}
			kbmap_queue.clear();
		}
	}

	// Never reached...
	return 0;
}

Sint32 OpenRemotePlay::SessionPerform(void)
{
	SetCaption("Connecting...");

	CURL *curl = curl_easy_init();

	ostringstream os;
	os << "http://";
	os << config.ps3_addr << ":" << config.ps3_port;
	os << ORP_GET_SESSION;

	curl_easy_setopt(curl, CURLOPT_URL, os.str().c_str());
	curl_easy_setopt(curl, CURLOPT_VERBOSE, 1);
	curl_easy_setopt(curl, CURLOPT_DEBUGFUNCTION, orpCurlDebug);
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
		orpPrintf("Required video codec not found: %s\n",
			orpGetHeaderValue(HEADER_VIDEO_CODEC, headerList));
		return -1;
	}
	AVCodec *audioCodec = GetCodec(
		orpGetHeaderValue(HEADER_AUDIO_CODEC, headerList));
	if (!audioCodec) {
		orpPrintf("Required audio codec not found: %s\n",
			orpGetHeaderValue(HEADER_AUDIO_CODEC, headerList));
		return -1;
	}

	session_id = orpGetHeaderValue(HEADER_SESSIONID, headerList);

	ps3_nickname = (char *)base64.Decode((const Uint8 *)
		orpGetHeaderValue(HEADER_PS3_NICKNAME, headerList));
	if (ps3_nickname)
		SetCaption((const char *)ps3_nickname);
	else
		SetCaption("Unknown");

	// Play sound...
	static bool played = false;
	if (!played) { played = true; orpPlaySound(launch_wav, launch_wav_len); }
#ifdef ORP_CLOCK_DEBUG
	timer = SDL_AddTimer(1000, orpClockTimer, (void *)&clock);
#endif

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
	videoConfig->stream->lock = SDL_CreateMutex();
	videoConfig->stream->cond = SDL_CreateCond();
	videoConfig->stream->buffer = &video_buffer;

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
	audioConfig->stream->lock = SDL_CreateMutex();
	audioConfig->stream->cond = SDL_CreateCond();
	audioConfig->stream->buffer = &video_buffer;

	if (!(thread_video_connection = SDL_CreateThread(orpThreadVideoConnection,
		videoConfig))) return -1;

	if (!(thread_audio_connection = SDL_CreateThread(orpThreadAudioConnection,
		audioConfig))) return -1;

	struct orpThreadDecode_t *videoDecode = new struct orpThreadDecode_t;
	videoDecode->terminate = false;
	videoDecode->view = &view;
	videoDecode->codec = videoCodec;
	videoDecode->stream = videoConfig->stream;
	videoDecode->clock = &clock;

	if (!(thread_video_decode = SDL_CreateThread(orpThreadVideoDecode,
		videoDecode))) return -1;

	struct orpThreadDecode_t *audioDecode = new struct orpThreadDecode_t;
	audioDecode->terminate = false;
	audioDecode->codec = audioCodec;
	audioDecode->stream = audioConfig->stream;
	audioDecode->clock = &clock;

	if (!(thread_audio_decode = SDL_CreateThread(orpThreadAudioDecode,
		audioDecode))) return -1;

	// Hang-out here until something happens...
	Sint32 result = SessionControl();

	// Shutdown...
	videoDecode->terminate = audioDecode->terminate = true;

	Sint32 thread_result;
	SDL_CondBroadcast(videoDecode->stream->cond);
	SDL_CondBroadcast(audioDecode->stream->cond);

	SDL_WaitThread(thread_video_connection, &thread_result);
	SDL_WaitThread(thread_audio_connection, &thread_result);

	SDL_WaitThread(thread_video_decode, &thread_result);
	SDL_WaitThread(thread_audio_decode, &thread_result);

	// TODO: Free, clean-up everything, being really lazy here...
	if (result == EVENT_RESTORE)
		orpPrintf("Session restore.\n");
	else
		orpPrintf("Session terminated.\n");

	thread_video_connection = thread_video_decode = NULL;
	thread_audio_connection = thread_audio_decode = NULL;

	return result;
}

// vi: ts=4
