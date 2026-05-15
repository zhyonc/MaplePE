// MainView.cpp : implementation file
//
#include "pch.h"
#include "MainView.h"
#include "FormatView.h"
#include "ColumnStyle.h"
#include "resource.h"

namespace {
	const size_t kBytesPerLine = 40;
}

// MainView dialog

IMPLEMENT_DYNAMIC(MainView, CDialogEx)

MainView::MainView(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_MAIN_DIALOG, pParent), BaseView(L"MainView")
{
	m_hIcon = AfxGetApp()->LoadIcon(IDI_MAIN_ICON);
	m_mainController = new MainController(this);
	m_settingView = new SettingView(m_mainController, this);
	m_jumpView = new JumpView(m_mainController, this);
	m_patchView = new PatchView(m_mainController, this);
}


MainView::~MainView()
{
	delete(m_settingView);
	delete(m_jumpView);
	delete(m_patchView);
	delete(m_mainController);
	m_settingView = nullptr;
	m_jumpView = nullptr;
	m_patchView = nullptr;
	m_mainController = nullptr;
}

void MainView::ImportPacketLog()
{
	try {
		CFileDialog dlg(TRUE, _T("json"), NULL,
			OFN_FILEMUSTEXIST | OFN_HIDEREADONLY,
			_T("JSON Files (*.json)|*.json|All Files (*.*)|*.*||"));

		if (dlg.DoModal() == IDOK) {
			CString cpath = dlg.GetPathName();
			std::wstring path(cpath.GetString());
			m_mainController->LoadPacketLogFromJSON(path);
		}
	}
	catch (const std::exception& e) {
		std::string errorMsg = e.what();
		std::wstring wErrorMsg = PacketScript::MultiByte2WideChar(errorMsg.c_str(), errorMsg.size());
		MBError(wErrorMsg);
	}
}

void MainView::ExportPacketLog()
{
	try {
		SYSTEMTIME st;
		GetLocalTime(&st);

		CString timestamp;
		timestamp.Format(_T("%04d%02d%02d_%02d%02d%02d"),
			st.wYear, st.wMonth, st.wDay,
			st.wHour, st.wMinute, st.wSecond);

		CString defaultName;
		defaultName.Format(_T("PacketLog_%s.json"), timestamp.GetString());

		CFileDialog dlg(FALSE, _T("json"), defaultName,
			OFN_OVERWRITEPROMPT,
			_T("JSON Files (*.json)|*.json|All Files (*.*)|*.*||"));

		if (dlg.DoModal() == IDOK) {
			CString cpath = dlg.GetPathName();
			std::wstring path(cpath.GetString());

			std::vector<int> selectedIndices;
			POSITION pos = m_packetLogListCtrl.GetFirstSelectedItemPosition();
			while (pos != NULL)
			{
				int nSelectedIndex = m_packetLogListCtrl.GetNextSelectedItem(pos);
				UINT state = m_packetLogListCtrl.GetItemState(nSelectedIndex, LVIS_FOCUSED);
				if (state != 0) {
					selectedIndices.push_back(nSelectedIndex);
				}
			}
			m_mainController->SavePacketLogToJSON(path, selectedIndices);
		}
	}
	catch (const std::exception& e) {
		std::string errorMsg = e.what();
		std::wstring wErrorMsg = PacketScript::MultiByte2WideChar(errorMsg.c_str(), errorMsg.size());
		MBError(wErrorMsg);
	}
}

void MainView::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_PACKET_LOG_LIST, m_packetLogListCtrl);
	DDX_Control(pDX, IDC_DATA_DETAIL_EDIT, m_dataDetailEdit);
	DDX_Control(pDX, IDC_PID_EDIT, m_pidEdit);
	DDX_Control(pDX, IDC_IS_INPACKET_CHECK, m_isInPacketCheck);
}

BOOL MainView::OnInitDialog()
{
	CDialogEx::OnInitDialog();
	this->SetHWND(this->GetSafeHwnd());
	// Add menu item
	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != nullptr) {
		pSysMenu->AppendMenu(MF_SEPARATOR);
		pSysMenu->AppendMenu(MF_STRING, IDM_IMPORT_MENU, L"Import");
		pSysMenu->AppendMenu(MF_STRING, IDM_EXPORT_MENU, L"Export");
		pSysMenu->AppendMenu(MF_STRING, IDM_SETTING_MENU, L"Setting");
		pSysMenu->AppendMenu(MF_STRING, IDM_SEARCH_MENU, L"Search\tCtrl+F");
		pSysMenu->AppendMenu(MF_STRING, IDM_JUMP_MENU, L"Jump\tCtrl+G");
	}
	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);
	// Set extended styles:
	// - LVS_EX_FULLROWSELECT: Selecting any column highlights the row
	// - LVS_EX_GRIDLINES: Displays grid lines between rows and columns for better readability
	m_packetLogListCtrl.SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);
	// Set column title and width
	CRect dlgClientRect;
	GetClientRect(&dlgClientRect);
	int totalWidth = dlgClientRect.Width();
	std::vector<int> columnWidths = GetColumnWidths(totalWidth, kLogColumnWidthRatio);
	for (size_t i = 0; i < columnWidths.size(); i++)
	{
		m_packetLogListCtrl.InsertColumn(static_cast<int>(i), kLogColumnTitle[i].c_str(), LVCFMT_LEFT, columnWidths[i]);
	}
	// Set "PID" as default value
	m_pidEdit.SendMessageW(EM_SETCUEBANNER, 0, (LPARAM)_T("PID"));
	return TRUE;
}

void MainView::InsertPacketLogItem(size_t id, const PacketLogModel& log)
{
	int nItem = m_packetLogListCtrl.GetItemCount();
	m_packetLogListCtrl.InsertItem(nItem, std::to_wstring(id).c_str());
	m_packetLogListCtrl.SetItemText(nItem, static_cast<int>(kLogColumnType::PID), log.GetPIDStr().c_str());
	m_packetLogListCtrl.SetItemText(nItem, static_cast<int>(kLogColumnType::Index), log.GetIndexStr().c_str());
	m_packetLogListCtrl.SetItemText(nItem, static_cast<int>(kLogColumnType::Type), log.GetType().c_str());
	m_packetLogListCtrl.SetItemText(nItem, static_cast<int>(kLogColumnType::Length), log.GetLengthStr().c_str());
	m_packetLogListCtrl.SetItemText(nItem, static_cast<int>(kLogColumnType::Opcode), log.GetOpcodeStr().c_str());
	m_packetLogListCtrl.SetItemText(nItem, static_cast<int>(kLogColumnType::Data), log.GetData().c_str());
}

void MainView::JumpLogItem(int logID) {
	int selected = m_packetLogListCtrl.GetNextItem(-1, LVNI_SELECTED);
	if (selected != -1 && selected != logID) {
		m_packetLogListCtrl.SetItemState(selected, 0, LVIS_SELECTED | LVIS_FOCUSED);
	}
	m_packetLogListCtrl.EnsureVisible(logID, FALSE);
	m_packetLogListCtrl.SetItemState(logID, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
	m_packetLogListCtrl.SetFocus();
}

void MainView::CreateSearchView()
{
	SearchView* pSearchView = new SearchView(m_mainController, this);
	pSearchView->Create(IDD_SEARCH_DIALOG, this);
	pSearchView->ShowWindow(SW_SHOW);
}


void MainView::CreateFormatView(int nSelectedID)
{
	FormatView* pFormatView = new FormatView(nSelectedID, m_mainController, this);
	pFormatView->Create(IDD_FORMAT_DIALOG, this);
	pFormatView->ShowWindow(SW_SHOW);
}

BEGIN_MESSAGE_MAP(MainView, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_BN_CLICKED(IDC_PATCH_BUTTON, &MainView::OnBnClickedPatchButton)
	ON_BN_CLICKED(IDC_EDIT_BUTTON, &MainView::OnBnClickedEditButton)
	ON_BN_CLICKED(IDC_ClEAR_BUTTON, &MainView::OnBnClickedClearButton)
	ON_NOTIFY(NM_DBLCLK, IDC_PACKET_LOG_LIST, &MainView::OnNMDblclkPacketLogList)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_PACKET_LOG_LIST, &MainView::OnLvnItemchangedPacketLogList)
	ON_BN_CLICKED(IDC_MAIN_SEND_BUTTON, &MainView::OnBnClickedSendButton)
END_MESSAGE_MAP()

// MainView message handlers

void MainView::OnSysCommand(UINT nID, LPARAM lParam) {
	switch (nID) {
	case IDM_IMPORT_MENU:
		this->ImportPacketLog();
		break;
	case IDM_EXPORT_MENU:
		this->ExportPacketLog();
		break;
	case IDM_SETTING_MENU: {
		m_settingView->DoModal();
		break;
	}
	case IDM_SEARCH_MENU: {
		this->CreateSearchView();
		break;
	}
	case IDM_JUMP_MENU: {
		m_jumpView->DoModal();
		break;
	}
	default:
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

BOOL MainView::PreTranslateMessage(MSG* pMsg)
{
	if (pMsg->message == WM_KEYDOWN)
	{
		if (pMsg->wParam == VK_RETURN) {
			// Avoid the dialog is closed by enter key
			return TRUE;
		}
		if (GetKeyState(VK_CONTROL) & 0x8000 && pMsg->wParam == 'G') {
			m_jumpView->DoModal();
			return TRUE;
		}
		if (GetKeyState(VK_CONTROL) & 0x8000 && pMsg->wParam == 'F')
		{
			this->CreateSearchView();
			return TRUE;
		}
	}
	return CDialogEx::PreTranslateMessage(pMsg);
}

void MainView::OnCancel() {
	if (MBYesNo(L"Exit?")) {
		CDialogEx::OnCancel();
	}
}

void MainView::OnNMDblclkPacketLogList(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	// TODO: Add your control notification handler code here
	int nSelectedIndex = pNMItemActivate->iItem;
	if (nSelectedIndex == -1) {
		MBError(L"Please select one row");
		return;
	}
	int nSelectedID = _wtoi(m_packetLogListCtrl.GetItemText(nSelectedIndex, 0));
	this->CreateFormatView(nSelectedID);
	*pResult = 0;
}


void MainView::OnBnClickedPatchButton()
{
	// TODO: Add your control notification handler code here
	if (m_patchView != nullptr) {
		m_patchView->DoModal();
	}
}

void MainView::OnBnClickedEditButton()
{
	// TODO: Add your control notification handler code here
	int nSelectedIndex = m_packetLogListCtrl.GetNextItem(-1, LVNI_SELECTED);
	if (nSelectedIndex == -1)
	{
		MBError(L"Please select one row");
		return;
	}
	int nSelectedID = _wtoi(m_packetLogListCtrl.GetItemText(nSelectedIndex, 0));
	this->CreateFormatView(nSelectedID);
}

void MainView::OnBnClickedClearButton()
{
	// TODO: Add your control notification handler code here
	if (MBYesNo(L"Clear all logs?")) {
		m_mainController->ClearPacketLogModel();
		m_packetLogListCtrl.DeleteAllItems();
	}
}

void MainView::OnLvnItemchangedPacketLogList(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
	// TODO: Add your control notification handler code here

	if ((pNMLV->uChanged & LVIF_STATE) &&
		!(pNMLV->uOldState & LVIS_SELECTED) &&
		(pNMLV->uNewState & LVIS_SELECTED))
	{
		int nItem = pNMLV->iItem;
		// Data
		CString text;
		CString data = m_packetLogListCtrl.GetItemText(nItem, static_cast<int>(kLogColumnType::Data));
		size_t dataLength = data.GetLength();
		size_t lineLength = kBytesPerLine * 3;
		for (size_t i = 0; i < dataLength; i += lineLength)
		{
			text += data.Mid(static_cast<int>(i), static_cast<int>(lineLength));
			if (i + lineLength < dataLength) {
				text += "\r\n";
			}
		}
		m_dataDetailEdit.SetWindowTextW(text);
		// PID
		CString pid = m_packetLogListCtrl.GetItemText(nItem, static_cast<int>(kLogColumnType::PID));
		m_pidEdit.SetWindowTextW(pid);
		// IsInPacket
		CString type = m_packetLogListCtrl.GetItemText(nItem, static_cast<int>(kLogColumnType::Type));
		std::wstring typeW(type.GetString());
		m_isInPacketCheck.SetCheck(typeW == L"In");
	}
	*pResult = 0;
}

void MainView::OnBnClickedSendButton()
{
	// TODO: Add your control notification handler code here
	// Data
	CString dataInput;
	m_dataDetailEdit.GetWindowTextW(dataInput);
	if (dataInput.GetLength() == 0) {
		MBError(L"Unable send empty data");
		return;
	}
	dataInput.Replace(_T("\r\n"), _T(""));
	dataInput.Replace(_T("\n"), _T(""));
	std::wstring data(dataInput);
	// PID
	CString pidInput;
	m_pidEdit.GetWindowTextW(pidInput);
	int pid = _wtoi(pidInput);
	// IsInPacket
	bool isInPacket = m_isInPacketCheck.GetCheck();
	std::wstring err = m_mainController->SendData(pid, isInPacket, data);
	if (!err.empty()) {
		MBError(err);
		return;
	}
	MBInfo(L"Send data ok");
}