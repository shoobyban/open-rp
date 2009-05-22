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

#include "orpui.h"

#include <wx/hyperlink.h>
#include <wx/filename.h>
#include <wx/mstream.h>
#include <wx/stdpaths.h>
#include <wx/joystick.h>

#include "images.h"

BEGIN_EVENT_TABLE(orpUIPanel, wxPanel)
EVT_PAINT(orpUIPanel::OnPaint)
END_EVENT_TABLE()

BEGIN_EVENT_TABLE(orpUIFrame, wxFrame)
EVT_BUTTON(wxID_OPEN, orpUIFrame::OnImport)
EVT_BUTTON(wxID_EDIT, orpUIFrame::OnEdit)
EVT_BUTTON(orpID_CONFIG, orpUIFrame::OnConfig)
EVT_BUTTON(wxID_DELETE, orpUIFrame::OnDelete)
EVT_BUTTON(orpID_LAUNCH, orpUIFrame::OnLaunch)
EVT_BUTTON(orpID_REFRESH, orpUIFrame::OnRefresh)
EVT_LIST_ITEM_ACTIVATED(orpID_LIST, orpUIFrame::OnActivate)
END_EVENT_TABLE()

BEGIN_EVENT_TABLE(orpUIEditPanel, wxPanel)
EVT_PAINT(orpUIEditPanel::OnPaint)
END_EVENT_TABLE()

BEGIN_EVENT_TABLE(orpUIEditFrame, wxFrame)
EVT_BUTTON(wxID_SAVE, orpUIEditFrame::OnSave)
EVT_BUTTON(wxID_CANCEL, orpUIEditFrame::OnCancel)
EVT_BUTTON(wxID_DELETE, orpUIEditFrame::OnDelete)
END_EVENT_TABLE()

BEGIN_EVENT_TABLE(orpUIJoystickPanel, wxPanel)
EVT_PAINT(orpUIJoystickPanel::OnPaint)
END_EVENT_TABLE()

BEGIN_EVENT_TABLE(orpUIJoystickFrame, wxFrame)
EVT_BUTTON(orpID_SCAN, orpUIJoystickFrame::OnScan)
END_EVENT_TABLE()

IMPLEMENT_APP(orpUIApp)

bool orpUIApp::OnInit()
{
	::wxInitAllImageHandlers();

	orpUIFrame *frame = new orpUIFrame( _T("Open Remote Play v1.1"),
		wxPoint(50, 50), wxSize(340, 480));
	frame->Show(TRUE);
	SetTopWindow(frame);

	return TRUE;
}

orpUIPanel::orpUIPanel(wxFrame *parent)
	: wxPanel(parent, wxID_ANY, wxDefaultPosition,
		wxDefaultSize, wxTAB_TRAVERSAL)
{
	wxMemoryInputStream image(logo_png, logo_png_len);
	logo = new wxImage(image);
}

void orpUIPanel::OnPaint(wxPaintEvent& WXUNUSED(event))
{
	wxPaintDC dc(this);
	wxBitmap bitmap(*logo);
	dc.DrawBitmap(bitmap, 5, 5, TRUE);
}

orpUIFrame::orpUIFrame(const wxString& title,
	const wxPoint& pos, const wxSize& size)
	: wxFrame((wxFrame *)NULL, wxID_ANY, title, pos, size)
{
#ifndef __WXMAC__
	wxIcon icon;
	icon.LoadFile(_T("icon.ico"), wxBITMAP_TYPE_ICO);
	SetIcon(icon);
#endif
	orpUIPanel *panel = new orpUIPanel(this);

	lb = new wxListCtrl(panel,
		orpID_LIST, wxDefaultPosition,wxSize(-1, 160),
		wxBORDER_SUNKEN | wxLC_REPORT | wxLC_SINGLE_SEL | wxLC_HRULES);

	lb->InsertColumn(0, _T("Name"));
	lb->InsertColumn(1, _T("Address"));

	wxBoxSizer *frame_sizer = new wxBoxSizer(wxVERTICAL);
	frame_sizer->AddSpacer(98);
	wxHyperlinkCtrl *link = new wxHyperlinkCtrl(panel, wxID_ANY,
		_T("v"ORP_VERSION), _T("http://ps3-hacks.com/"));
	link->SetNormalColour(wxColour(0, 134, 174));
	frame_sizer->Add(link, 0, wxALIGN_RIGHT | wxRIGHT, 5);
	frame_sizer->Add(lb, 1, wxEXPAND | wxALL, 5);

	wxStandardPaths sp;
	if (!::wxDirExists(sp.GetUserDataDir()))
		::wxMkdir(sp.GetUserDataDir());
	wxFileName path;
	path.AssignDir(sp.GetUserDataDir());
	path.SetFullName(_T("config.orp"));

	config = new struct orpConfigCtx_t;
	if (orpConfigOpen(config, path.GetFullPath().fn_str()) != 0) {
		wxMessageDialog msg(this,
			path.GetFullPath(), _T("Open Error!"),
			wxOK | wxICON_ERROR);
		msg.ShowModal();
	} else RefreshProfileList();

	wxBoxSizer *button_sizer = new wxBoxSizer(wxHORIZONTAL);
	button_sizer->Add(new wxButton(panel, wxID_OPEN, _T("&Import")),
		0, wxRIGHT | wxLEFT | wxBOTTOM, 5);
	button_sizer->Add(new wxButton(panel, wxID_EDIT),
		0, wxRIGHT | wxLEFT | wxBOTTOM, 5);
//	button_sizer->Add(new wxButton(panel, orpID_CONFIG, _T("&Config")),
//		0, wxRIGHT | wxLEFT | wxBOTTOM, 5);
	button_sizer->Add(new wxButton(panel, wxID_DELETE),
		0, wxRIGHT | wxLEFT | wxBOTTOM, 5);
	wxButton *launch = new wxButton(panel, orpID_LAUNCH, _T("&Launch!"));
	launch->SetDefault();
	button_sizer->Add(launch, 0, wxRIGHT | wxLEFT | wxBOTTOM, 5);

	frame_sizer->Add(button_sizer, 0, wxALIGN_CENTER);

	panel->SetSizer(frame_sizer);

	CreateStatusBar();
	SetStatusText(_T("http://ps3-hacks.com"));

	frame_sizer->SetSizeHints(panel);
	SetMinSize(panel->GetSize());

	int width, height;
	lb->GetClientSize(&width, &height);
	lb->SetColumnWidth(0, (width - 10) / 2);
	lb->SetColumnWidth(1, (width - 10) / 2);

	Center();
}

void orpUIFrame::OnImport(wxCommandEvent& WXUNUSED(event))
{
	wxFileDialog import_dialog(this, _T("Choose a file to import..."),
		_T(""), _T(""), _T(""), wxFD_OPEN | wxFD_FILE_MUST_EXIST);
	import_dialog.SetWildcard(_T("ORP configs (*.orp)|*.orp|All files (*.*)|*.*"));
	if (import_dialog.ShowModal() != wxID_OK) return;
	struct orpConfigCtx_t import;
	if (orpConfigOpen(&import, import_dialog.GetPath().fn_str()) < 0) {
		wxMessageDialog msg(this,
			_T("Error opening file for import."), _T("Import Error!"),
			wxOK | wxICON_ERROR);
		msg.ShowModal();
		return;
	}
	if (import.header.flags & ORP_CONFIG_KEYS) {
		orpConfigSetKey(config, orpID_KEY_0, import.header.skey0);
		orpConfigSetKey(config, orpID_KEY_1, import.header.skey1);
		orpConfigSetKey(config, orpID_KEY_2, import.header.skey2);
		wxMessageDialog msg(this,
			_T("Keys imported."), _T("Success!"), wxOK | wxICON_INFORMATION);
		msg.ShowModal();
		orpConfigClose(&import);
		return;
	}
	wxString imported;
	struct orpConfigRecord_t record;
	while (orpConfigRead(&import, &record)) {
		orpConfigSave(config, &record);
		RefreshProfileList();
		if (import.header.flags & ORP_CONFIG_EXPORT) {
			struct orpConfigRecord_t *rec = new struct orpConfigRecord_t;
			memcpy(rec, &record, sizeof(struct orpConfigRecord_t));
			orpUIEditFrame *edit = new orpUIEditFrame(this, config, rec);
			edit->Show();
		}
		imported.Append(
			wxString((const char *)record.ps3_nickname, wxConvUTF8));
		imported.Append(_T("\n"));
	}
	if (!(import.header.flags & ORP_CONFIG_EXPORT)) {
		if (imported.Length()) {
			imported.Prepend(_T("Imported:\n"));
			wxMessageDialog msg(this,
				imported, _T("Success!"),
				wxOK | wxICON_INFORMATION);
			msg.ShowModal();
		} else {
			wxMessageDialog msg(this,
				_T("No Profiles Found!"), _T("No Profiles Found!"),
				wxOK | wxICON_EXCLAMATION);
			msg.ShowModal();
		}
	}
	orpConfigClose(&import);
}

void orpUIFrame::OnEdit(wxCommandEvent& WXUNUSED(event))
{
	if (lb->GetSelectedItemCount() == 0) return;
	long idx = lb->GetNextItem(-1,
		wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
	if (idx == -1) return;
	wxListItem item;
	item.SetId(idx);
	item.SetColumn(0);
	item.SetMask(wxLIST_MASK_TEXT);
	if (!lb->GetItem(item)) return;
	struct orpConfigRecord_t *record = new struct orpConfigRecord_t;
	strcpy((char *)record->ps3_nickname, item.GetText().fn_str());
	if (!orpConfigFind(config, record)) {
		delete record;
		return;
	}
	orpUIEditFrame *edit = new orpUIEditFrame(this, config, record);
	edit->Show();
}

void orpUIFrame::OnConfig(wxCommandEvent& WXUNUSED(event))
{
	orpUIJoystickFrame *joyframe = new orpUIJoystickFrame(this);
	joyframe->Show();
}

void orpUIFrame::OnDelete(wxCommandEvent& WXUNUSED(event))
{
	if (lb->GetSelectedItemCount() == 0) return;
	long idx = lb->GetNextItem(-1,
		wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
	if (idx == -1) return;
	wxListItem item;
	item.SetId(idx);
	item.SetColumn(0);
	item.SetMask(wxLIST_MASK_TEXT);
	if (!lb->GetItem(item)) return;
	wxString msg;
	msg.Printf(_T("Delete this profile?\n%s"),
		item.GetText().c_str());
	wxMessageDialog confirm(this, msg, _T("Delete?"),
		wxYES | wxNO | wxICON_QUESTION);
	if (confirm.ShowModal() == wxID_NO) return;
	orpConfigDelete(config, item.GetText().fn_str());
	RefreshProfileList();
}

void orpUIFrame::OnLaunch(wxCommandEvent& WXUNUSED(event))
{
	if (!config->filename) return;

	if (lb->GetSelectedItemCount() == 0) return;
	long idx = lb->GetNextItem(-1,
		wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
	if (idx == -1) return;
	wxListItem item;
	item.SetId(idx);
	item.SetColumn(0);
	item.SetMask(wxLIST_MASK_TEXT);
	if (!lb->GetItem(item)) return;

	wxStandardPaths sp;
	wxFileName path = sp.GetExecutablePath();
	wxFileName orp;
	orp.AssignDir(path.GetPath());
#ifndef __WXMSW__
	orp.SetFullName(_T("orp"));
	orp.MakeAbsolute();
	if (!::wxFileExists(orp.GetFullPath())) {
		wxString text;
		text.Printf(_T("Unable to locate ORP player.\n%s"),
			orp.GetFullPath().c_str());
		wxMessageDialog msg(this,
			text, _T("Player Error!"), wxOK | wxICON_ERROR);
		msg.ShowModal();
		return;
	}
#else
	orp.SetFullName(_T("orp.exe"));
	::wxSetWorkingDirectory(path.GetPath());
#endif
	wxString cfg(config->filename, wxConvUTF8);
	const wxChar *argv[4];
	argv[0] = orp.GetFullPath().c_str();
	argv[1] = cfg.c_str();
	argv[2] = item.GetText().c_str();
	argv[3] = NULL;
	::wxExecute((wxChar **)argv);
}

void orpUIFrame::OnActivate(wxListEvent& event)
{
	if (lb->GetSelectedItemCount() == 0) return;
	long idx = lb->GetNextItem(-1,
		wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
	if (idx == -1) return;
	wxListItem item;
	item.SetId(idx);
	item.SetColumn(0);
	item.SetMask(wxLIST_MASK_TEXT);
	if (!lb->GetItem(item)) return;
	struct orpConfigRecord_t *record = new struct orpConfigRecord_t;
	strcpy((char *)record->ps3_nickname, item.GetText().fn_str());
	if (!orpConfigFind(config, record)) {
		delete record;
		return;
	}
	orpUIEditFrame *edit = new orpUIEditFrame(this, config, record);
	edit->Show();
}

void orpUIFrame::RefreshProfileList(void)
{
	lb->DeleteAllItems();
	orpConfigRewind(config);

	wxString addr;
	struct orpConfigRecord_t rec;
	while (orpConfigRead(config, &rec)) {
		long idx = lb->InsertItem(0,
			wxString((const char *)rec.ps3_nickname, wxConvUTF8));
		addr.Printf(_T("%s:%d"),
			wxString((const char *)rec.ps3_hostname, wxConvUTF8).c_str(),
			rec.ps3_port);
		lb->SetItem(idx, 1, addr);
	}
}

void orpUIFrame::OnRefresh(wxCommandEvent& WXUNUSED(event))
{
	RefreshProfileList();
}

orpUIEditPanel::orpUIEditPanel(wxFrame *parent)
	: wxPanel(parent, wxID_ANY, wxDefaultPosition,
		wxDefaultSize, wxTAB_TRAVERSAL)
{
	wxMemoryInputStream image(edit_png, edit_png_len);
	logo = new wxImage(image);
}

void orpUIEditPanel::OnPaint(wxPaintEvent& WXUNUSED(event))
{
	wxPaintDC dc(this);
	wxBitmap bitmap(*logo);
	dc.DrawBitmap(bitmap, logo_offset.x, logo_offset.y, TRUE);
}

orpUIEditFrame::orpUIEditFrame(wxFrame *parent,
	orpConfigCtx_t *config, orpConfigRecord_t *record)
	: config(config), record(record),
	wxFrame(parent, wxID_ANY, _T("Edit Profile"))
{
#ifndef __WXMAC__
	wxIcon icon;
	icon.LoadFile(_T("icon.ico"), wxBITMAP_TYPE_ICO);
	SetIcon(icon);
#endif
	SetTitle(wxString((const char *)record->ps3_nickname, wxConvUTF8));

	orpUIEditPanel *panel = new orpUIEditPanel(this);

	wxBoxSizer *frame_sizer = new wxBoxSizer(wxVERTICAL);

	wxBoxSizer *row_sizer;
	
	row_sizer = new wxBoxSizer(wxHORIZONTAL);
	row_sizer->Add(50, -1, 0);
	ps3_nickname = new wxTextCtrl(panel, wxID_ANY, 
		wxString((const char *)record->ps3_nickname, wxConvUTF8),
		wxDefaultPosition, wxSize(180, -1));
	row_sizer->Add(ps3_nickname, 0, wxRIGHT | wxLEFT | wxBOTTOM, 5);

	frame_sizer->Add(new wxStaticText(panel, wxID_ANY, _T("Name:")),
		0, wxRIGHT | wxLEFT | wxBOTTOM | wxTOP, 5);
	frame_sizer->Add(row_sizer);

	row_sizer = new wxBoxSizer(wxHORIZONTAL);
	row_sizer->Add(50, -1, 0);
	psp_owner = new wxTextCtrl(panel, wxID_ANY, 
		wxString((const char *)record->psp_owner, wxConvUTF8),
		wxDefaultPosition, wxSize(180, -1));
	row_sizer->Add(psp_owner, 0, wxRIGHT | wxLEFT | wxBOTTOM, 5);

	frame_sizer->Add(new wxStaticText(panel, wxID_ANY, _T("PSP Owner:")),
		0, wxRIGHT | wxLEFT | wxBOTTOM | wxTOP, 5);
	frame_sizer->Add(row_sizer);

	row_sizer = new wxBoxSizer(wxHORIZONTAL);
	row_sizer->Add(50, -1, 0);
	ps3_hostname = new wxTextCtrl(panel, wxID_ANY, 
		wxString((const char *)record->ps3_hostname, wxConvUTF8),
		wxDefaultPosition, wxSize(180, -1));
	row_sizer->Add(ps3_hostname, 0, wxRIGHT | wxLEFT | wxBOTTOM, 5);
	wxString port_label;
	port_label << (int)record->ps3_port;
	ps3_port = new wxSpinCtrl(panel, wxID_ANY,
		port_label, wxDefaultPosition, wxSize(80, -1),
		wxSP_ARROW_KEYS, 1, 65535, (int)record->ps3_port);
	row_sizer->Add(ps3_port, 0, wxRIGHT | wxBOTTOM | wxALIGN_TOP, 5);

	frame_sizer->Add(new wxStaticText(panel, wxID_ANY, _T("Address / Port:")),
		0, wxRIGHT | wxLEFT | wxBOTTOM, 5);
	frame_sizer->Add(row_sizer);

	int i;
	row_sizer = new wxBoxSizer(wxHORIZONTAL);
	row_sizer->Add(55, -1, 0);
	for (i = 0; i < ORP_MAC_LEN; i++) {
		wxString octet;
		octet.Printf(_T("%02X"), record->ps3_mac[i]);
		ps3_mac[i] = new wxTextCtrl(panel, wxID_ANY, octet,
			wxDefaultPosition, wxSize(32, -1));
		ps3_mac[i]->SetEditable(false);
		row_sizer->Add(ps3_mac[i], 0, wxRIGHT | wxBOTTOM, 5);
	}

	frame_sizer->Add(new wxStaticText(panel, wxID_ANY, _T("PS3 MAC Address:")),
		0, wxRIGHT | wxLEFT | wxBOTTOM, 5);
	frame_sizer->Add(row_sizer);

	row_sizer = new wxBoxSizer(wxHORIZONTAL);
	row_sizer->Add(55, -1, 0);
	for (i = 0; i < ORP_MAC_LEN; i++) {
		wxString octet;
		octet.Printf(_T("%02X"), record->psp_mac[i]);
		psp_mac[i] = new wxTextCtrl(panel, wxID_ANY, octet,
			wxDefaultPosition, wxSize(32, -1));
		psp_mac[i]->SetEditable(false);
		row_sizer->Add(psp_mac[i], 0, wxRIGHT | wxBOTTOM, 5);
	}

	frame_sizer->Add(new wxStaticText(panel, wxID_ANY, _T("PSP MAC Address:")),
		0, wxRIGHT | wxLEFT | wxBOTTOM, 5);
	frame_sizer->Add(row_sizer);

	row_sizer = new wxBoxSizer(wxHORIZONTAL);
	row_sizer->Add(55, -1, 0);
	for (i = 0; i < ORP_KEY_LEN; i++) {
		wxString octet;
		octet.Printf(_T("%02X"), record->psp_id[i]);
		psp_id[i] = new wxTextCtrl(panel, wxID_ANY, octet,
			wxDefaultPosition, wxSize(32, -1));
		psp_id[i]->SetEditable(false);
	}
	for (i = 0; i < 8; i++) row_sizer->Add(psp_id[i], 0, wxRIGHT, 5);

	frame_sizer->Add(new wxStaticText(panel, wxID_ANY, _T("PSP ID:")),
		0, wxRIGHT | wxLEFT | wxBOTTOM, 5);
	frame_sizer->Add(row_sizer, 0, wxBOTTOM, 2);

	row_sizer = new wxBoxSizer(wxHORIZONTAL);
	row_sizer->Add(55, -1, 0);
	for (; i < ORP_KEY_LEN; i++)
		row_sizer->Add(psp_id[i], 0, wxRIGHT | wxBOTTOM, 5);
	frame_sizer->Add(row_sizer);

	row_sizer = new wxBoxSizer(wxHORIZONTAL);
	row_sizer->Add(55, -1, 0);
	for (i = 0; i < ORP_KEY_LEN; i++) {
		wxString octet;
		octet.Printf(_T("%02X"), record->pkey[i]);
		pkey[i] = new wxTextCtrl(panel, wxID_ANY, octet,
			wxDefaultPosition, wxSize(32, -1));
		pkey[i]->SetEditable(false);
	}
	for (i = 0; i < 8; i++) row_sizer->Add(pkey[i], 0, wxRIGHT, 5);

	frame_sizer->Add(new wxStaticText(panel, wxID_ANY,
		_T("Remote Play Private Key:")),
		0, wxRIGHT | wxLEFT | wxBOTTOM, 5);
	frame_sizer->Add(row_sizer, 0, wxBOTTOM, 2);

	row_sizer = new wxBoxSizer(wxHORIZONTAL);
	row_sizer->Add(55, -1, 0);
	for (; i < ORP_KEY_LEN; i++)
		row_sizer->Add(pkey[i], 0, wxRIGHT | wxBOTTOM, 5);
	frame_sizer->Add(row_sizer);

	row_sizer = new wxBoxSizer(wxHORIZONTAL);
	row_sizer->Add(new wxButton(panel, wxID_SAVE),
		0, wxRIGHT | wxLEFT | wxBOTTOM, 5);
	row_sizer->Add(new wxButton(panel, wxID_DELETE),
		0, wxRIGHT | wxLEFT | wxBOTTOM, 5);
	wxButton *cancel = new wxButton(panel, wxID_CANCEL);
	cancel->SetDefault();
	row_sizer->Add(cancel, 0, wxRIGHT | wxLEFT | wxBOTTOM, 5);

	frame_sizer->Add(row_sizer, 0, wxALIGN_CENTER | wxTOP | wxBOTTOM, 8);

	panel->SetSizer(frame_sizer);
	frame_sizer->SetSizeHints(panel);
	wxSize size = panel->GetSize();
#ifdef __WXMAC__
	size.SetHeight(size.GetHeight() + 20);
#elif defined(__WXMSW__)
	size.SetHeight(size.GetHeight() + 40);
#endif
	SetMinSize(size);

	wxPoint pos1 = ps3_nickname->GetPosition();
	wxPoint pos2 = psp_owner->GetPosition();
	size = ps3_nickname->GetSize();
	panel->SetLogoOffset(pos1, pos2, size);

	CenterOnParent();
}

void orpUIEditFrame::OnCancel(wxCommandEvent& WXUNUSED(event))
{
	Close();
}

void orpUIEditFrame::OnDelete(wxCommandEvent& event)
{
	Close();
	::wxPostEvent(GetParent(), event);
}

void orpUIEditFrame::OnSave(wxCommandEvent& WXUNUSED(event))
{
	Close();

	wxString value(ps3_nickname->GetValue().Mid(0, ORP_NICKNAME_LEN - 1));
	strcpy((char *)record->ps3_nickname, value.fn_str());
	record->ps3_port = (unsigned short)ps3_port->GetValue();
	value = psp_owner->GetValue().Mid(0, ORP_NICKNAME_LEN - 1);
	strcpy((char *)record->psp_owner, value.fn_str());
	value = ps3_hostname->GetValue().Mid(0, ORP_HOSTNAME_LEN - 1);
	strcpy((char *)record->ps3_hostname, value.fn_str());

	// TODO: finish saving the rest of the config...

	orpConfigSave(config, record);

	wxCommandEvent event(wxEVT_COMMAND_BUTTON_CLICKED, orpID_REFRESH);
	::wxPostEvent(GetParent(), event);
}

orpUIJoystickPanel::orpUIJoystickPanel(wxFrame *parent)
	: wxPanel(parent, wxID_ANY, wxDefaultPosition,
		wxSize(320, 240), wxTAB_TRAVERSAL)
{
//	wxMemoryInputStream image(logo_png, logo_png_len);
//	logo = new wxImage(image);
}

void orpUIJoystickPanel::OnPaint(wxPaintEvent& WXUNUSED(event))
{
	wxPaintDC dc(this);
//	wxBitmap bitmap(*logo);
//	dc.DrawBitmap(bitmap, 5, 5, TRUE);
}

orpUIJoystickFrame::orpUIJoystickFrame(wxFrame *parent)
	: wxFrame(parent, wxID_ANY, _T("Joystick Configuration"))
{
#ifndef __WXMAC__
	wxIcon icon;
	icon.LoadFile(_T("icon.ico"), wxBITMAP_TYPE_ICO);
	SetIcon(icon);
#endif

	orpUIJoystickPanel *panel = new orpUIJoystickPanel(this);

	wxBoxSizer *frame_sizer = new wxBoxSizer(wxVERTICAL);

	wxBoxSizer *button_sizer = new wxBoxSizer(wxHORIZONTAL);
	jslist = new wxComboBox(panel, wxID_ANY,
		wxEmptyString, wxDefaultPosition,
		wxDefaultSize, 0, NULL, wxCB_READONLY | wxCB_SORT | wxCB_DROPDOWN);
//	jslist->SetPopupControl(new wxListViewComboPopup());

	button_sizer->Add(jslist, 0, wxRIGHT | wxLEFT | wxBOTTOM, 5);
	button_sizer->Add(new wxButton(panel, orpID_SCAN, _T("&Scan")),
		0, wxRIGHT | wxLEFT | wxBOTTOM, 5);

	frame_sizer->Add(button_sizer, 0, wxALIGN_CENTER);

	panel->SetSizer(frame_sizer);
	frame_sizer->SetSizeHints(panel);
//	SetMinSize(size);

	CenterOnParent();
}

void orpUIJoystickFrame::OnScan(wxCommandEvent& WXUNUSED(event))
{
#if 0
	int jc = wxJoystick::GetNumberJoysticks();
	std::cerr << "joysticks: " << jc << std::endl;
		wxJoystick stick(0);
	int i;
	for (i = 0; i < jc; i++) {
		wxJoystick stick(i);
		if (stick.IsOk())
		jslist->Insert(stick.GetProductName(), 0);
	}
#endif
}

// vi: ts=4
