#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

#include <iostream>
#include <iomanip>
#include <sstream>
#include <vector>

#include <string.h>

#include "orp.h"
#include "base64.h"

#ifdef __MINGW32__
// We don't want SDL to override our main()
#undef main
#endif

int main(int argc, char *argv[])
{
	if (argc != 3) {
		cerr << "Required argument missing.\n";
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
	orpConfigGetKey(&config_ctx, orpID_KEY_0, config.key.skey0);
	orpConfigGetKey(&config_ctx, orpID_KEY_1, config.key.skey1);
	orpConfigGetKey(&config_ctx, orpID_KEY_2, config.key.skey2);

	config.ps3_port = record.ps3_port;
	strcpy(config.ps3_addr, (const char *)record.ps3_hostname);
	strcpy(config.psp_owner, (const char *)record.psp_owner);
	memcpy(config.ps3_mac, record.ps3_mac, ORP_MAC_LEN);
	memcpy(config.psp_mac, record.psp_mac, ORP_MAC_LEN);
	memcpy(config.key.psp_id, record.psp_id, ORP_KEY_LEN);
	memcpy(config.key.pkey, record.pkey, ORP_KEY_LEN);

	orpConfigClose(&config_ctx);

	// Initialize ORP API object
	OpenRemotePlay orp(&config);

	// Create and start the ORP session
	try {
		orp.SessionCreate();
	} catch (int e) {
		cerr << "Exception: " << e << endl;
	}

	// Wait for the session to terminate
	orp.SessionDestroy();

	return 0;
}

// vi: ts=4
