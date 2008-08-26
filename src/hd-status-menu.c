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

/**
 * SECTION:hdstatusmenu
 * @short_description: A Status Menu
 * @include: hd-status-menu.h 
 * 
 * #HDStatusMenu is a Status Menu. Items in the Status Menu, which should show
 * status information, are subclasses of #HDStatusMenuItem and are implemented 
 * as plugins.
 *
 * See #HDStatusMenuItem for more information.
 *
 **/


/* FIXME Use the pixel sizes from the layout guide  */
#define STATUS_MENU_INNER_BORDER 4
#define STATUS_MENU_EXTERNAL_BORDER 40

#define STATUS_MENU_ITEM_HEIGHT 70 /* Master Layout Guide */
#define STATUS_MENU_ITEM_WIDTH 332 /* menu items (Master Layout Guide) */

enum
{
  PROP_0,
  PROP_PLUGIN_MANAGER
};

struct _HDStatusMenuPrivate
{
  GtkWidget       *box;
  GtkWidget       *pannable;

  GdkWindow       *transfer_window;

  HDPluginManager *plugin_manager;
};

#define HD_STATUS_MENU_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), HD_TYPE_STATUS_MENU, HDStatusMenuPrivate));

G_DEFINE_TYPE (HDStatusMenu, hd_status_menu, GTK_TYPE_WINDOW);

static void
notify_visible_items_cb (GObject      *object,
                         GParamSpec   *spec,
                         HDStatusMenu *status_menu)
{
  HDStatusMenuPrivate *priv = status_menu->priv;
  guint visible_items;

  g_object_get (object,
                "visible-items", &visible_items,
                NULL);

  gtk_widget_set_size_request (priv->pannable,
                               656,
                               MIN (MAX ((visible_items + 1) / 2, 1), 5) * STATUS_MENU_ITEM_HEIGHT);
}

static void
hd_status_menu_init (HDStatusMenu *status_menu)
{
  HDStatusMenuPrivate *priv = HD_STATUS_MENU_GET_PRIVATE (status_menu);
  GtkWidget *alignment; /* Used to center the pannable */

  /* Set priv member */
  status_menu->priv = priv;

  /* Create widgets */
  priv->box = hd_status_menu_box_new ();
  g_signal_connect (G_OBJECT (priv->box), "notify::visible-items",
                    G_CALLBACK (notify_visible_items_cb), status_menu);
  gtk_widget_show (priv->box);

  priv->pannable = hildon_pannable_area_new ();
  g_object_set (G_OBJECT (priv->pannable),
                "hscrollbar-policy", GTK_POLICY_NEVER,
                "vscrollbar-policy", GTK_POLICY_AUTOMATIC,
/* FIXME: unreleased hildon-1 >= 2.1.5 required
                "mov-mode", HILDON_MOVEMENT_MODE_VERT, */
                NULL);
  /* Set the size request of the pannable area (it is automatically updated if
   * the number of visible items in the status menu box changed)
   */
  gtk_widget_set_size_request (priv->pannable, 656, STATUS_MENU_ITEM_HEIGHT);
  gtk_widget_show (priv->pannable);

  alignment = gtk_alignment_new (0.5, 0.5, 0.0, 0.0);
  gtk_widget_show (alignment);

  /* Pack containers */
  hildon_pannable_area_add_with_viewport (HILDON_PANNABLE_AREA (priv->pannable), priv->box); 
  gtk_container_add (GTK_CONTAINER (alignment), priv->pannable);
  gtk_container_add (GTK_CONTAINER (status_menu), alignment);

  /* Set border */
  /* gtk_container_set_border_width (GTK_CONTAINER (status_menu), STATUS_MENU_INNER_BORDER); */

  gtk_window_set_modal (GTK_WINDOW (status_menu), TRUE);
}

static void
hd_status_menu_dispose (GObject *object)
{
  HDStatusMenuPrivate *priv = HD_STATUS_MENU (object)->priv;

  if (priv->plugin_manager)
    {
      g_object_unref (priv->plugin_manager);
      priv->plugin_manager = NULL;
    }

  G_OBJECT_CLASS (hd_status_menu_parent_class)->dispose (object);
}

static void
hd_status_menu_plugin_added_cb (HDPluginManager *plugin_manager,
                                GObject         *plugin,
                                HDStatusMenu    *status_menu)
{
  HDStatusMenuPrivate *priv = status_menu->priv;
  gchar *plugin_id;
  GKeyFile *keyfile;
  guint position;
  GError *error = NULL;

  /* Plugin must be a HDStatusMenuItem */
  if (!HD_IS_STATUS_MENU_ITEM (plugin))
    return;

  /* Read position in Status Menu from plugin configuration */
  keyfile = hd_plugin_manager_get_plugin_config_key_file (plugin_manager);
  plugin_id = hd_plugin_item_get_plugin_id (HD_PLUGIN_ITEM (plugin));

  position = (guint) g_key_file_get_integer (keyfile,
                                             plugin_id,
                                             "X-Status-Menu-Position",
                                             &error);
  g_free (plugin_id);

  if (error)
    {
      g_error_free (error);
      position = G_MAXUINT;
    }

  /* Pack the plugin into the box. The plugin is responsible to show 
   * the widget (required to support temporary visible items).
   */
  hd_status_menu_box_pack (HD_STATUS_MENU_BOX (priv->box), GTK_WIDGET (plugin), position);
}

static void
hd_status_menu_plugin_removed_cb (HDPluginManager *plugin_manager,
                                  GObject         *plugin,
                                  HDStatusMenu    *status_menu)
{
  HDStatusMenuPrivate *priv = status_menu->priv;

  /* Plugin must be a HDStatusMenuItem */
  if (!HD_IS_STATUS_MENU_ITEM (plugin))
    return;

  /* Remove the plugin from the container (and destroy it) */
  gtk_container_remove (GTK_CONTAINER (priv->box), GTK_WIDGET (plugin));
}

static void
update_position (GtkWidget *child,
                 GKeyFile  *keyfile)
{
  gchar *plugin_id;
  guint position;
  GError *error = NULL;

  plugin_id = hd_plugin_item_get_plugin_id (HD_PLUGIN_ITEM (child));

  position = (guint) g_key_file_get_integer (keyfile,
                                             plugin_id,
                                             "X-Status-Menu-Position",
                                             &error);
  g_free (plugin_id);

  if (error)
    {
      g_error_free (error);
      position = G_MAXUINT;
    }

  hd_status_menu_box_reorder_child (HD_STATUS_MENU_BOX (gtk_widget_get_parent (child)),
                                    child,
                                    position);
}

static void
hd_status_menu_plugin_configuration_loaded_cb (HDPluginManager *plugin_manager,
                                               GKeyFile        *key_file,
                                               HDStatusMenu    *status_menu)
{
  HDStatusMenuPrivate *priv = status_menu->priv;

  gtk_container_foreach (GTK_CONTAINER (priv->box), (GtkCallback) update_position, key_file);
}

static void
hd_status_menu_set_property (GObject      *object,
                             guint         prop_id,
                             const GValue *value,
                             GParamSpec   *pspec)
{
  HDStatusMenuPrivate *priv = HD_STATUS_MENU (object)->priv;

  switch (prop_id)
    {
    case PROP_PLUGIN_MANAGER:
      /* The property is CONSTRUCT_ONLY so there is no value yet */
      priv->plugin_manager = g_value_dup_object (value);
      if (priv->plugin_manager != NULL)
        {
          g_signal_connect_object (G_OBJECT (priv->plugin_manager), "plugin-added",
                                   G_CALLBACK (hd_status_menu_plugin_added_cb), object, 0);
          g_signal_connect_object (G_OBJECT (priv->plugin_manager), "plugin-removed",
                                   G_CALLBACK (hd_status_menu_plugin_removed_cb), object, 0);
          g_signal_connect_object (G_OBJECT (priv->plugin_manager), "plugin-configuration-loaded",
                                   G_CALLBACK (hd_status_menu_plugin_configuration_loaded_cb), object, 0);
        }
      else
        g_warning ("plugin-manager should not be NULL");
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
hd_status_menu_realize (GtkWidget *widget)
{
  GdkDisplay *display;
  Atom atom, wm_type;

  GTK_WIDGET_CLASS (hd_status_menu_parent_class)->realize (widget);

  /* Use only a border as decoration */
  gdk_window_set_decorations (widget->window, GDK_DECOR_BORDER);

  /* Set the _NET_WM_WINDOW_TYPE property to _HILDON_WM_WINDOW_TYPE_STATUS_AREA */
  display = gdk_drawable_get_display (widget->window);
  atom = gdk_x11_get_xatom_by_name_for_display (display,
                                                "_NET_WM_WINDOW_TYPE");
  wm_type = gdk_x11_get_xatom_by_name_for_display (display,
                                                   "_HILDON_WM_WINDOW_TYPE_STATUS_MENU");

  XChangeProperty (GDK_WINDOW_XDISPLAY (widget->window),
                   GDK_WINDOW_XID (widget->window),
                   atom, XA_ATOM, 32, PropModeReplace,
                   (unsigned char *)&wm_type, 1);
}

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
hd_status_menu_map (GtkWidget *widget)
{
  HDStatusMenuPrivate *priv = HD_STATUS_MENU (widget)->priv;

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
}

static void
hd_status_menu_unmap (GtkWidget *widget)
{
  HDStatusMenuPrivate *priv = HD_STATUS_MENU (widget)->priv;

  /* Remove the grab */
  if (priv->transfer_window != NULL)
    {
      gdk_display_pointer_ungrab (gtk_widget_get_display (widget),
                                  GDK_CURRENT_TIME);
      gtk_grab_remove (widget);

      gdk_window_destroy (priv->transfer_window);
      priv->transfer_window = NULL;
    }

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
hd_status_menu_check_resize (GtkContainer *container)
{
  GtkWindow *window = GTK_WINDOW (container);
  GtkWidget *widget = GTK_WIDGET (container);

  /* Handle a resize based on a configure notify event
   *
   * Assign size and position of the widget with a call to
   * gtk_widget_size_allocate ().
   */
  if (window->configure_notify_received)
    { 
      GtkAllocation allocation;

      window->configure_notify_received = FALSE;

      /* gtk_window_configure_event() filled in widget->allocation */
      allocation = widget->allocation;
      gtk_widget_size_allocate (widget, &allocation);

      gdk_window_process_updates (widget->window, TRUE);
      
      gdk_window_configure_finished (widget->window);

      return;
    }

  /* Handle a resize based on a change in size request */
  if (GTK_WIDGET_VISIBLE (container))
    {
      GtkRequisition req;

      gtk_widget_size_request (widget, &req);

      /* Request the window manager to resize the window to
       * the required size (will result in a configure notify event
       * see above) */
      gdk_window_resize (widget->window, req.width, req.height);
    }
}

static void
hd_status_menu_class_init (HDStatusMenuClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);
  GtkContainerClass *container_class = GTK_CONTAINER_CLASS (klass);

  object_class->dispose = hd_status_menu_dispose;
  object_class->set_property = hd_status_menu_set_property;

  widget_class->realize = hd_status_menu_realize;
  widget_class->map = hd_status_menu_map;
  widget_class->unmap = hd_status_menu_unmap;
  widget_class->button_release_event = hd_status_menu_button_release_event;

  container_class->check_resize = hd_status_menu_check_resize;

  g_object_class_install_property (object_class,
                                   PROP_PLUGIN_MANAGER,
                                   g_param_spec_object ("plugin-manager",
                                                        "Plugin Manager",
                                                        "The plugin manager which should be used",
                                                        HD_TYPE_PLUGIN_MANAGER,
                                                        G_PARAM_WRITABLE | G_PARAM_CONSTRUCT_ONLY));

  g_type_class_add_private (klass, sizeof (HDStatusMenuPrivate));
}

/**
 * hd_status_menu_new:
 * @plugin_manager a #HDPluginManager used to load the plugins
 *
 * Create a new Status Menu window.
 *
 * Returns: a new #HDStatusMenu.
 **/
GtkWidget *
hd_status_menu_new (HDPluginManager *plugin_manager)
{
  GtkWidget *status_menu;

  status_menu = g_object_new (HD_TYPE_STATUS_MENU,
                              "type", GTK_WINDOW_TOPLEVEL,
                              "plugin-manager", plugin_manager,
                              NULL);

  return status_menu;
}
