
// ChatClient.cpp: 애플리케이션에 대한 클래스 동작을 정의합니다.
//

#include "pch.h"
#include "framework.h"
#include "ChatClient.h"
#include "ChatClientDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CChatClientApp

BEGIN_MESSAGE_MAP(CChatClientApp, CWinApp)
	ON_COMMAND(ID_HELP, &CWinApp::OnHelp)
END_MESSAGE_MAP()


// CChatClientApp 생성

CChatClientApp::CChatClientApp()
{
	// TODO: 여기에 생성 코드를 추가합니다.
	// InitInstance에 모든 중요한 초기화 작업을 배치합니다.
}


// 유일한 CChatClientApp 개체입니다.

CChatClientApp theApp;


// CChatClientApp 초기화

BOOL CChatClientApp::InitInstance()
{
	CWinApp::InitInstance();

	WSADATA temp;
	WSAStartup(0x0202, &temp);

	CChatClientDlg dlg;
	m_pMainWnd = &dlg;
	dlg.DoModal();
	
	WSACleanup();

	return FALSE;
}

