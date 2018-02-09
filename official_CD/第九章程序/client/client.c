/*
 * TCP Stream Socket Client
 * �������˵�������ʵ��һ��Winsock�Ŀͻ��ˡ�
 * �����Ĺ����ǳ��򵥣�������һ����Ϣ�������������ҽ��մӷ������ķ�����Ϣ��
 * ���ָ����������Ϊ"localhost"����������TCP Stream Socket Server�������������ͬһ̨�����ϡ�
 * Ҳ����ͨ��ָ�����������������ڲ�ͬ�Ļ����Ϲ�����
*/
#include <windows.h> 
#include <winsock.h>

#define PORTNUM         5000          // �˿ں�
#define HOSTNAME        "localhost"   // ����������
                                      // ���ݷ������Ĳ�ͬ���������������
                                      
int WINAPI WinMain (
              HINSTANCE hInstance,    // ʵ�����
              HINSTANCE hPrevInstance,// NULL
              LPTSTR lpCmdLine,       // �������г����������
              int nCmdShow)           // ������������ʾ��ʽ
{
  int index = 0,                      // ����
      iReturn;                        // recv�����ķ���ֵ
  char szClientA[100];                // ASCII�봮
  TCHAR szClientW[100];               // Unicode��
  TCHAR szError[100];                 // ������Ϣ��

  SOCKET ServerSock = INVALID_SOCKET; // ��������socket
  SOCKADDR_IN destination_sin;        // ��������socket��ַ
  PHOSTENT phostent = NULL;           // ָ���������HOSTENT�ṹ
  WSADATA WSAData;                    // ������Winsocketִ�к��ϸ��

  // ��ʼ��Winsocket. 
  if (WSAStartup (MAKEWORD(1,1), &WSAData) != 0) 
  {
    wsprintf (szError, TEXT("WSAStartup failed. Error: %d"), 
              WSAGetLastError ());
    MessageBox (NULL, szError, TEXT("Error"), MB_OK);
    return FALSE;
  }

  // ����һ���󶨷�������TCP/IP socket
  if ((ServerSock = socket (AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET)
  {
    wsprintf (szError, TEXT("Allocating socket failed. Error: %d"), 
              WSAGetLastError ());
    MessageBox (NULL, szError, TEXT("Error"), MB_OK);
    return FALSE;
  }

  // �ѷ�������socket�ĵ�ַ��Ϣ����
  destination_sin.sin_family = AF_INET;

  // ͨ��������ȡ��������Ϣ
  if ((phostent = gethostbyname (HOSTNAME)) == NULL) 
  {
    wsprintf (szError, TEXT("Unable to get the host name. Error: %d"), 
              WSAGetLastError ());
    MessageBox (NULL, szError, TEXT("Error"), MB_OK);
    closesocket (ServerSock);
    return FALSE;
  }

  // ����socket IP��ַ
  memcpy ((char FAR *)&(destination_sin.sin_addr), 
          phostent->h_addr, 
          phostent->h_length);

  // ת���˿��ֶ�Ϊ���յ�
  destination_sin.sin_port = htons (PORTNUM);      

  // ����һ����������socket������.
  if (connect (ServerSock, 
               (PSOCKADDR) &destination_sin, 
               sizeof (destination_sin)) == SOCKET_ERROR) 
  {
    wsprintf (szError, 
              TEXT("Connecting to the server failed. Error: %d"),
              WSAGetLastError ());
    MessageBox (NULL, szError, TEXT("Error"), MB_OK);
    closesocket (ServerSock);
    return FALSE;
  }

  // �������������Ϣ
  if (send (ServerSock, "To Server.", strlen ("To Server.") + 1, 0)
           == SOCKET_ERROR) 
  {
    wsprintf (szError, 
              TEXT("Sending data to the server failed. Error: %d"),
              WSAGetLastError ());
    MessageBox (NULL, szError, TEXT("Error"), MB_OK);
  }

  // ��ֹ������������Ϣ
  shutdown (ServerSock, 0x01);

  for (;;)
  {
    // �ӷ�����������Ϣ
    iReturn = recv (ServerSock, szClientA, sizeof (szClientA), 0);

    // ������յ���������Ϣ����ʾ֮
    if (iReturn == SOCKET_ERROR)
    {
      wsprintf (szError, TEXT("No data is received, recv failed.")
                TEXT(" Error: %d"), WSAGetLastError ());
      MessageBox (NULL, szError, TEXT("Client"), MB_OK);
      break;
    }
    else if (iReturn == 0)
    {
      MessageBox (NULL, TEXT("Finished receiving data"), TEXT("Client"),
                  MB_OK);
      break;
    }
    else
    {
      // ��ASCII���ַ���ת����Unicode���ַ���
      for (index = 0; index <= sizeof (szClientA); index++)
        szClientW[index] = szClientA[index];

      // ��ʾ�ӷ��������յ����ַ���
      MessageBox (NULL, szClientW, TEXT("Received From Server"), MB_OK);
    }
  }

  // ��ֹ������������Ϣ
  shutdown (ServerSock, 0x00);

  // �رշ�����socket.
  closesocket (ServerSock);

  WSACleanup ();

  return TRUE;
}
