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

#ifndef __HD_STATUS_MENU_H__
#define __HD_STATUS_MENU_H__

#include <glib-object.h>

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define HD_TYPE_STATUS_MENU             (hd_status_menu_get_type ())
#define HD_STATUS_MENU(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), HD_TYPE_STATUS_MENU, HDStatusMenu))
#define HD_STATUS_MENU_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), HD_TYPE_STATUS_MENU, HDStatusMenuClass))
#define HD_IS_STATUS_MENU(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), HD_TYPE_STATUS_MENU))
#define HD_IS_STATUS_MENU_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), HD_TYPE_STATUS_MENU))
#define HD_STATUS_MENU_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), HD_TYPE_STATUS_MENU, HDStatusMenuClass))

typedef struct _HDStatusMenuClass HDStatusMenuClass;
typedef struct _HDStatusMenu HDStatusMenu;

struct _HDStatusMenuClass
{
  GtkWindowClass parent_class;
};

struct _HDStatusMenu
{
  GtkWindow parent_instance;
};

GType      hd_status_menu_get_type      (void) G_GNUC_CONST;

GtkWidget *hd_status_menu_new           (void);

void       hd_status_menu_add_plugin    (HDStatusMenu *status_menu,
                                         GtkWidget    *plugin);

void       hd_status_menu_remove_plugin (HDStatusMenu *status_menu,
                                         GtkWidget    *plugin);

G_END_DECLS

#endif /* __HD_STATUS_MENU_H__ */

