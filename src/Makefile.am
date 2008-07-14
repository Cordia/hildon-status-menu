bin_PROGRAMS = hildon-status-menu

hildondesktopconf_DATA = status-menu.conf

hildon_status_menu_CFLAGS = \
	$(HILDON_CFLAGS)							\
	$(LIBHILDONDESKTOP_CFLAGS)						\
	$(GNOME_VFS_CFLAGS)							\
	-DLOCALEDIR=\"$(localedir)\" 						\
	-DDATADIR=\"$(datadir)\"						\
	-DHD_DESKTOP_ENTRY_PATH=\"$(hildondesktopentrydir)\" 			\
	-DHD_DESKTOP_CONFIG_PATH=\"$(hildondesktopconfdir)\"			\
	-DHD_DESKTOP_MODULE_PATH=\"$(hildondesktoplibdir)\"			\
	-DHD_DESKTOP_MENU_PATH=\"$(hildondesktopmenudir)\"			\
	-DHD_DESKTOP_BACKGROUNDS_PATH=\"$(hildondesktopbackgroundsdir)\"	\
	-DHD_PLUGIN_LOADER_MODULES_PATH=\"$(hildonpluginloaderlibdir)\"		\
	-DHD_UI_POLICY_MODULES_PATH=\"$(hildonuipolicylibdir)\"

hildon_status_menu_SOURCES = \
	hildon-status-menu.c							\
	hd-status-area.c							\
	hd-status-area.h							\
	hd-status-menu.c							\
	hd-status-menu.h							\
	hd-status-menu-box.c							\
	hd-status-menu-box.h

hildon_status_menu_LDFLAGS = \
	-rdynamic           							\
	$(HILDON_LIBS)	    							\
	$(LIBHILDONDESKTOP_LIBS)						\
	$(GNOME_VFS_LIBS)
