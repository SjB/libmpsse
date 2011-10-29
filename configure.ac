AC_PREREQ([2.65])
AC_INIT([libmpsse],[1.0],[http://libmpsse.googlecode.com])
AC_CONFIG_SRCDIR([mpsse.c])

AC_PROG_CC
AC_LANG(C)

LDFLAGS="-lftdi $LDFLAGS"
CFLAGS="-Wall $CFLAGS"

if ! test $SWIG
then
	SWIG="$(which swig)"
fi

if ! test $PYDEV
then
	PYDEV="/usr/include/python$(python --version 2>&1 | awk '{print $2}' | cut -d'.' -f1,2)"
fi

if ! test $PYLIB
then
	echo -ne "import sys\nfor path in sys.path: print path" > pylib.py
	PYLIB="$(python pylib.py | tail -1)"
	rm -f pylib.py
fi

AC_TYPE_SIZE_T
AC_FUNC_MALLOC

AC_CHECK_LIB([ftdi],[ftdi_init],[],[echo "error: missing libftdi library" && exit],[])
AC_CHECK_HEADERS([ftdi.h],[],[echo "error: missing libftdi header files" && exit])
AC_CHECK_HEADERS([stdlib.h stdint.h string.h],[],[echo "error: missing standard header files" && exit])
AC_CHECK_FUNCS([malloc memset],[],AC_MSG_ERROR(["Missing necessary function"]))

if test -e $SWIG
then
	if ! test -d $PYDEV
	then
		echo "error: python include directory '$PYDEV' does not exist"
		exit 1
	fi
else
	echo "error: swig was not found on the system"
	exit 1
fi

cp confdefs.h config.h

AC_SUBST(SWIG, $SWIG)
AC_SUBST(PYDEV, $PYDEV)
AC_SUBST(PYLIB, $PYLIB)
AC_CONFIG_FILES([Makefile])
AC_OUTPUT
