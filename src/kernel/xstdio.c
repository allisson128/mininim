/*
  xstdio.c -- xstdio module;

  Copyright (C) 2015, 2016 Bruno Félix Rezende Ribeiro <oitofelix@gnu.org>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 3, or (at your option)
  any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "mininim.h"

int
xasprintf (char **ptr, const char *template, ...)
{
  va_list ap;
  va_start (ap, template);

  int r = vasprintf (ptr, template, ap);
  if (r < 0)
    error (-1, 0, "%s (%p, %p): cannot create string",
           __func__, ptr, template);
  va_end (ap);

  return r;
}
