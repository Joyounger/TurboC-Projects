/****************************************************/
/* �ļ��� scan.h                                    */
/* TINY��������ɨ�����ӿ�						    */
/****************************************************/

#ifndef _SCAN_H_
#define _SCAN_H_

/* һ���Ǻŵ����ĳ��� */
#define MAXTOKENLEN 40

/* tokenString���������洢ÿ���Ǻ� */
extern char tokenString[MAXTOKENLEN+1];

/* getToken��������Դ�ļ��е��¸��Ǻ�
 */
TokenType getToken(void);

#endif
