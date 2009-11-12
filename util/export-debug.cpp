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

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

#include <iostream>
#include <iomanip>
#include <sstream>
#include <vector>

#include <string.h>

#ifdef __MINGW32__
// We don't want SDL to override our main()
#undef main
#endif
#include "config.h"

static const char *ps3_nickname = "Nickname";
static const char *psp_owner= "Owner";
static const Uint8 psp_id[] = {
	0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
	0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0c, 0x0d, 0x0e
};
static const Uint8 ps3_mac[] = {
	0x00, 0x01, 0x02, 0x03, 0x04, 0x05
};
static const Uint8 psp_mac[] = {
	0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b
};
static const Uint8 pkey[] = {
	0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
	0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0c, 0x0d, 0x0e
};

int main(int argc, char *argv[])
{
	struct orpConfigRecord_t record;
	memset(&record, 0, sizeof(struct orpConfigRecord_t));
	record.ps3_port = ORP_PORT;
	strcpy((char *)record.ps3_hostname, "0.0.0.0");
	strncpy((char *)record.ps3_nickname, ps3_nickname, ORP_NICKNAME_LEN);
	strncpy((char *)record.psp_owner, psp_owner, ORP_NICKNAME_LEN);
	memcpy(record.ps3_mac, ps3_mac, ORP_MAC_LEN);
	memcpy(record.psp_mac, psp_mac, ORP_MAC_LEN);
	memcpy(record.psp_id, psp_id, ORP_KEY_LEN);
	memcpy(record.pkey, pkey, ORP_KEY_LEN);
	struct orpConfigHeader_t header;
	memset(&header, 0, sizeof(struct orpConfigHeader_t));
	header.magic[0] = 'O'; header.magic[1] = 'R'; header.magic[2] = 'P';
	header.version = ORP_CONFIG_VER;
	header.flags = ORP_CONFIG_EXPORT;
	FILE *h = fopen("export-debug.orp", "w");
	if (h) {
		fwrite(&header, 1, sizeof(struct orpConfigHeader_t), h);
		fwrite(&record, 1, sizeof(struct orpConfigRecord_t), h);
		fclose(h);
	}

	return 0;
}

// vi: ts=4
