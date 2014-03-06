# Optional configuration.
AC_DEFUN([AM_OPTIONS], [

AH_TEMPLATE(HAVE_TRACE, [Define to 1 for trace functionality.])
AC_MSG_CHECKING(if --enable-trace option specified)
AC_ARG_ENABLE(trace,
	[AS_HELP_STRING([--enable-trace],
	    [Configure for trace functionality.])], r=$enableval, r=no)
case "$r" in
no)	wt_cv_enable_trace=no;;
*)	AC_DEFINE(HAVE_TRACE)
	wt_cv_enable_trace=yes;;
esac
AC_MSG_RESULT($wt_cv_enable_trace)

])
