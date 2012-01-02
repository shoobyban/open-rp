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

#include <pspsdk.h>
#include <pspuser.h>
#include <pspdebug.h>
#include <psputility.h>
#include <psputility_netmodules.h>
#include <psputility_sysparam.h>
#include <pspwlan.h>
#include <pspopenpsid.h>
#include <pspreg.h>
#include <pspiofilemgr.h>
#include <pspiofilemgr_fcntl.h>

#include <string.h>
#include <malloc.h>
#include <sys/unistd.h>

typedef unsigned char Uint8;
typedef unsigned short Uint16;
typedef unsigned int Uint32;

#include "../orp-conf.h"

PSP_MODULE_INFO("ORP_Exporter", PSP_MODULE_USER, 1, 1);
PSP_MAIN_THREAD_ATTR(THREAD_ATTR_USER);

#define printf pspDebugScreenPrintf

/* Exit callback */
int exit_callback(int arg1, int arg2, void *common)
{
	sceKernelExitGame();
	return 0;
}

/* Callback thread */
int CallbackThread(SceSize args, void *argp)
{
	int cbid;

	cbid = sceKernelCreateCallback(" Exit Callback", exit_callback, NULL);
	sceKernelRegisterExitCallback(cbid);

	sceKernelSleepThreadCB();

	return 0;
}

/* Sets up the callback thread and returns its thread id */
int SetupCallbacks(void)
{
	int thid = 0;

	thid = sceKernelCreateThread("up date_thread",
		CallbackThread, 0x11, 0xFA0, 0, 0);
	if(thid >= 0) {
		sceKernelStartThread(thid , 0, 0);
	}

	return thid;
}

void *orpGetValue(char *key, SceSize *key_size)
{
	void *value = NULL;

	REGHANDLE rh;
	struct RegParam reg = {
		.regtype = 1,
		.namelen = strlen(SYSTEM_REGISTRY),
		.unk2 = 1,
		.unk3 = 1,
	};
	strcpy(reg.name, SYSTEM_REGISTRY);

	int rc;
	if ((rc = sceRegOpenRegistry(&reg, 1, &rh)) < 0) {
		printf("Error opening registry.\n");
		return value;
	}

	REGHANDLE cat;
	if ((rc = sceRegOpenCategory(rh, "/CONFIG/PREMO", 1, &cat)) < 0) {
		printf("Error opening: /CONFIG/PREMO\n");
		goto _orpGetValue_exit1;
	}

	unsigned int key_type;
	if ((rc = sceRegGetKeyInfoByName(cat, key, &key_type, key_size)) < 0)
		printf("Key info failed: %s: %d\n", key, rc);
	else {
//		printf("%s: type: %d, size: %d\n", key, key_type, *key_size);
		value = malloc(*key_size);
		if ((rc = sceRegGetKeyValueByName(cat, key, value, *key_size)) < 0) {
			free(value);
			value = NULL;
			printf("Key look-up failed: %s\n", key);
			goto _orpGetValue_exit2;
		}
	}

_orpGetValue_exit2:
	sceRegCloseCategory(cat);

_orpGetValue_exit1:
	sceRegCloseRegistry(rh);
	return value;
}

int main()
{
	pspDebugScreenInit();
	SetupCallbacks();

	printf("Open Remote Play Exporter!\n\n");

	int rc;
	struct orpConfigRecord_t record;
	memset(&record, 0, sizeof(struct orpConfigRecord_t));
	record.ps3_port = ORP_PORT;
	strcpy((char *)record.ps3_hostname, "0.0.0.0");

	SceSize key_size;
	unsigned char *value;
	if ((value = orpGetValue("ps3_name", &key_size)) != NULL) {
		printf("%16s: %s\n", "Name", value);
		strncpy((char *)record.ps3_nickname,
			(const char *)value, ORP_NICKNAME_LEN);
		free(value);
	} else {
		printf("This PSP has not been registered for Remote Play!\n");
		printf("\nPress HOME to quit.\n");

		sceKernelSleepThread();
		return 0;
	}

	if ((rc =
		sceUtilityGetSystemParamString(PSP_SYSTEMPARAM_ID_STRING_NICKNAME,
		(char *)record.psp_owner, ORP_NICKNAME_LEN)) == 0) {
		printf("%16s: %s\n", "PSP Owner", record.psp_owner);
	}

	if ((value = orpGetValue("ps3_mac", &key_size)) != NULL) {
		printf("%16s: ", "PS3 MAC Address");
		int i;
		for (i = 0; i < key_size - 1; i++)
			printf("%02x:", value[i]);
		printf("%02x\n", value[key_size - 1]);
		memcpy(record.ps3_mac, value, ORP_MAC_LEN);
		free(value);
	}

	u8 mac[8];
	if ((rc = sceWlanGetEtherAddr(mac)) == 0) {
		printf("%16s: ", "PSP MAC Address");
		int i;
		memcpy(record.psp_mac, mac, ORP_MAC_LEN);
		for (i = 0; i < ORP_MAC_LEN - 1; i++)
			printf("%02x:", record.psp_mac[i]);
		printf("%02x\n", record.psp_mac[ORP_MAC_LEN - 1]);
	}

	PspOpenPSID psid;
	memset(&psid, 0, sizeof(psid));
	if ((rc = sceOpenPSIDGetOpenPSID(&psid) == 0)) {
		printf("%16s: ", "PSP ID");
		int i;
		for (i = 0; i < sizeof(psid.data) - 1; i++)
			printf("%02x", psid.data[i]);
		printf("%02x\n", psid.data[sizeof(psid.data) - 1]);
		memcpy(record.psp_id, psid.data, ORP_KEY_LEN);
	}

	if ((value = orpGetValue("ps3_key", &key_size)) != NULL) {
		printf("%16s: ", "Private Key");
		int i;
		for (i = 0; i < key_size; i++)
			printf("%02x", value[i]);
		printf("\n");
		memcpy(record.pkey, value, ORP_KEY_LEN);
		free(value);
	}

	char path[] = { "ms0:/export.orp" };
	sceIoRemove(path);
	SceUID fd = sceIoOpen(path,
		PSP_O_WRONLY | PSP_O_CREAT, 0777);
	if (fd < 0)
		printf("\nUnable to open: %s\n", path);
	else {
		struct orpConfigHeader_t header;
		memset(&header, 0, sizeof(struct orpConfigHeader_t));
		header.magic[0] = 'O'; header.magic[1] = 'R'; header.magic[2] = 'P';
		header.version = ORP_CONFIG_VER;
		header.flags = ORP_CONFIG_EXPORT;
		sceIoWrite(fd, &header, sizeof(struct orpConfigHeader_t));
		sceIoWrite(fd, &record, sizeof(struct orpConfigRecord_t));
		sceIoClose(fd);
		printf("\nConfiguration saved to: %s\n", path);
	}

	printf("\nPress HOME to quit.\n");

	sceKernelSleepThread();
	return 0;
}

// vi: ts=4
