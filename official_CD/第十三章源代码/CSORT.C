/************************************************************************

 ���ƣ� csort.c
                                                                      
  һ��������������ʾ6�������㷨�Ķ�����

  �����������Ҫ�����graphics.libͷ�ļ�������CGA��Hercules����Ӧ�ð�����
  ���ͷ�ļ��С�

**************************************************************************/

#include <conio.h>
#include <ctype.h>
#include <dos.h>
#include <graphics.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <time.h>

#define MAXNUM   200    /* Ҫ�����Ԫ�ص������Ŀ */
#define XAXIS    260    /* ͼ����ʾ��Ļ��x����ֵ */
#define YAXIS    15     /* ͼ����ʾ��Ļ��y����ֵ */
#define MAXPICKS 8      /* �û�����ѡ�����ѡ���� */
#define TIMES    3      /* ִ�еĴ��� */

int xaxis = XAXIS;
int yaxis = YAXIS;

/* �����㷨 */
enum sort {bubble, delayed, shell, shell_metzner,
           quick, insertion, all, stop};

/* �û�ѡ������� */
char *sorts[MAXPICKS] =
                 {"Bubble Sort", "Delayed Exchange Sort", "Shell Sort",
                  "Shell-Metzner Sort",QuickSort", "Insertion Sort",
                   "All", "Exit to Dos"};

/*****  ����ԭ��  *************************/

void main( void );
void driver( enum sort atype, int *m, int elements,
             int random, int delay_factor );
enum sort pick_sort( int *elements, int *random, int *delay_factor );
void Initialize( void );
void Setscreen( int *m, int elements, int random );
int  Swap_Pixels( int *m, int i, int j, int delay_factor );
int  gprintf( int *xloc, int *yloc, char *fmt, ... );
void print_menu( char *mysorts[] );
void get_number( int *elements, int *times, char *tstring, int *x, int *y );
void Showdata ( int *m );

void Bubble( int *m, int elements, int delay_factor );
void Delayed( int *m, int elements, int delay_factor );
void Shell_Metzner( int *m, int elements, int delay_factor );
void Quicksort( int *m, int left, int right, int delay_factor );
void Insertion( int *m, int elements, int delay_factor );
void Shell( int *m, int elements, int delay_factor );


/*****  main����  ***************************************/
/*                                                  */
/****************************************************/

void main( void )
{
 int array[MAXNUM];    /*  Ҫ���������  */
 int elements;         /*  Ԫ�صĸ���       */
 int random;           /*  ������������� */
 int delay_factor;     /*  �ӳ�����0-1000     */

 enum sort stype = all;
 /* ��ʼ��ͼ�ν��� */
 Initialize();
 while( stype != stop )
   {
    random = 0;
    elements = 0;
    delay_factor = 0;

	/* ��ʾ�û�ѡ�������㷨������������ӳ����� */
    stype = pick_sort( &elements, &random, &delay_factor );
    if ( stype != stop )
      {
       driver( stype, array, elements, random, delay_factor );
       /* Showdata( array ); */
       delay( 1350 );
      }
   }
 closegraph();
}


/*****  pick_sort����  *******************************************************

  ��ʾһ���򵥵Ĳ˵�ѡ���ʾ�û�����һЩ���в���

  ��������:  main

  ����: print_menu
         gprintf
         get_number


  ����ֵ  :  ��Ҫ�������㷨(�����������㷨�е�һ�֡����е������㷨�����˳�)

  ����:

    *elements					Ԫ�ظ�����С��200
    *random						����ʽ���������������
    *delay_factor				�ӳ�����
*************************************************************************/

enum sort pick_sort( int *elements, int *random, int *delay_factor )
{
/* ���û�����ʾ */
 static char query1[] = "Which Sort (1-8)?";
 static char query2[] = "How Many Elements < 200?";
 static char query3[] = "(R)andom or (W)orst Case?";
 static char query4[] = "Delay Factor (0-1000)?";

 static char achar[2] = "x";
 char bchar = 0;
 char nstring[TIMES + 1];  /* �����ֵȼ۵��ַ���  */
 int tens = TIMES; /* ʮ��ָ�� */
 int *tpower;
 int x = 50;
 int y = 30;
 char pick = 0;
 int x2;
 int i;    /* ѭ������ */
 tpower = &tens;

 cleardevice();
 print_menu( sorts );

 /************** ȡ��������� *************************/
 gprintf( &x, &y, query1 );
 while ( pick <= 48 || pick >= 57 )  /* ��������1-8 */
  {
   pick = getch();
  }
 achar[0] = pick;
 x2 = x + 4 + textwidth( query1 );
 outtextxy( x2, y, achar );

 if ( pick != 56 )
   {
    y = 100;

    /******** ȡ�����������Ԫ�ظ��� *****/
    gprintf( &x, &y, query2 );
    x2 = x + 4 + textwidth( query2  );
    for ( i = 0; i < TIMES + 1; i++ )
      nstring[i] = 0;        /* ���ַ�����ʼ��Ϊ�� */

    get_number( elements, tpower, nstring, &x2, &y );
    if ( *elements == 0 || *elements > MAXNUM ) *elements = MAXNUM;

    y += textheight("H" ) + 1;

    /****** �����ģʽ�������ģʽ ***********/
    gprintf( &x, &y, query3 );
    bchar = 0;
    while( bchar != 82 && bchar != 87 )
      {
       bchar = toupper( getch( ) );
       if ( bchar == 13 ) bchar = 82;
      }
    *random = ( bchar ^ 87 ); /* XOR����Ƿ������ģʽ */
    achar[0] = bchar;
    x2 = x + 4 + textwidth( query3 );
    outtextxy( x2, y, achar );

    y += textheight( "H" ) + 1;

    /****** ȡ���ӳ�������Ŀ  ******************/
    gprintf( &x, &y, query4 );
    x2 = x + 4 + textwidth( query4 );
    *tpower = TIMES;
    for ( i = 0; i < TIMES + 1; i++ )
      nstring[i] = 0;        /* ���ַ�����ʼ��Ϊ�� */

    get_number( delay_factor, tpower, nstring, &x2, &y );

   }
  switch( pick - 48 )
    {
     case 1:
        return( bubble );
     case 2:
        return( delayed );
     case 3:
        return( shell );
     case 4:
        return( shell_metzner );
     case 5:
        return( quick );
     case 6:
        return( insertion );
     case 7:
        return( all );
     default:
        return( stop );
    }
}

/*****  print_menu����  *****************************************

   ��ͼ�ν�������ʾѡ��˵�
   ��Ļ��������Ƶģ�

                   1. Bubble Sort
                   2. Delayed Exchange Sort
                   3. Shell Sort
                   4. Shell Metzner Sort
                   5. Quicksort
                   6. Insertion Sort
                   7. All
                   8. Exit to Dos

    ������: pick_sort

*************************************************************/

void print_menu( char *mysorts[] )
{
 int x, y;   /* ��Ļ������ */
 int i;
 x = 240;
 y = 10;

 for ( i = 0; i < MAXPICKS; i++ )
   {
    gprintf( &x, &y, "%d. %s", i+1, mysorts[i] );
    y += textheight ( "H" ) + 1;
   }
}

/*****  get_number����  ******************************************************

һ���ݹ�ĳ��򣬰Ѵ�getch()���յ�������ʾ����Ļ�ϡ�
�������ֻ���մ�'0'��'9'���ַ��ͻس����������ַ��������ԡ�

    ������: pick_sort, get_number

    ����:

    int *a_number   һ��ָ��������ָ�룬���ظ����ú���ʹ��

    int *times      get_number��󱻵��õĴ�������Ϊһ�������ݹ�ı�־��
					����û������������������ֻ��߰��»س�������ʾ���������

    char *tstring   ����*a_number���ֵĵȼ��ַ�����ʽ��
                    ���磬���*a_number = 123,��ô*tstring = "123"��
					�ڳ�ʼ���õ�ʱ������ַ�������ʼ��Ϊ�մ���
                    �ַ����ĳ�������������������ֵ�ʮ��ָ��

    int *x, *y      �������ֵ���Ļ��ʾ����ֵ��ʹ��textwidth������������ı����
					������*x��ֵ
*************************************************************************/

void get_number( int *a_number, int *times, char *tstring, int *x, int *y )
{
int power;         /* ʮ��ָ������10��power���������� */
char achar[2];
char bchar = 0;
achar[1] = 0;

while ( bchar <= 47 || bchar >= 59 )  /* ���������0-9 */
  {
   bchar = getch();
   if ( bchar == 13 )   /* 13 = CR; �û�����ENTER��  */
     {
      bchar = 48;
      *times = 0;
      break;
     }
  }

 if ( *times )
   {
    achar[0] = bchar;

    outtextxy( *x, *y, achar );
    *x = *x + textwidth( achar );
    tstring[TIMES - ( (*times)--)] = achar[0];
    if ( *times )
    get_number( a_number, times, tstring, x, y );
   }

    power = (int)( pow10(( strlen( tstring ) - ((*times) + 1))));
    bchar = tstring[*times];
    *a_number += ( power  * ( bchar - 48 ));
    (*times )++;
}


/*****  driver  **********************************************************

   ���ݲ����ģ�ѡ����ʵ������㷨����

   ���õ���������͡�Ҫ����������׵�ַ��Ԫ�ص���Ŀ�����/�������ӳ�����

	�����ߣ�main

	����: Setscreen, gprintf, all the sort functions

    ����:

    enum sort atype    ָ�������㷨

    int *array         ��������������׵�ַ

    int elements       Ԫ�صĸ���

    int random         random = 1  worst case = 0

    int delay_factor   0 = û���ӳ�;  1000 = ����Ԫ�ؽ�����1���ӳ١�
					   ͨ����������������û�������Ĺ����и����Ե���ʶ��
					   ����1000�Ƿǳ����ġ�

*************************************************************************/

void driver( enum sort atype, int *array, int elements,
            int random, int delay_factor )
{

switch( atype )
  {
	/* ���������㷨 */
   case all    :
	/* ð������ */
   case bubble :
            Setscreen( array, elements, random );
            gprintf( &xaxis, &yaxis, *(sorts + bubble) );
            Bubble( array, elements, delay_factor );
            if ( atype != all ) break; else delay( 1350 );
	/* �ӳٽ������� */
   case delayed:
            Setscreen( array, elements, random );
            gprintf( &xaxis, &yaxis, *(sorts + delayed) );
            Delayed( array, elements, delay_factor );
            if ( atype != all ) break; else delay( 1350 );
	/* ϣ������ */
   case shell  :
            Setscreen( array, elements, random );
            gprintf( &xaxis, &yaxis, *(sorts + shell ));
            Shell( array, elements, delay_factor );
            if ( atype != all ) break; else delay( 1350 );
	/* Metznerϣ������ */
   case shell_metzner:
            Setscreen( array, elements, random );
            gprintf( &xaxis, &yaxis, *(sorts + shell_metzner) );
            Shell_Metzner( array, elements, delay_factor );
            if ( atype != all ) break; else delay( 1350 );
	/* �������� */
   case quick  :
            Setscreen( array, elements, random );
            gprintf( &xaxis, &yaxis, *(sorts + quick) );
            Quicksort( array, 0, elements - 1, delay_factor );
            if ( atype != all ) break; else delay( 1350 );
	/* �������� */
   case insertion:
            Setscreen( array, elements, random );
            gprintf( &xaxis, &yaxis, *(sorts + insertion) );
            Insertion( array, elements, delay_factor );
            if ( atype != all ) break; else delay( 1350 );
	/* ֹͣ */
   case stop:

   default:;
  }
}


/*****  initialize����  *********************************/
/*							                            */
/*  ��ʼ��ͼ������									    */
/*														*/
/********************************************************/

void Initialize( void )
{
 int    GraphDriver; /* ͼ���豸����   */
 int    GraphMode;   /* ͼ��ģʽֵ      */
 int    ErrorCode;   /* �������  */

 GraphDriver = DETECT;              /* �Զ������ʾ�豸���� */
 initgraph( &GraphDriver, &GraphMode, "" );
 ErrorCode = graphresult();   /* ��ȡ��ʼ���Ľ��*/
 if( ErrorCode != grOk )      /* ����ڳ�ʼ���Ĺ����з�������  */
   {
    printf(" Graphics System Error: %s\n", grapherrormsg( ErrorCode ) );
    exit( 1 );
   }

}

/*****  gprintf����  ************************************/
/*						                                */
/*  ��C��־���е�PRINTF���������ơ�����gprintf����      */
/*  ͼ��ģʽ����ָ��������λ����ʾ�ı�			        */
/*														*/
/*                                                      */
/*  gprintf�����ķ���ֵ��û�б�ʹ��						*/
/*                                                      */
/********************************************************/

int gprintf( int *xloc, int *yloc, char *fmt, ... )
{
 va_list  argptr;  /* �����б�ָ��          */
 char str[80];     /* �����������    */
 int count;        /* vspringf�ķŻ�ֵ  */

 va_start( argptr, fmt );               /* ��ʼ��va_ ����         */
 count = vsprintf( str, fmt, argptr );  /* ���ַ����ŵ��洢����	  */
 outtextxy( *xloc, *yloc, str );        /* ��ͼ��ģʽ����ʾ�ַ��� */
 va_end( argptr );                      /* �ر�va_ ����			  */
 return( count );                       /* ������ʾ���ַ���		  */

}


/*****  Setscreen  *******************************************************

   Ϊ�����ʼ����Ļ

   ������: driver

   ����:

    int *array         Ҫ���������

    int elements       Ԫ�صĸ���

    int random         random = 1����worst case = 0

*************************************************************************/

void Setscreen( int *array, int elements, int random )
{
int j;

cleardevice();

/* �������Ԫ��ϵ�� */
if ( random )
  {
   randomize();
   for ( j = 0; j < elements; j++ )
     {
      *( array + j) = random( elements );
      putpixel( 3*j, *(array+j), 10);
     }
  }
else /* ��ʼ����������Ԫ��ȫ������ */
  {
   for ( j = 0; j < elements; j++ )
     {
      *(array + j) = elements - j;
      putpixel( 3*j, *(array+j), 10);
     }

   }
}



/*****  Swap_Pixels����  ****************************/
/*                                                  */
/*  ������������Ԫ�أ����Ҹı���Ӧ������λ��        */
/*  ͨ���򿪺͹ر���ʾ����������Ч��				*/
/*                                                  */
/****************************************************/

int Swap_Pixels( int *array, int i, int j, int delay_factor )
{
int h;
h = *(array + i);
putpixel( 3 * i, *(array + i), 0);
putpixel( 3 * j, *(array + i), 10 );
*(array + i) = *(array + j);
putpixel( 3 * j, *( array + j), 0 );
putpixel( 3 * i, *(array + j), 10 );
*(array + j) = h;

delay( delay_factor );
return( h );
}

/*****  ð������ ************************************/
/*                                                  */
/****************************************************/

void Bubble( int *array, int elements, int delay_factor )
{
int i,j;

for ( i = 0; i < elements - 1 ; i++ )
 for ( j = i + 1; j < elements; j++ )
   {
    if ((*(array+i)) > (*(array+j)))
      {
       Swap_Pixels( array, i, j, delay_factor );
      }
   }
}

/*****  �ӳٽ�������  *******************************/
/*                                                  */
/****************************************************/

void Delayed( int *array, int elements, int delay_factor )
{
int p, h, k, i, j;

for ( p = 0; p < elements-1; p++ )
 {
  h = p;
  for ( k = p + 1; k < elements ; k++ )
    if (*(array+k) < *(array+h))
      h = k;
  if ( p != h )
   {
    i = h;
    j = p;
    Swap_Pixels( array, i, j, delay_factor );
   }
 }
}

/*****  ϣ������  ***********************************/
/*                                                  */
/****************************************************/

void Shell( int *array, int elements, int delay_factor )
{
 int p, f, i, j, m;


p = elements;
while ( p > 1)
  {
   p /= 2;
   
   m = elements - p;
   do{
     f = 0;
     for ( j = 0; j < m; j++ )
       {
        i = j + p;
        if (*(array + j) > *(array + i))
          {
           Swap_Pixels( array, i, j, delay_factor );
           f = 1;
          }
       }
     } while( f );
  }
}

/*****  Metznerϣ������  ****************************/
/*                                                  */
/****************************************************/

void Shell_Metzner( int *array, int elements, int delay_factor )
{
int p, k, t, i, j;

p = elements;
p /= 2;
while ( p != 0 )
  {
  k = elements - p;
  for ( t = 0; t < k; t++ )
    {
    i = t;
    while ( i >= 0 )
      {
      j = i + p;
      if (*(array+j) < *(array + i))
        {
        Swap_Pixels( array, i, j, delay_factor );
        i = i - p;
        }
      else
        break;
      }
    }
  p /= 2;
  }
}

/*****  ��������  ***********************************/
/*                                                  */
/****************************************************/

void Quicksort( int *array, int left, int right, int delay_factor )
{
 int i, j, t;

 if ( right > left )
 {
  i = left - 1; j = right;
  do {
      do i++;
        while ( array[i] < array[right] );
      do j--;
        while ( array[j] > array[right] && j > 0 );
      t = Swap_Pixels( array, i, j, delay_factor );
     } while ( j > i );

      putpixel( 3*j, *(array + j), 0);
      array[j] =array[i];
      putpixel( 3*j, *(array + j), 10 );
      putpixel( 3*i, *(array + i), 0 );
      array[i] =array[right];
      putpixel( 3*i, *(array + i), 10 );
      putpixel( 3*right, *(array + right), 0 );
      array[right] = t;
      putpixel( 3*right, *(array + right), 10 );

      Quicksort( array, left, i - 1, delay_factor );
      Quicksort( array, i + 1, right, delay_factor );

  }
}


/*****  ��������  ***********************************/
/*                                                  */
/****************************************************/

void Insertion( int *array, int elements, int delay_factor )
{
 int p, j, t;


 for ( p = 0; p < elements - 1; p++ )
   {
    t = *(array + p + 1);
    for ( j = p; j >= 0; j-- )
      {
       if ( t <= *(array + j) )
         {
          *(array + j + 1) = *(array + j);
          putpixel( 3*(j + 1), *(array + j + 1), 10 );
          putpixel( 3*j, *(array + j + 1), 0 );
          delay( delay_factor );
         }
       else
         break;
      }
    *(array + j + 1) = t;
    putpixel( 3*(p + 1), t, 0 );
    putpixel( 3*(j + 1), t, 10 );
   }
}

