#ifndef _INCLUDES_H
#define _INCLUDES_H

/* ANSI C forbids braced groups in expressions..the system include */
/* files do this..bleh */
#if defined(OPENBSD_SYS) || defined(__CYGWIN32__)   
#undef __GNUC__   
#endif

#include <stdio.h>
#if defined(HAVE_STDLIB_H)
#include <stdlib.h>
#endif
#include <sys/types.h>     /* for bitypes */

#if defined(_WIN32) && !defined(__CYGWIN32__)
/* this probably isn't right..anyone? */
#include <winsock.h>
#include <winnt.h>
#include <process.h>
#include <io.h>
#include <sys/timeb.h>     /* for select() */
#else
/* NOT windows */
#include <sys/utsname.h>   /* for OS and machine info */
#if !defined(__STRICT_ANSI__)
#define __STRICT_ANSI__
#if defined(HAVE_SYS_SOCKET_H)
#include <sys/socket.h>    /* for socket(), bind(), etc.. */
#endif
#undef __STRICT_ANSI__
#else
#if defined(HAVE_SYS_SOCKET_H)
#include <sys/socket.h>    /* for socket(), bind(), etc.. */
#endif
#endif

#include <netinet/in.h>    /* for sockaddr_in structure   */

#if defined(HAVE_NETDB_H)
#include <netdb.h>         /* for gethostbyaddr() */
#elif defined(HAVE_ARPA_NETDB_H)
#include <arpa/netdb.h>
#else
#include "../hdrfiles/netdb.h"     /* for gethostbyaddr() */
#endif

/* for non-blocking */
#if defined(HAVE_FCNTL_H)
#include <fcntl.h>
#elif defined(HAVE_SYS_FCNTL_H)
#include <sys/fcntl.h>
#elif defined(HAVE_SYS_IOCTL_H)
#include <sys/ioctl.h>
#endif

#include <arpa/telnet.h>   /* for IAC negotitation and options */
#if defined(HAVE_ARPA_INET_H)
#include <arpa/inet.h>     /* for inet_addr() */
#endif

#if defined(TIME_WITH_SYS_TIME)
#include <sys/time.h>
#include <time.h>
#elif defined(HAVE_SYS_TIME_H)
#include <sys/time.h>
#else
#include <time.h>
#endif

#include <sys/file.h>      /* for flock() */
/* end of non-windows else */
#endif

#if defined(HAVE_SYS_WAIT_H)
#include <sys/wait.h>
#endif

/* ALL */
#if defined(OPENBSD_SYS)
#include <machine/endian.h>        /* for network conversion stuff */
#endif
#include <sys/stat.h>      /* for stat() file info */

#if defined(HAVE_DIRENT_H)
# include <dirent.h>        /* for various directory ops */
#else
# define dirent direct
# if defined(HAVE_SYSDIR_H)
#  include <sys/dir.h>
# elif defined(HAVE_SYSNDIR_H)
#  include <sys/ndir.h>
# elif defined(HAVE_NDIR_H)
#  include <ndir.h>
# endif
#endif
#include <signal.h>        /* for signal handling */
#include <errno.h>         /* for error codes and messages */
/* for close(),read(),write(),getcwd() */
/* dup2(),execvp(),getpid(), etc */
#if defined(HAVE_UNISTD_H)
#include <unistd.h>
#elif defined(HAVE_SYS_UNISTD_H)
#include <sys/unistd.h>
#endif

#if defined(HAVE_CTYPE_H)
/* for islower(),tolower() type things */
#include <ctype.h>
#endif

#if defined(HAVE_STRING_H)
#include <string.h>        /* for strcpy(),strcmp(),memcpy(), etc */
#endif

#if defined(HAVE_STRINGS_H)
#include <strings.h>
#endif

/* #if defined(AIX_SYS) */
#if defined(HAVE_SYS_SELECT_H)
/* for select() on AIX..not in time.h */
#include <sys/select.h>
#endif

#if defined(HAVE_STDARG_H)
/* for va_start(),va_arg(),va_end() */
#include <stdarg.h>
#else
#include <varargs.h>
#endif

#if !defined(HAVE_BCOPY) && defined(HAVE_MEMCPY) && !defined(bcopy)
#define bcopy(from, to, len) memcpy ((to), (from), (len))
#endif

#if !defined(HAVE_STRERROR)
/*
extern char *sys_errlist[];
#define strerror(i) sys_errlist[i]
extern int sys_nerr;
#define strerror(n) \
    (((n) >= 0 && (n) < sys_nerr) ? sys_errlist[n] : "unknown error")
*/
#endif

#if !defined(HAVE_STRCHR)
# define strchr                 index
# define strrchr                rindex
#endif

#if !defined(HAVE_ERRNO_DECL)
extern int errno;
#endif

#endif /* _INCLUDES_H */

