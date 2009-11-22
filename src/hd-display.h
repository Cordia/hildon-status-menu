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

#ifndef __HD_DISPLAY_H__
#define __HD_DISPLAY_H__

#include <glib-object.h>

G_BEGIN_DECLS

#define HD_TYPE_DISPLAY            (hd_display_get_type ())
#define HD_DISPLAY(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), HD_TYPE_DISPLAY, HDDisplay))
#define HD_DISPLAY_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), HD_TYPE_DISPLAY, HDDisplayClass))
#define HD_IS_DISPLAY(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), HD_TYPE_DISPLAY))
#define HD_IS_DISPLAY_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), HD_TYPE_DISPLAY))
#define HD_DISPLAY_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), HD_TYPE_DISPLAY, HDDisplayClass))

typedef struct _HDDisplay        HDDisplay;
typedef struct _HDDisplayClass   HDDisplayClass;
typedef struct _HDDisplayPrivate HDDisplayPrivate;

struct _HDDisplay
{
  GObject parent;

  HDDisplayPrivate *priv;
};

struct _HDDisplayClass
{
  GObjectClass parent;
};

GType      hd_display_get_type   (void);

HDDisplay *hd_display_get        (void);

gboolean   hd_display_is_on      (HDDisplay *display);

G_END_DECLS

#endif

