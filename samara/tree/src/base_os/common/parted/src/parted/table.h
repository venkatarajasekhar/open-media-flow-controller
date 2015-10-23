/*
    parted - a frontend to libparted
    Copyright (C) 2006 Free Software Foundation, Inc.

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA
*/

#define _GNU_SOURCE
#ifdef ENABLE_NLS
#include <wchar.h>
#endif

#include <stdio.h>
#include <stdlib.h>

#include <assert.h>

#include "strlist.h"

#include <config.h>

#ifdef ENABLE_NLS
#       define _GNU_SOURCE
#       include <wchar.h>
#else
#       ifdef wchar_t
#               undef wchar_t
#       endif
#       define wchar_t char
#endif


/* opaque data type */
typedef void Table;

Table* table_new(int ncols);
void table_destroy (Table* t);

/* 
 * you must not free neither 'row' nor 'list'
 *      -- this will be done by table_destroy()
 */
void table_add_row (Table* t, wchar_t** row);
void table_add_row_from_strlist (Table* t, StrList* list);
        
wchar_t* table_render(Table* t);

