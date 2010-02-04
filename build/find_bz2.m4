dnl Derived from libxml2/configure.in

dnl --- save flags initially

_cppflags="${CPPFLAGS}"
_ldflags="${LDFLAGS}"

AC_ARG_WITH(bz2,
[  --with-bz2[[=DIR]]        use bz2 library in DIR],[
  if test "$withval" != "no" -a "$withval" != "yes"; then
    BZ2_DIR=$withval
    CPPFLAGS="${CPPFLAGS} -I$withval/include"
    LDFLAGS="${LDFLAGS} -L$withval/lib"
  fi
])


WITH_BZ2=0
BZ2_OK=0
if test "$with_bz2" = "no"; then
    echo "Disabling bz2 compression support"
	BZ2_OK=1
else
    AC_CHECK_HEADERS(bzlib.h,
        AC_CHECK_LIB(bz2, BZ2_bzCompress,[
            AC_DEFINE([HAVE_BZ2], [1], [Have bzip2 compression library])
            WITH_BZ2=1
			BZ2_OK=1
			RPM_DEPS="$RPM_DEPS bzip2-libs"
            if test "x${BZ2_DIR}" != "x"; then
                BZ2_CFLAGS="-I${BZ2_DIR}/include"
                BZ2_LIBS="-L${BZ2_DIR}/lib -lbz2"
                [case ${host} in
                    *-*-solaris*)
                        BZ2_LIBS="-L${BZ2_DIR}/lib -R${BZ2_DIR}/lib -lbz2"
                        ;;
                esac]
            else
                BZ2_LIBS="-lbz2"
            fi]))
fi

if test "$BZ2_OK" = 0; then
	AC_MSG_ERROR(bzip2 library not found. Either install the bzip2-libs package, use the --with-bz2 option, or use --without-bz2 to disable bzip2 compression.)
fi

AC_SUBST(BZ2_CFLAGS)
AC_SUBST(BZ2_LIBS)
AC_SUBST(WITH_BZ2)

dnl --- Restore flags

CPPFLAGS=${_cppflags}
LDFLAGS=${_ldflags}
