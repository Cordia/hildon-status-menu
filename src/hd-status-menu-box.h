/*
 * This file is part of hildon-status-menu
 * 
 * Copyright (C) 2008 Nokia Corporation.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * version 2.1 as published by the Free Software Foundation.
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

#ifndef __HD_STATUS_MENU_BOX_H__
#define __HD_STATUS_MENU_BOX_H__

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define HD_TYPE_STATUS_MENU_BOX             (hd_status_menu_box_get_type ())
#define HD_STATUS_MENU_BOX(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), HD_TYPE_STATUS_MENU_BOX, HDStatusMenuBox))
#define HD_STATUS_MENU_BOX_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), HD_TYPE_STATUS_MENU_BOX, HDStatusMenuBoxClass))
#define HD_IS_STATUS_MENU_BOX(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), HD_TYPE_STATUS_MENU_BOX))
#define HD_IS_STATUS_MENU_BOX_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), HD_TYPE_STATUS_MENU_BOX))
#define HD_STATUS_MENU_BOX_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), HD_TYPE_STATUS_MENU_BOX, HDStatusMenuBoxClass))

typedef struct _HDStatusMenuBox        HDStatusMenuBox;
typedef struct _HDStatusMenuBoxClass   HDStatusMenuBoxClass;
typedef struct _HDStatusMenuBoxPrivate HDStatusMenuBoxPrivate;

struct _HDStatusMenuBox
{
  GtkContainer            parent;

  HDStatusMenuBoxPrivate *priv;
};

struct _HDStatusMenuBoxClass
{
  GtkContainerClass parent;
};

GType      hd_status_menu_box_get_type (void) G_GNUC_CONST;

GtkWidget *hd_status_menu_box_new      (void);

G_END_DECLS

#endif /* __HD_STATUS_MENU_BOX_H__ */

