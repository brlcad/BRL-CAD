/*                         P R E V I E W . C
 * BRL-CAD
 *
 * Copyright (c) 2008-2009 United States Government as represented by
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
/** @file preview.c
 *
 * The preview command.
 *
 */

#include "common.h"

#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <time.h>
#include "bio.h"

#include "cmd.h"

#include "./ged_private.h"


static struct bn_vlblock *preview_vbp;
static double preview_delay;
static int preview_mode;
static int preview_desiredframe;
static int preview_finalframe;
static int preview_currentframe;
static int preview_tree_walk_needed;

static char rt_cmd_storage[MAXARGS*9];


int
ged_cm_anim(int argc, char **argv)
{

    if (ged_current_gedp->ged_wdbp->dbip == DBI_NULL)
	return 0;

    if ( db_parse_anim( ged_current_gedp->ged_wdbp->dbip, argc, (const char **)argv ) < 0 )  {
	bu_vls_printf(&ged_current_gedp->ged_result_str, "cm_anim:  %s %s failed\n", argv[1], argv[2]);
	return -1;		/* BAD */
    }

    preview_tree_walk_needed = 1;

    return 0;
}

int
ged_cm_clean(int argc, char **argv)
{
    if (ged_current_gedp->ged_wdbp->dbip == DBI_NULL)
	return 0;

    /*f_zap( (ClientData)NULL, interp, 0, (char **)0 );*/

    /* Free animation structures */
    db_free_anim(ged_current_gedp->ged_wdbp->dbip);

    preview_tree_walk_needed = 1;
    return 0;
}

int
ged_cm_end(int argc, char **argv)
{
    vect_t	xlate;
    vect_t	new_cent;
    vect_t	xv, yv;			/* view x, y */
    vect_t	xm, ym;			/* model x, y */
    struct bu_list		*vhead = &preview_vbp->head[0];

    /* Only display the frames the user is interested in */
    if ( preview_currentframe < preview_desiredframe )  return 0;
    if ( preview_finalframe && preview_currentframe > preview_finalframe )  return 0;

    /* Record eye path as a polyline.  Move, then draws */
    if ( BU_LIST_IS_EMPTY( vhead ) )  {
	RT_ADD_VLIST( vhead, ged_eye_model, BN_VLIST_LINE_MOVE );
    } else {
	RT_ADD_VLIST( vhead, ged_eye_model, BN_VLIST_LINE_DRAW );
    }

    /* First step:  put eye at view center (view 0, 0, 0) */
    MAT_COPY(ged_current_gedp->ged_gvp->gv_rotation, ged_viewrot);
    MAT_DELTAS_VEC_NEG(ged_current_gedp->ged_gvp->gv_center, ged_eye_model);
    ged_view_update(ged_current_gedp->ged_gvp);

    /*
     * Compute camera orientation notch to right (+X) and up (+Y)
     * Done here, with eye in center of view.
     */
    VSET(xv, 0.05, 0.0, 0.0);
    VSET(yv, 0.0, 0.05, 0.0);
    MAT4X3PNT(xm, ged_current_gedp->ged_gvp->gv_view2model, xv);
    MAT4X3PNT(ym, ged_current_gedp->ged_gvp->gv_view2model, yv);
    RT_ADD_VLIST(vhead, xm, BN_VLIST_LINE_DRAW);
    RT_ADD_VLIST(vhead, ged_eye_model, BN_VLIST_LINE_MOVE);
    RT_ADD_VLIST(vhead, ym, BN_VLIST_LINE_DRAW);
    RT_ADD_VLIST(vhead, ged_eye_model, BN_VLIST_LINE_MOVE);

    /*  Second step:  put eye at view 0, 0, 1.
     *  For eye to be at 0, 0, 1, the old 0, 0, -1 needs to become 0, 0, 0.
     */
    VSET(xlate, 0.0, 0.0, -1.0);	/* correction factor */
    MAT4X3PNT(new_cent, ged_current_gedp->ged_gvp->gv_view2model, xlate);
    MAT_DELTAS_VEC_NEG(ged_current_gedp->ged_gvp->gv_center, new_cent);
    ged_view_update(ged_current_gedp->ged_gvp);

    /* If new treewalk is needed, get new objects into view. */
    if ( preview_tree_walk_needed )  {
	const char *av[2];

	av[0] = "zap";
	av[1] = NULL;

	(void)ged_zap(ged_current_gedp, 1, av );
	ged_eraseobjpath(ged_current_gedp, ged_current_gedp->ged_gdp->gd_rt_cmd_len, (const char **)ged_current_gedp->ged_gdp->gd_rt_cmd, LOOKUP_QUIET, 0);
	ged_drawtrees(ged_current_gedp, ged_current_gedp->ged_gdp->gd_rt_cmd_len, (const char **)ged_current_gedp->ged_gdp->gd_rt_cmd, preview_mode, (struct ged_client_data *)0);
	ged_color_soltab((struct solid *)&ged_current_gedp->ged_gdp->gd_headSolid);
    }

    if (ged_current_gedp->ged_refresh_handler != GED_REFRESH_CALLBACK_PTR_NULL)
	(*ged_current_gedp->ged_refresh_handler)(ged_current_gedp->ged_refresh_clientdata);

    if ( preview_delay > 0 )  {
	struct timeval tv;
	fd_set readfds;

	FD_ZERO(&readfds);
	FD_SET(fileno(stdin), &readfds);
	tv.tv_sec = (long)preview_delay;
	tv.tv_usec = (long)((preview_delay - tv.tv_sec) * 1000000);
	select( fileno(stdin)+1, &readfds, (fd_set *)0, (fd_set *)0, &tv );
    }

    return 0;
}

int
ged_cm_multiview(int argc, char **argv)
{
    return -1;
}

int
ged_cm_start(int argc, char **argv)
{
    if (argc < 2)
	return -1;
    preview_currentframe = atoi(argv[1]);
    preview_tree_walk_needed = 0;

    return 0;
}

int
ged_cm_tree(int argc, char **argv)
{
    register int	i = 1;
    char *cp = rt_cmd_storage;

    for ( i = 1;  i < argc && i < MAXARGS; i++ )  {
	bu_strlcpy(cp, argv[i], MAXARGS*9);
	ged_current_gedp->ged_gdp->gd_rt_cmd[i] = cp;
	cp += strlen(cp) + 1;
    }
    ged_current_gedp->ged_gdp->gd_rt_cmd[i] = (char *)0;
    ged_current_gedp->ged_gdp->gd_rt_cmd_len = i;

    preview_tree_walk_needed = 1;

    return 0;
}

void
ged_setup_rt(struct ged *gedp, register char **vp, int printcmd)
{
    ged_current_gedp->ged_gdp->gd_rt_cmd_len = vp - ged_current_gedp->ged_gdp->gd_rt_cmd;
    ged_current_gedp->ged_gdp->gd_rt_cmd_len += ged_build_tops(gedp, 
				     vp, &ged_current_gedp->ged_gdp->gd_rt_cmd[MAXARGS]);

    if (printcmd) {
	/* Print out the command we are about to run */
	vp = &ged_current_gedp->ged_gdp->gd_rt_cmd[0];
	while (*vp)
	    bu_vls_printf(&gedp->ged_result_str, "%s ", *vp++);

	bu_vls_printf(&gedp->ged_result_str, "\n");
    }
}


struct command_tab ged_preview_cmdtab[] = {
    {"start", "frame number", "start a new frame",
     ged_cm_start,	2, 2},
    {"viewsize", "size in mm", "set view size",
     ged_cm_vsize,	2, 2},
    {"eye_pt", "xyz of eye", "set eye point",
     ged_cm_eyept,	4, 4},
    {"lookat_pt", "x y z [yflip]", "set eye look direction, in X-Y plane",
     ged_cm_lookat_pt,	4, 5},
    {"orientation", "quaturnion", "set view direction from quaturnion",
     ged_cm_orientation,	5, 5},
    {"viewrot", "4x4 matrix", "set view direction from matrix",
     ged_cm_vrot,	17, 17},
    {"end", 	"", "end of frame setup, begin raytrace",
     ged_cm_end,		1, 1},
    {"multiview", "", "produce stock set of views",
     ged_cm_multiview,	1, 1},
    {"anim", 	"path type args", "specify articulation animation",
     ged_cm_anim,	4, 999},
    {"tree", 	"treetop(s)", "specify alternate list of tree tops",
     ged_cm_tree,	1, 999},
    {"clean", "", "clean articulation from previous frame",
     ged_cm_clean,	1, 1},
    {"set", 	"", "show or set parameters",
     ged_cm_set,		1, 999},
    {"ae", "azim elev", "specify view as azim and elev, in degrees",
     ged_cm_null,		3, 3},
    {"opt", "-flags", "set flags, like on command line",
     ged_cm_null,		2, 999},
    {(char *)0, (char *)0, (char *)0,
     0,		0, 0}	/* END */
};

/**
 *  Preview a new style RT animation script.
 *  Note that the RT command parser code is used, rather than the
 *  MGED command parser, because of the differences in format.
 *  The RT parser expects command handlers of the form "ged_cm_xxx()",
 *  and all communications are done via global variables.
 *
 *  For the moment, the only preview mode is the normal one,
 *  moving the eyepoint as directed.
 *  However, as a bonus, the eye path is left behind as a vector plot.
 */
int
ged_preview(struct ged *gedp, int argc, const char *argv[])
{
    FILE *fp;
    char *cmd;
    int	c;
    vect_t temp;
    static const char *usage = "[-v] [-d sec_delay] [-D start frame] [-K last frame] rt_script_file";

    GED_CHECK_DATABASE_OPEN(gedp, BRLCAD_ERROR);
    GED_CHECK_DRAWABLE(gedp, BRLCAD_ERROR);
    GED_CHECK_VIEW(gedp, BRLCAD_ERROR);

    /* initialize result */
    bu_vls_trunc(&gedp->ged_result_str, 0);

    /* must be wanting help */
    if (argc == 1) {
	bu_vls_printf(&gedp->ged_result_str, "Usage: %s %s", argv[0], usage);
	return BRLCAD_HELP;
    }

    if (argc < 2) {
	bu_vls_printf(&gedp->ged_result_str, "Usage: %s %s", argv[0], usage);
	return BRLCAD_ERROR;
    }

    preview_delay = 0;			/* Full speed, by default */
    preview_mode = 1;			/* wireframe drawing */
    preview_desiredframe = 0;
    preview_finalframe = 0;
    ged_current_gedp = gedp;

    /* Parse options */
    bu_optind = 1;			/* re-init bu_getopt() */
    while ((c=bu_getopt(argc, (char * const *)argv, "d:vD:K:")) != EOF) {
	switch (c)  {
	    case 'd':
		preview_delay = atof(bu_optarg);
		break;
	    case 'D':
		preview_desiredframe = atof(bu_optarg);
		break;
	    case 'K':
		preview_finalframe = atof(bu_optarg);
		break;
	    case 'v':
		preview_mode = 3;	/* Like "ev" */
		break;
	    default:
	    {
		bu_vls_printf(&gedp->ged_result_str, "option '%c' unknown\n", c);
		bu_vls_printf(&gedp->ged_result_str, "        -d#     inter-frame delay\n");
		bu_vls_printf(&gedp->ged_result_str, "        -v      polygon rendering (visual)\n");
		bu_vls_printf(&gedp->ged_result_str, "        -D#     desired starting frame\n");
		bu_vls_printf(&gedp->ged_result_str, "        -K#     final frame\n");
	    }

	    break;
	}
    }
    argc -= bu_optind-1;
    argv += bu_optind-1;

    if ((fp = fopen(argv[1], "r")) == NULL) {
	perror(argv[1]);
	return BRLCAD_ERROR;
    }

    /* Build list of top-level objects in view, in ged_current_gedp->ged_gdp->gd_rt_cmd[] */
    ged_setup_rt(gedp, ged_current_gedp->ged_gdp->gd_rt_cmd, 1);

    preview_vbp = rt_vlblock_init();

    bu_vls_printf(&gedp->ged_result_str, "eyepoint at (0, 0, 1) viewspace\n");

    /*
     *  Initialize the view to the current one in MGED
     *  in case a view specification is never given.
     */
    MAT_COPY(ged_viewrot, gedp->ged_gvp->gv_rotation);
    VSET(temp, 0.0, 0.0, 1.0);
    MAT4X3PNT(ged_eye_model, gedp->ged_gvp->gv_view2model, temp);

    while ((cmd = rt_read_cmd(fp)) != NULL) {
	/* Hack to prevent running framedone scripts prematurely */
	if (cmd[0] == '!') {
	    if (preview_currentframe < preview_desiredframe ||
		(preview_finalframe && preview_currentframe > preview_finalframe)) {
		bu_free((genptr_t)cmd, "preview ! cmd");
		continue;
	    }
	}
	if (rt_do_cmd((struct rt_i *)0, cmd, ged_preview_cmdtab) < 0)
	    bu_vls_printf(&gedp->ged_result_str, "command failed: %s\n", cmd);
	bu_free((genptr_t)cmd, "preview cmd");
    }
    fclose(fp);
    fp = NULL;

    ged_cvt_vlblock_to_solids(gedp, preview_vbp, "EYE_PATH", 0);
    if (preview_vbp) {
	rt_vlblock_free(preview_vbp);
	preview_vbp = (struct bn_vlblock *)NULL;
    }
    db_free_anim(gedp->ged_wdbp->dbip);	/* Forget any anim commands */

    return BRLCAD_OK;
}


/*
 * Local Variables:
 * tab-width: 8
 * mode: C
 * indent-tabs-mode: t
 * c-file-style: "stroustrup"
 * End:
 * ex: shiftwidth=4 tabstop=8
 */
