/****************************************************/
/* �ļ���scan.c                                     */
/* TINYɨ������ִ�д���							*/
/****************************************************/

#include "globals.h"
#include "util.h"
#include "scan.h"

/* ɨ����������Զ�����״̬ */
typedef enum
   { START,INASSIGN,INCOMMENT,INNUM,INID,DONE }
   StateType;

/* �������浱ǰ�����ֻ��߱�ʶ�� */
char tokenString[MAXTOKENLEN+1];

/* BUFLEN����Դ�ļ����е�����ַ��� */
#define BUFLEN 256

static char lineBuf[BUFLEN]; /* ������ǰ�� */
static int linepos = 0; /* ����LineBuf�еĵ�ǰλ�� */
static int bufsize = 0; /* ��ǰ�����б�����ַ��� */
static int EOF_flag = FALSE; /* ������EOF���ļ�������������ungetNextChar�Ĳ��� */

/* getNextCharlineBufȡ����һ���ǿո���ַ�����������˻����е�ȫ���ַ���
   �Ͱ��µ�һ�ж��뻺�� */
static int getNextChar(void)
{ if (!(linepos < bufsize))
  { lineno++;
    if (fgets(lineBuf,BUFLEN-1,source))
    { if (EchoSource) fprintf(listing,"%4d: %s",lineno,lineBuf);
      bufsize = strlen(lineBuf);
      linepos = 0;
      return lineBuf[linepos++];
    }
    else
    { EOF_flag = TRUE;
      return EOF;
    }
  }
  else return lineBuf[linepos++];
}

/* ungetNextChar��һ���ַ��˻ص�lineBuf������ */
static void ungetNextChar(void)
{ if (!EOF_flag) linepos-- ;}

/* �����ֱ�*/
static struct
    { char* str;
      TokenType tok;
    } reservedWords[MAXRESERVED]
   = {{"if",IF},{"then",THEN},{"else",ELSE},{"end",END},
      {"repeat",REPEAT},{"until",UNTIL},{"read",READ},
      {"write",WRITE}};

/* T I N Y�Ա����ֵ�ʶ����ͨ�����Ƚ����ǿ����Ǳ�ʶ����
 * ֮�����ڱ����ֱ��в�����������ɵġ�
 * ���ǵ�ɨ�����ʹ����һ�ַǳ����ķ���������������
 * ����˳��ӿ�ͷ����β������������С�ͱ�񲻳����⡣
 */

static TokenType reservedLookup (char * s)
{ int i;
  for (i=0;i<MAXRESERVED;i++)
    if (!strcmp(s,reservedWords[i].str))
      return reservedWords[i].tok;
  return ID;
}

/****************************************/
/* ���������������  */
/****************************************/
/* �������ַ�������ͼ8.2.1
 * �е�DFA������һ����ʶ��ļǺ�
 */
TokenType getToken(void)
{  /* tokenString������ */
   int tokenStringIndex = 0;
   /* ���浱ǰҪ���صļǺ� */
   TokenType currentToken;
   /* ��ǰ��״̬�����Ǵ�START��ʼ */
   StateType state = START;
   /* �Ƿ񱣴浽tokenString�ı�� */
   int save;
   while (state != DONE)
   { int c = getNextChar();
     save = TRUE;
     switch (state)
     { case START:
         if (isdigit(c))
           state = INNUM;
         else if (isalpha(c))
           state = INID;
         else if (c == ':')
           state = INASSIGN;
         else if ((c == ' ') || (c == '\t') || (c == '\n'))
           save = FALSE;
         else if (c == '{')
         { save = FALSE;
           state = INCOMMENT;
         }
         else
         { state = DONE;
           switch (c)
           { case EOF:
               save = FALSE;
               currentToken = ENDFILE;
               break;
             case '=':
               currentToken = EQ;
               break;
             case '<':
               currentToken = LT;
               break;
             case '+':
               currentToken = PLUS;
               break;
             case '-':
               currentToken = MINUS;
               break;
             case '*':
               currentToken = TIMES;
               break;
             case '/':
               currentToken = OVER;
               break;
             case '(':
               currentToken = LPAREN;
               break;
             case ')':
               currentToken = RPAREN;
               break;
             case ';':
               currentToken = SEMI;
               break;
             default:
               currentToken = ERROR;
               break;
           }
         }
         break;
       case INCOMMENT:
         save = FALSE;
         if (c == EOF)
         { state = DONE;
           currentToken = ENDFILE;
         }
         else if (c == '}') state = START;
         break;
       case INASSIGN:
         state = DONE;
         if (c == '=')
           currentToken = ASSIGN;
         else
		   /*�˻ص������� */
           ungetNextChar();
           save = FALSE;
           currentToken = ERROR;
         }
         break;
       case INNUM:
         if (!isdigit(c))
         { /* �˻ص������� */
           ungetNextChar();
           save = FALSE;
           state = DONE;
           currentToken = NUM;
         }
         break;
       case INID:
         if (!isalpha(c))
         { /* �˻ص������� */
           ungetNextChar();
           save = FALSE;
           state = DONE;
           currentToken = ID;
         }
         break;
       case DONE:
       default: 
         fprintf(listing,"Scanner Bug: state= %d\n",state);
         state = DONE;
         currentToken = ERROR;
         break;
     }
     if ((save) && (tokenStringIndex <= MAXTOKENLEN))
       tokenString[tokenStringIndex++] = (char) c;
     if (state == DONE)
     { tokenString[tokenStringIndex] = '\0';
       if (currentToken == ID)
         currentToken = reservedLookup(tokenString);
     }
   }
   if (TraceScan) {
     fprintf(listing,"\t%d: ",lineno);
     printToken(currentToken,tokenString);
   }
   return currentToken;
} 

