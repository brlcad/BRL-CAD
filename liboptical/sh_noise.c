/*
 *	S H _ G R A V E L . C
 *
 *	Shaders:
 *	gravel		turbulence noise applied to color and surface normal
 *	turbump		turbulence noise applied to surface normal
 *	turcolor	turbulence noise applied to color
 *	fbmbump		fbm noise applied to surface normal
 *	fbmcolor	fbm noise applied to color
 *
 *
 */
#include "conf.h"

#include <stdio.h>
#include <math.h>
#include "machine.h"
#include "vmath.h"
#include "raytrace.h"
#include "./material.h"
#include "./mathtab.h"
#include "./rdebug.h"

#define gravel_MAGIC 0x1847
#define CK_gravel_SP(_p) RT_CKMAG(_p, gravel_MAGIC, "gravel_specific")

/* This allows us to specify the "size" parameter as values like ".5m"
 * or "27in" rather than using mm all the time.
 */
void
gravel_cvt_parse( sdp, name, base, value )
register CONST struct bu_structparse	*sdp;	/* structure description */
register CONST char			*name;	/* struct member name */
char					*base;	/* begining of structure */
CONST char				*value;	/* string containing value */
{
	double *p = (double *)(base+sdp->sp_offset);

	/* reconvert with optional units */
	*p = rt_mm_value(value);
}

void
gravel_deg_to_rad( sdp, name, base, value )
register CONST struct bu_structparse	*sdp;	/* structure description */
register CONST char			*name;	/* struct member name */
char					*base;	/* begining of structure */
CONST char				*value;	/* string containing value */
{
	double *p = (double *)(base+sdp->sp_offset);

	/* reconvert with optional units */
	*p = *p * (M_PI / 180.0);
}

/*
 * the shader specific structure contains all variables which are unique
 * to any particular use of the shader.
 */
struct gravel_specific {
	long	magic;	/* magic # for memory validity check, must come 1st */
	double	lacunarity;
	double	h_val;
	double	octaves;
	double	size;
	double	max_angle;
	point_t	vscale;	/* size of noise coordinate space */
	vect_t	delta;
	mat_t	m_to_sh;	/* model to shader space matrix */
	mat_t	sh_to_m;	/* shader to model space matrix */
	struct rt_vls	shader;
};

/* The default values for the variables in the shader specific structure */
CONST static
struct gravel_specific gravel_defaults = {
	gravel_MAGIC,
	2.1753974,	/* lacunarity */
	1.0,		/* h_val */
	4.0,		/* octaves */
	1.0,		/* size */
	M_PI_2,		/* max_angle */
	{ 1.0, 1.0, 1.0 },	/* vscale */
	{ 1000.0, 1000.0, 1000.0 },	/* delta into noise space */
	{	0.0, 0.0, 0.0, 0.0,
		0.0, 0.0, 0.0, 0.0,
		0.0, 0.0, 0.0, 0.0,
		0.0, 0.0, 0.0, 0.0 },
	{	0.0, 0.0, 0.0, 0.0,
		0.0, 0.0, 0.0, 0.0,
		0.0, 0.0, 0.0, 0.0,
		0.0, 0.0, 0.0, 0.0 },
	{ 0, (char *)0, 0, 0, 0 }
	};

#define SHDR_NULL	((struct gravel_specific *)0)
#define SHDR_O(m)	offsetof(struct gravel_specific, m)
#define SHDR_AO(m)	offsetofarray(struct gravel_specific, m)


/* description of how to parse/print the arguments to the shader
 * There is at least one line here for each variable in the shader specific
 * structure above
 */
struct bu_structparse gravel_print_tab[] = {
	{"%f",	1, "lacunarity",	SHDR_O(lacunarity),	FUNC_NULL },
	{"%f",	1, "H", 		SHDR_O(h_val),		FUNC_NULL },
	{"%f",	1, "octaves", 		SHDR_O(octaves),	FUNC_NULL },
	{"%f",  3, "delta",		SHDR_AO(delta),	FUNC_NULL },
	{"%f",  1, "size",		SHDR_O(size),		gravel_cvt_parse },
	{"%f",  1, "angle",		SHDR_O(max_angle),	gravel_deg_to_rad },
	{"%f",  3, "vscale",		SHDR_AO(vscale),	FUNC_NULL },
	{"",	0, (char *)0,		0,			FUNC_NULL }

};
struct bu_structparse gravel_parse_tab[] = {
	{"i",	bu_byteoffset(gravel_print_tab[0]), "gravel_print_tab", 0, FUNC_NULL },
	{"%f",	1, "lacunarity",	SHDR_O(lacunarity),	FUNC_NULL },
	{"%f",	1, "l",			SHDR_O(lacunarity),	FUNC_NULL },
	{"%f",	1, "H", 		SHDR_O(h_val),		FUNC_NULL },
	{"%f",	1, "octaves", 		SHDR_O(octaves),	FUNC_NULL },
	{"%f",	1, "o", 		SHDR_O(octaves),	FUNC_NULL },
	{"%f",  3, "delta",		SHDR_AO(delta),	FUNC_NULL },
	{"%f",  3, "d",			SHDR_AO(delta),	FUNC_NULL },
	{"%f",  1, "size",		SHDR_O(size),		gravel_cvt_parse },
	{"%f",  1, "s",			SHDR_O(size),		gravel_cvt_parse },
	{"%f",  1, "angle",		SHDR_O(max_angle),		gravel_deg_to_rad },
	{"%f",  1, "ang",		SHDR_O(max_angle),		gravel_deg_to_rad },
	{"%f",  1, "a",			SHDR_O(max_angle),		gravel_deg_to_rad },
	{"%f",  3, "vscale",		SHDR_AO(vscale),	FUNC_NULL },
	{"%f",  3, "vs",		SHDR_AO(vscale),	FUNC_NULL },
	{"%f",  3, "v",			SHDR_AO(vscale),	FUNC_NULL },
	{"",	0, (char *)0,		0,			FUNC_NULL }
};

HIDDEN int	gravel_setup(), gravel_render(),
		fbmbump_render(), turbump_render(),
		fbmcolor_render(), turcolor_render();
HIDDEN void	gravel_print(), gravel_free();

/* The "mfuncs" structure defines the external interface to the shader.
 * Note that more than one shader "name" can be associated with a given
 * shader by defining more than one mfuncs struct in this array.
 * See sh_phong.c for an example of building more than one shader "name"
 * from a set of source functions.  There you will find that "glass" "mirror"
 * and "plastic" are all names for the same shader with different default
 * values for the parameters.
 */
struct mfuncs gravel_mfuncs[] = {
	{"gravel",	0,	0,	MFI_NORMAL|MFI_HIT|MFI_UV,	0,
	gravel_setup,	gravel_render,	gravel_print,	gravel_free },

	{"fbmbump",	0,	0,	MFI_NORMAL|MFI_HIT|MFI_UV,	0,
	gravel_setup,	fbmbump_render,	gravel_print,	gravel_free },

	{"turbump",	0,	0,	MFI_NORMAL|MFI_HIT|MFI_UV,	0,
	gravel_setup,	turbump_render, gravel_print,	gravel_free },

	{"fbmcolor",	0,	0,	MFI_NORMAL|MFI_HIT|MFI_UV,	0,
	gravel_setup,	fbmcolor_render, gravel_print,	gravel_free },

	{"turcolor",	0,	0,	MFI_NORMAL|MFI_HIT|MFI_UV,	0,
	gravel_setup,	turcolor_render, gravel_print,	gravel_free },

	{(char *)0,	0,	0,		0,		0,
	0,		0,		0,		0 }
};


/*	G R A V E L _ S E T U P
 *
 *	This routine is called (at prep time)
 *	once for each region which uses this shader.
 *	Any shader-specific initialization should be done here.
 */
HIDDEN int
gravel_setup( rp, matparm, dpp, mfp, rtip)
register struct region	*rp;
struct rt_vls		*matparm;
char			**dpp;	/* pointer to reg_udata in *rp */
struct mfuncs		*mfp;
struct rt_i		*rtip;	/* New since 4.4 release */
{
	register struct gravel_specific	*gravel_sp;
	mat_t	tmp;
	mat_t model_to_region;
	vect_t	bb_min, bb_max, v_tmp;

	/* check the arguments */
	RT_CHECK_RTI(rtip);
	RT_VLS_CHECK( matparm );
	RT_CK_REGION(rp);


	if( rdebug&RDEBUG_SHADE)
		rt_log("gravel_setup(%s, %s)\n",
			rp->reg_name, rp->reg_mater.ma_shader);

	/* Get memory for the shader parameters and shader-specific data */
	GETSTRUCT( gravel_sp, gravel_specific );
	*dpp = (char *)gravel_sp;

	/* initialize the default values for the shader */
	memcpy(gravel_sp, &gravel_defaults, sizeof(struct gravel_specific) );

	/* parse the user's arguments for this use of the shader. */
	if( bu_struct_parse( matparm, gravel_parse_tab, (char *)gravel_sp ) < 0 )
		return(-1);

	bu_vls_init(&gravel_sp->shader);
	bu_vls_strcpy(&gravel_sp->shader, rp->reg_mater.ma_shader);

	/* Optional:
	 *
	 * If the shader needs to operate in a coordinate system which stays
	 * fixed on the region when the region is moved (as in animation)
	 * we need to get a matrix to perform the appropriate transform(s).
	 *
	 * db_shader_mat returns a matrix which maps points on/in the region
	 * into the unit cube.  This unit cube is formed by first mapping from
	 * world coordinates into "region coordinates" (the coordinate system
	 * in which the region is defined).  Then the bounding box of the 
	 * region is used to establish a mapping to the unit cube
	 *
*	db_shader_mat(xxx_sp->xxx_m_to_sh, rtip, rp, xxx_sp->xxx_min,
*		xxx_sp->xxx_max);
	 */

	db_region_mat(model_to_region, rtip->rti_dbip, rp->reg_name);

	mat_idn(tmp);
	if (gravel_sp->size != 1.0) {
		/* the user sets "size" to the size of the biggest
		 * noise-space blob in model coordinates
		 */
		tmp[0] = tmp[5] = tmp[10] = 1.0/gravel_sp->size;
	} else {
		tmp[0] = 1.0/gravel_sp->vscale[0];
		tmp[5] = 1.0/gravel_sp->vscale[1];
		tmp[10] = 1.0/gravel_sp->vscale[2];
	}

	mat_mul(gravel_sp->m_to_sh, tmp, model_to_region);

	/* Add any translation within shader/region space */
	mat_idn(tmp);
	tmp[MDX] = gravel_sp->delta[0];
	tmp[MDY] = gravel_sp->delta[1];
	tmp[MDZ] = gravel_sp->delta[2];
	mat_mul2(tmp, gravel_sp->m_to_sh);

	mat_inv(gravel_sp->sh_to_m, gravel_sp->m_to_sh);

	if( rdebug&RDEBUG_SHADE) {
		bu_struct_print( " Parameters:", gravel_print_tab, (char *)gravel_sp );
		mat_print( "m_to_sh", gravel_sp->m_to_sh );
	}

	return(1);
}

/*
 *	G R A V E L _ P R I N T
 */
HIDDEN void
gravel_print( rp, dp )
register struct region *rp;
char	*dp;
{
	bu_struct_print( rp->reg_name, gravel_print_tab, (char *)dp );
}

/*
 *	G R A V E L _ F R E E
 */
HIDDEN void
gravel_free( cp )
char *cp;
{
	rt_free( cp, "gravel_specific" );
}

/*
 *	G R A V E L _ R E N D E R
 *
 *	This is called (from viewshade() in shade.c) once for each hit point
 *	to be shaded.  The purpose here is to fill in values in the shadework
 *	structure.
 */
int
gravel_render( ap, pp, swp, dp )
struct application	*ap;
struct partition	*pp;
struct shadework	*swp;	/* defined in material.h */
char			*dp;	/* ptr to the shader-specific struct */
{
	register struct gravel_specific *gravel_sp =
		(struct gravel_specific *)dp;
	point_t pt;
	point_t u_pt, v_pt;
	vect_t u_vec, v_vec;
	double u_val, v_val, noise_space_delta;
	mat_t u_mat, v_mat;
	vect_t N, tmp;
	double val;

	/* check the validity of the arguments we got */
	RT_AP_CHECK(ap);
	RT_CHECK_PT(pp);
	CK_gravel_SP(gravel_sp);

	if( rdebug&RDEBUG_SHADE)
		bu_struct_print( "gravel_render Parameters:", gravel_print_tab, (char *)gravel_sp );

	/* If we are performing the shading in "region" space, we must 
	 * transform the hit point from "model" space to "region" space.
	 * See the call to db_region_mat in gravel_setup().
	 */
	MAT4X3PNT(pt, gravel_sp->m_to_sh, swp->sw_hit.hit_point);

	if( rdebug&RDEBUG_SHADE) {
		rt_log("%s:%d gravel_render(%s)  model:(%g %g %g) shader:(%g %g %g)\n", 
		__FILE__, __LINE__,
		bu_vls_addr(&gravel_sp->shader),
		V3ARGS(swp->sw_hit.hit_point),
		V3ARGS(pt) );
	}

	val = bn_noise_turb(pt, gravel_sp->h_val, gravel_sp->lacunarity,
			gravel_sp->octaves);

	VSCALE(swp->sw_color, swp->sw_color, val);


	/* dork the normal around
	 * Convert the normal to shader space, get u,v coordinate system
	 */

	if( rdebug&RDEBUG_SHADE) {
		VPRINT("Model space Normal", swp->sw_hit.hit_normal);
	}
	MAT4X3VEC(N, gravel_sp->m_to_sh, swp->sw_hit.hit_normal);
	VUNITIZE(N);
	if( rdebug&RDEBUG_SHADE) {
		VPRINT("Shader space Normal", N);
	}

	bn_vec_perp(u_vec, N);
	VCROSS(v_vec, N, u_vec);

	noise_space_delta = 1.0 / 
		pow(gravel_sp->lacunarity, gravel_sp->octaves);


	VJOIN1(u_pt, pt, noise_space_delta, u_vec);
	u_val = bn_noise_turb(u_pt, gravel_sp->h_val, gravel_sp->lacunarity,
		gravel_sp->octaves);

	VJOIN1(v_pt, pt, noise_space_delta, v_vec);
	v_val = bn_noise_turb(v_pt, gravel_sp->h_val, gravel_sp->lacunarity,
		gravel_sp->octaves);

	if( rdebug&RDEBUG_SHADE) {
		VPRINT("pt", pt);
		VPRINT("u_vec", u_vec);
		VPRINT("u_pt", u_pt);
		VPRINT("v_vec", v_vec);
		VPRINT("v_pt", v_pt);
		
		rt_log("u_val:%g  v_val:%g  nsd:%g\n",
			u_val, v_val, noise_space_delta);
	}

	bn_mat_arb_rot(u_mat, pt, u_vec, (val - v_val) * gravel_sp->max_angle);
	MAT4X3VEC(tmp, u_mat, N);

	bn_mat_arb_rot(v_mat, pt, v_vec, (val - u_val) * gravel_sp->max_angle);
	MAT4X3VEC(N, v_mat, tmp);

	if( rdebug&RDEBUG_SHADE) {
		VPRINT("old normal", swp->sw_hit.hit_normal);
	}

	MAT4X3VEC(swp->sw_hit.hit_normal, gravel_sp->sh_to_m, N);
	VUNITIZE(swp->sw_hit.hit_normal);
	if( rdebug&RDEBUG_SHADE) {
		VPRINT("new normal", swp->sw_hit.hit_normal);
	}

	/* shader must perform transmission/reflection calculations
	 *
	 * 0 < swp->sw_transmit <= 1 causes transmission computations
	 * 0 < swp->sw_reflect <= 1 causes reflection computations
	 */
	if( swp->sw_reflect > 0 || swp->sw_transmit > 0 )
		(void)rr_render( ap, pp, swp );

	return(1);
}
/*
 *	T U R C O L O R _ R E N D E R
 *
 *	This is called (from viewshade() in shade.c) once for each hit point
 *	to be shaded.  The purpose here is to fill in values in the shadework
 *	structure.
 */
int
turcolor_render( ap, pp, swp, dp )
struct application	*ap;
struct partition	*pp;
struct shadework	*swp;	/* defined in material.h */
char			*dp;	/* ptr to the shader-specific struct */
{
	register struct gravel_specific *gravel_sp =
		(struct gravel_specific *)dp;
	point_t pt;
	point_t u_pt, v_pt;
	vect_t u_vec, v_vec;
	double u_val, v_val, noise_space_delta;
	mat_t u_mat, v_mat;
	vect_t N, tmp;
	double val;

	/* check the validity of the arguments we got */
	RT_AP_CHECK(ap);
	RT_CHECK_PT(pp);
	CK_gravel_SP(gravel_sp);

	if( rdebug&RDEBUG_SHADE)
		bu_struct_print( "gravel_render Parameters:", gravel_print_tab, (char *)gravel_sp );

	/* If we are performing the shading in "region" space, we must 
	 * transform the hit point from "model" space to "region" space.
	 * See the call to db_region_mat in gravel_setup().
	 */
	MAT4X3PNT(pt, gravel_sp->m_to_sh, swp->sw_hit.hit_point);

	if( rdebug&RDEBUG_SHADE) {
		rt_log("%s:%d gravel_render(%s)  model:(%g %g %g) shader:(%g %g %g)\n", 
		__FILE__, __LINE__,
		bu_vls_addr(&gravel_sp->shader),
		V3ARGS(swp->sw_hit.hit_point),
		V3ARGS(pt) );
	}

	val = bn_noise_turb(pt, gravel_sp->h_val, gravel_sp->lacunarity,
			gravel_sp->octaves);

	VSCALE(swp->sw_color, swp->sw_color, val);

	/* shader must perform transmission/reflection calculations
	 *
	 * 0 < swp->sw_transmit <= 1 causes transmission computations
	 * 0 < swp->sw_reflect <= 1 causes reflection computations
	 */
	if( swp->sw_reflect > 0 || swp->sw_transmit > 0 )
		(void)rr_render( ap, pp, swp );

	return(1);
}


/*
 *	F B M C O L O R _ R E N D E R
 *
 *	This is called (from viewshade() in shade.c) once for each hit point
 *	to be shaded.  The purpose here is to fill in values in the shadework
 *	structure.
 */
int
fbmcolor_render( ap, pp, swp, dp )
struct application	*ap;
struct partition	*pp;
struct shadework	*swp;	/* defined in material.h */
char			*dp;	/* ptr to the shader-specific struct */
{
	register struct gravel_specific *gravel_sp =
		(struct gravel_specific *)dp;
	point_t pt;
	point_t u_pt, v_pt;
	vect_t u_vec, v_vec;
	double u_val, v_val, noise_space_delta;
	mat_t u_mat, v_mat;
	vect_t N, tmp;
	double val;

	/* check the validity of the arguments we got */
	RT_AP_CHECK(ap);
	RT_CHECK_PT(pp);
	CK_gravel_SP(gravel_sp);

	if( rdebug&RDEBUG_SHADE)
		bu_struct_print( "fbmcolor_render Parameters:", gravel_print_tab, (char *)gravel_sp );

	/* If we are performing the shading in "region" space, we must 
	 * transform the hit point from "model" space to "region" space.
	 * See the call to db_region_mat in gravel_setup().
	 */
	MAT4X3PNT(pt, gravel_sp->m_to_sh, swp->sw_hit.hit_point);

	if( rdebug&RDEBUG_SHADE) {
		rt_log("%s:%d gravel_render(%s)  model:(%g %g %g) shader:(%g %g %g)\n", 
		__FILE__, __LINE__,
		bu_vls_addr(&gravel_sp->shader),
		V3ARGS(swp->sw_hit.hit_point),
		V3ARGS(pt) );
	}

	val = bn_noise_fbm(pt, gravel_sp->h_val, gravel_sp->lacunarity,
			gravel_sp->octaves);

	val = (val+1.0)*0.5;
	VSCALE(swp->sw_color, swp->sw_color, val);

	/* shader must perform transmission/reflection calculations
	 *
	 * 0 < swp->sw_transmit <= 1 causes transmission computations
	 * 0 < swp->sw_reflect <= 1 causes reflection computations
	 */
	if( swp->sw_reflect > 0 || swp->sw_transmit > 0 )
		(void)rr_render( ap, pp, swp );

	return(1);
}





/*
 *	T U R B U M P _ R E N D E R
 *
 *	"turbulence" noise bumps
 */
int
turbump_render( ap, pp, swp, dp )
struct application	*ap;
struct partition	*pp;
struct shadework	*swp;	/* defined in material.h */
char			*dp;	/* ptr to the shader-specific struct */
{
	register struct gravel_specific *gravel_sp =
		(struct gravel_specific *)dp;
	point_t pt;
	point_t u_pt, v_pt;
	vect_t u_vec, v_vec;
	double u_val, v_val, noise_space_delta;
	mat_t u_mat, v_mat;
	vect_t N, tmp;
	double val;

	/* check the validity of the arguments we got */
	RT_AP_CHECK(ap);
	RT_CHECK_PT(pp);
	CK_gravel_SP(gravel_sp);

	if( rdebug&RDEBUG_SHADE)
		bu_struct_print( "gravel_render Parameters:", gravel_print_tab, (char *)gravel_sp );

	/* If we are performing the shading in "region" space, we must 
	 * transform the hit point from "model" space to "region" space.
	 * See the call to db_region_mat in gravel_setup().
	 */
	MAT4X3PNT(pt, gravel_sp->m_to_sh, swp->sw_hit.hit_point);

	if( rdebug&RDEBUG_SHADE) {
		rt_log("%s:%d gravel_render(%s)  model:(%g %g %g) shader:(%g %g %g)\n", 
		__FILE__, __LINE__,
		bu_vls_addr(&gravel_sp->shader),
		V3ARGS(swp->sw_hit.hit_point),
		V3ARGS(pt) );
	}

	val = bn_noise_turb(pt, gravel_sp->h_val, gravel_sp->lacunarity,
			gravel_sp->octaves);

	/* dork the normal around
	 * Convert the normal to shader space, get u,v coordinate system
	 */

	if( rdebug&RDEBUG_SHADE) {
		VPRINT("Model space Normal", swp->sw_hit.hit_normal);
	}
	MAT4X3VEC(N, gravel_sp->m_to_sh, swp->sw_hit.hit_normal);
	VUNITIZE(N);
	if( rdebug&RDEBUG_SHADE) {
		VPRINT("Shader space Normal", N);
	}

	bn_vec_perp(u_vec, N);
	VCROSS(v_vec, N, u_vec);

	noise_space_delta = 1.0 / 
		pow(gravel_sp->lacunarity, gravel_sp->octaves);


	VJOIN1(u_pt, pt, noise_space_delta, u_vec);
	u_val = bn_noise_turb(u_pt, gravel_sp->h_val, gravel_sp->lacunarity,
		gravel_sp->octaves);

	VJOIN1(v_pt, pt, noise_space_delta, v_vec);
	v_val = bn_noise_turb(v_pt, gravel_sp->h_val, gravel_sp->lacunarity,
		gravel_sp->octaves);

	if( rdebug&RDEBUG_SHADE) {
		VPRINT("pt", pt);
		VPRINT("u_vec", u_vec);
		VPRINT("u_pt", u_pt);
		VPRINT("v_vec", v_vec);
		VPRINT("v_pt", v_pt);
		
		rt_log("u_val:%g  v_val:%g  nsd:%g\n",
			u_val, v_val, noise_space_delta);
	}

	bn_mat_arb_rot(u_mat, pt, u_vec, (val - v_val) * gravel_sp->max_angle);
	MAT4X3VEC(tmp, u_mat, N);

	bn_mat_arb_rot(v_mat, pt, v_vec, (val - u_val) * gravel_sp->max_angle);
	MAT4X3VEC(N, v_mat, tmp);

	if( rdebug&RDEBUG_SHADE) {
		VPRINT("old normal", swp->sw_hit.hit_normal);
	}

	MAT4X3VEC(swp->sw_hit.hit_normal, gravel_sp->sh_to_m, N);
	VUNITIZE(swp->sw_hit.hit_normal);
	if( rdebug&RDEBUG_SHADE) {
		VPRINT("new normal", swp->sw_hit.hit_normal);
	}

	/* shader must perform transmission/reflection calculations
	 *
	 * 0 < swp->sw_transmit <= 1 causes transmission computations
	 * 0 < swp->sw_reflect <= 1 causes reflection computations
	 */
	if( swp->sw_reflect > 0 || swp->sw_transmit > 0 )
		(void)rr_render( ap, pp, swp );

	return(1);
}






/*
 *	F B M B U M P _ R E N D E R
 *
 *	This is called (from viewshade() in shade.c) once for each hit point
 *	to be shaded.  The purpose here is to fill in values in the shadework
 *	structure.
 */
int
fbmbump_render( ap, pp, swp, dp )
struct application	*ap;
struct partition	*pp;
struct shadework	*swp;	/* defined in material.h */
char			*dp;	/* ptr to the shader-specific struct */
{
	register struct gravel_specific *gravel_sp =
		(struct gravel_specific *)dp;
	point_t pt;
	point_t u_pt, v_pt;
	vect_t u_vec, v_vec;
	double u_val, v_val, noise_space_delta;
	mat_t u_mat, v_mat;
	vect_t N, tmp;
	double val;

	/* check the validity of the arguments we got */
	RT_AP_CHECK(ap);
	RT_CHECK_PT(pp);
	CK_gravel_SP(gravel_sp);

	if( rdebug&RDEBUG_SHADE)
		bu_struct_print( "gravel_render Parameters:", gravel_print_tab, (char *)gravel_sp );

	/* If we are performing the shading in "region" space, we must 
	 * transform the hit point from "model" space to "region" space.
	 * See the call to db_region_mat in gravel_setup().
	 */
	MAT4X3PNT(pt, gravel_sp->m_to_sh, swp->sw_hit.hit_point);

	if( rdebug&RDEBUG_SHADE) {
		rt_log("%s:%d gravel_render(%s)  model:(%g %g %g) shader:(%g %g %g)\n", 
		__FILE__, __LINE__,
		bu_vls_addr(&gravel_sp->shader),
		V3ARGS(swp->sw_hit.hit_point),
		V3ARGS(pt) );
	}

	val = bn_noise_fbm(pt, gravel_sp->h_val, gravel_sp->lacunarity,
			gravel_sp->octaves);

	/* dork the normal around
	 * Convert the normal to shader space, get u,v coordinate system
	 */

	if( rdebug&RDEBUG_SHADE) {
		VPRINT("Model space Normal", swp->sw_hit.hit_normal);
	}
	MAT4X3VEC(N, gravel_sp->m_to_sh, swp->sw_hit.hit_normal);
	VUNITIZE(N);
	if( rdebug&RDEBUG_SHADE) {
		VPRINT("Shader space Normal", N);
	}

	bn_vec_perp(u_vec, N);
	VCROSS(v_vec, N, u_vec);

	noise_space_delta = 1.0 / 
		pow(gravel_sp->lacunarity, gravel_sp->octaves);


	VJOIN1(u_pt, pt, noise_space_delta, u_vec);
	u_val = bn_noise_turb(u_pt, gravel_sp->h_val, gravel_sp->lacunarity,
		gravel_sp->octaves);

	VJOIN1(v_pt, pt, noise_space_delta, v_vec);
	v_val = bn_noise_turb(v_pt, gravel_sp->h_val, gravel_sp->lacunarity,
		gravel_sp->octaves);

	if( rdebug&RDEBUG_SHADE) {
		VPRINT("pt", pt);
		VPRINT("u_vec", u_vec);
		VPRINT("u_pt", u_pt);
		VPRINT("v_vec", v_vec);
		VPRINT("v_pt", v_pt);
		
		rt_log("u_val:%g  v_val:%g  nsd:%g\n",
			u_val, v_val, noise_space_delta);
	}

	bn_mat_arb_rot(u_mat, pt, u_vec, (val - v_val) * gravel_sp->max_angle);
	MAT4X3VEC(tmp, u_mat, N);

	bn_mat_arb_rot(v_mat, pt, v_vec, (val - u_val) * gravel_sp->max_angle);
	MAT4X3VEC(N, v_mat, tmp);

	if( rdebug&RDEBUG_SHADE) {
		VPRINT("old normal", swp->sw_hit.hit_normal);
	}

	MAT4X3VEC(swp->sw_hit.hit_normal, gravel_sp->sh_to_m, N);
	VUNITIZE(swp->sw_hit.hit_normal);
	if( rdebug&RDEBUG_SHADE) {
		VPRINT("new normal", swp->sw_hit.hit_normal);
	}

	/* shader must perform transmission/reflection calculations
	 *
	 * 0 < swp->sw_transmit <= 1 causes transmission computations
	 * 0 < swp->sw_reflect <= 1 causes reflection computations
	 */
	if( swp->sw_reflect > 0 || swp->sw_transmit > 0 )
		(void)rr_render( ap, pp, swp );

	return(1);
}
