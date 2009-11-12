/* $Id: dbg_break.h,v 1.3 2005/12/10 02:50:32 rockyb Exp $
Copyright (C) 2005, 2008 R. Bernstein <rocky@gnu.org>
This file is part of GNU Make (remake variant).

GNU Make is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2, or (at your option)
any later version.

GNU Make is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with GNU Make; see the file COPYING.  If not, write to
the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
Boston, MA 02111-1307, USA.  */

/** \file dbg_break.h
 *
 *  \brief debugger command beakpoint routines.
 */

#ifndef DBG_BREAK_H
#define DBG_BREAK_H

#include "types.h"

/*! Opaque type definition for an item in the breakpoint list. */
typedef struct breakpoint_node breakpoint_node_t;

/** Pointers to top of current breakpoint list. */
extern breakpoint_node_t *p_breakpoint_top;

/** Pointers to bottom of current breakpoint list. */
extern breakpoint_node_t *p_breakpoint_bottom;

/** The largest breakpoint number previously given. When a new
    breakpoint is set it will be i_breakpoints+1. */
extern unsigned int i_breakpoints;

/*! Add "p_target" to the list of breakpoints. Return true if 
    there were no errors
*/
extern bool add_breakpoint (file_t *p_target);

/*! Remove breakpoint i from the list of breakpoints. Return true if 
    there were no errors
*/
extern bool remove_breakpoint (unsigned int i);

/*! List breakpoints.*/
extern void list_breakpoints (void);

#endif /* DBG_BREAK_H */
