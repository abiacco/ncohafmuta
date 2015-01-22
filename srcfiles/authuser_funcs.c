#if defined(HAVE_CONFIG_H)
#include "../hdrfiles/config.h"
#endif

#include "../hdrfiles/includes.h"

/*--------------------------------------------------------*/
/* Talker-related include files                           */ 
/*--------------------------------------------------------*/
#include "../hdrfiles/osdefs.h"
/*
#include "../hdrfiles/text.h"
*/
#include "../hdrfiles/authuser.h"
#include "../hdrfiles/constants.h"
#include "../hdrfiles/protos.h"


/*
 ** authuser_funcs.c
 **
 ** 10/25/01: Fixed code to adhere to the set response timeout, as it didn't
 **           seem to before. Added some wrapper functions for compatibily 
 **           under windows systems. General cleanup.
 ** 2/2/93: Added auth_setreadtimeout() call.
 ** 7/21/92: Fixed SIGPIPE bug in auth_tcpuser3(). <pen@lysator.liu.se>
 ** 2/9/92: authuser 4.0. Public domain.
 ** 2/9/92: added bunches of zeroing just in case.
 ** 2/9/92: added auth_tcpuser3. uses bsd 4.3 select interface.
 ** 2/9/92: added auth_tcpsock, auth_sockuser.
 ** 2/9/92: added auth_fd2, auth_tcpuser2, simplified some of the code.
 ** 12/27/91: fixed up usercmp to deal with restricted tolower XXX
 ** 5/6/91 DJB baseline authuser 3.1. Public domain.
 */

/* CLORETS returns the errno we encountered. We first save the	*/
/* error in case CLOSE() returns an error, we wont lose it	*/
#define CLORETS(e) { saveerrno = errno; SHUTDOWN(s, 2); CLOSE(s); errno = saveerrno; return e; }

/* Definitions for authuser info from a remote identd */
unsigned short auth_tcpport = 113;
int            auth_rtimeout = 3; /* time to read an ident */
int            auth_wtimeout = 6; /* time to connect/write ident query */


static void clearsa(struct sockaddr_in *sa)
{
    register char *x;
    for (x = (char *) sa;x < sizeof(*sa) + (char *) sa;++x)
	*x = 0;
}

/* Not currently in use, but called by auth_xline	*/
static int usercmp(register char *u,register char *v)
{
    register char uc;
    register char vc='\0';
    register char ucvc;
    /* is it correct to consider Foo and fOo the same user? yes */
    /* but the function of this routine may change later */
    while ((uc = *u) && (vc = *v))
    {
	ucvc = (isupper((int)uc) ? tolower((int)uc) : uc) - (isupper((int)vc) ? tolower((int)vc) : vc);
	if (ucvc)
	    return ucvc;
	else
	    ++u,++v;
    }
    return uc || vc;
}

static char authline[SIZ];

/* I have no idea what this does. Ask Arctic	*/
char *auth_xline(register char *user, register int fd, register unsigned long *in)
{
    unsigned short local;
    unsigned short remote;
    register char *ruser;
    
    if (auth_fd(fd,in,&local,&remote) == -1)
	return 0;
    ruser = auth_tcpuser(*in,local,remote);
    if (!ruser)
	return 0;
    if (!user)
	user = ruser; /* forces X-Auth-User */
    sprintf(authline,
	    (usercmp(ruser,user) ? "X-Forgery-By: %s" : "X-Auth-User: %s"),
	    ruser);
    return authline;
}

/* We pass the target user's socket here.			  */
/* We estable ports and addresses on both sides first in auth_fd2 */
int auth_fd(register int fd, register unsigned long *in, register unsigned short *local, register unsigned short *remote)
{
    unsigned long inlocal;
    return auth_fd2(fd,&inlocal,in,local,remote);
}

int auth_fd2(register int fd, register unsigned long *inlocal, register unsigned long *inremote, register unsigned short *local, register unsigned short *remote)
{
    struct sockaddr_in sa;
    NET_SIZE_T dummy;
    
    dummy = sizeof(sa);
    if (getsockname(fd,(struct sockaddr *)&sa,&dummy) == -1) {
	write_log(ERRLOG,YESTIME,"IDENT: Socket getsockname failed! %s\n",get_error());
	return -1;
    }
    if (sa.sin_family != AF_INET)
    {
	errno = EAFNOSUPPORT;
	write_log(ERRLOG,YESTIME,"IDENT: Socket family not supported! %s\n",get_error());
	return -1;
    }
    *local = ntohs(sa.sin_port);
    *inlocal = sa.sin_addr.s_addr;
    dummy = sizeof(sa);
    if (getpeername(fd,(struct sockaddr *)&sa,&dummy) == -1) {
	write_log(ERRLOG,YESTIME,"IDENT: Socket getpeername failed! %s\n",get_error());
	return -1;
    }
    *remote = ntohs(sa.sin_port);
    *inremote = sa.sin_addr.s_addr;
    return 0;
}

static char ruser[SIZ];
static char realbuf[SIZ];
static char *buf;

/* Here we start calling functions for the remote connection	   */
/* This is a wrapper to the following function ONLY FOR auth_xline */
char *auth_tcpuser(register unsigned in, register unsigned short local, register unsigned short remote)
{
    return auth_tcpuser2(0,in,local,remote);
}

/* Here we call auth_tcpsock to start the connection	*/
/* to the target user's host and ident port.		*/
/* Then we call auth_sockuser to start select()ing	*/
/* the connection. We select() so that we can check	*/
/* that the remote sock is writable			*/
/* Then we select() the sock for readability, if this	*/
/* suceeds. We timeout on readability after x seconds	*/
/* so as to not hang ourselves on a slow connection!	*/
char *auth_tcpuser2(register unsigned long inlocal, register unsigned long inremote, register unsigned short local, register unsigned short remote)
{
    register int s;
    
    s = auth_tcpsock(inlocal,inremote);
    if (s == -1)
	return 0;
    return auth_sockuser(s,local,remote);
}

/* Make the remote connection. Return an error unless in progress */
int auth_tcpsock(register unsigned long inlocal, register unsigned long inremote)
{
    struct sockaddr_in sa;
    register int s;
/*    register int fl; */
    register int saveerrno;
#if defined(WINDOWS)
    unsigned long arg = 1;
#endif
    
    if ((s = socket(AF_INET,SOCK_STREAM,0)) == INVALID_SOCKET) {
	write_log(ERRLOG,YESTIME,"IDENT: Socket creation failed! %s\n",get_error());
	return -1;
    }
    if (inlocal)
    {
	clearsa(&sa);
#if defined(FREEBSD_SYS)
	sa.sin_len = sizeof(sa);
#endif
	sa.sin_family = AF_INET;
	sa.sin_port = (unsigned short)0;
	sa.sin_addr.s_addr = inlocal;

/*
#if defined(WINDOWS)
	      if (ioctlsocket(s, FIONBIO, &arg) == -1) {
#else
              if (fcntl(s,F_SETFL,NBLOCK_CMD)== -1) {
#endif
*/
              if (MY_FCNTL(s,MY_F_SETFL,NBLOCK_CMD)==SOCKET_ERROR) {
		write_log(ERRLOG,YESTIME,"BLOCK: IDENT error setting binding socket to non-blocking %s\n",get_error());
		CLORETS(-1);
              }

#if defined(WINDOWS)
	if (bind(s,(struct sockaddr *)&sa,sizeof(sa)) != 0) {
#else
	if (bind(s,(struct sockaddr *)&sa,sizeof(sa)) == -1) {
#endif
		write_log(ERRLOG,YESTIME,"IDENT: Socket bind failed! %s\n",get_error());
		CLORETS(-1);
	    }

/*
    if ((fl = fcntl(s,F_GETFL,0)) == -1)
	CLORETS(-1);
    if (fcntl(s,F_SETFL,NBLOCK_CMD | fl) == -1)
	CLORETS(-1);
*/
    clearsa(&sa);
    sa.sin_family = AF_INET;
    sa.sin_port = htons((unsigned short)auth_tcpport);
    sa.sin_addr.s_addr = inremote;
    if (connect(s,(struct sockaddr *)&sa,sizeof(sa)) == SOCKET_ERROR) {
	if (errno != EINPROGRESS) {
		write_log(ERRLOG,YESTIME,"IDENT: Connection failed to remote host! %s\n",get_error());
		CLORETS(-1);
	} /* end of errno if */

	} /* end of if */

    } /* end of in local */

	/* We're connected or in progress */
	return s;
}

/* Here we select()ing the remote sock for writability so we	*/
/* can send our IDENT query.					*/
/* This is a wrapper to the following function			*/
char *auth_sockuser(register int s, register unsigned short local, register unsigned short remote)
{
    return auth_sockuser2(s, local, remote, auth_rtimeout, auth_wtimeout);
}

/* Check the remote sock for writeability. If so, write our	*/
/* query to it, looping the write.				*/
/* Then let's check for a response. If we get one, parse it	*/
/* out and return the idented username to real_user()		*/
char *auth_sockuser2(register int s, register unsigned short local, register unsigned short remote, int rtimeout, int wtimeout)
{
    register int buflen;
    register int w;
    register int saveerrno;
    register int retval; /* CYGNUS */
    char ch;
/*
    unsigned short rlocal;
    unsigned short rremote;
*/
    short rlocal;
    short rremote;
/*    register int fl; */
    fd_set rd_fds;
    fd_set wfds;
    struct timeval rtv;
#if defined(WINDOWS)
    unsigned long arg = 1;
#endif
    
    /* old_sig = signal(SIGPIPE, (handler_t)SIG_IGN); */
	rtv.tv_sec = wtimeout;
	rtv.tv_usec = 0;
    
    FD_ZERO(&wfds);
    FD_SET(s,&wfds);

retval=select(s+1,(void *)0,(void *)&wfds,(void *)0,&rtv);
if (retval == SOCKET_ERROR) {
	write_log(ERRLOG,YESTIME,"IDENT: Select failed to write/connect to target host! %s\n", get_error());
	FD_CLR(s,&wfds);
	CLORETS(0);
	}

    if (!FD_ISSET(s,&wfds))
    {
	errno = ETIMEDOUT;
	write_log(ERRLOG,YESTIME,"IDENT: Select timed out connecting to target host! %s\n", get_error());
	FD_CLR(s,&wfds);
	CLORETS(0);
    }

    /* now s is writable */
/*
    if ((fl = fcntl(s,F_GETFL,0)) == -1)
    {
	FD_CLR(s,&wfds);
	CLORETS(0);
    }
    if (fcntl(s,F_SETFL,~NBLOCK_CMD & fl) == -1)
    {
	CLORETS(0);
    }
*/
/*
#if defined(WINDOWS)
	      if (ioctlsocket(s, FIONBIO, &arg) == -1) {
#else
              if (fcntl(s,F_SETFL,NBLOCK_CMD)== -1) {
#endif
*/
              if (MY_FCNTL(s,MY_F_SETFL,NBLOCK_CMD)==SOCKET_ERROR) {
		write_log(ERRLOG,YESTIME,"BLOCK: IDENT error setting ident socket to non-blocking %s\n",get_error());
		CLORETS(0);
              }

    buf = realbuf;
    sprintf(buf,"%u , %u\r\n",(unsigned int) remote,(unsigned int) local);
    /* note the reversed order---the example in RFC 931 is misleading */
    buflen = strlen(buf);
    while ((w = S_WRITE(s,buf,buflen)) < buflen)
	if (w == -1) /* should we worry about 0 as well? */
	{
	    write_log(ERRLOG,YESTIME,"IDENT: Connection failed/ERROR writing to ident socket! %s\n",get_error());
	    FD_CLR(s,&wfds);
	    CLORETS(0);
	}
	else
	{
	    buf += w;
	    buflen -= w;
	}
    buf = realbuf;
    
    do
    {
	rtv.tv_sec = rtimeout;
	rtv.tv_usec = 0;
    
	FD_ZERO(&rd_fds);
	FD_SET(s, &rd_fds);

retval=select(s+1,(void *)&rd_fds,(void *)0,(void *)0,&rtv);
if ((retval == SOCKET_ERROR) || (!retval)) {
	    w = -1;
	    write_log(ERRLOG,YESTIME,"IDENT: Select timed out reading from target host! %s\n", get_error());
	    goto END;
	}
	
	if ((w = S_READ(s,&ch,1)) == 1)
	{
	    *buf = ch;
	    if ((ch != ' ') && (ch != '\t') && (ch != '\r'))
		++buf;
	    if ((buf - realbuf == sizeof(realbuf) - 1) || (ch == '\n'))
		break;
	}
    } while (w == 1);
    
    END: 
    if (w == -1) {
	CLORETS(0);
    }

	    *buf = 0;
    
    if (sscanf(realbuf, "%hd,%hd: USERID :%*[^:]:%s",
	       &rremote, &rlocal, ruser) < 3)
    {
	/* makes sense, right? well, not when USERID failed to match ERROR */
	/* but there's no good error to return in that case */
	errno = EIO;
	write_log(ERRLOG,YESTIME,"IDENT: sscanf didn't return enough data! %s\n", get_error());
	CLORETS(0);
    }
    if ((remote != rremote) || (local != rlocal))
    {
	errno = EIO;
	write_log(ERRLOG,YESTIME,"IDENT: Host info returned doesn't match! %s\n", get_error());
	CLORETS(0);
    }
    /* we're not going to do any backslash processing */
    SHUTDOWN(s, 2);
    CLOSE(s);
    FD_CLR(s,&wfds);
    write_log(SYSTEMLOG,YESTIME,"IDENT: Ident performed and returned \"%s\"\n", ruser);
    return ruser;
}

/* We dont actually use these next 2 functions */
char *auth_tcpuser3(register unsigned long inlocal, register unsigned long inremote, register unsigned short local, register unsigned short remote)
{
    return auth_tcpuser4(inlocal,
			 inremote,
			 local,
			 remote,
			 auth_wtimeout,
			 auth_rtimeout);
}


char *auth_tcpuser4(register unsigned long inlocal, register unsigned long inremote, register unsigned short local, register unsigned short remote, register int wtimeout, register int rtimeout)
{
    register int s;
    struct timeval ctv;
    fd_set wfds;
    register int r;
    register int saveerrno;
    char *retval;
    
    s = auth_tcpsock(inlocal,inremote);
    if (s == -1)
    {
	return 0;
    }
    ctv.tv_sec = wtimeout;
    ctv.tv_usec = 0;
    FD_ZERO(&wfds);
    FD_SET(s,&wfds);
    r = select(s + 1,(void *) 0,(void *)&wfds,(void *) 0,&ctv);
    /* XXX: how to handle EINTR? */
    if (r == -1)
    {
	CLORETS(0);
    }
    if (!FD_ISSET(s,&wfds))
    {
	SHUTDOWN(s, 2);
	CLOSE(s);
	FD_CLR(s,&wfds);
	errno = ETIMEDOUT;
	return 0;
    }
    retval = auth_sockuser2(s,local,remote,rtimeout,wtimeout);
    
    return retval;
}


