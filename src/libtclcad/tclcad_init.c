/*                   T C L C A D _ I N I T . C
 * BRL-CAD
 *
 * Copyright (c) 2014-2020 United States Government as represented by
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
/** @file tclcad_init.c
 *
 * Functions for initializing Tcl environments and BRL-CAD's libtclcad
 * interface.
 *
 */


#include "common.h"

#define RESOURCE_INCLUDED 1
#include <tcl.h>
#ifdef HAVE_TK
#  include <tk.h>
#endif

#include "vmath.h"
#include "bn.h"
#include "dm.h"
#include "raytrace.h"
#include "tclcad.h"

/* Private headers */
#include "brlcad_version.h"
#include "tclcad_private.h"

/* defined in cmdhist_obj.c */
extern int Cho_Init(Tcl_Interp *interp);


int
library_initialized(int setit)
{
    static int initialized = 0;
    if (setit)
	initialized = 1;

    return initialized;
}


static int
wrapper_func(ClientData data, Tcl_Interp *interp, int argc, const char *argv[])
{
    struct bu_cmdtab *ctp = (struct bu_cmdtab *)data;

    return ctp->ct_func(interp, argc, argv);
}


void
tclcad_register_cmds(Tcl_Interp *interp, struct bu_cmdtab *cmds)
{
    struct bu_cmdtab *ctp = NULL;

    for (ctp = cmds; ctp->ct_name != (char *)NULL; ctp++) {
	(void)Tcl_CreateCommand(interp, ctp->ct_name, wrapper_func, (ClientData)ctp, (Tcl_CmdDeleteProc *)NULL);
    }
}


int
Tclcad_Init(Tcl_Interp *interp)
{
    if (library_initialized(0))
	return TCL_OK;

    if (Tcl_Init(interp) == TCL_ERROR) {
	return TCL_ERROR;
    }

#ifdef HAVE_TK
    if (Tk_Init(interp) == TCL_ERROR) {
	return TCL_ERROR;
    }
#endif

    /* Locate the BRL-CAD-specific Tcl scripts, set the auto_path */
    tclcad_auto_path(interp);

    /* Initialize [incr Tcl] */
    if (Tcl_Eval(interp, "package require Itcl") != TCL_OK) {
      bu_log("Tcl_Eval ERROR:\n%s\n", Tcl_GetStringResult(interp));
      return TCL_ERROR;
    }

#ifdef HAVE_TK
    /* Initialize [incr Tk] */
    if (Tcl_Eval(interp, "package require Itk") != TCL_OK) {
      bu_log("Tcl_Eval ERROR:\n%s\n", Tcl_GetStringResult(interp));
      return TCL_ERROR;
    }
#endif

    /* Initialize the Iwidgets package */
    if (Tcl_Eval(interp, "package require Iwidgets") != TCL_OK) {
	bu_log("Tcl_Eval ERROR:\n%s\n", Tcl_GetStringResult(interp));
	return TCL_ERROR;
    }

    /* Initialize libstruct dm */
    if (Dm_Init(interp) == TCL_ERROR) {
	bu_log("Dm_Init ERROR:\n%s\n", Tcl_GetStringResult(interp));
	return TCL_ERROR;
    }

    /* Initialize libbu */
    if (Bu_Init(interp) == TCL_ERROR) {
	bu_log("Bu_Init ERROR:\n%s\n", Tcl_GetStringResult(interp));
	return TCL_ERROR;
    }

    /* Initialize libbn */
    if (Bn_Init(interp) == TCL_ERROR) {
	bu_log("Bn_Init ERROR:\n%s\n", Tcl_GetStringResult(interp));
	return TCL_ERROR;
    }

    /* Initialize librt */
    if (Rt_Init(interp) == TCL_ERROR) {
	bu_log("Rt_Init ERROR:\n%s\n", Tcl_GetStringResult(interp));
	return TCL_ERROR;
    }

    /* Initialize the GED object */
    if (Go_Init(interp) == TCL_ERROR) {
	bu_log("Go_Init ERROR:\n%s\n", Tcl_GetStringResult(interp));
	return TCL_ERROR;
    }

    /* initialize command history objects */
    Cho_Init(interp);

    Tcl_PkgProvide(interp, "Tclcad", brlcad_version());

    (void)library_initialized(1);

    return TCL_OK;
}


/* avoid including itcl.h/itk.h due to their usage of internal headers */
extern int Itcl_Init(Tcl_Interp *);
#ifdef HAVE_TK
extern int Itk_Init(Tcl_Interp *);
#endif

int
tclcad_init(Tcl_Interp *interp, int init_gui, struct bu_vls *tlog)
{
    int try_auto_path = 0;
    int init_tcl = 1;
    int init_itcl = 1;
#ifdef HAVE_TK
    int init_tk = 1;
    int init_itk = 1;
#endif
    int ret = TCL_OK;

    if (!interp) return TCL_ERROR;

#ifndef HAVE_TK
    if (init_gui) {
	if (tlog) bu_vls_printf(tlog, "tclcad_init ERROR: graphical initialization requested, no graphics support available.\n");
	return TCL_ERROR;
    }
#endif

    /* a two-pass init loop.  the first pass just tries default init
     * routines while the second calls tclcad_auto_path() to help it
     * find other, potentially uninstalled, resources.
     */
    while (1) {

	/* not called first time through, give Tcl_Init() a chance */
	if (try_auto_path) {
	    /* Locate the BRL-CAD-specific Tcl scripts, set the auto_path */
	    tclcad_auto_path(interp);
	}

	/* Initialize Tcl */
	Tcl_ResetResult(interp);
	if (init_tcl && Tcl_Init(interp) == TCL_ERROR) {
	    if (!try_auto_path) {
		try_auto_path = 1;
		continue;
	    }
	    if (tlog) bu_vls_printf(tlog, "Tcl_Init ERROR:\n%s\n", Tcl_GetStringResult(interp));
	    ret = TCL_ERROR;
	    break;
	}
	init_tcl = 0;

#ifdef HAVE_TK
	if (init_gui) {
	    /* Initialize Tk */
	    Tcl_ResetResult(interp);
	    if (init_tk && Tk_Init(interp) == TCL_ERROR) {
		if (!try_auto_path) {
		    try_auto_path=1;
		    continue;
		}
		if (tlog) bu_vls_printf(tlog, "Tk_Init ERROR:\n%s\n", Tcl_GetStringResult(interp));
		ret = TCL_ERROR;
		break;
	    }
	    Tcl_StaticPackage(interp, "Tk", Tk_Init, Tk_SafeInit);
	    init_tk=0;
	}
#endif

	/* Initialize [incr Tcl] */
	Tcl_ResetResult(interp);

	/* Locate the BRL-CAD-specific Tcl scripts, set the auto_path */
	tclcad_auto_path(interp);

	/* NOTE: Calling "package require Itcl" may be a problem - there have
	 * been reports of the Combination Editor in mged failing with an
	 * iwidgets class already loaded error if we don't perform Itcl_Init()
	 * here (TODO: still true, and if so why??).
	 */
	if (init_itcl) {
	    ret = Tcl_Eval(interp, "package require Itcl");
	    if (ret == TCL_ERROR) {
		if (tlog) bu_vls_printf(tlog, "Itcl_Init ERROR:\n%s\n", Tcl_GetStringResult(interp));
		break;
	    }
	}
	init_itcl = 0;

#ifdef HAVE_TK
	if (init_gui && init_itk) {
	    /* Initialize [incr Tk] */
	    ret = Tcl_Eval(interp, "package require Itk");
	    if (ret == TCL_ERROR) {
		if (tlog) bu_vls_printf(tlog, "Itk_Init ERROR:\n%s\n", Tcl_GetStringResult(interp));
		break;
	    }
	    init_itk=0;
	}
#endif

	/* don't actually want to loop forever */
	break;

    } /* end iteration over Init() routines that need auto_path */
    Tcl_ResetResult(interp);

    /* if we haven't loaded by now, load auto_path so we find our tclscripts */
    if (!try_auto_path) {
	/* Locate the BRL-CAD-specific Tcl scripts */
	tclcad_auto_path(interp);
    }

    /* Import Itcl/Itk and iwidgets
     *
     * TODO - this is probably a bad idea - figure out why we're doing it and
     * whether we really need to... */

    if (Tcl_Import(interp, Tcl_GetGlobalNamespace(interp),
		"::itcl::*", /* allowOverwrite */ 1) != TCL_OK) {
	if (tlog) bu_vls_printf(tlog, "Tcl_Import ERROR:\n%s\n", Tcl_GetStringResult(interp));
	ret = TCL_ERROR;
	Tcl_ResetResult(interp);
    }
#ifdef HAVE_TK
    if (init_gui) {
	if (Tcl_Import(interp, Tcl_GetGlobalNamespace(interp),
		    "::itk::*", /* allowOverwrite */ 1) != TCL_OK) {
	    if (tlog) bu_vls_printf(tlog, "Tcl_Import ERROR:\n%s\n", Tcl_GetStringResult(interp));
	    ret = TCL_ERROR;
	    Tcl_ResetResult(interp);
	}
	if (Tcl_Eval(interp, "package require Iwidgets") != TCL_OK) {
	    if (tlog) bu_vls_printf(tlog, "Tcl_Eval ERROR:\n%s\n", Tcl_GetStringResult(interp));
	    ret = TCL_ERROR;
	    Tcl_ResetResult(interp);
	} else {
	    if (Tcl_Import(interp, Tcl_GetGlobalNamespace(interp),
			"::iwidgets::*", /* allowOverwrite */ 1) != TCL_OK) {
		if (tlog) bu_vls_printf(tlog, "Tcl_Import ERROR:\n%s\n", Tcl_GetStringResult(interp));
		ret = TCL_ERROR;
		Tcl_ResetResult(interp);
	    }
	}
    }
#endif

    if (Tcl_Eval(interp, "auto_mkindex_parser::slavehook { _%@namespace import -force ::itcl::* }") != TCL_OK) {
	bu_log("Tcl_Eval ERROR:\n%s\n", Tcl_GetStringResult(interp));
	return TCL_ERROR;
    }

#ifdef HAVE_TK
    if (init_gui) {
	if (Tcl_Eval(interp, "auto_mkindex_parser::slavehook { _%@namespace import -force ::tk::* }") != TCL_OK) {
	    bu_log("Tcl_Eval ERROR:\n%s\n", Tcl_GetStringResult(interp));
	    return TCL_ERROR;
	}
	if (Tcl_Eval(interp, "auto_mkindex_parser::slavehook { _%@namespace import -force ::itk::* }") != TCL_OK) {
	    bu_log("Tcl_Eval ERROR:\n%s\n", Tcl_GetStringResult(interp));
	    return TCL_ERROR;
	}
    }
#endif

    /* BRL-CAD specific components */

    /* Initialize libbu */
    if (Bu_Init(interp) == TCL_ERROR) {
	if (tlog) bu_vls_printf(tlog, "Bu_Init ERROR:\n%s\n", Tcl_GetStringResult(interp));
	ret = TCL_ERROR;
	Tcl_ResetResult(interp);
    }

    /* Initialize libbn */
    if (Bn_Init(interp) == TCL_ERROR) {
	if (tlog) bu_vls_printf(tlog, "Bn_Init ERROR:\n%s\n", Tcl_GetStringResult(interp));
	ret = TCL_ERROR;
	Tcl_ResetResult(interp);
    }

    /* Initialize librt */
    if (Rt_Init(interp) == TCL_ERROR) {
	if (tlog) bu_vls_printf(tlog, "Rt_Init ERROR:\n%s\n", Tcl_GetStringResult(interp));
	ret = TCL_ERROR;
	Tcl_ResetResult(interp);
    }
    Tcl_StaticPackage(interp, "Rt", Rt_Init, (Tcl_PackageInitProc *) NULL);

    /* Initialize libstruct dm */
    if (Dm_Init(interp) == TCL_ERROR) {
	if (tlog) bu_vls_printf(tlog, "Dm_Init ERROR:\n%s\n", Tcl_GetStringResult(interp));
	ret = TCL_ERROR;
	Tcl_ResetResult(interp);
    }
    Tcl_StaticPackage(interp, "Dm", (int (*)(struct Tcl_Interp *))Dm_Init, (Tcl_PackageInitProc *) NULL);
    Tcl_StaticPackage(interp, "Fb", (int (*)(struct Tcl_Interp *))Dm_Init, (Tcl_PackageInitProc *) NULL);

    /* Initialize libged */
    if (Go_Init(interp) == TCL_ERROR) {
	if (tlog) bu_vls_printf(tlog, "Ged_Init ERROR:\n%s\n", Tcl_GetStringResult(interp));
	ret = TCL_ERROR;
	Tcl_ResetResult(interp);
    }

    /* Initialize history */
    Cho_Init(interp);

#ifdef HAVE_TK
    /* If we're doing Tk, make sure we have a window */
    if (init_gui) {
	Tk_Window tkwin = Tk_MainWindow(interp);
	if (!tkwin) ret = TCL_ERROR;
    }
#endif

    return ret;
}

void
tclcad_set_argv(Tcl_Interp *interp, int argc, const char **argv)
{
    char buf[TCL_INTEGER_SPACE] = {0};
    char *args;
    Tcl_DString argString;
    char nstr = '\0';
    const char **av;

    if (!interp) return;

    /* create the tcl variables, even if they're empty */
    av = (!argv) ? (const char **)&nstr : argv;

    /* argc */
    sprintf(buf, "%ld", (long)(argc));
    Tcl_SetVar(interp, "argc", buf, TCL_GLOBAL_ONLY);

    /* argv */
    args = Tcl_Merge(argc, (const char * const *)av);
    Tcl_ExternalToUtfDString(NULL, args, -1, &argString);
    Tcl_SetVar(interp, "argv", Tcl_DStringValue(&argString), TCL_GLOBAL_ONLY);
    Tcl_DStringFree(&argString);
    ckfree(args);
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
