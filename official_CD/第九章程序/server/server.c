
/*
 *TCP Stream Socket Server
 *�������˵�����ʵ��һ��Winsock TCP stream socket��������
 *�����մӿͻ��˷���������Ϣ���ҷ���һ����Ϣ���ͻ��ˡ� 
*/
#include <windows.h> 
#include <winsock.h>
#include "resource.h"
#include "PROCESS.H"

#define PORTNUM               5000    // �˿ں�
#define MAX_PENDING_CONNECTS  4       // �ȴ����Ӷ��е���󳤶�

#define	WM_MESSAGE_REVED	(WM_USER+0)			//���յ����ݰ���Ϣ
#define	WM_MESSAGE_SENDED	(WM_USER+1)			//���ͳ������ݰ���Ϣ


//���߳�������պͷ��͵��̴߳��ݲ���
typedef	struct
{
	HWND	hwnd;					//���̵߳Ĵ��ھ��
	BOOL	bContinue;				//״ָ̬ʾ����	
	SOCKET	WinSocket;				//����socket
	SOCKADDR_IN local_sin,			//����socket��ַ   
              accept_sin;			//�������ӵ�socket��ַ
}
PARAMS,*PPARAMS;

char szServerA[100];                // ASCII string 
TCHAR szServerW[100];               // Unicode string
TCHAR szError[100];                 // Error message string
TCHAR		szSendBuffer[100];		// �̼߳乲�����
SOCKET ClientSock = INVALID_SOCKET; // �������Ϳͻ���ͨ�ŵ�Socket

BOOL WINAPI MainDlgProc( HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam );
void	NetworkThread(PVOID	pvoid);

int WINAPI WinMain (
               HINSTANCE hInstance,    // ʵ�����
              HINSTANCE hPrevInstance,// NULL
              LPTSTR lpCmdLine,       // �������г����������
              int nCmdShow)           // ������������ʾ��ʽ
{
  
	WNDCLASS wc;

   GetClassInfo( NULL, WC_DIALOG, &wc );          //  Get dialog box class info

   wc.hInstance = hInstance;
   wc.lpszClassName = "Client";                 //  so's we can superclass it
   wc.lpfnWndProc = DefDlgProc;               //  all this for an icon...sheesh
   wc.hIcon = LoadIcon( hInstance, MAKEINTRESOURCE( 100 ) );
   wc.cbWndExtra = DLGWINDOWEXTRA;

   if( ! RegisterClass( &wc ) )                  //  Quit if registration fails
      return( FALSE );

   //hInst = hInstance;               //  Now just start the main dialog and exit
   DialogBox( hInstance, MAKEINTRESOURCE(ID_CLIENT), NULL, MainDlgProc );
 
  return TRUE;
}

// === Main Dialog Box ========================================================

BOOL WINAPI MainDlgProc( HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam )
{   
	static 	SOCKADDR_IN local_sin,              // ����socket��ַ
              accept_sin;             // �������Ӷ���ĵ�ַ

	static SOCKADDR_IN destination_sin;        // ��������socket��ַ
	static PHOSTENT phostent = NULL;           // ָ���������HOSTENT�ṹ
	static WSADATA WSAData;                    // ������Winsocketִ�к��ϸ��
	static PARAMS	params;
	static SOCKET WinSocket = INVALID_SOCKET;  // Window socket
		 
   switch( msg )
   {

   case WM_INITDIALOG:						//��ʼ���Ի�����Ϣ
	   {
   		// ��ʼ��Winsocket. 
		  if (WSAStartup (MAKEWORD(1,1), &WSAData) != 0) 
		  {
			wsprintf (szError, TEXT("WSAStartup failed. Error: %d"), 
					  WSAGetLastError ());
			MessageBox (NULL, szError, TEXT("Error"), MB_OK);
			return FALSE;
		  }

		  //����һ��TCP/IP socket, WinSocket.
		  if ((WinSocket = socket (AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET) 
		  {
			wsprintf (szError, TEXT("Allocating socket failed. Error: %d"), 
					  WSAGetLastError ());
			MessageBox (NULL, szError, TEXT("Error"), MB_OK);
			return FALSE;
		  }

		  // ��д����socket��ַ��Ϣ
		  local_sin.sin_family = AF_INET;
		  local_sin.sin_port = htons (PORTNUM);  
		  local_sin.sin_addr.s_addr = htonl (INADDR_ANY);

		  // �ѱ��ص�ַ�󶨵�WinSocket.
		  if (bind (WinSocket, 
					(struct sockaddr *) &local_sin, 
					sizeof (local_sin)) == SOCKET_ERROR) 
		  {
			wsprintf (szError, TEXT("Binding socket failed. Error: %d"), 
					  WSAGetLastError ());
			MessageBox (NULL, szError, TEXT("Error"), MB_OK);
			closesocket (WinSocket);
			return FALSE;
		  }

		  // ����һ��socket��������������
		  if (listen (WinSocket, MAX_PENDING_CONNECTS) == SOCKET_ERROR) 
		  {
			wsprintf (szError, 
					  TEXT("Listening to the client failed. Error: %d"),
					  WSAGetLastError ());
			MessageBox (NULL, szError, TEXT("Error"), MB_OK);
			closesocket (WinSocket);
			return FALSE;
		  }
		  params.hwnd = hDlg;
		  params.bContinue = TRUE;
		  params.WinSocket = WinSocket;
		  params.accept_sin =  accept_sin ;
		  params.local_sin = local_sin ;
				
		  _beginthread(NetworkThread,0,&params);
  
	   }
		 
   	case WM_PAINT:              //  �Ի��������Ϣ
		{
		PAINTSTRUCT ps;
		HDC hDC;
		RECT Rect;

		hDC = BeginPaint( hDlg, &ps );
		GetClientRect( hDlg, &Rect );
		FillRect( hDC, &Rect, GetProp( hDlg, "BRUSH" ) );
		EndPaint( hDlg, &ps );
		return	0;
		}

	case	WM_MESSAGE_REVED:		//�����߳̽��յ���Ϣ
		{
		//�ѽ��յ�����Ϣ�ڶԻ�������ʾ
		SetWindowText( GetDlgItem(hDlg,IDC_EDIT1), szServerW );
		return	0;
		}

    case	WM_COMMAND:				//����ӿؼ�����������Ϣ
		{
	
		if(wParam == IDC_SEND )
		{
			//����û�������ͣ����������ȡ���ı�
			GetWindowText(GetDlgItem(hDlg,IDC_EDIT),
				  szSendBuffer,GetWindowTextLength(GetDlgItem(hDlg,IDC_EDIT)));

			//�����ı�
			if (send (ClientSock, szSendBuffer, strlen (szSendBuffer) + 1, 0)
					   == SOCKET_ERROR) 
			{
				wsprintf (szError, 
						  TEXT("Sending data to the server failed. Error: %d"),
						  WSAGetLastError ());
				MessageBox (NULL, szError, TEXT("Error"), MB_OK);
			}

		 }
		else if(wParam == IDCANCEL)
		{
			EndDialog( hDlg, TRUE ); 
		}
		return( TRUE );
		}
	
   }
   return FALSE;
}   
                      

//�����̣߳�����socketͨ��
void	NetworkThread(PVOID	pvoid)
{
	volatile	PPARAMS	pparams;
	int index = 0,                      // ����
			  iReturn;                  

	int accept_sin_len;                 // accept_sin�ĳ���



	pparams = (PPARAMS)pvoid;

	accept_sin_len = sizeof (pparams->accept_sin);

	// �������Է��͵�WinSocket����������
	ClientSock = accept (pparams->WinSocket, 
					   (struct sockaddr *) &(pparams->accept_sin), 
					   (int *) &accept_sin_len);

	// ֹͣ���տͻ��˵���������
	closesocket (pparams->WinSocket);
	if (ClientSock == INVALID_SOCKET) 
	{
		wsprintf (szError, TEXT("Accepting connection with client failed.")
				  TEXT(" Error: %d"), WSAGetLastError ());
		MessageBox (NULL, szError, TEXT("Error"), MB_OK);

	  }

	for (;;)
	  {
		// �ӿͻ��˽�������
		iReturn = recv (ClientSock, szServerA, sizeof (szServerA), 0);

		// ��������ݣ�����ʾ֮
		if (iReturn == SOCKET_ERROR)
		{
		  wsprintf (szError, TEXT("No data is received, recv failed.")
					TEXT(" Error: %d"), WSAGetLastError ());
		  MessageBox (NULL, szError, TEXT("Server"), MB_OK);
		  break;
		}
		else if (iReturn == 0)
		{
		  MessageBox (NULL, TEXT("Finished receiving data"), TEXT("Server"),
					  MB_OK);
		  break;
		}
		else
		{
		  // ��ASCII���ַ���ת����Unicode���ַ���
		  for (index = 0; index <= sizeof (szServerA); index++)
			szServerW[index] = szServerA[index];

	
		  // ��ʾ�ӷ��������յ����ַ���
		  // MessageBox (NULL, szClientW, TEXT("Received From Server"), MB_OK);
		  SendMessage(pparams->hwnd,WM_MESSAGE_REVED,0,0);
		}
	  } 

	  // ����һ����Ϣ���ͻ���
	

	  // ��ֹClientSock�Ľ��պͷ��Ͷ˿�.
	  shutdown (ClientSock, 0x02);

	  // �ر�ClientSock.
	  closesocket (ClientSock);

	  WSACleanup ();


}
