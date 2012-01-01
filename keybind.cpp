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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include <string.h>
#include <sstream>

#include "keybind.h"

using namespace std;

#ifdef __WXWINDOWS__
static struct orpKeyTable_t orpKeyTable[] = {
	{ SDLK_BACKSPACE, WXK_BACK, "Backspace" },
	{ SDLK_TAB, WXK_TAB, "Tab" },
	{ SDLK_CLEAR, WXK_CLEAR, "Clear" },
	{ SDLK_RETURN, WXK_RETURN, "Return" },
	{ SDLK_PAUSE, WXK_PAUSE, "Pause" },
	{ SDLK_ESCAPE, WXK_ESCAPE, "Esc" },
	{ SDLK_SPACE, WXK_SPACE, "Space" },
	{ SDLK_EXCLAIM, '!', "!" },
	{ SDLK_QUOTEDBL, '"', "\"" },
	{ SDLK_HASH, '#', "#" },
	{ SDLK_DOLLAR, '$', "$" },
	{ SDLK_AMPERSAND, '&', "&" },
	{ SDLK_QUOTE, '\'', "'" },
	{ SDLK_LEFTPAREN, '(', "(" },
	{ SDLK_RIGHTPAREN, ')', ")" },
	{ SDLK_ASTERISK, '*', "*" },
	{ SDLK_PLUS, '+', "+" },
	{ SDLK_COMMA, ',', "," },
	{ SDLK_MINUS, '-', "-" },
	{ SDLK_PERIOD, '.', "." },
	{ SDLK_SLASH, '/', "/" },
	{ SDLK_0, '0', "0" },
	{ SDLK_1, '1', "1" },
	{ SDLK_2, '2', "2" },
	{ SDLK_3, '3', "3" },
	{ SDLK_4, '4', "4" },
	{ SDLK_5, '5', "5" },
	{ SDLK_6, '6', "6" },
	{ SDLK_7, '7', "7" },
	{ SDLK_8, '8', "8" },
	{ SDLK_9, '9', "9" },
	{ SDLK_COLON, ':', ":" },
	{ SDLK_SEMICOLON, ';', ";" },
	{ SDLK_LESS, '<', "<" },
	{ SDLK_EQUALS, '=', "=" },
	{ SDLK_GREATER, '>', ">" },
	{ SDLK_QUESTION, '?', "?" },
	{ SDLK_AT, '@', "@" },
	{ SDLK_LEFTBRACKET, '[', "[" },
	{ SDLK_BACKSLASH, '\\', "\\" },
	{ SDLK_RIGHTBRACKET, ']', "]" },
	{ SDLK_CARET, '^', "^" },
	{ SDLK_UNDERSCORE, '_', "_" },
	{ SDLK_BACKQUOTE, '`', "`" },
	{ SDLK_a, 'A', "A" },
	{ SDLK_b, 'B', "B" },
	{ SDLK_c, 'C', "C" },
	{ SDLK_d, 'D', "D" },
	{ SDLK_e, 'E', "E" },
	{ SDLK_f, 'F', "F" },
	{ SDLK_g, 'G', "G" },
	{ SDLK_h, 'H', "H" },
	{ SDLK_i, 'I', "I" },
	{ SDLK_j, 'J', "J" },
	{ SDLK_k, 'K', "K" },
	{ SDLK_l, 'L', "L" },
	{ SDLK_m, 'M', "M" },
	{ SDLK_n, 'N', "N" },
	{ SDLK_o, 'O', "O" },
	{ SDLK_p, 'P', "P" },
	{ SDLK_q, 'Q', "Q" },
	{ SDLK_r, 'R', "R" },
	{ SDLK_s, 'S', "S" },
	{ SDLK_t, 'T', "T" },
	{ SDLK_u, 'U', "U" },
	{ SDLK_v, 'V', "V" },
	{ SDLK_w, 'W', "W" },
	{ SDLK_x, 'X', "X" },
	{ SDLK_y, 'Y', "Y" },
	{ SDLK_z, 'Z', "Z" },
	{ SDLK_DELETE, WXK_DELETE, "Del" },
	{ SDLK_KP0, WXK_NUMPAD0, "KP 0" },
	{ SDLK_KP1, WXK_NUMPAD1, "KP 1" },
	{ SDLK_KP2, WXK_NUMPAD2, "KP 2" },
	{ SDLK_KP3, WXK_NUMPAD3, "KP 3" },
	{ SDLK_KP4, WXK_NUMPAD4, "KP 4" },
	{ SDLK_KP5, WXK_NUMPAD5, "KP 5" },
	{ SDLK_KP6, WXK_NUMPAD6, "KP 6" },
	{ SDLK_KP7, WXK_NUMPAD7, "KP 7" },
	{ SDLK_KP8, WXK_NUMPAD8, "KP 8" },
	{ SDLK_KP9, WXK_NUMPAD9, "KP 9" },
	{ SDLK_KP_PERIOD, WXK_NUMPAD_DECIMAL, "KP ." },
	{ SDLK_KP_DIVIDE, WXK_NUMPAD_DIVIDE, "KP /" },
	{ SDLK_KP_MULTIPLY, WXK_NUMPAD_MULTIPLY, "KP *" },
	{ SDLK_KP_MINUS, WXK_NUMPAD_SUBTRACT, "KP -" },
	{ SDLK_KP_PLUS, WXK_NUMPAD_ADD, "KP +" },
	{ SDLK_KP_ENTER, WXK_NUMPAD_ENTER, "KP Enter" },
	{ SDLK_KP_EQUALS, WXK_NUMPAD_EQUAL, "KP =" },
	{ SDLK_UP, WXK_UP, "Up" },
	{ SDLK_DOWN, WXK_DOWN, "Down" },
	{ SDLK_RIGHT, WXK_RIGHT, "Right" },
	{ SDLK_LEFT, WXK_LEFT, "Left" },
	{ SDLK_INSERT, WXK_INSERT, "Ins" },
	{ SDLK_HOME, WXK_HOME, "Home" },
	{ SDLK_END, WXK_END, "End" },
	{ SDLK_PAGEUP, WXK_PAGEUP, "PgUp" },
	{ SDLK_PAGEDOWN, WXK_PAGEDOWN, "PgDn" },
	{ SDLK_F1, WXK_F1, "F1" },
	{ SDLK_F2, WXK_F2, "F2" },
	{ SDLK_F3, WXK_F3, "F3" },
	{ SDLK_F4, WXK_F4, "F4" },
	{ SDLK_F5, WXK_F5, "F5" },
	{ SDLK_F6, WXK_F6, "F6" },
	{ SDLK_F7, WXK_F7, "F7" },
	{ SDLK_F8, WXK_F8, "F8" },
	{ SDLK_F9, WXK_F9, "F9" },
	{ SDLK_F10, WXK_F10, "F10" },
	{ SDLK_F11, WXK_F11, "F11" },
	{ SDLK_F12, WXK_F12, "F12" },
	{ SDLK_F13, WXK_F13, "F13" },
	{ SDLK_F14, WXK_F14, "F14" },
	{ SDLK_F15, WXK_F15, "F15" },
	{ SDLK_NUMLOCK, WXK_NUMLOCK, "NumLock" },
	{ SDLK_HELP, WXK_HELP, "Help" },
	{ SDLK_PRINT, WXK_PRINT, "PrtSc" },
	{ SDLK_MENU, WXK_MENU, "Menu" },
	{ SDLK_UNKNOWN, 0, NULL },
};
#endif

orpKeyBinding::orpKeyBinding(const char *filename)
{
	LoadDefaults();
	FILE *fh = fopen(filename, "r+b");
	if (!fh)
		this->fh = fopen(filename, "w+b");
	else {
		this->fh = fh;
		Load();
	}
}

orpKeyBinding::~orpKeyBinding()
{
	long i;
	for (i = 0; i < map.size(); i++) delete map[i];
	if (fh) fclose(fh);
}

bool orpKeyBinding::Load(void)
{
	if (fh) return false;
	long i;
	for (i = 0; i < map.size(); i++) delete map[i];
	rewind(fh);
	while (!feof(fh)) {
		struct orpKeyBind_t *kb = new struct orpKeyBind_t;
		if (fread(kb, 1, sizeof(struct orpKeyBind_t), fh) !=
			sizeof(struct orpKeyBind_t)) {
				delete kb;
				break;
		}
		map.push_back(kb);
	}
	if (!map.size()) LoadDefaults();
	return true;
}

void orpKeyBinding::LoadDefaults(void)
{
	long i;
	for (i = 0; i < map.size(); i++) delete map[i];
	struct orpKeyBind_t *kb;

	kb = new struct orpKeyBind_t;
	kb->button = OBT_CIRCLE;
	kb->sym = SDLK_ESCAPE;
	kb->mod = KMOD_NONE;
	map.push_back(kb);

	kb = new struct orpKeyBind_t;
	kb->button = OBT_SQUARE;
	kb->sym = SDLK_F2;
	kb->mod = KMOD_NONE;
	map.push_back(kb);

	kb = new struct orpKeyBind_t;
	kb->button = OBT_TRIANGLE;
	kb->sym = SDLK_F1;
	kb->mod = KMOD_NONE;
	map.push_back(kb);

	kb = new struct orpKeyBind_t;
	kb->button = OBT_X;
	kb->sym = SDLK_RETURN;
	kb->mod = KMOD_NONE;
	map.push_back(kb);
	
	kb = new struct orpKeyBind_t;
	kb->button = OBT_DP_LEFT;
	kb->sym = SDLK_LEFT;
	kb->mod = KMOD_NONE;
	map.push_back(kb);

	kb = new struct orpKeyBind_t;
	kb->button = OBT_DP_RIGHT;
	kb->sym = SDLK_RIGHT;
	kb->mod = KMOD_NONE;
	map.push_back(kb);

	kb = new struct orpKeyBind_t;
	kb->button = OBT_DP_UP;
	kb->sym = SDLK_UP;
	kb->mod = KMOD_NONE;
	map.push_back(kb);

	kb = new struct orpKeyBind_t;
	kb->button = OBT_DP_DOWN;
	kb->sym = SDLK_DOWN;
	kb->mod = KMOD_NONE;
	map.push_back(kb);

	kb = new struct orpKeyBind_t;
	kb->button = OBT_SELECT;
	kb->sym = SDLK_F3;
	kb->mod = KMOD_NONE;
	map.push_back(kb);

	kb = new struct orpKeyBind_t;
	kb->button = OBT_START;
	kb->sym = SDLK_F4;
	kb->mod = KMOD_NONE;
	map.push_back(kb);

	kb = new struct orpKeyBind_t;
	kb->button = OBT_L1;
	kb->sym = SDLK_PAGEUP;
	kb->mod = KMOD_NONE;
	map.push_back(kb);

	kb = new struct orpKeyBind_t;
	kb->button = OBT_R1;
	kb->sym = SDLK_PAGEDOWN;
	kb->mod = KMOD_NONE;
	map.push_back(kb);

	kb = new struct orpKeyBind_t;
	kb->button = OBT_L2;
	kb->sym = SDLK_UNKNOWN;
	kb->mod = KMOD_NONE;
	map.push_back(kb);

	kb = new struct orpKeyBind_t;
	kb->button = OBT_R2;
	kb->sym = SDLK_UNKNOWN;
	kb->mod = KMOD_NONE;
	map.push_back(kb);

	kb = new struct orpKeyBind_t;
	kb->button = OBT_L3;
	kb->sym = SDLK_UNKNOWN;
	kb->mod = KMOD_NONE;
	map.push_back(kb);

	kb = new struct orpKeyBind_t;
	kb->button = OBT_R3;
	kb->sym = SDLK_UNKNOWN;
	kb->mod = KMOD_NONE;
	map.push_back(kb);

	kb = new struct orpKeyBind_t;
	kb->button = OBT_HOME;
	kb->sym = SDLK_HOME;
	kb->mod = KMOD_NONE;
	map.push_back(kb);
}

#ifdef __WXWINDOWS__
void orpKeyBinding::Bind(enum orpButton button, struct orpUIKeyData_t *key)
{
	struct orpKeyBind_t *kb;
	if (!(kb = ButtonLookup(button))) return;
	memset(kb, 0, sizeof(struct orpKeyBind_t));
	kb->button = button;
	if (key->ctrl) kb->mod = (SDLMod)(kb->mod | KMOD_CTRL);
	if (key->shift) kb->mod = (SDLMod)(kb->mod | KMOD_SHIFT);
	if (key->alt) kb->mod = (SDLMod)(kb->mod | KMOD_ALT);
	long i;
	for (i = 0; orpKeyTable[i].sdl != SDLK_UNKNOWN; i++) {
		if (orpKeyTable[i].wx != key->key) continue;
		kb->sym = orpKeyTable[i].sdl;
		break;
	}
}

struct orpKeyBind_t *orpKeyBinding::ButtonLookup(enum orpButton button)
{
	long i;
	for (i = 0; i < map.size(); i++) {
		if (button != map[i]->button) continue;
		return map[i];
	}
	return NULL;
}

void orpKeyBinding::UpdateName(struct orpUIKeyData_t *key)
{
	ostringstream os;
	if (key->ctrl) os << "Ctrl+";
	if (key->alt) os << "Alt+";
	if (key->shift) os << "Shift+";
	if (key->key != 0) {
		int i;
		for (i = 0; orpKeyTable[i].wx; i++) {
			if (orpKeyTable[i].wx != key->key) continue;
			os << orpKeyTable[i].name;
			break;
		}
	}
	key->name = wxString((const char *)os.str().c_str(), wxConvUTF8);
}

void orpKeyBinding::UpdateName(struct orpUIKeyData_t *key, enum orpButton button)
{
	int i;
	struct orpKeyBind_t *bind = NULL;
	for (i = 0; i < map.size(); i++) {
		if (map[i]->button != button) continue;
		bind = map[i];
		break;
	}
	if (!bind) return;

	ostringstream os;
	if (bind->mod & KMOD_CTRL) {
		os << "Ctrl+";
		key->ctrl = true;
	}
	if (bind->mod & KMOD_ALT) {
		os << "Alt+";
		key->alt = true;
	}
	if (bind->mod & KMOD_SHIFT) {
		os << "Shift+";
		key->shift = true;
	}
	if (bind->sym != 0) {
		for (i = 0; orpKeyTable[i].wx; i++) {
			if (orpKeyTable[i].sdl != bind->sym) continue;
			os << orpKeyTable[i].name;
			key->key = orpKeyTable[i].wx;
			break;
		}
	}
	key->name = wxString((const char *)os.str().c_str(), wxConvUTF8);
}

bool orpKeyBinding::Save(void)
{
	if (!fh) return false;
	rewind(fh);
	long i;
	for (i = 0; i < map.size(); i++)
		fwrite(map[i], 1, sizeof(struct orpKeyBind_t), fh);
	fflush(fh);
	return true;
}
#else
enum orpButton orpKeyBinding::KeyLookup(SDL_keysym *keysym)
{
	long i;
	for (i = 0; i < map.size(); i++) {
		if (map[i]->sym != keysym->sym || map[i]->mod != keysym->mod)
			continue;
		return map[i]->button;
	}
	return OBT_NONE;
}
#endif // __WXWINDOWS__

// vi: ts=4
