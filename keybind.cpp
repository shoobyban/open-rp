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

#include <string.h>

#include "keybind.h"

using namespace std;

orpKeyBinding::orpKeyBinding(const char *filename)
{
	this->filename = strdup(filename);
}

orpKeyBinding::~orpKeyBinding()
{
	long i;
	for (i = 0; i < map.size(); i++) delete map[i];
	if (filename) free(filename);
}

bool orpKeyBinding::Load(void)
{
	return false;
}

void orpKeyBinding::LoadDefaults(void)
{
}

#ifdef __WXWINDOWS__
bool orpKeyBinding::Bind(enum orpButton button, struct orpUIKeyData_t *key)
{
	return false;
}

void orpKeyBinding::UpdateName(struct orpUIKeyData_t *key)
{
}

bool orpKeyBinding::Save(void)
{
}
#else
enum orpButton orpKeyBinding::KeyLookup(SDL_keysym *keysym)
{
	return OBT_NONE;
}
#endif // __WXWINDOWS__

// vi: ts=4
