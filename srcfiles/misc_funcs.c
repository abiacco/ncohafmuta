#if defined(HAVE_CONFIG_H)
#include "../hdrfiles/config.h"
#endif

#include "../hdrfiles/includes.h"

/*--------------------------------------------------------*/
/* Talker-related include files                           */ 
/*--------------------------------------------------------*/
#include "../hdrfiles/osdefs.h"
/*
#include "../hdrfiles/authuser.h"
#include "../hdrfiles/text.h"
*/
#include "../hdrfiles/constants.h"
#include "../hdrfiles/protos.h"

extern int num_of_users;     /* total number of users online           */
extern char thishost[101];
extern char mess[ARR_SIZE+25];    /* functions use mess to send output   */
extern char t_mess[ARR_SIZE+25];    /* functions use mess to send output   */


/*-------------------------------------------------------------------------*/
/* locate a free position in the array to place an incoming user.  check   */
/* both the wizard and user allocations to place the user.                 */
/*-------------------------------------------------------------------------*/
int find_free_slot(char port)
{
int u;

/*-------------------------------------------------*/
/* check for full system                           */
/*-------------------------------------------------*/

if ( (port == '1' && num_of_users >= NUM_USERS) || 
     (port == '2' && num_of_users >= MAX_USERS) )
  {
   return -1;
  }

/*-------------------------------------------------*/
/* find a free slot                                */
/*-------------------------------------------------*/

if (port=='3') {
   for (u=0;u<MAX_WHO_CONNECTS;++u)
     {
      if (strlen(whoport[u].site) < 2)
        return u;
     }
   return -1;
  }
else if (port=='4') {
   for (u=0;u<MAX_WWW_CONNECTS;++u)
     {
      if (strlen(wwwport[u].site) < 2)
        return u;
     }
   return -1;
  }
else if (port=='5') {
   for (u=0;u<MAX_MISC_CONNECTS;++u)
     {
      if (miscconn[u].sock == -1)
        return u;
     }
   return -1;
  }
else {
   for (u=0;u<MAX_USERS;++u) 
     {
      if (ustr[u].area== -1 && !ustr[u].logging_in) {
	/* CYGNUS2 */
	if (ustr[u].conv == NULL) {
		ustr[u].conv = (ConvPtr) malloc (sizeof (ConvBuffer));
		if (ustr[u].conv == NULL)       /* malloc failed */
		{
		write_log(ERRLOG,YESTIME,"MALLOC: Failed for conv buffer in find_free_slot() %s\n",get_error());
		return -1;
		}
		init_conv_buffer(ustr[u].conv);
	} /* end of if conv null */
	/* CYGNUS3 */
	if (ustr[u].Macros == NULL) {
		ustr[u].Macros = (MacroPtr) malloc (sizeof (MacroBuffer));
		if (ustr[u].Macros == NULL)       /* malloc failed */
		{
		write_log(ERRLOG,YESTIME,"MALLOC: Failed for Macros in find_free_slot() %s\n",get_error());
		return -1;
		}
		init_macro_buffer(ustr[u].Macros);
	} /* end of if Macros null */
        return u;
       } /* end of if free slot */
     } /* end of for */
  return -1;
  } /* end of else */

}


int in_connlist(int user) {
int z=-1;

for (z=0;z<MAX_CONNLIST_ENTRIES;++z) {
 if (!strcmp(connlist[z].site,ustr[user].site)) {
	write_log(DEBUGLOG,YESTIME,"in_connlist: found in connlist pos %d\n",z);
	return z;
 }
} /* for */

return -1;
}


int find_free_connslot(void) {
int z=0;
int lowest=0,lowestpos=0;

for (z=0;z<MAX_CONNLIST_ENTRIES;++z) {
 if (connlist[z].connections == 0) return z;
} /* for */

write_log(DEBUGLOG,YESTIME,"find_free_connslot: all slots full\n");

/* all slots used, bump lowest one */
lowest=connlist[0].connections; /* we need to start somewhere */
lowestpos=0;

for (z=1;z<MAX_CONNLIST_ENTRIES;++z) {
 if (connlist[z].connections <= lowest) {
  lowest=connlist[z].connections;
  lowestpos=z;
 } /* if */
} /* for */

return lowestpos;
}


void realloc_str(char** strP, int size) {
int maxsizeP;

        maxsizeP = size * 5 / 4;
        *strP = RENEW( *strP, char, maxsizeP + 1 );

    if ( *strP == (char*) 0 )
        {
        write_log(ERRLOG,YESTIME,"QW : ERRNO: REALLOC: Out of memory!\n");
        }
}


/* If the errno is one within the cases below, the calling function is */
/* told it is ok to continue (i.e. a non-fatal error) */
int errno_ok(int myerr) {

switch(myerr) {
#ifdef EPROTO
                case EPROTO:
#endif
#if defined(EINTR) && ((EPROTO) != (EINTR))
                case EINTR:
#endif
#ifdef EAGAIN
                case EAGAIN:
#endif
#ifdef EPIPE
                case EPIPE:
#endif
#ifdef ECONNABORTED
                case ECONNABORTED:
#endif
                    /* Linux generates the rest of these, other tcp
                     * stacks (i.e. bsd) tend to hide them behind
                     * getsockopt() interfaces.  They occur when
                     * the net goes sour or the client disconnects
                     * after the three-way handshake has been done
                     * in the kernel but before userland has picked
                     * up the socket.
                     */
#ifdef ECONNRESET
                case ECONNRESET:
#endif
#ifdef ETIMEDOUT
                case ETIMEDOUT:
#endif
#ifdef EHOSTUNREACH
                case EHOSTUNREACH:
#endif
#ifdef ENETUNREACH
                case ENETUNREACH:
#endif
                    break;
		default: return 0;
}

return 1;
}


void check_connlist_entries(int mode) {
int z=0;

for (z=0;z<MAX_CONNLIST_ENTRIES;++z) {
 if (mode==-1) {
  /* boot init */
  goto DOCLEAR;
 }
 else if (mode==-2) {
  /* per-minute clear */
/*
  write_log(DEBUGLOG,YESTIME,"check_connlist_entries: per-minute: checking connlist pos %d\n",z);
*/
  if ((time(0) - connlist[z].starttime) >= 60) goto DOCLEAR;
  else continue;
 } /* else if */
 else {
  /* specific clear */
  z=mode;
  goto DOCLEAR;
 } /* else */

DOCLEAR:
/*
 write_log(DEBUGLOG,YESTIME,"check_connlist_entries: expiring connlist pos %d\n",z);
*/
 connlist[z].site[0]=0;
 connlist[z].connections=0;
 connlist[z].starttime=0;
 if (mode >= 0) break;
} /* for */

}


