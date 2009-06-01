AC_INIT([hildon-status-menu], [0.3.17])
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
                  [PKG_CHECK_MODULES(HILDON, hildon-1 >= 2.1.4)])

AC_SUBST(HILDON_LIBS)
AC_SUBST(HILDON_CFLAGS)

PKG_CHECK_MODULES(LIBHILDONDESKTOP,
                  [libhildondesktop-1 >= 2.1.6],
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

hildondesktopconfdir=${sysconfdir}/hildon-desktop
AC_SUBST(hildondesktopconfdir)

hildonstatusmenudesktopentrydir=`pkg-config libhildondesktop-1 --variable=hildonstatusmenudesktopentrydir`
AC_SUBST(hildonstatusmenudesktopentrydir)

hildondesktoplibdir=`pkg-config libhildondesktop-1 --variable=hildondesktoplibdir`
AC_SUBST(hildondesktoplibdir)

# Maemolauncher
AC_ARG_ENABLE([maemo-launcher],
              [AS_HELP_STRING([--enable-maemo-launcher],
                              [build with maemo-launcher support])],
                              [case "${enableval}" in
                               yes) maemo_launcher=true ;;
                               no)  maemo_launcher=false ;;
                               *) AC_MSG_ERROR([bad value ${enableval} for --enable-maemo-launcher]) ;; esac], [maemo_launcher=false])

if test x$maemo_launcher = xtrue
then
	PKG_CHECK_MODULES(MAEMO_LAUNCHER, [maemo-launcher-app])
	AC_SUBST(MAEMO_LAUNCHER_CFLAGS)
	AC_SUBST(MAEMO_LAUNCHER_LIBS)
fi

AC_OUTPUT([
Makefile
src/status-menu.conf
src/status-menu.plugins
src/Makefile
])
