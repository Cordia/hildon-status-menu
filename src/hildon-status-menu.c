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

static guint
load_priority_func (const gchar *plugin_id,
                    GKeyFile    *keyfile,
                    gpointer     data)
{
  GError *error = NULL;
  guint priority = G_MAXUINT;

  /* The permament status area items (clock, signal and
   * battery) should be loaded first (priority == 0) */
  if (g_key_file_has_key (keyfile,
                          plugin_id,
                          "X-Status-Area-Permanent-Item",
                          NULL))
    return 0;

  /* Then the plugins should be loaded regarding to there
   * position in the status area. */
  priority = (guint) g_key_file_get_integer (keyfile,
                                             plugin_id,
                                             "X-Status-Area-Position",
                                             &error);
  if (error == NULL)
    return priority;

  /* If position is not set, load last (priority == max) */
  g_error_free (error);

  return G_MAXUINT;
}

static gboolean
load_plugins_idle (gpointer data)
{

  /* Load the configuration of the plugin manager and load plugins */
  hd_plugin_manager_run (HD_PLUGIN_MANAGER (data));

  return FALSE;
}

int
main (int argc, char **argv)
{
/*  GtkWidget *status_menu; */
  GtkWidget *status_area;
  HDConfigFile *config_file;
  HDPluginManager *plugin_manager;
  gchar *user_config_dir;
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

  /* User configuration directory (~/) */
  user_config_dir = g_build_filename (g_get_user_config_dir (),
                                      "hildon-desktop",
                                      NULL);
  g_debug ("User config dir: %s", user_config_dir);

  /* Create a config file for the plugin manager
   *
   */
  config_file = hd_config_file_new (HD_DESKTOP_CONFIG_PATH,
                                    user_config_dir,
                                    "status-menu.conf");
  plugin_manager = hd_plugin_manager_new (config_file, HD_STATUS_MENU_STAMP_FILE);
  g_object_unref (config_file);
  g_free (user_config_dir);

  /* Set the load priority function */
  hd_plugin_manager_set_load_priority_func (plugin_manager,
                                            load_priority_func,
                                            NULL,
                                            NULL);

  /* Create simple window to show the Status Menu 
   */
  status_area = hd_status_area_new (plugin_manager);

  /* Show Status Area */
  gtk_widget_show (status_area);

  gdk_threads_add_idle (load_plugins_idle, plugin_manager);

  gtk_main ();

  return 0;
}
