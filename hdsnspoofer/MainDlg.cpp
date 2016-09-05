// MainDlg.cpp : implementation of the CMainDlg class
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include "funcs.h"
#include "MainDlg.h"

#ifdef DWMBLUR	//win7毛玻璃开关
#include <dwmapi.h>
#pragma comment(lib,"dwmapi.lib")
#endif

CMainDlg::CMainDlg() : SHostWnd(_T("LAYOUT:XML_MAINWND"))
    , originSN({0}), newSN({0})
{
    m_bLayoutInited = FALSE;
}

CMainDlg::~CMainDlg()
{
}

int CMainDlg::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
#ifdef DWMBLUR	//win7毛玻璃开关
    MARGINS mar = {5, 5, 30, 5};
    DwmExtendFrameIntoClientArea ( m_hWnd, &mar );
#endif

    SetMsgHandled(FALSE);
    return 0;
}

BOOL CMainDlg::OnInitDialog(HWND hWnd, LPARAM lParam)
{
    m_bLayoutInited = TRUE;
    InitWnd();

    return 0;
}


//TODO:消息映射
void CMainDlg::OnClose()
{
    CSimpleWnd::DestroyWindow();
}

void CMainDlg::OnMaximize()
{
    SendMessage(WM_SYSCOMMAND, SC_MAXIMIZE);
}

void CMainDlg::OnRestore()
{
    SendMessage(WM_SYSCOMMAND, SC_RESTORE);
}

void CMainDlg::OnMinimize()
{
    SendMessage(WM_SYSCOMMAND, SC_MINIMIZE);
}

void CMainDlg::InitWnd()
{
	serviceInstalled = IsServiceInstalled();
	if (serviceInstalled) {
		if (!GetSNInfo(originSN, newSN)) {
			SMessageBox(m_hWnd, L"获取硬盘序列号失败!", L"错误", MB_OK);
			OnClose();
		}

		SStatic* text;
		SWindow* wnd;
		SEdit* edit;
		TCHAR buf[SN_MAX_LEN + 1];

		if (originSN.count > 0) {
			for (int i = 0; i < SN_MAX_LEN; i+=2) {
				buf[i] = originSN.sn[0][i+1];
                buf[i+1] = originSN.sn[0][i];
			}
			buf[SN_MAX_LEN] = 0;
			text = FindChildByName2<SStatic>(L"hd0_raw");
			text->SetWindowText(buf);

			for (int i = 0; i < SN_MAX_LEN; i+=2) {
				buf[i] = newSN.sn[0][i+1];
                buf[i+1] = newSN.sn[0][i];
			}
			buf[SN_MAX_LEN] = 0;
			edit = FindChildByName2<SEdit>(L"hd0_new");
			edit->SetWindowText(buf);
		}

		if (originSN.count == 2) {
			for (int i = 0; i < SN_MAX_LEN; i+=2) {
				buf[i] = originSN.sn[1][i+1];
                buf[i+1] = originSN.sn[1][i];
			}
			buf[SN_MAX_LEN] = 0;
			text = FindChildByName2<SStatic>(L"hd1_raw");
			text->SetWindowText(buf);

			for (int i = 0; i < SN_MAX_LEN; i+=2) {
				buf[i] = newSN.sn[1][i+1];
                buf[i+1] = newSN.sn[1][i];
			}
			buf[SN_MAX_LEN] = 0;
			edit = FindChildByName2<SEdit>(L"hd1_new");
			edit->SetWindowText(buf);

			wnd = FindChildByName2<SWindow>(L"hd1");
			wnd->SetVisible(TRUE, TRUE);
		}
		else if (originSN.count > 2) {
			int n = originSN.count - 2;

			for (int m = 0; m < n; m++) {
				for (int i = 0; i < SN_MAX_LEN; i+=2) {
					buf[i] = originSN.sn[m + 2][i+1];
                    buf[i+1] = originSN.sn[m + 2][i];
				}
				buf[SN_MAX_LEN] = 0;
				SStringT ss;
				ss.Format(L"hd%d_raw", m + 2);
				text = FindChildByName2<SStatic>(ss);
				text->SetWindowText(buf);

				for (int i = 0; i < SN_MAX_LEN; i+=2) {
					buf[i] = newSN.sn[m + 2][i+1];
                    buf[i+1] = newSN.sn[m + 2][i];
				}
				buf[SN_MAX_LEN] = 0;
				ss.Format(L"hd%d_new", m + 2);
				edit = FindChildByName2<SEdit>(ss);
				edit->SetWindowText(buf);

				ss.Format(L"hd%d", m + 2);
				wnd = FindChildByName2<SWindow>(L"hd1");
				wnd->SetVisible(TRUE, TRUE);
			}

			CRect rct = GetWindowRect();
			SetWindowPos(NULL, 0, 0, rct.Width(), rct.Height() + 52 * n,
				SWP_NOZORDER | SWP_NOMOVE | SWP_NOSENDCHANGING | SWP_NOACTIVATE);
		}
		
		wnd = FindChildByName2<SWindow>(L"wnd_install");
		wnd->SetVisible(FALSE, TRUE);
		wnd = FindChildByName2<SWindow>(L"wnd_main");
		wnd->SetVisible(TRUE, TRUE);
	} else {
		SWindow* wnd;
		wnd = FindChildByName2<SWindow>(L"wnd_install");
		wnd->SetVisible(TRUE, TRUE);
		wnd = FindChildByName2<SWindow>(L"wnd_main");
		wnd->SetVisible(FALSE, TRUE);
	}
}

void CMainDlg::OnBtnInstall()
{
    if (serviceInstalled) {
        UninstallService();
		DeleteSys();
        SButton* btnInstall = FindChildByName2<SButton>(L"btn_install");
        btnInstall->SetWindowText(L"安装");
        serviceInstalled = FALSE;
    } else {
        if (!ReleaseSys() || !InstallService()) {
            SMessageBox(m_hWnd, L"驱动安装失败!", L"错误", MB_OK);
            OnClose();
            return;
        }
        SButton* btnInstall = FindChildByName2<SButton>(L"btn_install");
        btnInstall->SetWindowText(L"卸载");
        serviceInstalled = TRUE;
    }

	InitWnd();
}

void CMainDlg::OnBtnSpoof()
{
    int cnt = originSN.count;
    GenRandomSN(cnt, newSN);

	for (int i = 0; i < cnt; i++)
	{
		TCHAR buf[SN_MAX_LEN + 1];
		for (int j=0; j < SN_MAX_LEN; j+=2)
		{
			buf[j] = newSN.sn[i][j+1];
            buf[j+1] = newSN.sn[i][j];
		}
		buf[SN_MAX_LEN] = 0;

		SStringT str;
		str.Format(L"hd%d_new", i);
		SEdit* edit = FindChildByName2<SEdit>(str);
		edit->SetWindowText(buf);
	}

    if (SpoofHDSN(originSN, newSN)) {
        SMessageBox(m_hWnd, L"修改硬盘序列号成功!", L"完成", MB_OK);
    } else {
        SMessageBox(m_hWnd, L"修改硬盘序列号失败!", L"错误", MB_OK);
    }
}

void CMainDlg::OnBtnApply()
{
    SStringT str;
    TCHAR buf[64];
    for (int i = 0; i < originSN.count; i++) {
        str.Format(L"hd%d_new", i);
        SEdit* edit = FindChildByName2<SEdit>(str);
        SStringT text = edit->GetWindowText();
		for (int j = 0; j < SN_MAX_LEN; j++)
		{
			buf[j] = ' ';
		}
        memcpy(buf, text.GetBuffer(0), sizeof(TCHAR)*text.GetLength());
        for (int j = 0; j < SN_MAX_LEN; j+=2) {
            newSN.sn[i][j] = buf[j+1];
            newSN.sn[i][j+1] = buf[j];
        }
		newSN.sn[i][SN_MAX_LEN] = 0;
    }
    newSN.count = originSN.count;
    if (SpoofHDSN(originSN, newSN)) {
        SMessageBox(m_hWnd, L"修改硬盘序列号成功!", L"完成", MB_OK);
    } else {
        SMessageBox(m_hWnd, L"修改硬盘序列号失败!", L"错误", MB_OK);
    }
}

bool CMainDlg::ReleaseSys()
{
	BOOL bIsWow64 = FALSE;
	typedef BOOL(WINAPI *LPFN_ISWOW64PROCESS) (HANDLE, PBOOL);
	LPFN_ISWOW64PROCESS fnIsWow64Process = NULL;
	fnIsWow64Process = (LPFN_ISWOW64PROCESS)GetProcAddress(
		GetModuleHandle(TEXT("kernel32")), "IsWow64Process");
	if (fnIsWow64Process != NULL) {
		fnIsWow64Process(GetCurrentProcess(), &bIsWow64);
	}
	SStringT rstr = L"ID_BIN_32SYS";
	if (bIsWow64) {
		rstr = L"ID_BIN_64SYS";
	}
	SApplication *theApp = SApplication::getSingletonPtr();
	size_t sz = theApp->GetRawBufferSize(L"BIN", rstr);
	if (sz == 0) {
		return false;
	}
	
	SStringT sys = L"%windir%\\system32\\drivers\\snspoofer.sys";
	if (bIsWow64) {
		sys = L"%windir%\\Sysnative\\drivers\\snspoofer.sys";
	}

	TCHAR sysbuf[MAX_PATH];
	if (!ExpandEnvironmentStrings(sys, sysbuf, MAX_PATH)) {
		return false;
	}

	HANDLE hFile = CreateFile(sysbuf, GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
		NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (INVALID_HANDLE_VALUE == hFile) {
		return false;
	}

	char* buf = new char[sz];
	theApp->GetRawBuffer(L"BIN", rstr, buf, sz);
	DWORD cb;
	if (!WriteFile(hFile, buf, sz, &cb, NULL) || cb != sz) {
		CloseHandle(hFile);
		return false;
	}
	CloseHandle(hFile);
	return true;
}

bool CMainDlg::DeleteSys()
{
	BOOL bIsWow64 = FALSE;
	typedef BOOL(WINAPI *LPFN_ISWOW64PROCESS) (HANDLE, PBOOL);
	LPFN_ISWOW64PROCESS fnIsWow64Process = NULL;
	fnIsWow64Process = (LPFN_ISWOW64PROCESS)GetProcAddress(
		GetModuleHandle(TEXT("kernel32")), "IsWow64Process");
	if (fnIsWow64Process != NULL) {
		fnIsWow64Process(GetCurrentProcess(), &bIsWow64);
	}

	SStringT sys = L"%windir%\\system32\\drivers\\snspoofer.sys";
	if (bIsWow64) {
		sys = L"%windir%\\Sysnative\\drivers\\snspoofer.sys";
	}

	TCHAR sysbuf[MAX_PATH];
	if (!ExpandEnvironmentStrings(sys, sysbuf, MAX_PATH)) {
		return false;
	}

	return DeleteFile(sysbuf) == TRUE;
}
