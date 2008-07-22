AC_INIT([hildon-status-menu], [2.1.0])
AC_CANONICAL_SYSTEM

AM_CONFIG_HEADER(config.h)
AC_CONFIG_SRCDIR([src/hd-status-menu.c])

AM_MAINTAINER_MODE

AM_INIT_AUTOMAKE

AC_PROG_CC
AC_PROG_INSTALL
AM_PROG_LIBTOOL

AC_HEADER_STDC

AC_PATH_X
AC_PATH_XTRA
AC_SUBST(X_CFLAGS)

#+++++++++++++++
# Misc programs 
#+++++++++++++++

AC_PATH_PROG(GLIB_GENMARSHAL, glib-genmarshal)

GTK_DOC_CHECK([1.3])

#++++++++++++++++++
# Misc build flags
#++++++++++++++++++

AC_ARG_ENABLE(debug, 
	      [AC_HELP_STRING([--enable-debug],[Debug (default=no)])],
              [hildon_use_debug=yes],[hildon_use_debug=no])

AC_ARG_ENABLE(timestamping,
	      [AC_HELP_STRING([--enable-timestamping],[Define HILDON_USE_TIMESTAMPING (default=no)])],
	      [hildon_use_timestamping=yes],[hildon_use_timestamping=no])

AC_ARG_ENABLE(instrumenting,
	      [AC_HELP_STRING([--enable-instrumenting],[Compile with instrumentation flags (default=no)])],
	      [hildon_use_instrumenting=yes],[hildon_use_instrumenting=no])

if test "x${hildon_use_timestamping}" = "xyes"
then
    CFLAGS="$CFLAGS -DHILDON_USE_TIMESTAMPING"
fi

if test "x${hildon_use_instrumenting}" = "xyes"
then
    CFLAGS="$CFLAGS -Wall -Wmissing-prototypes -Wmissing-declarations -Werror -Wno-format-extra-args -g -finstrument-functions"
else
    CFLAGS="$CFLAGS -Wall -Wmissing-prototypes -Wmissing-declarations -Werror -Wno-format-extra-args"
fi

AC_SUBST(CFLAGS)

#++++++++++++
# i18n setup 
#++++++++++++

# ALL_LINGUAS="en_GB"
# AC_SUBST(ALL_LINGUAS)
# AM_GLIB_GNU_GETTEXT
# AC_DEFINE(GETTEXT_PACKAGE, ["maemo-af-desktop"], [Package name for gettext])

PKG_CHECK_MODULES(HILDON,
                  [hildon-1], 
                  [AC_DEFINE(HAVE_LIBHILDON, [], [Whether libhildon-1 is present on the system])], 
                  [PKG_CHECK_MODULES(HILDON, hildon-1 >= 2.1.0)])

AC_SUBST(HILDON_LIBS)
AC_SUBST(HILDON_CFLAGS)

PKG_CHECK_MODULES(LIBHILDONDESKTOP,
                  [libhildondesktop-1], 
                  [AC_DEFINE(HAVE_LIBHILDONDESKTOP, [], [Whether libhildondesktop-1 is present on the system])], 
                  [])

AC_SUBST(LIBHILDONDESKTOP_LIBS)
AC_SUBST(LIBHILDONDESKTOP_CFLAGS)

PKG_CHECK_MODULES(GNOME_VFS, gnome-vfs-2.0 >= 2.8.3)
AC_SUBST(GNOME_VFS_CFLAGS)
AC_SUBST(GNOME_VFS_LIBS)

#+++++++++++++++++++
# Directories setup
#+++++++++++++++++++

localedir="${datadir}/locale"
hildondesktoplibdir="${libdir}/hildon-desktop"

hildonhomedesktopentrydir="${datadir}/applications/hildon-home"
hildonstatusmenudesktopentrydir="${datadir}/applications/hildon-status-menu"

AC_SUBST(localedir)

hildondesktopconfdir=${sysconfdir}/hildon-desktop
#AC_SUBST(hildondesktopentrydir)
AC_SUBST(hildondesktoplibdir)
AC_SUBST(hildondesktopconfdir)

# Expand nested variables
eval "hildonhomedesktopentrydir=$hildonhomedesktopentrydir"
AC_SUBST(hildonhomedesktopentrydir)

# Expand nested variables
eval "hildonstatusmenudesktopentrydir=$hildonstatusmenudesktopentrydir"
AC_SUBST(hildonstatusmenudesktopentrydir)

hildondesktopbackgroundsdir=${datadir}/backgrounds
AC_SUBST(hildondesktopbackgroundsdir)

hildoncpdesktopentrydir=${datadir}/applications/hildon-control-panel
AC_SUBST(hildoncpdesktopentrydir)

hildonpluginloaderlibdir=${hildondesktoplibdir}/loaders
AC_SUBST(hildonpluginloaderlibdir)

hildonuipolicylibdir=${hildondesktoplibdir}/policies
AC_SUBST(hildonuipolicylibdir)

hildondesktopstartupdir=${sysconfdir}/osso-af-init
AC_SUBST(hildondesktopstartupdir)

hildondesktopmenudir=${sysconfdir}/xdg/menus
AC_SUBST(hildondesktopmenudir)

AC_OUTPUT([
Makefile
hildon-status-menu.desktop
src/status-menu.conf
src/Makefile
])
