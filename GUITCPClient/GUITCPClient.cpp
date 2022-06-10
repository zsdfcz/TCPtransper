#define _CRT_SECURE_NO_WARNINGS         // �ֽ� VC++ ������ �� ��� ����
#define _WINSOCK_DEPRECATED_NO_WARNINGS // �ֽ� VC++ ������ �� ��� ����
#pragma comment(lib, "ws2_32")
#include <winsock2.h>
#include <stdlib.h>
#include <stdio.h>
#include "resource.h"
#include <iostream>
#include <string>
#include <string.h>
#include <conio.h>


#define SERVERIP   "127.0.0.1"
#define SERVERPORT 9000
#define SERVERPORT2 9001
#define BUFSIZE    512

// ��ȭ���� ���ν���
BOOL CALLBACK DlgProc(HWND, UINT, WPARAM, LPARAM);
// ���� ��Ʈ�� ��� �Լ�
void DisplayText(char* fmt, ...);
// ���� ��� �Լ�
void err_quit(char* msg);
void err_display(char* msg);
// ����� ���� ������ ���� �Լ�
int recvn(SOCKET s, char* buf, int len, int flags);
// ���� ��� ������ �Լ�window
DWORD WINAPI ClientMain(LPVOID arg);
DWORD WINAPI Clientkey(LPVOID arg);
SOCKET sock; // ����
SOCKET sock2; // ����2
char buf[BUFSIZE + 1]; // ������ �ۼ��� ����
char key[BUFSIZE + 1];
int cryptocheck;
HANDLE hReadEvent, hWriteEvent , hReadEvent2, hWriteEvent2; // �̺�Ʈ
HWND hSendButton; // ������ ��ư
HWND keysendButton; // Ű������ ��ư
HWND hEdit1, hEdit2,hEdit3; // ���� ��Ʈ��
int x; //����Ű��g ^b mod p��;
int mya; //�� secreat Ű�� g^amodp������ a��
int real; //����Ű�� ��Ű���� ���� ����� ��ȣȭŰ g^mya ^x mod p
int mypri=11; //rsa��ȣȭ ���� �̿��� Ű�� �� ����Ű d d*e =1 mod84
int mypub=23; //rsa��ȣȭ ���� �̿��� Ű�� �� ����Ű e gcd(23,84)=1
int serverpub =19 ; //rsa��ȣȭ ���� �̿��� ������ ����Ű 
int n = 129; // �μҼ��� �� 3*43 phin�� 84

int keyset(char* abc, int len);
int compute(int a, int m, int n);





int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
    LPSTR lpCmdLine, int nCmdShow)
{
    
    // �̺�Ʈ ����
    hReadEvent = CreateEvent(NULL, FALSE, TRUE, NULL);
    hReadEvent2 = CreateEvent(NULL, FALSE, TRUE, NULL);
    if (hReadEvent == NULL) return 1;
    if (hReadEvent2 == NULL) return 1;
    hWriteEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
    hWriteEvent2 = CreateEvent(NULL, FALSE, FALSE, NULL);
    if (hWriteEvent == NULL) return 1;
    if (hWriteEvent2 == NULL) return 1;


    CreateThread(NULL, 0, Clientkey, NULL, 0, NULL);
    // ���� ��� ������ ����
    CreateThread(NULL, 0, ClientMain, NULL, 0, NULL);
    //Ű ��� ������ ����
    

    // ��ȭ���� ����
    DialogBox(hInstance, MAKEINTRESOURCE(IDD_DIALOG1), NULL, DlgProc);

    // �̺�Ʈ ����
    CloseHandle(hReadEvent);
    CloseHandle(hWriteEvent);

    // closesocket()
    closesocket(sock);
    closesocket(sock2);
    // ���� ����
    WSACleanup();
    return 0;
}


//������ư üũ
LRESULT onCommand(HWND hwnd, WPARAM wparam, LPARAM lParam) {
    switch (LOWORD(wparam))
    {
    case RADIO1:
        cryptocheck = 1;
        break;
    case IRADIO2:
        cryptocheck = 2;
        break;
    case IRADIO3:
        cryptocheck = 3;
        break;
    case IRADIO4:
        cryptocheck = 4;
        break;
    }
    return 0;
}




// ��ȭ���� ���ν���
BOOL CALLBACK DlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (wParam)
    {
   /**/
    case RADIO1: // xor
        cryptocheck = 1;
        break;
    case IRADIO2: //��������ȣ
        cryptocheck = 2;
        break;
    case IRADIO3: //S - DES
        cryptocheck = 3;
        break;
    case IRADIO4: //��
        cryptocheck = 4;
        break;
    } 





    switch (uMsg) {
    case WM_INITDIALOG:
        hEdit1 = GetDlgItem(hDlg, IDC_EDIT1);
        hEdit2 = GetDlgItem(hDlg, IDC_EDIT2);
        hEdit3 = GetDlgItem(hDlg, IDC_EDIT3);
        hSendButton = GetDlgItem(hDlg, IDOK);
        keysendButton = GetDlgItem(hDlg, Keysend);
        SendMessage(hEdit1, EM_SETLIMITTEXT, BUFSIZE, 0);
        SendMessage(hEdit3, EM_SETLIMITTEXT, BUFSIZE, 0);
        return TRUE;
    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case IDOK:
                
                EnableWindow(hSendButton, FALSE); // ������ ��ư ��Ȱ��ȭ
                WaitForSingleObject(hReadEvent, INFINITE); // �б� �Ϸ� ��ٸ���
                GetDlgItemText(hDlg, IDC_EDIT1, buf, BUFSIZE + 1);
                SetEvent(hWriteEvent); // ���� �Ϸ� �˸���
                SetFocus(hEdit1);
                SendMessage(hEdit1, EM_SETSEL, 0, -1);
                break;
          return TRUE;
        case IDCANCEL:
            EndDialog(hDlg, IDCANCEL);
            
             return TRUE;
        case Keysend:
            EnableWindow(keysendButton, FALSE); // ������ ��ư ��Ȱ��ȭ
            WaitForSingleObject(hReadEvent2, INFINITE); // �б� �Ϸ� ��ٸ���
            GetDlgItemText(hDlg, IDC_EDIT3, key, BUFSIZE + 1);

            SetEvent(hWriteEvent2); // ���� �Ϸ� �˸���
            SetFocus(hEdit3);
            SendMessage(hEdit3, EM_SETSEL, 0, -1);

            return TRUE;

        }
        return FALSE;
    }
    return FALSE;
}

// ���� ��Ʈ�� ��� �Լ�
void DisplayText(char* fmt, ...)
{
    va_list arg;
    va_start(arg, fmt);

    char cbuf[BUFSIZE + 256];
    vsprintf(cbuf, fmt, arg);

    int nLength = GetWindowTextLength(hEdit2);
    SendMessage(hEdit2, EM_SETSEL, nLength, nLength);
    SendMessage(hEdit2, EM_REPLACESEL, FALSE, (LPARAM)cbuf);

    va_end(arg);
}

// ���� �Լ� ���� ��� �� ����
void err_quit(char* msg)
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
void err_display(char* msg)
{
    LPVOID lpMsgBuf;
    FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
        NULL, WSAGetLastError(),
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPTSTR)&lpMsgBuf, 0, NULL);
    DisplayText("[%s] %s", msg, (char*)lpMsgBuf);
    LocalFree(lpMsgBuf);
}

// ����� ���� ������ ���� �Լ�
int recvn(SOCKET s, char* buf, int len, int flags)
{
    int received;
    char* ptr = buf;
    int left = len;

    while (left > 0) {
        received = recv(s, ptr, left, flags);
        if (received == SOCKET_ERROR)
            return SOCKET_ERROR;
        else if (received == 0)
            break;
        left -= received;
        ptr += received;
    }

    return (len - left);
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

void RSAEnc(){ //������ ����Ű�� ��ȣȭ
    int temp;
    for (int i = 0; i < strlen(buf); i++) {
        temp = (int)buf[i];
        for (int j = 1; j < serverpub; j++) {
            temp *= (int)buf[i];
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
            temp *= (int)buf[i];
            temp %= n;
        }
      
        buf[i] = temp;
    }
}

// TCP Ŭ���̾�Ʈ ���� �κ�
DWORD WINAPI ClientMain(LPVOID arg)
{
    int retval;

    // ���� �ʱ�ȭ
    WSADATA wsa;
    
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
        return 1;

 


    // socket()
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == INVALID_SOCKET) err_quit("socket()");
  

    // connect()
    SOCKADDR_IN serveraddr;
    ZeroMemory(&serveraddr, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_addr.s_addr = inet_addr(SERVERIP);
    serveraddr.sin_port = htons(SERVERPORT);
    retval = connect(sock, (SOCKADDR*)&serveraddr, sizeof(serveraddr));
    if (retval == SOCKET_ERROR) err_quit("connect()");



    // ������ ������ ���
    while (1) {
        WaitForSingleObject(hWriteEvent, INFINITE); // ���� �Ϸ� ��ٸ���
        // ���ڿ� ���̰� 0�̸� ������ ����
        if (strlen(buf) == 0) {
            EnableWindow(hSendButton, TRUE); // ������ ��ư Ȱ��ȭ
            SetEvent(hReadEvent); // �б� �Ϸ� �˸���
            continue;
        }
       

        DisplayText("[���� ������] %s\r\n", buf);
        if (cryptocheck == 1)XOR(real); // xor
        else if (cryptocheck == 2)CaeserEnc(real);// ��������ȣȭ
        else if (cryptocheck == 3)RSAEnc(); //RSA ��ȣȭ ������ ����Ű�� ��ȣȭ
      DisplayText("[���� ������] ��ȣȭ %s\r\n", buf);


        // ������ ������
      
        retval = send(sock, buf, strlen(buf), 0);
        if (retval == SOCKET_ERROR) {
            err_display("send()");
            break;
        }
        DisplayText("[TCP Ŭ���̾�Ʈ] %d����Ʈ�� ���½��ϴ�.\r\n", retval);

        // ������ �ޱ�
        retval = recvn(sock, buf, retval, 0);
        if (retval == SOCKET_ERROR) {
            err_display("recv()");
            break;
        }
        else if (retval == 0)
            break;

    
        // ���� ������ ���
        buf[retval] = '\0';
        DisplayText("[TCP Ŭ���̾�Ʈ] %d����Ʈ�� �޾ҽ��ϴ�.\r\n", retval);
        DisplayText("[���� ������] %s\r\n", buf);
        if (cryptocheck == 1)XOR(real);
        else if (cryptocheck == 2)CaeserDec(real);
        else if (cryptocheck == 3)RSADec();
        DisplayText("[���� ������] (��ȣȭ) %s\r\n", buf);
        DisplayText("\r\n");
        EnableWindow(hSendButton, TRUE);
        EnableWindow(keysendButton, TRUE); // ������ ��ư Ȱ��ȭ
        SetEvent(hReadEvent); // �б� �Ϸ� �˸���
    }

    return 0;
}


//Ű��ȯ
DWORD WINAPI Clientkey(LPVOID arg)
{
    
    
    // ���� �ʱ�ȭ
    int retval2;
    WSADATA wsa2;

    if (WSAStartup(MAKEWORD(2, 2), &wsa2) != 0)
        return 1;
    // socket()
    sock2 = socket(AF_INET, SOCK_STREAM, 0);
    if (sock2 == INVALID_SOCKET) err_quit("socket()");


    SOCKADDR_IN serveraddr2;
    ZeroMemory(&serveraddr2, sizeof(serveraddr2));
    serveraddr2.sin_family = AF_INET;
    serveraddr2.sin_addr.s_addr = inet_addr(SERVERIP);
    serveraddr2.sin_port = htons(SERVERPORT2);
    retval2 = connect(sock2, (SOCKADDR*)&serveraddr2, sizeof(serveraddr2));
    if (retval2 == SOCKET_ERROR) err_quit("connect()");


    // connect()


    // ������ ������ ���
    while (1) {
        WaitForSingleObject(hWriteEvent2, INFINITE); // ���� �Ϸ� ��ٸ���

        // ���ڿ� ���̰� 0�̸� ������ ����
        if (strlen(key) == 0) {
            EnableWindow(keysendButton, TRUE);
            SetEvent(hReadEvent2); // �б� �Ϸ� �˸���
            continue;
        }
        

        char key2[BUFSIZE + 1];
        strcpy(key2, key);
          // ������ ������
      mya=  keyset(key2, strlen(key2)); //string�� key���� int������ ��ȯ //g^a ���� a���� ���ϴ°���
      int sendmykey;
      sendmykey =compute(5,mya,97);  //g^mya modp (g=5 p=97)
      sprintf(key, "%d", sendmykey); //g^a modp�� key���� ����
        retval2 = send(sock2, key, strlen(key), 0);
        if (retval2 == SOCKET_ERROR) {
            err_display("send()");
            break;
        }
        DisplayText("[TCP Ŭ���̾�Ʈ] : Ű�� ���½��ϴ�.\r\n");
       
        // ������ �ޱ�
       

        retval2 = recvn(sock2, key, retval2, 0);
        if (retval2 == SOCKET_ERROR) {
            err_display("recv()");
            break;
        }
        else if (retval2 == 0)
            break;
        // ���� ������ ���
        x = atoi(key);
        key[retval2] = '\0';
        DisplayText("[TCP Ŭ���̾�Ʈ] Ű�� �޾ҽ��ϴ�.\r\n");
        real = compute(x, mya, 97);// 
       
        EnableWindow(keysendButton, TRUE); // ������ ��ư Ȱ��ȭ
        SetEvent(hReadEvent2); // �б� �Ϸ� �˸���
    }

    return 0;
}

//a^m(modn) a m n
int keyset(char* abc, int len) {
   
    int a = 5 ;
    int m = 0;

    for (int i = 0; i < len; i++)
    {
        m += (int)abc[i];
    }

    return m;


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




