/*
 *  $Id: msg.h,v 1.6 2013/09/02 11:01:39 itisango Exp $
 */

/*
 * FreeWnn is a network-extensible Kana-to-Kanji conversion system.
 * This file is part of FreeWnn.
 * 
 * Copyright Kyoto University Research Institute for Mathematical Sciences
 *                 1987, 1988, 1989, 1990, 1991, 1992
 * Copyright OMRON Corporation. 1987, 1988, 1989, 1990, 1991, 1992, 1999
 * Copyright ASTEC, Inc. 1987, 1988, 1989, 1990, 1991, 1992
 * Copyright FreeWnn Project 1999, 2000
 *
 * Maintainer:  FreeWnn Project   <freewnn@tomo.gr.jp>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#ifndef WNN_MSG_H
#define WNN_MSG_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef HAVE_SYS_PARAM_H
#include <sys/param.h>
#endif

struct msg_bd
{
  int msg_id;
  char *msg;
};

struct msg_cat
{
  char lang[32];
  char name[64];
  char nlspath[MAXPATHLEN];
  int msg_cnt;
  struct msg_cat *nextp;
  struct msg_bd *msg_bd;
  /*int encoding; */
};

#define DEF_MSG "Message not found.\n"
/*
#define DEF_LANG "C"
*/
#define DEF_LANG "ja_JP"

#if __STDC__
#define FRWNN_PARAMS(paramlist)	paramlist
#else
#define FRWNN_PARAMS(paramlist)	()
#endif

extern struct msg_cat *msg_open FRWNN_PARAMS((char *, char *, const char *));
extern char *msg_get FRWNN_PARAMS((struct msg_cat *, int id, char *, register char *lang));
extern void msg_close FRWNN_PARAMS((register struct msg_cat *));

#endif  /* WNN_MSG_H */
