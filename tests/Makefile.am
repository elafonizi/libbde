AM_CPPFLAGS = \
	-I$(top_srcdir)/include \
	-I$(top_srcdir)/common \
	@LIBCSTRING_CPPFLAGS@ \
	@LIBCERROR_CPPFLAGS@ \
	@LIBCTHREADS_CPPFLAGS@ \
	@LIBCDATA_CPPFLAGS@ \
	@LIBCLOCALE_CPPFLAGS@ \
	@LIBCNOTIFY_CPPFLAGS@ \
	@LIBCSPLIT_CPPFLAGS@ \
	@LIBUNA_CPPFLAGS@ \
	@LIBCFILE_CPPFLAGS@ \
	@LIBCPATH_CPPFLAGS@ \
	@LIBBFIO_CPPFLAGS@ \
	@LIBCSYSTEM_CPPFLAGS@ \
	@PTHREAD_CPPFLAGS@ \
	@LIBBDE_DLL_IMPORT@

if HAVE_PYTHON_TESTS
TESTS_PYBDE = \
	test_python_functions.sh
endif

TESTS = \
	test_api_functions.sh \
	test_bdeinfo.sh \
	$(TESTS_PYBDE)

check_SCRIPTS = \
	pybde_test_get_version.py \
	pybde_test_open_close.py \
	pybde_test_seek.py \
	test_api_functions.sh \
	test_bdeinfo.sh \
	test_python_functions.sh \
	test_runner.sh

EXTRA_DIST = \
	$(check_SCRIPTS)

check_PROGRAMS = \
	bde_test_get_version \
	bde_test_open_close \
	bde_test_read \
	bde_test_seek

bde_test_get_version_SOURCES = \
	bde_test_libbde.h \
	bde_test_libcstring.h \
	bde_test_get_version.c

bde_test_get_version_LDADD = \
	../libbde/libbde.la \
	@LIBCSTRING_LIBADD@

bde_test_open_close_SOURCES = \
	bde_test_libbde.h \
	bde_test_libcerror.h \
	bde_test_libcstring.h \
	bde_test_libcsystem.h \
	bde_test_open_close.c \
	bde_test_unused.h

bde_test_open_close_LDADD = \
	@LIBCSYSTEM_LIBADD@ \
	../libbde/libbde.la \
	@LIBCERROR_LIBADD@ \
	@LIBCSTRING_LIBADD@ \
	@PTHREAD_LIBADD@

bde_test_read_SOURCES = \
	bde_test_libcerror.h \
	bde_test_libcstring.h \
	bde_test_libcsystem.h \
	bde_test_libcthreads.h \
	bde_test_libbde.h \
	bde_test_read.c \
	bde_test_unused.h

bde_test_read_LDADD = \
	@LIBCSYSTEM_LIBADD@ \
	../libbde/libbde.la \
	@LIBCTHREADS_LIBADD@ \
	@LIBCERROR_LIBADD@ \
	@LIBCSTRING_LIBADD@ \
	@PTHREAD_LIBADD@

bde_test_seek_SOURCES = \
	bde_test_libcerror.h \
	bde_test_libcstring.h \
	bde_test_libcsystem.h \
	bde_test_libbde.h \
	bde_test_seek.c \
	bde_test_unused.h

bde_test_seek_LDADD = \
	@LIBCSYSTEM_LIBADD@ \
	../libbde/libbde.la \
	@LIBCERROR_LIBADD@ \
	@LIBCSTRING_LIBADD@ \
	@PTHREAD_LIBADD@

MAINTAINERCLEANFILES = \
	Makefile.in

distclean: clean
	/bin/rm -f Makefile

