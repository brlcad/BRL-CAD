/*                    D M - G E N E R I C . C
 * BRL-CAD
 *
 * Copyright (c) 1999-2014 United States Government as represented by
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
/** @file libdm/dm-generic.c
 *
 * Generic display manager routines.
 *
 */

#include "common.h"
#include "bio.h"

#include <string.h>

#include "tcl.h"

#include "bu.h"
#include "bn.h"
#include "vmath.h"
#include "dm.h"

#include "dm/dm-Null.h"

extern struct dm *txt_open(Tcl_Interp *interp, int argc, const char **argv);

#ifdef DM_OSG
#  if defined(HAVE_TK)
extern struct dm *osg_open();
extern void osg_fogHint();
extern int osg_share_dlist();
#  endif
#endif /* DM_OSG*/

#ifdef DM_QT
extern struct dm *qt_open();
#endif /* DM_QT */

HIDDEN struct dm *
null_open(Tcl_Interp *interp, int argc, const char *argv[])
{
    struct dm *dmp;

    if (argc < 0 || !argv)
	return DM_NULL;

    BU_ALLOC(dmp, struct dm);

    *dmp = dm_null;
    dmp->dm_interp = interp;

    return dmp;
}


struct dm *
dm_open(Tcl_Interp *interp, int type, int argc, const char *argv[])
{
    switch (type) {
	case DM_TYPE_NULL:
	    return null_open(interp, argc, argv);
	case DM_TYPE_TXT:
	    return txt_open(interp, argc, argv);
#ifdef DM_OSG
#  if defined(HAVE_TK)
	case DM_TYPE_OSG:
	    return osg_open(interp, argc, argv);
#  endif
#endif
#ifdef DM_QT
	case DM_TYPE_QT:
	    return qt_open(interp, argc, argv);
#endif
	default:
	    break;
    }

    return DM_NULL;
}

/*
 * Provides a way to (un)share display lists. If dmp2 is
 * NULL, then dmp1 will no longer share its display lists.
 */
int
dm_share_dlist(struct dm *dmp1, struct dm *dmp2)
{
    if (dmp1 == DM_NULL)
	return TCL_ERROR;

    /*
     * Only display managers of the same type and using the
     * same OGL server are allowed to share display lists.
     *
     * XXX - need a better way to check if using the same OGL server.
     */
    if (dmp2 != DM_NULL)
	if (dmp1->dm_type != dmp2->dm_type ||
	    bu_vls_strcmp(&dmp1->dm_dName, &dmp2->dm_dName))
	    return TCL_ERROR;

    switch (dmp1->dm_type) {
#ifdef DM_OSG
#  if defined(HAVE_TK)
	case DM_TYPE_OSG:
	    return osg_share_dlist(dmp1, dmp2);
#  endif
#endif
	default:
	    return TCL_ERROR;
    }
}

fastf_t
dm_Xx2Normal(struct dm *dmp, int x)
{
    return ((x / (fastf_t)dmp->dm_width - 0.5) * 2.0);
}

int
dm_Normal2Xx(struct dm *dmp, fastf_t f)
{
    return (f * 0.5 + 0.5) * dmp->dm_width;
}

fastf_t
dm_Xy2Normal(struct dm *dmp, int y, int use_aspect)
{
    if (use_aspect)
	return ((0.5 - y / (fastf_t)dmp->dm_height) / dmp->dm_aspect * 2.0);
    else
	return ((0.5 - y / (fastf_t)dmp->dm_height) * 2.0);
}

int
dm_Normal2Xy(struct dm *dmp, fastf_t f, int use_aspect)
{
    if (use_aspect)
	return (0.5 - f * 0.5 * dmp->dm_aspect) * dmp->dm_height;
    else
	return (0.5 - f * 0.5) * dmp->dm_height;
}

void
dm_fogHint(struct dm *dmp, int fastfog)
{
    if (!dmp) {
	bu_log("WARNING: NULL display (fastfog => %d)\n", fastfog);
	return;
    }

    switch (dmp->dm_type) {
#ifdef DM_OSG
#  if defined(HAVE_TK)
	case DM_TYPE_OSG:
	    osg_fogHint(dmp, fastfog);
	    return;
#  endif
#endif
	default:
	    return;
    }
}

void
dm_view_update(struct dm_view *gvp)
{
    vect_t work, work1;
    vect_t temp, temp1;

    if (!gvp)
	return;

    bn_mat_mul(gvp->gv_model2view,
	       gvp->gv_rotation,
	       gvp->gv_center);
    gvp->gv_model2view[15] = gvp->gv_scale;
    bn_mat_inv(gvp->gv_view2model, gvp->gv_model2view);

    /* Find current azimuth, elevation, and twist angles */
    VSET(work, 0.0, 0.0, 1.0);       /* view z-direction */
    MAT4X3VEC(temp, gvp->gv_view2model, work);
    VSET(work1, 1.0, 0.0, 0.0);      /* view x-direction */
    MAT4X3VEC(temp1, gvp->gv_view2model, work1);

    /* calculate angles using accuracy of 0.005, since display
     * shows 2 digits right of decimal point */
    bn_aet_vec(&gvp->gv_aet[0],
	       &gvp->gv_aet[1],
	       &gvp->gv_aet[2],
	       temp, temp1, (fastf_t)0.005);

    /* Force azimuth range to be [0, 360] */
    if ((NEAR_EQUAL(gvp->gv_aet[1], 90.0, (fastf_t)0.005) ||
	 NEAR_EQUAL(gvp->gv_aet[1], -90.0, (fastf_t)0.005)) &&
	gvp->gv_aet[0] < 0 &&
	!NEAR_ZERO(gvp->gv_aet[0], (fastf_t)0.005))
	gvp->gv_aet[0] += 360.0;
    else if (NEAR_ZERO(gvp->gv_aet[0], (fastf_t)0.005))
	gvp->gv_aet[0] = 0.0;

    /* apply the perspective angle to model2view */
    bn_mat_mul(gvp->gv_pmodel2view, gvp->gv_pmat, gvp->gv_model2view);

    if (gvp->gv_callback)
	(*gvp->gv_callback)(gvp, gvp->gv_clientData);
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
