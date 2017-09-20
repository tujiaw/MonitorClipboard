// MainDlg.h : interface of the CMainDlg class
//
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include <string>
#include <sstream>
#include "HookEvent.h"
#include "Constant.h"

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
		m_staticMouse.Attach(GetDlgItem(IDC_STATIC_MOUSEPOS));
		m_editKeyboard.Attach(GetDlgItem(IDC_EDIT_KEYBOARD));
		HookEvent::instance()->addMessageCallback(WM_MOUSEMOVE,
			std::bind(&CMainDlg::OnMouseMoveHook, this, std::placeholders::_1, std::placeholders::_2));
		HookEvent::instance()->addMessageCallback(WM_KEYDOWN,
			std::bind(&CMainDlg::OnKeyDownHook, this, std::placeholders::_1, std::placeholders::_2));
		return TRUE;
	}

	LRESULT OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	{
		// unregister message filtering and idle updates
		CMessageLoop* pLoop = _Module.GetMessageLoop();
		ATLASSERT(pLoop != NULL);
		pLoop->RemoveMessageFilter(this);
		pLoop->RemoveIdleHandler(this);
		HookEvent::instance()->clearMessageCallback();
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

		int count = m_listContent.GetCount();
		if (count > 3000) {
			m_listContent.DeleteString(count - 1);
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
			HookEvent::instance()->start();
			m_btnStart.SetWindowText(L"Stop");
		} else {
			ChangeClipboardChain(m_hwndNextViewer);
			HookEvent::instance()->stop();
			m_btnStart.SetWindowText(L"Start");
		}
		
		return 0;
	}

	void OnMouseMoveHook(WPARAM wParam, LPARAM lParam)
	{
		MOUSEHOOKSTRUCT *p = (MOUSEHOOKSTRUCT*)lParam;
		if (p) {
			std::wstringstream wstr;
			wstr << "x:" << p->pt.x << " ,y:" << p->pt.y;
			m_staticMouse.SetWindowText(wstr.str().c_str());
		}
	}

	void OnKeyDownHook(WPARAM wParam, LPARAM lParam)
	{
		KBDLLHOOKSTRUCT *p = (KBDLLHOOKSTRUCT*)lParam;
		if (p) {
			wchar_t buff[10], oldBuff[10240 + 2];
			BYTE keyState[256] = { 0 };
			int result = ToUnicodeEx(p->vkCode, p->scanCode, keyState, buff, _countof(buff), 0, NULL);
			m_editKeyboard.GetWindowText(oldBuff, _countof(oldBuff));
			std::wstring newStr = std::wstring(oldBuff) + std::wstring(buff);
			if (newStr.size() >= 10240) {
				newStr = newStr.substr(1024);
			}
			m_editKeyboard.SetWindowText(newStr.c_str());
		}
	}

private:
	HWND m_hwndNextViewer;
	CButton m_btnStart;
	CListBox m_listContent;
	CStatic m_staticMouse;
	CEdit m_editKeyboard;
};
