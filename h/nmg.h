/*
 *			N M G . H
 *
 *  Author -
 *	Lee A. Butler
 *  
 *  Source -
 *	The U. S. Army Research Laboratory
 *	Aberdeen Proving Ground, Maryland  21005-5066
 *  
 *  Copyright Notice -
 *	This software is Copyright (C) 1993 by the United States Army.
 *	All rights reserved.
 *
 *  Definition of data structures for "Non-Manifold Geometry Modelling."
 *  Developed from "Non-Manifold Geometric Boundary Modeling" by 
 *  Kevin Weiler, 5/7/87 (SIGGraph 1989 Course #20 Notes)
 *
 *  Include Sequencing -
 *	#include <stdio.h>
 *	#include <math.h>
 *	#include "machine.h"	/_* For fastf_t definition on this machine *_/
 *	#include "vmath.h"	/_* For vect_t definition *_/
 *	#include "rtlist.h"	/_* OPTIONAL, auto-included by raytrace.h *_/
 *	#include "rtstring.h"	/_* OPTIONAL, auto-included by raytrace.h *_/
 *	#include "nmg.h"
 *	#include "raytrace.h"
 *	#include "nurb.h"	/_* OPTIONAL, follows raytrace.h when used *_/
 *
 *  Libraries Used -
 *	LIBRT LIBRT_LIBES -lm -lc
 *
 *  $Header$
 */
#ifndef NMG_H
#define NMG_H seen

/* make sure all the prerequisite include files have been included
 */
#ifndef MACHINE_H
#include "machine.h"
#endif

#ifndef VMATH_H
#include "vmath.h"
#endif

#ifndef SEEN_RTLIST_H
#include "rtlist.h"
#endif

#ifndef NULL
#define NULL 0
#endif

#define	NMG_EXTERN(type_and_name,args)	RT_EXTERN(type_and_name,args)


#define DEBUG_PL_ANIM	0x00000001	/* 1 mged animated evaluation */
#define DEBUG_PL_SLOW	0x00000002	/* 2 add delays to animation */
#define DEBUG_GRAPHCL	0x00000004	/* 3 graphic classification */
#define DEBUG_PL_LOOP	0x00000008	/* 4 loop class (needs GRAPHCL) */
#define DEBUG_PLOTEM	0x00000010	/* 5 make plots in debugged routines (needs other flags set too) */
#define DEBUG_POLYSECT	0x00000020	/* 6 nmg_inter: face intersection */
#define DEBUG_VERIFY	0x00000040	/* 7 nmg_vshell() frequently, verify health */
#define DEBUG_BOOL	0x00000080	/* 8 nmg_bool:  */
#define DEBUG_CLASSIFY	0x00000100	/* 9 nmg_class: */
#define DEBUG_BOOLEVAL	0x00000200	/* 10 nmg_eval: what to retain */
#define DEBUG_UNUSED	0x00000400	/* 11 UNUSED */
#define DEBUG_MESH	0x00000800	/* 12 nmg_mesh: describe edge search */
#define DEBUG_MESH_EU	0x00001000	/* 13 nmg_mesh: list edges meshed */
#define DEBUG_POLYTO	0x00002000	/* 14 nmg_misc: polytonmg */
#define DEBUG_LABEL_PTS 0x00004000	/* 15 label points in plot files */
#define DEBUG_INS	0x00008000	/* 16 nmg_tbl table insert */
#define DEBUG_NMGRT	0x00010000	/* 17 ray tracing */
#define DEBUG_FINDEU	0x00020000	/* 18 nmg_mod: nmg_findeu() */
#define DEBUG_CMFACE	0x00040000	/* 19 nmg_mod: nmg_cmface() */
#define DEBUG_CUTLOOP	0x00080000	/* 024 nmg_mod: nmg_cut_loop */
#define DEBUG_VU_SORT	0x00100000	/* 025 nmg_fcut: coincident vu sort */
#define DEBUG_FCUT	0x00200000	/* 026 nmg_fcut: face cutter */
#define DEBUG_RT_SEGS	0x00400000	/* 027 nmg_rt_segs: */
#define DEBUG_RT_ISECT	0x00800000	/* 028 nmg_rt_isect: */
#define DEBUG_TRI	0x01000000	/* 029 nmg_tri */
#define NMG_DEBUG_FORMAT \
"\020\031TRIANG\030RT_ISECT\
\027RT_SEGS\026FCUT\025VU_SORT\024CUTLOOP\023CMFACE\022FINDEU\021RT_ISECT\020TBL_INS\
\017LABEL_PTS\016POLYTO\015MESH_EU\014MESH\013GRAZING\012BOOLEVAL\011CLASSIFY\
\010BOOL\7VERIFY\6POLYSECT\5PLOTEM\4PL_LOOP\3GRAPHCL\2PL_SLOW\1PL_ANIM"

/*
 *  Macros for providing function prototypes, regardless of whether
 *  the compiler understands them or not.
 *  It is vital that the argument list given for "args" be enclosed
 *  in parens.
 *  The setting of USE_PROTOTYPES is done in machine.h
 */
#if USE_PROTOTYPES
#	define	NMG_ARGS(args)			args
#else
#	define	NMG_ARGS(args)			()
#endif

/* Boolean operations */
#define NMG_BOOL_SUB 1		/* subtraction */
#define NMG_BOOL_ADD 2		/* addition/union */
#define NMG_BOOL_ISECT 4	/* intsersection */

/* Boolean classifications */
#define NMG_CLASS_AinB		0
#define NMG_CLASS_AonBshared	1
#define NMG_CLASS_AonBanti	2
#define NMG_CLASS_AoutB		3
#define NMG_CLASS_BinA		4
#define NMG_CLASS_BonAshared	5
#define NMG_CLASS_BonAanti	6
#define NMG_CLASS_BoutA		7

/* orientations available.  All topological elements are orientable. */
#define OT_NONE     0    /* no orientation */
#define OT_SAME     1    /* orientation same */
#define OT_OPPOSITE 2    /* orientation opposite */
#define OT_UNSPEC   3    /* orientation unspecified */
#define OT_BOOLPLACE 4   /* object is intermediate data for boolean ops */



/* support for pointer tables.  Our table is currently un-ordered, and is
 * merely a list of objects.  The support routine nmg_tbl manipulates the
 * list structure for you.  Objects to be referenced (inserted, deleted,
 * searched for) are passed as a "pointer to long" to the support routine.
 */
#define TBL_INIT 0	/* initialize list pointer struct & get storage */
#define TBL_INS	 1	/* insert an item (long *) into a list */
#define TBL_LOC  2	/* locate a (long *) in an existing list */
#define TBL_FREE 3	/* deallocate buffer associated with a list */
#define TBL_RST	 4	/* empty a list, but keep storage on hand */
#define TBL_CAT  5	/* catenate one list onto another */
#define TBL_RM	 6	/* remove all occurrences of an item from a list */
#define TBL_INS_UNIQUE	 7	/* insert item into list, if not present */

struct nmg_ptbl {
	long	magic;	/* magic */
	int	end;	/* index into buffer of first available location */
	int	blen;	/* # of (long *)'s worth of storage at *buffer */
	long  **buffer;	/* data storage area */
};
#define NMG_PTBL_MAGIC		0x7074626c		/* "ptbl" */
#define NMG_CK_PTBL(_p)		NMG_CKMAG(_p, NMG_PTBL_MAGIC, "nmg_ptbl")

/* For those routines that have to "peek" a little */
#define NMG_TBL_BASEADDR(p)	((p)->buffer)
#define NMG_TBL_END(p)		((p)->end)
#define NMG_TBL_GET(p,i)	((p)->buffer[(i)])

/*
 *  Magic Numbers.
 */
#define NMG_MODEL_MAGIC 	12121212
#define NMG_MODEL_A_MAGIC	0x68652062
#define NMG_REGION_MAGIC	23232323
#define NMG_REGION_A_MAGIC	0x696e6720
#define NMG_SHELL_MAGIC 	71077345	/* shell oil */
#define NMG_SHELL_A_MAGIC	0x65207761
#define NMG_FACE_MAGIC		45454545
#define NMG_FACE_G_MAGIC	0x726b6e65
#define NMG_FACEUSE_MAGIC	56565656
#define NMG_FACEUSE_A_MAGIC	0x20476f64
#define NMG_LOOP_MAGIC		67676767
#define NMG_LOOP_G_MAGIC	0x6420224c
#define NMG_LOOPUSE_MAGIC	78787878
#define NMG_LOOPUSE_A_MAGIC	0x68657265
#define NMG_EDGE_MAGIC		33333333
#define NMG_EDGE_G_MAGIC	0x6c696768
#define NMG_EDGEUSE_MAGIC	90909090
#define NMG_EDGEUSE_A_MAGIC	0x20416e64
#define NMG_VERTEX_MAGIC	123123
#define NMG_VERTEX_G_MAGIC	727737707
#define NMG_VERTEXUSE_MAGIC	12341234
#define NMG_VERTEXUSE_A_MAGIC	0x69676874

/* macros to check/validate a structure pointer
 */
#define NMG_CKMAG(_ptr, _magic, _str)	\
	if( !(_ptr) || *((long *)(_ptr)) != (_magic) )  { \
		rt_badmagic( (long *)(_ptr), _magic, _str, __FILE__, __LINE__ ); \
	}

#define NMG_CK_MODEL(_p)	NMG_CKMAG(_p, NMG_MODEL_MAGIC, "model")
#define NMG_CK_MODEL_A(_p)	NMG_CKMAG(_p, NMG_MODEL_A_MAGIC, "model_a")
#define NMG_CK_REGION(_p)	NMG_CKMAG(_p, NMG_REGION_MAGIC, "region")
#define NMG_CK_REGION_A(_p)	NMG_CKMAG(_p, NMG_REGION_A_MAGIC, "region_a")
#define NMG_CK_SHELL(_p)	NMG_CKMAG(_p, NMG_SHELL_MAGIC, "shell")
#define NMG_CK_SHELL_A(_p)	NMG_CKMAG(_p, NMG_SHELL_A_MAGIC, "shell_a")
#define NMG_CK_FACE(_p)		NMG_CKMAG(_p, NMG_FACE_MAGIC, "face")
#define NMG_CK_FACE_G(_p)	NMG_CKMAG(_p, NMG_FACE_G_MAGIC, "face_g")
#define NMG_CK_FACEUSE(_p)	NMG_CKMAG(_p, NMG_FACEUSE_MAGIC, "faceuse")
#define NMG_CK_FACEUSE_A(_p)	NMG_CKMAG(_p, NMG_FACEUSE_A_MAGIC, "faceuse_a")
#define NMG_CK_LOOP(_p)		NMG_CKMAG(_p, NMG_LOOP_MAGIC, "loop")
#define NMG_CK_LOOP_G(_p)	NMG_CKMAG(_p, NMG_LOOP_G_MAGIC, "loop_g")
#define NMG_CK_LOOPUSE(_p)	NMG_CKMAG(_p, NMG_LOOPUSE_MAGIC, "loopuse")
#define NMG_CK_LOOPUSE_A(_p)	NMG_CKMAG(_p, NMG_LOOPUSE_A_MAGIC, "loopuse_a")
#define NMG_CK_EDGE(_p)		NMG_CKMAG(_p, NMG_EDGE_MAGIC, "edge")
#define NMG_CK_EDGE_G(_p)	NMG_CKMAG(_p, NMG_EDGE_G_MAGIC, "edge_g")
#define NMG_CK_EDGEUSE(_p)	NMG_CKMAG(_p, NMG_EDGEUSE_MAGIC, "edgeuse")
#define NMG_CK_EDGEUSE_A(_p)	NMG_CKMAG(_p, NMG_EDGEUSE_A_MAGIC, "edgeuse_a")
#define NMG_CK_VERTEX(_p)	NMG_CKMAG(_p, NMG_VERTEX_MAGIC, "vertex")
#define NMG_CK_VERTEX_G(_p)	NMG_CKMAG(_p, NMG_VERTEX_G_MAGIC, "vertex_g")
#define NMG_CK_VERTEXUSE(_p)	NMG_CKMAG(_p, NMG_VERTEXUSE_MAGIC, "vertexuse")
#define NMG_CK_VERTEXUSE_A(_p)	NMG_CKMAG(_p, NMG_VERTEXUSE_A_MAGIC, "vertexuse_a")
#define NMG_CK_LIST(_p)		NMG_CKMAG(_p, RT_LIST_HEAD_MAGIC, "rt_list")

#define NMG_TEST_LOOPUSE(_p) \
	if (!(_p)->up.magic_p || !(_p)->l.forw || !(_p)->l.back || \
	    !(_p)->l_p || !(_p)->lumate_p || !(_p)->down.magic_p) { \
		rt_log("at %d in %s BAD loopuse member pointer\n", \
			__LINE__, __FILE__); nmg_pr_lu(_p, (char *)NULL); \
			rt_bomb("Null pointer\n"); }

#define NMG_TEST_EDGEUSE(_p) \
	if (!(_p)->l.forw || !(_p)->l.back || !(_p)->eumate_p || \
	    !(_p)->radial_p || !(_p)->e_p || !(_p)->vu_p || \
	    !(_p)->up.magic_p ) { \
		rt_log("in %s at %d Bad edgeuse member pointer\n",\
			 __FILE__, __LINE__);  nmg_pr_eu(_p, (char *)NULL); \
			rt_bomb("Null pointer\n"); \
	} else if ((_p)->vu_p->up.eu_p != (_p) || \
	(_p)->eumate_p->vu_p->up.eu_p != (_p)->eumate_p) {\
	    	rt_log("in %s at %d edgeuse lost vertexuse\n",\
	    		 __FILE__, __LINE__); rt_bomb("bye");}

/*
 *	N O T I C E !
 *
 *	We rely on the fact that the first long in a struct is the magic
 *	number (which is used to identify the struct type).
 *	This may be either a long, or an rt_list structure, which
 *	starts with a magic number.
 *
 *	To these ends, there is a standard ordering for fields in "object-use"
 *	structures.  That ordering is:
 *		1) magic number, or rt_list structure
 *		2) pointer to parent
 *		5) pointer to mate
 *		6) pointer to geometry
 *		7) pointer to attributes
 *		8) pointer to child(ren)
 */


/*
 *			M O D E L
 */
struct model {
	long			magic;
	struct model_a		*ma_p;
	struct rt_list		r_hd;	/* list of regions */
	long			index;	/* struct # in this model */
	char			*manifolds; /*  structure 1-3manifold table */
	long			maxindex; /* # of structs so far */
};

struct model_a {
	long			magic;
	long			index;	/* struct # in this model */
};

/*
 *			R E G I O N
 */
struct nmgregion {
	struct rt_list		l;	/* regions, in model's r_hd list */
	struct model   		*m_p;	/* owning model */
	struct nmgregion_a	*ra_p;	/* attributes */
	struct rt_list		s_hd;	/* list of shells in region */
	long			index;	/* struct # in this model */
};

struct nmgregion_a {
	long			magic;
	point_t			min_pt;	/* minimums of bounding box */
	point_t			max_pt;	/* maximums of bounding box */
	long			index;	/* struct # in this model */
};

/*
 *			S H E L L
 *
 *  When a shell encloses volume, it's done entirely by the list of faceuses.
 *
 *  The wire loopuses (each of which heads a list of edges) define a
 *  set of connected line segments which form a closed path, but do not
 *  enclose either volume or surface area.
 *
 *  The wire edgeuses are disconnected line segments.
 *  There is a special interpetation to the eu_hd list of wire edgeuses.
 *  Unlike edgeuses seen in loops, the eu_hd list contains eu1, eu1mate,
 *  eu2, eu2mate, ..., where each edgeuse and it's mate comprise a
 *  *non-connected* "wire" edge which starts at eu1->vu_p->v_p and ends
 *  at eu1mate->vu_p->v_p.  There is no relationship between the pairs
 *  of edgeuses at all, other than that they all live on the same linked
 *  list.
 */
struct shell {
	struct rt_list		l;	/* shells, in region's s_hd list */
	struct nmgregion	*r_p;	/* owning region */
	struct shell_a		*sa_p;	/* attribs */

	struct rt_list		fu_hd;	/* list of face uses in shell */
	struct rt_list		lu_hd;	/* wire loopuses (edge groups) */
	struct rt_list		eu_hd;	/* wire list (shell has wires) */
	struct vertexuse	*vu_p;	/* internal ptr to single vertexuse */
	long			index;	/* struct # in this model */
};

struct shell_a {
	long			magic;
	point_t			min_pt;	/* minimums of bounding box */
	point_t			max_pt;	/* maximums of bounding box */
	long			index;	/* struct # in this model */
};

/*
 *			F A C E
 *
 *  Note: there will always be exactly two faceuse's using a face.
 *  To find them, go up fu_p for one, then across fumate_p to other.
 */
struct face {
	struct rt_list		l;	/* faces in face_g's f_hd list */
	struct faceuse		*fu_p;	/* Ptr up to one use of this face */
	struct face_g		*fg_p;	/* geometry */
	int			flip;	/* !0 ==> flip normal of fg */
	/* These might be better stored in a face_a (not faceuse_a!) */
	/* These are not stored on disk */
	point_t			min_pt;	/* minimums of bounding box */
	point_t			max_pt;	/* maximums of bounding box */
	long			index;	/* struct # in this model */
};

struct face_g {
	long			magic;
	struct rt_list		f_hd;	/* list of faces sharing this surface */
	plane_t			N;	/* Plane equation (incl normal) */
#if 0
/* XXX This moves to struct face */
	point_t			min_pt;	/* minimums of bounding box */
	point_t			max_pt;	/* maximums of bounding box */
#endif
	long			index;	/* struct # in this model */
};

struct faceuse {
	struct rt_list		l;	/* fu's, in shell's fu_hd list */
	struct shell		*s_p;	/* owning shell */
	struct faceuse		*fumate_p;    /* opposite side of face */
	int			orientation;  /* rel to face geom defn */
	struct face		*f_p;	/* face definition and attributes */
	struct faceuse_a	*fua_p;	/* attributess */
	struct rt_list		lu_hd;	/* list of loops in face-use */
	long			index;	/* struct # in this model */
};

struct faceuse_a {
	long			magic;
	long			index;	/* struct # in this model */
};

/* Returns a 3-tuple (vect_t), given faceuse and state of flip flags */
#define NMG_GET_FU_NORMAL(_N, _fu)	{ \
	register CONST struct faceuse	*_fu1 = (_fu); \
	register CONST struct face_g	*_fg; \
	NMG_CK_FACEUSE(_fu1); \
	NMG_CK_FACE(_fu1->f_p); \
	_fg = _fu1->f_p->fg_p; \
	NMG_CK_FACE_G(_fg); \
	if( (_fu1->orientation != OT_SAME) != (_fu1->f_p->flip != 0) )  { \
		VREVERSE( _N, _fg->N); \
	} else { \
		VMOVE( _N, _fg->N ); \
	} }

/* Returns a 4-tuple (plane_t), given faceuse and state of flip flags */
#define NMG_GET_FU_PLANE(_N, _fu)	{ \
	register CONST struct faceuse	*_fu1 = (_fu); \
	register CONST struct face_g	*_fg; \
	NMG_CK_FACEUSE(_fu1); \
	NMG_CK_FACE(_fu1->f_p); \
	_fg = _fu1->f_p->fg_p; \
	NMG_CK_FACE_G(_fg); \
	if( (_fu1->orientation != OT_SAME) != (_fu1->f_p->flip != 0) )  { \
		HREVERSE( _N, _fg->N); \
	} else { \
		HMOVE( _N, _fg->N ); \
	} }

/*
 *			L O O P
 *
 *  To find all the uses of this loop, use lu_p for one loopuse,
 *  then go down and find an edge,
 *  then wander around either eumate_p or radial_p from there.
 *
 *  Normally, down_hd heads a doubly linked list of edgeuses.
 *  But, before using it, check RT_LIST_FIRST_MAGIC(&lu->down_hd)
 *  for the magic number type.
 *  If this is a self-loop on a single vertexuse, then get the vertex pointer
 *  with vu = RT_LIST_FIRST(vertexuse, &lu->down_hd)
 *
 *  This is an especially dangerous storage efficiency measure ("hack"),
 *  because the list that the vertexuse structure belongs to is headed,
 *  not by a superior element type, but by the vertex structure.
 *  When a loopuse needs to point down to a vertexuse, rip off the
 *  forw pointer.  Take careful note that this is just a pointer,
 *  **not** the head of a linked list (single, double, or otherwise)!
 *  Exercise great care!
 *
 *  The edges of an exterior (OT_SAME) loop occur in counter-clockwise
 *  order, as viewed from the normalward side (outside).
 */
#define RT_LIST_SET_DOWN_TO_VERT(_hp,_vu)	{ \
	(_hp)->forw = &((_vu)->l); (_hp)->back = (struct rt_list *)NULL; }

struct loop {
	long			magic;
	struct loopuse		*lu_p;	/* Ptr to one use of this loop */
	struct loop_g		*lg_p;  /* Geometry */
	long			index;	/* struct # in this model */
};

struct loop_g {
	long			magic;
	point_t			min_pt;	/* minimums of bounding box */
	point_t			max_pt;	/* maximums of bounding box */
	long			index;	/* struct # in this model */
};

struct loopuse {
	struct rt_list		l;	/* lu's, in fu's lu_hd, or shell's lu_hd */
	union {
		struct faceuse  *fu_p;	/* owning face-use */
		struct shell	*s_p;
		long		*magic_p;
	} up;
	struct loopuse		*lumate_p; /* loopuse on other side of face */
	int			orientation;  /* OT_SAME=outside loop */
	struct loop		*l_p;	/* loop definition and attributes */
	struct loopuse_a	*lua_p;	/* attributes */
	struct rt_list		down_hd; /* eu list or vu pointer */
	long			index;	/* struct # in this model */
};

struct loopuse_a {
	long			magic;
	long			index;	/* struct # in this model */
};

/*
 *			E D G E
 *
 *  To find all edgeuses of an edge, use eu_p to get an arbitrary edgeuse,
 *  then wander around either eumate_p or radial_p from there.
 *
 *  Only the first vertex of an edge is kept in an edgeuse (eu->vu_p).
 *  The other vertex can be found by either eu->eumate_p->vu_p or
 *  by RT_LIST_PNEXT_CIRC(edgeuse,eu)->vu_p.  Note that the first
 *  form gives a vertexuse in the faceuse of *opposite* orientation,
 *  while the second form gives a vertexuse in the faceuse of the correct
 *  orientation.  If going on to the vertex (vu_p->v_p), both forms
 *  are identical.
 */
struct edge {
	long			magic;
	struct edgeuse		*eu_p;	/* Ptr to one use of this edge */
	struct edge_g		*eg_p;  /* geometry */
	long			is_real;/* artifact or modeled edge */
	long			index;	/* struct # in this model */
};

struct edge_g {
	long			magic;
	long			usage;	/* # of uses of this geometry */
	point_t			e_pt;	/* parametric equation of the edge */
	vect_t			e_dir;
	long			index;	/* struct # in this model */
};

struct edgeuse {
	struct rt_list		l;	/* cw/ccw edges in loop or wire edges in shell */
	union {
		struct loopuse	*lu_p;
		struct shell	*s_p;
		long	        *magic_p; /* for those times when we're not sure */
	} up;
	struct edgeuse		*eumate_p;  /* eu on other face or other end of wire*/
	struct edgeuse		*radial_p;  /* eu on radially adj. fu (null if wire)*/
	struct edge		*e_p;	    /* edge definition and attributes */
	struct edgeuse_a	*eua_p;	    /* parametric space geom */
	int	  		orientation;/* compared to geom (null if wire) */
	struct vertexuse	*vu_p;	    /* first vu of eu in this orient */
	long			index;	/* struct # in this model */
};

struct edgeuse_a {
	long			magic;
	long			index;	/* struct # in this model */
};

/*
 *			V E R T E X
 *
 *  The vertex and vertexuse structures are connected in a way different
 *  from the superior kinds of topology elements.
 *  The vertex structure heads a linked list that all vertexuse's
 *  that use the vertex are linked onto.
 */
struct vertex {
	long			magic;
	struct rt_list		vu_hd;	/* heads list of vu's of this vertex */
	struct vertex_g		*vg_p;	/* geometry */
	long			index;	/* struct # in this model */
};

struct vertex_g {
	long			magic;
	point_t			coord;	/* coordinates of vertex in space */
	long			index;	/* struct # in this model */
};

struct vertexuse {
	struct rt_list		l;	/* list of all vu's on a vertex */
	union {
		struct shell	*s_p;	/* no fu's or eu's on shell */
		struct loopuse	*lu_p;	/* loopuse contains single vertex */
		struct edgeuse	*eu_p;	/* eu causing this vu */
		long		*magic_p; /* for those times when we're not sure */
	} up;
	struct vertex		*v_p;	/* vertex definition and attributes */
	struct vertexuse_a	*vua_p;	/* Attributes */
	long			index;	/* struct # in this model */
};

struct vertexuse_a {
	long			magic;
	vect_t			N;	/* (opt) surface Normal at vertexuse */
	long			index;	/* struct # in this model */
};


/*
 * storage allocation and de-allocation support
 *  Primarily used by nmg_mk.c
 */

#if __STDC__ && !alliant && !apollo
#   define NMG_GETSTRUCT(p,str) \
	p = (struct str *)rt_calloc(1,sizeof(struct str), "getstruct " #str)
#else
#   define NMG_GETSTRUCT(p,str) \
	p = (struct str *)rt_calloc(1,sizeof(struct str), "getstruct str")
#endif

#if __STDC__ && !alliant && !apollo
# define NMG_FREESTRUCT(ptr, str) \
	{ bzero((char *)(ptr), sizeof(struct str)); \
	  rt_free((char *)(ptr), "freestruct " #str); }
#else
# define NMG_FREESTRUCT(ptr, str) \
	{ bzero((char *)(ptr), sizeof(struct str)); \
	  rt_free((char *)(ptr), "freestruct str"); }
#endif

#if defined(SYSV) && !defined(bzero)
#	define bzero(str,n)		memset( str, '\0', n )
#	define bcopy(from,to,count)	memcpy( to, from, count )
#endif
/*
 *  Macros to create and destroy storage for the NMG data structures.
 *  Since nmg_mk.c is the only source file which should perform these
 *  most fundamental operations, the macros do not belong in nmg.h
 *  In particular, application code should NEVER do these things.
 *  Any need to do so should be handled by extending nmg_mk.c
 */
#define NMG_INCR_INDEX(_p,_m)	\
	NMG_CK_MODEL(_m); (_p)->index = ((_m)->maxindex)++

#define GET_MODEL_A(p,m)    {NMG_GETSTRUCT(p, model_a); NMG_INCR_INDEX(p,m);}
#define GET_REGION(p,m)	    {NMG_GETSTRUCT(p, nmgregion); NMG_INCR_INDEX(p,m);}
#define GET_REGION_A(p,m)   {NMG_GETSTRUCT(p, nmgregion_a); NMG_INCR_INDEX(p,m);}
#define GET_SHELL(p,m)	    {NMG_GETSTRUCT(p, shell); NMG_INCR_INDEX(p,m);}
#define GET_SHELL_A(p,m)    {NMG_GETSTRUCT(p, shell_a); NMG_INCR_INDEX(p,m);}
#define GET_FACE(p,m)	    {NMG_GETSTRUCT(p, face); NMG_INCR_INDEX(p,m);}
#define GET_FACE_G(p,m)	    {NMG_GETSTRUCT(p, face_g); NMG_INCR_INDEX(p,m);}
#define GET_FACEUSE(p,m)    {NMG_GETSTRUCT(p, faceuse); NMG_INCR_INDEX(p,m);}
#define GET_FACEUSE_A(p,m)  {NMG_GETSTRUCT(p, faceuse_a); NMG_INCR_INDEX(p,m);}
#define GET_LOOP(p,m)	    {NMG_GETSTRUCT(p, loop); NMG_INCR_INDEX(p,m);}
#define GET_LOOP_G(p,m)	    {NMG_GETSTRUCT(p, loop_g); NMG_INCR_INDEX(p,m);}
#define GET_LOOPUSE(p,m)    {NMG_GETSTRUCT(p, loopuse); NMG_INCR_INDEX(p,m);}
#define GET_LOOPUSE_A(p,m)  {NMG_GETSTRUCT(p, loopuse_a); NMG_INCR_INDEX(p,m);}
#define GET_EDGE(p,m)	    {NMG_GETSTRUCT(p, edge); NMG_INCR_INDEX(p,m);}
#define GET_EDGE_G(p,m)	    {NMG_GETSTRUCT(p, edge_g); (p)->usage = 1; NMG_INCR_INDEX(p,m);}
#define GET_EDGEUSE(p,m)    {NMG_GETSTRUCT(p, edgeuse); NMG_INCR_INDEX(p,m);}
#define GET_EDGEUSE_A(p,m)  {NMG_GETSTRUCT(p, edgeuse_a); NMG_INCR_INDEX(p,m);}
#define GET_VERTEX(p,m)	    {NMG_GETSTRUCT(p, vertex); NMG_INCR_INDEX(p,m);}
#define GET_VERTEX_G(p,m)   {NMG_GETSTRUCT(p, vertex_g); NMG_INCR_INDEX(p,m);}
#define GET_VERTEXUSE(p,m)  {NMG_GETSTRUCT(p, vertexuse); NMG_INCR_INDEX(p,m);}
#define GET_VERTEXUSE_A(p,m) {NMG_GETSTRUCT(p, vertexuse_a); NMG_INCR_INDEX(p,m);}

#define FREE_MODEL(p)	    NMG_FREESTRUCT(p, model)
#define FREE_MODEL_A(p)	    NMG_FREESTRUCT(p, model_a)
#define FREE_REGION(p)	    NMG_FREESTRUCT(p, nmgregion)
#define FREE_REGION_A(p)    NMG_FREESTRUCT(p, nmgregion_a)
#define FREE_SHELL(p)	    NMG_FREESTRUCT(p, shell)
#define FREE_SHELL_A(p)	    NMG_FREESTRUCT(p, shell_a)
#define FREE_FACE(p)	    NMG_FREESTRUCT(p, face)
#define FREE_FACE_G(p)	    NMG_FREESTRUCT(p, face_g)
#define FREE_FACEUSE(p)	    NMG_FREESTRUCT(p, faceuse)
#define FREE_FACEUSE_A(p)   NMG_FREESTRUCT(p, faceuse_a)
#define FREE_LOOP(p)	    NMG_FREESTRUCT(p, loop)
#define FREE_LOOP_G(p)	    NMG_FREESTRUCT(p, loop_g)
#define FREE_LOOPUSE(p)	    NMG_FREESTRUCT(p, loopuse)
#define FREE_LOOPUSE_A(p)   NMG_FREESTRUCT(p, loopuse_a)
#define FREE_EDGE(p)	    NMG_FREESTRUCT(p, edge)
#define FREE_EDGE_G(p)	    if (--((p)->usage) <= 0)  NMG_FREESTRUCT(p, edge_g)
#define FREE_EDGEUSE(p)	    NMG_FREESTRUCT(p, edgeuse)
#define FREE_EDGEUSE_A(p)   NMG_FREESTRUCT(p, edgeuse_a)
#define FREE_VERTEX(p)	    NMG_FREESTRUCT(p, vertex)
#define FREE_VERTEX_G(p)    NMG_FREESTRUCT(p, vertex_g)
#define FREE_VERTEXUSE(p)   NMG_FREESTRUCT(p, vertexuse)
#define FREE_VERTEXUSE_A(p) NMG_FREESTRUCT(p, vertexuse_a)

/* two edges share same vertices */
#define EDGESADJ(_e1, _e2) (((_e1)->vu_p->v_p == (_e2)->vu_p->v_p && \
		 (_e1)->eumate_p->vu_p->v_p == (_e2)->eumate_p->vu_p->v_p) || \
		 ((_e1)->vu_p->v_p == (_e2)->eumate_p->vu_p->v_p && \
		 (_e1)->eumate_p->vu_p->v_p == (_e2)->vu_p->v_p ) )

/* Used by nmg_class.c */
#define EUPRINT(_s, _eu)	nmg_euprint( (_s), (_eu) )
#define PLPRINT(_s, _pl) rt_log("%s %gx + %gy + %gz = %g\n", (_s), \
	(_pl)[0], (_pl)[1], (_pl)[2], (_pl)[3])


struct nmg_boolstruct {
	struct nmg_ptbl	ilist;		/* vertexuses on intersection line */
	fastf_t		tol;
	point_t		pt;		/* line of intersection */
	vect_t		dir;
	int		coplanar;
	char		*vertlist;
	int		vlsize;
	struct model	*model;
};

#define PREEXIST 1
#define NEWEXIST 2


#define VU_PREEXISTS(_bs, _vu) { chkidxlist((_bs), (_vu)); \
	(_bs)->vertlist[(_vu)->index] = PREEXIST; }

#define VU_NEW(_bs, _vu) { chkidxlist((_bs), (_vu)); \
	(_bs)->vertlist[(_vu)->index] = NEWEXIST; }


struct nmg_struct_counts {
	/* Actual structure counts (Xuse, then X) */
	long	model;
	long	model_a;
	long	region;
	long	region_a;
	long	shell;
	long	shell_a;
	long	faceuse;
	long	faceuse_a;
	long	face;
	long	face_g;
	long	loopuse;
	long	loopuse_a;
	long	loop;
	long	loop_g;
	long	edgeuse;
	long	edgeuse_a;
	long	edge;
	long	edge_g;
	long	vertexuse;
	long	vertexuse_a;
	long	vertex;
	long	vertex_g;
	/* Abstractions */
	long	max_structs;
	long	face_loops;
	long	face_edges;
	long	face_lone_verts;
	long	wire_loops;
	long	wire_loop_edges;
	long	wire_edges;
	long	wire_lone_verts;
	long	shells_of_lone_vert;
};

/*
 *  For use with tables subscripted by NMG structure "index" values,
 *  traditional test and set macros.
 *  A value of zero indicates unset, a value of one indicates set.
 *  test-and-set returns TRUE if value was unset;  in the process,
 *  value has become set.  This is often used to detect the first
 *  time an item is used, so an alternative name is given, for clarity.
 *  Note that the somewhat simpler auto-increment form
 *	( (tab)[(p)->index]++ == 0 )
 *  is not used, to avoid the possibility of integer overflow from
 *  repeated test-and-set operations on one item.
 */
#define NMG_INDEX_VALUE(_tab,_index)	((_tab)[_index])
#define NMG_INDEX_TEST(_tab,_p)		( (_tab)[(_p)->index] )
#define NMG_INDEX_SET(_tab,_p)		{(_tab)[(_p)->index] = 1;}
#define NMG_INDEX_CLEAR(_tab,_p)	{(_tab)[(_p)->index] = 0;}
#define NMG_INDEX_TEST_AND_SET(_tab,_p)	\
	( (_tab)[(_p)->index] == 0 ? ((_tab)[(_p)->index] = 1) : 0 )
#define NMG_INDEX_IS_SET(_tab,_p)	NMG_INDEX_TEST(_tab,_p)
#define NMG_INDEX_FIRST_TIME(_tab,_p)	NMG_INDEX_TEST_AND_SET(_tab,_p)
#define NMG_INDEX_ASSIGN(_tab,_p,_val)	{(_tab)[(_p)->index] = _val;}
#define NMG_INDEX_GET(_tab,_p)		((_tab)[(_p)->index])
#define NMG_INDEX_GETP(_ty,_tab,_p)	((struct _ty *)((_tab)[(_p)->index]))
#define NMG_INDEX_OR(_tab,_p,_val)	{(_tab)[(_p)->index] |= _val;}
#define NMG_INDEX_AND(_tab,_p,_val)	{(_tab)[(_p)->index] &= _val;}

/* flags for manifold-ness */
#define NMG_3MANIFOLD	16
#define NMG_2MANIFOLD	4
#define NMG_1MANIFOLD	2
#define NMG_0MANIFOLD	1
#if 0
# define NMG_DANGLING	8 /* NMG_2MANIFOLD + 4th bit for special cond */
#endif

#define NMG_SET_MANIFOLD(_t,_p,_v) NMG_INDEX_OR(_t, _p, _v)
#define NMG_MANIFOLDS(_t, _p)	   NMG_INDEX_VALUE(_t, (_p)->index)
#define NMG_CP_MANIFOLD(_t, _p, _q) (_t)[(_p)->index] = (_t)[(_q)->index]

/*
 *  Function table, for use with nmg_visit()
 *  Indended to have same generally the organization as nmg_struct_counts.
 *  The handler's args are long* to allow generic handlers to be written,
 *  in which case the magic number at long* specifies the object type.
 *
 *  The "vis_" prefix means the handler is visited only once.
 *  The "bef_" and "aft_" prefixes are called (respectively) before
 *  and after recursing into subsidiary structures.
 *  The 3rd arg is 0 for a "bef_" call, and 1 for an "aft_" call,
 *  to allow generic handlers to be written, if desired.
 */
struct nmg_visit_handlers {
	void	(*bef_model) NMG_ARGS((long *, genptr_t, int));
	void	(*aft_model) NMG_ARGS((long *, genptr_t, int));

	void	(*vis_model_a) NMG_ARGS((long *, genptr_t, int));

	void	(*bef_region) NMG_ARGS((long *, genptr_t, int));
	void	(*aft_region) NMG_ARGS((long *, genptr_t, int));

	void	(*vis_region_a) NMG_ARGS((long *, genptr_t, int));

	void	(*bef_shell) NMG_ARGS((long *, genptr_t, int));
	void	(*aft_shell) NMG_ARGS((long *, genptr_t, int));

	void	(*vis_shell_a) NMG_ARGS((long *, genptr_t, int));

	void	(*bef_faceuse) NMG_ARGS((long *, genptr_t, int));
	void	(*aft_faceuse) NMG_ARGS((long *, genptr_t, int));

	void	(*vis_faceuse_a) NMG_ARGS((long *, genptr_t, int));
	void	(*vis_face) NMG_ARGS((long *, genptr_t, int));
	void	(*vis_face_g) NMG_ARGS((long *, genptr_t, int));

	void	(*bef_loopuse) NMG_ARGS((long *, genptr_t, int));
	void	(*aft_loopuse) NMG_ARGS((long *, genptr_t, int));

	void	(*vis_loopuse_a) NMG_ARGS((long *, genptr_t, int));
	void	(*vis_loop) NMG_ARGS((long *, genptr_t, int));
	void	(*vis_loop_g) NMG_ARGS((long *, genptr_t, int));

	void	(*bef_edgeuse) NMG_ARGS((long *, genptr_t, int));
	void	(*aft_edgeuse) NMG_ARGS((long *, genptr_t, int));

	void	(*vis_edgeuse_a) NMG_ARGS((long *, genptr_t, int));
	void	(*vis_edge) NMG_ARGS((long *, genptr_t, int));
	void	(*vis_edge_g) NMG_ARGS((long *, genptr_t, int));

	void	(*bef_vertexuse) NMG_ARGS((long *, genptr_t, int));
	void	(*aft_vertexuse) NMG_ARGS((long *, genptr_t, int));

	void	(*vis_vertexuse_a) NMG_ARGS((long *, genptr_t, int));
	void	(*vis_vertex) NMG_ARGS((long *, genptr_t, int));
	void	(*vis_vertex_g) NMG_ARGS((long *, genptr_t, int));
};

#endif
