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
else
    AC_CHECK_HEADERS(zlib.h,
        AC_CHECK_LIB(z, gzread,[
            AC_DEFINE([HAVE_ZLIB], [1], [Have zlib compression library])
            WITH_ZLIB=1
			RPM_DEPS="$RPM_DEPS zlib"
			ZLIB_OK=1
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
	AC_MSG_ERROR(bzip2 library not found. Either install the zlib package, use the --with-zlib option, or use --without-zlib to disable zlib compression.)
fi

AC_SUBST(Z_CFLAGS)
AC_SUBST(Z_LIBS)
AC_SUBST(WITH_ZLIB)

dnl --- Restore flags

CPPFLAGS=${_cppflags}
LDFLAGS=${_ldflags}
