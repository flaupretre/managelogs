dnl Extracted from libxml2/configure.in

dnl --- save flags initially

_cppflags="${CPPFLAGS}"
_ldflags="${LDFLAGS}"


AC_ARG_WITH(zlib,
[  --with-zlib[[=DIR]]       use libz in DIR],[
  if test "$withval" != "no" -a "$withval" != "yes"; then
	Z_DIR=$withval
	CPPFLAGS="${CPPFLAGS} -I$withval/include"
	LDFLAGS="${LDFLAGS} -L$withval/lib"
  fi
])


WITH_ZLIB=0
ZLIB_OK=0
if test "$with_zlib" = "no"; then
	echo "Disabling zlib compression support"
	ZLIB_OK=1
	RPM_BUILD_OPTS="$RPM_BUILD_OPTS --without zlib"
else
	AC_CHECK_HEADERS(zlib.h,
		AC_CHECK_LIB(z, gzread,[
			AC_DEFINE([HAVE_ZLIB], [1], [Have zlib compression library])
			WITH_ZLIB=1
			AC_CHECK_PROG(GZIP_CMD,gzip,gzip)
			test -n "$GZIP_CMD" && ZLIB_OK=1
			RPM_RUNTIME_DEPS="$RPM_RUNTIME_DEPS zlib"
			RPM_BUILD_DEPS="$RPM_BUILD_DEPS zlib-devel"
			DEB_RUNTIME_DEPS="$DEB_RUNTIME_DEPS zlib1g"
			DEB_BUILD_DEPS="$DEB_BUILD_DEPS zlib1g-dev"
			if test "x${Z_DIR}" != "x"; then
				Z_CFLAGS="-I${Z_DIR}/include"
				Z_LIBS="-L${Z_DIR}/lib -lz"
				[case ${host} in
					*-*-solaris*)
						Z_LIBS="-L${Z_DIR}/lib -R${Z_DIR}/lib -lz"
						;;
				esac]
			else
				Z_LIBS="-lz"
			fi]))
fi

if test "$ZLIB_OK" = 0; then
	AC_MSG_ERROR(zlib not found)
fi

AC_SUBST(Z_CFLAGS)
AC_SUBST(Z_LIBS)
AC_SUBST(WITH_ZLIB)
AC_SUBST(GZIP_CMD)

dnl --- Restore flags

CPPFLAGS=${_cppflags}
LDFLAGS=${_ldflags}
