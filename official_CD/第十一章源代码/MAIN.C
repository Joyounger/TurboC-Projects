

char *greatest_composer_ever = "EDITOR";


#include "tdestr.h"             
#include "common.h"
#include "define.h"
#include "help.h"
#include "tdefunc.h"


#include <dos.h>                /* ���ļ���������Ҫ��������� */
#include <bios.h>               /* ֱ��ȡ�ü��̵�����Ҫ��Ҫ��������� */
#include <io.h>                 /* ȡ���ļ�������ֵ��Ҫ��������� */
#include <fcntl.h>              /* ���ļ���ʱ���־ */
#if defined( __MSC__ )
   #include <errno.h>
   #include <sys\types.h>       /* �����˳���������ʱ���ʽ���������� */
#endif
#include <sys\stat.h>           /* �������ļ�״̬λ�ȳ�����ȫ������ */

#if defined( __MSC__ )
void (interrupt far *old_control_c)( ); /* Ϊ�ϵ�ctrl-c���ñ��� */
void (interrupt far *old_int1b)( );     /* Ϊ�ϵ��ж�int 1b���ñ��� */
#endif


/*
 * ԭ���ļ��control-break��־λ
 */
static int s_cbrk;


/*
 * ��������:	����ϵͳ�����е�����Ҫ�ı༭������
 * ������		argc:  �����в����ĸ���
 *				argv:  �����в���
 */
void main( int argc, char *argv[] )
{
#if defined( __MSC__ )
   union REGS inregs, outregs;
#endif

   g_status.found_first = FALSE;
   g_status.arg         = 1;
   g_status.argc        = argc;
   g_status.argv        = argv;

   /*
    * control-break���ж϶������ǳ���û���ƻ����������ǰѶ����ļ��رա�
    */
#if defined( __MSC__ )
   inregs.h.ah = 0x33;
   inregs.h.al = 0;
   intdos( &inregs, &outregs );
   s_cbrk = outregs.h.dl;
   old_control_c = _dos_getvect( (unsigned)0x23 );
   _dos_setvect( 0x23, harmless );
   old_int1b = _dos_getvect( (unsigned)0x1b );
   _dos_setvect( 0x1b, ctrl_break );
   inregs.h.ah = 0x33;
   inregs.h.al = 1;
   inregs.h.dl = 0;
   intdos( &inregs, &outregs );
#else
   s_cbrk = getcbrk( );
   ctrlbrk( harmless );
   setcbrk( 0 );
#endif


   /*
    * Ϊ������һ���򵥵Ĺؼ���������
    */
   install_ceh( &ceh );
   ceh.flag = OK;

   initialize( );
   editor( );
   terminate( );
}



void error( int kind, int line, char *message )
{
char buff[MAX_COLS+2];          
char line_buff[(MAX_COLS+2)*2]; 

   
   switch (kind) {
      case FATAL:
        
         assert( strlen( main1 ) < MAX_COLS );
         strcpy( buff, main1 );
         break;
     case WARNING:
     default:
         
         assert( strlen( main2 ) < MAX_COLS );
         strcpy( buff, main2 );
         break;
   }

   
   strcat( buff, message );

   if (kind == WARNING)
      
      strcat( buff, main3 );

   
   save_screen_line( 0, line, line_buff );
   set_prompt( buff, line );

   if (kind == FATAL) {
      
      terminate( );
      exit( 1 );
   }

   getkey( );
   restore_screen_line( 0, line, line_buff );
   if (g_status.wrapped) {
      g_status.wrapped = FALSE;
      show_search_message( CLR_SEARCH, g_display.mode_color );
   }
}


#if defined( __MSC__ )
void interrupt far harmless( void )
#else
static int harmless( void )
#endif
{
}



void interrupt far ctrl_break( void )
{
   g_status.control_break = TRUE;
}



void terminate( void )
{
union REGS inregs, outregs;
register WINDOW     *wp;        
WINDOW              *w;         
register file_infos *fp;        
file_infos          *f;         
int                 i;

   /*
    * �ؽ�control-break���
    */
#if defined( __MSC__ )
   _dos_setvect( 0x1b, old_int1b );
   _dos_setvect( 0x23, old_control_c );
   inregs.h.ah = 0x33;
   inregs.h.al = 1;
   inregs.h.dl = (char)s_cbrk;
   intdos( &inregs, &outregs );
#else
   setcbrk( s_cbrk );
#endif

   /*
    * �������û���ͷŵ��ļ��ṹ����ô�ͷ����ǡ�
    */
   fp = g_status.file_list;
   while (fp != NULL) {
      f  = fp;
      fp = fp->next;
      free( f );
   }

   /*
    * �������û���ͷŵĴ��ڽṹ����ô�ͷ����ǡ�
    */
   wp = g_status.window_list;
   while (wp != NULL) {
      w  = wp;
      wp = wp->next;
      free( w );
   }


   /*
    * �ͷ���nfa�е��ַ���
    */
   for (i=0; i < REGX_SIZE; i++) {
      if (sas_nfa.class[i] == nfa.class[i]  &&  nfa.class[i] != NULL)
         free( nfa.class[i] );
      else if (sas_nfa.class[i] != NULL)
         free( sas_nfa.class[i] );
      else if (nfa.class[i] != NULL)
         free( nfa.class[i] );
   }


   /*
    * �������ù��Ĵ�С����ж��83/84���̹��߼�
    */
   set_cursor_size( mode.cursor_size == SMALL_INS ? g_display.insert_cursor :
                                                    g_display.overw_cursor );
   if (mode.enh_kbd == FALSE)
      simulate_enh_kbd( 0 );

   /*
    * �ؽ�ɨ����ɫ
    */
   if (g_display.adapter != MDA)
      set_overscan_color( g_display.old_overscan );
}


/*

 * ����: ��ʼ��������Ӳ������Ļ״̬��Ϣ
 *			���ҵ���Ӳ����������������ú�Ӳ����ص���Ϣ
 
 * ����: [g_status and g_display]
 * ע��: ��������ٶ�g_status��g_display��ʼʱ����Ķ���\0��
 */
void initialize( void )
{
int i;

   /*
    * ������Ӳ����ʼ��
    */
   hw_initialize( );

   /*
    * ��ʼ���༭����ģʽ��ָ��ͼ�����
    */
   bm.search_defined        = ERROR;
   sas_bm.search_defined    = ERROR;
   g_status.sas_defined     = ERROR;
   g_status.sas_search_type = ERROR;

   regx.search_defined      = ERROR;
   sas_regx.search_defined  = ERROR;

   if (mode.undo_max < 2)
      mode.undo_max = 2;

   g_status.marked_file = NULL;
   g_status.current_window = NULL;
   g_status.current_file = NULL;
   g_status.window_list = NULL;
   g_status.file_list = NULL;
   g_status.buff_node = NULL;

   g_status.window_count = 0;
   g_status.file_count = 0;
   g_status.line_buff_len = 0;
   g_status.tabout_buff_len = 0;
   g_status.command = 0;
   g_status.key_pressed = 0;
   g_status.sas_rcol  = 0;
   g_status.sas_rline = 0;
   g_status.recording_key = 0;

   g_status.key_pending = FALSE;
   g_status.found_first = FALSE;
   g_status.sas_found_first = FALSE;
   g_status.copied = FALSE;
   g_status.wrapped = FALSE;
   g_status.marked = FALSE;
   g_status.macro_executing = FALSE;
   g_status.replace_defined = FALSE;

   g_status.screen_display = TRUE;

   g_status.file_chunk = DEFAULT_BIN_LENGTH;

   g_status.sas_tokens[0] = '\0';
   g_status.path[0] = '\0';
   g_status.sas_path[0] = '\0';
   g_status.rw_name[0] = '\0';
   g_status.pattern[0] = '\0';
   g_status.subst[0] = '\0';


   /*
    * ���÷�ҳ����ȻҪ��ʾ���ĵ�ǰ��ҳ�е��кš�
    */
   g_status.overlap = 1;


   /*
    * ��ʼ��nfa�еĽڵ㡣
    */
   for (i=0; i < REGX_SIZE; i++) {
      sas_nfa.node_type[i] = nfa.node_type[i] = 0;
      sas_nfa.term_type[i] = nfa.term_type[i] = 0;
      sas_nfa.c[i] = nfa.c[i] = 0;
      sas_nfa.next1[i] = nfa.next1[i] = 0;
      sas_nfa.next2[i] = nfa.next2[i] = 0;
      sas_nfa.class[i] = nfa.class[i] = NULL;
   }

   /*
    * û����ִ�к�
    */
   connect_macros( );


   /*
    * ����
    */
   cls( );
   show_credits( );
}



void hw_initialize( void )
{
struct vcfg cfg;       
register int *clr;

   
   g_display.ncols     = MAX_COLS;
   g_display.nlines    = MAX_LINES - 1;
   g_display.mode_line = MAX_LINES;
   g_display.line_length = MAX_LINE_LENGTH;

   

   video_config( &cfg );
   g_display.display_address = (char far *)cfg.videomem;

  
   clr =  cfg.color == FALSE ? &colour.clr[0][0] : &colour.clr[1][0];

   g_display.head_color    = *clr++;
   g_display.text_color    = *clr++;
   g_display.dirty_color   = *clr++;
   g_display.mode_color    = *clr++;
   g_display.block_color   = *clr++;
   g_display.message_color = *clr++;
   g_display.help_color    = *clr++;
   g_display.diag_color    = *clr++;
   g_display.eof_color     = *clr++;
   g_display.curl_color    = *clr++;
   g_display.ruler_color   = *clr++;
   g_display.ruler_pointer = *clr++;
   g_display.hilited_file  = *clr++;
   g_display.overscan      = *clr;

   
   if (g_display.adapter != MDA)
      set_overscan_color( g_display.overscan );
}



int  get_help( WINDOW *window )
{
register char *help;
register int line;

   xygoto( -1, -1 );
   help = help_screen[1];
   for (line=0; help != NULL; line++) {
      s_output( help, line, 0, g_display.help_color );
      help = help_screen[line+2];
   }
   line = getkey( );
   redraw_screen( window );
   return( OK );
}



void show_credits( void )
{
register char *credit;
int  line;

   xygoto( -1, -1 );
   credit = credit_screen[0];
   for (line=0; credit != NULL; ) {
      s_output( credit, line+2, 11, g_display.text_color );
      credit = credit_screen[++line];
   }
}
