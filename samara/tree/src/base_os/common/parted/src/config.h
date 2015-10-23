/* config.h.  Generated by configure.  */
/* config.h.in.  Generated from configure.ac by autoheader.  */

/* Enable assertions, etc. */
#define DEBUG 1

/* Probing functionality only */
/* #undef DISCOVER_ONLY */

/* Lazy linking to fs libs */
/* #undef DYNAMIC_LOADING */

/* Include file system support. i.e. libparted/fs_... */
#define ENABLE_FS 1

/* Mtrace malloc() debugging */
/* #undef ENABLE_MTRACE */

/* Define to 1 if translation of program messages to the user's native
   language is requested. */
/* #undef ENABLE_NLS */

/* Include PC98 partition tables. (Sometimes excluded to avoid collisions with
   msdos partition tables */
/* #undef ENABLE_PC98 */

/* Define to 1 if you have the `canonicalize_file_name' function. */
#define HAVE_CANONICALIZE_FILE_NAME 1

/* Define if the GNU dcgettext() function is already present or preinstalled.
   */
/* #undef HAVE_DCGETTEXT */

/* Define to 1 if you have the <dlfcn.h> header file. */
#define HAVE_DLFCN_H 1

/* Define to 1 if you have the <getopt.h> header file. */
#define HAVE_GETOPT_H 1

/* Define if the GNU gettext() function is already present or preinstalled. */
/* #undef HAVE_GETTEXT */

/* Define to 1 if you have the `getuid' function. */
#define HAVE_GETUID 1

/* Define if you have the iconv() function. */
/* #undef HAVE_ICONV */

/* Define to 1 if you have the <inttypes.h> header file. */
#define HAVE_INTTYPES_H 1

/* Define to 1 if you have the `parted' library (-lparted). */
/* #undef HAVE_LIBPARTED */

/* have readline */
/* #undef HAVE_LIBREADLINE */

/* Have libreiserfs */
/* #undef HAVE_LIBREISERFS */

/* Define to 1 if you have the <memory.h> header file. */
#define HAVE_MEMORY_H 1

/* Define to 1 if you have the <readline/history.h> header file. */
/* #undef HAVE_READLINE_HISTORY_H */

/* Define to 1 if you have the <readline/readline.h> header file. */
/* #undef HAVE_READLINE_READLINE_H */

/* Have reiserfs_fs_check() */
/* #undef HAVE_REISERFS_FS_CHECK */

/* Define to 1 if you have the `rl_completion_matches' function. */
/* #undef HAVE_RL_COMPLETION_MATCHES */

/* Define to 1 if you have the `sigaction' function. */
#define HAVE_SIGACTION 1

/* Define to 1 if you have the <stdint.h> header file. */
#define HAVE_STDINT_H 1

/* Define to 1 if you have the <stdlib.h> header file. */
#define HAVE_STDLIB_H 1

/* Define to 1 if you have the <strings.h> header file. */
#define HAVE_STRINGS_H 1

/* Define to 1 if you have the <string.h> header file. */
#define HAVE_STRING_H 1

/* Define to 1 if you have the <sys/stat.h> header file. */
#define HAVE_SYS_STAT_H 1

/* Define to 1 if you have the <sys/types.h> header file. */
#define HAVE_SYS_TYPES_H 1

/* Define to 1 if you have the <termcap.h> header file. */
#define HAVE_TERMCAP_H 1

/* Define to 1 if you have the <unistd.h> header file. */
#define HAVE_UNISTD_H 1

/* Define to 1 if you have the <wctype.h> header file. */
/* #undef HAVE_WCTYPE_H */

/* Extract low level special HFS(+) files for debugging purposes when using
   the "check" command (NOT FOR PACKAGING) */
/* #undef HFS_EXTRACT_FS */

/* Name of package */
#define PACKAGE "parted"

/* Define to the address where bug reports for this package should be sent. */
#define PACKAGE_BUGREPORT ""

/* Define to the full name of this package. */
#define PACKAGE_NAME ""

/* Define to the full name and version of this package. */
#define PACKAGE_STRING ""

/* Define to the one symbol short name of this package. */
#define PACKAGE_TARNAME ""

/* Define to the version of this package. */
#define PACKAGE_VERSION ""

/* Disable all writing code */
/* #undef READ_ONLY */

/* The size of a `off_t', as computed by sizeof. */
#define SIZEOF_OFF_T 8

/* Define to 1 if you have the ANSI C header files. */
#define STDC_HEADERS 1

/* Version number of package */
#define VERSION "1.7.1"

/* Define to 1 if your processor stores words with the most significant byte
   first (like Motorola and SPARC, unlike Intel and VAX). */
/* #undef WORDS_BIGENDIAN */

/* Number of bits in a file offset, on hosts where this is settable. */
/* #undef _FILE_OFFSET_BITS */

/* Define for large files, on AIX-style hosts. */
/* #undef _LARGE_FILES */

/* Define to empty if `const' does not conform to ANSI C. */
/* #undef const */

/* Define to `__inline__' or `__inline' if that's what the C compiler
   calls it, or to nothing if 'inline' is not supported under any name.  */
#ifndef __cplusplus
/* #undef inline */
#endif

/* Define to equivalent of C99 restrict keyword, or to nothing if this is not
   supported. Do not define if restrict is supported directly. */
#define restrict __restrict
