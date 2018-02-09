

#include "tdestr.h"
#include "common.h"
#include "tdefunc.h"
#include "define.h"



int  mark_block( WINDOW *window )
{
int type;
int num;
long lnum;
register file_infos *file;      /* ��ʱ�ļ� */
register WINDOW *win;           /* �ѵ�ǰ����ָ��ŵ�һ����ʱ�Ĵ������� */
int rc;

   win  = window;
   file = win->file_info;
   if (win->rline > file->length || win->ll->len == EOF)
      return( ERROR );
   if (g_status.marked == FALSE) {
      g_status.marked = TRUE;
      g_status.marked_file = file;
   }
   if (g_status.command == MarkBox)
      type = BOX;
   else if (g_status.command == MarkLine)
      type = LINE;
   else if (g_status.command == MarkStream)
      type = STREAM;
   else
      return( ERROR );

   rc = OK;
   /*
    * ������һ���ļ�����顣�û�����������ļ����κδ��ڽ��в�����
    */
   if (file == g_status.marked_file) {

      /*
       * ���ܿ��ģʽ��ʶ�����ʼ����ֹλ��
       */
      if (file->block_type == NOTMARKED) {
         file->block_ec  = file->block_bc = win->rcol;
         file->block_er  = file->block_br = win->rline;
      } else {
         if (file->block_br > win->rline) {
            file->block_br = win->rline;
            if (file->block_bc < win->rcol && type != STREAM)
               file->block_ec = win->rcol;
            else
               file->block_bc = win->rcol;
         } else {
            if (type != STREAM) {
               file->block_ec = win->rcol;
               file->block_er = win->rline;
            } else {
               if (win->rline == file->block_br &&
                   win->rline == file->block_er) {
                  if (win->rcol < file->block_bc)
                     file->block_bc = win->rcol;
                  else
                     file->block_ec = win->rcol;
               } else if (win->rline == file->block_br)
                  file->block_bc = win->rcol;
               else {
                  file->block_ec = win->rcol;
                  file->block_er = win->rline;
               }
            }
         }

         /*
          * ����û���ʶ�Ŀ����ֹλ������ʼλ��ǰ����ô��������λ�á�
          */
         if (file->block_er < file->block_br) {
            lnum = file->block_er;
            file->block_er = file->block_br;
            file->block_br = lnum;
         }

         /*
          * ����û���ʶ�Ŀ����ֹ������ʼ��ǰ����ô��������λ�á�
          */
         if ((file->block_ec < file->block_bc) && (type != STREAM ||
              (type == STREAM && file->block_br == file->block_er))) {
            num = file->block_ec;
            file->block_ec = file->block_bc;
            file->block_bc = num;
         }
      }

      /*
       * ����������Ѿ������壬��������û�ʹ�û��ģʽ����ô���
	   * ���ͱ���Ϊ��ǰ�������
       
       */
      if (file->block_type != NOTMARKED) {
         /*
          * �����������Ǿ��ο飬��ôҪ��֤���Ͻ�С�����½�
          * ������������stream�飬��ô��֤��ʼ��С����ֹ��
          */
         if (type == BOX) {
            if (file->block_ec < file->block_bc) {
               num = file->block_ec;
               file->block_ec = file->block_bc;
               file->block_bc = num;
            }
         }
      }

      assert( file->block_er >= file->block_br );

      file->block_type = type;
      file->dirty = GLOBAL;
   } else {
      /*
       * �Ѿ�����ÿ�
       */
      error( WARNING, win->bottom_line, block1 );
      rc = ERROR;
   }
   return( rc );
}



int  unmark_block( WINDOW *arg_filler )
{
register file_infos *marked_file;

   if (g_status.marked == TRUE) {
      marked_file              = g_status.marked_file;
      g_status.marked          = FALSE;
      g_status.marked_file     = NULL;
      marked_file->block_start = NULL;
      marked_file->block_end   = NULL;
      marked_file->block_bc    = marked_file->block_ec = 0;
      marked_file->block_br    = marked_file->block_er = 0l;
      if (marked_file->block_type)
         marked_file->dirty = GLOBAL;
      marked_file->block_type  = NOTMARKED;
   }
   return( OK );
}



void restore_marked_block( WINDOW *window, int net_change )
{
long length;
register file_infos *marked_file;

   if (g_status.marked == TRUE && net_change != 0) {
      marked_file = g_status.marked_file;
      length = marked_file->length;

     
      if (marked_file == window->file_info) {

        
         if (marked_file->block_br > window->rline) {
            marked_file->block_br += net_change;
            marked_file->block_er += net_change;
            marked_file->dirty = GLOBAL;
        
         } else if (marked_file->block_er >= window->rline)
            marked_file->dirty = GLOBAL;

         
         if (marked_file->block_br > length)
            unmark_block( window );
         else if (marked_file->block_er > length) {
            marked_file->block_er = length;
            marked_file->dirty = GLOBAL;
         }
      }
   }
}



int  prepare_block( WINDOW *window, line_list_ptr ll, int bc )
{
register int pad = 0;  
register int len;

   assert( bc >= 0 );
   assert( bc < MAX_LINE_LENGTH );
   assert( ll->len != EOF );
   assert( g_status.copied == FALSE );

   copy_line( ll );
   detab_linebuff( );

   len = g_status.line_buff_len;
   pad = bc - len;
   if (pad > 0) {
      

      assert( pad >= 0 );
      assert( pad < MAX_LINE_LENGTH );

      memset( g_status.line_buff+len, ' ', pad );
      g_status.line_buff_len += pad;
   }
   
   return( un_copy_line( ll, window, FALSE ) );
}



int  pad_dest_line( WINDOW *window, file_infos *dest_file, line_list_ptr ll )
{
int rc;
text_ptr l;
line_list_ptr new_node;

   rc = OK;

   l = NULL;
   new_node = (line_list_ptr)my_malloc( sizeof(line_list_struc), &rc );
   if (rc == OK) {
      new_node->len   = 0;
      new_node->dirty = FALSE;
      new_node->line  = l;
      if (ll->next != NULL) {
         ll->next->prev = new_node;
         new_node->next = ll->next;
         ll->next = new_node;
         new_node->prev = ll;
      } else {
         new_node->next = ll;
         if (ll->prev != NULL)
            ll->prev->next = new_node;
         new_node->prev = ll->prev;
         ll->prev = new_node;
         if (new_node->prev == NULL)
            window->file_info->line_list = new_node;
      }
      ++dest_file->length;
   } else {
     
      error( WARNING, window->bottom_line, block4 );
      if (new_node != NULL)
         my_free( new_node );
      rc = ERROR;
   }
   return( rc );
}


int  move_copy_delete_overlay_block( WINDOW *window )
{
int  action;
WINDOW *source_window;          
line_list_ptr source;           
line_list_ptr dest;             
long number;                    
int  lens;                      
int  lend;                      
int  add;                       
int  block_len;                 
line_list_ptr block_start;      
line_list_ptr block_end;  
int  prompt_line;
int  same;                      
int  source_first;              
file_infos *source_file;
file_infos *dest_file;
int  rcol, bc, ec;              
long rline;                     
long br, er;                    
long block_num;                 
long block_inc;                 
int  block_just;                
int  block_type;
int  fill_char;
int  rc;

   entab_linebuff( );
   rc = un_copy_line( window->ll, window, TRUE );
   if (g_status.marked == FALSE || rc == ERROR)
      return( ERROR );
   switch (g_status.command) {
      case MoveBlock :
         action = MOVE;
         break;
      case DeleteBlock :
         action = DELETE;
         break;
      case CopyBlock :
         action = COPY;
         break;
      case KopyBlock :
         action = KOPY;
         break;
      case FillBlock :
         action = FILL;
         break;
      case OverlayBlock :
         action = OVERLAY;
         break;
      case NumberBlock :
         action = NUMBER;
         break;
      case SwapBlock :
         action = SWAP;
         break;
      default :
         return( ERROR );
   }
   source_file = g_status.marked_file;
   source_window = g_status.window_list;
   for (; ptoul( source_window->file_info ) != ptoul( source_file );)
      source_window = source_window->next;
   prompt_line = window->bottom_line;
   dest_file = window->file_info;
   check_block( );
   if (g_status.marked == FALSE)
      return( ERROR );
   block_start = source_file->block_start;
   block_end = source_file->block_end;
   if (block_start == NULL  ||  block_end == NULL)
      return( ERROR );

   block_type = source_file->block_type;
   if (block_type != LINE  &&  block_type != STREAM  &&  block_type != BOX)
      return( ERROR );

   dest = window->ll;
   rline = window->rline;
   if (dest->len == EOF)
      return( ERROR );
   rc = OK;

   bc = source_file->block_bc;
   ec = source_file->block_ec;
   br = source_file->block_br;
   er = source_file->block_er;

  
   rcol =  (action == FILL || action == NUMBER) ? bc : window->rcol;

   
   source_first = same = FALSE;
   if (action == FILL) {
      if (block_type == BOX) {
         if (get_block_fill_char( window, &fill_char ) == ERROR)
            return( ERROR );
         dest = block_start;
         same = TRUE;
      } else {
        
         error( WARNING, prompt_line, block2 );
         return( ERROR );
      }
   }
   block_inc = 1;
   if (action == NUMBER) {
      if (block_type == BOX) {
         if (get_block_numbers( window, &block_num, &block_inc, &block_just )
              == ERROR)
            return( ERROR );
         dest = block_start;
         same = TRUE;
      } else {
        
         error( WARNING, prompt_line, block3a );
         return( ERROR );
      }
   }
   if (action == SWAP) {
      if (block_type != BOX) {
        
         error( WARNING, prompt_line, block3b );
         return( ERROR );
      }
   }
   if (source_file == dest_file && action != DELETE && action != FILL) {
      same = TRUE;
      if (block_type == BOX && action == MOVE) {
         if (rline == br  &&  (rcol >= bc && rcol <= ec))
            
            return( ERROR );
      } else if (block_type == LINE || block_type == STREAM) {
         if (rline >= br && rline <= er) {
            if (block_type == LINE) {
               
               if (action == COPY || action == KOPY)
                  dest = block_end;
                
               else if (action == MOVE)
                  return( ERROR );
            } else {

              
               if ((rline > br && rline < er) ||
                   (br == er && rcol >= bc && rcol <= ec) ||
                   (br != er && ((rline == br && rcol >= bc) ||
                                 (rline == er && rcol <= ec)))) {

               
                  if (action == COPY || action == KOPY) {
                     dest = block_end;
                     rcol = ec + 1;
                     rline = er;
                  } else if (action == MOVE)
                     return( ERROR );
               }
            }
         }
      }
   }
   if (br < rline)
      source_first = TRUE;

   block_len = (ec+1) - bc;
   if (block_type == BOX) {
      if (action != DELETE && action != FILL) {
         if (rcol + block_len > MAX_LINE_LENGTH) {
          
            error( WARNING, prompt_line, ltol );
            return( ERROR );
         }
      }
   } else if (block_type == LINE) {
      block_len = 0;
      if (action == OVERLAY) {
        
         error( WARNING, prompt_line, block5 );
         return( ERROR );
      }
   } else if (block_type == STREAM) {

      if (action == OVERLAY) {
         
         error( WARNING, prompt_line, block5 );
         return( ERROR );
      }

      lend = block_end->len;
      if (action == DELETE || action == MOVE) {

        
         if (lend > ec)
            lend -= ec;
         else
            lend = 0;
         if (bc + lend > MAX_LINE_LENGTH) {
           
            error( WARNING, prompt_line, ltol );
            return( ERROR );
         }
      }

      if (action != DELETE) {

         
         lens = block_start->len;

        
         if (dest->line == window->ll->line)
            add = dest->len;
         else
            add = lens;

         
         if (lens > bc)
            lens -= bc;
         else
            lens = 0;
         if (rcol + lens > MAX_LINE_LENGTH) {
           
            error( WARNING, prompt_line, ltol );
            return( ERROR );
         }

       
         if (add > bc)
            add -= bc;
         else
            add = 0;
         if (lend > ec)
            lend -= ec;
         else
            lend = 0;
         if (add + lend > MAX_LINE_LENGTH) {
           
            error( WARNING, prompt_line, ltol );
            return( ERROR );
         }
      }
      if (ptoul( block_start ) == ptoul( block_end )) {
         block_type = BOX;
         block_len = (ec+1) - bc;
      }
   }

   if (mode.do_backups == TRUE) {
      switch (action) {
         case MOVE :
         case DELETE :
         case COPY :
         case KOPY :
         case SWAP :
            window->file_info->modified = TRUE;
            rc = backup_file( window );
            break;
      }
      switch (action) {
         case MOVE :
         case DELETE :
         case FILL :
         case NUMBER :
         case SWAP :
            source_window->file_info->modified = TRUE;
            if (rc != ERROR)
               rc = backup_file( source_window );
            break;
      }
   }
   source = block_start;

   assert( block_start != NULL );
   assert( block_start->len != EOF );
   assert( block_end != NULL );
   assert( block_end->len != EOF );

   if (block_type == LINE)
      do_line_block( window,  source_window,  action,
                     source_file,  dest_file,  block_start,  block_end,
                     source,  dest,  br,  er,  &rc );

   else if (block_type == STREAM)
      do_stream_block( window,  source_window,  action,
                       source_file,  dest_file,  block_start,  block_end,
                       source,  dest,  rline,  br,  er,  bc,  ec,  rcol,  &rc );

   else
      do_box_block( window,  source_window,  action,
                    source_file,  dest_file,  source,  dest,  br,  er,
                    block_inc, rline, block_num, block_just, fill_char,
                    same, block_len, bc, ec,  rcol, &rc );

   dest_file->modified = TRUE;
   dest_file->dirty = GLOBAL;
   if (action == MOVE || action == DELETE || action == FILL || action==NUMBER) {
      source_file->modified = TRUE;
      source_file->dirty = GLOBAL;
   }

  
   if (action == KOPY) {
      if (same && !source_first && block_type == LINE  &&  rc != ERROR) {
         number = (er+1) - br;
         source_file->block_br += number;
         source_file->block_er += number;
      }
   } else if (action != FILL && action != OVERLAY && action != NUMBER)
      unmark_block( window );
   show_avail_mem( );
   g_status.copied = FALSE;
   return( rc );
}



void do_line_block( WINDOW *window,  WINDOW *source_window,  int action,
                    file_infos *source_file,  file_infos *dest_file,
                    line_list_ptr block_start,  line_list_ptr block_end,
                    line_list_ptr source,  line_list_ptr dest,
                    long br,  long er, int *rc )
{
line_list_ptr temp_ll;          
text_ptr l;
int  lens;                      
long li;                        
long diff;

   if (action == COPY || action == KOPY) {

      assert( br >= 1 );
      assert( br <= source_file->length );
      assert( er >= br );
      assert( er <= source_file->length );

      for (li=br; li <= er  &&  *rc == OK; li++) {
         lens = source->len;

         assert( lens * sizeof(char) < MAX_LINE_LENGTH );

         l = (text_ptr)my_malloc( lens * sizeof(char), rc );
         temp_ll = (line_list_ptr)my_malloc( sizeof(line_list_struc), rc );
         if (*rc == OK) {
            if (lens > 0)
               _fmemcpy( l, source->line, lens );
            temp_ll->line  = l;
            temp_ll->len   = lens;
            temp_ll->dirty = TRUE;

            if (dest->next != NULL) {
               dest->next->prev = temp_ll;
               temp_ll->next = dest->next;
               dest->next = temp_ll;
               temp_ll->prev = dest;
            } else {
               temp_ll->next = dest;
               if (dest->prev != NULL)
                  dest->prev->next = temp_ll;
               temp_ll->prev = dest->prev;
               dest->prev = temp_ll;
               if (temp_ll->prev == NULL)
                  window->file_info->line_list = temp_ll;
            }

            dest = temp_ll;
            source = source->next;
         } else {
           
            error( WARNING, window->bottom_line, dir3 );
            if (l != NULL)
               my_free( l );
            if (temp_ll != NULL)
               my_free( temp_ll );
            *rc = ERROR;
            er = li - 1;
         }
      }
   } else if (action == MOVE) {
      if (dest->len != EOF  &&  dest->next != NULL) {
         temp_ll = block_start;
         for (li=br; li <= er  &&  *rc == OK; li++) {
            temp_ll->dirty = TRUE;
            temp_ll = temp_ll->next;
         }
         if (block_start->prev == NULL)
            source_file->line_list = block_end->next;
         if (block_start->prev != NULL)
            block_start->prev->next = block_end->next;
         block_end->next->prev = block_start->prev;
         dest->next->prev = block_end;
         block_start->prev = dest;
         block_end->next = dest->next;
         dest->next = block_start;
      }
   } else if (action == DELETE) {
      block_end->next->prev = block_start->prev;
      if (block_start->prev == NULL)
         source_file->line_list = block_end->next;
      else
         block_start->prev->next = block_end->next;
      block_end->next = NULL;
      while (block_start != NULL) {
         temp_ll = block_start;
         block_start = block_start->next;
         if (temp_ll->line != NULL)
            my_free( temp_ll->line );
         my_free( temp_ll );
      }
   }

   diff =  er + 1L - br;
   if (action == COPY || action == KOPY || action == MOVE)
      dest_file->length += diff;
   if (action == DELETE || action == MOVE)
      source_file->length -= diff;
   if (action == DELETE && source_window->rline >= br) {
      source_window->rline -= diff;
      if (source_window->rline < br)
         source_window->rline = br;
   }
  
   restore_cursors( dest_file );
   if (dest_file != source_file)
      restore_cursors( source_file );
   show_avail_mem( );
}



void do_stream_block( WINDOW *window,  WINDOW *source_window,  int action,
                    file_infos *source_file,  file_infos *dest_file,
                    line_list_ptr block_start,  line_list_ptr block_end,
                    line_list_ptr source,  line_list_ptr dest, long rline,
                    long br,  long er, int bc, int ec, int rcol, int *rc )
{
line_list_ptr temp_ll;          
text_ptr l;
int  lens;                      
int  lend;                      
long li;                        
long diff;
WINDOW s_w, d_w;                

   dup_window_info( &s_w, source_window );
   dup_window_info( &d_w, window );
   s_w.rline   = br;
   s_w.ll      = block_start;
   s_w.visible = FALSE;
   d_w.rline   = rline;
   d_w.ll      = dest;
   d_w.visible = FALSE;

   
   lens = block_start->len;
   detab_a_line( block_start->line, &lens );
   if (lens < bc || mode.inflate_tabs)
      *rc = prepare_block( &s_w, block_start, bc );

   
   lens = block_end->len;
   detab_a_line( block_end->line, &lens );
   if (*rc == OK  &&  (lens < ec+1  ||  mode.inflate_tabs))
      *rc = prepare_block( &s_w, block_end, ec+1 );

   
   copy_line( dest );
   detab_linebuff( );
   *rc = un_copy_line( dest, &d_w, FALSE );
   lend = dest->len;
   if (*rc == OK && (action==MOVE || action==COPY || action==KOPY)) {
      if (lend < rcol || mode.inflate_tabs)
         *rc = prepare_block( &d_w, dest, rcol );
   }

   if ((action == COPY || action == KOPY) && *rc == OK) {

   
      lens = dest->len - rcol;

      assert( lens >= 0 );
      assert( lens <= MAX_LINE_LENGTH );
      assert( ec + 1 >= 0 );
      assert( ec + 1 <= MAX_LINE_LENGTH );
      assert( rcol >= 0 );

      _fmemcpy( g_status.line_buff, block_end->line, ec+1 );
      _fmemcpy( g_status.line_buff+ec+1, dest->line+rcol, lens );
      lens += ec + 1;
      g_status.line_buff_len = lens;

      temp_ll = (line_list_ptr)my_malloc( sizeof(line_list_struc), rc );
      if (*rc == OK) {
         temp_ll->line  = NULL;
         temp_ll->len   = 0;
         temp_ll->dirty = FALSE;
         g_status.copied = TRUE;
         *rc = un_copy_line( temp_ll, &d_w, TRUE );

         if (*rc == OK) {
            dest->next->prev = temp_ll;
            temp_ll->next = dest->next;
            dest->next = temp_ll;
            temp_ll->prev = dest;
         } else
            if (temp_ll != NULL)
               my_free( temp_ll );
      } else {
         if (temp_ll != NULL)
            my_free( temp_ll );
      }

      if (*rc != OK)
         error( WARNING, window->bottom_line, dir3 );

      if (*rc == OK) {
         g_status.copied = FALSE;
         copy_line( dest );
         lens = block_start->len - bc;

         assert( lens >= 0 );
         assert( lens <= MAX_LINE_LENGTH );
         assert( bc >= 0 );
         assert( bc <= MAX_LINE_LENGTH );
         assert( rcol >= 0 );

         _fmemcpy( g_status.line_buff+rcol, block_start->line+bc, lens );
         lens = rcol + lens;
         g_status.line_buff_len = lens;
         *rc = un_copy_line( dest, &d_w, TRUE );
      }

      source = block_start->next;
      for (li=br+1; li < er  &&  *rc == OK; li++) {
         lens = source->len;
         temp_ll = (line_list_ptr)my_malloc( sizeof(line_list_struc), rc );

         assert( lens >= 0 );
         assert( lens <= MAX_LINE_LENGTH );

         l = (text_ptr)my_malloc( lens * sizeof(char), rc );
         if (*rc == OK) {
            if (lens > 0)
               _fmemcpy( l, source->line, lens );
            temp_ll->line  = l;
            temp_ll->len   = lens;
            temp_ll->dirty = TRUE;

            if (dest->next != NULL) {
               dest->next->prev = temp_ll;
               temp_ll->next = dest->next;
               dest->next = temp_ll;
               temp_ll->prev = dest;
            } else {
               temp_ll->next = dest;
               if (dest->prev != NULL)
                  dest->prev->next = temp_ll;
               temp_ll->prev = dest->prev;
               dest->prev = temp_ll;
               if (temp_ll->prev == NULL)
                  window->file_info->line_list = temp_ll;
            }

            dest = temp_ll;
            source = source->next;
         } else {
          
            error( WARNING, window->bottom_line, dir3 );
            if (l != NULL)
               my_free( l );
            if (temp_ll != NULL)
               my_free( temp_ll );
            *rc = WARNING;
         }
      }
   } else if (action == MOVE) {

      if (ptoul( dest ) == ptoul( block_start )) {

        
         lens = bc - rcol;
         lend = block_end->len - (ec + 1);
         g_status.copied = FALSE;
         copy_line( block_end );


         assert( lens >= 0 );
         assert( lens <= MAX_LINE_LENGTH );
         assert( lend >= 0 );
         assert( lend <= MAX_LINE_LENGTH );
         assert( ec + lens + 1 <= MAX_LINE_LENGTH );
         assert( rcol >= 0 );


         _fmemmove( g_status.line_buff + ec + lens + 1,
                    g_status.line_buff + ec + 1,  lend );
         _fmemcpy( g_status.line_buff+ec+1, block_start->line+rcol, lens );
         g_status.line_buff_len = block_end->len + lens;
         *rc = un_copy_line( block_end, &d_w, TRUE );

         if (*rc == OK) {
            lend = block_start->len - bc;
            copy_line( block_start );

            assert( lend >= 0 );
            assert( lend < MAX_LINE_LENGTH );

            _fmemmove( g_status.line_buff + rcol,
                       g_status.line_buff + bc, lend );

            assert( block_start->len - (bc - rcol) >= 0 );
            assert( block_start->len - (bc - rcol) <= MAX_LINE_LENGTH );

            g_status.line_buff_len = block_start->len - (bc - rcol);
            *rc = un_copy_line( block_start, &d_w, TRUE );
         }

     
      } else if (ptoul( dest ) == ptoul( block_end )) {

        
         lens = rcol - ec;
         lend = block_start->len - bc;
         g_status.copied = FALSE;
         copy_line( block_start );

         assert( lens >= 0 );
         assert( lens <= MAX_LINE_LENGTH );
         assert( lend >= 0 );
         assert( lend <= MAX_LINE_LENGTH );
         assert( bc + lens <= MAX_LINE_LENGTH );
         assert( ec + 1 >= 0 );

         _fmemmove( g_status.line_buff + bc + lens,
                    g_status.line_buff + bc,  lend );
         _fmemcpy( g_status.line_buff+bc, block_end->line+ec+1, lens );

         assert( block_start->len + lens >= 0 );
         assert( block_start->len + lens <= MAX_LINE_LENGTH );

         g_status.line_buff_len = block_start->len + lens;
         *rc = un_copy_line( block_start, &d_w, TRUE );

        
         if (*rc == OK) {
            lend = block_end->len - (rcol + 1);
            copy_line( block_end );

            assert( lend >= 0 );
            assert( lend <= MAX_LINE_LENGTH );
            assert( ec + 1 >= 0 );
            assert( rcol + 1 >= 0 );
            assert( ec + 1 <= MAX_LINE_LENGTH );
            assert( rcol + 1 <= MAX_LINE_LENGTH );
            assert( block_end->len - (rcol - ec) >= 0 );
            assert( block_end->len - (rcol - ec) <= MAX_LINE_LENGTH );


            _fmemmove( g_status.line_buff + ec + 1,
                       g_status.line_buff + rcol + 1, lend );
            g_status.line_buff_len = block_end->len - (rcol - ec);
            *rc = un_copy_line( block_end, &d_w, TRUE );
         }
      } else {

         lens = dest->len - rcol;

         assert( ec + 1 >= 0 );
         assert( ec + 1 <= MAX_LINE_LENGTH );
         assert( lens >= 0 );
         assert( lens <= MAX_LINE_LENGTH );
         assert( rcol >= 0 );
         assert( rcol <= MAX_LINE_LENGTH );

         _fmemcpy( g_status.line_buff, block_end->line, ec+1 );
         _fmemcpy( g_status.line_buff+ec+1, dest->line+rcol, lens );
         lens += ec + 1;
         g_status.line_buff_len = lens;

         temp_ll = (line_list_ptr)my_malloc( sizeof(line_list_struc), rc );
         if (*rc == OK) {
            temp_ll->line  = NULL;
            temp_ll->len   = 0;
            temp_ll->dirty = FALSE;
            g_status.copied = TRUE;
            *rc = un_copy_line( temp_ll, &d_w, TRUE );

            if (*rc != ERROR) {
               dest->next->prev = temp_ll;
               temp_ll->next = dest->next;
               dest->next = temp_ll;
               temp_ll->prev = dest;
            } else
               if (temp_ll != NULL)
                  my_free( temp_ll );
         } else {
            if (temp_ll != NULL)
               my_free( temp_ll );
         }

       
         if (*rc != OK)
            error( WARNING, window->bottom_line, dir3 );

         if (*rc == OK) {
            copy_line( dest );
            lens = block_start->len - bc;

            assert( bc >= 0 );
            assert( bc <= MAX_LINE_LENGTH );
            assert( lens >= 0 );
            assert( lens <= MAX_LINE_LENGTH );
            assert( rcol >= 0 );
            assert( rcol <= MAX_LINE_LENGTH );

            _fmemcpy( g_status.line_buff+rcol, block_start->line+bc, lens );
            g_status.line_buff_len = lens + rcol;
            *rc = un_copy_line( dest, &d_w, TRUE );
            dest->dirty = TRUE;
         }

         if (*rc == OK  &&  ptoul( block_start->next ) != ptoul( block_end )) {
            block_start->next->prev = dest;
            temp_ll->prev = block_end->prev;
            block_end->prev->next = temp_ll;
            dest->next = block_start->next;
         }

         if (*rc == OK) {
            copy_line( block_start );
            detab_linebuff( );
            lend = bc;
            lens = block_end->len - (ec + 1);

            assert( bc >= 0 );
            assert( bc <= MAX_LINE_LENGTH );
            assert( lens >= 0 );
            assert( lens <= MAX_LINE_LENGTH );
            assert( lend >= 0 );
            assert( lend <= MAX_LINE_LENGTH );
            assert( ec + 1 >= 0 );
            assert( ec + 1 <= MAX_LINE_LENGTH );
            assert( lens + lend >= 0 );
            assert( lens + lend <= MAX_LINE_LENGTH );

            _fmemcpy( g_status.line_buff+bc, block_end->line+ec+1, lens );
            g_status.line_buff_len = lend + lens;
            *rc = un_copy_line( block_start, &s_w, TRUE );
            block_start->dirty = TRUE;
            block_start->next = block_end->next;
            block_end->next->prev = block_start;
            if (block_end->line != NULL)
               my_free( block_end->line );
            my_free( block_end );
         }
      }
   } else if (action == DELETE) {
      copy_line( block_start );
      lens = block_end->len - (ec + 1);

      assert( bc >= 0 );
      assert( bc <= MAX_LINE_LENGTH );
      assert( lens >= 0 );
      assert( lens <= MAX_LINE_LENGTH );
      assert( ec + 1 >= 0 );
      assert( ec + 1 <= MAX_LINE_LENGTH );
      assert( bc + lens >= 0 );
      assert( bc + lens <= MAX_LINE_LENGTH );

      _fmemcpy( g_status.line_buff+bc, block_end->line + ec+1, lens );
      g_status.line_buff_len = bc + lens;
      *rc = un_copy_line( block_start, &s_w, TRUE );
      block_start->dirty = TRUE;
      source = block_start->next;
      block_start->next = block_end->next;
      block_end->next->prev = block_start;
      block_end->next = NULL;
      while (source != NULL) {
         temp_ll = source;
         source = source->next;
         if (temp_ll->line != NULL)
            my_free( temp_ll->line );
         my_free( temp_ll );
      }
   }

   if (*rc == OK) {
      diff = er - br;
      if (action == COPY || action == KOPY || action == MOVE)
         dest_file->length += diff;
      if (action == DELETE || action == MOVE)
         source_file->length -= diff;
      if (action == DELETE && source_window->rline >= br) {
         source_window->rline -= diff;
         if (source_window->rline < br)
            source_window->rline = br;
      }
   }

  
   restore_cursors( dest_file );
   if (dest_file != source_file)
      restore_cursors( source_file );
   show_avail_mem( );
}



void do_box_block( WINDOW *window,  WINDOW *source_window,  int action,
                    file_infos *source_file,  file_infos *dest_file,
                    line_list_ptr source,  line_list_ptr dest, long br,
                    long er, long block_inc,
                    long rline, long block_num, int block_just, int fill_char,
                    int same, int block_len, int bc, int ec, int rcol, int *rc )
{
line_list_ptr p;                
int  lens;                      
int  lend;                      
int  add;                       
char *block_buff;
char *swap_buff;
int  xbc, xec;                  
long li;                        
long dest_add;                  
WINDOW s_w, d_w;       
int  padded_file;
WINDOW *w;

   padded_file = FALSE;
   dup_window_info( &s_w, source_window );
   dup_window_info( &d_w, window );
   s_w.rline   = br;
   s_w.ll      = source;
   s_w.visible = FALSE;
   d_w.rline   = rline;
   d_w.ll      = dest;
   d_w.visible = FALSE;

   block_buff = (char *)calloc( BUFF_SIZE + 2, sizeof(char) );
   swap_buff  = (char *)calloc( BUFF_SIZE + 2, sizeof(char) );
   if (block_buff == NULL || swap_buff == NULL) {
      error( WARNING, window->bottom_line, block4 );
      *rc = ERROR;
   }

  
   if (*rc == OK  &&  (action == OVERLAY || action == SWAP) &&
           same  &&  rline > br  &&  rline <= er) {

     
      dest_add = rline - br;
      if (dest_add + er > window->file_info->length) {
         dest_add = dest_add - (window->file_info->length - er);
         p = dest_file->line_list_end->prev;
         for (; dest_add > 0  &&  *rc == OK; dest_add--)
            *rc = pad_dest_line( window, dest_file, p );
         padded_file = TRUE;
      }

      
      for (li=er-br; li > 0; li--) {
         load_undo_buffer( dest_file, dest->line, dest->len );
         dest = dest->next;
         ++d_w.rline;
         source = source->next;
         ++s_w.rline;
      }

      
      for (li=er; *rc == OK  &&  li >= br  &&  !g_status.control_break;
                                       li--, s_w.rline--, d_w.rline--) {
         lens = find_end( source->line, source->len );
         lend = find_end( dest->line, dest->len );
         if (lens != 0 || lend != 0) {
            load_box_buff( block_buff, source, bc, ec, ' ' );
            if (action == SWAP)
               load_box_buff( swap_buff, dest, rcol, rcol+block_len, ' ' );
            *rc = copy_buff_2file( &d_w, block_buff, dest, rcol,
                                    block_len, action );
            dest->dirty = TRUE;
            if (action == SWAP) {
               add = 0;
               *rc = copy_buff_2file( &s_w, swap_buff, source, bc,
                                block_len, action );
               source->dirty = TRUE;
            }
         }
         source = source->prev;
         dest = dest->prev;
      }
   } else {
      if (action == FILL)
         block_fill( block_buff, fill_char, block_len );
      for (li=br; *rc == OK  &&  li <= er  &&  !g_status.control_break;
                           li++, s_w.rline++, d_w.rline++) {
         lens = find_end( source->line, source->len );
         lend = find_end( dest->line, dest->len );

         switch (action) {
            case FILL    :
            case NUMBER  :
            case DELETE  :
            case MOVE    :
               load_undo_buffer( source_file, source->line, source->len );
               break;
            case COPY    :
            case KOPY    :
            case OVERLAY :
               load_undo_buffer( dest_file, dest->line, dest->len );
               break;
         }

       
         if (action == FILL || action == NUMBER) {
            if (action == NUMBER) {
              number_block_buff( block_buff, block_len, block_num, block_just );
              block_num += block_inc;
            }
            *rc = copy_buff_2file( &s_w, block_buff, source, rcol,
                                block_len, action );
            source->dirty = TRUE;

         
         } else if (lens != 0 || lend != 0) {

            
            if (action == MOVE     ||  action == COPY || action == KOPY ||
                action == OVERLAY  ||  action == SWAP) {
               xbc = bc;
               xec = ec;
               if (action != OVERLAY  &&  action != SWAP  &&  same) {
                  if (rcol < bc && rline > br && rline <=er)
                     if (li >= rline) {
                        xbc = bc + block_len;
                        xec = ec + block_len;
                     }
               }
               load_box_buff( block_buff, source, xbc, xec, ' ' );
               if (action == SWAP)
                  load_box_buff( swap_buff, dest, rcol, rcol+block_len, ' ' );
               *rc = copy_buff_2file( &d_w, block_buff, dest, rcol,
                                block_len, action );
               dest->dirty = TRUE;
               if (action == SWAP && *rc == OK) {
                  *rc = copy_buff_2file( &s_w, swap_buff, source, xbc,
                                   block_len, action );
                  source->dirty = TRUE;
               }
            }

           
            if (action == MOVE || action == DELETE) {
               lens = find_end( source->line, source->len );
               if (lens >= (bc + 1)) {
                  source->dirty = TRUE;
                  add = block_len;
                  xbc = bc;
                  if (lens <= (ec + 1))
                     add = lens - bc;
                  if (same && action == MOVE) {
                     if (rcol < bc && rline >= br && rline <=er)
                        if (li >= rline) {
                           xbc = bc + block_len;
                           if (lens <= (ec + block_len + 1))
                              add = lens - xbc;
                        }
                  }
                  if (add > 0)
                     *rc = delete_box_block( &s_w, source, xbc, add );
               }
            }
         }

        
         source = source->next;

         
         if (action != DELETE && action != FILL && action != NUMBER) {
            p = dest->next;
            if (p->len != EOF)
               dest = p;
            else if (li < er) {
               padded_file = TRUE;
               pad_dest_line( window, dest_file, p );
               dest = dest->next;
            }
         }
      }
   }
   if (block_buff != NULL)
      free( block_buff );
   if (swap_buff != NULL)
      free( swap_buff );
   if (padded_file) {
      w = g_status.window_list;
      while (w != NULL) {
         if (w->file_info == dest_file  &&  w->visible)
            show_size( w );
         w = w->next;
      }
   }
   show_avail_mem( );
}



void load_box_buff( char *block_buff, line_list_ptr ll, int bc, int ec,
                    char filler )
{
int len;
int avlen;
register int i;
register char *bb;
text_ptr s;

   assert( bc >= 0 );
   assert( ec >= bc );
   assert( ec < MAX_LINE_LENGTH );

   bb = block_buff;
   len = ll->len;
   s = detab_a_line( ll->line, &len );
  
   if (len < ec + 1) {
     
      assert( ec + 1 - bc >= 0 );

      memset( block_buff, filler, (ec + 1) - bc );
      if (len >= bc) {
        
         avlen = len - bc;
         s += bc;
         for (i=avlen; i>0; i--)
            *bb++ = *s++;
      }
   } else {
     
      avlen = (ec + 1) - bc;
      s = s + bc;
      for (i=avlen; i>0; i--)
         *bb++ = *s++;
   }
}



int  copy_buff_2file( WINDOW *window, char *block_buff, line_list_ptr dest,
                      int rcol, int block_len, int action )
{
char *s;
char *d;
int len;
int pad;
int add;

   copy_line( dest );
   if (mode.inflate_tabs)
      detab_linebuff( );

   len = g_status.line_buff_len;

   assert( len >= 0 );
   assert( len < MAX_LINE_LENGTH );
   assert( rcol >= 0 );
   assert( rcol < MAX_LINE_LENGTH );
   assert( block_len >= 0 );
   assert( block_len < BUFF_SIZE );

   if (rcol > len) {
      pad = rcol - len;

      assert( pad >= 0 );
      assert( pad < MAX_LINE_LENGTH );

      memset( g_status.line_buff + len, ' ', pad );
      len += pad;
   }

   s = g_status.line_buff + rcol;

 
   if (action == FILL || action == OVERLAY || action == NUMBER || action == SWAP) {
      add = len - rcol;
      if (add < block_len) {
         pad = block_len - add;

         assert( pad >= 0 );
         assert( pad < MAX_LINE_LENGTH );

         memset( g_status.line_buff + len, ' ', pad );
         len += pad;
      }
   } else {
      d = s + block_len;
      add = len - rcol;

      assert( add >= 0 );
      assert( add < MAX_LINE_LENGTH );

      memmove( d, s, add );
      len += block_len;
   }

   assert( rcol + block_len <= len );
   assert( len >= 0 );
   assert( len < MAX_LINE_LENGTH );

   memmove( s, block_buff, block_len );
   g_status.line_buff_len = len;
   if (mode.inflate_tabs)
      entab_linebuff( );
   return( un_copy_line( dest, window, TRUE ) );
}



void block_fill( char *block_buff, int fill_char, int block_len )
{
   assert( block_len >= 0 );
   assert( block_len < BUFF_SIZE );
   assert( block_buff != NULL );

   memset( block_buff, fill_char, block_len );
}



void number_block_buff( char *block_buff, int block_len, long block_num,
                        int just )
{
int len;                
int i;
char temp[MAX_COLS];    

   assert( block_len >= 0 );
   assert( block_len < BUFF_SIZE );

   block_fill( block_buff, ' ', block_len );
   len = strlen( ltoa( block_num, temp, 10 ) );
   if (just == RIGHT) {
      block_len--;
      len--;
      for (;block_len >= 0 && len >= 0; block_len--, len--)
         block_buff[block_len] = temp[len];
   } else {
      for (i=0; block_len > 0 && i < len; block_len--, i++)
         block_buff[i] = temp[i];
   }
}



void restore_cursors( file_infos *file )
{
register WINDOW *window;
line_list_ptr ll;
long n;

   assert( file != NULL );

   window = g_status.window_list;
   while (window != NULL) {
      if (window->file_info == file) {
         window->bin_offset = 0;
         if (window->rline < 1L)
            window->rline = 1L;
         if (window->rline > file->length)
            window->rline = file->length;
         ll = file->line_list;
         n = 1L;
         for (; n < window->rline; n++) {
            window->bin_offset += ll->len;
            ll = ll->next;
         }
         window->ll = ll;
         if (window->rline < (window->cline - (window->top_line+window->ruler-1)))
            window->cline = (int)window->rline + window->top_line+window->ruler-1;
         if (window->cline < window->top_line + window->ruler)
            window->cline = window->top_line + window->ruler;
         if (window->visible)
            show_size( window );
      }
      window = window->next;
   }
}



int  delete_box_block( WINDOW *s_w, line_list_ptr source, int bc, int add )
{
char *s;
int number;

   assert( s_w != NULL );
   assert( source != NULL );
   assert( bc >= 0 );
   assert( bc < MAX_LINE_LENGTH );
   assert( add >= 0 );
   assert( add < MAX_LINE_LENGTH );

   copy_line( source );
   detab_linebuff( );
   number = g_status.line_buff_len - bc;
   s = g_status.line_buff + bc + add;

   assert( number >= 0 );
   assert( number < MAX_LINE_LENGTH );
   assert( bc + add >= 0 );
   assert( bc + add < MAX_LINE_LENGTH );
   assert( add <= g_status.line_buff_len );

   memmove( s - add, s, number );
   g_status.line_buff_len -= add;
   entab_linebuff( );
   return( un_copy_line( source, s_w, TRUE ) );
}



void check_block( void )
{
register file_infos *file;
WINDOW filler;

   file = g_status.marked_file;
   if (file == NULL || file->block_br > file->length)
      unmark_block( &filler );
   else {
      if (file->block_er > file->length)
         file->block_er = file->length;
      find_begblock( file );
      find_endblock( file );
   }
}


void find_begblock( file_infos *file )
{
line_list_ptr ll;
long li;           

   assert( file != NULL );
   assert( file->line_list != NULL );

   ll = file->line_list;
   for (li=1; li<file->block_br && ll->next != NULL; li++)
      ll = ll->next;

   file->block_start = ll;
}



void find_endblock( file_infos *file )
{
line_list_ptr ll; 
long i;           
register file_infos *fp;

   assert( file != NULL );
   assert( file->block_start != NULL );

   fp = file;
   ll = fp->block_start;
   if (ll != NULL) {
      for (i=fp->block_br;  i < fp->block_er && ll->next != NULL; i++)
         ll = ll->next;
      if (ll != NULL)
         fp->block_end = ll;
      else {

      
         fp->block_end = fp->line_list_end->prev;
         fp->block_er = fp->length;
      }
   }
}



int  block_write( WINDOW *window )
{
int prompt_line;
int rc;
char buff[MAX_COLS+2]; 
char line_buff[(MAX_COLS+1)*2]; 
file_infos *file;
int block_type;
int fattr;

  
   entab_linebuff( );
   rc = un_copy_line( window->ll, window, TRUE );
   check_block( );
   if (rc == OK  &&  g_status.marked == TRUE) {
      prompt_line = window->bottom_line;
      file        = g_status.marked_file;

      assert( file != NULL );

      block_type  = file->block_type;

     
      save_screen_line( 0, prompt_line, line_buff );
      *g_status.rw_name = '\0';
      if (get_name( block6, prompt_line, g_status.rw_name,
                    g_display.message_color ) == OK) {
        
         rc = get_fattr( g_status.rw_name, &fattr );
         if (rc == OK) {
           
            set_prompt( block7, prompt_line );
            switch (get_oa( )) {
               case A_OVERWRITE :
                  change_mode( g_status.rw_name, prompt_line );
                 
                  combine_strings( buff, block8, g_status.rw_name, "'" );
                  s_output( buff, prompt_line, 0, g_display.message_color );
                  rc = hw_save( g_status.rw_name, file, file->block_br,
                                file->block_er, block_type );
                  if (rc == ERROR)
                    
                     error( WARNING, prompt_line, block9 );
                  break;
               case A_APPEND :
                
                  combine_strings( buff, block10, g_status.rw_name, "'" );
                  s_output( buff, prompt_line, 0, g_display.message_color );
                  rc = hw_append( g_status.rw_name, file, file->block_br,
                                  file->block_er, block_type );
                  if (rc == ERROR)
                     
                     error( WARNING, prompt_line, block11 );
                  break;
               case AbortCommand :
               default :
                  rc = ERROR;
                  break;
            }
         } else if (rc != ERROR) {
            
            combine_strings( buff, block12, g_status.rw_name, "'" );
            s_output( buff, prompt_line, 0, g_display.message_color );
            if (hw_save( g_status.rw_name, file, file->block_br, file->block_er,
                         block_type ) == ERROR) {
               
               error( WARNING, prompt_line, block9 );
               rc = ERROR;
            }
         }
      }
      restore_screen_line( 0, prompt_line, line_buff );
   } else
      rc = ERROR;
   return( rc );
}



int  block_print( WINDOW *window )
{
char answer[MAX_COLS];          
char line_buff[(MAX_COLS+1)*2]; 
int  col;
int  func;
int  prompt_line;
line_list_ptr block_start;   
file_infos *file;
int  block_type;
char *p;
int  len;
int  bc;
int  ec;
int  last_c;
long lbegin;
long lend;
long l;
int  color;
int  rc;

   color = g_display.message_color;
   entab_linebuff( );
   if (un_copy_line( window->ll, window, TRUE ) == ERROR)
      return( ERROR );
   rc = OK;
   prompt_line = window->bottom_line;
   save_screen_line( 0, prompt_line, line_buff );
   

   assert( strlen( block13 ) < MAX_COLS );

   strcpy( answer, block13 );
   col = strlen( answer );
   s_output( answer, prompt_line, 0, color );
   eol_clear( col, prompt_line, g_display.text_color );
   xygoto( col, prompt_line );
   func = col = 0;
   while (col != 'f' && col != 'F' && col != 'b' && col != 'B' &&
          func != AbortCommand) {
      col = getkey( );
      func = getfunc( col );
      if (col == ESC  ||  func == AbortCommand)
         rc = ERROR;
   }

   if (rc == OK) {
     
      fflush( stdprn );
      if (ceh.flag == ERROR)
         rc = ERROR;
   }

   if (rc != ERROR) {
      file = window->file_info;
      block_type  = NOTMARKED;
      if (col == 'f' || col == 'F') {
         block_start = file->line_list;
         lend =   l  = file->length;
      } else {
         check_block( );
         if (g_status.marked == TRUE) {
            file        = g_status.marked_file;
            block_start = file->block_start;
            block_type  = file->block_type;
            lend =   l  = file->block_er + 1l - file->block_br;
         } else
            rc = ERROR;
      }

      if (rc != ERROR) {
         eol_clear( 0, prompt_line, color );
       
         s_output( block14, prompt_line, 0, color );
         ltoa( l, answer, 10 );
         s_output( answer, prompt_line, 25, color );
         xygoto( 14, prompt_line );
         if (block_type == BOX || block_type == STREAM) {
            bc = file->block_bc;
            ec = file->block_ec;
            last_c = ec + 1 - bc;
         }
         p = g_status.line_buff;
         lbegin = 1;
         for (col=OK; l>0 && col == OK && !g_status.control_break; l--) {
            ltoa( lbegin, answer, 10 );
            s_output( answer, prompt_line, 14, color );
            g_status.copied = FALSE;
            if (block_type == BOX) {
               load_box_buff( p, block_start, bc, ec, ' ' );
               len = last_c;
            } else if (block_type == STREAM && lbegin == 1) {
               len = block_start->len;
               detab_a_line( block_start->line, &len );
               if (bc > len)
                  len = 0;
               else {
                  if (lbegin == lend) {
                     load_box_buff( p, block_start, bc, ec, ' ' );
                     len = last_c;
                  } else {
                     len = len - bc;
                     g_status.copied = TRUE;

                     assert( len >= 0 );
                     assert( len < MAX_LINE_LENGTH );

                     _fmemcpy( p, block_start->line + bc, len );
                  }
               }
            } else if (block_type == STREAM && l == 1L) {
               copy_line( block_start );
               detab_linebuff( );
               len = g_status.line_buff_len;
               if (len > ec + 1)
                  len = ec + 1;
            } else {
               copy_line( block_start );
               len = g_status.line_buff_len;
            }

            assert( len >= 0 );
            assert( len < MAX_LINE_LENGTH );

            *(p+len) = '\r';
            ++len;
            *(p+len) = '\n';
            ++len;
            if (fwrite( p, sizeof( char ), len, stdprn ) < (unsigned)len ||
                ceh.flag == ERROR)
               col = ERROR;
            block_start = block_start->next;
            ++lbegin;
         }
         g_status.copied = FALSE;
         if (ceh.flag != ERROR)
            fflush( stdprn );
         else
            rc = ERROR;
      }
   }
   g_status.copied = FALSE;
   restore_screen_line( 0, prompt_line, line_buff );
   return( rc );
}



int  get_block_fill_char( WINDOW *window, int *c )
{
char answer[MAX_COLS];
char line_buff[(MAX_COLS+1)*2]; 
register int col;
int prompt_line;
int rc;

   rc = OK;
   prompt_line = window->bottom_line;
   save_screen_line( 0, prompt_line, line_buff );
   

   assert( strlen( block15 ) < MAX_COLS );

   strcpy( answer, block15 );
   s_output( answer, prompt_line, 0, g_display.message_color );
   col = strlen( answer );
   eol_clear( col, prompt_line, g_display.text_color );
   xygoto( col, prompt_line );
   col = getkey( );
   if (col >= 256)
      rc = ERROR;
   else
      *c = col;
   restore_screen_line( 0, prompt_line, line_buff );
   return( rc );
}



int  get_block_numbers( WINDOW *window, long *block_num, long *block_inc,
                        int *just )
{
char answer[MAX_COLS];
int prompt_line;
register int rc;
char line_buff[(MAX_COLS+1)*2]; 
register int col;

   prompt_line = window->bottom_line;

   
   answer[0] = '\0';
   
   rc = get_name( block16, prompt_line, answer, g_display.message_color );
   if (answer[0] == '\0')
      rc = ERROR;
   if (rc != ERROR) {
      *block_num = atol( answer );

      
      answer[0] = '1';
      answer[1] = '\0';
      
      rc = get_name( block17, prompt_line, answer, g_display.message_color );
      if (answer[0] == '\0')
         rc = ERROR;
      if (rc != ERROR) {
         *block_inc = atol( answer );

         
         save_screen_line( 0, prompt_line, line_buff );
         

         assert( strlen( block18 ) < MAX_COLS );

         strcpy( answer, block18 );
         s_output( answer, prompt_line, 0, g_display.message_color );
         col = strlen( answer );
         eol_clear( col, prompt_line, g_display.text_color );
         xygoto( col, prompt_line );
         rc = get_lr( );
         if (rc != ERROR) {
            *just = rc;
            rc = OK;
         }
         restore_screen_line( 0, prompt_line, line_buff );
      }
   }

   
   return( rc );
}



int  block_trim_trailing( WINDOW *window )
{
int prompt_line;
int rc;
line_list_ptr p;             
file_infos *file;
WINDOW *sw, s_w;
long er;
int  trailing;               

  
   prompt_line = window->bottom_line;
   entab_linebuff( );
   rc = un_copy_line( window->ll, window, TRUE );
   check_block( );
   if (rc != ERROR && g_status.marked == TRUE) {

      trailing = mode.trailing;
      mode.trailing = TRUE;
      file = g_status.marked_file;
      if (file->block_type != LINE) {
         
         error( WARNING, prompt_line, block21 );
         return( ERROR );
      }

      
      sw = g_status.window_list;
      for (; ptoul( sw->file_info ) != ptoul( file );)
         sw = sw->next;
      if (mode.do_backups == TRUE) {
         file->modified = TRUE;
         rc = backup_file( sw );
      }
      dup_window_info( &s_w, sw );

      
      s_w.visible = FALSE;

      p  = file->block_start;
      er = file->block_er;
      s_w.rline = file->block_br;
      for (; rc == OK && s_w.rline <= er  &&  !g_status.control_break; s_w.rline++) {

         
         copy_line( p );
         rc = un_copy_line( p, &s_w, TRUE );
         p = p->next;
      }

      
      g_status.copied = FALSE;
      file->dirty = GLOBAL;
      mode.trailing = trailing;
      show_avail_mem( );
   }
   return( rc );
}



int  block_email_reply( WINDOW *window )
{
int prompt_line;
int add;
int len;
int rc;
char *source;    
char *dest;      
line_list_ptr p;                     
file_infos *file;
WINDOW *sw, s_w;
long er;

   
   prompt_line = window->bottom_line;
   entab_linebuff( );
   rc = un_copy_line( window->ll, window, TRUE );
   check_block( );
   if (rc != ERROR  &&  g_status.marked == TRUE) {
      file = g_status.marked_file;
      if (file->block_type != LINE) {
        
         error( WARNING, prompt_line, block25 );
         return( ERROR );
      }

      
      sw = g_status.window_list;
      for (; ptoul( sw->file_info ) != ptoul( file );)
         sw = sw->next;
      if (mode.do_backups == TRUE) {
         file->modified = TRUE;
         rc = backup_file( sw );
      }

      
      dup_window_info( &s_w, sw );

      
      s_w.visible = FALSE;
      s_w.rline = file->block_br;
      p  = file->block_start;
      er = file->block_er;

     
      for (; rc == OK  &&  s_w.rline <= er  &&  !g_status.control_break;
                                                             s_w.rline++) {

         
         copy_line( p );
         if (*(p->line) == '>')
            add = 1;
         else
            add = 2;

         
         len = g_status.line_buff_len;
         if (len + add < MAX_LINE_LENGTH) {
            source = g_status.line_buff;
            dest = source + add;

            assert( len >= 0 );
            assert( len < MAX_LINE_LENGTH );
            assert( len + add < MAX_LINE_LENGTH );

            memmove( dest, source, len );
            *source = '>';
            if (add > 1)
              *(source+1) = ' ';
            g_status.line_buff_len = len + add;
            rc = un_copy_line( p, &s_w, TRUE );
         }
         p = p->next;
         g_status.copied = FALSE;
      }

      
      g_status.copied = FALSE;
      file->dirty = GLOBAL;
      show_avail_mem( );
   }
   return( OK );
}



int  block_convert_case( WINDOW *window )
{
int  len;
int  block_type;
line_list_ptr begin;
register file_infos *file;
WINDOW *sw;
long number;
long er;
unsigned int count;
int  bc, ec;
int  block_len;
int  rc;
void (*char_func)( text_ptr, unsigned int );

   
   entab_linebuff( );
   if (un_copy_line( window->ll, window, TRUE ) == ERROR)
      return( ERROR );
   rc = OK;
   check_block( );
   if (g_status.marked == TRUE) {

      
      switch (g_status.command) {
         case BlockUpperCase  :
            char_func = upper_case;
            break;
         case BlockLowerCase  :
            char_func = lower_case;
            break;
         case BlockRot13      :
            char_func = rot13;
            break;
         case BlockFixUUE     :
            char_func = fix_uue;
            break;
         case BlockStripHiBit :
            char_func = strip_hi;
            break;
         default :
            return( ERROR );
      }

      file  = g_status.marked_file;
      file->modified = TRUE;
      if (mode.do_backups == TRUE) {
         sw = g_status.window_list;
         for (; ptoul( sw->file_info ) != ptoul( file );)
            sw = sw->next;
         rc = backup_file( sw );
      }

      if (rc == OK) {
         block_type = file->block_type;
         ec = file->block_ec;

         begin  = file->block_start;

         er = file->block_er;
         block_len = ec + 1 - file->block_bc;
         for (number=file->block_br; number <= er; number++) {
            begin->dirty = TRUE;
            count = len = begin->len;
            bc = 0;
            if (block_type == STREAM) {
               if (number == file->block_br) {
                  bc = file->block_bc;
                  if (len < file->block_bc) {
                     count = 0;
                     bc = len;
                  }
               }
               if (number == file->block_er) {
                  if (ec < len)
                     ec = len;
                  count = ec - bc + 1;
               }
            } else if (block_type == BOX) {
               bc = file->block_bc;
               count =  len >= ec ? block_len : len - bc;
            }
            if (len > bc) {

               assert( count < MAX_LINE_LENGTH );
               assert( bc >= 0 );
               assert( bc < MAX_LINE_LENGTH );

               (*char_func)( begin->line+bc, count );
            }
            begin = begin->next;
         }

        
         g_status.copied = FALSE;
         file->dirty = GLOBAL;
      }
   } else
      rc = ERROR;
   return( rc );
}



void upper_case( text_ptr s, size_t count )
{
   if (s != NULL) {
      for (; count > 0; s++, count-- ) {
         if (*s >= 'a'  &&  *s <= 'z')
            *s ^= 0x20;
      }
   }
}



void lower_case( text_ptr s, size_t count )
{
   if (s != NULL) {
      for (; count > 0; s++, count-- ) {
         if (*s >= 'A'  &&  *s <= 'Z')
            *s |= 0x20;
      }
   }
}



void rot13( text_ptr s, size_t count )
{
register size_t c;

   c = *s;
}



void fix_uue( text_ptr s, size_t  count )
{
   if (s != NULL) {
      for (; count > 0; s++, count-- ) {
         switch (*s) {
            case 0x5d :
               *s = 0x7c;
               break;
            case 0xd5 :
               *s = 0x5b;
               break;
            case 0xe5 :
               *s = 0x5d;
               break;
            default :
               break;
         }
      }
   }
}



void strip_hi( text_ptr s, size_t count )
{
   if (s != NULL) {
      for (; count > 0; s++, count-- ) {
         if (*s >= 0x80)
            *s &= 0x7f;
      }
   }
}
