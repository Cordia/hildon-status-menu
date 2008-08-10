/*
 * This file is part of hildon-status-menu
 * 
 * Copyright (C) 2006, 2007, 2008 Nokia Corporation.
 *
 * Based on main.c from hildon-desktop.
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

#include <glib/gstdio.h>
#include <libgnomevfs/gnome-vfs.h>
#include <libhildondesktop/libhildondesktop.h>
#include <hildon/hildon.h>

#include <libintl.h>
#include <locale.h>
#include <signal.h>
#include <stdlib.h>

#include "hd-status-area.h"
#include "hd-status-menu.h"

#define HD_STAMP_DIR   "/tmp/osso-appl-states/hildon-desktop/"
#define HD_STATUS_MENU_STAMP_FILE HD_STAMP_DIR "status-menu.stamp"

#if 0
static void
show_button_clicked_cb (GtkButton    *button,
                        HDStatusMenu *status_menu)
{
  gtk_widget_show (GTK_WIDGET (status_menu));
}
#endif

/* signal handler, hildon-desktop sends SIGTERM to all tracked applications
 * when it receives SIGTEM itselgf */
static void
signal_handler (int signal)
{
  if (signal == SIGTERM)
  {
    /* 
     * Clean up stamp file created by hd_plugin_manager_run
     * On next startup the stamp file is created again and hildon-desktop remains
     * in normal operation mode without entering into safe mode where some plugins
     * are disabled.
     */
    if (g_file_test (HD_STATUS_MENU_STAMP_FILE, G_FILE_TEST_EXISTS)) 
      {
        g_unlink (HD_STATUS_MENU_STAMP_FILE);
      }

    exit (0);
  }
}


int
main (int argc, char **argv)
{
/*  GtkWidget *status_menu; */
  GtkWidget *status_area;
  HDConfigFile *config_file;
  HDPluginManager *plugin_manager;
/*  GtkWidget *window, *button;
  HildonProgram *program;*/

  g_thread_init (NULL);
  setlocale (LC_ALL, "");

  /* FIXME: No translations required (yet)
  bindtextdomain (GETTEXT_PACKAGE, LOCALEDIR);
  bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
  textdomain (GETTEXT_PACKAGE);
  */

  gtk_init (&argc, &argv);

  gnome_vfs_init ();

  signal (SIGTERM, signal_handler);

  /* Create a config file for the plugin manager
   *
   * FIXME: HDPluginManager could be a subclass of HDConfigFile.
   */
  config_file = hd_config_file_new (HD_DESKTOP_CONFIG_PATH,
                                    NULL,
                                    "status-menu.conf");
  plugin_manager = hd_plugin_manager_new (config_file, HD_STATUS_MENU_STAMP_FILE);
  g_object_unref (config_file);

  /* Create simple window to show the Status Menu 
   */
  status_area = hd_status_area_new (plugin_manager);

  /* Load the configuration of the plugin manager and load plugins */
  hd_plugin_manager_run (plugin_manager);

  /* Show Status Area */
  gtk_widget_show (status_area);

#if 0
  button = gtk_button_new_with_label ("Show Status Menu");
  gtk_widget_show (button);
  g_signal_connect (G_OBJECT (button), "clicked",
                    G_CALLBACK (show_button_clicked_cb), status_menu);

  window = hildon_window_new ();
  gtk_container_add (GTK_CONTAINER (window), button);
  gtk_widget_show (window);
  g_signal_connect_swapped (G_OBJECT (window), "destroy",
		            G_CALLBACK (gtk_widget_destroy), status_menu);
  g_signal_connect (G_OBJECT (window), "destroy",
		    G_CALLBACK (gtk_main_quit), NULL);
  
  program = hildon_program_get_instance ();
  g_set_application_name ("Status Menu");
  hildon_program_add_window (program, HILDON_WINDOW (window));
#endif

  gtk_main ();

  return 0;
}
