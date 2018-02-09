
#include "windows.h"
#include "winsock.h"
#include "resource.h"
#include "PROCESS.H"

#define	STATUS_SEND		0
#define	STATUS_REVE		1
#define	STATUS_DONE		2

#define	WM_MESSAGE_REVED	(WM_USER+0)
#define	WM_MESSAGE_SENDED	(WM_USER+1)

#define PORTNUM         5000          // �˿ں�
#define HOSTNAME        "localhost"   // ����������
                                      // ���ݷ������Ĳ�ͬ���������������


typedef	struct
{
	HWND	hwnd;
	BOOL	bContinue;
	SOCKET	ServerSock;
}
PARAMS,*PPARAMS;


// === Function Prototypes ====================================================

BOOL WINAPI MainDlgProc( HWND, UINT, WPARAM, LPARAM );
void	NetworkThread(PVOID	pvoid);

// === Global Variables =======================================================

HINSTANCE	hInst;
char		szClientA[100];                // ASCII�봮
TCHAR		szClientW[100];               // Unicode��
TCHAR		szSendBuffer[100];
TCHAR szError[100];                 // ������Ϣ��


// === Application Entry Point ================================================

int APIENTRY WinMain( HINSTANCE hInstance, HINSTANCE hPrev, LPSTR lpCmd,
                      int nShow )
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

   hInst = hInstance;               //  Now just start the main dialog and exit
   DialogBox( hInstance, MAKEINTRESOURCE(ID_CLIENT), NULL, MainDlgProc );
   return( FALSE );
}

// === Main Dialog Box ========================================================

BOOL WINAPI MainDlgProc( HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam )
{   
	static SOCKET ServerSock = INVALID_SOCKET; // ��������socket
	static SOCKADDR_IN destination_sin;        // ��������socket��ַ
	static PHOSTENT phostent = NULL;           // ָ���������HOSTENT�ṹ
	static WSADATA WSAData;                    // ������Winsocketִ�к��ϸ��
	static PARAMS	params;

		 
   switch( msg )
   {
   case WM_INITDIALOG:
	   {
   	
		if (WSAStartup (MAKEWORD(1,1), &WSAData) != 0) 
		{
			wsprintf (szError, TEXT("WSAStartup failed. Error: %d"), 
					  WSAGetLastError ());
			MessageBox (NULL, szError, TEXT("Error"), MB_OK);
			return FALSE;
		 }
		 return	0;
	   }
		 
	    

	case WM_PAINT:              //  Paint dialog box background COLOR_BTNFACE
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

	case	WM_MESSAGE_REVED:
		{
		SetWindowText( GetDlgItem(hDlg,IDC_EDIT1), szClientW );
		return	0;
		}

    case	WM_COMMAND:
		{
		if( wParam == IDC_CONNECT )                          //  ���ӷ�����
		{
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
			  params.hwnd = hDlg;
			  params.bContinue = TRUE;
			  params.ServerSock = ServerSock;
			  _beginthread(NetworkThread,0,&params);
		}
		
		else if(wParam == IDC_SEND )
		{
			GetWindowText(GetDlgItem(hDlg,IDC_EDIT),
				  szSendBuffer,GetWindowTextLength(GetDlgItem(hDlg,IDC_EDIT)));

			if (send (ServerSock, szSendBuffer, strlen (szSendBuffer) + 1, 0)
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
                                                   //  End of MainDlgProc()
void	NetworkThread(PVOID	pvoid)
{
	volatile	PPARAMS	pparams;
	int index = 0,                      // ����
			  iReturn;                        // recv�����ķ���ֵ

	pparams = (PPARAMS)pvoid;

	for (;;)
	  {
		// �ӷ�����������Ϣ
		iReturn = recv (pparams->ServerSock, szClientA, sizeof (szClientA), 0);

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
		  // MessageBox (NULL, szClientW, TEXT("Received From Server"), MB_OK);
		  SendMessage(pparams->hwnd,WM_MESSAGE_REVED,0,0);

		}
	  }

	  // ��ֹ������������Ϣ
	  shutdown (pparams->ServerSock, 0x00);

	  // �رշ�����socket.
	  closesocket (pparams->ServerSock);

	  WSACleanup ();



}






























































