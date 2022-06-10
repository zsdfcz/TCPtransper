#define _CRT_SECURE_NO_WARNINGS         // 최신 VC++ 컴파일 시 경고 방지
#define _WINSOCK_DEPRECATED_NO_WARNINGS // 최신 VC++ 컴파일 시 경고 방지
#pragma comment(lib, "ws2_32")
#include <winsock2.h>
#include <stdlib.h>
#include <stdio.h>
#include "resource.h"
#define SERVERPORT 9000
#define SERVERPORT2 9001
#define BUFSIZE    512


// 윈도우 프로시저
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
//대화상자 프로시저
BOOL CALLBACK DlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
// 편집 컨트롤 출력 함수
void DisplayText(char *fmt, ...);
// 오류 출력 함수
void err_quit(char *msg);
void err_display(char *msg);
int keyset(char* abc, int len);
int compute(int a, int m, int n);
// 소켓 통신 스레드 함수
DWORD WINAPI ServerMain(LPVOID arg);
DWORD WINAPI ProcessClient(LPVOID arg);
DWORD WINAPI keyClient(LPVOID arg);
char buf[BUFSIZE + 1]; // 데이터 송수신 버퍼
char key[BUFSIZE + 1]; // 키값 버퍼
HINSTANCE hInst; // 인스턴스 핸들
HWND hEdit; // 편집 컨트롤
CRITICAL_SECTION cs; // 임계 영역
int b= 17; // 내키값중 b의값 이를 g^b mod p해서 abc에저장한 후에 클라이언트에게 전달
int x; //받은키값 g^a mod p
//real = x^b mod p == g^ab mod p
int real;
int mypri = 31; //rsa암호화 사용시 이용할 키값 중 개인키 d d*e =1 mod84
int mypub = 19; //rsa암호화 사용시 이용할 키값 중 공개키 e gcd(23,84)=1
int clientpub =23; //rsa암호화 사용시 이요할 클라이언트의 공개키 
int n = 129; // 두소수의 곱 3*43 phin은 84

HWND r1, r2, r3, r4;
int cryptocheck;





int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
    LPSTR lpCmdLine, int nCmdShow)
{
    hInst = hInstance;
    InitializeCriticalSection(&cs);

    // 윈도우 클래스 등록
    WNDCLASS wndclass;
    wndclass.style = CS_HREDRAW | CS_VREDRAW;
    wndclass.lpfnWndProc = WndProc;
    wndclass.cbClsExtra = 0;
    wndclass.cbWndExtra = 0;
    wndclass.hInstance = hInstance;
    wndclass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wndclass.hCursor = LoadCursor(NULL, IDC_ARROW);
    wndclass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
    wndclass.lpszMenuName = NULL;
    wndclass.lpszClassName = "MyWndClass";
    if (!RegisterClass(&wndclass)) return 1;

    // 윈도우 생성
    HWND hWnd = CreateWindow("MyWndClass", "TCP 서버", WS_OVERLAPPEDWINDOW,
        0, 0, 600, 600, NULL, NULL, hInstance, NULL);
    if (hWnd == NULL) return 1;
    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);
    
    // 소켓 통신 스레드 생성
    CreateThread(NULL, 0, ServerMain, NULL, 0, NULL);
    DialogBox(hInstance, MAKEINTRESOURCE(IDD_DIALOG1), NULL, DlgProc);
    // 메시지 루프
    MSG msg;
    while (GetMessage(&msg, 0, 0, 0) > 0) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    DeleteCriticalSection(&cs);
    return msg.wParam;
}

// 윈도우 프로시저
LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg) {
    case WM_CREATE:
        hEdit = CreateWindow("edit", NULL,
            WS_CHILD | WS_VISIBLE | WS_HSCROLL |
            WS_VSCROLL | ES_AUTOHSCROLL |
            ES_AUTOVSCROLL | ES_MULTILINE | ES_READONLY,
            100,100,100,100, hWnd, (HMENU)100, hInst, NULL);
            
        return 0;
    case WM_SIZE:
        MoveWindow(hEdit, 0, 0, LOWORD(lParam), HIWORD(lParam), TRUE);
        return 0;
    case WM_SETFOCUS:
        SetFocus(hEdit);
        return 0;
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProc(hWnd, uMsg, wParam, lParam);
}


BOOL CALLBACK DlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{

    switch (uMsg) {
    case WM_INITDIALOG:
        return TRUE;
    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case IDOK:


            return TRUE;
        case IDCANCEL:
            EndDialog(hDlg, IDCANCEL);

            return TRUE;
        case IDC_RADIO1: // xor
            cryptocheck = 1;
            break;
        case IDC_RADIO2: //시이저암호
            cryptocheck = 2;
            break;
        case IDC_RADIO3: //RSA
            cryptocheck = 3;
            break;
        case IDC_RADIO4: //평문
            cryptocheck = 4;
            break;
        }
        return FALSE;
    }
    return FALSE;

}





// 편집 컨트롤 출력 함수
void DisplayText(char *fmt, ...)
{
    va_list arg;
    va_start(arg, fmt);

    char cbuf[BUFSIZE + 256];
    vsprintf(cbuf, fmt, arg);

    EnterCriticalSection(&cs);
    int nLength = GetWindowTextLength(hEdit);
    SendMessage(hEdit, EM_SETSEL, nLength, nLength);
    SendMessage(hEdit, EM_REPLACESEL, FALSE, (LPARAM)cbuf);
    LeaveCriticalSection(&cs);

    va_end(arg);
}

// 소켓 함수 오류 출력 후 종료
void err_quit(char *msg)
{
    LPVOID lpMsgBuf;
    FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
        NULL, WSAGetLastError(),
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPTSTR)&lpMsgBuf, 0, NULL);
    MessageBox(NULL, (LPCTSTR)lpMsgBuf, msg, MB_ICONERROR);
    LocalFree(lpMsgBuf);
    exit(1);
}

// 소켓 함수 오류 출력
void err_display(char *msg)
{
    LPVOID lpMsgBuf;
    FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
        NULL, WSAGetLastError(),
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPTSTR)&lpMsgBuf, 0, NULL);
    DisplayText("[%s] %s", msg, (char *)lpMsgBuf);
    LocalFree(lpMsgBuf);
}

// TCP 서버 시작 부분
DWORD WINAPI ServerMain(LPVOID arg)
{
    int retval;
    int retval2;

    // 윈속 초기화
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
        return 1;

    // socket()
    SOCKET listen_sock = socket(AF_INET, SOCK_STREAM, 0);
    SOCKET listen_sock2 = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_sock == INVALID_SOCKET) err_quit("socket()");

    // bind()
    SOCKADDR_IN serveraddr;
    ZeroMemory(&serveraddr, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
    serveraddr.sin_port = htons(SERVERPORT);
    SOCKADDR_IN serveraddr2;
    ZeroMemory(&serveraddr2, sizeof(serveraddr2));
    serveraddr2.sin_family = AF_INET;
    serveraddr2.sin_addr.s_addr = htonl(INADDR_ANY);
    serveraddr2.sin_port = htons(SERVERPORT2);
    retval2 = bind(listen_sock2, (SOCKADDR*)&serveraddr2, sizeof(serveraddr2));
    if (retval2 == SOCKET_ERROR) err_quit("bind()");
    retval = bind(listen_sock, (SOCKADDR *)&serveraddr, sizeof(serveraddr));
    if (retval == SOCKET_ERROR) err_quit("bind()");

    // listen()
    retval = listen(listen_sock, SOMAXCONN);
    if (retval == SOCKET_ERROR) err_quit("listen()");
    retval2 = listen(listen_sock2, SOMAXCONN);
    if (retval2 == SOCKET_ERROR) err_quit("listen()");

    // 데이터 통신에 사용할 변수
    SOCKET client_sock;
    SOCKADDR_IN clientaddr;
    int addrlen;
    HANDLE hThread;

    SOCKET client_sock2;
    SOCKADDR_IN clientaddr2;
    int addrlen2;
    HANDLE hThread2;

    while (1) {
        // accept()
        addrlen = sizeof(clientaddr);
        addrlen2 = sizeof(clientaddr);
        client_sock = accept(listen_sock, (SOCKADDR *)&clientaddr, &addrlen);
        client_sock2 = accept(listen_sock2, (SOCKADDR*)&clientaddr2, &addrlen2);
        if (client_sock == INVALID_SOCKET) {
            err_display("accept()");
            break;
        }
        if (client_sock2 == INVALID_SOCKET) {
            err_display("accept()");
            break;
        }

        // 접속한 클라이언트 정보 출력
        DisplayText("\r\n[TCP 서버] 클라이언트 접속: IP 주소=%s, 포트 번호=%d\r\n",
            inet_ntoa(clientaddr.sin_addr), ntohs(clientaddr.sin_port));
        DisplayText("\r\n[키교환] 클라이언트 접속: IP 주소=%s, 포트 번호=%d\r\n",
            inet_ntoa(clientaddr2.sin_addr), ntohs(clientaddr2.sin_port));
        // 스레드 생성
        hThread = CreateThread(NULL, 0, ProcessClient,
            (LPVOID)client_sock, 0, NULL);
        hThread2 = CreateThread(NULL, 0, keyClient,
            (LPVOID)client_sock2, 0, NULL);

        if (hThread == NULL) { closesocket(client_sock); }
        else { CloseHandle(hThread); }
  
  
    }

    // closesocket()
    closesocket(listen_sock);
    closesocket(listen_sock2);
    // 윈속 종료
    WSACleanup();
    return 0;
}

void XOR(int real) {

    for (int i = 0; i < strlen(buf); i++)
    {
        buf[i] ^= real;
    }
}
void CaeserEnc(int real) {
    for (int i = 0; i < strlen(buf); i++)
        buf[i] += real;
}
void CaeserDec(int real) {
    for (int i = 0; i < strlen(buf); i++)
        buf[i] -= real;
}
void RSAEnc() { //클라이언트의공개키로 암호화
    int temp;
    for (int i = 0; i < strlen(buf); i++) {
        temp = (int)buf[i];
        for (int j = 1; j < clientpub; j++) {
            temp *= (int)buf[i];;
            temp %= n;
        }
        buf[i] = temp;
    }

}
void RSADec() { //내 개인키로 복호화
    int temp;
    for (int i = 0; i < strlen(buf); i++) {
        temp = (int)buf[i];
        for (int j = 1; j < mypri; j++) {
            temp *= (int)buf[i];;
            temp %= n;
        }
        buf[i] = temp;
    }
}
// 클라이언트와 키 통신
DWORD WINAPI keyClient(LPVOID arg)
{
   
    SOCKET client_sock2 = (SOCKET)arg;
    int retval2;
    SOCKADDR_IN clientaddr2;
    int addrlen2;

    // 클라이언트 정보 얻기

    addrlen2 = sizeof(clientaddr2);
    getpeername(client_sock2, (SOCKADDR*)&clientaddr2, &addrlen2);
      DisplayText("키통신스레드 생성 \r\n");
    while (1) {
        // 데이터 받기

        retval2 = recv(client_sock2, key, BUFSIZE, 0);
        if (retval2 == SOCKET_ERROR) {
            err_display("recv()");
            break;
        }
        else if (retval2 == 0)
            break;


        key[retval2] = '\0';
        DisplayText("[TCP/%s:%d] (디피헬먼) 키수신 \r\n", inet_ntoa(clientaddr2.sin_addr),
            ntohs(clientaddr2.sin_port));

        
        x = atoi(key);
        int abc = compute(5, b, 97); //g^b mod p b는 나의 개인키 서버에 저장되어있다.
        real = compute(x,b,97); // g^b ^a mod p x는 받은키값 


        sprintf(key, "%d", abc); //key문자열에 abc의 문자열값이 저장
        DisplayText("[TCP/%s:%d] (디피헬먼) 키송신 \r\n", inet_ntoa(clientaddr2.sin_addr),
            ntohs(clientaddr2.sin_port));
        retval2 = send(client_sock2, key, retval2, 0); // key를 송신
        if (retval2 == SOCKET_ERROR) {
            err_display("send()");
            break;
        }


    }

    // closesocket()

    closesocket(client_sock2);
    DisplayText("키송수신 종료 : IP 주소=%s, 포트 번호=%d\r\n",
        inet_ntoa(clientaddr2.sin_addr), ntohs(clientaddr2.sin_port));

    return 0;
}


// 클라이언트와 데이터 통신
DWORD WINAPI ProcessClient(LPVOID arg)
{
    SOCKET client_sock = (SOCKET)arg;
    int retval;
    SOCKADDR_IN clientaddr;
    int addrlen;

    
    // 클라이언트 정보 얻기
    addrlen = sizeof(clientaddr);
    getpeername(client_sock, (SOCKADDR *)&clientaddr, &addrlen);
    DisplayText("데이터 통신스레드 생성 \r\n");
    while (1) {
        // 데이터 받기
        retval = recv(client_sock, buf, BUFSIZE, 0);
        if (retval == SOCKET_ERROR) {
            err_display("recv()");
            break;
        }
        else if (retval == 0)
            break;


       

        buf[retval] = '\0';
        DisplayText("[TCP/%s:%d]받은데이터 %s\r\n", inet_ntoa(clientaddr.sin_addr),
            ntohs(clientaddr.sin_port), buf);
        if (cryptocheck == 1)XOR(real); // xor
        else if (cryptocheck == 2)CaeserDec(real);
        else if (cryptocheck == 3)RSADec();
        DisplayText("[TCP/%s:%d]받은데이터 복호화 : %s\r\n", inet_ntoa(clientaddr.sin_addr),
            ntohs(clientaddr.sin_port), buf);


        if (cryptocheck == 1)XOR(real); // xor
        else if (cryptocheck == 2)CaeserEnc(real);
        else if (cryptocheck == 3)RSAEnc();
        // 데이터 보내기
        DisplayText("[TCP/%s:%d]받은데이터 암호화 : %s\r\n", inet_ntoa(clientaddr.sin_addr),
            ntohs(clientaddr.sin_port), buf);
        DisplayText("\r\n");
        retval = send(client_sock, buf, retval, 0);
        if (retval == SOCKET_ERROR) {
            err_display("send()");
            break;
        }



    }

    // closesocket()
    closesocket(client_sock);
    DisplayText("[TCP 서버] 클라이언트 종료: IP 주소=%s, 포트 번호=%d\r\n",
        inet_ntoa(clientaddr.sin_addr), ntohs(clientaddr.sin_port));

    return 0;
}




// Function to compute `a^m mod n`
int compute(int a, int m, int n)
{
    int r;
    int y = 1;

    while (m > 0)
    {
        r = m % 2;

        // fast exponention
        if (r == 1) {
            y = (y * a) % n;
        }
        a = a * a % n;
        m = m / 2;
    }

    return y;
}

