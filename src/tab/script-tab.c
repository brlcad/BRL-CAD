/*                    S C R I P T - T A B . C
 * BRL-CAD
 *
 * Copyright (c) 2004-2008 United States Government as represented by
 * the U.S. Army Research Laboratory.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * version 2.1 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this file; see the file named COPYING for more
 * information.
 */
/** @file script-tab.c
 *
 *  Given an RT-style viewpoint animation script,
 *  extract out the essential viewing parameters,
 *  in a form suitable for input back to tabinterp.
 *
 *  This allows scripts generated by one set of tools to be
 *  easily interpolated to higher or lower temporal resolution.
 *
 *  Author -
 *	Michael John Muuss
 *
 *  Source -
 *	The U. S. Army Research Laboratory
 *	Aberdeen Proving Ground, Maryland  21005-5068  USA
 */

#include "common.h"

#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <math.h>
#ifdef HAVE_UNIX_IO
#  include <sys/types.h>
#  include <sys/stat.h>
#endif
#ifdef HAVE_UNISTD_H
#  include <unistd.h>
#endif

#include "machine.h"
#include "vmath.h"
#include "raytrace.h"


point_t	eye_model;		/* model-space location of eye */
mat_t	Viewrotscale;
fastf_t	viewsize;
int	curframe;		/* current frame number */

/*
 *			C M _ S T A R T
 *
 *  Process "start" command in new format input stream
 */
int
cm_start( argc, argv )
int	argc;
char	**argv;
{
	curframe = atoi(argv[1]);
	return 0;
}

int
cm_vsize( argc, argv )
int	argc;
char	**argv;
{
	viewsize = atof( argv[1] );
	return(0);
}

int
cm_eyept( argc, argv )
int	argc;
char	**argv;
{
	register int i;

	for ( i=0; i<3; i++ )
		eye_model[i] = atof( argv[i+1] );
	return(0);
}

int
cm_lookat_pt( argc, argv )
int	argc;
char	**argv;
{
	point_t	pt;
	vect_t	dir;
	int	yflip = 0;

	if ( argc < 4 )
		return(-1);
	pt[X] = atof(argv[1]);
	pt[Y] = atof(argv[2]);
	pt[Z] = atof(argv[3]);
	if ( argc > 4 )
		yflip = atoi(argv[4]);

	/*
	 *  eye_pt must have been specified before here (for now)
	 */
	VSUB2( dir, pt, eye_model );
	VUNITIZE( dir );
	bn_mat_lookat( Viewrotscale, dir, yflip );
	return(0);
}

int
cm_vrot( argc, argv )
int	argc;
char	**argv;
{
	register int i;

	for ( i=0; i<16; i++ )
		Viewrotscale[i] = atof( argv[i+1] );
	return(0);
}

int
cm_orientation( argc, argv )
int	argc;
char	**argv;
{
	register int	i;
	quat_t		quat;

	for ( i=0; i<4; i++ )
		quat[i] = atof( argv[i+1] );
	quat_quat2mat( Viewrotscale, quat );
	return(0);
}

/*
 *			C M _ E N D
 *
 *  The output occurs here.
 *
 *  framenumber, viewsize, eye x y z, orientation x y z w
 */
int
cm_end( argc, argv )
int	argc;
char	**argv;
{
	quat_t	orient;

	/* If no matrix or az/el specified yet, use params from cmd line */
	if ( Viewrotscale[15] <= 0.0 )
		bu_exit(EXIT_FAILURE, "cm_end:  matrix not specified\n");

	quat_mat2quat( orient, Viewrotscale );

	/* Output information about this frame */
	printf("%d %.15e %.15e %.15e %.15e %.15e %.15e %.15e %.15e\n",
		curframe,
		viewsize,
		V3ARGS(eye_model),
		V4ARGS(orient) );

	return(0);
}

int
cm_tree( argc, argv )
int		argc;
const char	**argv;
{
	/* No-op */
	return(0);
}

int
cm_multiview( argc, argv )
int	argc;
char	**argv;
{
	bu_exit(EXIT_FAILURE, "cm_multiview: not supported\n");
	return( 0 );	/* for the compilers */
}

/*
 *			C M _ A N I M
 *
 *  Experimental animation code
 *
 *  Usage:  anim <path> <type> args
 */
int
cm_anim( argc, argv )
int	argc;
char	**argv;
{
	/* No-op */
	return(0);
}

/*
 *			C M _ C L E A N
 *
 *  Clean out results of last rt_prep(), and start anew.
 */
int
cm_clean( argc, argv )
int	argc;
char	**argv;
{
	/* No-op */
	return(0);
}

/*
 *			C M _ S E T
 *
 *  Allow variable values to be set or examined.
 */
int
cm_set( argc, argv )
int	argc;
char	**argv;
{
	/* No-op */
	return(0);
}

/*
 *			C M _ A E
 */
int
cm_ae( argc, argv )
int	argc;
char	**argv;
{
	bu_exit(EXIT_FAILURE, "cm_ae: Unable to compute model min/max RPP\n");
	return(0);
}

/*
 *			C M _ O P T
 */
int
cm_opt( argc, argv )
int	argc;
char	**argv;
{
	/* No-op */
	return(0);
}

/*
 *  Command table for RT control script language
 *  Copied verbatim from ../rt/do.c
 */

struct command_tab rt_cmdtab[] = {
	{"start", "frame number", "start a new frame",
		cm_start,	2, 2},
	{"viewsize", "size in mm", "set view size",
		cm_vsize,	2, 2},
	{"eye_pt", "xyz of eye", "set eye point",
		cm_eyept,	4, 4},
	{"lookat_pt", "x y z [yflip]", "set eye look direction, in X-Y plane",
		cm_lookat_pt,	4, 5},
	{"viewrot", "4x4 matrix", "set view direction from matrix",
		cm_vrot,	17, 17},
	{"orientation", "quaturnion", "set view direction from quaturnion",
		cm_orientation,	5, 5},
	{"end", 	"", "end of frame setup, begin raytrace",
		cm_end,		1, 1},
	{"multiview", "", "produce stock set of views",
		cm_multiview,	1, 1},
	{"anim", 	"path type args", "specify articulation animation",
		cm_anim,	4, 999},
	{"tree", 	"treetop(s)", "specify alternate list of tree tops",
		cm_tree,	1, 999},
	{"clean", "", "clean articulation from previous frame",
		cm_clean,	1, 1},
	{"set", 	"", "show or set parameters",
		cm_set,		1, 999},
	{"ae", "azim elev", "specify view as azim and elev, in degrees",
		cm_ae,		3, 3},
	{"opt", "-flags", "set flags, like on command line",
		cm_opt,		2, 999},
	{(char *)0, (char *)0, (char *)0,
		0,		0, 0}	/* END */
};

/*
 *			M A I N
 */
int
main( argc, argv )
int	argc;
char	**argv;
{
	register char	*buf;
	register int	ret;

	if ( argc != 1 || isatty(fileno(stdin)) )  {
		fprintf(stderr, "Usage: script-tab < script > table\n");
		return 1;
	}

	/*
	 * New way - command driven.
	 * Process sequence of input commands.
	 * All the work happens in the functions
	 * called by rt_do_cmd().
	 */
	while ( (buf = rt_read_cmd( stdin )) != (char *)0 )  {
#if		0
		fprintf(stderr, "cmd: %s\n", buf );
#endif
		ret = rt_do_cmd( NULL, buf, rt_cmdtab );
		if ( ret < 0 )  {
			bu_log("Command failure on '%s'\n", buf);
			bu_free( buf, "cmd buf" );
			break;
		}
		bu_free( buf, "cmd buf" );
	}
	return 0;
}

/*
 * Local Variables:
 * mode: C
 * tab-width: 8
 * indent-tabs-mode: t
 * c-file-style: "stroustrup"
 * End:
 * ex: shiftwidth=4 tabstop=8
 */
