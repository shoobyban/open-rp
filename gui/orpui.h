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

class orpUIApp : public wxApp
{
private:
	virtual bool OnInit();
};

class orpUIEditPanel : public wxPanel
{
public:
	orpUIEditPanel(wxFrame *parent);

	void OnPaint(wxPaintEvent& event);
	void SetLogoOffset(wxPoint& pos1, wxPoint& pos2, wxSize &size)
	{
		int image_height = logo->GetHeight();
		logo_offset.x = pos1.x + size.GetWidth() + 10;
		logo_offset.y = pos1.y;
		logo_offset.y += size.GetHeight();
		logo_offset.y = (pos2.y - ((pos2.y - logo_offset.y) / 2)) - image_height / 2;
	};

	DECLARE_EVENT_TABLE()

private:
	wxImage *logo;
	wxPoint logo_offset;
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
	wxTextCtrl *psp_owner;
	wxTextCtrl *ps3_hostname;
	wxSpinCtrl *ps3_port;
	wxCheckBox *ps3_nosrch;
	wxTextCtrl *ps3_mac[ORP_MAC_LEN];
	wxTextCtrl *psp_mac[ORP_MAC_LEN];
	wxTextCtrl *psp_id[ORP_KEY_LEN];
	wxTextCtrl *pkey[ORP_KEY_LEN];
};

class orpUIJoystickPanel : public wxPanel
{
public:
	orpUIJoystickPanel(wxFrame *parent);

	void OnPaint(wxPaintEvent& event);

	DECLARE_EVENT_TABLE()

private:
//	wxImage *logo;
};

class orpUIJoystickFrame : public wxFrame
{
public:
	orpUIJoystickFrame(wxFrame *parent);

	void OnScan(wxCommandEvent& event);

	DECLARE_EVENT_TABLE()

private:
	wxComboBox *jslist;
};

class orpUIPanel : public wxPanel
{
public:
	orpUIPanel(wxFrame *parent);

	void OnPaint(wxPaintEvent& event);

	DECLARE_EVENT_TABLE()

private:
	wxImage *logo;
};

class orpUIFrame : public wxFrame
{
public:
	orpUIFrame(const wxString& title, const wxPoint& pos, const wxSize& size);

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
