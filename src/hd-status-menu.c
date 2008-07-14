/*
 * This file is part of hildon-status-menu
 * 
 * Copyright (C) 2008 Nokia Corporation.
 *
 * Based on hildon-app-menu.c from hildon-widgets.
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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <hildon/hildon.h>
#include <libhildondesktop/libhildondesktop.h>

#include <gdk/gdkx.h>

#include <X11/X.h>
#include <X11/Xatom.h>

#include <string.h>

#include "hd-status-menu.h"
#include "hd-status-menu-box.h"


/* FIXME */
#define STATUS_MENU_INNER_BORDER 4
#define STATUS_MENU_EXTERNAL_BORDER 40

#define STATUS_MENU_ITEM_HEIGHT 70 /* Master Layout Guide */
#define STATUS_MENU_ITEM_WIDTH 332 /* menu items (Master Layout Guide) */

typedef struct _HDStatusMenuPrivate HDStatusMenuPrivate;

struct _HDStatusMenuPrivate {
    GtkWidget *box;
    GtkWidget *pannable;

    GdkWindow *transfer_window;
};

#define HD_STATUS_MENU_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), HD_TYPE_STATUS_MENU, HDStatusMenuPrivate));

G_DEFINE_TYPE (HDStatusMenu, hd_status_menu, GTK_TYPE_WINDOW);

static void
hd_status_menu_init (HDStatusMenu *status_menu)
{
  HDStatusMenuPrivate *priv;
  GtkWidget *hbox, *vbox; /* used to center the pannable */

  priv = HD_STATUS_MENU_GET_PRIVATE (status_menu);

  priv->box = hd_status_menu_box_new ();
  gtk_widget_show (priv->box);

  priv->pannable = hildon_pannable_area_new ();
  g_object_set (G_OBJECT (priv->pannable), "hindicator-mode", HILDON_PANNABLE_AREA_INDICATOR_MODE_HIDE, NULL);
  /* FIXME: ? */
  g_object_set (G_OBJECT (priv->pannable), "vindicator-mode", HILDON_PANNABLE_AREA_INDICATOR_MODE_HIDE, NULL);
  /* FIXME: requires HildonPannableArea from sandbox */
  /* g_object_set (G_OBJECT (priv->pannable), "mov-mode", HILDON_PANNABLE_AREA_INDICATOR_MOV_MODE_VERTICAL, NULL); */
  /* FIXME: use dynamic size */
  gtk_widget_set_size_request (priv->pannable, 656, 280);
  gtk_widget_show (priv->pannable);

  hbox = gtk_hbox_new (FALSE, 0);
  gtk_widget_show (hbox);

  vbox = gtk_hbox_new (FALSE, 0);
  gtk_widget_show (vbox);

  /* Pack containers */
  hildon_pannable_area_add_with_viewport (HILDON_PANNABLE_AREA (priv->pannable), priv->box); 
  gtk_box_pack_start (GTK_BOX (hbox), priv->pannable, FALSE, FALSE, 0);
  gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 0);
  gtk_container_add (GTK_CONTAINER (status_menu), vbox);

  /* Set border */
  /* gtk_container_set_border_width (GTK_CONTAINER (status_menu), STATUS_MENU_INNER_BORDER); */

  gtk_window_set_modal (GTK_WINDOW (status_menu), TRUE);
}

void
hd_status_menu_add_plugin (HDStatusMenu *status_menu,
                           GtkWidget    *plugin)
{
  HDStatusMenuPrivate *priv;

  priv = HD_STATUS_MENU_GET_PRIVATE (status_menu);

  gtk_container_add (GTK_CONTAINER (priv->box), plugin);

  if (GTK_WIDGET_MAPPED (status_menu))
    g_signal_emit_by_name (G_OBJECT (plugin), "status-menu-map", 0);
}

void
hd_status_menu_remove_plugin (HDStatusMenu *status_menu,
                              GtkWidget    *plugin)
{
  HDStatusMenuPrivate *priv;

  priv = HD_STATUS_MENU_GET_PRIVATE (status_menu);

  gtk_container_remove (GTK_CONTAINER (priv->box), plugin);
}

static void
hd_status_menu_finalize (GObject *object)
{
  G_OBJECT_CLASS (hd_status_menu_parent_class)->finalize (object);
}

static void
hd_status_menu_realize (GtkWidget *widget)
{
  GdkDisplay *display;
  Atom atom;
  const gchar *notification_type = "_HILDON_WM_WINDOW_TYPE_APP_MENU";

  GTK_WIDGET_CLASS (hd_status_menu_parent_class)->realize (widget);

  gdk_window_set_decorations (widget->window, GDK_DECOR_BORDER);

  /* Copied from HildonAppMenu */
  display = gdk_drawable_get_display (widget->window);
  atom = gdk_x11_get_xatom_by_name_for_display (display, "_NET_WM_WINDOW_TYPE");
  XChangeProperty (GDK_WINDOW_XDISPLAY (widget->window), GDK_WINDOW_XID (widget->window),
                   atom, XA_STRING, 8, PropModeReplace, (guchar *) notification_type,
                   strlen (notification_type));
}

#if 0
static void
hd_status_menu_size_request (GtkWidget      *widget,
                             GtkRequisition *requisition)
{
  HDStatusMenuPrivate *priv;
  /*  GdkScreen *screen; */
  GtkRequisition child_requisition;

  priv = HD_STATUS_MENU_GET_PRIVATE (widget);

  gtk_widget_size_request (priv->table, &child_requisition);

  /*  screen = gtk_widget_get_screen (widget);
      requisition->width = gdk_screen_get_width (screen) - 2 * STATUS_MENU_EXTERNAL_BORDER;
      requisition->height = MIN (gdk_screen_get_height (screen) - STATUS_MENU_EXTERNAL_BORDER, child_requisition.height + 2 * STATUS_MENU_INNER_BORDER);*/
}
#endif

/* Grab transfer window (based on the one from GtkMenu) */
static GdkWindow *
grab_transfer_window_get (GtkWidget *widget)
{
  GdkWindow *window;
  GdkWindowAttr attributes;
  gint attributes_mask;

  attributes.x = 0;
  attributes.y = 0;
  attributes.width = 10;
  attributes.height = 10;
  attributes.window_type = GDK_WINDOW_TEMP;
  attributes.wclass = GDK_INPUT_ONLY;
  attributes.override_redirect = TRUE;
  attributes.event_mask = 0;

  attributes_mask = GDK_WA_X | GDK_WA_Y | GDK_WA_NOREDIR;

  window = gdk_window_new (gtk_widget_get_root_window (widget),
                           &attributes, attributes_mask);
  gdk_window_set_user_data (window, widget);

  gdk_window_show (window);

  return window;
}

static void
hd_status_menu_emit_void_signal (GtkWidget *widget,
                                 gpointer   data)
{
  g_signal_emit (widget, GPOINTER_TO_UINT (data), 0);
}

static void
hd_status_menu_map (GtkWidget *widget)
{
  HDStatusMenuPrivate *priv;
  guint status_menu_map_id;

  priv = HD_STATUS_MENU_GET_PRIVATE (widget);

  GTK_WIDGET_CLASS (hd_status_menu_parent_class)->map (widget);

  /* Grab pointer and keyboard */
  if (priv->transfer_window == NULL) {
      gboolean has_grab = FALSE;

      priv->transfer_window = grab_transfer_window_get (widget);

      if (gdk_pointer_grab (priv->transfer_window, TRUE,
                            GDK_BUTTON_RELEASE_MASK,
                            NULL, NULL,
                            GDK_CURRENT_TIME) == GDK_GRAB_SUCCESS) {
          if (gdk_keyboard_grab (priv->transfer_window, TRUE,
                                 GDK_CURRENT_TIME) == GDK_GRAB_SUCCESS) {
              has_grab = TRUE;
          } else {
              gdk_display_pointer_ungrab (gtk_widget_get_display (widget),
                                          GDK_CURRENT_TIME);
          }
      }

      if (has_grab) {
          gtk_grab_add (widget);
      } else {
          gdk_window_destroy (priv->transfer_window);
          priv->transfer_window = NULL;
      }
  }

  status_menu_map_id = g_signal_lookup ("status-menu-map", HD_TYPE_STATUS_MENU_ITEM);
  gtk_container_foreach (GTK_CONTAINER (priv->box), hd_status_menu_emit_void_signal, GUINT_TO_POINTER (status_menu_map_id));
}

static void
hd_status_menu_unmap (GtkWidget *widget)
{
  HDStatusMenuPrivate *priv;
  guint status_menu_unmap_id;

  priv = HD_STATUS_MENU_GET_PRIVATE (widget);

  /* Remove the grab */
  if (priv->transfer_window != NULL)
    {
      gdk_display_pointer_ungrab (gtk_widget_get_display (widget),
                                  GDK_CURRENT_TIME);
      gtk_grab_remove (widget);

      gdk_window_destroy (priv->transfer_window);
      priv->transfer_window = NULL;
    }

  status_menu_unmap_id = g_signal_lookup ("status-menu-unmap", HD_TYPE_STATUS_MENU_ITEM);
  gtk_container_foreach (GTK_CONTAINER (priv->box), hd_status_menu_emit_void_signal, GUINT_TO_POINTER (status_menu_unmap_id));

  GTK_WIDGET_CLASS (hd_status_menu_parent_class)->unmap (widget);
}

static gboolean
hd_status_menu_button_release_event (GtkWidget      *widget,
                                     GdkEventButton *event)
{
  int x, y;
  gboolean released_outside;

  gdk_window_get_position (widget->window, &x, &y);

  /* Whether the button has been released outside the widget */
  released_outside = (event->x_root < x || event->x_root > x + widget->allocation.width ||
                      event->y_root < y || event->y_root > y + widget->allocation.height);

  if (released_outside)
    {
      gtk_widget_hide (widget);
    }

  if (GTK_WIDGET_CLASS (hd_status_menu_parent_class)->button_release_event)
    {
      return GTK_WIDGET_CLASS (hd_status_menu_parent_class)->button_release_event (widget, event);
    } 
  else
    {
      return FALSE;
    }
}

static void
hd_status_menu_class_init (HDStatusMenuClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  object_class->finalize = hd_status_menu_finalize;

  widget_class->realize = hd_status_menu_realize;
  widget_class->map = hd_status_menu_map;
  widget_class->unmap = hd_status_menu_unmap;
  widget_class->button_release_event = hd_status_menu_button_release_event;
  /*  widget_class->size_request = hd_status_menu_size_request; */

  g_type_class_add_private (klass, sizeof (HDStatusMenuPrivate));
}

GtkWidget *
hd_status_menu_new (void)
{
  GtkWidget *status_menu;

  status_menu = g_object_new (HD_TYPE_STATUS_MENU,
                              "type", GTK_WINDOW_TOPLEVEL,
                              NULL);

  return status_menu;
}
