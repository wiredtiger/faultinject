#
# faultinject
#
# Copyright (C) 2014 WiredTiger inc
# All rights reserved.
#

PACKAGE=faultinject

AC_INIT([faultinject],[01], [http://wiredtiger.com])
AC_CONFIG_SRCDIR([src])
AC_CONFIG_AUX_DIR(config)
AC_CONFIG_MACRO_DIR([m4])

PANDORA_CANONICAL_TARGET(less-warnings)
PANDORA_REQUIRE_LIBDL

AC_CHECK_SIZEOF(mode_t)
AC_CHECK_SIZEOF(int)

AC_CHECK_DECLS(open)
AC_CHECK_FUNCS(open)
AC_CHECK_DECLS(open64)
AC_CHECK_FUNCS(open64)

AC_CONFIG_FILES(Makefile docs/index.html)


