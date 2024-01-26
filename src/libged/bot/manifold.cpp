/*                     M A N I F O L D . C P P
 * BRL-CAD
 *
 * Copyright (c) 2020-2023 United States Government as represented by
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
/** @file libged/bot/manifold.cpp
 *
 * The LIBGED bot manifold subcommand.
 *
 * Checks if a BoT is manifold according to the Manifold library.  If
 * not, and if an output object name is specified, it will attempt
 * various repair operations to try and produce a manifold output
 * using the specified test bot as an input.
 */

#include "common.h"

#include <stdlib.h>
#include <string.h>

#include <algorithm>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <list>
#include <map>
#include <queue>
#include <set>
#include <sstream>
#include <string>
#include <vector>

#include "manifold/manifold.h"
#ifdef USE_ASSETIMPORT
#include "manifold/meshIO.h"
#endif

#include "geogram/basic/process.h"
#include <geogram/basic/command_line.h>
#include <geogram/basic/command_line_args.h>
#include "geogram/mesh/mesh.h"
#include "geogram/mesh/mesh_geometry.h"
#include "geogram/mesh/mesh_repair.h"
#include "geogram/mesh/mesh_fill_holes.h"

#include "bu/cmd.h"
#include "bu/color.h"
#include "bu/opt.h"
#include "bg/chull.h"
#include "bg/trimesh.h"
#include "rt/geom.h"
#include "wdb.h"

#include "./ged_bot.h"
#include "../ged_private.h"

static struct rt_bot_internal *
manifold_process(struct rt_bot_internal *bot, int repair)
{
    if (!bot)
	return NULL;

    manifold::Mesh bot_mesh;
    for (size_t j = 0; j < bot->num_vertices ; j++)
	bot_mesh.vertPos.push_back(glm::vec3(bot->vertices[3*j], bot->vertices[3*j+1], bot->vertices[3*j+2]));
    if (bot->orientation == RT_BOT_CW) {
	for (size_t j = 0; j < bot->num_faces; j++)
	    bot_mesh.triVerts.push_back(glm::vec3(bot->faces[3*j], bot->faces[3*j+2], bot->faces[3*j+1]));
    } else {
	for (size_t j = 0; j < bot->num_faces; j++)
	    bot_mesh.triVerts.push_back(glm::vec3(bot->faces[3*j], bot->faces[3*j+1], bot->faces[3*j+2]));
    }

    manifold::MeshGL bot_gl(bot_mesh);
    if (!bot_gl.NumTri()) {
	// GetMeshGL failed, cannot process.
	return NULL;
    }

    if (!bot_gl.Merge()) {
	// BoT is already manifold
	return bot;
    }

    // If we're not going to try and repair it, it's just a
    // non-manifold mesh report at this point.
    if (!repair)
	return NULL;

    manifold::Manifold omanifold(bot_gl);
    if (omanifold.Status() != manifold::Manifold::Error::NoError) {
	// MeshGL.Merge() failed to produce a manifold mesh"
	return NULL;
    }

    // Original wasn't manifold, but the Manifold Merge() function was
    // able to produce a variation that is.  Make a new BoT
    manifold::Mesh omesh = omanifold.GetMesh();
    struct rt_bot_internal *nbot;
    BU_GET(nbot, struct rt_bot_internal);
    nbot->magic = RT_BOT_INTERNAL_MAGIC;
    nbot->mode = RT_BOT_SOLID;
    nbot->orientation = RT_BOT_CCW;
    nbot->thickness = NULL;
    nbot->face_mode = (struct bu_bitv *)NULL;
    nbot->bot_flags = 0;
    nbot->num_vertices = (int)omesh.vertPos.size();
    nbot->num_faces = (int)omesh.triVerts.size();
    nbot->vertices = (double *)calloc(omesh.vertPos.size()*3, sizeof(double));;
    nbot->faces = (int *)calloc(omesh.triVerts.size()*3, sizeof(int));
    for (size_t j = 0; j < omesh.vertPos.size(); j++) {
	nbot->vertices[3*j] = omesh.vertPos[j].x;
	nbot->vertices[3*j+1] = omesh.vertPos[j].y;
	nbot->vertices[3*j+2] = omesh.vertPos[j].z;
    }
    for (size_t j = 0; j < omesh.triVerts.size(); j++) {
	nbot->faces[3*j] = omesh.triVerts[j].x;
	nbot->faces[3*j+1] = omesh.triVerts[j].y;
	nbot->faces[3*j+2] = omesh.triVerts[j].z;
    }

    return nbot;
}

static struct rt_bot_internal *
geogram_mesh_repair(struct rt_bot_internal *bot)
{
    if (!bot)
	return NULL;

    GEO::CmdLine::set_arg("algo:hole_filling", "loop_split");

    GEO::Mesh gm;
    gm.vertices.assign_points((double *)bot->vertices, 3, bot->num_vertices);

    GEO::vector<GEO::index_t> tris;
    for (size_t i = 0; i < bot->num_faces * 3; i++) {
	tris.push_back(bot->faces[i]);
    }
    gm.facets.assign_triangle_mesh(tris, false);

    GEO::fill_holes(gm, 1e30);

    GEO::mesh_repair(gm, GEO::MeshRepairMode(GEO::MESH_REPAIR_DEFAULT));

    // Post repair, make sure mesh is still a triangle mesh
    gm.facets.triangulate();

    // Once Geogram is done with it, ask Manifold what it thinks
    // of the result - if Manifold doesn't think we're there, then
    // the results won't fly for boolean evaluation and we go ahead
    // and reject.
    manifold::Mesh gmm;
    for(GEO::index_t v = 0; v < gm.vertices.nb(); v++) {
	double gm_v[3];
	const double *p = gm.vertices.point_ptr(v);
	for (int i = 0; i < 3; i++) {
	    gm_v[i] = p[i];
	}
	gmm.vertPos.push_back(glm::vec3(gm_v[0], gm_v[1], gm_v[2]));
    }
    for (GEO::index_t f = 0; f < gm.facets.nb(); f++) {
	double tri_verts[3];
	for (int i = 0; i < 3; i++) {
	    tri_verts[i] = gm.facets.vertex(f, i);
	}
	// TODO - CW vs CCW orientation handling?
	gmm.triVerts.push_back(glm::vec3(tri_verts[0], tri_verts[1], tri_verts[2]));
    }

    manifold::Manifold gmanifold(gmm);
    if (gmanifold.Status() != manifold::Manifold::Error::NoError) {
	// Repair failed
	return NULL;
    }

    // If output is manifold, make a new bot
    manifold::Mesh omesh = gmanifold.GetMesh();
    struct rt_bot_internal *nbot;
    BU_GET(nbot, struct rt_bot_internal);
    nbot->magic = RT_BOT_INTERNAL_MAGIC;
    nbot->mode = RT_BOT_SOLID;
    nbot->orientation = RT_BOT_CCW;
    nbot->thickness = NULL;
    nbot->face_mode = (struct bu_bitv *)NULL;
    nbot->bot_flags = 0;
    nbot->num_vertices = (int)omesh.vertPos.size();
    nbot->num_faces = (int)omesh.triVerts.size();
    nbot->vertices = (double *)calloc(omesh.vertPos.size()*3, sizeof(double));;
    nbot->faces = (int *)calloc(omesh.triVerts.size()*3, sizeof(int));
    for (size_t j = 0; j < omesh.vertPos.size(); j++) {
	nbot->vertices[3*j] = omesh.vertPos[j].x;
	nbot->vertices[3*j+1] = omesh.vertPos[j].y;
	nbot->vertices[3*j+2] = omesh.vertPos[j].z;
    }
    for (size_t j = 0; j < omesh.triVerts.size(); j++) {
	nbot->faces[3*j] = omesh.triVerts[j].x;
	nbot->faces[3*j+1] = omesh.triVerts[j].y;
	nbot->faces[3*j+2] = omesh.triVerts[j].z;
    }
    return nbot;
}

extern "C" int
_bot_cmd_manifold(void *bs, int argc, const char **argv)
{
    const char *usage_string = "bot manifold <objname> [repaired_obj_name]";
    const char *purpose_string = "Check if Manifold thinks the BoT is a manifold mesh.  If not, and a repaired_obj_name has been supplied, attempt to produce a manifold output using objname's geometry as an input.  If successful, the resulting manifold geometry will be written to repaired_obj_name.";
    if (_bot_cmd_msgs(bs, argc, argv, usage_string, purpose_string)) {
	return BRLCAD_OK;
    }

    // GEO::initialize needs to be called once.  TODO - this probably
    // should happen once for all of libged, if we use geogram in
    // multiple commands...
    static int geo_inited = 0;
    if (!geo_inited) {
	GEO::initialize();
	geo_inited = 1;
    }

    struct _ged_bot_info *gb = (struct _ged_bot_info *)bs;

    argc--; argv++;

    if (argc != 1 && argc != 2) {
	bu_vls_printf(gb->gedp->ged_result_str, "%s", usage_string);
	return BRLCAD_ERROR;
    }

    if (_bot_obj_setup(gb, argv[0]) & BRLCAD_ERROR) {
	return BRLCAD_ERROR;
    }

    struct rt_bot_internal *bot = (struct rt_bot_internal *)(gb->intern->idb_ptr);
    if (bot->mode != RT_BOT_SOLID) {
	bu_vls_printf(gb->gedp->ged_result_str, "%s is a non-solid BoT, skipping", gb->dp->d_namep);
	return BRLCAD_ERROR;
    }

    int repair_flag = (argc == 2) ?  1 : 0;

    struct rt_bot_internal *mbot = manifold_process(bot, repair_flag);

    // If we're already manifold, there's nothing to do
    if (mbot && mbot == bot) {
	bu_vls_printf(gb->gedp->ged_result_str, "BoT %s is manifold", gb->dp->d_namep);
	return BRLCAD_OK;
    }

    // If we don't have a repair target, just report non-manifold
    if (!mbot && !repair_flag) {
    	bu_vls_printf(gb->gedp->ged_result_str, "BoT %s is NOT manifold", gb->dp->d_namep);
	return BRLCAD_ERROR;
    }

    // If we're repairing and Manifold Merge was able to fix it, go with that
    if (mbot && repair_flag) {
	struct rt_db_internal intern;
	RT_DB_INTERNAL_INIT(&intern);
	intern.idb_major_type = DB5_MAJORTYPE_BRLCAD;
	intern.idb_type = ID_BOT;
	intern.idb_meth = &OBJ[ID_BOT];
	intern.idb_ptr = (void *)mbot;

	const char *rname = argv[1];

	struct directory *dp = db_diradd(gb->gedp->dbip, rname, RT_DIR_PHONY_ADDR, 0, RT_DIR_SOLID, (void *)&intern.idb_type);
	if (dp == RT_DIR_NULL) {
	    bu_vls_printf(gb->gedp->ged_result_str, "Failed to write out new BoT %s", rname);
	    return BRLCAD_ERROR;
	}

	if (rt_db_put_internal(dp, gb->gedp->dbip, &intern, &rt_uniresource) < 0) {
	    bu_vls_printf(gb->gedp->ged_result_str, "Failed to write out new BoT %s", rname);
	    return BRLCAD_ERROR;
	}

	bu_vls_printf(gb->gedp->ged_result_str, "Repair completed successfully (Manifold Merge op, minimal changes) and written to %s", rname);
	return BRLCAD_OK;
    }

    // Not manifold, not fixed by the first pass, and we want to repair it.
    // Time to attempt mesh repair
    mbot = geogram_mesh_repair(bot);

    if (!mbot) {
	bu_vls_printf(gb->gedp->ged_result_str, "Unable to repair BoT %s", gb->dp->d_namep);
	return BRLCAD_ERROR;
    }

    struct rt_db_internal intern;
    RT_DB_INTERNAL_INIT(&intern);
    intern.idb_major_type = DB5_MAJORTYPE_BRLCAD;
    intern.idb_type = ID_BOT;
    intern.idb_meth = &OBJ[ID_BOT];
    intern.idb_ptr = (void *)mbot;

    const char *rname = argv[1];

    struct directory *dp = db_diradd(gb->gedp->dbip, rname, RT_DIR_PHONY_ADDR, 0, RT_DIR_SOLID, (void *)&intern.idb_type);
    if (dp == RT_DIR_NULL) {
	bu_vls_printf(gb->gedp->ged_result_str, "Failed to write out new BoT %s", rname);
	return BRLCAD_ERROR;
    }

    if (rt_db_put_internal(dp, gb->gedp->dbip, &intern, &rt_uniresource) < 0) {
	bu_vls_printf(gb->gedp->ged_result_str, "Failed to write out new BoT %s", rname);
	return BRLCAD_ERROR;
    }

    bu_vls_printf(gb->gedp->ged_result_str, "Repair completed successfully (Geogram hole fill) and written to %s", rname);
    return BRLCAD_OK;
}

// Local Variables:
// tab-width: 8
// mode: C++
// c-basic-offset: 4
// indent-tabs-mode: t
// c-file-style: "stroustrup"
// End:
// ex: shiftwidth=4 tabstop=8

