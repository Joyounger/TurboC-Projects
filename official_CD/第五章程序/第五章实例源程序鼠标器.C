/*Ϊ���ñ�ʵ�������ܼ���ͻ������̣�������������������ɢ���ߵ�ע�������Һ����˺ܶ�ʵ�ֵ�ϸ�ڣ����߶��ڿ�����ʮ���Ʊ༭���������ַ�"F"֮������ⲻҪ̫�Ծ�������Ȥ�Ķ��߿��Ժ����׵��������"bug"*/
#include<dos.h>
#include<conio.h>
#include<string.h>
#include<math.h>
#include<stdlib.h>
#include<stdio.h>
#define HEX 0
#define DEC 1
#define OCT 2
#define BIN 3

int index=0,choice=0;
/*��ʼ�������*/
void initmouse()
{
	_AX=0;
	geninterrupt(0x33);

}
/*��ʾ�����*/
void showmouse()
{
	_AX=1;
	geninterrupt(0x33);
}
/*�������*/
void hidemouse()
{
	_AX=2;
	geninterrupt(0x33);
}
/*ȡ���״̬�����λ��*/
void getmouse(int *button,int *x,int *y)
{
	_AX=3;
	_BX=0;
	geninterrupt(0x33);
	*button=_BL;
	*x=_CX;
	*y=_DX;
}
/*���ó����е������ַ���*/
int getnum(char c)
{
	int	j;
	char alpha_set[36]="0123456789abcdefghijklmnopqrstuvwzyz";
	for(j=0;j<36;j++)
	{
		if(alpha_set[j]==c)
			break;
	}
	return j;
}
/*������radix���Ƶ�����ת��Ϊʮ������*/
unsigned long convert_to_decimal(char *_num,int radix)
{
	int i,len;
	unsigned long dec=0;
	len=strlen(_num);
	len--;
	for(i=0;_num[i]!=NULL;i++,len--)
	{
		dec+=(getnum(_num[i])*pow(radix,len));
	}
	return dec;
}
/*�ڸ���������(x,y)��λ����ʾһ���ַ���*/
void display(int x,int y,char string[80])
{	gotoxy(x,y);
	cprintf(string);
}
/*���������Ұѱ���ɫ����Ϊ����ɫ*/
void make_screen_cyan()
{
	textbackground(CYAN);
	clrscr();
}
/*���Ƴ���ĳ�ʼ����*/
void start_screen()
{
	index=0;
	hidemouse();
	make_screen_cyan();
	textbackground(RED);
	textcolor(BLACK);
	display(20,1,"Radix Converter by NILOY MONDAL.");
	display(10,2,"Email:-yiwei@zju.edu.cn");
	display(30,3,"Press Escape to Quit");
	textbackground(CYAN);
	display(10,5,"Hexadecimal:- ");
	display(10,7,"Decimal    :- ");
	display(10,9,"Octal      :- ");
	display(10,11,"Binary     :- ");
	textbackground(BLUE);
	display(23,5,"                                                      ");
	display(23,7,"                                                      ");
	display(23,9,"                                                      ");
	display(23,11,"                                                      ");
	if(choice==HEX)
		gotoxy(24,5);
	else if(choice==DEC)
		gotoxy(24,7);
	else if(choice==OCT)
		gotoxy(24,9);
	else if(choice==BIN)
		gotoxy(24,11);
	showmouse();
}

void main()
{
	char text[80]="\0",buffer[80];
	char ch,*charhex,*chardec,*charoct,*charbin;
	int button,mousex,mousey,x,y;
	unsigned long deci;

	initmouse();
	start_screen();
	showmouse();
	while(1)
	{
		if(kbhit())
		{
			ch=getch();
/*ESCAPE����ASCIIֵ��27������û�����ESC���������*/
			if(ch==27)		
				break;
/*�����ɾ�������ҹ��û��Խ�������ı߽�*/
			if(ch=='\b'&&wherex()>=24)   
			{
/*�ѹ���˺�һ�񣬲��ҷ�һ���ո�������*/
				cprintf("\b");
				cprintf("%c",255);
/*�ٰѹ���˺�һ�񣬵���ɾ�����ַ�*/
				cprintf("\b");
				if(index!=0)
					index--;			
				text[index]=NULL;
			}
			else if(wherex()>=24&&ch>='0'&&ch<='f')
			{
				cprintf("%c",ch);
/*���ַ������ַ�����*/
				text[index]=ch;			
/*���������һλ����*/
				text[index+1]=NULL;	
				index++;
			}
/*���浱ǰ������*/
			x=wherex(),y=wherey();
/*�������Ȱ��ַ���ͨ��convert_to_decimal()ת��Ϊʮ��������֮�����stdlib.h�����ltoa()��ʮ������ת��Ϊ��������������������ơ��˽��ơ�ʮ�����Ƶȵ�.*/
			switch(choice)
			{
				case HEX:
					deci=convert_to_decimal(text,16);
					gotoxy(24,7);
					printf("%ld                             ",deci);
					gotoxy(24,11);
					printf("%s                              ",ltoa(deci,buffer,2));
					gotoxy(24,9);
					printf("%s                              ",ltoa(deci,buffer,8));
					break;
				case DEC:
					deci=atol(text);
					gotoxy(24,5);
					printf("%s                             ",ltoa(deci,buffer,16));
					gotoxy(24,9);
					printf("%s                             ",ltoa(deci,buffer,8));
					gotoxy(24,11);
					printf("%s                             ",ltoa(deci,buffer,2));
					break;
				case OCT:
					deci=convert_to_decimal(text,8);
					gotoxy(24,7);
					printf("%ld                             ",deci);
					gotoxy(24,5);
					printf("%s                             ",ltoa(deci,buffer,16));
					gotoxy(24,11);
					printf("%s                              ",ltoa(deci,buffer,2));
					break;
				case BIN:
					deci=convert_to_decimal(text,2);
					gotoxy(24,5);
					printf("%s                            ",ltoa(deci,buffer,16));
					gotoxy(24,7);
					printf("%ld                            ",deci);
					gotoxy(24,9);
					printf("%s                            ",ltoa(deci,buffer,8));
					break;
			}
			gotoxy(x,y);
		}
		getmouse(&button,&mousex,&mousey);
		mousex++,mousey++;
/*����Ĵ������û���ʲô�ط�����ʲô��ť*/
		if(mousex/8>23&&mousex/8<50&&mousey/16==2&&button&1==1)
		{
			choice=HEX;
			start_screen();
		}
		else if(mousex/8>23&&mousex/8<50&&mousey/16==3&&button&1==1)
		{
			choice=DEC;
			start_screen();
		}

		else if(mousex/8>23&&mousex/8<50&&mousey/16==4&&button&1==1)
		{
			choice=OCT;
			start_screen();
		}
		else if(mousex/8>23&&mousex/8<50&&mousey/16==5&&button&1==1)
		{
			choice=BIN;
			start_screen();
		}
	}

}
