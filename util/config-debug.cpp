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

#ifndef ORP_CONFIG_DEBUG
#error "ORP_CONFIG_DEBUG not defined."
#endif
#include "orp-conf.h"

using namespace std;

int main(int argc, char *argv[])
{
	if (argc != 2) {
		cerr << "Required argument missing.\n";
		return 1;
	}

	struct orpConfigCtx_t config_ctx;
	if (orpConfigOpen(&config_ctx, argv[1]) < 0) {
		cerr << "Configuration open error: %s";
		cerr << argv[1] << endl;
		return 1;
	}

	orpConfigDebug(&config_ctx, stderr);
	return 0;
}

// vi: ts=4
