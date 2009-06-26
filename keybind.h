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

#ifndef _KEYBIND_H
#define _KEYBIND_H

#include <vector>

#include <SDL/SDL_keyboard.h>
#ifdef __WXWINDOWS__
#include <wx/event.h>
#endif

using namespace std;

enum orpButton {
	OBT_NONE,
	OBT_CIRCLE,
	OBT_SQUARE,
	OBT_TRIANGLE,
	OBT_X,
	OBT_DP_LEFT,
	OBT_DP_RIGHT,
	OBT_DP_UP,
	OBT_DP_DOWN,
	OBT_SELECT,
	OBT_START,
	OBT_L1,
	OBT_L2,
	OBT_L3,
	OBT_R1,
	OBT_R2,
	OBT_R3,
	OBT_HOME
};

struct orpKeyBind_t {
	enum orpButton button;
	SDLKey sym;
	SDLMod mod;
};

#ifdef __WXWINDOWS__
struct orpKeyTable_t {
	SDLKey sdl;
	int wx;
	char *name;
};

struct orpUIKeyData_t {
	int key;
	bool ctrl;
	bool shift;
	bool alt;
	bool meta;
	wxString name;
};
#endif

class orpKeyBinding
{
public:
	orpKeyBinding(const char *filename);
	~orpKeyBinding();

	bool Load(void);
	void LoadDefaults(void);
#ifdef __WXWINDOWS__
	bool Save(void);
	void Bind(enum orpButton button, struct orpUIKeyData_t *key);
	struct orpKeyBind_t *ButtonLookup(enum orpButton button);
	void UpdateName(struct orpUIKeyData_t *key);
	void UpdateName(struct orpUIKeyData_t *key, enum orpButton button);
#else
	enum orpButton KeyLookup(SDL_keysym *keysym);
#endif

private:
	vector<struct orpKeyBind_t *> map;
	FILE *fh;
};

#endif // _KEYBIND_H
// vi: ts=4
