dnl aclocal.m4 for ICU
dnl Copyright (c) 1999-2000, International Business Machines Corporation and
dnl others. All Rights Reserved.
dnl Stephen F. Booth

dnl @TOP@

dnl ICU_CHECK_MH_FRAG
AC_DEFUN(ICU_CHECK_MH_FRAG, [
	AC_CACHE_CHECK(
		[which Makefile fragment to use],
		[icu_cv_host_frag],
		[
case "${host}" in
*-*-solaris*)
	if test "$ac_cv_prog_gcc" = yes; then	
		icu_cv_host_frag=mh-solaris-gcc 
	else
		if test "$SOL64" = yes; then
	                icu_cv_host_frag=mh-solaris-sparcv9  
		else
			icu_cv_host_frag=mh-solaris 
		fi
	fi ;;
*-*-irix*)	icu_cv_host_frag=mh-irix ;;
alpha*-*-linux-gnu)
	if test "$ac_cv_prog_gcc" = yes; then
		icu_cv_host_frag=mh-alpha-linux-gcc
	else  
		icu_cv_host_frag=mh-alpha-linux-cc
	fi ;;
*-dec-osf) icu_cv_host_frag=mh-alpha-osf ;;
*-*-linux*) icu_cv_host_frag=mh-linux ;;
*-*-cygwin)	icu_cv_host_frag=mh-cygwin ;;
*-*-freebsd*|*-*-netbsd*) 	icu_cv_host_frag=mh-bsd-gcc ;;
*-*-aix*) 	
	case "$CXX" in
	*vacpp*)icu_cv_host_frag=mh-aix-va ;;
	*)	icu_cv_host_frag=mh-aix ;;
	esac;;
*-sequent-*) 	icu_cv_host_frag=mh-ptx ;;
*-*-hpux*)
	case "$CXX" in 
	*aCC)    icu_cv_host_frag=mh-hpux-acc ;;
	*CC)     icu_cv_host_frag=mh-hpux-cc ;;
	esac;;
*-*-os390*)	icu_cv_host_frag=mh-os390 ;;
*-*-os400*)	icu_cv_host_frag=mh-os400 ;;
*-apple-rhapsody*)	icu_cv_host_frag=mh-darwin ;;
*-apple-darwin*)	icu_cv_host_frag=mh-darwin ;;
*) 		icu_cv_host_frag=mh-unknown ;;
esac
		]
	)
])

dnl ICU_CONDITIONAL - Taken from Automake 1.4
AC_DEFUN(ICU_CONDITIONAL,
[AC_SUBST($1_TRUE)
AC_SUBST($1_FALSE)
if $2; then
  $1_TRUE=
  $1_FALSE='#'
else
  $1_TRUE='#'
  $1_FALSE=
fi])

dnl AC_SEARCH_LIBS_FIRST(FUNCTION, SEARCH-LIBS [, ACTION-IF-FOUND
dnl            [, ACTION-IF-NOT-FOUND [, OTHER-LIBRARIES]]])
dnl Search for a library defining FUNC, then see if it's not already available.
 
AC_DEFUN(AC_SEARCH_LIBS_FIRST,
[AC_PREREQ([2.13])
AC_CACHE_CHECK([for library containing $1], [ac_cv_search_$1],
[ac_func_search_save_LIBS="$LIBS"
ac_cv_search_$1="no"
for i in $2; do
LIBS="-l$i $5 $ac_func_search_save_LIBS"
AC_TRY_LINK_FUNC([$1],
[ac_cv_search_$1="-l$i"
break])
done
if test "$ac_cv_search_$1" = "no"; then
AC_TRY_LINK_FUNC([$1], [ac_cv_search_$1="none required"])
fi
LIBS="$ac_func_search_save_LIBS"])
if test "$ac_cv_search_$1" != "no"; then
  test "$ac_cv_search_$1" = "none required" || LIBS="$ac_cv_search_$1 $LIBS"
  $3
else :
  $4
fi])

dnl Strict compilation options.
AC_DEFUN(AC_CHECK_STRICT_COMPILE,
[
    AC_MSG_CHECKING([whether strict compiling is on])
    AC_ARG_ENABLE(strict,[  --enable-strict         compile with strict compiler options [default=no]], [
    	if test "$enableval" = no
    	then
	    ac_use_strict_options=no
        else
	    ac_use_strict_options=yes
        fi
      ], [ac_use_strict_options=no])
    AC_MSG_RESULT($ac_use_strict_options)

    if test "$ac_use_strict_options" = yes
    then
        if test "$GCC" = yes
        then
	    CFLAGS="$CFLAGS -Wall -ansi -pedantic -Wshadow -Wpointer-arith -Wmissing-prototypes -Wwrite-strings"
        fi
        if test "$GXX" = yes
        then
	    CXXFLAGS="$CXXFLAGS -Wall -ansi -pedantic -W -Wpointer-arith -Wmissing-prototypes -Wwrite-strings"
        fi
    fi
])

dnl Define a sizeof checking macro that is a bit better than autoconf's
dnl builtin (and heavily based on it, of course). The new macro is
dnl AC_DO_CHECK_SIZEOF(TYPE [, CROSS_SIZE [, INCLUDES])
AC_DEFUN(AC_DO_CHECK_SIZEOF,
[changequote(<<, >>)dnl
dnl The name to #define.
define(<<AC_TYPE_NAME>>, translit(sizeof_$1, [a-z *], [A-Z_P]))dnl
dnl The cache variable name.
define(<<AC_CV_NAME>>, translit(ac_cv_sizeof_$1, [ *], [_p]))dnl
changequote([, ])dnl
AC_MSG_CHECKING(size of $1)
AC_CACHE_VAL(AC_CV_NAME,
[AC_TRY_RUN($3
[#include <stdio.h>
main()
{
  FILE *f=fopen("conftestval", "w");
  if (!f) exit(1);
  fprintf(f, "%d\n", sizeof($1));
  exit(0);
}], AC_CV_NAME=`cat conftestval`, AC_CV_NAME=0, ifelse([$2], , , AC_CV_NAME=$2))])dnl
AC_MSG_RESULT($AC_CV_NAME)
AC_DEFINE_UNQUOTED(AC_TYPE_NAME, $AC_CV_NAME)
undefine([AC_TYPE_NAME])dnl
undefine([AC_CV_NAME])dnl
])
