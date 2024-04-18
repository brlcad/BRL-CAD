/*                     F A C E T I Z E . C P P
 * BRL-CAD
 *
 * Copyright (c) 2008-2024 United States Government as represented by
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
/** @file libged/facetize.cpp
 *
 * The facetize command.
 *
 */

#include "common.h"

#include <charconv>
#include <map>
#include <sstream>
#include <string>
#include <vector>

#include "bu/app.h"
#include "bu/path.h"
#include "bu/opt.h"

#include "../ged_private.h"

#define TESS_OPTS_IMPLEMENTATION
#include "./tess_opts.h"
#include "./ged_facetize.h"
#include "./subprocess.h"

void
_facetize_methods_help(struct ged *gedp)
{
        // Build up the path to the ged_tessellate executable
      char tess_exec[MAXPATHLEN];
      bu_dir(tess_exec, MAXPATHLEN, BU_DIR_BIN, "ged_tessellate", BU_DIR_EXT, NULL);

      const char *tess_cmd[MAXPATHLEN] = {NULL};
      tess_cmd[ 0] = tess_exec;
      tess_cmd[ 1] = "--list-methods";
      tess_cmd[ 2] = NULL;

      struct subprocess_s mp;
      if (subprocess_create(tess_cmd, subprocess_option_no_window, &mp))
          return; // Unable to create subprocess??
      int w_rc;
      if (subprocess_join(&mp, &w_rc))
          return ; // Unable to join??
      char mraw[MAXPATHLEN] = {'\0'};
      subprocess_read_stdout(&mp, mraw, MAXPATHLEN);
      std::string method_list(mraw);

      tess_cmd[ 2] = "-h";
      tess_cmd[ 3] = NULL;

      struct subprocess_s mop;
      if (subprocess_create(tess_cmd, subprocess_option_no_window, &mop))
          return; // Unable to create subprocess??
      if (subprocess_join(&mop, &w_rc))
          return ; // Unable to join??
      char moraw[MAXPATHLEN*10] = {'\0'};
      subprocess_read_stdout(&mop, moraw, MAXPATHLEN*10);
      std::string method_options(moraw);

      bu_vls_printf(gedp->ged_result_str, "Available BoT tessellation methods: %s\n", method_list.c_str());
      bu_vls_printf(gedp->ged_result_str, "Method specific options:\n%s\n", method_options.c_str());
}

struct _ged_facetize_state *
_ged_facetize_state_create()
{
    struct _ged_facetize_state *s = NULL;
    BU_GET(s, struct _ged_facetize_state);
    s->quiet= 0;
    s->verbosity = 0;
    s->no_empty = 0;
    s->make_nmg = 0;
    s->nonovlp_brep = 0;

    s->regions = 0;
    s->resume = 0;
    s->in_place = 0;

    BU_GET(s->faceted_suffix, struct bu_vls);
    bu_vls_init(s->faceted_suffix);
    bu_vls_sprintf(s->faceted_suffix, ".bot");

    s->max_time = 0;
    s->max_pnts = 0;

    s->tol = NULL;
    s->nonovlp_threshold = 0;

    s->gedp = NULL;

    BU_ALLOC(s->c_map, struct bu_attribute_value_set);
    BU_ALLOC(s->s_map, struct bu_attribute_value_set);
    bu_avs_init_empty(s->c_map);
    bu_avs_init_empty(s->s_map);

    return s;
}
void _ged_facetize_state_destroy(struct _ged_facetize_state *s)
{
    if (!s)
       	return;

    if (s->faceted_suffix) {
	bu_vls_free(s->faceted_suffix);
	BU_PUT(s->faceted_suffix, struct bu_vls);
    }

    bu_avs_free(s->c_map);
    bu_avs_free(s->s_map);
    bu_free(s->c_map, "comb map");
    bu_free(s->s_map, "solid map");

    BU_PUT(s, struct _ged_facetize_state);
}

int
_ged_facetize_objs(struct _ged_facetize_state *s, int argc, const char **argv)
{
    struct ged *gedp = s->gedp;
    int ret = BRLCAD_ERROR;
    int newobj_cnt, i;
    const char *oname = NULL;
    const char *av[2];
    struct directory **dpa = NULL;
    struct directory *idpa[2];
    struct db_i *dbip = gedp->dbip;

    RT_CHECK_DBI(dbip);

    if (argc < 0) return BRLCAD_ERROR;

    dpa = (struct directory **)bu_calloc(argc, sizeof(struct directory *), "dp array");
    newobj_cnt = _ged_sort_existing_objs(gedp, argc, argv, dpa);
    if (_ged_validate_objs_list(s, argc, argv, newobj_cnt) == BRLCAD_ERROR) {
	bu_free(dpa, "dp array");
	return BRLCAD_ERROR;
    }

    if (!s->in_place) {
	oname = argv[argc-1];
	argc--;
    }

    /* If we're doing an NMG output, use the old-school libnmg booleval */
    if (s->make_nmg) {
	if (!s->in_place) {
	    ret = _ged_facetize_nmgeval(s, argc, argv, oname);
	    goto booleval_cleanup;
	} else {
	    for (i = 0; i < argc; i++) {
		av[0] = argv[i];
		av[1] = NULL;
		ret = _ged_facetize_nmgeval(s, 1, av, argv[i]);
		if (ret == BRLCAD_ERROR)
		    goto booleval_cleanup;
	    }
	    goto booleval_cleanup;
	}
    }

    // If we're not doing NMG, use the Manifold booleval
    if (!s->in_place) {
	ret = _ged_facetize_booleval(s, newobj_cnt, dpa, oname, NULL, NULL, false);
    } else {
	for (i = 0; i < argc; i++) {
	    idpa[0] = dpa[i];
	    idpa[1] = NULL;
	    ret = _ged_facetize_booleval(s, 1, (struct directory **)idpa, argv[i], NULL, NULL, false);
	    if (ret == BRLCAD_ERROR)
		goto booleval_cleanup;
	}
    }

booleval_cleanup:
    bu_free(dpa, "dp array");
    return ret;
}

extern "C" int
ged_facetize_core(struct ged *gedp, int argc, const char *argv[])
{
    int ret = BRLCAD_OK;
    static const char *usage = "Usage: facetize [options] [old_obj1 ...] [new_obj]\n";
    int print_help = 0;
    int need_help = 0;
    method_options_t *method_options = new method_options_t;
    std::map<std::string, std::map<std::string,std::string>>::iterator o_it;
    struct _ged_facetize_state *s = _ged_facetize_state_create();
    s->gedp = gedp;
    s->method_opts = method_options;

    /* General options */
    struct bu_opt_desc d[15];
    BU_OPT(d[ 0], "h", "help",                                      "",                  NULL,           &print_help, "Print help and exit");
    BU_OPT(d[ 1], "v", "verbose",                                   "",            &_ged_vopt,       &(s->verbosity), "Verbose output (multiple flags increase verbosity)");
    BU_OPT(d[ 2], "q", "quiet",                                     "",                  NULL,           &(s->quiet), "Suppress all output (overrides verbose flag)");
    BU_OPT(d[ 3], "n", "nmg-output",                                "",                  NULL,        &(s->make_nmg), "Create an N-Manifold Geometry (NMG) object (default is to create a triangular BoT mesh).  Note that this will disable most other processing options and may reduce the conversion success rate.");
    BU_OPT(d[ 4], "r", "regions",                                   "",                  NULL,         &(s->regions), "For combs, walk the trees and create new copies of the hierarchies with each region's CSG tree replaced by a facetized evaluation of that region. (Default is to create one facetized object.)");
    BU_OPT(d[ 5],  "", "in-place",                                  "",                  NULL,        &(s->in_place), "Replace the specified object(s) with their facetizations. (Warning: this option changes pre-existing geometry!)");
    BU_OPT(d[ 6],  "", "max-time",                                 "#",           &bu_opt_int,        &(s->max_time), "Maximum time to spend per object (in seconds).  Default is method specific.  Note that specifying shorter times may cut off conversions (particularly using sampling methods) that could succeed with longer runtimes.  Per-method time limits can also be adjusted to allow longer runtimes on slower methods.");
    BU_OPT(d[ 7],  "", "max-pnts",                                 "#",           &bu_opt_int,        &(s->max_pnts), "Maximum number of pnts per object to use when applying ray sampling methods.");
    BU_OPT(d[ 8],  "", "resume",                                    "",                  NULL,          &(s->resume), "Resume an interrupted conversion");
    BU_OPT(d[ 9],  "", "methods",                          "m1,m2,...", &_tess_active_methods,        method_options, "Specify methods to use when tessellating primitives into BoTs.");
    BU_OPT(d[10],  "", "method-opts",    "METHOD opt1=val opt2=val...",    &_tess_method_opts,        method_options, "For the specified method, set the specified options.");
    BU_OPT(d[11],  "", "no-empty",                                  "",                  NULL,        &(s->no_empty), "Do not output empty BoT objects if the boolean evaluation results in an empty solid.");
    BU_OPT(d[12], "B", "",                                          "",                  NULL,      &s->nonovlp_brep, "EXPERIMENTAL: non-overlapping facetization to BoT objects of union-only brep comb tree.");
    BU_OPT(d[13], "t", "threshold",                                "#",       &bu_opt_fastf_t, &s->nonovlp_threshold, "EXPERIMENTAL: max ovlp threshold length for -B mode.");
    BU_OPT_NULL(d[14]);

    GED_CHECK_DATABASE_OPEN(gedp, BRLCAD_ERROR);
    GED_CHECK_READ_ONLY(gedp, BRLCAD_ERROR);
    GED_CHECK_ARGC_GT_0(gedp, argc, BRLCAD_ERROR);

    /* skip command name argv[0] */
    argc-=(argc>0); argv+=(argc>0);

    /* initialize result */
    bu_vls_trunc(gedp->ged_result_str, 0);

    /* parse standard options */
    struct bu_vls omsg = BU_VLS_INIT_ZERO;
    argc = bu_opt_parse(&omsg, argc, argv, d);
    if (argc < 0) {
	bu_vls_printf(gedp->ged_result_str, "option parsing failed: %s\n", bu_vls_cstr(&omsg));
	ret = BRLCAD_ERROR;
	bu_vls_free(&omsg);
	goto ged_facetize_memfree;
    }
    bu_vls_free(&omsg);

    // If we got a max-time top level arg, override any times that aren't specifically set
    // by method options
    if (s->max_time) {
	for (o_it = method_options->options_map.begin(); o_it != method_options->options_map.end(); o_it++) {
	    std::map<std::string,std::string>::iterator m_it = o_it->second.find(std::string("max_time"));
	    if (m_it == o_it->second.end()) {
		// max-time wasn't explicitly set by a method, and we have an option - override
		method_options->max_time[o_it->first] = s->max_time;
		o_it->second[std::string("max_time")] = std::to_string(s->max_time);
	    }
	}
    }

    /* Sync -q and -v options */
    if (s->quiet && s->verbosity)
       	s->verbosity = 0;

    /* Don't allow incorrect type suffixes */
    if (s->make_nmg && BU_STR_EQUAL(bu_vls_cstr(s->faceted_suffix), ".bot")) {
	bu_vls_sprintf(s->faceted_suffix, ".nmg");
    }
    if (!s->make_nmg && BU_STR_EQUAL(bu_vls_cstr(s->faceted_suffix), ".nmg")) {
	bu_vls_sprintf(s->faceted_suffix, ".bot");
    }

    /* We can only resume in region mode - TODO - this shouldn't be true any more... */
    if (s->resume && !s->regions) {
	bu_vls_printf(gedp->ged_result_str, "--resume is only supported with with region (-r) mode\n");
	ret = BRLCAD_ERROR;
	goto ged_facetize_memfree;
    }

    /* Check if we want/need help */
    need_help += (argc < 1);
    need_help += (argc < 2 && !s->in_place && !s->regions && !s->resume && !s->nonovlp_brep);
    if (print_help || need_help || argc < 1) {
	_ged_cmd_help(gedp, usage, d);
	_facetize_methods_help(gedp);
	ret = (need_help) ? BRLCAD_ERROR : BRLCAD_OK;
	goto ged_facetize_memfree;
    }

    /* If we're doing the experimental brep-only logic, it's a separate process */
    if (s->nonovlp_brep) {
	// TODO - for this mode, we need to xpush the tree(s) being processed.  Overlaps occur
	// in instances, so (unlike the default method) we need to mimic the older approach of
	// tessellating each individual instance in the tree.  Since we're making a working
	// copy of the .g file, we can just use xpush to achieve this state as a pre-processing
	// step rather than having to change up the whole processing chain.
	if (NEAR_ZERO(s->nonovlp_threshold, SMALL_FASTF)) {
	    bu_vls_printf(gedp->ged_result_str, "-B option requires a specified length threshold\n");
	    return BRLCAD_ERROR;
	}
	ret = _nonovlp_brep_facetize(s, argc, argv);
	goto ged_facetize_memfree;
    }

    /* Multi-region mode has a different processing logic */
    if (s->regions) {
	ret = _ged_facetize_regions(s, argc, argv);
    } else {
	ret = _ged_facetize_objs(s, argc, argv);
    }

ged_facetize_memfree:
    _ged_facetize_state_destroy(s);
    delete method_options;

    return ret;
}




#ifdef GED_PLUGIN
#include "../include/plugin.h"
extern "C" {
struct ged_cmd_impl facetize_cmd_impl = { "facetize", ged_facetize_core, GED_CMD_DEFAULT };
const struct ged_cmd facetize_cmd = { &facetize_cmd_impl };
const struct ged_cmd *facetize_cmds[] = { &facetize_cmd,  NULL };

static const struct ged_plugin pinfo = { GED_API,  facetize_cmds, 1 };

COMPILER_DLLEXPORT const struct ged_plugin *ged_plugin_info(void)
{
    return &pinfo;
}
}
#endif

// Local Variables:
// tab-width: 8
// mode: C++
// c-basic-offset: 4
// indent-tabs-mode: t
// c-file-style: "stroustrup"
// End:
// ex: shiftwidth=4 tabstop=8

