/*
 *  test-surfaces.c - Test GstVaapiSurface and GstVaapiSurfacePool
 *
 *  gstreamer-vaapi (C) 2010 Splitted-Desktop Systems
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
 */

#include <gst/vaapi/gstvaapisurface.h>
#include <gst/vaapi/gstvaapisurfacepool.h>
#include <gst/vaapi/gstvaapidisplay_x11.h>

#define MAX_SURFACES 4

int
main(int argc, char *argv[])
{
    GstVaapiDisplay *display;
    GstVaapiSurface *surface;
    GstVaapiSurface *surfaces[MAX_SURFACES];
    GstVaapiVideoPool *pool;
    GstCaps *caps;
    gint i;

    static const GstVaapiChromaType chroma_type = GST_VAAPI_CHROMA_TYPE_YUV420;
    static const guint              width       = 320;
    static const guint              height      = 240;

    gst_init(&argc, &argv);

    display = gst_vaapi_display_x11_new(NULL);
    if (!display)
        g_error("could not create Gst/VA display");

    surface = gst_vaapi_surface_new(display, chroma_type, width, height);
    if (!surface)
        g_error("could not create Gst/VA surface");
    g_object_unref(surface);

    caps = gst_caps_new_simple(
        "video/x-vaapi-surface",
        "width", G_TYPE_INT, width,
        "height", G_TYPE_INT, height,
        NULL
    );
    if (!caps)
        g_error("cound not create Gst/VA surface caps");

    pool = gst_vaapi_surface_pool_new(display, caps);
    if (!pool)
        g_error("could not create Gst/VA surface pool");

    for (i = 0; i < MAX_SURFACES; i++) {
        surface = gst_vaapi_video_pool_get_object(pool);
        if (!surface)
            g_error("could not allocate Gst/VA surface from pool");
        g_print("created surface 0x%08x from pool\n",
                gst_vaapi_surface_get_id(surface));
        surfaces[i] = surface;
    }

    /* Check the pool doesn't return the last free'd surface */
    surface = g_object_ref(surfaces[1]);

    for (i = 0; i < 2; i++)
        gst_vaapi_video_pool_put_object(pool, surfaces[i]);

    for (i = 0; i < 2; i++) {
        surfaces[i] = gst_vaapi_video_pool_get_object(pool);
        if (!surfaces[i])
            g_error("could not re-allocate Gst/VA surface%d from pool", i);
        g_print("created surface 0x%08x from pool (realloc)\n",
                gst_vaapi_surface_get_id(surfaces[i]));
    }

    if (surface == surfaces[0])
        g_error("Gst/VA pool doesn't queue free surfaces");

    for (i = MAX_SURFACES - 1; i >= 0; i--) {
        if (!surfaces[i])
            continue;
        gst_vaapi_video_pool_put_object(pool, surfaces[i]);
        surfaces[i] = NULL;
    }

    /* Unref in random order to check objects are correctly refcounted */
    g_print("unref display\n");
    g_object_unref(display);
    gst_caps_unref(caps);
    g_print("unref pool\n");
    g_object_unref(pool);
    g_print("unref surface\n");
    g_object_unref(surface);
    gst_deinit();
    return 0;
}
