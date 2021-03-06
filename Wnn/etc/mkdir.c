/*
 *  $Id: mkdir.c,v 1.3 2005/04/10 15:26:37 aonoto Exp $
 */

/*
 * FreeWnn is a network-extensible Kana-to-Kanji conversion system.
 * This file is part of FreeWnn.
 * 
 * Copyright Kyoto University Research Institute for Mathematical Sciences
 *                 1987, 1988, 1989, 1990, 1991, 1992
 * Copyright OMRON Corporation. 1987, 1988, 1989, 1990, 1991, 1992, 1999
 * Copyright ASTEC, Inc. 1987, 1988, 1989, 1990, 1991, 1992
 * Copyright FreeWnn Project 1999, 2000, 2002
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

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#if !defined(HAVE_MKDIR)

#if STDC_HEADERS
#  include <stdlib.h>
#endif

#include <sys/types.h>
#include <sys/wait.h>

#if !defined(WIFEXITED)
#define WEXITSTATUS(status) (((status) & 0xff00) >> 8)
#endif /* !WIFEXITED */
#if !defined(WIFSIGNALED)
#define WTERMSIG(status) ((status) & 0x7f)
#endif /* !WIFSIGNALED */
#if !defined(WIFSTOPPED)
#define WSTOPSIG(status) WEXITSTATUS(status)
#endif /* !WIFSTOPPED */
#if !defined(WIFEXITED)
#define WIFEXITED(status) (__WTERMSIG(status) == 0)
#endif /* !WIFEXITED */

int
mkdir (path, mode)
     const char *path;
     mode_t mode;
{
  const char *args[3];
  int status;

  if (!path)
    return -1;

  args[0] = "/bin/mkdir";
  args[1] = path;
  args[2] = NULL;

  if (!fork ())
    execv (args[0], args);
  else
    wait (&status);

  return !(WIFEXITED (status));
}
#endif
