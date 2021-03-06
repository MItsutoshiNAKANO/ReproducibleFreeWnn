/*
 *  $Id: select_ele.c,v 1.6 2013/09/02 11:01:40 itisango Exp $
 */

/*
 * FreeWnn is a network-extensible Kana-to-Kanji conversion system.
 * This file is part of FreeWnn.
 * 
 * Copyright Kyoto University Research Institute for Mathematical Sciences
 *                 1987, 1988, 1989, 1990, 1991, 1992
 * Copyright OMRON Corporation. 1987, 1988, 1989, 1990, 1991, 1992, 1999
 * Copyright ASTEC, Inc. 1987, 1988, 1989, 1990, 1991, 1992
 * Copyright FreeWnn Project 1999, 2000, 2002, 2012
 *
 * Maintainer:  FreeWnn Project
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

/* 一つの要素をユーザーに取り出してもらうためのルーチン */
#if HAVE_CONFIG_H
#  include <config.h>
#endif

#define DD_MAX 512

#include <stdio.h>
#if STDC_HEADERS
#  include <string.h>
#elif HAVE_STRINGS_H
#  include <strings.h>
#endif /* STDC_HEADERS */

#include "commonhd.h"
#include "sdefine.h"
#include "sheader.h"
#include "buffer.h"
#include "jslib.h"
#include "jllib.h"

#define printh(mm)\
  if(state == 0){\
    printf("%c.",((mm - dd[cc]) > 9)? mm - dd[cc] - 10 + 'A':\
                                      mm - dd[cc] + '0');\
    printf("%s", data[mm]);\
  }else{\
    printf("%c.",((mm > 9)? mm - 10  + 'A':mm + '0'));\
    printf("%s", data[mm]);}


#define kouho_len(x) (strlen(x) + 4)

/*
#define WIDTH (maxlength - disp_mode_length - 6)
*/
#define LEND c_b->vlen + 1
#define WIDTH (LEND - 7)

static char **data;             /* mojiretu data area */
static int dd[512];             /* gamen no sentou to naru youso wo motte iru */
static int dd_size;             /* dd no size */
static char *msg;
static int msg_len;
static int cc;                  /* ima no gamen */
static int mm;                  /* hyouji mojiretu */
static int state;               /* choose whether to number all or not */
                                /* 0: line by line */
                                /* 1: all */
static int kosuu;               /* kosuu of elements */

static void display_all (int, int), display (int, int);
static void reverse (int, int);
static void clear_it (int, int);
static int find_cc (int), set_kouho (void), change_decimal (int), findcp (int, int);

int
select_one_element (c, kosuu1, init, msg1, msg_len1, state1, key_tbl)
     char **c;                  /*表示文字列の配列のポインタ */
     int kosuu1;
     int state1;
     int init;
     char *msg1;
     int msg_len1;
     int (**key_tbl) ();
{
  int oldmm;                    /* hitotu mae no mm */
  int d;
  int c1;
  int ret;

  extern int henkan_off ();

  int not_redrawtmp = not_redraw;
  not_redraw = 1;               /* リドローしない */
  push_cursor ();
  kk_cursor_invisible ();       /* カーソルを消す */

  kosuu = kosuu1;
  data = c;
  msg = msg1;
  msg_len = msg_len1;
  state = state1;

  dd_size = set_kouho ();  /* dd_size = set_kouho (kosuu);  */
  cc = find_cc (init);

  mm = init;
  display_all (mm, cc);
  for (;;)
    {
      c1 = keyin ();
      if (!ONEBYTE_CHAR (c1))
        continue;
      oldmm = mm;
      d = change_decimal (c1);
      if (state == 0)
        {
          if (d >= 0 && d < dd[cc + 1] - dd[cc])
            {
              not_redraw = not_redrawtmp;
              pop_cursor ();
              return (d + dd[cc]);
            }
        }
      else
        {
          if (d >= 0 && d < kosuu)
            {
              not_redraw = not_redrawtmp;
              pop_cursor ();
              return (d);
            }
        }
      if ((c1 < 256) && (key_tbl[c1] != NULL))
        {

          if (henkan_off == key_tbl[c1])
            {                   /* added by T.S 10 Jan. '88 */
              henkan_off ();    /* What a mess!!  */
              throw_c (LEND);   /* not beautiful!! */
              flush ();
              continue;
            }
          else if ((ret = (*key_tbl[c1]) ()) == 1)
            {
              not_redraw = not_redrawtmp;
              pop_cursor ();
              return (mm);
            }
          else if (ret == -1)
            {
              not_redraw = not_redrawtmp;
              pop_cursor ();
              return (-1);
            }
        }
      else
        {
          ring_bell ();
        }
      if (mm < dd[cc] || mm >= dd[cc + 1])
        {
          cc = find_cc (mm);
          display_all (mm, cc);
        }
      else
        {
          if (mm != oldmm)
            {
              clear_it (oldmm, cc);
              reverse (mm, cc);
            }
        }
    }
}

static int
find_mm (cc_num, banme)
     int cc_num, banme;
{
  if (dd[cc_num + 1] - dd[cc_num] > banme)
    {
      return (dd[cc_num] + banme);
    }
  else
    {
      return (dd[cc_num + 1] - 1);
    }
}

static int
find_cc (mm)
     int mm;
{
  int k;
  for (k = dd_size - 1; k >= 0; k--)
    {
      if (dd[k] <= mm)
        {
          return (k);
        }
    }
  return (0);
}

static void
display_all (mm, cc)
     int mm, cc;
{
  int cp;
  int k;

  throw_c (0);
  clr_line ();
  printf (msg);
  for (k = dd[cc]; k < dd[cc + 1]; k++)
    {
      cp = findcp (k, cc);
      throw_c (cp);
      if (k == mm)
        h_r_on ();
      printh (k);
      if (k == mm)
        h_r_off ();
    }
  throw_c (WIDTH);
  printf ("%3d/%-3d", dd[cc + 1], dd[dd_size]);
  flush ();
}

static int
findcp (mm, cc)
     int mm, cc;
{
  int k;
  int cp = msg_len;

  for (k = dd[cc]; k < mm; k++)
    {
      cp += kouho_len (data[k]);
    }
  return (cp);
}

static void
clear_it (mm, cc)
     int mm, cc;
{
  int cp;

  cp = findcp (mm, cc);
  throw_c (cp);
  printh (mm);
  flush ();
}

static void
reverse (mm, cc)
     int mm, cc;
{
  int cp;

  cp = findcp (mm, cc);
  throw_c (cp);
  h_r_on ();
  printh (mm);
  h_r_off ();
  throw_c (LEND);
  flush ();
}
static int
set_kouho ()
{
  int cp = msg_len;
  int k;
  int pt = 0;

  dd[pt++] = 0;
  for (k = 0; k < kosuu; k++)
    {
      if ((cp >= (WIDTH - kouho_len (data[k])) && (dd[pt - 1] != k)) || (k - dd[pt - 1]) >= min (max_ichiran_kosu, (10 + 26)))
        {
          /* limit of selection is alphabet26 + number10 */
          dd[pt++] = k;
          cp = msg_len;
        }
      cp += kouho_len (data[k]);
    }
  dd[pt] = k;
  return (pt);
}

static int
change_decimal (c1)
     int c1;
{
  if (c1 >= '0' && c1 <= '9')
    {
      return (c1 - '0');
    }
  if (c1 >= 'A' && c1 <= 'Z')
    {
      return (c1 - 'A' + 10);
    }
  if (c1 >= 'a' && c1 <= 'z')
    {
      return (c1 - 'a' + 10);
    }
  return (-1);
}

int
forward_select ()
{
  if (mm < kosuu - 1)
    {
      mm++;
    }
  else
    {
      mm = 0;
    }
  return (0);
}

int
backward_select ()
{
  if (mm > 0)
    {
      mm--;
    }
  else
    {
      mm = kosuu - 1;
    }
  return (0);
}

int
lineend_select ()
{
  mm = dd[cc + 1] - 1;
  return (0);
}

int
linestart_select ()
{
  mm = dd[cc];
  return (0);
}

int
select_select ()
{
  return (1);                   /* return mm from upper function */
}

int
quit_select ()
{
  return (-1);
}

int
previous_select ()
{
  if (cc > 0)
    {
      mm = find_mm (cc - 1, mm - dd[cc]);
    }
  else
    {
      mm = find_mm (dd_size - 1, mm - dd[cc]);
    }
  return (0);
}

int
next_select ()
{
  if (cc < dd_size - 1)
    {
      mm = find_mm (cc + 1, mm - dd[cc]);
    }
  else
    {
      mm = find_mm (0, mm - dd[cc]);
    }
  return (0);
}

int
redraw_select ()
{
  display_all (mm, cc);
  return (0);
}




/* these are function for select element with displaying all one element
   in a line */
static int lc;                  /* line count */
static int lc_offset = 0;       /* title count */
static int sc;                  /* screen count */

void
set_lc_offset (line)
     int line;
{
  lc_offset = line;
}

int
select_line_element (c, kosuu1, init, msg1, msg_len1, state1, key_tbl)
     char **c;                  /*表示文字列の配列のポインタ */
     int kosuu1;
     int init;
     char *msg1;
     int msg_len1;
     int state1;
     int (**key_tbl) ();
{
  int c1;
  int ret;

  int not_redrawtmp = not_redraw;
  not_redraw = 1;               /* リドローしない */
  push_cursor ();

  data = c;
  msg = msg1;
  msg_len = msg_len1;
  state = state1;
  kosuu = kosuu1;

  lc = init;
  sc = 0;
  for (;;)
    {
      display (lc, sc);
      c1 = keyin ();
      if (!ONEBYTE_CHAR (c1))
        continue;
/*
    d = change_decimal(c1);
    if(d >= 0 && d < kosuu){
      not_redraw = not_redrawtmp;
      pop_cursor();
      return(d);
    }
*/
      if ((c1 < 256) && (key_tbl[c1] != NULL))
        {
          if ((ret = (*key_tbl[c1]) ()) == 1)
            {
              not_redraw = not_redrawtmp;
              pop_cursor ();
              return (lc);
            }
          else if (ret == -1)
            {
              not_redraw = not_redrawtmp;
              pop_cursor ();
              return (-1);
            }
        }
      else
        {
          ring_bell ();
        }
    }
}

int
cnt_of_screen (lc)
     int lc;
{
  return (strlen (data[lc]) / st_colum (1) + 1);
}


static void
display (lc, sc)
     int lc, sc;
{
  int eof = 0;
  char *c;
  char *start = 0;
  char *end;
  char buf[1024];
  throw_c (0);
  clr_line ();
  if (sc >= cnt_of_screen (lc))
    {
      sc = cnt_of_screen (lc) - 1;
    }
  for (c = data[lc]; *c;)
    {
      if (start == 0)
        {
          if (c >= st_colum (sc) + data[lc])
            start = c;
        }
      else
        {
          if (c > st_colum (sc) + (c_b->vlen - 2) + data[lc])
            {
              eof = 1;
              break;
            }
        }
      if (*c & 0x80)
        {
          c += 2;
        }
      else
        {
          c++;
        }
    }
  end = c;
  if (start == 0)
    {
/*
    printf("Error sc = %d",sc);
    flush();
*/
      return;
    }
  strncpy (buf, start, end - start);
  buf[end - start] = 0;
  if (sc != 0)
    {
      printf ("%c", '$');
    }
  else
    {
      printf ("%c", ' ');
    }
  printf ("%s", buf);
  if (eof != 0)
    {
      printf ("%c", '$');
    }
  else
    {
      printf ("%c", ' ');
    }

  flush ();
}

int
st_colum (x)
     int x;
{
  return (((c_b->vlen - 2) * 3 / 4) * x);
}


int
next_select_line ()
{
  if (lc < kosuu - 1)
    {
      lc++;
    }
  else
    {
      lc = 0;
    }
  return (0);
}

int
previous_select_line ()
{
  if (lc > 0)
    {
      lc--;
    }
  else
    {
      lc = kosuu - 1;
    }
  return (0);
}

int
select_select_line ()
{
  return (1);
}

int
linestart_select_line ()
{
  sc = 0;
  return (0);
}

int
lineend_select_line ()
{
  sc = cnt_of_screen (lc) - 1;
  return (0);
}

int
quit_select_line ()
{
  return (-1);
}

int
forward_select_line ()
{
  if (sc < cnt_of_screen (lc) - 1)
    {
      sc++;
    }
  return (0);
}

int
backward_select_line ()
{
  if (sc > 0)
    {
      sc--;
    }
  return (0);
}

int
redraw_select_line ()
{
  display (lc, sc);
  return (0);
}

/*
 *
 *  Functions Called during select element (dictionaries).
 *
 */

/* jishodel for kensaku */
int
kdicdel ()
{
  char buf[512];

  /*
     sprintf(buf, "辞書 %s を削除します。(Y/N)", dicinfo[lc-lc_offset].fname);
   */
  sprintf (buf, "%s%s%s", MSG_GET (31), dicinfo[lc - lc_offset].fname, MSG_GET (24));
  if (yes_or_no (buf) == 1)
    {
      if (jl_dic_delete (bun_data_, dicinfo[lc - lc_offset].dic_no) == -1)
        {
          errorkeyin ();
          return (0);
        }
      return (1);
    }
  return (0);
}


int
kdicuse ()
{
  char buf[512];

  if (dicinfo[lc - lc_offset].enablef)
    {
      /*
         sprintf(buf, "辞書 %s の使用を中止します。(Y/N)", dicinfo[lc-lc_offset].fname);
       */
      sprintf (buf, "%s%s%s", MSG_GET (31), dicinfo[lc - lc_offset].fname, MSG_GET (26));
    }
  else
    {
      /*
         sprintf(buf, "辞書 %s の使用を再開します。(Y/N)", dicinfo[lc-lc_offset].fname);
       */
      sprintf (buf, "%s%s%s", MSG_GET (31), dicinfo[lc - lc_offset].fname, MSG_GET (32));
    }
  if (yes_or_no (buf) == 1)
    {
      if (jl_dic_use (bun_data_, dicinfo[lc - lc_offset].dic_no, !dicinfo[lc - lc_offset].enablef) == -1)
        {
          errorkeyin ();
          return (0);
        }
      return (1);
    }
  return (0);
}

/* jishodel for kensaku */
int
kdiccom ()
{
  w_char com[512];
#ifdef nodef
  char st[32];
#endif
  int v = 1;

  com[0] = 0;
#ifdef nodef
  if (dicinfo[lc - lc_offset].hindo == -1)
    {
      v = 1;
    }
  else
    {
      /*
         if(kana_in("1:辞書 2:頻度 どちらにコメントをセットしますか。>" ,UNDER_LINE_MODE  , com , 512) == -1)return(0);
       */
      if (kana_in (MSG_GET (33), UNDER_LINE_MODE, com, 512) == -1)
        return (0);
      if (wchartochar (com, st) || sscanf (st, "%d", &v) <= 0 || (v != 1 && v != 2))
        {
          /*
             print_msg_getc("1 か 2 を指定して下さい。(如何?)");
           */
          print_msg_getc (MSG_GET (34));
          return (0);
        }
    }
#endif
  com[0] = 0;
  /*
     if(kana_in("コメント: " ,UNDER_LINE_MODE  , com , 512) == -1)return(0);
   */
  if (kana_in (MSG_GET (35), UNDER_LINE_MODE, com, 512) == -1)
    return (0);
  if (v == 1)
    {
      if (jl_dic_comment_set (bun_data_, dicinfo[lc - lc_offset].dic_no, com) == -1)
        {
          errorkeyin ();
          return (0);
        }
    }
  else
    {
#ifdef nodef
      if (jl_hindo_comment_set (bun_data_, dicinfo[lc - lc_offset].dic_no, com) == -1)
        {
          errorkeyin ();
          return (0);
        }
#endif
    }
  return (1);
}

/*
 *
 *  Functions Called during select element (dic_entries).
 *
 */

/* Defined in kensaku.c */
extern struct wnn_jdata *word_searched;

int
kworddel ()
{
  char buf[512];
  int type;

  type = dicinfo[find_dic_by_no (word_searched[lc].dic_no)].type;
  if (type != WNN_UD_DICT && type != WNN_REV_DICT && type != BWNN_REV_DICT && type != CWNN_REV_DICT)
    {
      /*
         print_msg_getc("固定形式の辞書のエントリは削除出来ません。(如何?)");
       */
      print_msg_getc (MSG_GET (21));
      return (0);
    }
  if (dicinfo[find_dic_by_no (word_searched[lc].dic_no)].rw == WNN_DIC_RDONLY)
    {
      /*
         print_msg_getc("リードオンリーの辞書のエントリは削除出来ません。(如何?)");
       */
      print_msg_getc (MSG_GET (22));
      return (0);
    }

  /*
     strcpy(buf, "「");
   */
  strcpy (buf, MSG_GET (23));
  sStrcpy (buf + strlen (buf), word_searched[lc].kanji);
  /*
     sprintf(buf + strlen(buf), "」を削除します。(Y/N)");
   */
  sprintf (buf + strlen (buf), "%s", MSG_GET (24));	/* FIXME: check buffer length */
  if (yes_or_no (buf) == 1)
    {
      if (jl_word_delete (bun_data_, word_searched[lc].dic_no, word_searched[lc].serial) == -1)
        {
          errorkeyin ();
          return (0);
        }
      return (1);
    }
  return (0);
}

int
kworduse ()
{
  char buf[512];

  if (dicinfo[find_dic_by_no (word_searched[lc].dic_no)].hindo_rw == WNN_DIC_RDONLY)
    {
      /*
         print_msg_getc("リードオンリーの頻度は変更出来ません。(如何?)");
       */
      print_msg_getc (MSG_GET (25));
      return (0);
    }
  /*
     strcpy(buf, "「");
   */
  strcpy (buf, MSG_GET (23));
  sStrcpy (buf + strlen (buf), word_searched[lc].kanji);
  /* FIXME: check buffer length */
  if (word_searched[lc].hindo == -1 || (word_searched[lc].int_hindo == -1 && !word_searched[lc].int_ima))
    {
      /*
         sprintf(buf + strlen(buf), "」の使用を再開します。(Y/N)");
       */
      sprintf (buf + strlen (buf), "%s", MSG_GET (32));
    }
  else
    {
      /*
         sprintf(buf + strlen(buf), "」の使用を中止します。(Y/N)");
       */
      sprintf (buf + strlen (buf), "%s", MSG_GET (26));
    }
  if (yes_or_no (buf) == 1)
    {
      if (jl_word_use (bun_data_, word_searched[lc].dic_no, word_searched[lc].serial) == -1)
        {
          errorkeyin ();
          return (0);
        }
      return (1);
    }
  return (0);
}

int
kwordcom ()
{
  w_char com[512];
  int type;
  com[0] = 0;

  type = dicinfo[find_dic_by_no (word_searched[lc].dic_no)].type;
  if (type != WNN_UD_DICT && type != WNN_REV_DICT && type != BWNN_REV_DICT && type != CWNN_REV_DICT)
    {
      /*
         print_msg_getc("固定形式辞書のエントリのコメントは変更出来ません。(如何?)");
       */
      print_msg_getc (MSG_GET (36));

    }
  if (dicinfo[find_dic_by_no (word_searched[lc].dic_no)].rw == WNN_DIC_RDONLY)
    {
      /*
         print_msg_getc("リードオンリー辞書のエントリのコメントは変更出来ません。(如何?)");
       */
      print_msg_getc (MSG_GET (37));
      return (0);
    }

  /*
     if(kana_in(" コメント: " ,UNDER_LINE_MODE  , com , 512) == -1)return(0);
   */
  if (kana_in (MSG_GET (35), UNDER_LINE_MODE, com, 512) == -1)
    return (0);
  if (jl_word_comment_set (bun_data_, word_searched[lc].dic_no, word_searched[lc].serial, com) == -1)
    {
      errorkeyin ();
      return (0);
    }
  return (1);

}

static int
call_hindo_set (ima, hindo)
     int ima;
     int hindo;
{
  if (js_hindo_set (bun_data_->env, word_searched[lc].dic_no, word_searched[lc].serial, ima, hindo) == -1)
    {
      errorkeyin ();
      return (0);
    }
  return (1);
}

int
kwordima ()
{
  if (word_searched[lc].ima)
    {
      if (call_hindo_set (WNN_IMA_OFF, WNN_HINDO_NOP))
        return (1);
    }
  else
    {
      if (call_hindo_set (WNN_IMA_ON, WNN_HINDO_NOP))
        return (1);
    }
  return (0);
}

int
kwordhindo ()
{
  int cur_hindo;
  w_char kana_buf[512];
  char st[32];
  int newhindo;
  char message[80];

  if (dicinfo[find_dic_by_no (word_searched[lc].dic_no)].hindo_rw == WNN_DIC_RDONLY)
    {
      print_msg_getc (MSG_GET (25));
      return (0);
    }
  cur_hindo = word_searched[lc].hindo;

  sprintf (message, MSG_GET (46), MSG_GET (99), cur_hindo);
Retry:
  kana_buf[0] = 0;
  if (kana_in (message, UNDER_LINE_MODE, kana_buf, 1024) == -1)
    return (0);
  if (wchartochar (kana_buf, st) || sscanf (st, "%d", &newhindo) <= 0)
    {
      print_msg_getc (MSG_GET (47));
      goto Retry;
    }
  else if (newhindo < 0)
    {
      print_msg_getc (MSG_GET (48));
      goto Retry;
    }
  return (call_hindo_set (WNN_HINDO_NOP, newhindo));
}

/*
  Local Variables:
  eval: (setq kanji-flag t)
  eval: (setq kanji-fileio-code 0)
  eval: (mode-line-kanji-code-update)
  End:
*/
