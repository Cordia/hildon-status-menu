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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <gdk/gdkx.h>

#include <X11/X.h>
#include <X11/Xatom.h>

#include <string.h>

#include "hd-status-area-box.h"
#include "hd-status-menu.h"
#include "hd-status-menu-config.h"

#include "hd-status-area.h"

/* UI Style guide */
#define STATUS_AREA_HEIGHT 56
#define MINIMUM_STATUS_AREA_WIDTH 112

#define STATUS_AREA_ICON_HEIGHT 16
#define SPECIAL_ICON_WIDTH 48

/* Configuration file keys */

#define HD_STATUS_AREA_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), HD_TYPE_STATUS_AREA, HDStatusAreaPrivate));

static GQuark      quark_hd_status_area_image  = 0;
static const gchar hd_status_area_image[] = "hd_status_area_image";

enum
{
  PROP_0,
  PROP_PLUGIN_MANAGER
};

struct _HDStatusAreaPrivate
{
  HDPluginManager *plugin_manager;

  GtkWidget *status_menu;

  GtkWidget *icon_box;

  GtkWidget *signal_image, *battery_image;

  GtkWidget *clock_box;
};

G_DEFINE_TYPE (HDStatusArea, hd_status_area, GTK_TYPE_WINDOW);

static gboolean
button_release_event_cb (GtkWidget      *widget,
                         GdkEventButton *event,
                         HDStatusArea   *status_area)
{
  HDStatusAreaPrivate *priv = status_area->priv;

  gtk_widget_show (priv->status_menu);

  return TRUE;
}

static void
hd_status_area_init (HDStatusArea *status_area)
{
  HDStatusAreaPrivate *priv = HD_STATUS_AREA_GET_PRIVATE (status_area);

  /* UI Style guide */
  GtkWidget *ebox, *alignment, *main_hbox, *left_vbox;

  GtkWidget *left_top_row, *left_hsep;

  /* Set priv member */
  status_area->priv = priv;

  /* Create Status area UI */
  ebox = gtk_event_box_new ();
  gtk_widget_add_events (GTK_WIDGET (ebox), GDK_BUTTON_RELEASE_MASK);
  g_signal_connect (G_OBJECT (ebox), "button-release-event",
                    G_CALLBACK (button_release_event_cb), status_area);
  gtk_widget_show (ebox);

  alignment = gtk_alignment_new (0.5, 0.5, 0.0, 0.0);
  gtk_alignment_set_padding (GTK_ALIGNMENT (alignment), 4, 10, 10, 10);
  gtk_widget_show (alignment);

  main_hbox = gtk_hbox_new (FALSE, 8);
  gtk_widget_show (main_hbox);

  left_vbox = gtk_vbox_new (FALSE, 0);  
  gtk_widget_show (left_vbox);

  left_top_row = gtk_hbox_new (TRUE, 0);
  gtk_widget_set_size_request (left_top_row, SPECIAL_ICON_WIDTH * 2, STATUS_AREA_ICON_HEIGHT);
  gtk_widget_show (left_top_row);

  left_hsep = gtk_hseparator_new ();
  gtk_widget_set_size_request (left_hsep, -1, 8);
  gtk_widget_show (left_hsep);

  priv->clock_box = gtk_hbox_new (FALSE, 0);
  gtk_widget_set_size_request (priv->clock_box, SPECIAL_ICON_WIDTH * 2, STATUS_AREA_ICON_HEIGHT);
  gtk_widget_show (priv->clock_box);

  priv->signal_image = gtk_image_new ();
  gtk_widget_set_size_request (priv->signal_image, SPECIAL_ICON_WIDTH, STATUS_AREA_ICON_HEIGHT);
  gtk_widget_show (priv->signal_image);

  priv->battery_image = gtk_image_new ();
  gtk_widget_set_size_request (priv->battery_image, SPECIAL_ICON_WIDTH, STATUS_AREA_ICON_HEIGHT);
  gtk_widget_show (priv->battery_image);

  priv->icon_box = hd_status_area_box_new ();
  gtk_widget_show (priv->icon_box);

  /* Pack widgets */
  gtk_container_add (GTK_CONTAINER (status_area), ebox);
  gtk_container_add (GTK_CONTAINER (ebox), alignment);
  gtk_container_add (GTK_CONTAINER (alignment), main_hbox);
  gtk_box_pack_start (GTK_BOX (main_hbox), left_vbox, FALSE, FALSE, 0);
  gtk_box_pack_start (GTK_BOX (main_hbox), priv->icon_box, TRUE, TRUE, 0);
  gtk_box_pack_start (GTK_BOX (left_vbox), left_top_row, FALSE, FALSE, 0);
  gtk_box_pack_start (GTK_BOX (left_vbox), left_hsep, FALSE, FALSE, 0);
  gtk_box_pack_start (GTK_BOX (left_vbox), priv->clock_box, FALSE, FALSE, 0);

  gtk_box_pack_start (GTK_BOX (left_top_row), priv->signal_image, FALSE, FALSE, 0);
  gtk_box_pack_start (GTK_BOX (left_top_row), priv->battery_image, FALSE, FALSE, 0);
}

static GObject *
hd_status_area_constructor (GType                  type,
                            guint                  n_construct_properties,
                            GObjectConstructParam *construct_properties)
{
  GObject *object;
  HDStatusAreaPrivate *priv;

  object = G_OBJECT_CLASS (hd_status_area_parent_class)->constructor (type,
                                                                      n_construct_properties,
                                                                      construct_properties);

  /* Create Status Menu */
  priv = HD_STATUS_AREA (object)->priv;
  priv->status_menu = hd_status_menu_new (priv->plugin_manager);

  return object;
}

static void
hd_status_area_dispose (GObject *object)
{
  HDStatusAreaPrivate *priv = HD_STATUS_AREA (object)->priv;

  if (priv->plugin_manager)
    {
      g_object_unref (priv->plugin_manager);
      priv->plugin_manager = NULL;
    }

  G_OBJECT_CLASS (hd_status_area_parent_class)->dispose (object);
}

static void
hd_status_area_finalize (GObject *object)
{
  G_OBJECT_CLASS (hd_status_area_parent_class)->finalize (object);
}

static void
status_area_icon_changed (HDStatusPluginItem *plugin)
{
  GtkWidget *image;
  GdkPixbuf *pixbuf;

  /* Get the image connected with the plugin */
  image = g_object_get_qdata (G_OBJECT (plugin),
                              quark_hd_status_area_image);

  /* Update icon */
  g_object_get (G_OBJECT (plugin),
                "status-area-icon", &pixbuf,
                NULL);
  gtk_image_set_from_pixbuf (GTK_IMAGE (image), pixbuf);

  /* Hide image if icon is not set */
  if (pixbuf)
    {
      g_object_unref (pixbuf);

      gtk_widget_show (image);
    }
  else
    gtk_widget_hide (image);
}

static void
hd_status_area_plugin_added_cb (HDPluginManager *plugin_manager,
                                GObject         *plugin,
                                HDStatusArea    *status_area)
{
  HDStatusAreaPrivate *priv = status_area->priv;
  gchar *plugin_id;
  GtkWidget *image;
  GKeyFile *keyfile;
  gchar *permanent_item;

  /* Plugin must be a HDStatusMenuItem */
  if (!HD_IS_STATUS_PLUGIN_ITEM (plugin))
    return;

  g_object_ref (plugin);

  /* Read position in Status Menu from plugin configuration */
  keyfile = hd_plugin_manager_get_plugin_config_key_file (plugin_manager);
  plugin_id = hd_plugin_item_get_plugin_id (HD_PLUGIN_ITEM (plugin));

  /* Check if the plugin one of the permament plugins on the left
   * side of the Status Area
   */
  permanent_item = g_key_file_get_string (keyfile,
                                          plugin_id,
                                          HD_STATUS_AREA_CONFIG_KEY_PERMANENT_ITEM,
                                          NULL);

  /* Check if plugin is the special permanent clock plugin */
  if (permanent_item && strcmp (HD_STATUS_AREA_CONFIG_VALUE_CLOCK, permanent_item) == 0)
    {
      GtkWidget *clock_widget;

      g_object_get (plugin,
                    "status-area-widget", &clock_widget,
                    NULL);

      gtk_box_pack_start (GTK_BOX (priv->clock_box), clock_widget, FALSE, FALSE, 0);

      g_object_unref (clock_widget);

      g_free (plugin_id);
      return;
    }

  /* Check if plugin is the special permanent battery or signal item */
  if (permanent_item && strcmp (HD_STATUS_AREA_CONFIG_VALUE_SIGNAL, permanent_item) == 0)
    {
      image = priv->signal_image;
      g_object_set_qdata (plugin, quark_hd_status_area_image, image);
    }
  else if (permanent_item && strcmp (HD_STATUS_AREA_CONFIG_VALUE_BATTERY, permanent_item) == 0)
    {
      image = priv->battery_image;
      g_object_set_qdata (plugin, quark_hd_status_area_image, image);
    }
  else
    {
      guint position;
      GError *error = NULL;

      /* Create GtkImage to display the icon */
      image = gtk_image_new ();
      g_object_set_qdata_full (plugin, quark_hd_status_area_image,
                               image, (GDestroyNotify) gtk_widget_destroy);

      /* Get position */
      position = (guint) g_key_file_get_integer (keyfile,
                                                 plugin_id,
                                                 HD_STATUS_AREA_CONFIG_KEY_POSITION,
                                                 &error);
      if (error)
        {
          g_error_free (error);
          position = G_MAXUINT;
        }

      hd_status_area_box_pack (HD_STATUS_AREA_BOX (priv->icon_box),
                               image,
                               position);
    }

  g_signal_connect (plugin, "notify::status-area-icon",
                    G_CALLBACK (status_area_icon_changed), NULL);
  status_area_icon_changed (HD_STATUS_PLUGIN_ITEM (plugin));

  g_free (plugin_id);
}

static void
hd_status_area_plugin_removed_cb (HDPluginManager *plugin_manager,
                                  GObject         *plugin,
                                  HDStatusArea    *status_area)
{
  HDStatusAreaPrivate *priv = status_area->priv;

  /* Plugin must be a HDStatusMenuItem */
  if (!HD_IS_STATUS_PLUGIN_ITEM (plugin))
    return;

  if (g_object_get_qdata (plugin, quark_hd_status_area_image))
    {
      /* Disconnect signal handler */
      g_signal_handlers_disconnect_by_func (plugin,
                                            status_area_icon_changed,
                                            NULL);
      /* Reset image and destroy it if created in plugin_added_cb */
      g_object_set_qdata (plugin, quark_hd_status_area_image, NULL);
    }
  else
    {
      /* Remove all widgets from the clock box */
      gtk_container_foreach (GTK_CONTAINER (priv->clock_box), (GtkCallback) gtk_widget_destroy, NULL);
    }

  g_object_unref (plugin);
}

static void
hd_status_area_plugin_configuration_loaded_cb (HDPluginManager *plugin_manager,
                                               GKeyFile        *keyfile,
                                               HDStatusArea    *status_area)
{
}

static void
hd_status_area_set_property (GObject      *object,
                             guint         prop_id,
                             const GValue *value,
                             GParamSpec   *pspec)
{
  HDStatusAreaPrivate *priv = HD_STATUS_AREA (object)->priv;

  switch (prop_id)
    {
    case PROP_PLUGIN_MANAGER:
      /* The property is CONSTRUCT_ONLY so there is no value yet */
      priv->plugin_manager = g_value_dup_object (value);
      if (priv->plugin_manager != NULL)
        {
          g_signal_connect_object (G_OBJECT (priv->plugin_manager), "plugin-added",
                                   G_CALLBACK (hd_status_area_plugin_added_cb), object, 0);
          g_signal_connect_object (G_OBJECT (priv->plugin_manager), "plugin-removed",
                                   G_CALLBACK (hd_status_area_plugin_removed_cb), object, 0);
          g_signal_connect_object (G_OBJECT (priv->plugin_manager), "plugin-configuration-loaded",
                                   G_CALLBACK (hd_status_area_plugin_configuration_loaded_cb), object, 0);
        }
      else
        g_warning ("plugin-manager should not be NULL");
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
hd_status_area_realize (GtkWidget *widget)
{
  GdkDisplay *display;
  Atom atom, wm_type;

  GTK_WIDGET_CLASS (hd_status_area_parent_class)->realize (widget);

  /* Use only a border as decoration */
  gdk_window_set_decorations (widget->window, 0);

  /* Set the _NET_WM_WINDOW_TYPE property to _HILDON_WM_WINDOW_TYPE_STATUS_AREA */
  display = gdk_drawable_get_display (widget->window);
  atom = gdk_x11_get_xatom_by_name_for_display (display,
                                                "_NET_WM_WINDOW_TYPE");
  wm_type = gdk_x11_get_xatom_by_name_for_display (display,
                                                   "_HILDON_WM_WINDOW_TYPE_STATUS_AREA");

  XChangeProperty (GDK_WINDOW_XDISPLAY (widget->window),
                   GDK_WINDOW_XID (widget->window),
                   atom, XA_ATOM, 32, PropModeReplace,
                   (unsigned char *)&wm_type, 1);
}

static void
hd_status_area_check_resize (GtkContainer *container)
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

      /* Resize children (also if size not changed and so no
       * configure notify event is triggered) */
      gtk_container_resize_children (GTK_CONTAINER (widget));
    }
}

static void
hd_status_area_class_init (HDStatusAreaClass *klass)
{
  GObjectClass* object_class = G_OBJECT_CLASS (klass);
  GtkWidgetClass* widget_class = GTK_WIDGET_CLASS (klass);
  GtkContainerClass *container_class = GTK_CONTAINER_CLASS (klass);

  quark_hd_status_area_image = g_quark_from_static_string (hd_status_area_image);

  object_class->constructor = hd_status_area_constructor;
  object_class->dispose = hd_status_area_dispose;
  object_class->finalize = hd_status_area_finalize;
  object_class->set_property = hd_status_area_set_property;

  widget_class->realize = hd_status_area_realize;

  container_class->check_resize = hd_status_area_check_resize;

  g_object_class_install_property (object_class,
                                   PROP_PLUGIN_MANAGER,
                                   g_param_spec_object ("plugin-manager",
                                                        "Plugin Manager",
                                                        "The plugin manager which should be used",
                                                        HD_TYPE_PLUGIN_MANAGER,
                                                        G_PARAM_WRITABLE | G_PARAM_CONSTRUCT_ONLY));

  g_type_class_add_private (klass, sizeof (HDStatusAreaPrivate));
}

GtkWidget *
hd_status_area_new (HDPluginManager *plugin_manager)
{
  GtkWidget *status_area;

  status_area = g_object_new (HD_TYPE_STATUS_AREA,
                              "type", GTK_WINDOW_TOPLEVEL,
                              "plugin-manager", plugin_manager,
                              NULL);

  return status_area;
}
