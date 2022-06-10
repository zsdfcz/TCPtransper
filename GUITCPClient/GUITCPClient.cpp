#define _CRT_SECURE_NO_WARNINGS         // 최신 VC++ 컴파일 시 경고 방지
#define _WINSOCK_DEPRECATED_NO_WARNINGS // 최신 VC++ 컴파일 시 경고 방지
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

// 대화상자 프로시저
BOOL CALLBACK DlgProc(HWND, UINT, WPARAM, LPARAM);
// 편집 컨트롤 출력 함수
void DisplayText(char* fmt, ...);
// 오류 출력 함수
void err_quit(char* msg);
void err_display(char* msg);
// 사용자 정의 데이터 수신 함수
int recvn(SOCKET s, char* buf, int len, int flags);
// 소켓 통신 스레드 함수window
DWORD WINAPI ClientMain(LPVOID arg);
DWORD WINAPI Clientkey(LPVOID arg);
SOCKET sock; // 소켓
SOCKET sock2; // 소켓2
char buf[BUFSIZE + 1]; // 데이터 송수신 버퍼
char key[BUFSIZE + 1];
int cryptocheck;
HANDLE hReadEvent, hWriteEvent , hReadEvent2, hWriteEvent2; // 이벤트
HWND hSendButton; // 보내기 버튼
HWND keysendButton; // 키보내기 버튼
HWND hEdit1, hEdit2,hEdit3; // 편집 컨트롤
int x; //받은키값g ^b mod p값;
int mya; //내 secreat 키값 g^amodp에서의 a값
int real; //받은키와 내키값을 통해 계산한 암호화키 g^mya ^x mod p
int mypri=11; //rsa암호화 사용시 이용할 키값 중 개인키 d d*e =1 mod84
int mypub=23; //rsa암호화 사용시 이용할 키값 중 공개키 e gcd(23,84)=1
int serverpub =19 ; //rsa암호화 사용시 이용할 서버의 공개키 
int n = 129; // 두소수의 곱 3*43 phin은 84

int keyset(char* abc, int len);
int compute(int a, int m, int n);





int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
    LPSTR lpCmdLine, int nCmdShow)
{
    
    // 이벤트 생성
    hReadEvent = CreateEvent(NULL, FALSE, TRUE, NULL);
    hReadEvent2 = CreateEvent(NULL, FALSE, TRUE, NULL);
    if (hReadEvent == NULL) return 1;
    if (hReadEvent2 == NULL) return 1;
    hWriteEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
    hWriteEvent2 = CreateEvent(NULL, FALSE, FALSE, NULL);
    if (hWriteEvent == NULL) return 1;
    if (hWriteEvent2 == NULL) return 1;


    CreateThread(NULL, 0, Clientkey, NULL, 0, NULL);
    // 소켓 통신 스레드 생성
    CreateThread(NULL, 0, ClientMain, NULL, 0, NULL);
    //키 통신 스레드 생성
    

    // 대화상자 생성
    DialogBox(hInstance, MAKEINTRESOURCE(IDD_DIALOG1), NULL, DlgProc);

    // 이벤트 제거
    CloseHandle(hReadEvent);
    CloseHandle(hWriteEvent);

    // closesocket()
    closesocket(sock);
    closesocket(sock2);
    // 윈속 종료
    WSACleanup();
    return 0;
}


//라디오버튼 체크
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




// 대화상자 프로시저
BOOL CALLBACK DlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (wParam)
    {
   /**/
    case RADIO1: // xor
        cryptocheck = 1;
        break;
    case IRADIO2: //시이저암호
        cryptocheck = 2;
        break;
    case IRADIO3: //S - DES
        cryptocheck = 3;
        break;
    case IRADIO4: //평문
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
                
                EnableWindow(hSendButton, FALSE); // 보내기 버튼 비활성화
                WaitForSingleObject(hReadEvent, INFINITE); // 읽기 완료 기다리기
                GetDlgItemText(hDlg, IDC_EDIT1, buf, BUFSIZE + 1);
                SetEvent(hWriteEvent); // 쓰기 완료 알리기
                SetFocus(hEdit1);
                SendMessage(hEdit1, EM_SETSEL, 0, -1);
                break;
          return TRUE;
        case IDCANCEL:
            EndDialog(hDlg, IDCANCEL);
            
             return TRUE;
        case Keysend:
            EnableWindow(keysendButton, FALSE); // 보내기 버튼 비활성화
            WaitForSingleObject(hReadEvent2, INFINITE); // 읽기 완료 기다리기
            GetDlgItemText(hDlg, IDC_EDIT3, key, BUFSIZE + 1);

            SetEvent(hWriteEvent2); // 쓰기 완료 알리기
            SetFocus(hEdit3);
            SendMessage(hEdit3, EM_SETSEL, 0, -1);

            return TRUE;

        }
        return FALSE;
    }
    return FALSE;
}

// 편집 컨트롤 출력 함수
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

// 소켓 함수 오류 출력 후 종료
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

// 소켓 함수 오류 출력
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

// 사용자 정의 데이터 수신 함수
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

void RSAEnc(){ //서버의 공개키로 암호화
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
void RSADec() { //내 개인키로 복호화
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

// TCP 클라이언트 시작 부분
DWORD WINAPI ClientMain(LPVOID arg)
{
    int retval;

    // 윈속 초기화
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



    // 서버와 데이터 통신
    while (1) {
        WaitForSingleObject(hWriteEvent, INFINITE); // 쓰기 완료 기다리기
        // 문자열 길이가 0이면 보내지 않음
        if (strlen(buf) == 0) {
            EnableWindow(hSendButton, TRUE); // 보내기 버튼 활성화
            SetEvent(hReadEvent); // 읽기 완료 알리기
            continue;
        }
       

        DisplayText("[보낼 데이터] %s\r\n", buf);
        if (cryptocheck == 1)XOR(real); // xor
        else if (cryptocheck == 2)CaeserEnc(real);// 시이저암호화
        else if (cryptocheck == 3)RSAEnc(); //RSA 암호화 서버의 공개키로 암호화
      DisplayText("[보낼 데이터] 암호화 %s\r\n", buf);


        // 데이터 보내기
      
        retval = send(sock, buf, strlen(buf), 0);
        if (retval == SOCKET_ERROR) {
            err_display("send()");
            break;
        }
        DisplayText("[TCP 클라이언트] %d바이트를 보냈습니다.\r\n", retval);

        // 데이터 받기
        retval = recvn(sock, buf, retval, 0);
        if (retval == SOCKET_ERROR) {
            err_display("recv()");
            break;
        }
        else if (retval == 0)
            break;

    
        // 받은 데이터 출력
        buf[retval] = '\0';
        DisplayText("[TCP 클라이언트] %d바이트를 받았습니다.\r\n", retval);
        DisplayText("[받은 데이터] %s\r\n", buf);
        if (cryptocheck == 1)XOR(real);
        else if (cryptocheck == 2)CaeserDec(real);
        else if (cryptocheck == 3)RSADec();
        DisplayText("[받은 데이터] (복호화) %s\r\n", buf);
        DisplayText("\r\n");
        EnableWindow(hSendButton, TRUE);
        EnableWindow(keysendButton, TRUE); // 보내기 버튼 활성화
        SetEvent(hReadEvent); // 읽기 완료 알리기
    }

    return 0;
}


//키교환
DWORD WINAPI Clientkey(LPVOID arg)
{
    
    
    // 윈속 초기화
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


    // 서버와 데이터 통신
    while (1) {
        WaitForSingleObject(hWriteEvent2, INFINITE); // 쓰기 완료 기다리기

        // 문자열 길이가 0이면 보내지 않음
        if (strlen(key) == 0) {
            EnableWindow(keysendButton, TRUE);
            SetEvent(hReadEvent2); // 읽기 완료 알리기
            continue;
        }
        

        char key2[BUFSIZE + 1];
        strcpy(key2, key);
          // 데이터 보내기
      mya=  keyset(key2, strlen(key2)); //string의 key값을 int형으로 변환 //g^a 에서 a값을 정하는과정
      int sendmykey;
      sendmykey =compute(5,mya,97);  //g^mya modp (g=5 p=97)
      sprintf(key, "%d", sendmykey); //g^a modp를 key값에 저장
        retval2 = send(sock2, key, strlen(key), 0);
        if (retval2 == SOCKET_ERROR) {
            err_display("send()");
            break;
        }
        DisplayText("[TCP 클라이언트] : 키를 보냈습니다.\r\n");
       
        // 데이터 받기
       

        retval2 = recvn(sock2, key, retval2, 0);
        if (retval2 == SOCKET_ERROR) {
            err_display("recv()");
            break;
        }
        else if (retval2 == 0)
            break;
        // 받은 데이터 출력
        x = atoi(key);
        key[retval2] = '\0';
        DisplayText("[TCP 클라이언트] 키를 받았습니다.\r\n");
        real = compute(x, mya, 97);// 
       
        EnableWindow(keysendButton, TRUE); // 보내기 버튼 활성화
        SetEvent(hReadEvent2); // 읽기 완료 알리기
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




