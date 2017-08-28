/*                             G D A L . C P P
 * BRL-CAD
 *
 * Copyright (c) 2013 Tom Browder
 * Copyright (c) 2017 United States Government as represented by the U.S. Army
 * Research Laboratory.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */
/** @file gdal.cpp
 *
 * Geospatial Data Abstraction Library plugin for libgcv.
 *
 */


#include "common.h"
#include "vmath.h"

#include <cmath>
#include <fstream>
#include <iomanip>
#include <map>
#include <set>
#include <sstream>
#include <stdexcept>

/* GDAL headers */
#include <gdal.h>
#include <gdalwarper.h>
#include <gdal_utils.h>
#include <cpl_conv.h>
#include <cpl_string.h>
#include <cpl_multiproc.h>
#include <ogr_spatialref.h>
#include "vrtdataset.h"

#include "raytrace.h"
#include "gcv/api.h"
#include "gcv/util.h"
#include "wdb.h"

struct gdal_read_options
{
    const char *t_srs;
    const char *fmt;
};

struct conversion_state {
    const struct gcv_opts *gcv_options;
    const struct gdal_read_options *ops;
    const char *input_file;
    struct rt_wdb *wdbp;

    /* GDAL state */
    OGRSpatialReference *sp;
    GDALDatasetH hDataset;
    bool info;
    bool debug;
    int scalex;
    int scaley;
    int scalez;
    double adfGeoTransform[6];
    int az;
    int el;
    int pixsize;
};

HIDDEN void
gdal_state_init(struct conversion_state *gs)
{
    int i = 0;
    if(!gs) return;
    gs->gcv_options = NULL;
    gs->ops = NULL;
    gs->input_file = NULL;
    gs->wdbp = RT_WDB_NULL;

    gs->sp = NULL;
    gs->hDataset = NULL;
    gs->info = false;
    gs->debug = false;
    gs->scalex = 1;
    gs->scaley = 1;
    gs->scalez = 1;
    for(i = 0; i < 6; i++) gs->adfGeoTransform[i] = 0.0;
    gs->az = 35;
    gs->el = 35;
    gs->pixsize = 512 * 3;
}

HIDDEN int
get_dataset_info(GDALDatasetH hDataset)
{
    char *gdal_info = GDALInfo(hDataset, NULL);
    if (gdal_info) bu_log("%s", gdal_info);
    CPLFree(gdal_info);
    return 0;
}

HIDDEN int
gdal_can_read(const char *data)
{
    GDALDatasetH hDataset;
    GDALAllRegister();

    if (!data) return 0;

    if (!bu_file_exists(data,NULL)) return 0;

    hDataset = GDALOpenEx(data, GDAL_OF_READONLY | GDAL_OF_RASTER , NULL, NULL, NULL);

    if (!hDataset) return 0;

    GDALClose(hDataset);

    return 1;
}

/* Pull the latitude and longitude out of the dataset.  Based on gdalinfo code, so
 * this function (gdal_ll) is:
 *
 * Copyright (c) 1998, Frank Warmerdam
 * Copyright (c) 2007-2015, Even Rouault <even.rouault at spatialys.com>
 * Copyright (c) 2015, Faza Mahamood
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */
HIDDEN int
gdal_ll(GDALDatasetH hDataset, double *lat_center, double *long_center)
{
    double longitude, latitude;
    double adfGeoTransform[6];
    const char *pzp = NULL;
    OGRCoordinateTransformationH htfm = NULL;
    double gx = GDALGetRasterXSize(hDataset)/2.0;
    double gy = GDALGetRasterYSize(hDataset)/2.0;
    if(GDALGetGeoTransform(hDataset, adfGeoTransform) == CE_None ) {
	longitude = adfGeoTransform[0] + adfGeoTransform[1] * gx  + adfGeoTransform[2] * gy;
	latitude = adfGeoTransform[3] + adfGeoTransform[4] * gx  + adfGeoTransform[5] * gy;
	pzp = GDALGetProjectionRef(hDataset);
    } else {
	return 0;
    }

    if(pzp != NULL && strlen(pzp) > 0) {
	OGRSpatialReferenceH hpj, hll = NULL;
	hpj = OSRNewSpatialReference( pzp );
	if(hpj != NULL) hll = OSRCloneGeogCS(hpj);
	if(hll != NULL)	{
	    CPLPushErrorHandler(CPLQuietErrorHandler);
	    /* The bit we need... */
	    htfm = OCTNewCoordinateTransformation(hpj, hll);
	    CPLPopErrorHandler();
	    OSRDestroySpatialReference(hll);
	}
	if(hpj != NULL) OSRDestroySpatialReference(hpj);
    }

    if(htfm != NULL) {
	if (OCTTransform(htfm,1,&longitude,&latitude,NULL)) {
	    if (lat_center)  (*lat_center)  = latitude;
	    if (long_center) (*long_center) = longitude;
	} else {
	    return 0;
	}
    } else {
	if (lat_center)  (*lat_center)  = latitude;
	if (long_center) (*long_center) = longitude;
    }
    bu_log("lat: %f, long: %f\n", latitude, longitude);

    return 1;
}


/* Get the UTM zone of the GDAL dataset - see
 * https://gis.stackexchange.com/questions/241696/how-to-convert-from-lat-lon-to-utm-with-gdaltransform
 * and the linked posts in the answers for more info. */
HIDDEN int
gdal_utm_zone(struct conversion_state *state)
{
    int zone = INT_MAX;
    double longitude = 0.0;
    if (gdal_ll(state->hDataset, NULL, &longitude)) {
	zone = floor((longitude + 180) / 6) + 1;
    }
    bu_log("zone: %d\n", zone);
    return zone;
}


/* Get corresponding EPSG number of the UTM zone - see
 * https://gis.stackexchange.com/questions/241696/how-to-convert-from-lat-lon-to-utm-with-gdaltransform
 * and the linked posts in the answers for more info. */
HIDDEN int
gdal_utm_epsg(struct conversion_state *state, int zone)
{
    int epsg = INT_MAX;
    double latitude = 0.0;
    if (gdal_ll(state->hDataset, &latitude, NULL)) {
	epsg = (latitude > 0) ? 32600 + zone : 32700 + zone;
    }
    bu_log("epsg: %d\n", epsg);
    return epsg;
}

HIDDEN int
gdal_read(struct gcv_context *context, const struct gcv_opts *gcv_options,
	const void *options_data, const char *source_path)
{
    struct conversion_state *state;
    BU_GET(state, struct conversion_state);
    gdal_state_init(state);
    state->gcv_options = gcv_options;
    state->ops = (struct gdal_read_options *)options_data;
    state->input_file = source_path;
    state->wdbp = context->dbip->dbi_wdbp;

    GDALAllRegister();

    state->hDataset = GDALOpenEx(source_path, GDAL_OF_READONLY | GDAL_OF_VERBOSE_ERROR, NULL, NULL, NULL);
    if (!state->hDataset) {
	bu_log("GDAL Reader: error opening input file %s\n", source_path);
	BU_PUT(state, struct conversion_state);
	return 0;
    }

    (void)get_dataset_info(state->hDataset);

    int zone = gdal_utm_zone(state);
    int epsg = gdal_utm_epsg(state, zone);
    struct bu_vls epsg_str = BU_VLS_INIT_ZERO;
    bu_vls_sprintf(&epsg_str, "EPSG:%d", epsg);
    OGRSpatialReference oSRS;
    oSRS.SetWellKnownGeogCS(bu_vls_addr(&epsg_str));
    oSRS.SetUTM(zone, TRUE);
    char *dst_Wkt = NULL;
    oSRS.exportToWkt(&dst_Wkt);
    GDALDatasetH hOutDS = GDALAutoCreateWarpedVRT(state->hDataset, NULL, dst_Wkt, GRA_CubicSpline, 0.0, NULL);
    bu_log("\nTransformed dataset info:\n");
    (void)get_dataset_info(hOutDS);

    /* UTM reprojection applied, now prepare for DSP (preliminary
     * steps found in gdal_translate before writing an image file...) */
    GDALDatasetH indata = hOutDS;
    unsigned int xsize = GDALGetRasterXSize(indata);
    unsigned int ysize = GDALGetRasterYSize(indata);
    double dfScale=1.0, dfOffset=0.0;
    double dfScaleDstMin = 0.0, dfScaleDstMax = 255.999;
    double adfCMinMax[2];
    double adfGeoTransform[6];
    VRTDataset *poVDS = (VRTDataset *)VRTCreate(GDALGetRasterXSize(indata), GDALGetRasterYSize(indata));
    poVDS->SetProjection(GDALGetProjectionRef(indata));
    GDALGetGeoTransform(indata, adfGeoTransform);
    poVDS->SetGeoTransform(adfGeoTransform);
    GDALRasterBand *poSrcBand = ((GDALDataset *)indata)->GetRasterBand(1);
    poVDS->AddBand(GDALGetRasterDataType(poSrcBand), NULL);
    VRTSourcedRasterBand *poVRTBand = (VRTSourcedRasterBand *)poVDS->GetRasterBand(1);

    GDALComputeRasterMinMax(poSrcBand, TRUE, adfCMinMax);
    dfScale = (dfScaleDstMax - dfScaleDstMin) / (adfCMinMax[1] - adfCMinMax[0]);
    dfOffset = -1 * adfCMinMax[0] * dfScale + dfScaleDstMin;
    VRTComplexSource* poSource = new VRTComplexSource();
    poSource->SetLinearScaling(dfOffset, dfScale);
    poSource->SetResampling(NULL);
    poVRTBand->ConfigureSource(poSource, poSrcBand, FALSE, 0, 0, xsize, ysize, 0, 0, xsize, ysize);
    poVRTBand->AddSource(poSource);
    GDALDatasetH flatDS = GDALCreateCopy(GDALGetDriverByName("MEM"), "", (GDALDatasetH) poVDS, 0, NULL, GDALTermProgress, NULL);
    GDALClose((GDALDatasetH)poVDS);
    indata = flatDS;

#if 1
    /* Read the data into something a DSP can process */
    unsigned short *uint16_array = (unsigned short *)bu_calloc(xsize*ysize, sizeof(unsigned short), "unsigned short array");
    GDALRasterBandH band = GDALGetRasterBand(indata, 1);
    int bx = GDALGetRasterBandXSize(band);
    int by = GDALGetRasterBandYSize(band);
    /* If we're going to DSP we need the unsigned short read. */
    uint16_t *scanline = (uint16_t *)CPLMalloc(sizeof(uint16_t)*bx);
    for(unsigned int i = 0; i < ysize; i++) {
	if (GDALRasterIO(band, GF_Read, 0, i, bx, 1, scanline, bx, 1, GDT_UInt16, 0, 0) == CPLE_None) {
	    for (unsigned int j = 0; j < xsize; ++j) {
		/* This is the critical assignment point - if we get this
		 * indexing wrong, data will not look right in dsp */
		uint16_array[ysize*i+j] = scanline[j];
		//bu_log("%d, %d: %d\n", i, j, scanline[j]);
	    }
	}
    }

    /* Got it - write the binary object to the .g file */
    mk_binunif(state->wdbp, "test.data", (void *)uint16_array, WDB_BINUNIF_UINT16, xsize * ysize);

    /* Done reading - close out inputs */
    CPLFree(scanline);
    GDALClose(state->hDataset);
    //GDALClose(pjdata);
#endif

#if 0
    unsigned int xsize = 1187;
    unsigned int ysize = 1249;
    unsigned short *uint16_array = (unsigned short *)bu_calloc(xsize*ysize, sizeof(unsigned short), "unsigned short array");
    uint16_array[ysize*600+1157] = 100;
    uint16_array[ysize*600+1158] = 100;
    uint16_array[ysize*601+1157] = 100;
    uint16_array[ysize*601+1158] = 100;
    mk_binunif(state->wdbp, "test.data", (void *)uint16_array, WDB_BINUNIF_UINT16, xsize * ysize);
#endif

    /* TODO: if we're going to BoT (3-space mesh, will depend on the transform requested) we will need different logic... */

    /* Write out the dsp.  Since we're using a data object, instead of a file,
     * do the setup by hand. */
    struct rt_dsp_internal *dsp;
    BU_ALLOC(dsp, struct rt_dsp_internal);
    dsp->magic = RT_DSP_INTERNAL_MAGIC;
    bu_vls_init(&dsp->dsp_name);
    bu_vls_strcpy(&dsp->dsp_name, "test.data");
    dsp->dsp_datasrc = RT_DSP_SRC_OBJ;
    dsp->dsp_xcnt = ysize;
    dsp->dsp_ycnt = xsize;
    dsp->dsp_smooth = 1;
    dsp->dsp_cuttype = DSP_CUT_DIR_ADAPT;
    MAT_IDN(dsp->dsp_stom);
    dsp->dsp_stom[0] = dsp->dsp_stom[5] = 1000;
    bn_mat_inv(dsp->dsp_mtos, dsp->dsp_stom);

    wdb_export(state->wdbp, "test.s", (void *)dsp, ID_DSP, 1);


    return 1;
}

extern "C"
{
    struct gcv_filter gcv_conv_gdal_read =
    {"GDAL Reader", GCV_FILTER_READ, BU_MIME_MODEL_AUTO, gdal_can_read, NULL, NULL, gdal_read};

    static const struct gcv_filter * const filters[] = {&gcv_conv_gdal_read, NULL};
    const struct gcv_plugin gcv_plugin_info_s = { filters };
    GCV_EXPORT const struct gcv_plugin *gcv_plugin_info(){return &gcv_plugin_info_s;}
}

// Local Variables:
// tab-width: 8
// mode: C++
// c-basic-offset: 4
// indent-tabs-mode: t
// c-file-style: "stroustrup"
// End:
// ex: shiftwidth=4 tabstop=8
