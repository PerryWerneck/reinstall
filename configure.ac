
dnl Process this file with autoconf to produce a configure script.

dnl The minimum version of autoconf required.
AC_PREREQ(2.59)

dnl Initialise automake with the package name, version and
dnl bug-reporting address.
AC_INIT([reinstall], [5.0.0], [perry.werneck@gmail.com])

dnl Place auxilliary scripts here.
AC_CONFIG_AUX_DIR([scripts])

dnl Compute the canonical host-system type
AC_CANONICAL_HOST

dnl Put macro definitions here (though they aren't used).
AC_CONFIG_HEADER([src/include/config.h])

dnl Initialise automake stuff.
AM_INIT_AUTOMAKE

dnl Check for iconv
AM_ICONV

dnl Set gettext version
AM_GNU_GETTEXT_VERSION([0.14])

dnl Checks for programs.
AC_PROG_CXX
AC_PROG_CC
AC_PROG_SED
AC_PROG_LN_S

AC_LANG([C++])

AC_PATH_TOOL([STRIP], [strip])
AC_SUBST(STRIP)
AC_PATH_TOOL([XGETTEXT], [xgettext], [no])
AC_PATH_TOOL([MSGCAT], [msgcat], [no])
AC_PATH_TOOL([MSGINIT], [msginit], [no])
AC_PATH_TOOL([MSGMERGE], [msgmerge], [no])
AC_PATH_TOOL([MSGFMT], [msgfmt], [no])

PKG_CHECK_EXISTS

dnl ---------------------------------------------------------------------------
dnl Version info
dnl ---------------------------------------------------------------------------

AC_SUBST(PACKAGE_DESCRIPTION,"Instalar/Reinstalar terminal")

app_vrs_major=$(echo $VERSION | cut -d. -f1)
app_vrs_minor=$(echo $VERSION | cut -d. -f2)
app_vrs_release=$(echo $VERSION | cut -d. -f3)

AC_DEFINE_UNQUOTED(PACKAGE_MAJOR_VERSION, $app_vrs_major, [The library major version])
AC_DEFINE_UNQUOTED(PACKAGE_MINOR_VERSION, $app_vrs_minor, [The library minor version])
AC_DEFINE_UNQUOTED(PACKAGE_MINOR_VERSION, $app_vrs_release, [The library release number])

AC_SUBST(PACKAGE_MAJOR_VERSION,$app_vrs_major)
AC_SUBST(PACKAGE_MINOR_VERSION,$app_vrs_minor)
AC_SUBST(PACKAGE_MINOR_VERSION,$app_vrs_minor)

AC_ARG_WITH([release], [AS_HELP_STRING([--with-release], [Set release])], [ app_cv_release="$withval" ],[ app_cv_release="`date +%-y`.`date +%-m`.`date +%-d`" ])

app_rls_major=$(echo $app_cv_release.0.0 | cut -d. -f1)
app_rls_minor=$(echo $app_cv_release.0.0 | cut -d. -f2)

app_vrs_revision=$(date +%Y%m%d)
AC_SUBST(PACKAGE_REVISION,$app_vrs_revision)
AC_DEFINE_UNQUOTED(PACKAGE_REVISION,$app_vrs_revision,[The package revision (for win32 installer)])

AC_DEFINE_UNQUOTED(PACKAGE_MAJOR_RELEASE, $app_rls_major, [The library major release])
AC_DEFINE_UNQUOTED(PACKAGE_MINOR_RELEASE, $app_rls_minor, [The library minor release])
AC_DEFINE_UNQUOTED(PACKAGE_RELEASE, "$app_cv_release", [The package release])

AC_SUBST(PACKAGE_RELEASE,"$app_cv_release")
AC_SUBST(PACKAGE_MAJOR_RELEASE,$app_rls_major)
AC_SUBST(PACKAGE_MINOR_RELEASE,$app_rls_minor)

dnl ---------------------------------------------------------------------------
dnl Check for OS specifics
dnl ---------------------------------------------------------------------------

app_cv_modules="isowriter isobuilder"
AC_CONFIG_FILES(src/modules/isowriter/Makefile)
AC_CONFIG_FILES(src/modules/isobuilder/Makefile)

case "$host" in
	*-mingw32|*-pc-msys)
		EXEEXT=".exe"
		OSNAME="windows"

		CFLAGS="$CFLAGS -D_WIN32_WINNT=0x0600"
		DLL_LDFLAGS="-shared -Wl,--output-def,\$(@D)/\$(LIBNAME).def"
		DLLEXT=".dll"
		AC_SUBST(SONAME,"reinstall.dll")

		app_win32_revision=$(date +%y.%m.%d.%H)
		AC_SUBST(WIN32_VERSION,$app_win32_revision)

		app_win32_fileversion=$(date +%_y,%_m,%_d,%_H)
		AC_SUBST(WIN32_FILEVERSION,$app_win32_fileversion)

		if test "$host_cpu" = "x86_64"; then
			app_cv_programfiles="PROGRAMFILES64"
			app_cv_winarch="64"
		else
			app_cv_programfiles="PROGRAMFILES"
			app_cv_winarch="32"
		fi
		
		AC_SUBST(PROGRAMFILES,$app_cv_programfiles)
		AC_SUBST(WINARCH,$app_cv_winarch)

		AC_PATH_TOOL([WINDRES], [windres], [no])
		AC_PATH_TOOL([ZIP], [zip], [no])
dnl		AC_CONFIG_FILES(src/os/windows/resources.rc)
dnl		AC_CONFIG_FILES(src/os/windows/app-manifest.xml)
dnl		AC_CONFIG_FILES(win/userinfo.nsi)
		;;

 	*)
		EXEEXT=""
		DLLEXT=".so"
		OSNAME="linux"
		CFLAGS="$CFLAGS -pthread"
		LDFLAGS="$LDFLAGS -pthread"
		DLL_LDFLAGS="-shared -Wl,-soname,\$(@F)"
		AC_SUBST(SONAME,libreinstall.so.$app_vrs_major.$app_vrs_minor)
		AC_CONFIG_FILES(sysconf/launcher.desktop)

		app_cv_modules="$app_cv_modules netinstall grub"

		AC_CONFIG_FILES(src/modules/netinstall/Makefile)
		AC_CONFIG_FILES(src/modules/grub/Makefile)

esac

AC_SUBST(LIBS)
AC_SUBST(EXEEXT)
AC_SUBST(DLLEXT)
AC_SUBST(OSNAME)
AC_SUBST(DLL_LDFLAGS)

dnl ---------------------------------------------------------------------------
dnl Check for other programs
dnl ---------------------------------------------------------------------------
AC_PATH_TOOL([DOXYGEN], [doxygen])
AC_PATH_TOOL([VALGRIND], [valgrind], [no])
AC_PATH_TOOL([AR], [ar], [ar])
AC_PATH_TOOL([CONVERT], [convert], [no])
AC_PATH_TOOL([INSTALL_DESKTOP],[desktop-file-install],[no])

dnl ---------------------------------------------------------------------------
dnl test for usefull functions
dnl ---------------------------------------------------------------------------

AC_CHECK_FUNCS(pwrite, AC_DEFINE(HAVE_PWRITE))
AC_CHECK_FUNCS(localtime_r, AC_DEFINE(HAVE_LOCALTIME_R) )

dnl ---------------------------------------------------------------------------
dnl test for reentrant time routines
dnl ---------------------------------------------------------------------------

AC_MSG_CHECKING(for localtime_r in time.h)
AC_EGREP_HEADER(localtime_r, time.h,
 [ AC_DEFINE([HAVE_LOCALTIME_R], [1], [Define to 1 if localtime_r is available.]) AC_MSG_RESULT(yes) ],
 AC_MSG_RESULT(no))

AC_MSG_CHECKING(for gmtime_r in time.h)
AC_EGREP_HEADER(gmtime_r, time.h,
 [ AC_DEFINE([HAVE_GMTIME_R], [1], [Define to 1 if gmtime_r is available.]) AC_MSG_RESULT(yes) ],
 AC_MSG_RESULT(no))

dnl ---------------------------------------------------------------------------
dnl Check for C++ 2017 support
dnl ---------------------------------------------------------------------------
AC_DEFUN([AX_CHECK_COMPILE_FLAG],
[AC_PREREQ(2.59)dnl for _AC_LANG_PREFIX
AS_VAR_PUSHDEF([CACHEVAR],[ax_cv_check_[]_AC_LANG_ABBREV[]flags_$4_$1])dnl
AC_CACHE_CHECK([whether _AC_LANG compiler accepts $1], CACHEVAR, [
  ax_check_save_flags=$[]_AC_LANG_PREFIX[]FLAGS
  _AC_LANG_PREFIX[]FLAGS="$[]_AC_LANG_PREFIX[]FLAGS $4 $1"
  AC_COMPILE_IFELSE([AC_LANG_PROGRAM()],
    [AS_VAR_SET(CACHEVAR,[yes])],
    [AS_VAR_SET(CACHEVAR,[no])])
  _AC_LANG_PREFIX[]FLAGS=$ax_check_save_flags])
AS_IF([test x"AS_VAR_GET(CACHEVAR)" = xyes],
  [m4_default([$2], :)],
  [m4_default([$3], :)])
AS_VAR_POPDEF([CACHEVAR])dnl
])dnl AX_CHECK_COMPILE_FLAGS

AX_CHECK_COMPILE_FLAG([-std=gnu++17], [CPPFLAGS="$CPPFLAGS -std=gnu++17"])

dnl ---------------------------------------------------------------------------
dnl Check for libudjat
dnl ---------------------------------------------------------------------------

PKG_CHECK_MODULES( [UDJAT], [libudjat], AC_DEFINE(HAVE_UDJAT,,[Do we have udjat?]), AC_MSG_ERROR([UDJAT not present.]) )
AC_SUBST(UDJAT_LIBS)
AC_SUBST(UDJAT_CFLAGS)

AC_ARG_WITH([product-name], [AS_HELP_STRING([--with-product-name], [Set product name])], [ app_cv_product="$withval" ],[ app_cv_product=$PACKAGE ])
AC_DEFINE_UNQUOTED(PRODUCT_NAME, "$app_cv_product",[The product name])
AC_SUBST(PRODUCT_NAME,"$app_cv_product")

AC_ARG_WITH([product-id], [AS_HELP_STRING([--with-product-id], [Set product id])], [ app_cv_product_id="$withval" ],[ app_cv_product_id="`pkg-config --variable=product_id libudjat`" ])
AC_SUBST(PRODUCT_ID,$app_cv_product_id)
AC_DEFINE_UNQUOTED(PRODUCT_ID,"$app_cv_product_id",[The product id])

AC_ARG_WITH([module-path], [AS_HELP_STRING([--with-module-path], [Set module-path])], [ app_cv_module_path="$withval" ],[ app_cv_module_path=`pkg-config --variable=module_path libudjat` ])

AC_DEFINE_UNQUOTED(MODULE_PATH, $app_cv_module_path, [The path for module installation])
AC_SUBST(MODULE_PATH,$app_cv_module_path)

AC_DEFINE_UNQUOTED(GETTEXT_PACKAGE,"$app_cv_product",[The gettext package name.])
AC_SUBST(GETTEXT_PACKAGE,$app_cv_product)

dnl ---------------------------------------------------------------------------
dnl Check for libslp
dnl ---------------------------------------------------------------------------

AC_CHECK_HEADER(slp.h, [app_cv_libslp="yes"], [AC_MSG_NOTICE([Can't find required file slp.h])])

if test x$app_cv_libslp = xyes; then
        AC_DEFINE(HAVE_LIBSLP, 1, [OpenSLP is available])
        AC_SUBST(LIBSLP_LIBS,"-lslp")
else
        AC_SUBST(LIBSLP_LIBS,"")
fi

AC_SUBST(LIBSLP_CFLAGS)

dnl ---------------------------------------------------------------------------
dnl Check for ZIPLIB
dnl ---------------------------------------------------------------------------

PKG_CHECK_MODULES( [ZIPLIB], [libzip], AC_DEFINE(HAVE_ZIPLIB,,[ "Do we have zziplib?" ]), AC_MSG_ERROR([ziplib not present.]))

AC_SUBST(ZIPLIB_LIBS)
AC_SUBST(ZIPLIB_CFLAGS)

dnl ---------------------------------------------------------------------------
dnl Check for GTK
dnl ---------------------------------------------------------------------------

GLIB_GSETTINGS

PKG_CHECK_MODULES( [GTK], [gtkmm-3.0], AC_DEFINE(HAVE_GTK,,[ "Do we have GTK++?" ]), AC_MSG_ERROR([GTK3++ not present.]))

AC_SUBST(GTK_LIBS)
AC_SUBST(GTK_CFLAGS)

dnl ---------------------------------------------------------------------------
dnl Check for pugixml
dnl ---------------------------------------------------------------------------

PKG_CHECK_MODULES(PUGIXML, pugixml, AC_DEFINE(HAVE_PUGIXML,[],[Do we have PUGIXML?]),AC_MSG_ERROR(pugixml is required))
AC_SUBST(PUGIXML_LIBS)
AC_SUBST(PUGIXML_CFLAGS)

dnl ---------------------------------------------------------------------------
dnl Check for fdisk
dnl ---------------------------------------------------------------------------

PKG_CHECK_MODULES(FDISK, fdisk, AC_DEFINE(HAVE_FDISK,[],[Do we have libfdisk?]),AC_MSG_ERROR(libfdisk is required))
AC_SUBST(FDISK_LIBS)
AC_SUBST(FDISK_CFLAGS)

dnl ---------------------------------------------------------------------------
dnl Check for libburn
dnl ---------------------------------------------------------------------------

PKG_CHECK_MODULES( [BURN], [libburn-1], AC_DEFINE(HAVE_BURN,[],[Do we have libburn?]), AC_MSG_NOTICE([libburn not present.]) )

AC_SUBST(BURN_LIBS)
AC_SUBST(BURN_CFLAGS)

dnl ---------------------------------------------------------------------------
dnl Check for Libisofs
dnl ---------------------------------------------------------------------------

PKG_CHECK_MODULES( [ISOFS], [libisofs-1], AC_DEFINE(HAVE_ISOFS,[],[Do we have libisofs?]), AC_MSG_NOTICE([libisofs not present.]) )

AC_SUBST(ISOFS_LIBS)
AC_SUBST(ISOFS_CFLAGS)

dnl ---------------------------------------------------------------------------
dnl Check for libdbus
dnl ---------------------------------------------------------------------------

PKG_CHECK_MODULES( [DBUS], [dbus-1], AC_DEFINE(HAVE_DBUS,[],[Do we have d-bus?]), AC_MSG_NOTICE([ No DBUS support.]) )
AC_SUBST(DBUS_LIBS)
AC_SUBST(DBUS_CFLAGS)

dnl ---------------------------------------------------------------------------
dnl Check for jsoncpp
dnl ---------------------------------------------------------------------------
dnl 
PKG_CHECK_MODULES( [JSON], [jsoncpp], AC_DEFINE(HAVE_GUESTFS,[],[Do we have jsoncpp?]), AC_MSG_ERROR([jsoncpp not present.]) )

AC_SUBST(JSON_LIBS)
AC_SUBST(JSON_CFLAGS)

dnl ---------------------------------------------------------------------------
dnl Output config
dnl ---------------------------------------------------------------------------

AC_SUBST(SRCDIR,$ac_pwd)
AC_SUBST(BINDIR,$ac_pwd/.bin)
AC_SUBST(MODULES,$app_cv_modules)

dnl ---------------------------------------------------------------------------
dnl Configure which files to generate.
dnl ---------------------------------------------------------------------------

AC_CONFIG_FILES(src/library/Makefile)
AC_CONFIG_FILES(locale/Makefile)
AC_CONFIG_FILES(Makefile)

dnl ---------------------------------------------------------------------------
dnl Output the generated config.status script.
dnl ---------------------------------------------------------------------------
AC_OUTPUT

