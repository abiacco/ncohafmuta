#ifndef _SNPRINTF_H
#define _SNPRINTF_H

#ifdef HAVE_SNPRINTF
#ifdef LIBXT
#define _XtSnprintf snprintf
#define _XtVsnprintf vsnprintf
#endif
#ifdef LIBX11
#define _XSnprintf snprintf
#define _XVsnprintf vsnprintf
#endif
#else /* !HAVE_SNPRINTF */

#ifdef LIBXT
#define snprintf _XtSnprintf
#define vsnprintf _XtVsnprintf
#endif
#ifdef LIBX11
#define snprintf _XSnprintf
#define vsnprintf _XVsnprintf
#endif

/* changed from char to const char to satisfy vsnprintf const char cast */
static void dopr(char *buffer, const char *format, va_list args);
static char *end;

/*
#if 1
#include <X11/Xos.h>
#include <X11/Xlib.h>
#else
*/
#include <sys/types.h>
/* adjust the following defines if necessary (pre-ANSI) */
#define NeedFunctionPrototypes 1
#define NeedVarargsPrototypes 1
/* #endif */

#if NeedVarargsPrototypes
#if !defined(HAVE_STDARG_H)
#define HAVE_STDARG_H
#endif
#endif

#ifdef HAVE_STDARG_H
int snprintf (char *str, size_t count, const char *fmt, ...);
int vsnprintf (char *str, size_t count, const char *fmt, va_list arg);
#else
int snprintf ();
int vsnprintf ();
#endif

#endif /* HAVE_SNPRINTF */

#endif /* _SNPRINTF_H */
