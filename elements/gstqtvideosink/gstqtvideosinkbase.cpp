/*
    Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies). <qt-info@nokia.com>
    Copyright (C) 2011-2012 Collabora Ltd. <info@collabora.com>

    This library is free software; you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License version 2.1
    as published by the Free Software Foundation.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "gstqtvideosinkbase.h"
#include "gstqtvideosinksurface.h"

#include <QtCore/QCoreApplication>


GstVideoSinkClass *GstQtVideoSinkBase::s_parent_class = NULL;

//------------------------------

GType GstQtVideoSinkBase::get_type()
{
  /* The typedef for GType may be gulong or gsize, depending on the
   * system and whether the compiler is c++ or not. The g_once_init_*
   * functions always take a gsize * though ... */
    static volatile gsize gonce_data = 0;
    if (g_once_init_enter(&gonce_data)) {
        GType type;
        type = gst_type_register_static_full(
            GST_TYPE_VIDEO_SINK,
            g_intern_static_string("GstQtVideoSinkBase"),
            sizeof(GstQtVideoSinkBaseClass),
            &GstQtVideoSinkBase::base_init,
            NULL,   /* base_finalize */
            &GstQtVideoSinkBase::class_init,
            NULL,   /* class_finalize */
            NULL,   /* class_data */
            sizeof(GstQtVideoSinkBase),
            0,      /* n_preallocs */
            &GstQtVideoSinkBase::init,
            NULL,
            (GTypeFlags) 0);
        g_once_init_leave(&gonce_data, (gsize) type);
    }
    return (GType) gonce_data;
}

//------------------------------

void GstQtVideoSinkBase::emit_update(GstQtVideoSinkBase* sink)
{
    GstQtVideoSinkBaseClass *klass = GST_QT_VIDEO_SINK_BASE_GET_CLASS(sink);
    if (klass->update) {
        klass->update(sink);
    }
}

//------------------------------

void GstQtVideoSinkBase::base_init(gpointer g_class)
{
    GstElementClass *element_class = GST_ELEMENT_CLASS(g_class);

    static GstStaticPadTemplate sink_pad_template =
        GST_STATIC_PAD_TEMPLATE("sink", GST_PAD_SINK, GST_PAD_ALWAYS,
            GST_STATIC_CAPS(
                "video/x-raw-rgb, "
                "framerate = (fraction) [ 0, MAX ], "
                "width = (int) [ 1, MAX ], "
                "height = (int) [ 1, MAX ]"
                "; "
                "video/x-raw-yuv, "
                "framerate = (fraction) [ 0, MAX ], "
                "width = (int) [ 1, MAX ], "
                "height = (int) [ 1, MAX ]"
                "; "
            )
        );

    gst_element_class_add_pad_template(
            element_class, gst_static_pad_template_get(&sink_pad_template));
}

void GstQtVideoSinkBase::class_init(gpointer g_class, gpointer class_data)
{
    Q_UNUSED(class_data);

    s_parent_class = reinterpret_cast<GstVideoSinkClass*>(g_type_class_peek_parent(g_class));

    GObjectClass *object_class = G_OBJECT_CLASS(g_class);
    object_class->finalize = GstQtVideoSinkBase::finalize;
    object_class->set_property = GstQtVideoSinkBase::set_property;
    object_class->get_property = GstQtVideoSinkBase::get_property;

    GstElementClass *element_class = GST_ELEMENT_CLASS(g_class);
    element_class->change_state = GstQtVideoSinkBase::change_state;

    GstBaseSinkClass *base_sink_class = GST_BASE_SINK_CLASS(g_class);
    base_sink_class->get_caps = GstQtVideoSinkBase::get_caps;
    base_sink_class->set_caps = GstQtVideoSinkBase::set_caps;

    GstVideoSinkClass *video_sink_class = GST_VIDEO_SINK_CLASS(g_class);
    video_sink_class->show_frame = GstQtVideoSinkBase::show_frame;

    GstQtVideoSinkBaseClass *qt_video_sink_base_class = GST_QT_VIDEO_SINK_BASE_CLASS(g_class);
    qt_video_sink_base_class->update = NULL;


    /**
     * GstQtVideoSinkBase::force-aspect-ratio
     *
     * If set to TRUE, the sink will scale the video respecting its original aspect ratio
     * and any remaining space will be filled with black.
     * If set to FALSE, the sink will scale the video to fit the whole drawing area.
     **/
    g_object_class_install_property(object_class, PROP_FORCE_ASPECT_RATIO,
        g_param_spec_boolean("force-aspect-ratio", "Force aspect ratio",
                             "When enabled, scaling will respect original aspect ratio",
                             FALSE, static_cast<GParamFlags>(G_PARAM_READWRITE)));

}

void GstQtVideoSinkBase::init(GTypeInstance *instance, gpointer g_class)
{
    GstQtVideoSinkBase *sink = GST_QT_VIDEO_SINK_BASE(instance);
    Q_UNUSED(g_class);

    sink->surface = new GstQtVideoSinkSurface(sink);
    sink->formatDirty = true;
}

void GstQtVideoSinkBase::finalize(GObject *object)
{
    GstQtVideoSinkBase *sink = GST_QT_VIDEO_SINK_BASE(object);

    delete sink->surface;
    sink->surface = 0;
}

void GstQtVideoSinkBase::set_property(GObject *object, guint prop_id,
                                  const GValue *value, GParamSpec *pspec)
{
    GstQtVideoSinkBase *sink = GST_QT_VIDEO_SINK_BASE(object);

    switch (prop_id) {
    case PROP_FORCE_ASPECT_RATIO:
        sink->surface->setForceAspectRatio(g_value_get_boolean(value));
        break;
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
        break;
    }
}

void GstQtVideoSinkBase::get_property(GObject *object, guint prop_id,
                                  GValue *value, GParamSpec *pspec)
{
    GstQtVideoSinkBase *sink = GST_QT_VIDEO_SINK_BASE(object);

    switch (prop_id) {
    case PROP_FORCE_ASPECT_RATIO:
        g_value_set_boolean(value, sink->surface->forceAspectRatio());
        break;
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
        break;
    }
}

GstStateChangeReturn GstQtVideoSinkBase::change_state(GstElement *element, GstStateChange transition)
{
    GstQtVideoSinkBase *sink = GST_QT_VIDEO_SINK_BASE(element);

    switch (transition) {
    case GST_STATE_CHANGE_READY_TO_PAUSED:
        sink->surface->setActive(true);
        break;
    case GST_STATE_CHANGE_PAUSED_TO_READY:
        sink->surface->setActive(false);
        break;
    default:
        break;
    }

    return GST_ELEMENT_CLASS(s_parent_class)->change_state(element, transition);
}

GstCaps *GstQtVideoSinkBase::get_caps(GstBaseSink *base)
{
    GstQtVideoSinkBase *sink = GST_QT_VIDEO_SINK_BASE(base);
    GstCaps *caps = gst_caps_new_empty();

    Q_FOREACH(GstVideoFormat format, sink->surface->supportedPixelFormats()) {
        gst_caps_append(caps, BufferFormat::newTemplateCaps(format));
    }

    return caps;
}

gboolean GstQtVideoSinkBase::set_caps(GstBaseSink *base, GstCaps *caps)
{
    GstQtVideoSinkBase *sink = GST_QT_VIDEO_SINK_BASE(base);

    GST_LOG_OBJECT(sink, "new caps %" GST_PTR_FORMAT, caps);
    sink->formatDirty = true;
    return TRUE;
}

GstFlowReturn GstQtVideoSinkBase::show_frame(GstVideoSink *video_sink, GstBuffer *buffer)
{
    GstQtVideoSinkBase *sink = GST_QT_VIDEO_SINK_BASE(video_sink);

    GST_TRACE_OBJECT(sink, "Posting new buffer (%"GST_PTR_FORMAT") for rendering. "
                           "Format dirty: %d", buffer, (int)sink->formatDirty);

    QCoreApplication::postEvent(sink->surface,
            new GstQtVideoSinkSurface::BufferEvent(buffer, sink->formatDirty));

    sink->formatDirty = false;
    return GST_FLOW_OK;
}