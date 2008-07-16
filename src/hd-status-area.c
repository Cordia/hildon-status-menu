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

#include "hd-status-area.h"
#include "hd-status-menu.h"

/* UI Style guide */
#define STATUS_AREA_HEIGHT 56
#define MINIMUM_STATUS_AREA_WIDTH 112

G_DEFINE_TYPE (HDStatusArea, hd_status_area, GTK_TYPE_WINDOW);

static void
size_request_cb (GtkWidget *widget, GtkRequisition *requisition, gpointer data)
{
  requisition->width = MAX (requisition->width, MINIMUM_STATUS_AREA_WIDTH);
  requisition->height = STATUS_AREA_HEIGHT;
}
static gboolean
button_release_event_cb (GtkWidget *widget, GdkEventButton *event, gpointer data)
{
  GtkWidget *menu;

  g_warning ("FOO");

  menu = hd_status_menu_new (NULL);
  gtk_widget_show (menu);

  return TRUE;
}

static void
hd_status_area_init (HDStatusArea *object)
{
  /* UI Style guide */
  GtkWidget *rows;

  GtkWidget *clock, *signal;

  GtkWidget *ebox;

  ebox = gtk_event_box_new ();
  gtk_widget_show (ebox);

  rows = gtk_vbox_new (TRUE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (rows), 4);
  g_signal_connect (G_OBJECT (rows), "size-request",
                    G_CALLBACK (size_request_cb), NULL);
  gtk_widget_show (rows);
  gtk_container_add (GTK_CONTAINER (ebox), rows);

  clock = gtk_label_new (NULL);
  gtk_label_set_markup (GTK_LABEL (clock), "<big>15:48</big>AM");
  gtk_widget_show (clock);

  gtk_box_pack_start (GTK_BOX (rows), clock, TRUE, TRUE, 0);

  signal = gtk_label_new ("S");
  gtk_widget_show (signal);
  gtk_box_pack_start (GTK_BOX (rows), signal, FALSE, FALSE, 0);

  gtk_widget_add_events (GTK_WIDGET (ebox), GDK_BUTTON_RELEASE_MASK);
  g_signal_connect (G_OBJECT (ebox), "button-release-event",
                    G_CALLBACK (button_release_event_cb), NULL);
  gtk_container_add (GTK_CONTAINER (object), ebox);
}

static void
hd_status_area_finalize (GObject *object)
{
  G_OBJECT_CLASS (hd_status_area_parent_class)->finalize (object);
}

static void
hd_status_area_class_init (HDStatusAreaClass *klass)
{
  GObjectClass* object_class = G_OBJECT_CLASS (klass);
  /*  GtkWindowClass* parent_class = GTK_WINDOW_CLASS (klass); */

  object_class->finalize = hd_status_area_finalize;
}

GtkWidget *
hd_status_area_new (void)
{
  GtkWidget *status_area;

  status_area = g_object_new (HD_TYPE_STATUS_AREA,
                              "type", GTK_WINDOW_TOPLEVEL,
                              NULL);

  return status_area;
}
