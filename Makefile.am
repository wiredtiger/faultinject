#
# faultinject
#

AM_CPPFLAGS = -I.

bin_PROGRAMS =
check_PROGRAMS =
noinst_PROGRAMS =
lib_LTLIBRARIES =
noinst_LTLIBRARIES =
nobase_include_HEADERS =
noinst_HEADERS =
CLEANFILES =
DISTCLEANFILES =
EXTRA_DIST = \
  docs/index.html

bin_SCRIPTS = faultinject
libexec_SCRIPTS = faultinject.sh

do_subst = @SED@ \
	-e 's!@''libdir''@!$(libdir)!g' \
	-e 's!@''libexecdir''@!$(libexecdir)!g'

faultinject: faultinject.in Makefile
	$(do_subst) $(srcdir)/faultinject.in > $@
	chmod +x faultinject

faultinject.sh: faultinject.sh.in Makefile
	$(do_subst) $(srcdir)/faultinject.sh.in > $@

lib_LTLIBRARIES += libfaultinject.la

libfaultinject_la_SOURCES = \
  src/faultinject.c

libfaultinject_la_CFLAGS = \
	$(AM_CFLAGS) \
  $(NO_WERROR) \
	-DBUILDING_FAULTINJECT

libfaultinject_la_LIBADD = $(LIBDL_LIBS) -lpthread
libfaultinject_la_LDFLAGS = $(AM_LDFLAGS) -avoid-version

