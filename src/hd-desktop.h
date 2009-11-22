/*
 * This file is part of hildon-home
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

#ifndef __HD_DESKTOP_H__
#define __HD_DESKTOP_H__

#include <glib-object.h>

G_BEGIN_DECLS

#define HD_TYPE_DESKTOP            (hd_desktop_get_type ())
#define HD_DESKTOP(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), HD_TYPE_DESKTOP, HDDesktop))
#define HD_DESKTOP_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), HD_TYPE_DESKTOP, HDDesktopClass))
#define HD_IS_DESKTOP(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), HD_TYPE_DESKTOP))
#define HD_IS_DESKTOP_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), HD_TYPE_DESKTOP))
#define HD_DESKTOP_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), HD_TYPE_DESKTOP, HDDesktopClass))

typedef struct _HDDesktop        HDDesktop;
typedef struct _HDDesktopClass   HDDesktopClass;
typedef struct _HDDesktopPrivate HDDesktopPrivate;

struct _HDDesktop
{
  GObject parent;

  HDDesktopPrivate *priv;
};

struct _HDDesktopClass
{
  GObjectClass parent;
};

GType      hd_desktop_get_type (void);

HDDesktop *hd_desktop_get      (void);

gboolean   hd_desktop_is_task_switcher_visible (HDDesktop *desktop);

G_END_DECLS

#endif

