AC_INIT([k8s-logger],[1.4],[https://github.com/kernelkit/k8s-logger/issues])

AC_CONFIG_AUX_DIR(.aux)
AM_INIT_AUTOMAKE([1.11 foreign])

AC_CONFIG_SRCDIR([main.c])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_FILES([Makefile])

AC_PROG_CC
AC_PROG_INSTALL

AC_ARG_WITH([syslogp], AS_HELP_STRING([--with-syslogp], [Build with sysklogd's syslogp() interface]))

AS_IF([test "x$with_syslogp" = "xyes"], [
   PKG_CHECK_MODULES([syslog], [libsyslog >= 2.0.0])
   AC_DEFINE(HAVE_SYSLOGP, 1, [System has sysklogd and a libsyslog with syslogp()])
])
PKG_CHECK_MODULES([lite], [libite >= 2.4.0])

AM_CONDITIONAL(SYSLOGP, [test "x$with_syslogp" = "xyes"])
AC_OUTPUT

cat <<EOF

------------------ Summary ------------------
 $PACKAGE_NAME version $PACKAGE_VERSION
  Prefix................: $prefix
  Exec prefix...........: $eprefix
  Sysconfdir............: `eval echo $sysconfdir`
  Localstatedir.........: `eval echo $localstatedir`
  System environment....: ${sysconfig_path:-${sysconfig}}
  C Compiler............: $CC $CFLAGS $CPPFLAGS $LDFLAGS $LIBS

Optional features:
  Use syslogp() API.....: $with_syslogp

------------- Compiler version --------------
$($CC --version || true)
---------------------------------------------

Check the above options and compile with:
 ${MAKE-make}

EOF
