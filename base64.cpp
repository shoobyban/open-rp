#include <string.h>

#include <SDL.h>

#include "base64.h"

static Uint8 *base64_chars = (Uint8 *)
	"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

// Base64::Encode
// If the destination argument is NULL, a return buffer is 
// allocated, and the data therein will be null-terminated.  
// If the destination argument is not NULL, it is assumed to
// be of sufficient size, and the contents will not be null-
// terminated by this routine.
//
// Returns null if the allocation fails.
Uint8 *Base64::Encode(const Uint8 *src, Uint32 len, Uint8 *dst)
{
	if (len == 0) len = strlen((const char *)src);
	if (dst == NULL) {
		Uint32 dst_len = ((len + 2) / 3) * 4;
		if((dst = (Uint8 *)new Uint8[dst_len + 1]) == NULL) return NULL;
		dst[dst_len] = 0;
	}

	this->PrivateEncode(src, len, dst);
	return dst;
}

// Base64::Decode
// If the destination argument is NULL, a return buffer is
// allocated and the data therein will be null-terminated.
// If the destination argument is not null, it is assumed
// to be of sufficient size, and the data will not be null-
// terminated by this routine.
//
// Returns null if the allocation fails, or if the source string is 
// not well-formed.
Uint8 *Base64::Decode(const Uint8 *src, Uint32 len, Uint8 *dst)
{
	bool allocated = false;

	if (src == NULL) return NULL;
	if (len == 0) len = strlen((const char *)src);
	if (len && (len & 3) == 0) {
		if (src[len - 1] == (Uint8)'=') {
			if (src[len - 2] == (Uint8)'=') len -= 2;
			else len -= 1;
		}
	}

	if (dst == NULL) {
		Uint32 dst_len = ((len * 3) / 4);
		if ((dst = (Uint8 *)new Uint8[dst_len + 1]) == NULL) return NULL;
		dst[dst_len] = 0;
		allocated = true;
	}

	if (this->PrivateDecode(src, len, dst) != 0) {
		if (allocated) delete [] dst;
		return NULL;
	}

	return dst;
}

Sint32 Base64::CodeToValue(Uint8 c)
{
	if(c >= (Uint8)'A' && c <= (Uint8)'Z')
		return (Sint32)(c - (Uint8)'A');
	else if(c >= (Uint8)'a' && c <= (Uint8)'z')
		return ((Sint32)(c - (Uint8)'a') + 26);
	else if(c >= (Uint8)'0' && c <= (Uint8)'9')
		return ((Sint32)(c - (Uint8)'0') + 52);
	else if(c == (Uint8)'+')
		return (Sint32)62;
	else if(c == (Uint8)'/')
		return (Sint32)63;

	return -1;
}

void Base64::Encode1To4(const Uint8 *src, Uint8 *dst)
{
	dst[0] = base64_chars[(Uint32)((src[0] >> 2) & 0x3F)];
    dst[1] = base64_chars[(Uint32)((src[0] & 0x03) << 4)];
	dst[2] = (Uint8)'=';
	dst[3] = (Uint8)'=';
}

void Base64::Encode2To4(const Uint8 *src, Uint8 *dst)
{
	dst[0] = base64_chars[(Uint32)((src[0] >> 2) & 0x3F)];
	dst[1] = base64_chars[(Uint32)(((src[0] & 0x03) << 4) | ((src[1] >> 4) & 0x0F))];
	dst[2] = base64_chars[(Uint32)((src[1] & 0x0F) << 2)];
	dst[3] = (Uint8)'=';
}

void Base64::Encode3To4(const Uint8 *src, Uint8 *dst)
{
	Uint32 b32 = 0;
	Sint32 i, j = 18;

	for(i = 0; i < 3; i++) {
		b32 <<= 8;
		b32 |= (Uint32)src[i];
	}

	for(i = 0; i < 4; i++) {
		dst[i] = base64_chars[(Uint32)((b32 >> j) & 0x3F)];
		j -= 6;
	}
}

void Base64::PrivateEncode(const Uint8 *src, Uint32 len, Uint8 *dst)
{
	while(len >= 3) {
		this->Encode3To4(src, dst);
		src += 3; dst += 4; len -= 3;
	}

	switch(len) {
	case 0:
		break;
	case 1:
		this->Encode1To4(src, dst);
		break;
	case 2:
		this->Encode2To4(src, dst);
		break;
	}
}

Sint32 Base64::Decode2To1(const Uint8 *src, Uint8 *dst)
{
	Uint32 b32;
	Sint32 bits;
	Uint32 ubits;

	if ((bits = this->CodeToValue(src[0])) < 0) return -1;

	ubits = (Uint32)bits;
	b32 = (ubits << 2);

	if ((bits = this->CodeToValue(src[1])) < 0) return -1;

	ubits = (Uint32)bits;
	b32 |= (ubits >> 4);

	dst[0] = (Uint8)b32;

	return 0;
}

Sint32 Base64::Decode3To2(const Uint8 *src, Uint8 *dst)
{
	Uint32 b32 = 0;
	Sint32 bits;
	Uint32 ubits;

	if ((bits = this->CodeToValue(src[0])) < 0) return -1;

	b32 = (Uint32)bits;
	b32 <<= 6;

	if ((bits = this->CodeToValue(src[1])) < 0) return -1;

	b32 |= (Uint32)bits;
	b32 <<= 4;

	if ((bits = this->CodeToValue(src[2])) < 0) return -1;

	ubits = (Uint32)bits;
	b32 |= (ubits >> 2);

	dst[0] = (Uint8)((b32 >> 8) & 0xFF);
	dst[1] = (Uint8)(b32 & 0xFF);

	return 0;
}

Sint32 Base64::Decode4To3(const Uint8 *src, Uint8 *dst)
{
	Uint32 b32 = 0;
	Sint32 i, bits;

	for(i = 0; i < 4; i++) {
		if ((bits = this->CodeToValue(src[i])) < 0) return -1;
		b32 <<= 6;
		b32 |= bits;
	}

	dst[0] = (Uint8)((b32 >> 16) & 0xFF);
	dst[1] = (Uint8)((b32 >> 8) & 0xFF);
	dst[2] = (Uint8)((b32) & 0xFF);

	return 0;
}

Sint32 Base64::PrivateDecode(const Uint8 *src, Uint32 len, Uint8 *dst)
{
	while(len >= 4) {
		if (this->Decode4To3(src, dst) != 0) return -1;
		src += 4; dst += 3; len -= 4;
	}

	Sint32 result;

	switch(len) {
	case 1:
	default:
		result = -1;
		break;
	case 0:
		result = 0;
		break;
	case 2:
		result = this->Decode2To1(src, dst);
		break;
	case 3:
		result = this->Decode3To2(src, dst);
		break;
	}

	return result;
}

// vi: ts=4
