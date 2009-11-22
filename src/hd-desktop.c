/*
 * This file is part of hildon-desktop
 *
 * Copyright (C) 2009 Nokia Corporation.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * as published by the Free Software Foundation; either version 2.1 of
 * the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
 * 02110-1301 USA
 *
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <gdk/gdkx.h>

#include "hd-desktop.h"

#define HD_DESKTOP_GET_PRIVATE(object) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((object), HD_TYPE_DESKTOP, HDDesktopPrivate))

struct _HDDesktopPrivate
{
  GdkWindow *root_window;

  gboolean task_switcher_shown;
};

enum
{
  TASK_SWITCHER_SHOW,
  TASK_SWITCHER_HIDE,

  LAST_SIGNAL
};

static guint desktop_signals[LAST_SIGNAL] = { 0, };

static void hd_desktop_dispose (GObject *object);

static void initialize_root_window (HDDesktop *desktop);
static GdkFilterReturn filter_property_changed (GdkXEvent *xevent,
                                                GdkEvent  *event,
                                                gpointer   data);

G_DEFINE_TYPE (HDDesktop, hd_desktop, G_TYPE_OBJECT);

HDDesktop *
hd_desktop_get (void)
{
  static gpointer desktop = NULL;

  if (desktop == NULL)
    {
      desktop = g_object_new (HD_TYPE_DESKTOP,
                              NULL);
      g_object_add_weak_pointer (desktop, &desktop);
      return desktop;
    }
  else
    {
      return g_object_ref (desktop);
    }
}

static void
hd_desktop_class_init (HDDesktopClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->dispose = hd_desktop_dispose;

  desktop_signals[TASK_SWITCHER_SHOW] = g_signal_new ("task-switcher-show",
                                                      HD_TYPE_DESKTOP,
                                                      0, 0,
                                                      NULL, NULL,
                                                      g_cclosure_marshal_VOID__VOID,
                                                      G_TYPE_NONE,
                                                      0);
  desktop_signals[TASK_SWITCHER_HIDE] = g_signal_new ("task-switcher-hide",
                                                      HD_TYPE_DESKTOP,
                                                      0, 0,
                                                      NULL, NULL,
                                                      g_cclosure_marshal_VOID__VOID,
                                                      G_TYPE_NONE,
                                                      0);

  g_type_class_add_private (klass, sizeof (HDDesktopPrivate));
}

static void
hd_desktop_init (HDDesktop *desktop)
{
  desktop->priv = HD_DESKTOP_GET_PRIVATE (desktop);

  initialize_root_window (desktop);
}

static void
initialize_root_window (HDDesktop *desktop)
{
  HDDesktopPrivate *priv = desktop->priv;

  priv->root_window = gdk_window_foreign_new_for_display (gdk_display_get_default (),
                                                          gdk_x11_get_default_root_xwindow ());

  gdk_window_set_events (priv->root_window,
                         gdk_window_get_events (priv->root_window) | GDK_PROPERTY_CHANGE_MASK);

  gdk_window_add_filter (priv->root_window,
                         filter_property_changed,
                         desktop);
}

static GdkFilterReturn
filter_property_changed (GdkXEvent *xevent,
                         GdkEvent  *event,
                         gpointer   data)
{
  HDDesktop *desktop = data;
  HDDesktopPrivate *priv = desktop->priv;

  XEvent *ev = (XEvent *) xevent;

  if (ev->type == PropertyNotify)
    {
      if (ev->xproperty.atom == gdk_x11_get_xatom_by_name ("_MB_CURRENT_APP_WINDOW"))
        {
          Atom actual_type;
          int actual_format;
          unsigned long nitems, bytes;
          unsigned char *atom_data = NULL;

          if (XGetWindowProperty (GDK_WINDOW_XDISPLAY (priv->root_window),
                                  GDK_WINDOW_XID (priv->root_window),
                                  gdk_x11_get_xatom_by_name ("_MB_CURRENT_APP_WINDOW"),
                                  0,
                                  (~0L),
                                  False,
                                  AnyPropertyType,
                                  &actual_type,
                                  &actual_format,
                                  &nitems,
                                  &bytes,
                                  &atom_data) == Success)
            {
              if (nitems == 1) {
                  guint32 *new_value = (void *) atom_data;

                  if (*new_value == 0xFFFFFFFF)
                    {
                      if (!priv->task_switcher_shown)
                        {
                          priv->task_switcher_shown = TRUE;
                          g_signal_emit (desktop,
                                         desktop_signals [TASK_SWITCHER_SHOW],
                                         0);
                        }
                    }
                  else
                    {
                      if (priv->task_switcher_shown)
                        {
                          priv->task_switcher_shown = FALSE;
                          g_signal_emit (desktop,
                                         desktop_signals [TASK_SWITCHER_HIDE],
                                         0);
                        }
                    }
              }
            }
	  
          if (atom_data)
            XFree (atom_data);
        }
    }

  return GDK_FILTER_CONTINUE;
}

static void
hd_desktop_dispose (GObject *object)
{
  HDDesktop *desktop = HD_DESKTOP (object);
  HDDesktopPrivate *priv = desktop->priv;

  if (priv->root_window)
    priv->root_window = (g_object_unref (priv->root_window), NULL);

  G_OBJECT_CLASS (hd_desktop_parent_class)->dispose (object);
}

gboolean
hd_desktop_is_task_switcher_visible (HDDesktop *desktop)
{
  g_return_val_if_fail (HD_IS_DESKTOP (desktop), FALSE);

  return desktop->priv->task_switcher_shown;
}
