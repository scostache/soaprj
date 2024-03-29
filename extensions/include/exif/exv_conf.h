/* ./config/config.h.  Generated from config.h.in by configure.  */
/* config.h.in.  Generated from configure.ac by autoheader.  */

/* Define to 1 if you have the `alarm' function. */
/* #undef EXV_HAVE_ALARM */

/* Define to 1 if you have the declaration of `strerror_r', and to 0 if you
   don't. */
#define EXV_HAVE_DECL_STRERROR_R 1

/* Define to 1 if you have the `gmtime_r' function. */
#define EXV_HAVE_GMTIME_R 1

/* Define to 1 if you have the <inttypes.h> header file. */
#define EXV_HAVE_INTTYPES_H 1

/* Define to 1 if you have the <libintl.h> header file. */
#define EXV_HAVE_LIBINTL_H 1

#ifndef EXV_COMMERCIAL_VERSION

/* Define to 1 to enable translation of Exif.Nikon3.LensData. */
#define EXV_HAVE_LENSDATA 1

/* Define to 1 if translation of program messages to the user's
   native language is requested. */
#define EXV_ENABLE_NLS 1

/* Define to 1 if you have the `iconv' function. */
#define EXV_HAVE_ICONV 1

/* Define to 1 to enable conversion of UCS2 encoded Windows tags to UTF-8. */
#define EXV_HAVE_PRINTUCS2 1

#endif /* !EXV_COMMERCIAL_VERSION */

/* Define to `const' or to empty, depending on the second argument of `iconv'. */
#define EXV_ICONV_CONST 

/* Define to 1 if you have the <libintl.h> header file. */
#define EXV_HAVE_LIBINTL_H 1

/* Define to 1 if your system has a GNU libc compatible `malloc' function, and
   to 0 otherwise. */
/* #undef EXV_HAVE_MALLOC */

/* Define to 1 if you have the <malloc.h> header file. */
#define EXV_HAVE_MALLOC_H 1

/* Define to 1 if you have the <memory.h> header file. */
#define EXV_HAVE_MEMORY_H 1

/* Define to 1 if you have the `memset' function. */
#define EXV_HAVE_MEMSET 1

/* Define to 1 if your system has a GNU libc compatible `realloc' function,
   and to 0 otherwise. */
/* #undef EXV_HAVE_REALLOC */

/* Define to 1 if stdbool.h conforms to C99. */
#define EXV_HAVE_STDBOOL_H 1

/* Define to 1 if you have the <stdint.h> header file. */
#define EXV_HAVE_STDINT_H 1

/* Define to 1 if you have the <stdlib.h> header file. */
#define EXV_HAVE_STDLIB_H 1

/* Define to 1 if you have the `strchr' function. */
#define EXV_HAVE_STRCHR 1

/* Define to 1 if you have the `strerror' function. */
#define EXV_HAVE_STRERROR 1

/* Define to 1 if you have the `strerror_r' function. */
#define EXV_HAVE_STRERROR_R 1

/* Define to 1 if you have the <strings.h> header file. */
#define EXV_HAVE_STRINGS_H 1

/* Define to 1 if you have the <string.h> header file. */
#define EXV_HAVE_STRING_H 1

/* Define to 1 if you have the `strtol' function. */
#define EXV_HAVE_STRTOL 1

/* Define to 1 if you have the `mmap' function. */
#define EXV_HAVE_MMAP 1

/* Define to 1 if you have the `munmap' function. */
#define EXV_HAVE_MUNMAP 1

/* Define to 1 if you have the <sys/stat.h> header file. */
#define EXV_HAVE_SYS_STAT_H 1

/* Define to 1 if you have the <sys/time.h> header file. */
/* #undef EXV_HAVE_SYS_TIME_H */

/* Define to 1 if you have the <sys/types.h> header file. */
#define EXV_HAVE_SYS_TYPES_H 1

/* Define to 1 if you have the `timegm' function. */
#define EXV_HAVE_TIMEGM 1

/* Define to 1 if you have the <unistd.h> header file. */
#define EXV_HAVE_UNISTD_H 1

/* Define to 1 if you have the `vprintf' function. */
/* #undef EXV_HAVE_VPRINTF */

/* Define to 1 if you have the <wchar.h> header file. */
/* #undef EXV_HAVE_WCHAR_H */

/* Define to 1 if you have the <sys/mman.h> header file. */
#define EXV_HAVE_SYS_MMAN_H 1

/* Define to 1 if you have the `zlib' library. */
#define EXV_HAVE_LIBZ 1

/* Define to 1 if you have the Adobe XMP Toolkit. */
/* #undef EXV_HAVE_XMP_TOOLKIT */

/* Define to 1 if the system has the type `_Bool'. */
#define EXV_HAVE__BOOL 1

/* Define to 1 if `lstat' dereferences a symlink specified with a trailing
   slash. */
/* #undef EXV_LSTAT_FOLLOWS_SLASHED_SYMLINK */

/* Define to the address where bug reports for this package should be sent. */
#define EXV_PACKAGE_BUGREPORT "ahuggel@gmx.net"

/* Define to the name of this package. */
#define EXV_PACKAGE "exiv2"

/* Define to the full name of this package. */
#define EXV_PACKAGE_NAME "exiv2"

/* Define to the full name and version of this package. */
#define EXV_PACKAGE_STRING "exiv2 0.17.1"

/* Define to the one symbol short name of this package. */
#define EXV_PACKAGE_TARNAME "exiv2"

/* Define to the version of this package. */
#define EXV_PACKAGE_VERSION "0.17.1"

/* Define to 1 if you have the ANSI C header files. */
#define EXV_STDC_HEADERS 1

/* Define to 1 if strerror_r returns char *. */
/* #undef EXV_STRERROR_R_CHAR_P */

/* Define to 1 if you can safely include both <sys/time.h> and <time.h>. */
/* #undef EXV_TIME_WITH_SYS_TIME */

/* Define to 1 if your <sys/time.h> declares `struct tm'. */
/* #undef EXV_TM_IN_SYS_TIME */

/* Define to empty if `const' does not conform to ANSI C. */
/* #undef const */

/* Define to `__inline__' or `__inline' if that's what the C compiler
   calls it, or to nothing if 'inline' is not supported under any name.  */
#ifndef __cplusplus
/* #undef inline */
#endif

/* Define to rpl_malloc if the replacement function should be used. */
/* #undef malloc */

/* Define to `int' if <sys/types.h> does not define. */
/* #undef pid_t */

/* Define to rpl_realloc if the replacement function should be used. */
/* #undef realloc */

/* Define to `unsigned' if <sys/types.h> does not define. */
/* #undef size_t */

#if defined __CYGWIN32__ && !defined __CYGWIN__
   /* For backwards compatibility with Cygwin b19 and
      earlier, we define __CYGWIN__ here, so that
      we can rely on checking just for that macro. */
#define __CYGWIN__  __CYGWIN32__
#endif
     
/* File path seperator */
#if defined WIN32 && !defined __CYGWIN__
#define EXV_SEPERATOR_STR "\\"
#define EXV_SEPERATOR_CHR '\\'
#else
#define EXV_SEPERATOR_STR "/"
#define EXV_SEPERATOR_CHR '/'
#endif
