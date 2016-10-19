/*
 * Copyright(C) 2016 Pedro H. Penna <pedrohenriquepenna@gmail.com>
 * 
 * This file is part of Nanvix.
 * 
 * Nanvix is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 * 
 * Nanvix is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with Nanvix. If not, see <http://www.gnu.org/licenses/>.
 */
/*
FUNCTION
	<<toupper>>---translate characters to uppercase

INDEX
	toupper
INDEX
	_toupper

ANSI_SYNOPSIS
	#include <ctype.h>
	int toupper(int <[c]>);
	int _toupper(int <[c]>);

TRAD_SYNOPSIS
	#include <ctype.h>
	int toupper(<[c]>);
	int _toupper(<[c]>);


DESCRIPTION
<<toupper>> is a macro which converts lowercase characters to uppercase,
leaving all other characters unchanged.  It is only defined when
<[c]> is an integer in the range <<EOF>> to <<255>>.

You can use a compiled subroutine instead of the macro definition by
undefining this macro using `<<#undef toupper>>'.

<<_toupper>> performs the same conversion as <<toupper>>, but should
only be used when <[c]> is known to be a lowercase character (<<a>>--<<z>>).

RETURNS
<<toupper>> returns the uppercase equivalent of <[c]> when it is a
character between <<a>> and <<z>>, and <[c]> otherwise.

<<_toupper>> returns the uppercase equivalent of <[c]> when it is a
character between <<a>> and <<z>>.  If <[c]> is not one of these
characters, the behaviour of <<_toupper>> is undefined.

PORTABILITY
<<toupper>> is ANSI C.  <<_toupper>> is not recommended for portable programs.

No supporting OS subroutines are required.
*/

#include <_ansi.h>
#include <ctype.h>
#if defined (_MB_EXTENDED_CHARSETS_ISO) || defined (_MB_EXTENDED_CHARSETS_WINDOWS)
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <wctype.h>
#include <wchar.h>
#endif

int
_DEFUN(toupper,(c),int c)
{
#if defined (_MB_EXTENDED_CHARSETS_ISO) || defined (_MB_EXTENDED_CHARSETS_WINDOWS)
  if ((unsigned char) c <= 0x7f)
    return islower (c) ? c - 'a' + 'A' : c;
  else if (c != EOF && MB_CUR_MAX == 1 && islower (c))
    {
      char s[MB_LEN_MAX] = { c, '\0' };
      wchar_t wc;
      if (mbtowc (&wc, s, 1) >= 0
	  && wctomb (s, (wchar_t) towupper ((wint_t) wc)) == 1)
	c = (unsigned char) s[0];
    }
  return c;
#else
  return islower (c) ? c - 'a' + 'A' : c;
#endif
}
