
// ChatClientDlg.cpp: 구현 파일
//

#include "pch.h"
#include "framework.h"
#include "ChatClient.h"
#include "ChatClientDlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CChatClientDlg 대화 상자



CChatClientDlg::CChatClientDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_CHATCLIENT_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CChatClientDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST1, m_event_list);
}

BEGIN_MESSAGE_MAP(CChatClientDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_SEND_BTN, &CChatClientDlg::OnBnClickedSendBtn)
	ON_BN_CLICKED(IDOK, &CChatClientDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDC_CONNECT_BTN, &CChatClientDlg::OnBnClickedConnectBtn)
END_MESSAGE_MAP()


void CChatClientDlg::AddEventString(CString parm_string) {
	int index = m_event_list.InsertString(-1, parm_string);
	m_event_list.SetCurSel(index);
}

// CChatClientDlg 메시지 처리기

BOOL CChatClientDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 이 대화 상자의 아이콘을 설정합니다.  응용 프로그램의 주 창이 대화 상자가 아닐 경우에는
	//  프레임워크가 이 작업을 자동으로 수행합니다.
	SetIcon(m_hIcon, TRUE);			// 큰 아이콘을 설정합니다.
	SetIcon(m_hIcon, FALSE);		// 작은 아이콘을 설정합니다.

	// TODO: 여기에 추가 초기화 작업을 추가합니다.

	return TRUE;  // 포커스를 컨트롤에 설정하지 않으면 TRUE를 반환합니다.
}

// 대화 상자에 최소화 단추를 추가할 경우 아이콘을 그리려면
//  아래 코드가 필요합니다.  문서/뷰 모델을 사용하는 MFC 애플리케이션의 경우에는
//  프레임워크에서 이 작업을 자동으로 수행합니다.

void CChatClientDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 그리기를 위한 디바이스 컨텍스트입니다.

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 클라이언트 사각형에서 아이콘을 가운데에 맞춥니다.
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 아이콘을 그립니다.
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

// 사용자가 최소화된 창을 끄는 동안에 커서가 표시되도록 시스템에서
//  이 함수를 호출합니다.
HCURSOR CChatClientDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CChatClientDlg::ConnectProcess(LPARAM lParam) {
	if (WSAGETSELECTERROR(lParam) == 0) {
		//접속 성공
		WSAAsyncSelect(mh_socket, m_hWnd, 27002, FD_READ | FD_CLOSE);
		AddEventString(L"서버에 접속했습니다.");
		GetDlgItem(IDC_IP_EDIT)->EnableWindow(FALSE);
		GetDlgItem(IDC_CONNECT_BTN)->EnableWindow(FALSE);
	}
	else {
		//접속 실패
		DestroySocket();
		AddEventString(L"서버에 접속할 수 없습니다.");
	}
}

void CChatClientDlg::DestroySocket() {
	LINGER temp_linger = { TRUE, 0 };
	setsockopt(mh_socket, SOL_SOCKET, SO_LINGER, (char*)&temp_linger, sizeof(temp_linger));

	closesocket(mh_socket);
	mh_socket = INVALID_SOCKET;
}

void CChatClientDlg::ReveiveData(char* parm_p_buffer, int parm_size) {
	int current_size, total_size = 0, retry_count = 0;

	while (total_size < parm_size) {
		current_size = recv(mh_socket, parm_p_buffer + total_size, parm_size - total_size, 0);

		if (current_size == SOCKET_ERROR) {
			retry_count++;
			Sleep(50);
			if (retry_count > 5) {
				break;
			}
		}
		else {
			retry_count = 0;
			total_size = total_size + current_size;
		}
	}
}

void CChatClientDlg::ReadFrameData() {
	char key, message_id;
	recv(mh_socket, &key, 1, 0);
	if (key == 27) {
		unsigned short int body_size;
		recv(mh_socket, (char*)&body_size, 2, 0);
		recv(mh_socket, &message_id, 1, 0);
		if (body_size > 0) {
			char* p_body_data = new char[body_size];

			ReveiveData(p_body_data, body_size);

			if (message_id == 1) {
				AddEventString((wchar_t*)p_body_data);
			}

			delete[] p_body_data;
		}
	}
	else {
		DestroySocket();
		AddEventString(L"잘못된 프로토콜입니다.");
	}
}

LRESULT CChatClientDlg::WindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
	// TODO: 여기에 특수화된 코드를 추가 및/또는 기본 클래스를 호출합니다.

	if (message == 27001) { //FD_CONNECT
		ConnectProcess(lParam);
	}
	else if(message == 27002) { //FD_READ or FD_CLOSE
		if (WSAGETSELECTEVENT(lParam) == FD_READ) {
			WSAAsyncSelect(mh_socket, m_hWnd, 27002, FD_CLOSE);
			
			ReadFrameData();

			if (mh_socket != INVALID_SOCKET) {
				WSAAsyncSelect(mh_socket, m_hWnd, 27002, FD_CLOSE | FD_READ);
			}
		}
		else {
			DestroySocket();
			AddEventString(L"서버에서 연결을 해제하였습니다.");
		}
	}

	return CDialogEx::WindowProc(message, wParam, lParam);
}

void CChatClientDlg::SendFrameData(SOCKET parm_h_socket, unsigned char parm_id, const void* parm_p_data, int parm_size) {
	char* p_send_data = new char[parm_size + 4];

	*p_send_data = 27;
	*(unsigned short int*)(p_send_data + 1) = parm_size;
	*(p_send_data + 3) = parm_id;

	memcpy(p_send_data + 4, parm_p_data, parm_size);

	send(parm_h_socket, p_send_data, parm_size + 4, 0);

	delete[] p_send_data;
}

void CChatClientDlg::OnBnClickedSendBtn()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	
	if (mh_socket != INVALID_SOCKET) {
		CString str;
		GetDlgItemText(IDC_EDIT1, str);

		SendFrameData(mh_socket, 1, (const wchar_t*)str, (str.GetLength() + 1) * 2);
		SetDlgItemText(IDC_EDIT1, L"");
		GotoDlgCtrl(GetDlgItem(IDC_EDIT1));
	}
	else {
		AddEventString(L"서버에 접속된 상태가 아닙니다.");
	}
}


void CChatClientDlg::OnBnClickedOk()
{
	OnBnClickedSendBtn();
	
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	//CDialogEx::OnOK();
}


void CChatClientDlg::OnBnClickedConnectBtn()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.

	CString str;
	GetDlgItemText(IDC_IP_EDIT, str);

	if (!str.IsEmpty()) {
		AddEventString(L"서버에 접속을 시도합니다. : " + str);
		mh_socket = socket(AF_INET, SOCK_STREAM, 0);

		sockaddr_in srv_addr;
		srv_addr.sin_family = AF_INET;
		srv_addr.sin_addr.s_addr = inet_addr((CStringA)str);
		srv_addr.sin_port = htons(20001);

		WSAAsyncSelect(mh_socket, m_hWnd, 27001, FD_CONNECT);
		connect(mh_socket, (sockaddr*)&srv_addr, sizeof(srv_addr));
	}
}


BOOL CChatClientDlg::PreTranslateMessage(MSG* pMsg)
{
	// TODO: 여기에 특수화된 코드를 추가 및/또는 기본 클래스를 호출합니다.
	if (WM_KEYDOWN == pMsg->message)
	{
		if (VK_ESCAPE == pMsg->wParam)
		{
			return TRUE;
		}
	}
	return CDialogEx::PreTranslateMessage(pMsg);
}
