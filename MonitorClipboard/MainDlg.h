// MainDlg.h : interface of the CMainDlg class
//
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include <string>

// 自定义消息
// 获取剪切板内容
#define WM_CUSTOM_GET_CLIPBOARD WM_USER + 1

class CMainDlg : public CDialogImpl<CMainDlg>, public CUpdateUI<CMainDlg>,
		public CMessageFilter, public CIdleHandler
{
public:
	enum { IDD = IDD_MAINDLG };

	virtual BOOL PreTranslateMessage(MSG* pMsg)
	{
		return CWindow::IsDialogMessage(pMsg);
	}

	virtual BOOL OnIdle()
	{
		UIUpdateChildWindows();
		return FALSE;
	}

	BEGIN_UPDATE_UI_MAP(CMainDlg)
	END_UPDATE_UI_MAP()

	BEGIN_MSG_MAP(CMainDlg)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
		MESSAGE_HANDLER(WM_CHANGECBCHAIN, OnChangeCBChain)
		MESSAGE_HANDLER(WM_DRAWCLIPBOARD, OnDrawClipboard)
		MESSAGE_HANDLER(WM_CUSTOM_GET_CLIPBOARD, OnCustomGetClipboard)
		COMMAND_ID_HANDLER(ID_APP_ABOUT, OnAppAbout)
		COMMAND_ID_HANDLER(IDOK, OnOK)
		COMMAND_ID_HANDLER(IDCANCEL, OnCancel)
		COMMAND_HANDLER(IDC_BUTTON_START, BN_CLICKED, OnBnClickedButtonStart)
	END_MSG_MAP()

// Handler prototypes (uncomment arguments if needed):
//	LRESULT MessageHandler(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
//	LRESULT CommandHandler(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
//	LRESULT NotifyHandler(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/)

	LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	{
		// center the dialog on the screen
		CenterWindow();

		// set icons
		HICON hIcon = AtlLoadIconImage(IDR_MAINFRAME, LR_DEFAULTCOLOR, ::GetSystemMetrics(SM_CXICON), ::GetSystemMetrics(SM_CYICON));
		SetIcon(hIcon, TRUE);
		HICON hIconSmall = AtlLoadIconImage(IDR_MAINFRAME, LR_DEFAULTCOLOR, ::GetSystemMetrics(SM_CXSMICON), ::GetSystemMetrics(SM_CYSMICON));
		SetIcon(hIconSmall, FALSE);

		// register object for message filtering and idle updates
		CMessageLoop* pLoop = _Module.GetMessageLoop();
		ATLASSERT(pLoop != NULL);
		pLoop->AddMessageFilter(this);
		pLoop->AddIdleHandler(this);

		UIAddChildWindowContainer(m_hWnd);

		//////////////////////////////////////////////////////////////////////////
		m_hwndNextViewer = nullptr;
		m_btnStart.Attach(GetDlgItem(IDC_BUTTON_START));
		m_listContent.Attach(GetDlgItem(IDC_LIST_HISTEXT));

		return TRUE;
	}

	LRESULT OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	{
		// unregister message filtering and idle updates
		CMessageLoop* pLoop = _Module.GetMessageLoop();
		ATLASSERT(pLoop != NULL);
		pLoop->RemoveMessageFilter(this);
		pLoop->RemoveIdleHandler(this);

		return 0;
	}


	LRESULT OnChangeCBChain(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/)
	{
		if ((HWND)wParam == m_hwndNextViewer) {
			m_hwndNextViewer = (HWND)lParam;
		} else if (m_hwndNextViewer) {
			SendMessage(m_hwndNextViewer, uMsg, wParam, lParam);
		}
		OutputDebugString(L"11");
		return 0;
	}

	LRESULT OnDrawClipboard(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/)
	{
		if (m_hwndNextViewer) {
			SendMessage(m_hwndNextViewer, uMsg, wParam, lParam);
		}
		PostMessage(WM_CUSTOM_GET_CLIPBOARD);
		return 0;
	}

	LRESULT OnCustomGetClipboard(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/)
	{
		if (!OpenClipboard()) {
			return 0;
		}

		HANDLE handle = GetClipboardData(CF_UNICODETEXT);
		if (handle) {
			LPTSTR content = (LPTSTR)GlobalLock(handle);
			if (content) {
				OutputDebugString(content);
				m_listContent.InsertString(0, content);
				GlobalUnlock(handle);
			}
		}

		CloseClipboard();
		return 0;
	}
	
	LRESULT OnAppAbout(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		//CAboutDlg dlg;
		//dlg.DoModal();
		return 0;
	}

	LRESULT OnOK(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		// TODO: Add validation code 
		CloseDialog(wID);
		return 0;
	}

	LRESULT OnCancel(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		CloseDialog(wID);
		return 0;
	}

	void CloseDialog(int nVal)
	{
		DestroyWindow();
		::PostQuitMessage(nVal);
	}
	LRESULT OnBnClickedButtonStart(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		TCHAR buf[1024] = { 0 };
		m_btnStart.GetWindowText(buf, sizeof(buf) / sizeof(buf[0]));
		std::wstring wstr(buf);
		if (wstr == L"Start") {
			m_hwndNextViewer = SetClipboardViewer();
			m_btnStart.SetWindowText(L"Stop");
		} else {
			ChangeClipboardChain(m_hwndNextViewer);
			m_btnStart.SetWindowText(L"Start");
		}
		
		return 0;
	}

private:
	HWND m_hwndNextViewer;
	CButton m_btnStart;
	CListBox m_listContent;
};
