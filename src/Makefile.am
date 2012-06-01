bin_PROGRAMS = hildon-status-menu

hildondesktopconf_DATA = \
	status-menu.conf	\
	status-menu.plugins

hildon_status_menu_CFLAGS = \
	$(HILDON_CFLAGS)							\
	$(LIBHILDONDESKTOP_CFLAGS)						\
	$(GNOME_VFS_CFLAGS)							\
	-DHD_DESKTOP_CONFIG_PATH=\"$(hildondesktopconfdir)\"			\
	$(MAEMO_LAUNCHER_CFLAGS)

hildon_status_menu_SOURCES = \
	hildon-status-menu.c							\
	hd-status-area.c							\
	hd-status-area.h							\
	hd-status-area-box.c							\
	hd-status-area-box.h							\
	hd-status-menu.c							\
	hd-status-menu.h							\
	hd-status-menu-box.c							\
	hd-status-menu-box.h							\
	hd-desktop.c								\
	hd-desktop.h								\
	hd-display.c								\
	hd-display.h

hildon_status_menu_LDFLAGS = \
	$(HILDON_LIBS)	    							\
	$(LIBHILDONDESKTOP_LIBS)						\
	$(X11_LIBS)								\
	$(GNOME_VFS_LIBS)							\
	$(MAEMO_LAUNCHER_LIBS)
