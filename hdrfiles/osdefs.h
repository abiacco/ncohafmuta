#ifndef _OSDEFS_H
#define _OSDEFS_H

#if defined(__sun__)
 /* SunOS of some sort */
  #if defined(__svr4__) || defined(__SVR4)
  /* Solaris, i.e. SunOS 5.x */
   #define SOL_SYS
 #else
  /* __sun__ && !__svr4__ (SunOS 4.x presumeably) */
  #define SUN4_SYS
 #endif /* __svr4__ */
#endif /* __sun__ */

#if defined(__MWERKS__) && defined(macintosh)
# if macintosh==1
#  if !defined(MAC_OS_GUSI_SOURCE)
#    define MAC_OS_pre_X
#    define NO_SYS_TYPES_H
     typedef long ssize_t;
#  endif
#  define NO_SYS_PARAM_H
# endif
#endif

#if defined(_WIN32) && !defined(WIN32) && !defined(__CYGWIN32__)
#  define WIN32
#endif

#if (defined(WIN32) || defined(WIN16)) && !defined(__CYGWIN32__)
#  if !defined(WINDOWS)
#    define WINDOWS
#  endif
#  if !defined(MSDOS)
#    define MSDOS
#  endif
#endif

#if defined(WIN32)
#define get_last_sys_error()    GetLastError()
#define clear_sys_error()       SetLastError(0)
#else
#define get_last_sys_error()    errno
#define clear_sys_error()       errno=0
#endif

/* This tried to make common macros for these socket functions */
/* to be portable to win32 systems */

#if defined(WINDOWS)
HANDLE hThread;
#define get_last_socket_error() WSAGetLastError()
#define clear_socket_error()    WSASetLastError(0)
#define CLOSE(s)                closesocket(s)
#define S_READ(s,b,n)           recv((s),(b),(n),0)
#define S_READ(s,b,n)           send((s),(b),(n),0)
#define EADDRINUSE              WSAEADDRINUSE

/* dunno about these..MIGHT be not needed */
#define EINTR WSAEINTR
#define EMFILE WSAEMFILE
#define EWOULDBLOCK WSAEWOULDBLOCK
#define EINPROGRESS WSAEINPROGRESS
#define EAFNOSUPPORT WSAEAFNOSUPPORT
#define ENETUNREACH WSAENETUNREACH
#define ETIMEDOUT WSAETIMEDOUT
#define ECONNREFUSED WSAECONNREFUSED
#define EIO WSAEIO /* EIO is used in ident code, but what */
                   /* function is it an errno for         */
#define errno WSAGetLastError()

#elif defined(MAC_OS_pre_X)
#define get_last_socket_error() errno
#define clear_socket_error()    errno=0
#define CLOSE(s)                MacSocket_close(s)
#define S_READ(s,b,n)           MacSocket_recv((s),(b),(n),true)
#define S_WRITE(s,b,n)          MacSocket_send((s),(b),(n))
#else
#define get_last_socket_error() errno
#define clear_socket_error()    errno=0
#define CLOSE(s)                close(s)
#define S_READ(s,b,n)           read((s),(b),(n))
#define S_WRITE(s,b,n)          write((s),(b),(n))
#endif

#if defined(HAVE_FCNTL_H) || defined(HAVE_SYS_FCNTL_H)
#define MY_F_SETFL F_SETFL
#define MY_F_GETFD F_GETFD
/* */
#elif defined(HAVE_SYS_IOCTL_H)
# if !defined FIONBIO
#  define FIONBIO 0x5421
# endif
# if !defined FIOCLEX
#  define FIOCLEX 0x5451
# endif

#define MY_F_SETFL FIONBIO
#define MY_F_GETFD FIOCLEX

# define NBLOCK_CMD 1
#else
/* windows */
# if !defined FIONBIO
#  define FIONBIO 0x5421
# endif
# if !defined FIOCLEX
#  define FIOCLEX 0x5451
# endif

#define MY_F_SETFL FIONBIO
#define MY_F_GETFD FIOCLEX

#define NBLOCK_CMD 1
#endif

#if defined(WINDOWS) || defined(MSDOS)
#  if defined(WINDOWS)
#    include <windows.h>
#    include <winsock.h>
#    include <stddef.h>
#    include <errno.h>
#    include <string.h>
#    include <malloc.h>
#  endif
#  include <io.h>
#  include <fcntl.h>

#  define ssize_t long

#  if defined (__BORLANDC__)
#    define _setmode setmode
#    define _O_TEXT O_TEXT
#    define _O_BINARY O_BINARY
#    define _int64 __int64
#    define _kbhit kbhit
#  endif
#elif defined(MAC_OS_pre_X)
#    include "MacSocket.h"
#endif

/* If not defined, i.e. unix system, define it so we cut */
/* down on ifdefs between win32 and *nix                 */
#if !defined(SOCKET_ERROR)
#define SOCKET_ERROR -1
#endif

/* If not defined, i.e. unix system, define it so we cut */
/* down on ifdefs between win32 and *nix                 */
#if !defined(INVALID_SOCKET)
#define INVALID_SOCKET -1
#endif

/* If the OS doesn't define a max number of file      */
/* descriptors, define a normal number for select()   */
#if !defined(FD_SETSIZE)
#define FD_SETSIZE 256
#endif

/* BSD 4.2 and maybe some others need these defined */
#if !defined(FD_ZERO)
#define fd_set int
#define FD_ZERO(p)       (*p = 0)
#define FD_SET(n,p)      (*p |= (1<<(n)))
#define FD_CLR(n,p)      (*p &= ~(1<<(n)))
#define FD_ISSET(n,p)    (*p & (1<<(n)))
#endif

/* signal stuff */
#undef ARGS
#if defined(__STDC__) || defined(__cplusplus)
# define ARGS(p) p
#else
# define ARGS(p) ()
#endif
typedef RETSIGTYPE (*handler_t) ARGS((int));    /* signal handler */

/* If the system is missing a prototype for these functions,  */
/* i.e. (void (*)()), such as on OS X, redefine them          */ 
#if !defined(HAVE_SIG_IGN_PROTO)
#undef SIG_IGN
#undef SIG_ERR
#define SIG_IGN         (void (*)(int))1
#define SIG_ERR         (void (*)(int))-1
#endif

#if defined(__ultrix)
#  if !defined(ssize_t)
#    define ssize_t int
#  endif
#endif

#if defined(NEXT) || defined(NeXT)
#  define ap_wait_t       union wait
#  define waitpid(a,b,c) wait4((a) == -1 ? 0 : (a),(union wait *)(b),c,NULL)
#  define WEXITSTATUS(status)     (int)( WIFEXITED(status) ? ( (status).w_retcode ) : -1)
#  define WTERMSIG(status)        (int)( (status).w_termsig )
#  define pid_t int /* pid_t is missing on NEXTSTEP/OPENSTEP */
#  define ssize_t int /* ditto */

#elif defined(NEWSOS) || defined(NEWS4) || defined(NEWSOS4)
#  define pid_t int
#  define ssize_t int

#elif defined(__OSF__) || defined(__osf__)
/* WNOHANG causes the wait to not hang if there are no stopped */
/* or terminated                                               */
#define WNOHANG         0x1     /* dont hang in wait */

#endif

/* glibc changes the type for arg3 of accept() and other socket calls */
/* from int to socklen_t, we want to eliminate some warnings          */
/*
#if !defined(__GLIBC__) || (__GLIBC__ < 2)
#if !defined(OPENBSD_SYS) && !defined(FREEBSD_SYS) && !defined(__NetBSD__)
#define socklen_t       int
#endif
#endif
#if !defined(socklen_t)
#define socklen_t       int
#endif
*/

#if !defined(EPROTO)
#define EPROTO EINTR
#endif
#if !defined(SIGCLD)
#define SIGCLD SIGCHLD
#endif
#if !defined(MAXHOSTNAMELEN)
#define MAXHOSTNAMELEN 64   
#endif

#define TRUE            1
#define FALSE           0

#ifndef NULL
#define NULL            0
#endif

/* Define ECHO if it isn't defined */
#if !defined(ECHO)
#define ECHO 0x00000008
#endif

/* Figure out OS's socket non-blocking option */
/* first if because might be set above for ioctl or ioctlsocket */
#if !defined(NBLOCK_CMD)
#if defined(FNONBLOCK)                     /* SYSV,AIX,SOLARIS,IRIX,HP-UX */
# define NBLOCK_CMD FNONBLOCK
#else
# if defined(O_NDELAY)                     /* BSD,LINUX,SOLARIS,IRIX */
#  define NBLOCK_CMD O_NDELAY
# else
#  if defined(FNDELAY)                     /* BSD,LINUX,SOLARIS,IRIX */
#   define NBLOCK_CMD FNDELAY
#  else
#   if defined(FNBIO)                      /* SYSV */
#    define NBLOCK_CMD FNBIO
#   else
#    if defined(FNONBIO)                   /* ? */
#     define NBLOCK_CMD FNONBIO
#    else
#     if defined(FNONBLK)                  /* IRIX */
#      define NBLOCK_CMD FNONBLK
#     else
#      define NBLOCK_CMD 0
#     endif
#    endif
#   endif
#  endif
# endif
#endif   
#endif   

#if !defined(ABORT)
#define ABORT 238
#endif
#if !defined(SUSP)
#define SUSP 237
#endif
#if !defined(xEOF)
#define xEOF 236
#endif

#if !defined(TELOPT_TUID)
#define TELOPT_TUID 26
#endif
#if !defined(TELOPT_OUTMRK)
#define TELOPT_OUTMRK 27
#endif
#if !defined(TELOPT_TTYLOC)
#define TELOPT_TTYLOC 28
#endif
#if !defined(TELOPT_3270REGIME)
#define TELOPT_3270REGIME 29
#endif
#if !defined(TELOPT_X3PAD)
#define TELOPT_X3PAD 30
#endif
#if !defined(TELOPT_NAWS)
#define TELOPT_NAWS 31
#endif
#if !defined(TELOPT_TSPEED)
#define TELOPT_TSPEED 32
#endif
#if !defined(TELOPT_LFLOW)
#define TELOPT_LFLOW 33
#endif
#if !defined(TELOPT_LINEMODE)
#define TELOPT_LINEMODE 34
#endif
#if !defined(TELOPT_XDISPLOC)
#define TELOPT_XDISPLOC 35
#endif
#if !defined(TELOPT_OLD_ENVIRON)
#define TELOPT_OLD_ENVIRON 36
#endif
#if !defined(TELOPT_AUTHENTICATION)
#define TELOPT_AUTHENTICATION 37
#endif
#if !defined(TELOPT_ENCRYPT)
#define TELOPT_ENCRYPT 38
#endif
#if !defined(TELOPT_NEW_ENVIRON)
#define TELOPT_NEW_ENVIRON 39
#endif

#if !defined(HAVE_STRFTIME)
# if defined(_DEFINING_OSDEFS)
 char clocker_daylist[7][4] = {"Sun","Mon","Tue","Wed","Thu","Fri","Sat"};
 char clocker_monthlist[12][4] = {"Jan","Feb","Mar","Apr","May","Jun","Jul","Aug","Sep","Oct","Nov","Dec"};
# else
 extern char clocker_daylist[7][4];
 extern char clocker_monthlist[12][4];
# endif
#endif

#endif /* _OSDEFS_H */

