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

	return 0;
}

int orpConfigClose(struct orpConfigCtx_t *ctx)
{
	if (ctx->h_file) fclose(h_file);
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

// vi: ts=4
