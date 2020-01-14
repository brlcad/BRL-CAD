/*            C D T _ O V L P S _ G R P S . C P P
 * BRL-CAD
 *
 * Copyright (c) 2007-2019 United States Government as represented by
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
/** @addtogroup libbrep */
/** @{ */
/** @file cdt_ovlps.cpp
 *
 * Constrained Delaunay Triangulation of NURBS B-Rep objects.
 *
 * This file is logic specifically for refining meshes to clear overlaps
 * between sets of triangles in different meshes that need to share a common
 * tessellation.
 *
 */

#include "common.h"
#include <iostream>
#include <numeric>
#include <queue>
#include <random>
#include <string>
#include "bu/str.h"
#include "bg/chull.h"
#include "bg/tri_pt.h"
#include "bg/tri_tri.h"
#include "./cdt.h"

void
ovlp_grp::plot(const char *fname, int ind)
{

    omesh_t *om = (!ind) ? om1 : om2;
    std::set<triangle_t> &tris = (!ind) ? tris1 : tris2;

    struct bu_color c = BU_COLOR_INIT_ZERO;
    unsigned char rgb[3] = {0,0,255};

    // Randomize the blue so (usually) subsequent
    // draws of different meshes won't erase the
    // previous wireframe
    std::random_device rd;
    std::mt19937 reng(rd());
    std::uniform_int_distribution<> rrange(100,250);
    int brand = rrange(reng);
    rgb[2] = (unsigned char)brand;

    FILE *plot = fopen(fname, "w");

    RTree<size_t, double, 3>::Iterator tree_it;
    size_t t_ind;
    triangle_t tri;

    bu_color_from_rgb_chars(&c, (const unsigned char *)rgb);

    om->fmesh->tris_tree.GetFirst(tree_it);
    while (!tree_it.IsNull()) {
	t_ind = *tree_it;
	tri = om->fmesh->tris_vect[t_ind];
	om->fmesh->plot_tri(tri, &c, plot, 255, 0, 0);
	++tree_it;
    }

    double tri_r = 0;

    bu_color_rand(&c, BU_COLOR_RANDOM_LIGHTENED);
    pl_color_buc(plot, &c);
    std::set<triangle_t>::iterator ts_it;
    for (ts_it = tris.begin(); ts_it != tris.end(); ts_it++) {
	tri = *ts_it;
	double tr = om->fmesh->tri_pnt_r(tri.ind);
	tri_r = (tr > tri_r) ? tr : tri_r;
	om->fmesh->plot_tri(tri, &c, plot, 255, 0, 0);
    }

    fclose(plot);
}

void
ovlp_grp::plot(const char *fname)
{
    struct bu_vls omname = BU_VLS_INIT_ZERO;
    bu_vls_sprintf(&omname, "%s-0.plot3", fname);
    plot(bu_vls_cstr(&omname), 0);
    bu_vls_sprintf(&omname, "%s-1.plot3", fname);
    plot(bu_vls_cstr(&omname), 1);
    bu_vls_free(&omname);
}



// Validate that all triangles in tris are still in the fmesh - if
// not, remove them so we don't try to process them.
bool
ovlp_grp::validate()
{
    // Once we're invalid, no point in doing the work again
    if (state == -1) {
	return false;
    }

    // Assume valid until proven otherwise...
    state = 1;

    std::set<triangle_t> etris;
    std::set<triangle_t>::iterator tr_it;
    for (tr_it = tris1.begin(); tr_it != tris1.end(); tr_it++) {
	if (!om1->fmesh->tri_active(tr_it->ind)) {
	    etris.insert(*tr_it);
	}
    }
    for (tr_it = etris.begin(); tr_it != etris.end(); tr_it++) {
	tris1.erase(*tr_it);
    }

    etris.clear();
    for (tr_it = tris2.begin(); tr_it != tris2.end(); tr_it++) {
	if (!om1->fmesh->tri_active(tr_it->ind)) {
	    etris.insert(*tr_it);
	}
    }
    for (tr_it = etris.begin(); tr_it != etris.end(); tr_it++) {
	tris2.erase(*tr_it);
    }

    if (!tris1.size() || !tris2.size()) {
	state = -1;
	return false;
    }

    return true;
}

bool
ovlp_grp::fit_plane()
{
    std::set<long> verts1;
    std::set<long> verts2;

    // Clear out anything stale
    if (!validate()) return false;

    // Collect the vertices
    std::set<triangle_t>::iterator tr_it;
    for (tr_it = tris1.begin(); tr_it != tris1.end(); tr_it++) {
	for (int i = 0; i < 3; i++) {
	    verts1.insert(tr_it->v[i]);
	}
    }
    for (tr_it = tris2.begin(); tr_it != tris2.end(); tr_it++) {
	for (int i = 0; i < 3; i++) {
	    verts2.insert(tr_it->v[i]);
	}
    }

    if (!verts1.size() || !verts2.size()) {
	return false;
    }

    std::set<ON_3dPoint> plane1_pnts;
    std::set<ON_3dPoint> plane2_pnts;
    std::set<ON_3dPoint> plane_pnts;
    std::set<long>::iterator vp_it;
    for (vp_it = verts1.begin(); vp_it != verts1.end(); vp_it++) {
	ON_3dPoint p = *om1->fmesh->pnts[*vp_it];
	plane1_pnts.insert(p);
	plane_pnts.insert(p);
    }
    for (vp_it = verts2.begin(); vp_it != verts2.end(); vp_it++) {
	ON_3dPoint p = *om2->fmesh->pnts[*vp_it];
	plane2_pnts.insert(p);
	plane_pnts.insert(p);
    }

    ON_Plane fplane1;
    {
	point_t pcenter;
	vect_t pnorm;
	point_t *fpnts = (point_t *)bu_calloc(plane1_pnts.size(), sizeof(point_t), "fitting points");
	std::set<ON_3dPoint>::iterator p_it;
	int pcnt = 0;
	for (p_it = plane1_pnts.begin(); p_it != plane1_pnts.end(); p_it++) {
	    fpnts[pcnt][X] = p_it->x;
	    fpnts[pcnt][Y] = p_it->y;
	    fpnts[pcnt][Z] = p_it->z;
	    pcnt++;
	}
	if (bn_fit_plane(&pcenter, &pnorm, plane1_pnts.size(), fpnts)) {
	    std::cout << "fitting plane failed!\n";
	}
	bu_free(fpnts, "fitting points");
	fplane1 = ON_Plane(pcenter, pnorm);
    }
    fp1 = fplane1;

    ON_Plane fplane2;
    {
	point_t pcenter;
	vect_t pnorm;
	point_t *fpnts = (point_t *)bu_calloc(plane2_pnts.size(), sizeof(point_t), "fitting points");
	std::set<ON_3dPoint>::iterator p_it;
	int pcnt = 0;
	for (p_it = plane2_pnts.begin(); p_it != plane2_pnts.end(); p_it++) {
	    fpnts[pcnt][X] = p_it->x;
	    fpnts[pcnt][Y] = p_it->y;
	    fpnts[pcnt][Z] = p_it->z;
	    pcnt++;
	}
	if (bn_fit_plane(&pcenter, &pnorm, plane2_pnts.size(), fpnts)) {
	    std::cout << "fitting plane failed!\n";
	}
	bu_free(fpnts, "fitting points");
	fplane2 = ON_Plane(pcenter, pnorm);
    }
    fp2 = fplane2;

    double pangle = ang_deg(fplane1.Normal(), fplane2.Normal());
    if (pangle > 30) {
	// If the planes are too far from parallel, we're done - there is no
	// sensible common plane for projections.
	separate_planes = true;
	return true;
    }
    separate_planes = false;

    ON_Plane fplane;
    {
	point_t pcenter;
	vect_t pnorm;
	point_t *fpnts = (point_t *)bu_calloc(plane_pnts.size(), sizeof(point_t), "fitting points");
	std::set<ON_3dPoint>::iterator p_it;
	int pcnt = 0;
	for (p_it = plane_pnts.begin(); p_it != plane_pnts.end(); p_it++) {
	    fpnts[pcnt][X] = p_it->x;
	    fpnts[pcnt][Y] = p_it->y;
	    fpnts[pcnt][Z] = p_it->z;
	    pcnt++;
	}
	if (bn_fit_plane(&pcenter, &pnorm, plane_pnts.size(), fpnts)) {
	    std::cout << "fitting plane failed!\n";
	}
	bu_free(fpnts, "fitting points");
	fplane = ON_Plane(pcenter, pnorm);
    }

    fp1 = fplane;
    fp2 = fplane;

    return true;
}

// TODO - the optimize calls here are calling the mesh repair subroutines, which don't
// (yet) have an option for allowing the specification of an explicit plane when
// building the initial polygon loop - it's calculating that plane from the triangle.
// That won't give us our shared plane needed for this step, so going to have to
// generalize that routine to allow an optional externally supplied projection plane.
bool
ovlp_grp::optimize()
{
    if (!fit_plane()) {
	return false;
    }

    double ang1 = om1->fmesh->max_tri_angle(fp1, tris1);
    std::cout << "ang1: " << ang1 << "\n";
    om1->fmesh->optimize(tris1, fp1);

    if (om1->fmesh->new_tris.size()) {
	tris1 = om1->fmesh->new_tris;
	// Need to update vertex bboxes after changing triangles!
	std::set<triangle_t>::iterator n_it;
	std::set<overt_t *> tri_verts;
	for (n_it = om1->fmesh->new_tris.begin(); n_it != om1->fmesh->new_tris.end(); n_it++) {
	    for (int i = 0; i < 3; i++) {
		tri_verts.insert(om1->overts[(*n_it).v[i]]);
	    }
	}
	std::set<overt_t *>::iterator o_it;
	for (o_it = tri_verts.begin(); o_it != tri_verts.end(); o_it++) {
	    (*o_it)->update();
	}
    }

    double ang2 = om2->fmesh->max_tri_angle(fp2, tris2);
    std::cout << "ang2: " << ang2 << "\n";
    om2->fmesh->optimize(tris2, fp2);

    if (om2->fmesh->new_tris.size()) {
	tris2 = om2->fmesh->new_tris;
	// Need to update vertex bboxes after changing triangles!
	std::set<triangle_t>::iterator n_it;
	std::set<overt_t *> tri_verts;
	for (n_it = om2->fmesh->new_tris.begin(); n_it != om2->fmesh->new_tris.end(); n_it++) {
	    for (int i = 0; i < 3; i++) {
		tri_verts.insert(om2->overts[(*n_it).v[i]]);
	    }
	}
	std::set<overt_t *>::iterator o_it;
	for (o_it = tri_verts.begin(); o_it != tri_verts.end(); o_it++) {
	    (*o_it)->update();
	}
    }

    if (!overlapping()) {
	state = 2;
    }

    return true;
}

/* If the two polygons in a group are aligned (i.e. every vertex (interior or
 * on the loop) has a nearby vertex in the other polygon) it is possible to
 * replace the triangles in both polygons with a single CDT, rather than having
 * to try to get compatible CDTs independently. */

// TODO - need to preserve the polygon generated by optimize() - being deleted
// right now...
//
// It's not 100% that the polygons from optimize will work, since they may be
// more aggressive about including triangles than they have to be (they're not
// constrained by points relevant to the ovlp groups, and for the initial pass
// we probably don't want that anyway - we may need "repair" behavior
// initially.  We may also still need the fallback to re-building the polygons
// with a tight focus on just the group triangles and minimal additional
// inclusions to get aligned polygons...
bool
polygons_aligned(ovlp_grp &grp)
{
    if (!grp.tris1.size() || !grp.tris2.size()) {
	return false;
    }

    // First, check the sizes - if they're not the same, there's no point going any further.

    // If we're passed all the rejection critieria, they're aligned.
    return true;
}


/* A grouping of overlapping triangles is defined as the set of triangles from
 * two meshes that overlap and are surrounded in both meshes by either
 * non-involved triangles or face boundary edges.  Visually, these are "clusters"
 * of overlapping triangles on the mesh surfaces. */
std::vector<ovlp_grp>
find_ovlp_grps(
	std::set<std::pair<cdt_mesh_t *, cdt_mesh_t *>> &check_pairs
	)
{
    std::vector<ovlp_grp> bins;

    std::map<std::pair<omesh_t *, size_t>, size_t> bin_map;

    std::set<std::pair<cdt_mesh_t *, cdt_mesh_t *>>::iterator cp_it;
    for (cp_it = check_pairs.begin(); cp_it != check_pairs.end(); cp_it++) {
	omesh_t *omesh1 = cp_it->first->omesh;
	omesh_t *omesh2 = cp_it->second->omesh;
	if (!omesh1->intruding_tris.size() || !omesh2->intruding_tris.size()) {
	    continue;
	}
	std::set<size_t> om1_tris = omesh1->intruding_tris;
	while (om1_tris.size()) {
	    size_t t1 = *om1_tris.begin();
triangle_t tri1 = omesh1->fmesh->tris_vect[t1];
	    std::pair<omesh_t *, size_t> key1(omesh1, t1);
	    om1_tris.erase(t1);
	    ON_BoundingBox tri_bb = omesh1->fmesh->tri_bbox(t1);
	    std::set<size_t> ntris = omesh2->tris_search(tri_bb);
	    std::set<size_t>::iterator nt_it;
	    for (nt_it = ntris.begin(); nt_it != ntris.end(); nt_it++) {
		triangle_t tri2 = omesh2->fmesh->tris_vect[*nt_it];

		int real_ovlp = tri_isect(tri1, tri2, 1);
		if (!real_ovlp) continue;

		std::pair<omesh_t *, size_t> key2(omesh2, *nt_it);
		size_t key_id;
		if (bin_map.find(key1) == bin_map.end() && bin_map.find(key2) == bin_map.end()) {
		    // New group
		    ovlp_grp ngrp(key1.first, key2.first);
		    ngrp.add_tri(tri1);
		    ngrp.add_tri(tri2);
		    bins.push_back(ngrp);
		    key_id = bins.size() - 1;
		    bin_map[key1] = key_id;
		    bin_map[key2] = key_id;
		} else {
		    if (bin_map.find(key1) != bin_map.end()) {
			key_id = bin_map[key1];
			bins[key_id].add_tri(tri2);
			bin_map[key2] = key_id;
		    } else {
			key_id = bin_map[key2];
			bins[key_id].add_tri(tri1);
			bin_map[key1] = key_id;
		    }
		}
	    }
	}
    }

    return bins;
}

bool
ovlp_grp::overlapping()
{
    // For now, just do the naive comparison without acceleration
    // structures - if these sets get big it might be worth building
    // rtrees specific to the group tris.

    std::set<triangle_t>::iterator i_it, j_it;
    for (i_it = tris1.begin(); i_it != tris1.end(); i_it++) {
	triangle_t tri1 = *i_it;
	for (j_it = tris2.begin(); j_it != tris2.end(); j_it++) {
	    triangle_t tri2 = *j_it;
	    if (tri_isect(tri1, tri2, 1)) {
		return true;
	    }
	}
    }

    return false;
}

// TODO - probably don't really need a full CDT for this -
// just find the interior edge, the other possible interior
// edge, and if the other one is shorter switch the triangles...
bool
ovlp_grp::pair_realign(int ind)
{
    std::set<triangle_t> &gtris = (ind) ? tris1 : tris2;
    std::set<triangle_t> tris = (ind) ? tris1 : tris2;
    omesh_t *omesh = tris.begin()->m->omesh;

    std::set<long> t_pts;
    std::set<triangle_t>::iterator r_it;
    for (r_it = tris.begin(); r_it != tris.end(); r_it++) {
	for (int i = 0; i < 3; i++) {
	    t_pts.insert(r_it->v[i]);
	}
    }

    // If we don't have exactly 4 vertices from the triangles, we can't do this
    if (t_pts.size() != 4) {
	return false;
    }

    point_t pcenter;
    vect_t pnorm;
    point_t *fpnts = (point_t *)bu_calloc(t_pts.size()+1, sizeof(point_t), "fitting points");
    std::set<long>::iterator p_it;
    int tpind = 0;
    for (p_it = t_pts.begin(); p_it != t_pts.end(); p_it++) {
	ON_3dPoint *p = omesh->fmesh->pnts[*p_it];
	fpnts[tpind][X] = p->x;
	fpnts[tpind][Y] = p->y;
	fpnts[tpind][Z] = p->z;
	tpind++;
    }
    if (bn_fit_plane(&pcenter, &pnorm, t_pts.size(), fpnts)) {
	std::cout << "fitting plane failed!\n";
    }
    bu_free(fpnts, "fitting points");

    ON_Plane fit_plane(pcenter, pnorm);

    // Build our polygon out of the two triangles.
    //
    // First step, add the 2D projection of the triangle vertices to the
    // polygon data structure.
    cpolygon_t *polygon = new cpolygon_t;
    polygon->pdir = fit_plane.Normal();
    polygon->tplane = fit_plane;
    for (p_it = t_pts.begin(); p_it != t_pts.end(); p_it++) {
	double u, v;
	long pind = *p_it;
	ON_3dPoint *p = omesh->fmesh->pnts[pind];
	fit_plane.ClosestPointTo(*p, &u, &v);
	std::pair<double, double> proj_2d;
	proj_2d.first = u;
	proj_2d.second = v;
	polygon->pnts_2d.push_back(proj_2d);
	polygon->p2o[polygon->pnts_2d.size() - 1] = pind;
	polygon->o2p[pind] = polygon->pnts_2d.size() - 1;
    }

    // Initialize the polygon edges with one of the triangles.
    triangle_t tri1 = *tris.begin();
    tris.erase(tris.begin());
    struct edge2d_t e1(polygon->o2p[tri1.v[0]], polygon->o2p[tri1.v[1]]);
    struct edge2d_t e2(polygon->o2p[tri1.v[1]], polygon->o2p[tri1.v[2]]);
    struct edge2d_t e3(polygon->o2p[tri1.v[2]], polygon->o2p[tri1.v[0]]);
    polygon->add_edge(e1);
    polygon->add_edge(e2);
    polygon->add_edge(e3);

    std::set<uedge_t> uedges;
    uedges.insert(uedge_t(tri1.v[0], tri1.v[1]));
    uedges.insert(uedge_t(tri1.v[1], tri1.v[2]));
    uedges.insert(uedge_t(tri1.v[2], tri1.v[0]));

    // Grow the polygon with the other triangle.
    std::set<uedge_t> new_edges;
    std::set<uedge_t> shared_edges;
    triangle_t tri2 = *tris.begin();
    tris.erase(tris.begin());
    for (int i = 0; i < 3; i++) {
	int v1 = i;
	int v2 = (i < 2) ? i + 1 : 0;
	uedge_t ue1(tri2.v[v1], tri2.v[v2]);
	if (uedges.find(ue1) == uedges.end()) {
	    new_edges.insert(ue1);
	} else {
	    shared_edges.insert(ue1);
	}
    }
    polygon->replace_edges(new_edges, shared_edges);

    polygon->cdt();

    omesh->fmesh->tri_remove(tri1);
    omesh->fmesh->tri_remove(tri2);

    gtris.clear();
    std::set<triangle_t>::iterator v_it;
    for (v_it = polygon->tris.begin(); v_it != polygon->tris.end(); v_it++) {
	triangle_t vt = *v_it;
	vt.m = omesh->fmesh;
	orient_tri(*omesh->fmesh, vt);
	omesh->fmesh->tri_add(vt);
	gtris.insert(vt);
    }

    return true;
}

bool
ovlp_grp::pairs_realign()
{
    // If we don't have exactly 2 triangles in each of the grp sets, we can't do this
    if (tris1.size() != 2 || tris2.size() != 2) {
	return false;
    }

    bool p1 = pair_realign(0);
    bool p2 = pair_realign(1);

    if (!overlapping()) {
	state = 2;
    }

    return (p1 && p2);
}


void
resolve_ovlp_grps(std::set<std::pair<cdt_mesh_t *, cdt_mesh_t *>> &check_pairs)
{
    std::vector<ovlp_grp> bins;

   size_t bin_cnt = INT_MAX;
   int cycle_cnt = 0;

   while (bin_cnt && cycle_cnt < 5) {
       bins = find_ovlp_grps(check_pairs);
       bin_cnt = bins.size();
#if 1
       for (size_t i = 0; i < bins.size(); i++) {
	   struct bu_vls pname = BU_VLS_INIT_ZERO;
	   bu_vls_sprintf(&pname, "group_%zu", i);
	   bins[i].plot(bu_vls_cstr(&pname));
	   bu_vls_free(&pname);
       }
#endif

       for (size_t i = 0; i < bins.size(); i++) {
	   bool v1 = bins[i].om1->fmesh->valid(1);
	   bool v2 = bins[i].om2->fmesh->valid(1);

	   if (!v1 || !v2) {
	       std::cout << "Starting bin processing with invalid inputs!\n";
	   }


	   if (!bins[i].pairs_realign()) {
	       bins[i].optimize();
	   }
       }

       bool have_invalid = false;
       bool have_unresolved = false;
       for (size_t i = 0; i < bins.size(); i++) {
	   if (bins[i].state == -1) {
	       have_invalid = true;
	   }
	   if (bins[i].state == 1) {
	       have_unresolved = true;
	   }
       }
       if (!have_invalid && !have_unresolved) {
	   break;
       }

       omesh_ovlps(check_pairs, 1);
       plot_active_omeshes(check_pairs);
       cycle_cnt++;
   }

   plot_active_omeshes(check_pairs);
}

/** @} */

// Local Variables:
// mode: C++
// tab-width: 8
// c-basic-offset: 4
// indent-tabs-mode: t
// c-file-style: "stroustrup"
// End:
// ex: shiftwidth=4 tabstop=8

