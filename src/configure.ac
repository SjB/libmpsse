AC_PREREQ([2.65])
AC_INIT([libmpsse],[1.3],[http://libmpsse.googlecode.com])

AC_ARG_ENABLE([python],AS_HELP_STRING([--disable-python], [Do not build Python wrappers]))
AS_IF([test "x$enable_python" != "xno"], [BUILD="swig"], [BUILD=""])

AC_CONFIG_SRCDIR([mpsse.c])

AC_PROG_CC
AC_LANG(C)

AC_TYPE_SIZE_T
AC_FUNC_MALLOC

CFLAGS="-Wall -fPIC -fno-strict-aliasing $CFLAGS"

AC_CHECK_HEADERS([libftdi1/ftdi.h], [LIBFTDI1=1], [LIBFTDI1=""])

if ! test $LIBFTDI1
then
	AC_CHECK_HEADERS([ftdi.h],[LIBFTDI1=0],[echo "error: missing libftdi header files" && exit 1])
	AC_CHECK_LIB([ftdi],[ftdi_init],[],[echo "error: missing libftdi library" && exit 1],[])
	LDFLAGS="-lftdi $LDFLAGS"
else
	LDFLAGS="-lftdi1 $LDFLAGS"
fi

if test $BUILD
then
	if ! test $SWIG
	then
		SWIG="$(which swig)"
	fi

	if ! test $SWIG
	then
		echo "error: could not locate swig"
		exit 1
	fi

	if ! test $PYDEV
	then
		PYDEV="/usr/include/python$(python --version 2>&1 | awk '{print $2}' | cut -d'.' -f1,2)"
	fi

	AC_CHECK_HEADERS([$PYDEV/Python.h],[],[echo "error: missing python header files" && exit 1])

	if ! test $PYLIB
	then
		dnl This is a hack to get a valid python library path
		echo "import sys" > pylib.py
		echo "for path in sys.path: print path" >> pylib.py
		PYLIB="$(python pylib.py | grep -e '-packages$' | head -1)"
		rm -f pylib.py
	fi

	if ! test -d $PYLIB
	then
		echo "error: could not locate python library path"
		exit 1
	fi
fi

dnl OSX specific compiler flags
if test "$(uname)" == "Darwin"
then
	SONAME="-install_name"
	FRAMEWORK="-framework Python"
else
	SONAME="-soname"
	FRAMEWORK=""
fi

cp confdefs.h config.h

AC_SUBST(BUILD, $BUILD)
AC_SUBST(SWIG, $SWIG)
AC_SUBST(PYDEV, $PYDEV)
AC_SUBST(PYLIB, $PYLIB)
AC_SUBST(SONAME, $SONAME)
AC_SUBST(FRAMEWORK, $FRAMEWORK)
AC_SUBST(LIBFTDI1, $LIBFTDI1)
AC_CONFIG_FILES([Makefile])
AC_OUTPUT
