/*	$Id: registers.c,v 1.1.1.1 2003/09/19 17:05:53 gregs Exp $	*/

/*
 * Copyright (c) 1997-2001 Sandro Sigala.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "config.h"

#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#include "zile.h"
#include "extern.h"

#define NUM_REGISTERS	256

static struct {
	char	*text;
	size_t	size;
} regs[NUM_REGISTERS];

DEFUN("copy-to-register", copy_to_register)
/*+
Copy region into the user specified register.
+*/
{
	struct region r;
	char *p;
	int reg;

	minibuf_write("Copy to register: ");
	if ((reg = cur_tp->getkey()) == KBD_CANCEL)
		return cancel();
	minibuf_clear();
	reg %= NUM_REGISTERS;

	if (warn_if_no_mark())
		return FALSE;

	calculate_region(&r);

	p = copy_text_block(r.startn, r.starto, r.size);
	if (regs[reg].text != NULL)
		free(regs[reg].text);
	regs[reg].text = p;
	regs[reg].size = r.size;

	cur_wp->bp->markp = NULL;

	return TRUE;
}

DEFUN("insert-register", insert_register)
/*+
Insert contents of the user specified register.
Puts point before and mark after the inserted text.
+*/
{
	int reg;

	if (warn_if_readonly_buffer())
		return FALSE;

	minibuf_write("Insert register: ");
	if ((reg = cur_tp->getkey()) == KBD_CANCEL)
		return cancel();
	minibuf_clear();
	reg %= NUM_REGISTERS;

	if (regs[reg].text == NULL) {
		minibuf_error("Register does not contain text");
		return FALSE;
	}

	undo_save(UNDO_REMOVE_BLOCK, cur_wp->pointn, cur_wp->pointo,
		  regs[reg].size, 0);
	undo_nosave = TRUE;
	insert_nstring(regs[reg].text, regs[reg].size);
	undo_nosave = FALSE;

	set_mark_command();

	return TRUE;
}

static void write_registers_list(va_list ap)
{
	unsigned int i, count;

	bprintf("%-8s %8s\n", "Register", "Size");
	bprintf("%-8s %8s\n", "--------", "----");
	for (i = count = 0; i < NUM_REGISTERS; ++i)
		if (regs[i].text != NULL) {
			char buf[24];
			++count;
			if (isprint(i))
				sprintf(buf, "`%c'", i);
			else
				sprintf(buf, "`\\%o'", i);
			bprintf("%-8s %8d\n", buf, regs[i].size);
		}
	if (!count)
		bprintf("No register defined\n");
}

DEFUN("list-registers", list_registers)
/*+
List defined registers.
+*/
{
	write_temp_buffer("*Registers List*", write_registers_list);
	return TRUE;
}

void free_registers(void)
{
	int i;
	for (i = 0; i < NUM_REGISTERS; ++i)
		if (regs[i].text != NULL)
			free(regs[i].text);
}
