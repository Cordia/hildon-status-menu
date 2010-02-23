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
#include <stdlib.h>

#include "hd-status-menu.h"
#include "hd-status-menu-box.h"
#include "hd-status-menu-config.h"

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

#define STATUS_MENU_ITEM_HEIGHT 70 /* Master Layout Guide */
#define STATUS_MENU_ITEM_WIDTH 332 /* menu items (Master Layout Guide) */

#define STATUS_MENU_PANNABLE_WIDTH_LANDSCAPE 656
#define STATUS_MENU_PANNABLE_WIDTH_PORTRAIT 328

#define DSME_SIGNAL_INTERFACE "com.nokia.dsme.signal"
#define DSME_SHUTDOWN_SIGNAL_NAME "shutdown_ind"

enum
{
  PROP_0,
  PROP_PLUGIN_MANAGER
};

struct _HDStatusMenuPrivate
{
  GtkWidget       *box;
  GtkWidget       *pannable;

  HDPluginManager *plugin_manager;

  gboolean         pressed_outside;

  gboolean         portrait;
};

#define HD_STATUS_MENU_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), HD_TYPE_STATUS_MENU, HDStatusMenuPrivate));

G_DEFINE_TYPE (HDStatusMenu, hd_status_menu, GTK_TYPE_WINDOW);

static void
notify_visible_items_cb (HDStatusMenu *status_menu)
{
  HDStatusMenuPrivate *priv = status_menu->priv;
  guint visible_items;

  g_object_get (priv->box,
                "visible-items", &visible_items,
                NULL);

  if (priv->portrait)
    {
      gtk_widget_set_size_request (priv->pannable,
                                   STATUS_MENU_PANNABLE_WIDTH_PORTRAIT,
                                   MIN (MAX (visible_items, 1), 8) * STATUS_MENU_ITEM_HEIGHT);
    }
  else
    {
      gtk_widget_set_size_request (priv->pannable,
                                   STATUS_MENU_PANNABLE_WIDTH_LANDSCAPE,
                                   MIN (MAX ((visible_items + 1) / 2, 1), 5) * STATUS_MENU_ITEM_HEIGHT);
    }
}

static DBusHandlerResult
hd_status_menu_dbus_handler (DBusConnection *conn,
                             DBusMessage *msg, void *data)
{
  if (dbus_message_is_signal(msg, DSME_SIGNAL_INTERFACE,
                             DSME_SHUTDOWN_SIGNAL_NAME))
    {
            /*
      g_warning ("%s: " DSME_SHUTDOWN_SIGNAL_NAME " from DSME", __func__);
      */
      exit (0);
    }

  return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
}

static void
hd_status_menu_init (HDStatusMenu *status_menu)
{
  DBusConnection *sysbus;
  DBusError derror;
  HDStatusMenuPrivate *priv = HD_STATUS_MENU_GET_PRIVATE (status_menu);
  GtkWidget *alignment; /* Used to center the pannable */

  /* Set priv member */
  status_menu->priv = priv;

  /* connect to D-Bus system bus */
  dbus_error_init (&derror);
  sysbus = dbus_bus_get (DBUS_BUS_SYSTEM, &derror);
  if (!sysbus)
    {
      dbus_error_free (&derror);
      g_warning ("%s: failed to connect to the system bus", __func__);
    }
  else
    {
      /* listen to shutdown_ind from DSME */
      dbus_bus_add_match (sysbus, "type='signal', interface='"
                          DSME_SIGNAL_INTERFACE "'", NULL);

      dbus_connection_add_filter (sysbus, hd_status_menu_dbus_handler,
                                  NULL, NULL);
    }

  /* Create widgets */
  priv->box = hd_status_menu_box_new ();
  g_signal_connect_swapped (G_OBJECT (priv->box), "notify::visible-items",
                            G_CALLBACK (notify_visible_items_cb), status_menu);
  gtk_widget_show (priv->box);

  priv->pannable = hildon_pannable_area_new ();
  g_object_set (G_OBJECT (priv->pannable),
                "hscrollbar-policy", GTK_POLICY_NEVER,
                "vscrollbar-policy", GTK_POLICY_AUTOMATIC,
                "mov-mode", HILDON_MOVEMENT_MODE_VERT,
                NULL);
  /* Set the size request of the pannable area (it is automatically updated if
   * the number of visible items in the status menu box changed)
   */
  gtk_widget_set_size_request (priv->pannable,
                               STATUS_MENU_PANNABLE_WIDTH_LANDSCAPE,
                               STATUS_MENU_ITEM_HEIGHT);
  gtk_widget_show (priv->pannable);

  alignment = gtk_alignment_new (0.5, 0.5, 0.0, 0.0);
  gtk_widget_show (alignment);

  /* Pack containers */
  hildon_pannable_area_add_with_viewport (HILDON_PANNABLE_AREA (priv->pannable), priv->box); 
  gtk_container_add (GTK_CONTAINER (alignment), priv->pannable);
  gtk_container_add (GTK_CONTAINER (status_menu), alignment);

  g_signal_connect (status_menu, "delete-event",
                    G_CALLBACK (gtk_widget_hide_on_delete), NULL);

  gtk_window_set_modal (GTK_WINDOW (status_menu), TRUE);

  hildon_gtk_window_set_portrait_flags (GTK_WINDOW (status_menu),
                                        HILDON_PORTRAIT_MODE_SUPPORT);
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
                                             HD_STATUS_MENU_CONFIG_KEY_POSITION,
                                             &error);
  g_free (plugin_id);

  /* Use G_MAXUINT as default position */
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

  /* Get the position from the plugin configuration file */
  position = (guint) g_key_file_get_integer (keyfile,
                                             plugin_id,
                                             HD_STATUS_MENU_CONFIG_KEY_POSITION,
                                             &error);
  g_free (plugin_id);

  /* Use G_MAXUINT as default */
  if (error)
    {
      g_error_free (error);
      position = G_MAXUINT;
    }

  /* Reorder Child */
  hd_status_menu_box_reorder_child (HD_STATUS_MENU_BOX (gtk_widget_get_parent (child)),
                                    child,
                                    position);
}

static void
hd_status_menu_items_configuration_loaded_cb (HDPluginManager *plugin_manager,
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
          g_signal_connect_object (G_OBJECT (priv->plugin_manager), "items-configuration-loaded",
                                   G_CALLBACK (hd_status_menu_items_configuration_loaded_cb), object, 0);
        }
      else
        g_warning ("plugin-manager should not be NULL");
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static gboolean
is_portrait_mode (GtkWidget *widget)
{
  GdkScreen *screen;

  screen = gtk_widget_get_screen (widget);

  return gdk_screen_get_height (screen) > gdk_screen_get_width (screen);
}

static void
update_portrait (HDStatusMenu *status_menu)
{
  HDStatusMenuPrivate *priv = status_menu->priv;

  priv->portrait = is_portrait_mode (GTK_WIDGET (status_menu));

  g_object_set (priv->box,
                "columns", priv->portrait ? 1 : 2,
                NULL);

  notify_visible_items_cb (status_menu);

  hildon_pannable_area_jump_to (HILDON_PANNABLE_AREA (priv->pannable),
                                0, 0);
}

static void
hd_status_menu_realize (GtkWidget *widget)
{
  GdkScreen *screen;
  GdkDisplay *display;
  Atom atom, wm_type;

  screen = gtk_widget_get_screen (widget);
  g_signal_connect_swapped (screen, "size-changed",
                            G_CALLBACK (update_portrait),
                            widget);
  update_portrait (HD_STATUS_MENU (widget));

  GTK_WIDGET_CLASS (hd_status_menu_parent_class)->realize (widget);

  /* Use only a border as decoration */
  gdk_window_set_decorations (widget->window, GDK_DECOR_BORDER);

  /* Set the _NET_WM_WINDOW_TYPE property to _HILDON_WM_WINDOW_TYPE_STATUS_MENU */
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

static void
hd_status_menu_unrealize (GtkWidget *widget)
{
  GdkScreen *screen;

  screen = gtk_widget_get_screen (widget);
  g_signal_handlers_disconnect_by_func (screen,
                                        update_portrait,
                                        HD_STATUS_MENU (widget));

  GTK_WIDGET_CLASS (hd_status_menu_parent_class)->unrealize (widget);
}

static void
hd_status_menu_map (GtkWidget *widget)
{
  HDStatusMenuPrivate *priv = HD_STATUS_MENU (widget)->priv;
  GdkScreen *screen;
  gint window_width;

  GTK_WIDGET_CLASS (hd_status_menu_parent_class)->map (widget);

  update_portrait (HD_STATUS_MENU (widget));

  if (priv->portrait)
    window_width = STATUS_MENU_PANNABLE_WIDTH_PORTRAIT;
  else
    window_width = STATUS_MENU_PANNABLE_WIDTH_LANDSCAPE;

  /* Horizontally center menu */
  screen = gtk_widget_get_screen (widget);
  gtk_window_move (GTK_WINDOW (widget), (gdk_screen_get_width (screen) - window_width) / 2, 0);

  hildon_pannable_area_jump_to (HILDON_PANNABLE_AREA (priv->pannable),
                                0, 0);
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

      /* FIXME check if it is really sized the correct way
       * (works with the hildon wm).
       */
      gtk_widget_queue_resize (widget);

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

      /* Resize children (also if size not changed and so no
       * configure notify event is triggered) */
      gtk_container_resize_children (GTK_CONTAINER (widget));
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
  widget_class->unrealize = hd_status_menu_unrealize;
  widget_class->map = hd_status_menu_map;

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
                              "accept-focus", FALSE,
                              "plugin-manager", plugin_manager,
                              NULL);

  return status_menu;
}
