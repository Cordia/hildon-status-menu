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

#include <hildon/hildon.h>
#include <gdk/gdkx.h>

#include <X11/X.h>
#include <X11/Xatom.h>

#include <string.h>

#include "hd-desktop.h"
#include "hd-display.h"

#include "hd-status-area-box.h"
#include "hd-status-menu.h"
#include "hd-status-menu-config.h"

#include "hd-status-area.h"

/* UI Style guide */
#define STATUS_AREA_HEIGHT 56
#define MINIMUM_STATUS_AREA_WIDTH 112

#define SPECIAL_ICON_WIDTH 18
#define SPECIAL_ICON_HEIGHT 36

#define CUSTOM_MARGIN_9 9
#define CUSTOM_MARGIN_10 10

/* Configuration file keys */

#define HD_STATUS_AREA_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), HD_TYPE_STATUS_AREA, HDStatusAreaPrivate));

static GQuark      quark_hd_status_area_image = 0;
static const gchar hd_status_area_image[] = "hd_status_area_image";

static GQuark      quark_hd_status_area_plugin_id = 0;
static const gchar hd_status_area_plugin_id[] = "hd_status_area_plugin_id";

enum
{
  PROP_0,
  PROP_PLUGIN_MANAGER
};

struct _HDStatusAreaPrivate
{
  HDPluginManager *plugin_manager;

  HDDesktop *desktop;
  HDDisplay *display;
  GList *status_plugins;

  GtkWidget *status_menu;

  GtkWidget *icon_box;

  GtkWidget **special_item_image;

  GtkWidget *clock_box;

  GtkWidget *main_alignment;

  gboolean resize_after_map : 1;
  gboolean status_area_visible;
};

G_DEFINE_TYPE (HDStatusArea, hd_status_area, GTK_TYPE_WINDOW);

static gboolean
button_release_event_cb (GtkWidget      *widget,
                       GdkEventButton *event,
                       HDStatusArea   *status_area)
{
  HDStatusAreaPrivate *priv = status_area->priv;

  gtk_widget_show (priv->status_menu);
  if (!GTK_WIDGET_VISIBLE (priv->status_menu))
    /* Failed to show the status menu because it got deleted.
     * Make sure it doesn't retain the grab. */
    gtk_grab_remove (GTK_WIDGET (priv->status_menu));

  return TRUE;
}

static gboolean
is_widget_on_screen (GtkWidget *widget)
{
  GdkWindow *window;
  gint x, y, width, height;

  window = gtk_widget_get_window (widget);
  gdk_window_get_root_origin (window, &x, &y);
  gdk_window_get_geometry (window, NULL, NULL, &width, &height, NULL);

  /* the compositor moves obscured windows off the screen, so we can use
   * that to determine whether the status area is visible */
  return (x + width > 0) && (y + height > 0);
}

static void
update_status_area_visibility (HDStatusArea *status_area)
{
  HDStatusAreaPrivate *priv = status_area->priv;
  gboolean visible;
  GList *l;

  visible = (is_widget_on_screen (GTK_WIDGET (status_area)) &&
             !hd_desktop_is_task_switcher_visible (priv->desktop) &&
             hd_display_is_on (priv->display));

  if (visible != priv->status_area_visible)
    {
      priv->status_area_visible = visible;

      /* inform status area plugins if the status area is obscured or not */
      for (l = priv->status_plugins; l; l = l->next)
        {
          g_object_set (l->data, "status-area-visible", visible, NULL);
        }
    }
}

static gboolean
configure_event_cb (GtkWidget         *widget,
                    GdkEventConfigure *event,
                    gpointer           user_data)
{
  HDStatusArea *status_area;

  status_area = HD_STATUS_AREA (widget);

  update_status_area_visibility (status_area);

  return FALSE;
}

static void
hd_status_area_init (HDStatusArea *status_area)
{
  HDStatusAreaPrivate *priv = HD_STATUS_AREA_GET_PRIVATE (status_area);
  GtkWidget *left_alignment, *main_hbox, *left_hbox, *special_hbox;
  guint i;

  /* Set priv member */
  status_area->priv = priv;

  priv->desktop = hd_desktop_get ();
  g_signal_connect_swapped (priv->desktop, "task-switcher-show",
                            G_CALLBACK (update_status_area_visibility), status_area);
  g_signal_connect_swapped (priv->desktop, "task-switcher-hide",
                            G_CALLBACK (update_status_area_visibility), status_area);
  priv->display = hd_display_get ();
  g_signal_connect_swapped (priv->display, "display-status-changed",
                            G_CALLBACK (update_status_area_visibility), status_area);
  update_status_area_visibility (status_area);

  priv->status_plugins = NULL;

  /* Create Status area UI */
  gtk_widget_add_events (GTK_WIDGET (status_area), GDK_BUTTON_RELEASE_MASK);
  g_signal_connect (G_OBJECT (status_area), "button-release-event",
                    G_CALLBACK (button_release_event_cb), status_area);
  gtk_widget_set_app_paintable (GTK_WIDGET (status_area), TRUE);
  gtk_widget_set_size_request (GTK_WIDGET (status_area), -1, STATUS_AREA_HEIGHT);

  priv->main_alignment = gtk_alignment_new (0.5, 0.5, 0.0, 0.0);
  gtk_widget_show (priv->main_alignment);

  left_alignment = gtk_alignment_new (0.5, 0.5, 0.0, 0.0);
  gtk_alignment_set_padding (GTK_ALIGNMENT (left_alignment),
                             CUSTOM_MARGIN_10 - CUSTOM_MARGIN_9,
                             CUSTOM_MARGIN_10 - CUSTOM_MARGIN_9,
                             0, 0);
  gtk_widget_show (left_alignment);

  main_hbox = gtk_hbox_new (FALSE, 0);
  gtk_widget_show (main_hbox);

  left_hbox = gtk_hbox_new (FALSE, HILDON_MARGIN_DOUBLE);
  gtk_widget_show (left_hbox);

  special_hbox = gtk_hbox_new (FALSE, HILDON_MARGIN_HALF);
  gtk_widget_show (special_hbox);

  priv->clock_box = gtk_alignment_new (0.5, 0.5, 0.0, 0.0);
/*  gtk_widget_set_size_request (priv->clock_box, SPECIAL_ICON_WIDTH * 2, STATUS_AREA_ICON_HEIGHT); */
  gtk_widget_show (priv->clock_box);

  priv->special_item_image = g_new0 (GtkWidget*, HD_STATUS_AREA_NUM_SPECIAL_ITEMS);
  for (i = 0; i < HD_STATUS_AREA_NUM_SPECIAL_ITEMS; i++)
    {
      priv->special_item_image[i] = gtk_image_new ();
      gtk_widget_set_size_request (priv->special_item_image[i], SPECIAL_ICON_WIDTH, SPECIAL_ICON_HEIGHT);
      gtk_widget_show (priv->special_item_image[i]);
    }

  priv->icon_box = hd_status_area_box_new ();
  gtk_widget_show (priv->icon_box);

  /* Pack widgets */
  gtk_container_add (GTK_CONTAINER (status_area), priv->main_alignment);
  gtk_container_add (GTK_CONTAINER (priv->main_alignment), main_hbox);
  gtk_box_pack_start (GTK_BOX (main_hbox), left_alignment, FALSE, FALSE, 0);
  gtk_container_add (GTK_CONTAINER (left_alignment), left_hbox);
  gtk_box_pack_start (GTK_BOX (left_hbox), priv->clock_box, FALSE, FALSE, 0);
  gtk_box_pack_start (GTK_BOX (left_hbox), special_hbox, FALSE, FALSE, 0);
  for (i = 0; i < HD_STATUS_AREA_NUM_SPECIAL_ITEMS; i++)
    gtk_box_pack_start (GTK_BOX (special_hbox), priv->special_item_image[i], FALSE, FALSE, 0);
  gtk_box_pack_start (GTK_BOX (main_hbox), priv->icon_box, TRUE, TRUE, 0);

  /* Detect when the entire status area is moved off screen (this happens when a
   * program is full-screen) */
  g_signal_connect (G_OBJECT (status_area), "configure-event",
                    G_CALLBACK (configure_event_cb), status_area);
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
  HDStatusArea *status_area = HD_STATUS_AREA (object);
  HDStatusAreaPrivate *priv = status_area->priv;

  if (priv->plugin_manager)
    priv->plugin_manager = (g_object_unref (priv->plugin_manager), NULL);

  if (priv->desktop)
    {
      g_signal_handlers_disconnect_by_func (priv->desktop,
                                            update_status_area_visibility,
                                            status_area);
      priv->desktop = (g_object_unref (priv->desktop), NULL);
    }

  if (priv->display)
    {
      g_signal_handlers_disconnect_by_func (priv->display,
                                            update_status_area_visibility,
                                            status_area);
      priv->display = (g_object_unref (priv->display), NULL);
    }

  G_OBJECT_CLASS (hd_status_area_parent_class)->dispose (object);
}

static void
hd_status_area_finalize (GObject *object)
{
  HDStatusAreaPrivate *priv = HD_STATUS_AREA (object)->priv;

  if (priv->special_item_image)
    priv->special_item_image = (g_free (priv->special_item_image), NULL);

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

  /*
  g_debug ("status_area_icon_changed. plugin: %s, icon %x",
           hd_status_plugin_item_get_dl_filename (plugin),
           (guint) pixbuf);
           */

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
  GtkWidget *image = NULL;
  GKeyFile *keyfile;
  gchar *permanent_item;
  guint i;

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

      gtk_container_add (GTK_CONTAINER (priv->clock_box), clock_widget);

      g_object_unref (clock_widget);

      g_free (plugin_id);
      return;
    }

  /* Check if plugin is the special permanent item */
  for (i = 0; i < HD_STATUS_AREA_NUM_SPECIAL_ITEMS; i++)
    {
      gchar *value = g_strdup_printf (HD_STATUS_AREA_CONFIG_VALUE_SPECIAL_ITEM, i);

      if (permanent_item && strcmp (value, permanent_item) == 0)
        {
          image = priv->special_item_image [i];
          g_object_set_qdata_full (plugin, quark_hd_status_area_image, image, (GDestroyNotify) gtk_widget_destroy);

          g_free (value);
          break;
        }

      g_free (value);
    }

  if (!image)
    {
      guint position;
      GError *error = NULL;

      /* Create GtkImage to display the icon */
      image = gtk_image_new ();
      g_object_set_qdata_full (plugin, quark_hd_status_area_image,
                               image, (GDestroyNotify) gtk_widget_destroy);
      g_object_set_qdata_full (G_OBJECT (image), quark_hd_status_area_plugin_id,
                               g_strdup (plugin_id), (GDestroyNotify) g_free);

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

  priv->status_plugins = g_list_prepend (priv->status_plugins, plugin);
  g_object_set (plugin, "status-area-visible", priv->status_area_visible, NULL);

  g_signal_connect (plugin, "notify::status-area-icon",
                    G_CALLBACK (status_area_icon_changed), NULL);
  status_area_icon_changed (HD_STATUS_PLUGIN_ITEM (plugin));

  g_free (plugin_id);
}

static void
remove_from_container (GtkWidget    *widget,
                       GtkContainer *container)
{
  gtk_container_remove (container,
                        widget);
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
      gtk_container_foreach (GTK_CONTAINER (priv->clock_box),
                             (GtkCallback) remove_from_container,
                             priv->clock_box);
    }

  priv->status_plugins = g_list_remove (priv->status_plugins, plugin);
  g_object_unref (plugin);
}

static void
update_position (GtkWidget *child,
                 GKeyFile  *keyfile)
{
  gchar *plugin_id;
  guint position;
  GError *error = NULL;

  plugin_id = g_object_get_qdata (G_OBJECT (child), quark_hd_status_area_plugin_id);

  /* Get the position from the plugin configuration file */
  position = (guint) g_key_file_get_integer (keyfile,
                                             plugin_id,
                                             HD_STATUS_AREA_CONFIG_KEY_POSITION,
                                             &error);

  /* Use G_MAXUINT as default */
  if (error)
    {
      g_error_free (error);
      position = G_MAXUINT;
    }

  /* Reorder Child */
  hd_status_area_box_reorder_child (HD_STATUS_AREA_BOX (gtk_widget_get_parent (child)),
                                    child,
                                    position);
}

static void
hd_status_area_items_configuration_loaded_cb (HDPluginManager *plugin_manager,
                                               GKeyFile        *key_file,
                                               HDStatusArea    *status_area)
{
  HDStatusAreaPrivate *priv = status_area->priv;

  gtk_container_foreach (GTK_CONTAINER (priv->icon_box), (GtkCallback) update_position, key_file);
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
          g_signal_connect_object (G_OBJECT (priv->plugin_manager), "items-configuration-loaded",
                                   G_CALLBACK (hd_status_area_items_configuration_loaded_cb), object, 0);
        }
      else
        g_warning ("plugin-manager should not be NULL");
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static gboolean
hd_status_area_expose_event (GtkWidget *widget,
                             GdkEventExpose *event)
{
  cairo_t *cr;

  /* Create cairo context */
  cr = gdk_cairo_create (GDK_DRAWABLE (widget->window));
  gdk_cairo_region (cr, event->region);
  cairo_clip (cr);

  /* Draw transparent background */
  cairo_set_operator (cr, CAIRO_OPERATOR_SOURCE);
  cairo_set_source_rgba (cr, 0.0, 0.0, 0.0, 0.0);
  cairo_paint (cr);

  cairo_destroy (cr);

  return GTK_WIDGET_CLASS (hd_status_area_parent_class)->expose_event (widget,
                                                                       event);
}

static gboolean
is_portrait_mode (GtkWidget *widget)
{
  GdkScreen *screen;

  screen = gtk_widget_get_screen (widget);

  return gdk_screen_get_height (screen) > gdk_screen_get_width (screen);
}

static void
update_alignemnt_padding (HDStatusArea *status_area)
{
  HDStatusAreaPrivate *priv = status_area->priv;
  gboolean portrait = is_portrait_mode (GTK_WIDGET (status_area));
  guint left_right_padding;

  if (portrait)
    left_right_padding = HILDON_MARGIN_DEFAULT;
  else
    left_right_padding = HILDON_MARGIN_DOUBLE;

  gtk_alignment_set_padding (GTK_ALIGNMENT (priv->main_alignment),
                             CUSTOM_MARGIN_9, CUSTOM_MARGIN_9,
                             left_right_padding, left_right_padding);
}

static void
hd_status_area_realize (GtkWidget *widget)
{
  GdkScreen *screen;
  GdkDisplay *display;
  Atom atom, wm_type;
  GdkPixmap *pixmap;
  cairo_t *cr;

  screen = gtk_widget_get_screen (widget);
  gtk_widget_set_colormap (widget,
                           gdk_screen_get_rgba_colormap (screen));

  g_signal_connect_swapped (screen, "size-changed",
                            G_CALLBACK (update_alignemnt_padding),
                            widget);
  update_alignemnt_padding (HD_STATUS_AREA (widget));

  gtk_widget_set_app_paintable (widget,
                                TRUE);

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

  /* Set background to transparent pixmap */
  pixmap = gdk_pixmap_new (GDK_DRAWABLE (widget->window), 1, 1, -1);
  cr = gdk_cairo_create (GDK_DRAWABLE (pixmap));
  cairo_set_operator (cr, CAIRO_OPERATOR_SOURCE);
  cairo_set_source_rgba (cr, 0.0, 0.0, 0.0, 0.0);
  cairo_paint (cr);
  cairo_destroy (cr);

  gdk_window_set_back_pixmap (widget->window, pixmap, FALSE);
}

static void
hd_status_area_unrealize (GtkWidget *widget)
{
  GdkScreen *screen;

  screen = gtk_widget_get_screen (widget);
  g_signal_handlers_disconnect_by_func (screen,
                                        update_alignemnt_padding,
                                        HD_STATUS_AREA (widget));

  GTK_WIDGET_CLASS (hd_status_area_parent_class)->unrealize (widget);
}


static void
hd_status_area_map (GtkWidget *widget)
{
  HDStatusAreaPrivate *priv = HD_STATUS_AREA (widget)->priv;

  priv->resize_after_map = TRUE;

  GTK_WIDGET_CLASS (hd_status_area_parent_class)->map (widget);
}

static void
hd_status_area_check_resize (GtkContainer *container)
{
  HDStatusAreaPrivate *priv = HD_STATUS_AREA (container)->priv;
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
      gint width, height;

      gtk_widget_size_request (widget, &req);

      gdk_drawable_get_size (GDK_DRAWABLE (widget->window),
                             &width, &height);

      if (priv->resize_after_map ||
          req.width != width || req.height != height)
        {
          priv->resize_after_map = FALSE;

          /* Request the window manager to resize the window to
           * the required size (will result in a configure notify event
           * see above) */
          gdk_window_resize (widget->window, req.width, req.height);
        }

      /* Resize children (also if size not changed and so no
         configure notify event is triggered) */
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
  quark_hd_status_area_plugin_id = g_quark_from_static_string (hd_status_area_plugin_id);

  object_class->constructor = hd_status_area_constructor;
  object_class->dispose = hd_status_area_dispose;
  object_class->finalize = hd_status_area_finalize;
  object_class->set_property = hd_status_area_set_property;

  widget_class->realize = hd_status_area_realize;
  widget_class->unrealize = hd_status_area_unrealize;
  widget_class->map = hd_status_area_map;
  widget_class->expose_event = hd_status_area_expose_event;

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
                              "accept-focus", FALSE,
                              "plugin-manager", plugin_manager,
                              NULL);

  return status_area;
}
