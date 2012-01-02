//////////////////////////////////////////////////////////////////////////////
//
// Open Remote Play
// http://code.google.com/p/open-rp/
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

#include "wxorp.h"

#include <wx/hyperlink.h>
#include <wx/filename.h>
#include <wx/mstream.h>
#include <wx/stdpaths.h>
#include <wx/joystick.h>
#include <wx/artprov.h>

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
END_EVENT_TABLE()

BEGIN_EVENT_TABLE(orpUIEditFrame, wxFrame)
EVT_RADIOBUTTON(orpID_NET_PUB, orpUIEditFrame::OnNetworkType)
EVT_RADIOBUTTON(orpID_NET_PRIV, orpUIEditFrame::OnNetworkType)
EVT_BUTTON(wxID_SAVE, orpUIEditFrame::OnSave)
EVT_BUTTON(wxID_CANCEL, orpUIEditFrame::OnCancel)
EVT_BUTTON(wxID_DELETE, orpUIEditFrame::OnDelete)
END_EVENT_TABLE()

BEGIN_EVENT_TABLE(orpPlayStationButton, wxControl)
EVT_PAINT(orpPlayStationButton::OnPaint)
EVT_LEFT_DOWN(orpPlayStationButton::OnLeftDown)
EVT_LEFT_UP(orpPlayStationButton::OnLeftUp)
END_EVENT_TABLE()

BEGIN_EVENT_TABLE(orpUIKeyboardPanel, wxPanel)
END_EVENT_TABLE()

BEGIN_EVENT_TABLE(orpUIKeyboardFrame, wxFrame)
EVT_BUTTON(wxID_SAVE, orpUIKeyboardFrame::OnSave)
EVT_BUTTON(orpID_DEFAULT, orpUIKeyboardFrame::OnDefault)
EVT_BUTTON(orpID_RESET, orpUIKeyboardFrame::OnReset)
EVT_BUTTON(wxID_CANCEL, orpUIKeyboardFrame::OnCancel)
EVT_BUTTON(orpID_CIRCLE, orpUIKeyboardFrame::OnButton)
EVT_BUTTON(orpID_SQUARE, orpUIKeyboardFrame::OnButton)
EVT_BUTTON(orpID_TRIANGLE, orpUIKeyboardFrame::OnButton)
EVT_BUTTON(orpID_X, orpUIKeyboardFrame::OnButton)
EVT_BUTTON(orpID_DP_LEFT, orpUIKeyboardFrame::OnButton)
EVT_BUTTON(orpID_DP_RIGHT, orpUIKeyboardFrame::OnButton)
EVT_BUTTON(orpID_DP_UP, orpUIKeyboardFrame::OnButton)
EVT_BUTTON(orpID_DP_DOWN, orpUIKeyboardFrame::OnButton)
EVT_BUTTON(orpID_SELECT, orpUIKeyboardFrame::OnButton)
EVT_BUTTON(orpID_START, orpUIKeyboardFrame::OnButton)
EVT_BUTTON(orpID_L1, orpUIKeyboardFrame::OnButton)
EVT_BUTTON(orpID_L2, orpUIKeyboardFrame::OnButton)
EVT_BUTTON(orpID_L3, orpUIKeyboardFrame::OnButton)
EVT_BUTTON(orpID_R1, orpUIKeyboardFrame::OnButton)
EVT_BUTTON(orpID_R2, orpUIKeyboardFrame::OnButton)
EVT_BUTTON(orpID_R3, orpUIKeyboardFrame::OnButton)
EVT_BUTTON(orpID_HOME, orpUIKeyboardFrame::OnButton)
END_EVENT_TABLE()

BEGIN_EVENT_TABLE(orpKeyboardCtrl, wxTextCtrl)
EVT_KEY_DOWN(orpKeyboardCtrl::OnKeyDown)
END_EVENT_TABLE()

IMPLEMENT_APP(orpUIApp)

bool orpUIApp::OnInit()
{
	::wxInitAllImageHandlers();

	orpUIFrame *frame = new orpUIFrame( _T("Open Remote Play"));
	frame->Show(TRUE);
	SetTopWindow(frame);

	return TRUE;
}

orpUIPanel::orpUIPanel(wxFrame *parent)
	: wxPanel(parent, wxID_ANY, wxDefaultPosition,
		wxDefaultSize, wxTAB_TRAVERSAL)
{
	wxMemoryInputStream *image;
	image = new wxMemoryInputStream(__images_logo_png, __images_logo_png_len);
	logo = new wxImage(*image);
	delete image;
}

void orpUIPanel::OnPaint(wxPaintEvent& WXUNUSED(event))
{
	wxPaintDC dc(this);
	wxBitmap *bitmap;
	bitmap = new wxBitmap(*logo);
	dc.DrawBitmap(*bitmap, 5, 5, TRUE);
	delete bitmap;
}

orpUIFrame::orpUIFrame(const wxString& title)
	: wxFrame((wxFrame *)NULL, wxID_ANY, title)
{
#ifndef __WXMAC__
	wxIcon icon;
	icon.LoadFile(_T("icon.ico"), wxBITMAP_TYPE_ICO);
	SetIcon(icon);
#endif
	orpUIPanel *panel = new orpUIPanel(this);

	lb = new wxListCtrl(panel,
		orpID_LIST, wxDefaultPosition, wxSize(-1, 120),
		wxBORDER_SUNKEN | wxLC_REPORT | wxLC_SINGLE_SEL | wxLC_HRULES);

	lb->InsertColumn(0, _T("Name"));
	lb->InsertColumn(1, _T("Address"));

	wxBoxSizer *frame_sizer = new wxBoxSizer(wxVERTICAL);
	frame_sizer->AddSpacer(98);
	wxHyperlinkCtrl *link = new wxHyperlinkCtrl(panel, wxID_ANY,
		_T("v"PACKAGE_VERSION), _T("http://code.google.com/p/open-rp/"));
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
	button_sizer->Add(new wxButton(panel, orpID_CONFIG, _T("I&nput")),
		0, wxRIGHT | wxLEFT | wxBOTTOM, 5);
	button_sizer->Add(new wxButton(panel, wxID_DELETE),
		0, wxRIGHT | wxLEFT | wxBOTTOM, 5);
	wxButton *launch = new wxButton(panel, orpID_LAUNCH, _T("&Launch!"));
	launch->SetDefault();
	button_sizer->Add(launch, 0, wxRIGHT | wxLEFT | wxBOTTOM, 5);

	frame_sizer->Add(button_sizer, 0, wxALIGN_CENTER);

	panel->SetSizer(frame_sizer);

	CreateStatusBar();
	SetStatusText(_T("http://code.google.com/p/open-rp/"));

	frame_sizer->SetSizeHints(panel);
	SetInitialSize(wxSize(480, -1));
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
	orpUIKeyboardFrame *frame = new orpUIKeyboardFrame(this);
	frame->Show();
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
	if (confirm.ShowModal() == wxID_YES) {
		orpConfigDelete(config, item.GetText().fn_str());
		RefreshProfileList();
	}
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
		wxDefaultSize, wxTAB_TRAVERSAL) { }

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

	frame_sizer->Add(new wxStaticText(panel, wxID_ANY, _T("Connection Type:")),
		0, wxRIGHT | wxLEFT | wxBOTTOM | wxTOP, 5);

	row_sizer = new wxBoxSizer(wxHORIZONTAL);

	row_sizer->Add(50, -1, 0);
	net_public = new wxRadioButton(panel, orpID_NET_PUB, _T("Public"),
		wxDefaultPosition, wxDefaultSize, wxRB_GROUP);
	row_sizer->Add(net_public, 0, wxLEFT | wxRIGHT | wxBOTTOM, 5);
	net_private = new wxRadioButton(panel, orpID_NET_PRIV, _T("Private"));
	row_sizer->Add(net_private, 0, wxBOTTOM, 5);
	frame_sizer->Add(row_sizer);

	row_sizer = new wxBoxSizer(wxHORIZONTAL);
	row_sizer->Add(50, -1, 0);
	psn_login = new wxTextCtrl(panel, wxID_ANY, 
		wxString((const char *)record->psn_login, wxConvUTF8),
		wxDefaultPosition, wxSize(180, -1));
	if (record->flags & ORP_CONFIG_PRIVATE) {
		psn_login->Enable(false);
		net_private->SetValue(true);
	} else {
		psn_login->Enable(true);
		net_public->SetValue(true);
	}
	row_sizer->Add(psn_login, 0, wxRIGHT | wxLEFT | wxBOTTOM, 5);
	wxHyperlinkCtrl *link = new wxHyperlinkCtrl(panel, wxID_ANY,
		_T("Help!"), _T("http://code.google.com/p/open-rp/wiki/EditProfile"));
	row_sizer->Add(link, 0, wxRIGHT | wxBOTTOM | wxALIGN_BOTTOM, 5);
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
		0, wxRIGHT | wxLEFT | wxTOP | wxBOTTOM, 5);
	frame_sizer->Add(row_sizer);

	row_sizer = new wxBoxSizer(wxHORIZONTAL);
	row_sizer->Add(50, -1, 0);
	ps3_nosrch = new wxCheckBox(panel, wxID_ANY,
		_T("Disable UDP Search?"));
	if (record->flags & ORP_CONFIG_NOSRCH)
		ps3_nosrch->SetValue(true);
	row_sizer->Add(ps3_nosrch, 0, wxRIGHT | wxLEFT | wxBOTTOM, 5);
	frame_sizer->Add(row_sizer);

	row_sizer = new wxBoxSizer(wxHORIZONTAL);
	row_sizer->Add(50, -1, 0);
	ps3_wolr = new wxCheckBox(panel, wxID_ANY,
		_T("Enable WoL Reflector?"));
	if (record->flags & ORP_CONFIG_WOLR)
		ps3_wolr->SetValue(true);
	row_sizer->Add(ps3_wolr, 0, wxRIGHT | wxLEFT | wxBOTTOM, 5);
	frame_sizer->Add(row_sizer);

	bitrates[BR_1024] = _T("+2 (1024k)");
	bitrates[BR_768] = _T("+1 (768k)");
	bitrates[BR_512] = _T("Normal (512k)");
	bitrates[BR_384] = _T("-1 (384k)");
	bitrates[BR_256] = _T("-2 (256k)");
	ps3_bitrate = new wxComboBox(panel, wxID_ANY, 
		bitrates[BR_1024], wxDefaultPosition, wxDefaultSize,
		BR_MAX, bitrates, wxCB_DROPDOWN | wxCB_READONLY);

	if (record->flags & ORP_CONFIG_BR256)
		ps3_bitrate->SetValue(bitrates[BR_256]);
	else if (record->flags & ORP_CONFIG_BR384)
		ps3_bitrate->SetValue(bitrates[BR_384]);
	else if (record->flags & ORP_CONFIG_BR512)
		ps3_bitrate->SetValue(bitrates[BR_512]);
	else if (record->flags & ORP_CONFIG_BR768)
		ps3_bitrate->SetValue(bitrates[BR_768]);
	else if (record->flags & ORP_CONFIG_BR1024)
		ps3_bitrate->SetValue(bitrates[BR_1024]);
	else
		ps3_bitrate->SetValue(bitrates[BR_1024]);

	row_sizer = new wxBoxSizer(wxHORIZONTAL);
	row_sizer->Add(50, -1, 0);
	row_sizer->Add(ps3_bitrate, 0, wxBOTTOM, 5);

	frame_sizer->Add(new wxStaticText(panel, wxID_ANY, _T("Default Bitrate:")),
		0, wxRIGHT | wxLEFT | wxTOP, 5);
	frame_sizer->Add(row_sizer, 0, wxTOP, 5);

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
	SetInitialSize();
	wxSize size = panel->GetSize();
#ifdef __WXMAC__
	size.SetHeight(size.GetHeight() + 20);
#endif
	SetMinSize(size);
	CenterOnParent();
}

void orpUIEditFrame::OnNetworkType(wxCommandEvent& event)
{
	switch (event.GetId()) {
	case orpID_NET_PUB:
		psn_login->Enable();
		psn_login->SetFocus();
		break;
	case orpID_NET_PRIV:
		psn_login->Enable(false);
		break;
	}
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
	if (net_public->GetValue() && psn_login->IsEmpty()) {
		wxMessageDialog msg(this,
			_T("You must enter your PSN login ID\nfor public connection types.\n"),
			_T("Save Error!"), wxOK | wxICON_ERROR);
		msg.ShowModal();
		return;
	}

	Close();

	wxString value(ps3_nickname->GetValue().Mid(0, ORP_NICKNAME_LEN - 1));
	strcpy((char *)record->ps3_nickname, value.fn_str());
	record->ps3_port = (unsigned short)ps3_port->GetValue();
	value = ps3_hostname->GetValue().Mid(0, ORP_HOSTNAME_LEN - 1);
	strcpy((char *)record->ps3_hostname, value.fn_str());
	if (ps3_nosrch->IsChecked())
		record->flags |= ORP_CONFIG_NOSRCH;
	else
		record->flags &= ~ORP_CONFIG_NOSRCH;
	if (ps3_wolr->IsChecked())
		record->flags |= ORP_CONFIG_WOLR;
	else
		record->flags &= ~ORP_CONFIG_WOLR;
	if (net_private->GetValue())
		record->flags |= ORP_CONFIG_PRIVATE;
	else {
		record->flags &= ~ORP_CONFIG_PRIVATE;
		value = wxString(psn_login->GetValue().Mid(0, ORP_NICKNAME_LEN - 1));
		strcpy((char *)record->psn_login, value.fn_str());
	}

	if (ps3_bitrate->GetValue() == bitrates[BR_256]) {
		record->flags |= ORP_CONFIG_BR256;
		record->flags &= ~ORP_CONFIG_BR384;
		record->flags &= ~ORP_CONFIG_BR512;
		record->flags &= ~ORP_CONFIG_BR768;
		record->flags &= ~ORP_CONFIG_BR1024;
	} else if (ps3_bitrate->GetValue() == bitrates[BR_384]) {
		record->flags &= ~ORP_CONFIG_BR256;
		record->flags |= ORP_CONFIG_BR384;
		record->flags &= ~ORP_CONFIG_BR512;
		record->flags &= ~ORP_CONFIG_BR768;
		record->flags &= ~ORP_CONFIG_BR1024;
	} else if (ps3_bitrate->GetValue() == bitrates[BR_512]) {
		record->flags &= ~ORP_CONFIG_BR384;
		record->flags &= ~ORP_CONFIG_BR256;
		record->flags |= ORP_CONFIG_BR512;
		record->flags &= ~ORP_CONFIG_BR768;
		record->flags &= ~ORP_CONFIG_BR1024;
	} else if (ps3_bitrate->GetValue() == bitrates[BR_768]) {
		record->flags &= ~ORP_CONFIG_BR384;
		record->flags &= ~ORP_CONFIG_BR256;
		record->flags &= ~ORP_CONFIG_BR512;
		record->flags |= ORP_CONFIG_BR768;
		record->flags &= ~ORP_CONFIG_BR1024;
	} else if (ps3_bitrate->GetValue() == bitrates[BR_1024]) {
		record->flags &= ~ORP_CONFIG_BR384;
		record->flags &= ~ORP_CONFIG_BR256;
		record->flags &= ~ORP_CONFIG_BR512;
		record->flags &= ~ORP_CONFIG_BR768;
		record->flags |= ORP_CONFIG_BR1024;
	}
	orpConfigSave(config, record);

	wxCommandEvent event(wxEVT_COMMAND_BUTTON_CLICKED, orpID_REFRESH);
	::wxPostEvent(GetParent(), event);
}

orpPlayStationButton::orpPlayStationButton(wxWindow *parent, wxWindowID id)
	: normal(wxNullBitmap), disabled(wxNullBitmap),
	wxControl(parent, id, wxDefaultPosition, wxDefaultSize, wxBORDER_NONE)
{
	wxMemoryInputStream *stream0 = NULL;
	wxMemoryInputStream *stream1 = NULL;

	switch (id) {
	case orpID_CIRCLE:
		stream0 = new wxMemoryInputStream(
			__images_circle0_png, __images_circle1_png_len);
		stream1 = new wxMemoryInputStream(
			__images_circle1_png, __images_circle1_png_len);
		break;
	case orpID_SQUARE:
		stream0 = new wxMemoryInputStream(
			__images_square0_png, __images_square0_png_len);
		stream1 = new wxMemoryInputStream(
			__images_square1_png, __images_square1_png_len);
		break;
	case orpID_TRIANGLE:
		stream0 = new wxMemoryInputStream(
			__images_triangle0_png, __images_triangle0_png_len);
		stream1 = new wxMemoryInputStream(
			__images_triangle1_png, __images_triangle1_png_len);
		break;
	case orpID_X:
		stream0 = new wxMemoryInputStream(
			__images_x0_png, __images_x0_png_len);
		stream1 = new wxMemoryInputStream(
			__images_x1_png, __images_x1_png_len);
		break;
	case orpID_DP_LEFT:
		stream0 = new wxMemoryInputStream(
			__images_dp_left0_png, __images_dp_left0_png_len);
		stream1 = new wxMemoryInputStream(
			__images_dp_left1_png, __images_dp_left1_png_len);
		break;
	case orpID_DP_RIGHT:
		stream0 = new wxMemoryInputStream(
			__images_dp_right0_png, __images_dp_right0_png_len);
		stream1 = new wxMemoryInputStream(
			__images_dp_right1_png, __images_dp_right1_png_len);
		break;
	case orpID_DP_UP:
		stream0 = new wxMemoryInputStream(
			__images_dp_up0_png, __images_dp_up0_png_len);
		stream1 = new wxMemoryInputStream(
			__images_dp_up1_png, __images_dp_up1_png_len);
		break;
	case orpID_DP_DOWN:
		stream0 = new wxMemoryInputStream(
			__images_dp_down0_png, __images_dp_down0_png_len);
		stream1 = new wxMemoryInputStream(
			__images_dp_down1_png, __images_dp_down1_png_len);
		break;
	case orpID_SELECT:
		stream0 = new wxMemoryInputStream(
			__images_select0_png, __images_select0_png_len);
		stream1 = new wxMemoryInputStream(
			__images_select1_png, __images_select1_png_len);
		break;
	case orpID_START:
		stream0 = new wxMemoryInputStream(
			__images_start0_png, __images_start0_png_len);
		stream1 = new wxMemoryInputStream(
			__images_start1_png, __images_start1_png_len);
		break;
	case orpID_L1:
		stream0 = new wxMemoryInputStream(
			__images_l10_png, __images_l10_png_len);
		stream1 = new wxMemoryInputStream(
			__images_l11_png, __images_l11_png_len);
		break;
	case orpID_L2:
		stream0 = new wxMemoryInputStream(
			__images_l20_png, __images_l20_png_len);
		stream1 = new wxMemoryInputStream(
			__images_l21_png, __images_l21_png_len);
		break;
	case orpID_L3:
		stream0 = new wxMemoryInputStream(
			__images_l30_png, __images_l30_png_len);
		stream1 = new wxMemoryInputStream(
			__images_l31_png, __images_l31_png_len);
		break;
	case orpID_R1:
		stream0 = new wxMemoryInputStream(
			__images_r10_png, __images_r10_png_len);
		stream1 = new wxMemoryInputStream(
			__images_r11_png, __images_r11_png_len);
		break;
	case orpID_R2:
		stream0 = new wxMemoryInputStream(
			__images_r20_png, __images_r20_png_len);
		stream1 = new wxMemoryInputStream(
			__images_r21_png, __images_r21_png_len);
		break;
	case orpID_R3:
		stream0 = new wxMemoryInputStream(
			__images_r30_png, __images_r30_png_len);
		stream1 = new wxMemoryInputStream(
			__images_r31_png, __images_r31_png_len);
		break;
	case orpID_HOME:
		stream0 = new wxMemoryInputStream(
			__images_ps0_png, __images_ps0_png_len);
		stream1 = new wxMemoryInputStream(
			__images_ps1_png, __images_ps1_png_len);
		break;
	}

	normal = (stream1) ? wxBitmap(*stream1) : wxNullBitmap;
	disabled = (stream0) ? wxBitmap(*stream0) : wxNullBitmap;

	if (stream0) delete stream0;
	if (stream1) delete stream1;

	SetInitialSize(wxSize(normal.GetWidth() + 10, normal.GetHeight() + 10));
	SetCursor(wxCURSOR_HAND);
}

void orpPlayStationButton::OnPaint(wxPaintEvent& WXUNUSED(event))
{
	wxPaintDC dc(this);
	wxSize size = GetClientSize();
	wxBitmap *bitmap;
	if (IsEnabled())
		bitmap = &normal;
	else
		bitmap = &disabled;
	dc.DrawBitmap(*bitmap,
		(size.GetWidth() - bitmap->GetWidth()) / 2,
		(size.GetHeight() - bitmap->GetHeight()) / 2, TRUE);
}

void orpPlayStationButton::OnLeftDown(wxMouseEvent& event)
{
	event.Skip();
}

void orpPlayStationButton::OnLeftUp(wxMouseEvent& event)
{
	wxCommandEvent cmd(wxEVT_COMMAND_BUTTON_CLICKED, GetId());
	::wxPostEvent(GetParent(), cmd);
	event.Skip();
}

orpKeyboardCtrl::orpKeyboardCtrl(wxWindow *parent, orpKeyBinding *keybind, orpButton button)
	: keybind(keybind), button(button),
	wxTextCtrl(parent, wxID_ANY, wxEmptyString, wxDefaultPosition,
		wxSize(180, -1), wxTE_PROCESS_ENTER | wxTE_PROCESS_TAB | wxTE_CENTER)
{
	data.ctrl = false;
	data.alt = false;
	data.shift = false;
	data.key = 0;

	keybind->UpdateName(&data, button);
	SetValue(data.name);
}

void orpKeyboardCtrl::OnKeyDown(wxKeyEvent& event)
{
	switch (event.GetKeyCode()) {
	case WXK_CONTROL:
		if (data.ctrl) data.ctrl = false;
		else data.ctrl = true;
		break;
	case WXK_ALT:
		if (data.alt) data.alt = false;
		else data.alt = true;
		break;
	case WXK_SHIFT:
		if (data.shift) data.shift = false;
		else data.shift = true;
		break;
	default:
		data.key = event.GetKeyCode();
		break;
	}

	keybind->UpdateName(&data);
	SetValue(data.name);
}

orpUIKeyboardPanel::orpUIKeyboardPanel(wxFrame *parent)
	: wxPanel(parent, wxID_ANY, wxDefaultPosition,
		wxDefaultSize, wxTAB_TRAVERSAL) { }

orpUIKeyboardFrame::orpUIKeyboardFrame(wxFrame *parent)
	: wxFrame(parent, wxID_ANY, _T("Input Key Bindings"))
{
#ifndef __WXMAC__
	wxIcon icon;
	icon.LoadFile(_T("icon.ico"), wxBITMAP_TYPE_ICO);
	SetIcon(icon);
#endif
	orpUIKeyboardPanel *panel = new orpUIKeyboardPanel(this);
	wxBoxSizer *frame_sizer = new wxBoxSizer(wxVERTICAL);
	wxBoxSizer *row_sizer;
	wxTextCtrl *txt;

	keybind = new orpKeyBinding("keybind.dat");

	row_sizer = new wxBoxSizer(wxHORIZONTAL);
	row_sizer->Add(new orpPlayStationButton(panel, orpID_HOME));
	bt_home = new orpKeyboardCtrl(panel, keybind, OBT_HOME);
	bt_home->SetEditable(false);
	bt_home->Enable(false);
	row_sizer->Add(bt_home, 0, wxALIGN_CENTER_VERTICAL);
	frame_sizer->Add(row_sizer);

	row_sizer = new wxBoxSizer(wxHORIZONTAL);
	row_sizer->Add(new orpPlayStationButton(panel, orpID_SELECT));
	bt_select = new orpKeyboardCtrl(panel, keybind, OBT_SELECT);
	bt_select->SetEditable(false);
	bt_select->Enable(false);
	row_sizer->Add(bt_select, 0, wxALIGN_CENTER_VERTICAL);
	row_sizer->Add(new orpPlayStationButton(panel, orpID_START));
	bt_start = new orpKeyboardCtrl(panel, keybind, OBT_START);
	bt_start->SetEditable(false);
	bt_start->Enable(false);
	row_sizer->Add(bt_start, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 5);
	frame_sizer->Add(row_sizer);

	row_sizer = new wxBoxSizer(wxHORIZONTAL);
	row_sizer->Add(new orpPlayStationButton(panel, orpID_DP_LEFT));
	bt_dp_left = new orpKeyboardCtrl(panel, keybind, OBT_DP_LEFT);
	bt_dp_left->SetEditable(false);
	bt_dp_left->Enable(false);
	row_sizer->Add(bt_dp_left, 0, wxALIGN_CENTER_VERTICAL);
	row_sizer->Add(new orpPlayStationButton(panel, orpID_DP_RIGHT));
	bt_dp_right = new orpKeyboardCtrl(panel, keybind, OBT_DP_RIGHT);
	bt_dp_right->SetEditable(false);
	bt_dp_right->Enable(false);
	row_sizer->Add(bt_dp_right, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 5);
	frame_sizer->Add(row_sizer);

	row_sizer = new wxBoxSizer(wxHORIZONTAL);
	row_sizer->Add(new orpPlayStationButton(panel, orpID_DP_UP));
	bt_dp_up = new orpKeyboardCtrl(panel, keybind, OBT_DP_UP);
	bt_dp_up->SetEditable(false);
	bt_dp_up->Enable(false);
	row_sizer->Add(bt_dp_up, 0, wxALIGN_CENTER_VERTICAL);
	row_sizer->Add(new orpPlayStationButton(panel, orpID_DP_DOWN));
	bt_dp_down = new orpKeyboardCtrl(panel, keybind, OBT_DP_DOWN);
	bt_dp_down->SetEditable(false);
	bt_dp_down->Enable(false);
	row_sizer->Add(bt_dp_down, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 5);
	frame_sizer->Add(row_sizer);

	row_sizer = new wxBoxSizer(wxHORIZONTAL);
	row_sizer->Add(new orpPlayStationButton(panel, orpID_SQUARE));
	bt_square = new orpKeyboardCtrl(panel, keybind, OBT_SQUARE);
	bt_square->SetEditable(false);
	bt_square->Enable(false);
	row_sizer->Add(bt_square, 0, wxALIGN_CENTER_VERTICAL);
	row_sizer->Add(new orpPlayStationButton(panel, orpID_TRIANGLE));
	bt_triangle = new orpKeyboardCtrl(panel, keybind, OBT_TRIANGLE);
	bt_triangle->SetEditable(false);
	bt_triangle->Enable(false);
	row_sizer->Add(bt_triangle, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 5);
	frame_sizer->Add(row_sizer);

	row_sizer = new wxBoxSizer(wxHORIZONTAL);
	row_sizer->Add(new orpPlayStationButton(panel, orpID_CIRCLE));
	bt_circle = new orpKeyboardCtrl(panel, keybind, OBT_CIRCLE);
	bt_circle->SetEditable(false);
	bt_circle->Enable(false);
	row_sizer->Add(bt_circle, 0, wxALIGN_CENTER_VERTICAL);
	row_sizer->Add(new orpPlayStationButton(panel, orpID_X));
	bt_x = new orpKeyboardCtrl(panel, keybind, OBT_X);
	bt_x->SetEditable(false);
	bt_x->Enable(false);
	row_sizer->Add(bt_x, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 5);
	frame_sizer->Add(row_sizer);

	row_sizer = new wxBoxSizer(wxHORIZONTAL);
	row_sizer->Add(new orpPlayStationButton(panel, orpID_L1));
	bt_l1 = new orpKeyboardCtrl(panel, keybind, OBT_L1);
	bt_l1->SetEditable(false);
	bt_l1->Enable(false);
	row_sizer->Add(bt_l1, 0, wxALIGN_CENTER_VERTICAL);
	row_sizer->Add(new orpPlayStationButton(panel, orpID_R1));
	bt_r1 = new orpKeyboardCtrl(panel, keybind, OBT_R1);
	bt_r1->SetEditable(false);
	bt_r1->Enable(false);
	row_sizer->Add(bt_r1, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 5);
	frame_sizer->Add(row_sizer);

	row_sizer = new wxBoxSizer(wxHORIZONTAL);
	row_sizer->Add(new orpPlayStationButton(panel, orpID_L2));
	bt_l2 = new orpKeyboardCtrl(panel, keybind, OBT_L2);
	bt_l2->SetEditable(false);
	bt_l2->Enable(false);
	row_sizer->Add(bt_l2, 0, wxALIGN_CENTER_VERTICAL);
	row_sizer->Add(new orpPlayStationButton(panel, orpID_R2));
	bt_r2 = new orpKeyboardCtrl(panel, keybind, OBT_R2);
	bt_r2->SetEditable(false);
	bt_r2->Enable(false);
	row_sizer->Add(bt_r2, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 5);
	frame_sizer->Add(row_sizer);

	row_sizer = new wxBoxSizer(wxHORIZONTAL);
	row_sizer->Add(new orpPlayStationButton(panel, orpID_L3));
	bt_l3 = new orpKeyboardCtrl(panel, keybind, OBT_L3);
	bt_l3->SetEditable(false);
	bt_l3->Enable(false);
	row_sizer->Add(bt_l3, 0, wxALIGN_CENTER_VERTICAL);
	row_sizer->Add(new orpPlayStationButton(panel, orpID_R3));
	bt_r3 = new orpKeyboardCtrl(panel, keybind, OBT_R3);
	bt_r3->SetEditable(false);
	bt_r3->Enable(false);
	row_sizer->Add(bt_r3, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 5);
	frame_sizer->Add(row_sizer);

	row_sizer = new wxBoxSizer(wxHORIZONTAL);
	row_sizer->Add(new wxButton(panel, wxID_SAVE),
		0, wxRIGHT | wxLEFT | wxBOTTOM, 5);
	row_sizer->Add(new wxButton(panel, orpID_RESET, _T("&Reset")),
		0, wxRIGHT | wxLEFT | wxBOTTOM, 5);
	row_sizer->Add(new wxButton(panel, orpID_DEFAULT, _T("&Default")),
		0, wxRIGHT | wxLEFT | wxBOTTOM, 5);
	wxButton *cancel = new wxButton(panel, wxID_CANCEL);
	cancel->SetDefault();
	row_sizer->Add(cancel, 0, wxLEFT | wxBOTTOM, 5);
	frame_sizer->Add(row_sizer, 0, wxALIGN_CENTER | wxTOP | wxBOTTOM, 8);

	panel->SetSizer(frame_sizer);
	frame_sizer->SetSizeHints(panel);
	SetInitialSize();
	wxSize size = panel->GetSize();
	SetMinSize(size);

	CenterOnParent();
}

orpUIKeyboardFrame::~orpUIKeyboardFrame()
{
	if (keybind) delete keybind;
}

void orpUIKeyboardFrame::OnSave(wxCommandEvent& WXUNUSED(event))
{
//	orpKeyBindErrorDialog *dialog = new orpKeyBindErrorDialog(this,
//		BIND_DUPLICATE, orpID_CIRCLE, orpID_START);
//	dialog->ShowModal();
	keybind->Save();
	Close();
}

void orpUIKeyboardFrame::OnDefault(wxCommandEvent& WXUNUSED(event))
{
}

void orpUIKeyboardFrame::OnReset(wxCommandEvent& WXUNUSED(event))
{
}

void orpUIKeyboardFrame::OnCancel(wxCommandEvent& WXUNUSED(event))
{
	Close();
}

void orpUIKeyboardFrame::OnButton(wxCommandEvent& event)
{
	orpKeyboardCtrl *ctrl = NULL;

	switch (event.GetId()) {
	case orpID_CIRCLE:
		ctrl = bt_circle;
		break;
	case orpID_SQUARE:
		ctrl = bt_square;
		break;
	case orpID_TRIANGLE:
		ctrl = bt_triangle;
		break;
	case orpID_X:
		ctrl = bt_x;
		break;
	case orpID_DP_LEFT:
		ctrl = bt_dp_left;
		break;
	case orpID_DP_RIGHT:
		ctrl = bt_dp_right;
		break;
	case orpID_DP_UP:
		ctrl = bt_dp_up;
		break;
	case orpID_DP_DOWN:
		ctrl = bt_dp_down;
		break;
	case orpID_SELECT:
		ctrl = bt_select;
		break;
	case orpID_START:
		ctrl = bt_start;
		break;
	case orpID_L1:
		ctrl = bt_l1;
		break;
	case orpID_L2:
		ctrl = bt_l2;
		break;
	case orpID_L3:
		ctrl = bt_l3;
		break;
	case orpID_R1:
		ctrl = bt_r1;
		break;
	case orpID_R2:
		ctrl = bt_r2;
		break;
	case orpID_R3:
		ctrl = bt_r3;
		break;
	case orpID_HOME:
		ctrl = bt_home;
		break;
	default:
		return;
	}

	long id_list[17] = {
		orpID_CIRCLE, orpID_SQUARE, orpID_TRIANGLE,
		orpID_X, orpID_DP_LEFT, orpID_DP_RIGHT,
		orpID_DP_UP, orpID_DP_DOWN, orpID_SELECT,
		orpID_START, orpID_L1, orpID_L2, orpID_L3,
		orpID_R1, orpID_R2, orpID_R3, orpID_HOME
	};

	if (ctrl->IsEnabled()) {
		ctrl->Enable(false);
		for (int i = 0; i < sizeof(id_list); i++) {
			wxWindow *button = wxWindow::FindWindowById(id_list[i], this);
			if (button) { button->Enable(true); button->Refresh(); }
		}
		wxWindow *button;
		button = wxWindow::FindWindowById(wxID_SAVE, this);
		if (button) button->Enable(true);
		button = wxWindow::FindWindowById(orpID_DEFAULT, this);
		if (button) button->Enable(true);
		button = wxWindow::FindWindowById(orpID_RESET, this);
		if (button) button->Enable(true);
	}
	else {
		for (int i = 0; i < sizeof(id_list); i++) {
			wxWindow *button = wxWindow::FindWindowById(id_list[i], this);
			if (button && id_list[i] != event.GetId()) {
				button->Enable(false); button->Refresh();
			}
		}
		wxWindow *button;
		button = wxWindow::FindWindowById(wxID_SAVE, this);
		if (button) button->Enable(false);
		button = wxWindow::FindWindowById(orpID_DEFAULT, this);
		if (button) button->Enable(false);
		button = wxWindow::FindWindowById(orpID_RESET, this);
		if (button) button->Enable(false);
		ctrl->Enable(true);
		ctrl->SetFocus();
	}
}

orpKeyBindErrorDialog::orpKeyBindErrorDialog(wxFrame *parent, wxWindowID button1, wxWindowID button2)
	: wxDialog(parent, wxID_ANY, wxString(_T("Key Bind Error!")))
{
	wxBoxSizer *frame_sizer = new wxBoxSizer(wxVERTICAL);

	wxBoxSizer *row_sizer;
	row_sizer = new wxBoxSizer(wxHORIZONTAL);
	row_sizer->Add(new wxStaticBitmap(this, wxID_ANY,
		wxArtProvider::GetBitmap(wxART_ERROR, wxART_CMN_DIALOG)), 0,
		wxALL, 10);

	wxBoxSizer *msg_sizer = NULL;
	wxBoxSizer *dup_sizer = NULL;
//	switch (result) {
//	case BIND_DUPLICATE:
		msg_sizer = new wxBoxSizer(wxVERTICAL);
		msg_sizer->Add(new wxStaticText(this, wxID_ANY, 
			_("Duplicate key binding detected:")),
			0, wxBOTTOM, 5);

		dup_sizer = new wxBoxSizer(wxHORIZONTAL);
		dup_sizer->Add(new orpPlayStationButton(this, button1));
		dup_sizer->Add(new wxStaticText(this, wxID_ANY,
			_T("is a duplicate of")),
			0, wxLEFT | wxRIGHT | wxALIGN_CENTER_VERTICAL, 5);
		dup_sizer->Add(new orpPlayStationButton(this, button2));

		msg_sizer->Add(dup_sizer);
		row_sizer->Add(msg_sizer, 0, wxTOP | wxRIGHT | wxBOTTOM, 10);
//		break;
//	default:
//		row_sizer->Add(new wxStaticText(this, wxID_ANY, 
//			_("Unknown error!")), 0, wxTOP | wxRIGHT | wxBOTTOM, 10);
//	}

	frame_sizer->Add(row_sizer);

	wxStdDialogButtonSizer *button_sizer = CreateStdDialogButtonSizer(wxOK);
	frame_sizer->Add(button_sizer, 0, wxALL | wxALIGN_RIGHT, 10);

	SetSizer(frame_sizer);
	frame_sizer->SetSizeHints(this);
	CenterOnParent();
}

// vi: ts=4
