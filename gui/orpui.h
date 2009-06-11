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

#ifndef _ORPUI_H
#define _ORPUI_H

#include <wx/wx.h>
#include <wx/spinctrl.h>
#include <wx/listbase.h>
#include <wx/listctrl.h>
#include <wx/combo.h>

#include "config.h"

#define orpID_LAUNCH	(wxID_HIGHEST + 1)
#define orpID_LIST		(wxID_HIGHEST + 2)
#define orpID_REFRESH	(wxID_HIGHEST + 3)
#define orpID_CONFIG	(wxID_HIGHEST + 4)
#define orpID_SCAN		(wxID_HIGHEST + 5)
#define orpID_CIRCLE	(wxID_HIGHEST + 6)
#define orpID_SQUARE	(wxID_HIGHEST + 7)
#define orpID_TRIANGLE	(wxID_HIGHEST + 8)
#define orpID_X			(wxID_HIGHEST + 9)
#define orpID_DP_LEFT	(wxID_HIGHEST + 10)
#define orpID_DP_RIGHT	(wxID_HIGHEST + 11)
#define orpID_DP_UP		(wxID_HIGHEST + 12)
#define orpID_DP_DOWN	(wxID_HIGHEST + 13)
#define orpID_SELECT	(wxID_HIGHEST + 14)
#define orpID_START		(wxID_HIGHEST + 15)
#define orpID_L1		(wxID_HIGHEST + 16)
#define orpID_L2		(wxID_HIGHEST + 17)
#define orpID_L3		(wxID_HIGHEST + 18)
#define orpID_R1		(wxID_HIGHEST + 19)
#define orpID_R2		(wxID_HIGHEST + 20)
#define orpID_R3		(wxID_HIGHEST + 21)
#define orpID_HOME		(wxID_HIGHEST + 22)

class orpUIApp : public wxApp
{
private:
	virtual bool OnInit();
};

class orpUIEditPanel : public wxPanel
{
public:
	orpUIEditPanel(wxFrame *parent);

	DECLARE_EVENT_TABLE()
};

class orpUIEditFrame : public wxFrame
{
public:
	orpUIEditFrame(wxFrame *parent,
		orpConfigCtx_t *config, orpConfigRecord_t *record);
	~orpUIEditFrame() { if (record) delete record; };
		
	void OnSave(wxCommandEvent& event);
	void OnCancel(wxCommandEvent& event);
	void OnDelete(wxCommandEvent& event);

	DECLARE_EVENT_TABLE()

private:
	struct orpConfigCtx_t *config;
	struct orpConfigRecord_t *record;
	wxTextCtrl *ps3_nickname;
	wxTextCtrl *ps3_hostname;
	wxSpinCtrl *ps3_port;
	wxCheckBox *ps3_nosrch;
	wxCheckBox *ps3_wolr;
};

class orpPlayStationButton : public wxControl
{
public:
	orpPlayStationButton(wxWindow *parent, wxWindowID id,
		const wxBitmap &normal, const wxBitmap &disabled);

	void OnPaint(wxPaintEvent &event);
	void OnLeftDown(wxMouseEvent &event);
	void OnLeftUp(wxMouseEvent &event);
	void OnEnterWindow(wxMouseEvent &event);
	void OnLeaveWindow(wxMouseEvent &event);

	DECLARE_EVENT_TABLE()

private:
	wxBitmap normal;
	wxBitmap disabled;
};

class orpKeyboardCtrl : public wxTextCtrl
{
public:
	orpKeyboardCtrl(wxWindow *parent);

	DECLARE_EVENT_TABLE()

	void OnKeyDown(wxKeyEvent& event);
	void UpdateValue(void);

private:
	int key;
	bool ctrl;
	bool shift;
	bool alt;
	bool meta;
};

class orpUIKeyboardPanel : public wxPanel
{
public:
	orpUIKeyboardPanel(wxFrame *parent);

	DECLARE_EVENT_TABLE()

private:
//	wxImage *logo;
};

class orpUIKeyboardFrame : public wxFrame
{
public:
	orpUIKeyboardFrame(wxFrame *parent);

	void OnCancel(wxCommandEvent& event);
	void OnButton(wxCommandEvent& event);

	DECLARE_EVENT_TABLE()

private:
	//wxBitmapButton *CreateButton(wxWindow *parent, wxWindowID id);
	orpPlayStationButton *CreateButton(wxWindow *parent, wxWindowID id);

	orpKeyboardCtrl *bt_square;
	orpKeyboardCtrl *bt_triangle;
	orpKeyboardCtrl *bt_circle;
	orpKeyboardCtrl *bt_x;
	orpKeyboardCtrl *bt_dp_left;
	orpKeyboardCtrl *bt_dp_right;
	orpKeyboardCtrl *bt_dp_up;
	orpKeyboardCtrl *bt_dp_down;
	orpKeyboardCtrl *bt_select;
	orpKeyboardCtrl *bt_start;
	orpKeyboardCtrl *bt_l1;
	orpKeyboardCtrl *bt_l2;
	orpKeyboardCtrl *bt_l3;
	orpKeyboardCtrl *bt_r1;
	orpKeyboardCtrl *bt_r2;
	orpKeyboardCtrl *bt_r3;
	orpKeyboardCtrl *bt_home;
};

class orpUIPanel : public wxPanel
{
public:
	orpUIPanel(wxFrame *parent);

	void OnPaint(wxPaintEvent& event);

	DECLARE_EVENT_TABLE()

private:
	wxImage *logo;
	wxImage *dash_hacks;
};

class orpUIFrame : public wxFrame
{
public:
	orpUIFrame(const wxString& title);

	void OnImport(wxCommandEvent& event);
	void OnEdit(wxCommandEvent& event);
	void OnConfig(wxCommandEvent& event);
	void OnDelete(wxCommandEvent& event);
	void OnLaunch(wxCommandEvent& event);
	void OnActivate(wxListEvent& event);
	void OnRefresh(wxCommandEvent& event);

	DECLARE_EVENT_TABLE()

private:
	wxListCtrl *lb;
	struct orpConfigCtx_t *config;

	void RefreshProfileList(void);
};

#endif // _ORPUI_H
// vi: ts=4
