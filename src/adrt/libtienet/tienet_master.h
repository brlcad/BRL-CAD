/*                 T I E N E T _ M A S T E R . H
 * BRL-CAD / ADRT
 *
 * Copyright (c) 2002-2008 United States Government as represented by
 * the U.S. Army Research Laboratory.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * version 2.1 as published by the Free Software Foundation.
 *
 * This library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this file; see the file named COPYING for more
 * information.
 */
/** @file tienet_master.h
 *
 *  Comments -
 *      TIE Networking Master Header
 *
 *  Author -
 *      Justin L. Shumaker
 *
 *  Source -
 *      The U. S. Army Research Laboratory
 *      Aberdeen Proving Ground, Maryland  21005-5068  USA
 *
 * $Id$
 */

#ifndef _TIENET_MASTER_H
#define _TIENET_MASTER_H

#include <inttypes.h>
#include "tienet_util.h"

extern void   	tienet_master_init(int port, void fcb_result(tienet_buffer_t *result), char *list, char *exec, int buffer_size, int ver_key, int verbose);
extern void	tienet_master_free();
extern void	tienet_master_push(void *data, int size);
extern void	tienet_master_begin();
extern void	tienet_master_end();
extern void	tienet_master_wait();
extern void	tienet_master_shutdown();
extern void	tienet_master_broadcast(void *mesg, int mesg_len);
extern uint64_t	tienet_master_get_rays_fired();

extern int	tienet_master_active_slaves;
extern int	tienet_master_socket_num;
extern uint64_t	tienet_master_transfer;

#endif

/*
 * Local Variables:
 * mode: C
 * tab-width: 8
 * indent-tabs-mode: t
 * c-file-style: "stroustrup"
 * End:
 * ex: shiftwidth=4 tabstop=8
 */
