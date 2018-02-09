


#include "tdestr.h"             
#include "common.h"
#include "define.h"
#include "tdefunc.h"


#include <dos.h>                
#include <bios.h>               
#include <io.h>                 
#include <fcntl.h>              
#if defined( __MSC__ )
   #include <errno.h>
   #include <sys\types.h>       
#endif
#include <sys\stat.h>           


int  hw_fattrib( char *name )
{
register int rc;
int  fattr;

   rc = get_fattr( name, &fattr );
   return( rc == OK ? rc : ERROR );
}


int  change_mode( char *name, int line )
{
int  result;
int  fattr;
register int rc;
char line_buff[(MAX_COLS+1)*2]; 

   rc = OK;
   result = get_fattr( name, &fattr );
   if (result != OK)
      rc = ERROR;
   else if (result == OK && fattr & READ_ONLY) {
     
      save_screen_line( 0, line, line_buff );
     
      set_prompt( main6, line );
      if (get_yn( ) != A_YES)
         rc = ERROR;
      if (rc == OK && set_fattr( name, ARCHIVE ) != OK)
         rc = ERROR;
      restore_screen_line( 0, line, line_buff );
   }
   return( rc );
}



int  write_file( char *name, int open_mode, file_infos *file, long start,
                 long end, int block )
{
FILE *fp;       
char *p;
char *z = "\x1a";
register int rc;
int  bc;
int  ec;
int  len;
int  write_z;
int  write_eol;
long number;
line_list_ptr ll;
char *open_string;
char *eol;
size_t eol_count;

   write_z = mode.control_z;
   switch (open_mode) {
      case APPEND :
         open_string = "ab";
         break;
      case OVERWRITE :
      default :
         open_string = "wb";
         break;
   }
   switch (file->crlf) {
      case BINARY   :
         eol_count = 0;
         eol = "";
         write_z = FALSE;
         break;
      case CRLF   :
         eol_count = 2;
         eol = "\r\n";
         break;
      case LF     :
         eol_count = 1;
         eol = "\n";
         break;
      default     :
         assert( FALSE );
   }
   rc = OK;
   if ((fp = fopen( name, open_string )) == NULL || ceh.flag == ERROR)
      rc = ERROR;
   else {
      ec = bc = len = 0;
      ll = file->line_list;
      if (block == LINE || block == BOX || block == STREAM) {
         if (g_status.marked_file == NULL)
            rc = ERROR;
         else
            file = g_status.marked_file;
         if (rc != ERROR) {
            ll = file->line_list;
            for (number=1; number<start && ll->next != NULL; number++)
               ll = ll->next;
         }
         if (rc != ERROR && (block == BOX || block == STREAM)) {
            bc  = file->block_bc;
            ec  = file->block_ec;
            len = ec + 1 - bc;
         }
         if (rc != ERROR  &&  block == STREAM) {
            if (start == end )
               block = BOX;
         }
      } else {
         for (number=1; number<start && ll->next != NULL; number++)
            ll = ll->next;
      }
      p = g_status.line_buff;
      if (rc == OK) {
         if (block == BOX) {

            assert( len >= 0 );
            assert( len < MAX_LINE_LENGTH );

            for (;start <= end  &&  ll->len != EOF && rc == OK; start++) {
               g_status.copied = FALSE;
               load_box_buff( p, ll, bc, ec, ' ' );
               if (fwrite( p, sizeof( char ), len, fp ) < (unsigned)len ||
                          ceh.flag == ERROR)
                  rc = ERROR;
               if (rc != ERROR  && fwrite( eol, sizeof( char ), eol_count, fp )
                                    < eol_count || ceh.flag == ERROR)
                  rc = ERROR;
               ll = ll->next;
               if (ll == NULL)
                  rc = ERROR;
            }
         } else {
            for (number=start; number <= end && rc == OK && ll->len != EOF;
                      number++) {
               g_status.copied = FALSE;
               copy_line( ll );
               len = g_status.line_buff_len;
               if (block == STREAM) {
                  if (number == start) {
                     bc = bc > len ? len : bc;
                     len = len - bc;

                     assert( len >= 0 );

                     memmove( p, p + bc, len );
                  } else if (number == end) {
                     ++ec;
                     len =  ec > len ? len : ec;
                  }
               }

               assert( len >= 0 );
               assert( len < MAX_LINE_LENGTH );

               if (fwrite( p, sizeof( char ), len, fp ) < (unsigned)len ||
                       ceh.flag == ERROR)
                  rc = ERROR;

             
               write_eol = TRUE;
               if (number == end) {
                  if (file->crlf == CRLF ||  file->crlf == LF) {
                     if (len > 0  &&  *(p + len - 1) == '\x1a') {
                        write_eol = FALSE;
                        write_z = FALSE;
                     }
                  }
               }

               if (write_eol == TRUE  &&  rc != ERROR  &&
                     fwrite( eol, sizeof( char ), eol_count, fp ) < eol_count
                     || ceh.flag == ERROR)
                  rc = ERROR;
               ll = ll->next;
               if (ll == NULL)
                  rc = ERROR;
            }
         }
         if (rc != ERROR  &&  write_z) {
            if (fwrite( z, sizeof( char ), 1, fp ) < 1 || ceh.flag == ERROR)
               rc = ERROR;
         }
         g_status.copied = FALSE;
         if (ceh.flag != ERROR) {
            if (fclose( fp ) != 0)
               rc = ERROR;
         }
      }
   }
   return( rc );
}



int hw_save( char *name, file_infos *file, long start, long end, int block )
{
   return( write_file( name, OVERWRITE, file, start, end, block ) );
}



int hw_append( char *name, file_infos *file, long start, long end, int block )
{
   return( write_file( name, APPEND, file, start, end, block ) );
}


/*
 * ����: ��һ���ļ����ص��ı�ָ��������To load a file into the array of text pointers.
 * ����:  name:       �����ļ�������
 *          fp:       ָ���ļ��ṹ��ָ��
 *          file_mode:  �����ƻ����ı�
 *          bin_len:    ����Ƕ�����ģʽ�򿪵ģ��ڵ��еĳ���
 * ����ֵ: �������������ERROR������OK
 */
int  load_file( char *name, file_infos *fp, int *file_mode, int bin_len )
{
FILE *stream;                           
int  rc;
char buff[MAX_COLS+2];
char line_buff[(MAX_COLS+2)*2];         
text_ptr l;
line_list_ptr ll;
line_list_ptr temp_ll;
unsigned long line_count;
char *e;
char *residue;
int  len;
int  res;
size_t t1, t2;
int  crlf;
int  prompt_line;

   /*
    * ��ʼ����������ָ��
    */
   rc = OK;
   len = 1;
   line_count = 0;
   res = 0;
   residue = g_status.line_buff;
   prompt_line = g_display.nlines;
   fp->length  = 0;
   fp->undo_count = 0;
   fp->undo_top = fp->undo_bot = NULL;
   fp->line_list_end = fp->line_list = NULL;
   ll = (line_list_ptr)my_malloc( sizeof(line_list_struc), &rc );

   if (ll != NULL) {
      ll->dirty = FALSE;
      ll->len   = EOF;
      ll->line  = NULL;
      ll->next  = ll->prev = NULL;
      fp->undo_top = fp->undo_bot = ll;
   }

   ll = (line_list_ptr)my_malloc( sizeof(line_list_struc), &rc );

   if (ll != NULL) {
      ll->dirty = FALSE;
      ll->len   = EOF;
      ll->line  = NULL;
      ll->next  = ll->prev = NULL;
      fp->line_list_end = fp->line_list = ll;
   }

   if ((stream = fopen( name, "rb" )) == NULL || ceh.flag == ERROR ||
         rc == ERROR) {
      /*
       * û���ҵ��ļ����߼��ص�ʱ���������
       */
      combine_strings( buff, main7a, name, main7b );
      save_screen_line( 0, prompt_line, line_buff );
      set_prompt( buff, prompt_line );
      getkey( );
      restore_screen_line( 0, prompt_line, line_buff );
      if (fp->line_list != NULL)
         my_free( fp->line_list );
      if (fp->undo_top != NULL)
         my_free( fp->undo_top );
      rc = ERROR;
   } else {
      if (*file_mode == BINARY) {
         mode.trailing = FALSE;
         crlf = BINARY;
         if (bin_len < 0  ||  bin_len > READ_LENGTH)
            bin_len = DEFAULT_BIN_LENGTH;
         for (; rc == OK;) {
            t1 = fread( g_status.line_buff, sizeof(char), bin_len, stream );
            if (ferror( stream )  ||  ceh.flag == ERROR) {
               combine_strings( buff, "error reading file '", name, "'" );
               error( WARNING, prompt_line, buff );
               rc = ERROR;
            } else if (t1) {

               assert( t1 < MAX_LINE_LENGTH );

               l = (text_ptr)my_malloc( t1 * sizeof(char), &rc );
               temp_ll = (line_list_ptr)my_malloc( sizeof(line_list_struc), &rc );

               if (rc != ERROR) {

                  /*
                   * ���û�����⣬��io���������ı��������ڴ���
                   */
                  if (t1 > 0)
                     _fmemcpy( l, g_status.line_buff, t1 );

                  ++line_count;
                  temp_ll->line = l;
                  temp_ll->dirty = FALSE;
                  temp_ll->len  = t1;
                  insert_node( fp, ll, temp_ll );
                  ll = temp_ll;
               } else
                  rc = show_file_2big( name, prompt_line, temp_ll, l );
            } else
               break;
         }
      } else {
         crlf = LF;
         for (; rc == OK;) {
            t1 = fread( g_status.line_buff, sizeof(char), READ_LENGTH, stream );
            if (ferror( stream )  ||  ceh.flag == ERROR) {
               combine_strings( buff, "error reading file '", name, "'" );
               error( WARNING, prompt_line, buff );
               rc = ERROR;
            } else {

               /*
                * "e" ����������1���ҵ��ļ�β�� 
                * "t1"���滺����1�е��ַ���
                */
               e = g_status.line_buff;
               while (t1 && rc == OK) {

                  /*
                   * ��t1�ǿգ�����len�����ֵС����eһֱ����ֱ������<LF>.
                   */
                  for (; t1 && len < READ_LENGTH &&  *e != '\n'; len++, e++, t1--);

                  /*
                   * ��t1�ǿգ�e����������<LF>���߳������ֵ
                   */
                  if (t1  ||  len >= READ_LENGTH) {

                     if (len > 1 && *e == '\n') {
                        if (len - res == 1) {
                           if (*(residue + res - 1) == '\r') {
                              --len;
                              --res;
                              crlf = CRLF;
                           }
                        } else {
                           if (*(e - 1) == '\r') {
                              --len;
                              crlf = CRLF;
                           }
                        }
                     }
                     if (len > 0)
                        --len;

                     assert( len >= 0 );
                     assert( len < MAX_LINE_LENGTH );

                     /*
                      * Ϊ���Ǹոն�����з���ռ�
                      */
                     l = (text_ptr)my_malloc( len * sizeof(char), &rc );
                     temp_ll =
                       (line_list_ptr)my_malloc( sizeof(line_list_struc), &rc );

                     if (rc != ERROR) {

                        /*
                         * ���һ����������io�����е����ݿ������ڴ���ȥ
                         * "residue" ����io�����е�����
                         */
                        if (res > 0) {

                           assert( res >= 0 );
                           assert( len - res >= 0 );

                           if (res > 0)
                              _fmemcpy( l, residue, res );
                           if (len - res > 0)
                              _fmemcpy( l + res, g_status.line_buff, len - res );
                           res = 0;
                        } else
                           if (len > 0)
                              _fmemcpy( l, residue, len );

                        ++line_count;
                        temp_ll->line = l;
                        temp_ll->dirty = FALSE;
                        temp_ll->len  = len;
                        insert_node( fp, ll, temp_ll );
                        ll = temp_ll;

                        /*
                         * ��������io�����ָ��ͼ�����
                         */
                        len = 1;
                        if (t1 == 0)
                           residue = g_status.tabout_buff;
                        else {
                           t1--;
                           residue =  t1 == 0 ? g_status.tabout_buff : ++e;
                        }
                     } else
                        rc = show_file_2big( name, prompt_line, temp_ll, l );
                  } else if (len < READ_LENGTH ) {
                     if (!feof( stream ))
                        res = len - 1;
                  } else {
                     error( WARNING, prompt_line, "FRANK: error reading file!" );
                     rc = ERROR;
                  }
               }
            }

            if (rc != OK)
               break;

            /*
             * �����Ѿ����������е���'\n'��β���У�����'\n'֮����ܻ���
			 *  һЩ�������ַ�������^Z��
             */
            if (feof( stream )) {
               if (len > 1) {
                  --len;
                  if (t1 == 0)
                     --e;

                  assert( len >= 0 );
                  assert( len < MAX_LINE_LENGTH );

                  /*
                   *  Ϊ�ոն�����з����ڴ档
                   */
                  l = (text_ptr)my_malloc( len * sizeof(char), &rc );
                  temp_ll =
                       (line_list_ptr)my_malloc( sizeof(line_list_struc), &rc );

                  if (rc != ERROR) {

                     /*
                      * ���һ����������io�����е����ݿ������ڴ���ȥ
                      * "residue" ����io�����е�����
                      */
                     if (res > 0) {

                        assert( res >= 0 );
                        assert( res < MAX_LINE_LENGTH);
                        assert( len - res >= 0 );
                        assert( len - res < MAX_LINE_LENGTH);

                        if (res > 0 )
                           _fmemcpy( l, residue, res );
                        if (len - res > 0)
                           _fmemcpy( l + res, g_status.line_buff, len - res );
                     } else
                        if (len > 0)
                           _fmemcpy( l, residue, len );
                     ++line_count;
                     temp_ll->line = l;
                     temp_ll->dirty = FALSE;
                     temp_ll->len  = len;
                     insert_node( fp, ll, temp_ll );
                  } else
                     rc = show_file_2big( name, prompt_line, temp_ll, l );
               }
               break;
            }

            t2 = fread( g_status.tabout_buff, sizeof(char), READ_LENGTH, stream );
            if (ferror( stream )  ||  ceh.flag == ERROR) {
               combine_strings( buff, "error reading file '", name, "'" );
               error( WARNING, prompt_line, buff );
               rc = ERROR;
            } else if (rc == OK) {
               e = g_status.tabout_buff;
               while (t2 && rc == OK) {
                  for (; t2 && len < READ_LENGTH &&  *e != '\n'; len++, e++, t2--);
                  if (t2  ||  len >= READ_LENGTH) {

                     if (len > 1 && *e == '\n') {
                        if (len - res == 1) {
                           if (*(residue + res - 1) == '\r') {
                              --len;
                              --res;
                              crlf = CRLF;
                           }
                        } else {
                           if (*(e - 1) == '\r') {
                              --len;
                              crlf = CRLF;
                           }
                        }
                     }
                     if (len > 0)
                        --len;

                     assert( len >= 0 );
                     assert( len < MAX_LINE_LENGTH );

                     l = (text_ptr)my_malloc( len * sizeof(char), &rc );
                     temp_ll =
                       (line_list_ptr)my_malloc( sizeof(line_list_struc), &rc );

                     if (rc != ERROR) {
                        if (res > 0) {

                           assert( res >= 0 );
                           assert( res < MAX_LINE_LENGTH);
                           assert( len - res >= 0 );
                           assert( len - res < MAX_LINE_LENGTH);

                           if (res > 0)
                              _fmemcpy( l, residue, res );
                           if (len - res > 0)
                              _fmemcpy( l+res, g_status.tabout_buff, len - res );
                           res = 0;
                        } else
                           if (len > 0)
                              _fmemcpy( l, residue, len );

                        ++line_count;
                        temp_ll->line = l;
                        temp_ll->dirty = FALSE;
                        temp_ll->len  = len;
                        insert_node( fp, ll, temp_ll );
                        ll = temp_ll;

                        len = 1;
                        if (t2 == 0)
                           residue = g_status.line_buff;
                        else {
                           t2--;
                           residue =  t2 == 0 ? g_status.line_buff : ++e;
                        }
                     } else
                        rc = show_file_2big( name, prompt_line, temp_ll, l );
                  } else if (len < READ_LENGTH) {
                     if (!feof( stream ))
                        res = len - 1;
                  } else {
                     error( WARNING, prompt_line, "FRANK: error reading file!" );
                     rc = ERROR;
                  }
               }
            }

            if (rc != ERROR  &&  feof( stream )) {
               if (len > 1) {
                  --len;
                  if (t2 == 0)
                     --e;

                  assert( len >= 0 );
                  assert( len < MAX_LINE_LENGTH );

                  l = (text_ptr)my_malloc( len * sizeof(char), &rc );
                  temp_ll =
                       (line_list_ptr)my_malloc( sizeof(line_list_struc), &rc );

                  if (rc != ERROR) {
                     if (res > 0) {

                        assert( res >= 0 );
                        assert( res < MAX_LINE_LENGTH);
                        assert( len - res >= 0 );
                        assert( len - res < MAX_LINE_LENGTH);

                        if (res > 0)
                           _fmemcpy( l, residue, res );
                        if (len - res > 0)
                           _fmemcpy( l+res, g_status.tabout_buff, len - res );
                     } else
                        if (len > 0)
                           _fmemcpy( l, residue, len );

                     ++line_count;
                     temp_ll->line = l;
                     temp_ll->dirty = FALSE;
                     temp_ll->len  = len;
                     insert_node( fp, ll, temp_ll );
                  } else
                     rc = show_file_2big( name, prompt_line, temp_ll, l );
               }
               break;
            }
         }
         *file_mode = crlf;
      }

      /*�ر��ļ�close the file
       */
      fp->length = line_count;
   }
   if (stream != NULL)
      fclose( stream );
   return( rc );
}



void insert_node( file_infos *fp, line_list_ptr current, line_list_ptr new )
{

   
   if (current->next != NULL) {
      current->next->prev = new;
      new->next = current->next;
      current->next = new;
      new->prev = current;
   
   } else {
      new->next = current;
      if (current->prev != NULL)
         current->prev->next = new;
      new->prev = current->prev;
      current->prev = new;
      if (new->prev == NULL)
         fp->line_list = new;
   }
}



int  show_file_2big( char *name, int prompt_line, line_list_ptr ll, text_ptr t )
{
char buff[MAX_COLS+2];

   combine_strings( buff, main10a, name, main10b );
   error( WARNING, prompt_line, buff );
   if (t != NULL)
      my_free( t );
   if (ll != NULL)
      my_free( ll );
   return( WARNING );
}



int  backup_file( WINDOW *window )
{
char *old_line_buff;
char *old_tabout_buff;
int  old_line_buff_len;
int  old_tabout_buff_len;
int  old_copied;
int  rc;
file_infos *file;

   rc = OK;
   file = window->file_info;
   if (file->backed_up == FALSE  &&  file->modified == TRUE) {
      old_copied = g_status.copied;
      old_line_buff_len = g_status.line_buff_len;
      old_line_buff = calloc( MAX_LINE_LENGTH, sizeof(char) );
      old_tabout_buff_len = g_status.tabout_buff_len;
      old_tabout_buff = calloc( MAX_LINE_LENGTH, sizeof(char) );

      if (old_line_buff != NULL  &&  old_tabout_buff != NULL) {
         memcpy( old_line_buff, g_status.line_buff, MAX_LINE_LENGTH );
         memcpy( old_tabout_buff, g_status.tabout_buff, MAX_LINE_LENGTH );
         if ((rc = save_backup( window )) != ERROR)
            file->backed_up = TRUE;
         else
            rc = ERROR;
         memcpy( g_status.line_buff, old_line_buff, MAX_LINE_LENGTH );
         memcpy( g_status.tabout_buff, old_tabout_buff, MAX_LINE_LENGTH );
         g_status.line_buff_len = old_line_buff_len;
         g_status.tabout_buff_len = old_tabout_buff_len;
         g_status.copied = old_copied;
      } else {
         error( WARNING, window->bottom_line, main4 );
         rc = ERROR;
      }
      if (old_line_buff != NULL)
         free( old_line_buff );
      if (old_tabout_buff != NULL)
         free( old_tabout_buff );
   }
   return( rc );
}



int  edit_file( char *name, int file_mode, int bin_length )
{
int  rc;        
int  existing;
int  line;
int  rcol;
register file_infos *file; 
file_infos *fp;
long found_line;
line_list_ptr ll;
line_list_ptr temp_ll;

   line = g_display.nlines;
   rc = OK;
  
   file = (file_infos *)calloc( 1, sizeof(file_infos) );
   if (file == NULL) {
      error( WARNING, line, main4 );
      rc = ERROR;
   }
   existing = FALSE;
   if (rc == OK  &&  hw_fattrib( name ) == OK) {
      existing = TRUE;
     

      if (g_status.command != DefineGrep  &&
          g_status.command != DefineRegXGrep  &&
          g_status.command != RepeatGrep)
         rc = load_file( name, file, &file_mode, bin_length );
      else {
         if (g_status.sas_defined) {
            rc = load_file( name, file, &file_mode, bin_length );
            if (rc != ERROR) {
               found_line = 1L;
               rcol = 0;
               if (g_status.sas_search_type == BOYER_MOORE)
                  ll = search_forward( file->line_list, &found_line,
                                       (size_t *)&rcol );
               else
                  ll = regx_search_forward( file->line_list, &found_line,
                                            &rcol );
               if (ll == NULL)
                  rc = ERROR;
               else {
                  g_status.sas_rline = found_line;
                  g_status.sas_rcol  = rcol;
                  g_status.sas_ll    = ll;
               }
            }
         } else
            rc = ERROR;
      }
   } else {
      if (ceh.flag == ERROR)
         rc = ERROR;
      else {
         existing = FALSE;
         file->length = 0l;
         file->undo_top = file->undo_bot = NULL;
         file->line_list_end = file->line_list = NULL;
         file->undo_count = 0;
         ll = (line_list_ptr)my_malloc( sizeof(line_list_struc), &rc );
         if (ll != NULL) {
            ll->line  = NULL;
            ll->next  = ll->prev = NULL;
            ll->dirty = FALSE;
            ll->len   = EOF;
            file->undo_top = file->undo_bot = ll;
         } else
            rc = ERROR;

         ll = (line_list_ptr)my_malloc( sizeof(line_list_struc), &rc );
         if (ll != NULL) {
            ll->line = NULL;
            ll->next = ll->prev = NULL;
            ll->dirty = FALSE;
            ll->len   = EOF;
            file->line_list_end = file->line_list = ll;
         } else
            rc = ERROR;
         if (rc == ERROR) {
            if (file->undo_top != NULL)
               my_free( file->undo_top );
            if (file->line_list != NULL)
               my_free( file->line_list );
         } else
            if (file_mode == TEXT)
               file_mode = CRLF;
      }
   }

   if (rc != ERROR) {
     
      file->prev = NULL;
      file->next = NULL;
      if (g_status.file_list == NULL)
         g_status.file_list = file;
      else {
         fp = g_status.current_file;
         file->prev = fp;
         if (fp->next)
            fp->next->prev = file;
         file->next = fp->next;
         fp->next = file;
      }

      

      assert( file_mode == CRLF  ||  file_mode == LF  ||  file_mode == BINARY );
      assert( strlen( name ) < MAX_COLS );

      strcpy( file->file_name, name );
      get_fattr( name, (int *)&file->file_attrib );
      file->block_type  = NOTMARKED;
      file->block_br    = file->block_er = 0l;
      file->block_bc    = file->block_ec = 0;
      file->ref_count   = 0;
      file->modified    = FALSE;
      file->backed_up   = FALSE;
      file->new_file    = !existing;
      file->next_letter = 'a';
      file->file_no     = ++g_status.file_count;
      file->crlf        = file_mode;
      g_status.current_file = file;
      make_backup_fname( file );
   } else if (file != NULL) {
     
      ll = file->undo_top;
      while (ll != NULL) {
         temp_ll = ll->next;
         if (ll->line != NULL)
            my_free( ll->line );
         my_free( ll );
         ll = temp_ll;
      }

      ll = file->line_list;
      while (ll != NULL) {
         temp_ll = ll->next;
         if (ll->line != NULL)
            my_free( ll->line );
         my_free( ll );
         ll = temp_ll;
      }

#if defined( __MSC__ )
      _fheapmin( );
#endif

      free( file );
   }
   return( rc );
}



int  edit_another_file( WINDOW *window )
{
char fname[MAX_COLS];           
char spdrive[_MAX_DRIVE];       
char spdir[_MAX_DIR];           
char spname[_MAX_FNAME];        
char spext[_MAX_EXT];           
register WINDOW *win;           
int  rc;
int  file_mode;
int  bin_length;

   win = window;
   entab_linebuff( );
   if (un_copy_line( win->ll, win, TRUE ) == ERROR)
      return( ERROR );
  
   fname[0] = '\0';
   
   if ((rc = get_name( ed15, win->bottom_line, fname,
                 g_display.message_color )) == OK  &&  *fname != '\0') {
      file_mode = TEXT;
      bin_length = 0;

      assert( strlen( fname ) <= MAX_COLS );

      _splitpath( fname, spdrive, spdir, spname, spext );
      if (stricmp( spext, ".exe" ) == 0  ||  stricmp( spext, ".com" ) == 0) {
         file_mode = BINARY;
         bin_length = g_status.file_chunk;
      }
      rc = attempt_edit_display( fname, LOCAL, file_mode, bin_length );
      if (rc == OK)
         show_avail_mem( );
   }
   return( rc );
}



int  edit_next_file( WINDOW *window )
{
char name[MAX_COLS];            
char spdrive[_MAX_DRIVE];       
char spdir[_MAX_DIR];           
char spname[_MAX_FNAME];        
char spext[_MAX_EXT];           
int  file_mode;
int  bin_length;
int  i;
int  update_type;
register int rc = ERROR;
register WINDOW *win;           

   win = window;
   update_type = win == NULL ? GLOBAL : LOCAL;
   if (g_status.arg < g_status.argc) {
      if (win != NULL) {
         entab_linebuff( );
         if (un_copy_line( win->ll, win, TRUE ) == ERROR)
            return( ERROR );
      }

      while (rc == ERROR && g_status.arg < g_status.argc) {

         
         if (g_status.found_first == FALSE) {

            assert( strlen( g_status.argv[g_status.arg] ) < MAX_COLS );

            strcpy( name, g_status.argv[g_status.arg] );
            rc = get_fattr( name, &i );

            if (rc == OK || rc == 2) {
               ++g_status.arg;
               rc = OK;

            } else if (rc != ERROR) {
               rc = my_findfirst( &g_status.dta, name, NORMAL | READ_ONLY |
                               HIDDEN | SYSTEM | ARCHIVE );

             
               if (rc == OK) {
                  g_status.found_first = TRUE;
                  i = strlen( name ) - 1;
                  while (i >= 0) {
                     if (name[i] == ':' || name[i] == '\\')
                        break;
                     --i;
                  }
                  name[++i] = '\0';

                  assert( strlen( name ) < MAX_COLS );

                  strcpy( g_status.path, name );
                  strcpy( name, g_status.path );
                  strcat( name, g_status.dta.name );
               } else {
                  ++g_status.arg;
                  if (win != NULL)
                    
                     error( WARNING, win->bottom_line, win8 );
               }
            } else if (rc == ERROR)
               ++g_status.arg;
         } else {

           
            rc = my_findnext( &g_status.dta );
            if (rc == OK) {

               assert( strlen( g_status.path ) + strlen( g_status.dta.name )
                           < MAX_COLS );

               strcpy( name, g_status.path );
               strcat( name, g_status.dta.name );
            } else {
               g_status.found_first = FALSE;
               ++g_status.arg;
            }
         }

        
         if (rc == OK) {
            file_mode = g_status.file_mode;
            bin_length = g_status.file_chunk;

            assert( strlen( name ) <= MAX_COLS );

            _splitpath( name, spdrive, spdir, spname, spext );
            if (stricmp( spext, ".exe" ) == 0 || stricmp( spext, ".com" ) == 0)
               file_mode = BINARY;
            rc = attempt_edit_display( name, update_type, file_mode, bin_length );
            if (rc == OK)
               show_avail_mem( );
         }

         
         else
            rc = ERROR;
      }
   }
   if (rc == ERROR  &&  g_status.arg >= g_status.argc  &&  win != NULL)
    
      error( WARNING, win->bottom_line, win9 );
   return( rc );
}



int  search_and_seize( WINDOW *window )
{
char name[MAX_COLS];            
char searching[MAX_COLS];       
char line_buff[(MAX_COLS+1)*2]; 
char spdrive[_MAX_DRIVE];       
char spdir[_MAX_DIR];           
char spname[_MAX_FNAME];        
char spext[_MAX_EXT];           
int  file_mode;
int  bin_length;
int  i;
int  update_type;
char *tokens;
register int rc = ERROR;
register WINDOW *win;           
int  bottom_line;

   win = window;
   update_type = win == NULL ? GLOBAL : LOCAL;
   if (update_type == LOCAL) {
      if (!g_status.sas_defined ||  g_status.command == DefineGrep ||
                              g_status.command == DefineRegXGrep) {

        
         if (g_status.command == DefineGrep)
            g_status.sas_search_type = BOYER_MOORE;
         else
            g_status.sas_search_type = REG_EXPRESSION;

         if (g_status.sas_search_type == BOYER_MOORE) {
            *sas_bm.pattern = '\0';
            if (get_name( win16a, win->bottom_line, (char *)sas_bm.pattern,
                             g_display.message_color ) == ERROR)
               return( ERROR );
            if (*sas_bm.pattern == '\0')
               return( ERROR );
         } else {
            *sas_regx.pattern = '\0';
            if (get_name( win16b, win->bottom_line, (char *)sas_regx.pattern,
                             g_display.message_color ) == ERROR)
               return( ERROR );
            if (*sas_regx.pattern == '\0')
               return( ERROR );
            else
               strcpy( (char *)regx.pattern, (char *)sas_regx.pattern );
         }
         *g_status.sas_tokens = '\0';
         if (get_name( win17, win->bottom_line, g_status.sas_tokens,
                          g_display.message_color ) == ERROR)
            return( ERROR );
         i = 0;
         tokens = strtok( g_status.sas_tokens, SAS_DELIMITERS );
         while (tokens != NULL) {
            g_status.sas_arg_pointers[i++] = tokens;
            tokens = strtok( NULL, SAS_DELIMITERS );
         }
         if (i == 0)
            return( ERROR );
         g_status.sas_arg_pointers[i] = NULL;
         g_status.sas_argc = i;
         g_status.sas_arg = 0;
         g_status.sas_argv = g_status.sas_arg_pointers;
         g_status.sas_found_first = FALSE;
         if (g_status.command == DefineGrep) {
            g_status.sas_defined = TRUE;
            bm.search_defined = sas_bm.search_defined = OK;
            build_boyer_array( );
         } else {
            i = build_nfa( );
            if (i == OK) {
               g_status.sas_defined = TRUE;
               regx.search_defined = sas_regx.search_defined = OK;
            } else
               g_status.sas_defined = FALSE;
         }
      }
      bottom_line = win->bottom_line;
   } else
      bottom_line = g_display.nlines;
   if (g_status.sas_defined && g_status.sas_arg < g_status.sas_argc) {
      if (win != NULL) {
         entab_linebuff( );
         un_copy_line( win->ll, win, TRUE );
      }

      
      while (rc == ERROR && g_status.sas_arg < g_status.sas_argc) {

         
         if (g_status.sas_found_first == FALSE) {

            assert( strlen( g_status.sas_argv[g_status.sas_arg] ) < MAX_COLS );

            strcpy( name, g_status.sas_argv[g_status.sas_arg] );
            rc = get_fattr( name, &i );

            
            if (rc == OK || rc == 2) {
               ++g_status.sas_arg;
               rc = OK;

            
            } else if (rc != ERROR) {
               rc = my_findfirst( &g_status.sas_dta, name, NORMAL | READ_ONLY |
                               HIDDEN | SYSTEM | ARCHIVE );

              
               if (rc == OK) {
                  g_status.sas_found_first = TRUE;
                  i = strlen( name ) - 1;
                  while (i >= 0) {
                     if (name[i] == ':' || name[i] == '\\')
                        break;
                     --i;
                  }
                  name[++i] = '\0';

                  assert( strlen( name ) + strlen( g_status.sas_dta.name )
                                   < MAX_COLS );

                  strcpy( g_status.sas_path, name );
                  strcpy( name, g_status.sas_path );
                  strcat( name, g_status.sas_dta.name );
               } else {
                  ++g_status.sas_arg;
                  if (win != NULL)
                  
                     error( WARNING, win->bottom_line, win8 );
               }
            } else if (rc == ERROR)
               ++g_status.sas_arg;
         } else {

           
            rc = my_findnext( &g_status.sas_dta );
            if (rc == OK) {

               assert( strlen( g_status.sas_path ) +
                       strlen( g_status.sas_dta.name ) < MAX_COLS );

               strcpy( name, g_status.sas_path );
               strcat( name, g_status.sas_dta.name );
            } else {
               g_status.sas_found_first = FALSE;
               ++g_status.sas_arg;
            }
         }

        
         if (rc == OK) {

            assert( strlen( win19 ) + strlen( name ) < MAX_COLS );

            strcpy( searching, win19 );
            strcat( searching, name );
            save_screen_line( 0, bottom_line, line_buff );
            set_prompt( searching, bottom_line );
            file_mode = TEXT;
            bin_length = 0;

            assert( strlen( name ) <= MAX_COLS );

            _splitpath( name, spdrive, spdir, spname, spext );
            if (stricmp( spext, ".exe" ) == 0 || stricmp( spext, ".com" ) == 0){
               file_mode = BINARY;
               bin_length = g_status.file_chunk;
            }
            rc = attempt_edit_display( name, update_type, file_mode, bin_length );
            if (rc == OK)
               show_avail_mem( );
            restore_screen_line( 0, bottom_line, line_buff );

            if (rc == OK) {
               win = g_status.current_window;
               bin_offset_adjust( win, g_status.sas_rline );
               find_adjust( win, g_status.sas_ll, g_status.sas_rline,
                            g_status.sas_rcol );
               make_ruler( win );
               show_ruler( win );
               show_ruler_pointer( win );
               show_window_header( win );
               if (win->vertical)
                  show_vertical_separator( win );
               win->file_info->dirty = LOCAL;
            }
         }

         else
            rc = ERROR;
      }
   }
   if (rc == ERROR &&  g_status.sas_arg >= g_status.sas_argc  && win != NULL)
      
      error( WARNING, win->bottom_line, win9 );
   return( rc );
}



int  attempt_edit_display( char *fname, int update_type, int file_mode,
                           int bin_len )
{
register int rc;
WINDOW *win;

   rc = edit_file( fname, file_mode, bin_len );
   if (rc != ERROR) {
      rc = initialize_window( );
      if (rc != ERROR) {
         win = g_status.current_window;
         if (update_type == LOCAL) {
            if (g_status.command != DefineGrep  &&
                        g_status.command != DefineRegXGrep  &&
                        g_status.command != RepeatGrep)
               redraw_current_window( win );
            show_file_count( g_status.file_count );
            show_window_count( g_status.window_count );
            show_avail_mem( );
         } else if (update_type == GLOBAL)
            redraw_screen( win );
         if (win->file_info->new_file) {
            g_status.command = AddLine;
            insert_newline( win );
            win->file_info->modified = FALSE;
         }
      }
   }
   return( rc );
}



void make_backup_fname( file_infos *file )
{
char name[MAX_COLS];            
char *p;
int  i;
int  len;

   if (file->new_file)
      file->backed_up = TRUE;

   
   else {
      assert( strlen( file->file_name ) < MAX_COLS );
      strcpy( name, file->file_name );
      len = strlen( name );
      for (i=len,p=name+len; i>=0; i--) {

        
         if (*p == '.')
            break;

         
         else if (*p == '\\' || *p == ':') {
            p = name + len;
            break;

        
         } else if (i == 0) {
            p = name + len;
            break;
         }
         --p;
      }
      assert( strlen( name ) < MAX_COLS );
      strcpy( p, ".bak" );
      strcpy( file->backup_fname, name );
   }
}



int  file_file( WINDOW *window )
{
   if (save_file( window ) == OK)
      finish( window );
   return( OK );
}



int  save_file( WINDOW *window )
{
char name[MAX_COLS]; 
register file_infos *file;
int  rc;
line_list_ptr temp_ll;

   entab_linebuff( );
   if (un_copy_line( window->ll, window, TRUE ) == ERROR)
      return( ERROR );
   file = window->file_info;
   if (file->modified == FALSE)
      return( OK );
   
   assert( strlen( file->file_name ) < MAX_COLS );
   strcpy( name, file->file_name );

   if (strlen( name ) == 0)
      rc = save_as_file( window );
   else {
      
      rc = write_to_disk( window, name );
      if (rc != ERROR) {
         file->modified = FALSE;
         file->new_file = FALSE;
      }
   }

  
   if (rc == OK) {
      temp_ll = window->file_info->line_list;
      for (; temp_ll->len != EOF; temp_ll=temp_ll->next)
         temp_ll->dirty = FALSE;
      window->file_info->dirty = GLOBAL;
   }
   return( rc );
}



int  save_backup( WINDOW *window )
{
  
   return( write_to_disk( window, window->file_info->backup_fname ) );
}



int  write_to_disk( WINDOW *window, char *fname )
{
char name[MAX_COLS]; 
char status_line[MAX_COLS+2]; 
char line_buff[(MAX_COLS+1)*2]; 
register file_infos *file;
int  rc;
int  prompt_line;
int  fattr;

   file = window->file_info;
   prompt_line = window->bottom_line;

   
   assert( strlen( fname ) < MAX_COLS );
   strcpy( name, fname );
   save_screen_line( 0, prompt_line, line_buff );
   eol_clear( 0, prompt_line, g_display.message_color );

   combine_strings( status_line, utils6, name, "'" );
   s_output( status_line, prompt_line, 0, g_display.message_color );
   if ((rc = hw_save( name, file, 1L, file->length, NOTMARKED )) == ERROR) {
      if (ceh.flag != ERROR) {
         if (get_fattr( name, &fattr ) == OK && fattr & READ_ONLY)
        
            combine_strings( status_line, utils7a, name, utils7b );
         else
           
            combine_strings( status_line, utils8, name, "'" );
         error( WARNING, prompt_line, status_line );
      }
   }
   restore_screen_line( 0, prompt_line, line_buff );
   return( rc );
}



int  save_as_file( WINDOW *window )
{
char name[MAX_COLS];            
char line_buff[(MAX_COLS+1)*2]; 
int  prompt_line;
int  rc;
int  fattr;
register WINDOW *win;           
line_list_ptr temp_ll;

   win = window;
   entab_linebuff( );
   if (un_copy_line( win->ll, win, TRUE ) == ERROR)
      return( ERROR );
  
   prompt_line = win->bottom_line;
   save_screen_line( 0, prompt_line, line_buff );
   name[0] = '\0';
   
   if ((rc = get_name( utils9, prompt_line, name,
                       g_display.message_color )) == OK  &&  *name != '\0') {

      
      rc = get_fattr( name, &fattr );
      if (rc == OK) {   
         
         set_prompt( utils10, prompt_line );
         if (get_yn( ) != A_YES  ||  change_mode( name, prompt_line ) == ERROR)
            rc = ERROR;
      }
      if (rc != ERROR)
         rc = write_to_disk( win, name );

      
   }
   restore_screen_line( 0, prompt_line, line_buff );
   return( rc );
}



int  change_fattr( WINDOW *window )
{
char name[MAX_COLS];            
char line_buff[(MAX_COLS+1)*2]; 
file_infos *file;
WINDOW *wp;
int  prompt_line;
register int ok;
unsigned char fattr;
char *s;
int  rc;

   prompt_line = window->bottom_line;
   save_screen_line( 0, prompt_line, line_buff );
   name[0] = '\0';
  
   if ((ok = get_name( utils14, prompt_line, name,
                       g_display.message_color )) == OK) {
      if (*name != '\0') {
         fattr = 0;
         s = name;

         
         while (ok = *s++) {
            switch (ok) {
               case 'a' :
               case 'A' :
                  fattr |= ARCHIVE;
                  break;
               case 's' :
               case 'S' :
                  fattr |= SYSTEM;
                  break;
               case 'h' :
               case 'H' :
                  fattr |= HIDDEN;
                  break;
               case 'r' :
               case 'R' :
                  fattr |= READ_ONLY;
                  break;
               default :
                  break;
            }
         }
         file = window->file_info;
         if (set_fattr( file->file_name, fattr ))
          
            error( WARNING, prompt_line, utils15 );
         else {
            file->file_attrib = fattr;
            for (wp=g_status.window_list; wp!=NULL; wp=wp->next) {
               if (wp->file_info == file && wp->visible)
                  show_window_fname( wp );
            }
         }
      }
      rc = OK;
   } else
      rc = ERROR;
   restore_screen_line( 0, prompt_line, line_buff );
   return( rc );
}



int  get_fattr( char far *fname, int *fattr )
{
int  rc;                
int  attr;

   ASSEMBLE {
        push    ds
        mov     dx, WORD PTR fname      
        mov     ax, WORD PTR fname+2    
        mov     ds, ax                  
        mov     ax, 0x4300              
        int     0x21                    
        pop     ds

        jc      an_error                
        xor     ax, ax                  
        jmp     SHORT get_out           
   }
an_error:


   ASSEMBLE {
        xor     cx, cx                  
   }
get_out:

   ASSEMBLE {
        mov     WORD PTR rc, ax         
        mov     WORD PTR attr, cx       
   }
   *fattr = attr;
   if (ceh.flag == ERROR)
      rc = ERROR;
   return( rc );
}



int  set_fattr( char far *fname, int fattr )
{
int  rc;                

   ASSEMBLE {
        push    ds
        mov     dx, WORD PTR fname      
        mov     ax, WORD PTR fname+2    
        mov     ds, ax                  
        mov     cx, WORD PTR fattr      
        mov     ax, 0x4301              
        int     0x21                    
        pop     ds

        jc      get_out                 
        xor     ax, ax                  
   }
get_out:

   ASSEMBLE {
        mov     WORD PTR rc, ax         
   }
   if (ceh.flag == ERROR)
      rc = ERROR;
   return( rc );
}



int  get_current_directory( char far *path, int drive )
{
int  rc;

   ASSEMBLE {
        push    si                      
        push    ds                      

        mov     dx, WORD PTR drive      
        mov     si, WORD PTR path       
        mov     ax, WORD PTR path+2     
        mov     ds, ax                  
        mov     ah, 0x47                
        int     0x21                    
        xor     ax, ax                  
        jnc     no_error                
        mov     ax, ERROR               
   }
no_error:

   ASSEMBLE {
        pop     ds                      
        pop     si                      
        mov     WORD PTR rc, ax         
   }
   if (ceh.flag == ERROR)
      rc = ERROR;
   return( rc );
}



int  set_current_directory( char far *new_path )
{
int  rc;

   ASSEMBLE {
        push    ds                      

        mov     dx, WORD PTR new_path   
        mov     ax, WORD PTR new_path+2 
        mov     ds, ax                  
        mov     ah, 0x3b                
        int     0x21                    
        xor     ax, ax                  
        jnc     no_error                
        mov     ax, ERROR               
   }
no_error:

   ASSEMBLE {
        pop     ds                      
        mov     WORD PTR rc, ax         
   }
   if (ceh.flag == ERROR)
      rc = ERROR;
   return( rc );
}
