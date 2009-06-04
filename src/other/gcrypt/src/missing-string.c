/* missing-string.c - missing string utilities
 * Copyright (C) 1994, 1998, 1999, 2000, 2001, 
 *               2003 Free Software Foundation, Inc.
 *
 * This file is part of Libgcrypt.
 *
 * Libgcrypt is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation; either version 2.1 of
 * the License, or (at your option) any later version.
 *
 * Libgcrypt is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA
 */

#include <config.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "g10lib.h"


#ifndef HAVE_STPCPY
char *
stpcpy(char *a,const char *b)
{
    while( *b )
	*a++ = *b++;
    *a = 0;

    return (char*)a;
}
#endif


#ifndef HAVE_STRCASECMP
int
strcasecmp( const char *a, const char *b )
{
    for( ; *a && *b; a++, b++ ) {
	if( *a != *b && toupper(*a) != toupper(*b) )
	    break;
    }
    return *(const byte*)a - *(const byte*)b;
}
#endif


#ifdef __MINGW32__
/* 
 * Like vsprintf but provides a pointer to malloc'd storage, which
 * must be freed by the caller (gcry_free).  Taken from libiberty as
 * found in gcc-2.95.2 and a little bit modernized.
 * FIXME: Write a new CRT for W32.
 */
int
vasprintf ( char **result, const char *format, va_list args)
{
  const char *p = format;
  /* Add one to make sure that it is never zero, which might cause malloc
     to return NULL.  */
  int total_width = strlen (format) + 1;
  va_list ap;

  /* this is not really portable but works under Windows */
  memcpy ( &ap, &args, sizeof (va_list));

  while (*p != '\0')
    {
      if (*p++ == '%')
	{
	  while (strchr ("-+ #0", *p))
	    ++p;
	  if (*p == '*')
	    {
	      ++p;
	      total_width += abs (va_arg (ap, int));
	    }
	  else
            {
              char *endp;  
              total_width += strtoul (p, &endp, 10);
              p = endp;
            }
	  if (*p == '.')
	    {
	      ++p;
	      if (*p == '*')
		{
		  ++p;
		  total_width += abs (va_arg (ap, int));
		}
	      else
                {
                  char *endp;
                  total_width += strtoul (p, &endp, 10);
                  p = endp;
                }
	    }
	  while (strchr ("hlL", *p))
	    ++p;
	  /* Should be big enough for any format specifier except %s
             and floats.  */
	  total_width += 30;
	  switch (*p)
	    {
	    case 'd':
	    case 'i':
	    case 'o':
	    case 'u':
	    case 'x':
	    case 'X':
	    case 'c':
	      (void) va_arg (ap, int);
	      break;
	    case 'f':
	    case 'e':
	    case 'E':
	    case 'g':
	    case 'G':
	      (void) va_arg (ap, double);
	      /* Since an ieee double can have an exponent of 307, we'll
		 make the buffer wide enough to cover the gross case. */
	      total_width += 307;
	    
	    case 's':
	      total_width += strlen (va_arg (ap, char *));
	      break;
	    case 'p':
	    case 'n':
	      (void) va_arg (ap, char *);
	      break;
	    }
	}
    }
  *result = gcry_malloc (total_width);
  if (*result != NULL)
    return vsprintf (*result, format, args);
  else
    return 0;
}

#endif /*__MINGW32__*/

