#ifndef _BASE64_H
#define _BASE64_H

class Base64
{
public:
	Uint8 *Encode(const Uint8 *src, Uint32 len = 0, Uint8 *dst = NULL);
	Uint8 *Decode(const Uint8 *src, Uint32 len = 0, Uint8 *dst = NULL);

private:
	Sint32 CodeToValue(Uint8 c);

	void Encode1To4(const Uint8 *src, Uint8 *dst);
	void Encode2To4(const Uint8 *src, Uint8 *dst);
	void Encode3To4(const Uint8 *src, Uint8 *dst);

	void PrivateEncode(const Uint8 *src, Uint32 len, Uint8 *dst);

	Sint32 Decode2To1(const Uint8 *src, Uint8 *dst);
	Sint32 Decode3To2(const Uint8 *src, Uint8 *dst);
	Sint32 Decode4To3(const Uint8 *src, Uint8 *dst);

	Sint32 PrivateDecode(const Uint8 *src, Uint32 len, Uint8 *dst);
};

// vi: ts=4
#endif // _BASE64_H
