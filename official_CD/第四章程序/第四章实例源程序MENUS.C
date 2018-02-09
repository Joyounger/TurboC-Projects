
#include <conio.h>
#include<dos.h>
#include <graphics.h>
#include<stdio.h>
#include<stdlib.h>

/*����һЩ����*/
#define NO 0
#define YES 1
#define NONE -1
#define MAX_MENU 7/*��������ı�����ڲ˵���ʾ�������*/

/*ȫ�ֲ���*/
int X,Y;
int Selection;
int button,x,y;
void *p;
size_t area;

/*�˵��ṹ*/
struct MENU
{
      /*���Ͻ�*/
      int x;	
      int y;
      /*���½�*/
      int x1;
      int y1;
      unsigned char Active[MAX_MENU];/* �˵����Ƿ񼤻�ı�־*/
      char *head;/* �˵������֣���ѡ�*/
};

int MouseOn(struct MENU *t);
void Process();
void Xorbar(int sx,int sy,int ex,int ey,int c);
void Show();
void Hide();
void Status();

/* ͨ������Ĳ����û����Ըı䰴ť��λ�á�*/

struct MENU File={200,110,250,130};
struct MENU Edit={295,110,345,130};
struct MENU Help={390,110,440,130};
struct MENU Close={600,10,620,30};
struct MENU Neeraj={380,300,460,315};

void main()
{
 int gd = DETECT,gm;
 
 /*��ʼ��ͼ�ν���*/
 initgraph(&gd, &gm,"");
 Selection=NONE;
 /*�Ƿ񼤻�˵��YES��ʾ���NO��ʾȡ������*/
 for(gd=0;gd<MAX_MENU;gd++)
 {
	File.Active[gd]=YES;
	Edit.Active[gd]=YES;
	Help.Active[gd]=YES;
	Close.Active[gd]=YES;
 }
 /*�˵�������֣��û������޸�����ʾ����������*/
 File.head="[ File Menu ]";
 Edit.head="[ Edit Menu ]";
 Help.head="[ Help Menu ]";
 Close.head="[ Exit ]";
 /*Ϊ��������ڴ�*/
 area=imagesize(0,0,150,150);
 p=malloc(area);
 
 /*û���㹻���ڴ�*/
 if(p==NULL)
 {
	closegraph();
	restorecrtmode();
	printf("Not Enough Memory !\n try to reduce the area used.\n");
	exit(1);
 }
 setfillstyle(1,7);
 bar(0,0,640,480);
 X=2;Y=2;
 /*�����������*/
 Process();
 End();
 /*�ر�ͼ����ʾ��ʽ*/
 closegraph();
 printf("GUI-MENUS By Neeraj Sharma  -   n21@indiatimes.com\n");
 getch();
}

void Process()
{
	/*�ļ��˵�����Ӳ˵���*/
	char *fm[]={"New","Open","Save","Save as","Exit"};
	/*�༭�˵�����Ӳ˵���*/
	char *em[]={"Undo","Cut","Copy","Paste","Delete","Select All"};
	/*�����˵�����Ӳ˵���*/
	char *hm[]={"Help Topics","About Menus"};
	
	/* ����û�������ı���˲˵����ֵ����ôҲͬ����Ҫ�ı��ں���
	   �е�PullMenuDown(char *,int)ֵ*/
	char KEY;
	/* ���Ƹ����˵� */
	Win(140,100,500,140);
	/* �����ļ��˵��� */
	DrawBtn(&File,"File");
	/* ���Ʊ༭�˵��� */
	DrawBtn(&Edit,"Edit");
	/* ���ư����˵���*/
	DrawBtn(&Help,"Help");
	settextstyle(0,0,0);
	outtextxy(Close.x+7,Close.y+7,"x");
	Comments();
	Show();
	/* ����������ʾ�û����Լ����ȡ������һ���˵���
	   �����'save as'��ȡ������*/
	File.Active[3]=NO;/* Save as*/
	Edit.Active[0]=NO;/* ȡ������*/
	while(1)
	{
		if(kbhit())
		{
			KEY=getch();
			/*����û�����"ESC"����"-"�Ƴ�����*/
			if(KEY==27|| KEY==45) break;
		}
		Status();
		/*�������������*/
		if(button==1)
		{
		  /*�û����Close*/
		  if(MouseOn(&Close)) break;
		  /*�û����File*/
		  if(MouseOn(&File))
		  {
			  PullMenuDown(fm,5,&File);/* �˵���Ϊ5*/
			  if(Selection==NONE) Msg("No Selection !");
			  else Msg(fm[Selection]);
			  if(Selection==4) break;
		  }
		  /*�û����Edit*/
		  if(MouseOn(&Edit))
		  {
			  PullMenuDown(em,6,&Edit);/* �˵���Ϊ6*/
			  if(Selection==NONE) Msg("No Selection !");
			  else Msg(em[Selection]);
		  }
		  /*�û����Help*/
		  if(MouseOn(&Help))
		  {
			  PullMenuDown(hm,2,&Help);/* �˵���Ϊ2*/
			  if(Selection==NONE) Msg("No Selection !");
			  else Msg(hm[Selection]);
			  if(Selection==1) Msg("* Programming By NEERAJ SHARMA *");
		  }
		}
		else
		{
			/*������Ǹ��˵����ϣ��͸�����ʾ֮*/
			
			/*������ʾ�ļ��˵���*/
			if(MouseOn(&File)) Hlight(&File);
			/*������ʾ�༭�˵���*/
			if(MouseOn(&Edit)) Hlight(&Edit);
			/*������ʾ�����˵���*/
			if(MouseOn(&Help)) Hlight(&Help);
			/*������ʾ�رղ˵���*/
			if(MouseOn(&Close)) Hlight(&Close);
			if(MouseOn(&Neeraj))
			{
				Msg("Hello, everyone");
				while(MouseOn(&Neeraj))
				{
					Status();
					if(button!=0) break;
					if(kbhit()) break;
				 }
				 Msg("");
			}
		}
	}
	Msg(":: Bye Bye ::");
}

/* ���Ʋ˵��� */
DrawBtn(struct MENU *tm,char *txt)
{
	/*���û��Ƶ���ɫ*/
	setcolor(0);
	/*�����ı���ʽ*/
	settextstyle(2,0,4);
	/*���txt�ı�*/
	outtextxy(tm->x+15,tm->y+4,txt);
}
/*������ʾ����*/
Hlight(struct MENU *t)
{
/* ���� */
 winp(t,0);
/* ��ʾ�˵�������  */
 Msg(t->head);
 /*��û�����������£�һֱ����*/
 while(MouseOn(t))
 {
	Status();
	if(button!=0) break;
	if(kbhit()) break;
 }
 /*��������*/
 setcolor(7);
 Hide();
 rectangle(t->x,t->y,t->x1,t->y1);
 Msg("");
 /*�ػ�*/
 Show();
}

int MouseOn(struct MENU *t)
{
 int flag;
 flag=0;
 if(x>=t->x && x<=t->x1 && y>=t->y && y<=t->y1) flag=1;
 return flag;
}

void Xorbar(int sx,int sy,int ex,int ey,int c)
{
 int i;
 setwritemode(1);
 setcolor(c);
 Hide();
 for(i=sy;i<=ey;i++)
 line(sx,i,ex,i);
 Show();
 setwritemode(0);
}

Push(struct MENU *tm)
{
	winp(tm,1);
	while(button==1)
	{
		Status();
		if(kbhit()) break;
	}
}

Up(struct MENU *tm)
{
	setcolor(7);
	Hide();
	rectangle(tm->x,tm->y,tm->x1,tm->y1);
	Show();
	while(button==1)
	{
		Status();
		if(kbhit()) break;
	}
}

/*���������˵�*/
PullMenuDown(char *H[],int n,struct MENU *tm)
{
	int i;
	i=n*20;
	/* ���汳��*/
	Push(tm);
	/* ���������*/
	Hide();
	/* �Ѳ˵���λͼ�����ڴ棬�Թ��ջص�����ʱ���»���*/
	getimage(tm->x+1,tm->y1+2,tm->x+100,tm->y1+i+5,p);
	/* ���������������*/
	Win(tm->x+1,tm->y1+2,tm->x+100,tm->y1+i+5);
	setcolor(0);
	settextstyle(2,0,4);
	/*���������˵���*/
	for(i=0;i<n;i++)
	{
		if(!tm->Active[i])
		{
			setcolor(15);
			outtextxy(tm->x+15,tm->y1+9+i*20,H[i]);
			setcolor(0);
			outtextxy(tm->x+14,tm->y1+8+i*20,H[i]);
		}
		else    outtextxy(tm->x+14,tm->y1+8+i*20,H[i]);
	}
	/*��ʾ�����*/
	Show();
	while(button==1)  Status();
	HandleMenu(n,tm);
}

PullMenuUp(int n,struct MENU *tm)
{
	int i;
	/*���������*/
	Hide();
	/* �Ѷ������ݵ�ԭ�˵���λͼ���»��Ƴ���*/
	putimage(tm->x+1,tm->y1+2,p,COPY_PUT);
	/*��ʾ�����*/
	Show();
	Up(tm);
}
/* ����˵�������� */
HandleMenu(int n,struct MENU *tm)
{
	int i;
	char Key;
	setcolor(0);
	Selection=NONE;
	i=0;
	while(1)
	{
		Status();
		if(button==1) break;
		for(i=0;i<n;i++)
		{
			Status();
			if(MouseOn1(tm->x+4,tm->y1+6+i*20,tm->x+97,tm->y1+22+i*20))
			{
			    Xorbar(tm->x+3,tm->y1+6+i*20,tm->x+97,tm->y1+22+i*20,14);
			    while(MouseOn1(tm->x+4,tm->y1+6+i*20,tm->x+97,tm->y1+22+i*20))
			    {
				Status();
				if(button==1)
				{
					if(tm->Active[i]==YES)
					{
						Selection=i;
						break;
					}
					else
					Selection=NONE;
				}
				if(kbhit()) break;
			    }
			    Xorbar(tm->x+3,tm->y1+6+i*20,tm->x+97,tm->y1+22+i*20,14);
			}
		}
	}
	PullMenuUp(n,tm);
}

MouseOn1(int cx,int cy,int cx1,int cy1)
{
 int flag;
 flag=0;
 if(x>=cx && x<=cx1 && y>=cy && y<=cy1) flag=1;
 return flag;
}

/* Mouse & win routines */

union REGS i,o;
struct SREGS ss;

/* ��ʾ����꺯�� */
void Show()
{
	 i.x.ax=1;
	 int86(0x33,&i,&o);
}

/* ��������꺯��*/
void Hide()
{
	 i.x.ax=2;
	 int86(0x33,&i,&o);
}

/* ������״̬���� */
void Status()
{
	 i.x.ax=3;
	 int86(0x33,&i,&o);
	 button=o.x.bx;
	 x=o.x.cx;
	 y=o.x.dx;
}
/*���Ʋ˵���ı����ͱ߿�*/
winp(struct MENU *t,int state)
{
   Hide();
   /*�Ǹ���ģʽ*/
   if(state==1)
   {
   	/*�������߿���*/
	setcolor(0);
	line(t->x,t->y,t->x,t->y1);
	line(t->x,t->y,t->x1,t->y);
	/*�������߿���*/
	setcolor(15);
	line(t->x1,t->y,t->x1,t->y1);
	line(t->x,t->y1,t->x1,t->y1);
  }
  /*����ģʽ*/
  else
  {
	/*�������߿���*/
	setcolor(15);
	line(t->x,t->y,t->x,t->y1);
	line(t->x,t->y,t->x1,t->y);
	/*�������߿���*/
	setcolor(0);
	line(t->x1,t->y,t->x1,t->y1);
	line(t->x,t->y1,t->x1,t->y1);
  }
  Show();
}
/*����һ����������*/
Win(int sx,int sy,int ex,int ey)
{
   Hide();
   /*����������ɫ*/
   setfillstyle(1,7);
   /*����һ������*/
   bar(sx,sy,ex,ey);
   /*�������߿���*/
   setcolor(15);
   line(sx,sy,sx,ey);
   line(sx,sy,ex,sy);
   /*�������߿���*/
   setcolor(0);
   line(ex,sy,ex,ey);
   line(sx,ey,ex,ey);
   Show();
}

Msg(char *m)
{
 int l;
 setcolor(14);
 rectangle(18,438,622,477);
 setfillstyle(1,0);
 bar(20,440,620,475);
 if(MouseOn(&Neeraj))
 {
	 settextstyle(2,0,7);
	 setcolor(11);
	 l=textwidth(m)/2;
	 outtextxy(320-l,440,m);
 }
 else
 {
	 settextstyle(2,0,8);
	 setcolor(15);
	 l=textwidth(m)/2;
	 outtextxy(320-l,440,m);
	 outtextxy(321-l,441,m);
 }
}

Comments()
{
   setfillstyle(1,0);
   bar(140,5,500,40);
   setcolor(14);
   rectangle(140,5,500,40);
   settextstyle(2,0,8);
   setcolor(15);
   outtextxy(260,10,"GUI-MENUS");
   setfillstyle(1,8);
   bar(140,150,502,402);
   setcolor(14);
   rectangle(138,148,502,402);
   Msg("Welcome !");
   settextstyle(2,0,5);
   setcolor(10);
   outtextxy(158,180,"Hello Friends !");
   settextstyle(2,0,4);
   outtextxy(260,200,"Here i am with my menu program.");
   outtextxy(158,220,"As you can see it's like windows menu,you can");
   outtextxy(158,240,"add more menus to it. It's just a concept for");
   outtextxy(158,260,"building more menus. I hope you will like my effort,");
   outtextxy(158,280,"email me at :      n21@indiatimes.com");
   setcolor(14);
   outtextxy(370,300,"- Neeraj Sharma");
   line(380,304+textheight("A"),380+textwidth("Neeraj Sharma"),304+textheight("A"));
}

End()
{
  int i,j;
  setcolor(0);
  Hide();
  for(j=0;j<40;j++)
  {
	for(i=0;i<481;i+=20)
	   line(0,0+i+j,640,j+0+i);
	delay(100);
	if(kbhit()) if(getch()==27) break;
  }
  settextstyle(2,0,5);
  setcolor(15);
  outtextxy(220,180,"Thanks for being with us !");
  Show();
  delay(3000);
}

