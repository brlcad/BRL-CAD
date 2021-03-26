/*                      D M G L . C P P
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
/** @file dmgl.cpp
 *
 * Brief description
 *
 */

#include <QOpenGLWidget>
#include <QKeyEvent>
#include <QGuiApplication> // for qGuiApp

extern "C" {
#include "bu/parallel.h"
#include "ged.h"
}
#include "dmgl.h"

DMRenderer::DMRenderer(dmGL *w)
    : m_w(w)
{
}

DMRenderer::~DMRenderer()
{
}

void DMRenderer::resize()
{
}

void DMRenderer::render()
{
    if (m_exiting)
	return;

    int w = m_w->width();
    int h = m_w->height();
    // Zero size == nothing to do
    if (!w || !h)
	return;

    if (!changed) {
	// Avoid a hot spin
	usleep(10000);
	return;
    }

    // Since we're in a separate rendering thread, there is
    // some preliminary work we need to do before proceeding
    // with OpenGL calls
    QOpenGLContext *ctx = m_w->context();
    if (!ctx) // QOpenGLWidget not yet initialized
	return;
    // Grab the context.
    m_grabMutex.lock();
    emit contextWanted();
    m_grabCond.wait(&m_grabMutex);
    QMutexLocker lock(&m_renderMutex);
    m_grabMutex.unlock();
    if (m_exiting)
	return;
    Q_ASSERT(ctx->thread() == QThread::currentThread());


    // Have context, initialize if necessary
    m_w->makeCurrent();
    if (!m_init) {
	initializeOpenGLFunctions();
	m_init = true;
	if (!dmp) {
	    const char *acmd = "attach";
	    dmp = dm_open((void *)m_w, "qtgl", 1, &acmd);
	}
    }

    // TODO - libdm drawing calls


    // Make no context current on this thread and move the QOpenGLWidget's
    // context back to the gui thread.
    m_w->doneCurrent();
    ctx->moveToThread(qGuiApp->thread());

    // Schedule composition. Note that this will use QueuedConnection, meaning
    // that update() will be invoked on the gui thread.
    QMetaObject::invokeMethod(m_w, "update");

    // Avoid a hot spin
    usleep(10000);
}

dmGL::dmGL(QWidget *parent)
    : QOpenGLWidget(parent)
{
    BU_GET(v, struct bview);
    ged_view_init(v);

    connect(this, &QOpenGLWidget::aboutToCompose, this, &dmGL::onAboutToCompose);
    connect(this, &QOpenGLWidget::frameSwapped, this, &dmGL::onFrameSwapped);
    connect(this, &QOpenGLWidget::aboutToResize, this, &dmGL::onAboutToResize);
    connect(this, &QOpenGLWidget::resized, this, &dmGL::onResized);

    m_thread = new QThread;

    // Create the renderer
    m_renderer = new DMRenderer(this);
    m_renderer->moveToThread(m_thread);
    connect(m_thread, &QThread::finished, m_renderer, &QObject::deleteLater);

    // Let dmGL know to trigger the renderer
    connect(this, &dmGL::renderRequested, m_renderer, &DMRenderer::render);
    connect(m_renderer, &DMRenderer::contextWanted, this, &dmGL::grabContext);

    m_thread->start();

    // This is an important Qt setting for interactivity - it allowing key
    // bindings to propagate to this widget and trigger actions such as
    // resolution scaling, rotation, etc.
    setFocusPolicy(Qt::WheelFocus);
}

dmGL::~dmGL()
{
    m_renderer->prepareExit();
    m_thread->quit();
    m_thread->wait();
    delete m_thread;
    BU_PUT(v, struct bview);
}

void dmGL::onAboutToCompose()
{
    // We are on the gui thread here. Composition is about to
    // begin. Wait until the render thread finishes.
    m_renderer->lockRenderer();
}

void dmGL::onFrameSwapped()
{
    m_renderer->unlockRenderer();
    // Assuming a blocking swap, our animation is driven purely by the
    // vsync in this example.
    emit renderRequested();
}

void dmGL::onAboutToResize()
{
    m_renderer->lockRenderer();
}

void dmGL::onResized()
{
    m_renderer->changed = true;
    m_renderer->unlockRenderer();
}

void dmGL::grabContext()
{
    if (m_renderer->m_exiting)
	return;
    m_renderer->lockRenderer();
    QMutexLocker lock(m_renderer->grabMutex());
    context()->moveToThread(m_thread);
    m_renderer->grabCond()->wakeAll();
    m_renderer->unlockRenderer();
}

void dmGL::keyPressEvent(QKeyEvent *k) {
    //QString kstr = QKeySequence(k->key()).toString();
    //bu_log("%s\n", kstr.toStdString().c_str());
#if 0
    switch (k->key()) {
	case '=':
	    m_renderer->res_incr();
	    emit renderRequested();
	    update();
	    return;
	    break;
	case '-':
	    m_renderer->res_decr();
	    emit renderRequested();
	    update();
	    return;
	    break;
    }
#endif
    QOpenGLWidget::keyPressEvent(k);
}


void dmGL::mouseMoveEvent(QMouseEvent *e) {

    bu_log("(%d,%d)\n", e->x(), e->y());
    if (x_prev > -INT_MAX && y_prev > -INT_MAX) {
	bu_log("Delta: (%d,%d)\n", e->x() - x_prev, e->y() - y_prev);
    }

    x_prev = e->x();
    y_prev = e->y();

    QOpenGLWidget::mouseMoveEvent(e);
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

