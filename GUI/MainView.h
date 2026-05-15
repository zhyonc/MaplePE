#pragma once
#include "Model.h"
#include "BaseView.h"
#include "SettingView.h"
#include "SearchView.h"
#include "JumpView.h"
#include "PatchView.h"
#include "MainController.h"

// MainView dialog

class MainView final : public CDialogEx, public BaseView
{
	DECLARE_DYNAMIC(MainView)

public:
	MainView(CWnd* pParent = nullptr);   // standard constructor
	void InsertPacketLogItem(size_t id, const PacketLogModel& log);
	void JumpLogItem(int logID);
	~MainView();

private:
	MainController* m_mainController = nullptr;
	SettingView* m_settingView = nullptr;
	JumpView* m_jumpView = nullptr;
	PatchView* m_patchView = nullptr;

	// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_MAIN_DIALOG };
#endif

	void ImportPacketLog();
	void ExportPacketLog();

protected:
	HICON m_hIcon = nullptr;
	CListCtrl m_packetLogListCtrl;
	CEdit m_dataDetailEdit;
	CEdit m_pidEdit;
	CButton m_isInPacketCheck;

	void DoDataExchange(CDataExchange* pDX) override;    // DDX/DDV support
	BOOL OnInitDialog() override;
	void CreateSearchView();
	void CreateFormatView(int nSelectedID);
	// Generated message map functions
	DECLARE_MESSAGE_MAP()

public:
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg void OnNMDblclkPacketLogList(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnCancel();
	afx_msg void OnBnClickedPatchButton();
	afx_msg void OnBnClickedEditButton();
	afx_msg void OnBnClickedClearButton();
	afx_msg void OnLvnItemchangedPacketLogList(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnBnClickedSendButton();
};
