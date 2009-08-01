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

#ifndef _CONFIG_H
#define _CONFIG_H

#ifndef ORP_PSP
#include <SDL/SDL_stdinc.h>
#endif

#define ORP_KEY_LEN			16
#define ORP_MAC_LEN			6
#define ORP_NICKNAME_LEN	128
#define ORP_HOSTNAME_LEN	128

// Default Remote Play TCP/UDP port
#define ORP_PORT			9293

// WoL packet length
#define ORP_WOLPKT_LEN		256

// Config format version
#define ORP_CONFIG_VER		2

// Config header and record flags
#define ORP_CONFIG_DELETED	0x10000000
#define ORP_CONFIG_EXPORT	0x20000000
#define ORP_CONFIG_KEYS		0x40000000
#define ORP_CONFIG_NOSRCH	0x80000000
#define ORP_CONFIG_WOLR		0x01000000
#define ORP_CONFIG_BR384	0x02000000
#define ORP_CONFIG_BR768	0x04000000
#define ORP_CONFIG_BR1024	0x08000000
#define ORP_CONFIG_BR256	0x00100000
#define ORP_CONFIG_BR512	0x00200000
#define ORP_CONFIG_PRIVATE	0x00400000

struct orpConfigHeader_t
{
	Uint8 magic[3];
	Uint8 version;
	Uint32 flags;
	Uint8 reserved[8];
	Uint8 skey0[ORP_KEY_LEN];
	Uint8 skey1[ORP_KEY_LEN];
	Uint8 skey2[ORP_KEY_LEN];
};

struct orpConfigRecord_v1_t
{
	Uint32 flags;
	Uint16 ps3_port;
	Uint8 ps3_hostname[ORP_HOSTNAME_LEN];
	Uint8 ps3_nickname[ORP_NICKNAME_LEN];
	Uint8 ps3_mac[ORP_MAC_LEN];
	Uint8 psp_mac[ORP_MAC_LEN];
	Uint8 psp_id[ORP_KEY_LEN];
	Uint8 psp_owner[ORP_NICKNAME_LEN];
	Uint8 pkey[ORP_KEY_LEN];
};

#define orpConfigRecord_t	orpConfigRecord_v2_t

struct orpConfigRecord_v2_t
{
	Uint32 flags;
	Uint16 ps3_port;
	Uint8 ps3_hostname[ORP_HOSTNAME_LEN];
	Uint8 ps3_nickname[ORP_NICKNAME_LEN];
	Uint8 ps3_mac[ORP_MAC_LEN];
	Uint8 psp_mac[ORP_MAC_LEN];
	Uint8 psp_id[ORP_KEY_LEN];
	Uint8 psp_owner[ORP_NICKNAME_LEN];
	Uint8 pkey[ORP_KEY_LEN];
	Uint8 psn_login[ORP_NICKNAME_LEN];
};

#ifndef ORP_PSP
struct orpConfigCtx_t
{
	FILE *h_file;
	char *filename;
	struct orpConfigHeader_t header;
};

enum orpID_KEY {
	orpID_KEY_0,
	orpID_KEY_1,
	orpID_KEY_2
};

int orpConfigOpen(struct orpConfigCtx_t *ctx, const char *filename);
int orpConfigUpgrade(struct orpConfigCtx_t *ctx);
int orpConfigClose(struct orpConfigCtx_t *ctx);
int orpConfigRewind(struct orpConfigCtx_t *ctx);
int orpConfigRead(struct orpConfigCtx_t *ctx, struct orpConfigRecord_t *rec);
long orpConfigFind(struct orpConfigCtx_t *ctx, struct orpConfigRecord_t *rec);
int orpConfigDelete(struct orpConfigCtx_t *ctx, const char *nickname);
int orpConfigSave(struct orpConfigCtx_t *ctx, struct orpConfigRecord_t *rec);
int orpConfigGetKey(struct orpConfigCtx_t *ctx, enum orpID_KEY which, Uint8 *dst);
int orpConfigSetKey(struct orpConfigCtx_t *ctx, enum orpID_KEY which, Uint8 *src);
#ifdef ORP_CONFIG_DEBUG
void orpConfigDebug(struct orpConfigCtx_t *ctx, FILE *output);
#endif

#endif // ORP_PSP
#endif // _CONFIG_H
// vi: ts=4
