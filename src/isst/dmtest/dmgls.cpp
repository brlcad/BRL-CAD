/*                      D M G L S . C P P
 * BRL-CAD
 *
 * Copyright (c) 2021 United States Government as represented by
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
/** @file dmgls.cpp
 *
 * Brief description
 *
 */

#include <QOpenGLWidget>
#include <QKeyEvent>
#include <QGuiApplication> // for qGuiApp

extern "C" {
#include "bu/parallel.h"
#include "bview/util.h"
#include "ged.h"
}
#include "dmgls.h"

dmGL::dmGL(QWidget *parent)
    : QOpenGLWidget(parent)
{
    BU_GET(v, struct bview);
    bview_init(v);

    // This is an important Qt setting for interactivity - it allowing key
    // bindings to propagate to this widget and trigger actions such as
    // resolution scaling, rotation, etc.
    setFocusPolicy(Qt::WheelFocus);
}

dmGL::~dmGL()
{
    BU_PUT(v, struct bview);
}


void dmGL::paintGL()
{
    int w = width();
    int h = height();
    // Zero size == nothing to do
    if (!w || !h)
	return;

    if (!gedp) {
	return;
    }

    if (!m_init) {
	initializeOpenGLFunctions();
	m_init = true;
	if (!dmp) {
	    const char *acmd = "attach";
	    dmp = dm_open((void *)this, NULL, "qtgl", 1, &acmd);
	    if (gedp) {
		gedp->ged_dmp = (void *)dmp;
		dm_set_vp(dmp, &gedp->ged_gvp->gv_scale);
	    }
	    dm_configure_win(dmp, 0);
	}
    }

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Go ahead and set the flag, but (unlike the rendering thread
    // implementation) we need to do the draw routine every time in paintGL, or
    // we end up with unrendered frames.
    dm_set_dirty(dmp, 0);

    if (gedp) {
	matp_t mat = gedp->ged_gvp->gv_model2view;
	dm_loadmatrix(dmp, mat, 0);
	unsigned char geometry_default_color[] = { 255, 0, 0 };
	dm_draw_display_list(dmp, gedp->ged_gdp->gd_headDisplay,
		1.0, gedp->ged_gvp->gv_isize, -1, -1, -1, 1,
		0, 0, geometry_default_color, 1, 0);
    }
}

void dmGL::resizeGL(int, int)
{
    if (gedp && gedp->ged_dmp) {
	dm_configure_win((struct dm *)gedp->ged_dmp, 0);
	dm_set_dirty((struct dm *)gedp->ged_dmp, 1);
    }
}

void dmGL::keyPressEvent(QKeyEvent *k) {

    if (!gedp || !gedp->ged_dmp) {
	QOpenGLWidget::keyPressEvent(k);
	return;
    }

    // Let bview know what the current view width and height are, in
    // case the dx/dy mouse translations need that information
    v->gv_width = width();
    v->gv_height = height();


    if (CADkeyPressEvent(v, x_prev, y_prev, k)) {
	dm_set_dirty((struct dm *)gedp->ged_dmp, 1);
	update();
    }

    QOpenGLWidget::keyPressEvent(k);
}

void dmGL::mousePressEvent(QMouseEvent *e) {

    if (!gedp || !gedp->ged_dmp) {
	QOpenGLWidget::mousePressEvent(e);
	return;
    }

    // Let bview know what the current view width and height are, in
    // case the dx/dy mouse translations need that information
    v->gv_width = width();
    v->gv_height = height();

    if (CADmousePressEvent(v, x_prev, y_prev, e)) {
	dm_set_dirty((struct dm *)gedp->ged_dmp, 1);
	update();
    }

    QOpenGLWidget::mousePressEvent(e);
}

void dmGL::mouseMoveEvent(QMouseEvent *e)
{
    if (!gedp || !gedp->ged_dmp) {
	QOpenGLWidget::mouseMoveEvent(e);
	return;
    }

    // Let bview know what the current view width and height are, in
    // case the dx/dy mouse translations need that information
    v->gv_width = width();
    v->gv_height = height();

    if (CADmouseMoveEvent(v, x_prev, y_prev, e)) {
	 dm_set_dirty((struct dm *)gedp->ged_dmp, 1);
	 update();
    }

    // Current positions are the new previous positions
    x_prev = e->x();
    y_prev = e->y();

    QOpenGLWidget::mouseMoveEvent(e);
}

void dmGL::wheelEvent(QWheelEvent *e) {

    if (!gedp || !gedp->ged_dmp) {
	QOpenGLWidget::wheelEvent(e);
	return;
    }

    // Let bview know what the current view width and height are, in
    // case the dx/dy mouse translations need that information
    v->gv_width = width();
    v->gv_height = height();

    if (CADwheelEvent(v, e)) {
	 dm_set_dirty((struct dm *)gedp->ged_dmp, 1);
	 update();
    }

    QOpenGLWidget::wheelEvent(e);
}

void dmGL::mouseReleaseEvent(QMouseEvent *e) {

    // To avoid an abrupt jump in scene motion the next time movement is
    // started with the mouse, after we release we return to the default state.
    x_prev = -INT_MAX;
    y_prev = -INT_MAX;

    QOpenGLWidget::mouseReleaseEvent(e);
}

void dmGL::save_image() {
    QImage image = this->grabFramebuffer();
    image.save("file.png");
}

// Local Variables:
// tab-width: 8
// mode: C++
// c-basic-offset: 4
// indent-tabs-mode: t
// c-file-style: "stroustrup"
// End:
// ex: shiftwidth=4 tabstop=8

