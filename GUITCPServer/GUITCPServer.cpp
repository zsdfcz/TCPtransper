#define _CRT_SECURE_NO_WARNINGS         // �ֽ� VC++ ������ �� ��� ����
#define _WINSOCK_DEPRECATED_NO_WARNINGS // �ֽ� VC++ ������ �� ��� ����
#pragma comment(lib, "ws2_32")
#include <winsock2.h>
#include <stdlib.h>
#include <stdio.h>
#include "resource.h"
#define SERVERPORT 9000
#define SERVERPORT2 9001
#define BUFSIZE    512


// ������ ���ν���
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
//��ȭ���� ���ν���
BOOL CALLBACK DlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
// ���� ��Ʈ�� ��� �Լ�
void DisplayText(char *fmt, ...);
// ���� ��� �Լ�
void err_quit(char *msg);
void err_display(char *msg);
int keyset(char* abc, int len);
int compute(int a, int m, int n);
// ���� ��� ������ �Լ�
DWORD WINAPI ServerMain(LPVOID arg);
DWORD WINAPI ProcessClient(LPVOID arg);
DWORD WINAPI keyClient(LPVOID arg);
char buf[BUFSIZE + 1]; // ������ �ۼ��� ����
char key[BUFSIZE + 1]; // Ű�� ����
HINSTANCE hInst; // �ν��Ͻ� �ڵ�
HWND hEdit; // ���� ��Ʈ��
CRITICAL_SECTION cs; // �Ӱ� ����
int b= 17; // ��Ű���� b�ǰ� �̸� g^b mod p�ؼ� abc�������� �Ŀ� Ŭ���̾�Ʈ���� ����
int x; //����Ű�� g^a mod p
//real = x^b mod p == g^ab mod p
int real;
int mypri = 31; //rsa��ȣȭ ���� �̿��� Ű�� �� ����Ű d d*e =1 mod84
int mypub = 19; //rsa��ȣȭ ���� �̿��� Ű�� �� ����Ű e gcd(23,84)=1
int clientpub =23; //rsa��ȣȭ ���� �̿��� Ŭ���̾�Ʈ�� ����Ű 
int n = 129; // �μҼ��� �� 3*43 phin�� 84

HWND r1, r2, r3, r4;
int cryptocheck;





int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
    LPSTR lpCmdLine, int nCmdShow)
{
    hInst = hInstance;
    InitializeCriticalSection(&cs);

    // ������ Ŭ���� ���
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

    // ������ ����
    HWND hWnd = CreateWindow("MyWndClass", "TCP ����", WS_OVERLAPPEDWINDOW,
        0, 0, 600, 600, NULL, NULL, hInstance, NULL);
    if (hWnd == NULL) return 1;
    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);
    
    // ���� ��� ������ ����
    CreateThread(NULL, 0, ServerMain, NULL, 0, NULL);
    DialogBox(hInstance, MAKEINTRESOURCE(IDD_DIALOG1), NULL, DlgProc);
    // �޽��� ����
    MSG msg;
    while (GetMessage(&msg, 0, 0, 0) > 0) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    DeleteCriticalSection(&cs);
    return msg.wParam;
}

// ������ ���ν���
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
        case IDC_RADIO2: //��������ȣ
            cryptocheck = 2;
            break;
        case IDC_RADIO3: //RSA
            cryptocheck = 3;
            break;
        case IDC_RADIO4: //��
            cryptocheck = 4;
            break;
        }
        return FALSE;
    }
    return FALSE;

}





// ���� ��Ʈ�� ��� �Լ�
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

// ���� �Լ� ���� ��� �� ����
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

// ���� �Լ� ���� ���
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

// TCP ���� ���� �κ�
DWORD WINAPI ServerMain(LPVOID arg)
{
    int retval;
    int retval2;

    // ���� �ʱ�ȭ
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

    // ������ ��ſ� ����� ����
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

        // ������ Ŭ���̾�Ʈ ���� ���
        DisplayText("\r\n[TCP ����] Ŭ���̾�Ʈ ����: IP �ּ�=%s, ��Ʈ ��ȣ=%d\r\n",
            inet_ntoa(clientaddr.sin_addr), ntohs(clientaddr.sin_port));
        DisplayText("\r\n[Ű��ȯ] Ŭ���̾�Ʈ ����: IP �ּ�=%s, ��Ʈ ��ȣ=%d\r\n",
            inet_ntoa(clientaddr2.sin_addr), ntohs(clientaddr2.sin_port));
        // ������ ����
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
    // ���� ����
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
void RSAEnc() { //Ŭ���̾�Ʈ�ǰ���Ű�� ��ȣȭ
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
void RSADec() { //�� ����Ű�� ��ȣȭ
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
// Ŭ���̾�Ʈ�� Ű ���
DWORD WINAPI keyClient(LPVOID arg)
{
   
    SOCKET client_sock2 = (SOCKET)arg;
    int retval2;
    SOCKADDR_IN clientaddr2;
    int addrlen2;

    // Ŭ���̾�Ʈ ���� ���

    addrlen2 = sizeof(clientaddr2);
    getpeername(client_sock2, (SOCKADDR*)&clientaddr2, &addrlen2);
      DisplayText("Ű��Ž����� ���� \r\n");
    while (1) {
        // ������ �ޱ�

        retval2 = recv(client_sock2, key, BUFSIZE, 0);
        if (retval2 == SOCKET_ERROR) {
            err_display("recv()");
            break;
        }
        else if (retval2 == 0)
            break;


        key[retval2] = '\0';
        DisplayText("[TCP/%s:%d] (�������) Ű���� \r\n", inet_ntoa(clientaddr2.sin_addr),
            ntohs(clientaddr2.sin_port));

        
        x = atoi(key);
        int abc = compute(5, b, 97); //g^b mod p b�� ���� ����Ű ������ ����Ǿ��ִ�.
        real = compute(x,b,97); // g^b ^a mod p x�� ����Ű�� 


        sprintf(key, "%d", abc); //key���ڿ��� abc�� ���ڿ����� ����
        DisplayText("[TCP/%s:%d] (�������) Ű�۽� \r\n", inet_ntoa(clientaddr2.sin_addr),
            ntohs(clientaddr2.sin_port));
        retval2 = send(client_sock2, key, retval2, 0); // key�� �۽�
        if (retval2 == SOCKET_ERROR) {
            err_display("send()");
            break;
        }


    }

    // closesocket()

    closesocket(client_sock2);
    DisplayText("Ű�ۼ��� ���� : IP �ּ�=%s, ��Ʈ ��ȣ=%d\r\n",
        inet_ntoa(clientaddr2.sin_addr), ntohs(clientaddr2.sin_port));

    return 0;
}


// Ŭ���̾�Ʈ�� ������ ���
DWORD WINAPI ProcessClient(LPVOID arg)
{
    SOCKET client_sock = (SOCKET)arg;
    int retval;
    SOCKADDR_IN clientaddr;
    int addrlen;

    
    // Ŭ���̾�Ʈ ���� ���
    addrlen = sizeof(clientaddr);
    getpeername(client_sock, (SOCKADDR *)&clientaddr, &addrlen);
    DisplayText("������ ��Ž����� ���� \r\n");
    while (1) {
        // ������ �ޱ�
        retval = recv(client_sock, buf, BUFSIZE, 0);
        if (retval == SOCKET_ERROR) {
            err_display("recv()");
            break;
        }
        else if (retval == 0)
            break;


       

        buf[retval] = '\0';
        DisplayText("[TCP/%s:%d]���������� %s\r\n", inet_ntoa(clientaddr.sin_addr),
            ntohs(clientaddr.sin_port), buf);
        if (cryptocheck == 1)XOR(real); // xor
        else if (cryptocheck == 2)CaeserDec(real);
        else if (cryptocheck == 3)RSADec();
        DisplayText("[TCP/%s:%d]���������� ��ȣȭ : %s\r\n", inet_ntoa(clientaddr.sin_addr),
            ntohs(clientaddr.sin_port), buf);


        if (cryptocheck == 1)XOR(real); // xor
        else if (cryptocheck == 2)CaeserEnc(real);
        else if (cryptocheck == 3)RSAEnc();
        // ������ ������
        DisplayText("[TCP/%s:%d]���������� ��ȣȭ : %s\r\n", inet_ntoa(clientaddr.sin_addr),
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
    DisplayText("[TCP ����] Ŭ���̾�Ʈ ����: IP �ּ�=%s, ��Ʈ ��ȣ=%d\r\n",
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

