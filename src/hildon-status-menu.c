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

#include <libgnomevfs/gnome-vfs.h>
#include <libhildondesktop/libhildondesktop.h>
#include <hildon/hildon.h>

#include <libintl.h>
#include <locale.h>
#include <signal.h>
#include <stdlib.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "hd-status-area.h"
#include "hd-status-menu.h"
#include "hd-status-menu-config.h"

#define HD_STAMP_DIR   "/tmp/hildon-desktop/"
#define HD_STATUS_MENU_STAMP_FILE HD_STAMP_DIR "status-menu.stamp"

/* signal handler, hildon-desktop sends SIGTERM to all tracked applications
 * when it receives SIGTEM itself */
static void
signal_handler (int signal)
{
  gtk_main_quit ();
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
                          HD_STATUS_AREA_CONFIG_KEY_PERMANENT_ITEM,
                          NULL))
    return 0;

  /* Then the plugins should be loaded regarding to there
   * position in the status area. */
  priority = (guint) g_key_file_get_integer (keyfile,
                                             plugin_id,
                                             HD_STATUS_AREA_CONFIG_KEY_POSITION,
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

static void
console_quiet(void)
{
  close(0);
  close(1);
  close(2);

  if (open("/dev/null", O_RDONLY) < 0)
    g_warning ("%s: failed opening /dev/null read-only", __func__);
  if (dup(open("/dev/null", O_WRONLY)) < 0)
    g_warning ("%s: failed opening /dev/null write-only", __func__);
}

int
main (int argc, char **argv)
{
  GtkWidget *status_area;
  HDPluginManager *plugin_manager;

  if (!g_thread_supported ())
    g_thread_init (NULL);
  setlocale (LC_ALL, "");

  /* Initialize Gtk+ */
  gtk_init (&argc, &argv);

  /* Initialize Hildon */
  hildon_init ();

  /* Initialize GnomeVFS */
  gnome_vfs_init ();

  /* Add handler for TERM and signals */
  signal (SIGTERM, signal_handler);
  signal (SIGINT, signal_handler);

  if (getenv ("DEBUG_OUTPUT") == NULL)
    console_quiet ();

  /* Setup Stamp File */
  hd_stamp_file_init (HD_STATUS_MENU_STAMP_FILE);

  /* Create a plugin manager instance */
  plugin_manager = hd_plugin_manager_new (
                     hd_config_file_new_with_defaults ("status-menu.conf"));

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

  /* Load Plugins when idle */
  gdk_threads_add_idle (load_plugins_idle, plugin_manager);

  /* Start the main loop */
  gtk_main ();

  /* Delete the stamp file */
  hd_stamp_file_finalize (HD_STATUS_MENU_STAMP_FILE);

  return 0;
}
