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

#include <dbus/dbus.h>

#include <mce/dbus-names.h>
#include <mce/mode-names.h>

#include <string.h>

#include "hd-display.h"

#define HD_DISPLAY_GET_PRIVATE(object) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((object), HD_TYPE_DISPLAY, HDDisplayPrivate))

struct _HDDisplayPrivate
{
  DBusConnection *system_bus;

  gboolean display_on : 1;
};

enum
{
  DISPLAY_STATUS_CHANGED,

  LAST_SIGNAL
};

static guint display_signals[LAST_SIGNAL] = { 0, };

static void hd_display_dispose     (GObject *object);

static void initialize_system_dbus (HDDisplay *display);
static DBusHandlerResult system_bus_signal_filter (DBusConnection *system_bus,
                                                   DBusMessage    *msg,
                                                   void           *data);

G_DEFINE_TYPE (HDDisplay, hd_display, G_TYPE_OBJECT);

HDDisplay *
hd_display_get (void)
{
  static gpointer display = NULL;

  if (display == NULL)
    {
      display = g_object_new (HD_TYPE_DISPLAY,
                              NULL);
      g_object_add_weak_pointer (display, &display);
      return display;
    }
  else
    {
      return g_object_ref (display);
    }
}

static void
hd_display_class_init (HDDisplayClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->dispose = hd_display_dispose;

  display_signals[DISPLAY_STATUS_CHANGED] = g_signal_new ("display-status-changed",
                                                          HD_TYPE_DISPLAY,
                                                          0, 0,
                                                          NULL, NULL,
                                                          g_cclosure_marshal_VOID__VOID,
                                                          G_TYPE_NONE,
                                                          0);

  g_type_class_add_private (klass, sizeof (HDDisplayPrivate));
}

static void
hd_display_init (HDDisplay *display)
{
  display->priv = HD_DISPLAY_GET_PRIVATE (display);

  display->priv->display_on = TRUE;

  initialize_system_dbus (display);
}

static void
initialize_system_dbus (HDDisplay *display)
{
  HDDisplayPrivate *priv = display->priv;
  DBusError error;

  dbus_error_init (&error);
  priv->system_bus = dbus_bus_get (DBUS_BUS_SYSTEM,
                                   &error);
  if (dbus_error_is_set (&error))
   {
     g_warning ("%s. Could not connect to System D_Bus. %s",
                __FUNCTION__,
                error.message);
     dbus_error_free (&error);
     return;
   }

  dbus_bus_add_match (priv->system_bus,
                      "type='signal', interface='" MCE_SIGNAL_IF "'",
                      NULL);
  dbus_connection_add_filter (priv->system_bus,
                              system_bus_signal_filter,
                              display,
                              NULL);
}

static DBusHandlerResult
system_bus_signal_filter (DBusConnection *system_bus,
                          DBusMessage    *msg,
                          void           *data)
{
  HDDisplay *display = data;
  HDDisplayPrivate *priv = display->priv;

  if (dbus_message_is_signal (msg,
                              MCE_SIGNAL_IF,
                              MCE_DISPLAY_SIG))
    {
      DBusMessageIter iter;

      if (dbus_message_iter_init (msg, &iter))
        if (dbus_message_iter_get_arg_type (&iter) == DBUS_TYPE_STRING)
          {
            const char *value;
            gboolean display_on = TRUE;

            dbus_message_iter_get_basic(&iter, &value);
            if (strcmp (value, MCE_DISPLAY_ON_STRING) == 0)
              display_on = TRUE;
            else if (strcmp (value, MCE_DISPLAY_DIM_STRING) == 0)
              display_on = TRUE;
            else if (strcmp (value, MCE_DISPLAY_OFF_STRING) == 0)
              display_on = FALSE;
            else
              g_warning ("%s. Unknown value %s for signal %s.%s",
                         __FUNCTION__,
                         value,
                         MCE_SIGNAL_IF,
                         MCE_DISPLAY_SIG);

            priv->display_on = display_on;

            g_signal_emit (display,
                           display_signals[DISPLAY_STATUS_CHANGED],
                           0);
          }
     }

  return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
}

static void
hd_display_dispose (GObject *object)
{
  HDDisplay *display = HD_DISPLAY (object);
  HDDisplayPrivate *priv = display->priv;

  if (priv->system_bus)
    {
      dbus_bus_remove_match (priv->system_bus,
                             "type='signal', interface='" MCE_SIGNAL_IF "'",
                             NULL);
      dbus_connection_remove_filter (priv->system_bus,
                                     system_bus_signal_filter,
                                     display);
      priv->system_bus = NULL;
    }

  G_OBJECT_CLASS (hd_display_parent_class)->dispose (object);
}

gboolean
hd_display_is_on (HDDisplay *display)
{
  HDDisplayPrivate *priv;

  g_return_val_if_fail (HD_IS_DISPLAY (display), FALSE);

  priv = display->priv;

  return priv->display_on;
}
