/* Some of the utility functions are straight from the Bind source */

#if defined(HAVE_CONFIG_H)
#include "../hdrfiles/config.h"
#endif

#include "../hdrfiles/includes.h"

#if defined(HAVE_NAMESER_H)
#include <nameser.h>
#elif defined(HAVE_ARPA_NAMESER_H)
#include <arpa/nameser.h>
#else
#include "../hdrfiles/nameser.h"
#endif

#if defined(HAVE_RESOLV_H)
#include <resolv.h>
#elif defined(HAVE_ARPA_RESOLV_H)
#include <arpa/resolv.h>
#else
#include "../hdrfiles/resolv.h"
#endif

/* LOGFILE should be the only thing you have to set, if deviating from */
/* the normal directory structure */
#define LOGFILE		"logfiles/resolve.log"

#define FCLOSE(file) if (file) fclose(file)

#define RESOLVELOG	1
#define YESTIME	1
#define NOTIME	0
#define FILE_NAME_LEN 256

#if !defined(NS_INT32SZ)
#define NS_INT32SZ      4       /* #/bytes of data in a u_int32_t */
#endif
#if !defined(MAXCDNAME)
#define MAXCDNAME	255
#endif
#if !defined(NS_MAXCDNAME)
#define NS_MAXCDNAME MAXCDNAME
#endif
#if !defined(NS_CMPRSFLGS)
#define NS_CMPRSFLGS    0xc0    /* Flag bits indicating name compression. */
#endif

#define _DEFINING_OSDEFS
#include "../hdrfiles/osdefs.h"
#include "../hdrfiles/resolver_clipon.h"
/* #define _DEFINING_CONSTANTS */
/* #include "../hdrfiles/constants.h" */

RETSIGTYPE handle_sig(int sig);
handler_t setsignal(int sig, handler_t handler);
/* RETSIGTYPE (*setsignal(int, RETSIGTYPE (*)(int)))(int); */

#if defined(OPENBSD_SYS)
/* our own ctime because the one in OpenBSD behaves badly */
char *ctime(const time_t *mytm);
#endif

int SHUTDOWN(int sock, int how);

#define MAXALIASES		35
#define MAXADDRS		35
#define MSGSIZ			512
#define NS_INT16SZ      2       /* #/bytes of data in a u_int16_t */

#define HAVE_GET16
#define HAVE_GET32
#define HAVE_PUT16
#define HAVE_PUT32

u_int my_ns_get16(u_char *src);
#if !defined(NS_GET16)
#define NS_GET16(s, cp) do { \
        register u_char *t_cp = (u_char *)(cp); \
        (s) = ((unsigned short)t_cp[0] << 8) \
            | ((unsigned short)t_cp[1]) \
            ; \
        (cp) += NS_INT16SZ; \
} while (0)
#undef HAVE_GET16
#endif

u_long my_ns_get32(u_char *src);
#if !defined(NS_GET32)
#define NS_GET32(l, cp) do { \
        register u_char *t_cp = (u_char *)(cp); \
        (l) = ((u_int32_t)t_cp[0] << 24) \
            | ((u_int32_t)t_cp[1] << 16) \
            | ((u_int32_t)t_cp[2] << 8) \
            | ((u_int32_t)t_cp[3]) \
            ; \
        (cp) += NS_INT32SZ; \
} while (0)
#undef HAVE_GET32
#endif

void my_putshort(unsigned short src, u_char *dst);
void my_ns_put16(u_int src, u_char *dst);
#if !defined(NS_PUT16)
#define NS_PUT16(s, cp) do { \
        register unsigned short t_s = (unsigned short)(s); \
        register u_char *t_cp = (u_char *)(cp); \
        *t_cp++ = t_s >> 8; \
        *t_cp   = t_s; \
        (cp) += NS_INT16SZ; \
} while (0)
#undef HAVE_PUT16
#endif

#if !defined(NS_PUT32)
#define NS_PUT32(l, cp) do { \
        register u_int32_t t_l = (u_int32_t)(l); \
        register u_char *t_cp = (u_char *)(cp); \
        *t_cp++ = t_l >> 24; \
        *t_cp++ = t_l >> 16; \
        *t_cp++ = t_l >> 8; \
        *t_cp   = t_l; \
        (cp) += NS_INT32SZ; \
} while (0)
void putlong(u_int32_t src, u_char *dst);
void ns_put32(u_long src, u_char *dst);
#undef HAVE_PUT32
#endif

#if !defined(BOUNDS_CHECK)
#define BOUNDS_CHECK(ptr, count) \
        do { \
                if ((ptr) + (count) > eom) { \
                        write_log(RESOLVELOG,NOTIME,"(form error.)\n"); \
                        return (NULL); \
                } \
        } while (0)
#endif

#define MATCH(line, name) \
        (!strncmp(line, name, sizeof(name) - 1) && \
        (line[sizeof(name) - 1] == ' ' || \
         line[sizeof(name) - 1] == '\t'))

void write_log(int type, int wanttime, char *str, ...);
void remove_first(char *inpstr);
void handle_sig(int sig);
void init_structs(int slot);
void process_result(int slot);
void my_print_query(u_char *msg, u_char *eom);
void midcpy(char *strf, char *strt, int fr, int to);
int write_stdout(char *msg);
int get_slot(int fd);
int find_free_ns_slot(void);
int find_user_slot(int user, char *site);
int read_result(int slot);
int start_query(int slot, char *ipaddy);
int restart_query(int slot);
char *Calloc(register int num, register int size);
char *Malloc(int size);
char *get_time(time_t ref,int mode);
char *get_error(void);
char *my_p_type(int mytype);
char *my_p_class(int myclass);
char *my_p_rcode(int myrcode);
char *get_ip(unsigned long addr);

const char *_res_opcodes[] = {
        "QUERY",
        "IQUERY",
        "CQUERYM",
        "CQUERYU",      /* experimental */
        "NOTIFY",       /* experimental */
        "UPDATE",
        "6",
        "7",
        "8",
        "9",
        "10",
        "11",
        "12",
        "13",
        "ZONEINIT",
        "ZONEREF",
};

#if !defined(HAVE_RES_NMKQUERY) && defined(CYGWIN_SYS)
 struct __res_state _res;
#else
#if !(defined(BSD) && (BSD >= 199103)) && !defined(OSF_SYS) && !defined(HPUX_SYS)
# if (!defined(__RES)) || (__RES < 19940415)
#  ifdef SCOR5
        extern struct __res_state       _res;
#  else
        extern struct state         _res;
#  endif
# endif
#endif
#endif
/*
struct __res_state _res;
*/
#if defined(WHICH_RES_STATE)
#define _my_res WHICH_RES_STATE()
#else
# if !defined(HAVE_RES_NMKQUERY)
#  define _my_res _res
# else
#  define _my_res &_res
# endif
#endif

int res_mkquery(int op,                     /* opcode of query */
            const char *dname,          /* domain name */
            int class, int type,        /* class and type of query */
            const u_char *data,         /* resource record data */
            int datalen,                /* length of data */
            const u_char *newrr_in,     /* new rr for modify or append */
            u_char *buf,                /* buffer to put query */
            int buflen);                 /* size of buffer */
#if !defined(HAVE_RES_NMKQUERY)
int res_nmkquery(struct __res_state statp,
             int op,                    /* opcode of query */
             const char *dname,         /* domain name */
             int class, int type,       /* class and type of query */
             const u_char *data,        /* resource record data */
             int datalen,               /* length of data */
             const u_char *newrr_in,    /* new rr for modify or append */
             u_char *buf,               /* buffer to put query */
             int buflen);               /* size of buffer */
#endif
static int dn_find(const u_char *domain, const u_char *msg,
        const u_char * const *dnptrs,
        const u_char * const *lastdnptr);
int dn_comp(const char *src, u_char *dst, int dstsiz,
 	u_char **dnptrs, u_char **lastdnptr);
int dn_expand(const u_char *msg, const u_char *eom, const u_char *src,
          char *dst, int dstsiz);
int dn_skipname(const u_char *ptr, const u_char *eom);
int ns_name_compress(const char *src, u_char *dst, size_t dstsiz,
                 const u_char **dnptrs, const u_char **lastdnptr);
int ns_name_uncompress(const u_char *msg, const u_char *eom, const u_char *src,
                   char *dst, size_t dstsiz);
int ns_name_skip(const u_char **ptrptr, const u_char *eom);
int ns_name_pton(const char *src, u_char *dst, size_t dstsiz);
int ns_name_ntop(const u_char *src, char *dst, size_t dstsiz);
int ns_name_pack(const u_char *src, u_char *dst, int dstsiz,
             const u_char **dnptrs, const u_char **lastdnptr);
int ns_name_unpack(const u_char *msg, const u_char *eom, const u_char *src,
               u_char *dst, size_t dstsiz);
u_char *Print_cdname(const u_char *cp, const u_char *msg, const u_char *eom);
u_char *Print_rr(const u_char *ocp, const u_char *msg, const u_char *eom);

static int special(int ch);
static int printable(int ch);
static int mklower(int ch);
void __h_errno_set(struct __res_state *res, int err);
#define RES_SET_H_ERRNO(r,x) __h_errno_set(r,x)

typedef int Boolean;
unsigned short nsport = NAMESERVER_PORT;

unsigned long naddy        = 0;
unsigned long second_naddy = 0;
unsigned long third_naddy  = 0;
unsigned long ftemp        = 0;
int number_of_servers      = 0;
static const char digits[] = "0123456789";
static u_char  hostbuf[MAXDNAME];

/*    u_char qb2[64*1024]; */
typedef union {
    HEADER qb1;
    u_char qb2[256];
} querybuf;

struct {
	int fd;
	int user;
	int status;
	int result;
	int buflen;
	int tries;
	unsigned long nameserver;
	char site[21];
	char hostname[64];
	char badlookup[64];
	querybuf buf;
	querybuf answer;
	time_t starttime;
} resolves[MAX_RESOLVES_IN_PROGRESS];

typedef union {
    int32_t al;
    char ac;
} align;

#define GetShort(cp)    my_ns_get16(cp); cp += INT16SZ;
#define GetLong(cp)     my_ns_get32(cp); cp += INT32SZ;

/*
 * Modified struct hostent from <netdb.h>
 *
 * "Structures returned by network data base library.  All addresses
 * are supplied in host order, and returned in network order (suitable
 * for use in system calls)."
 */

int             queryType = T_PTR;
int             queryClass = C_IN;
char		mess[256];
char nameserver[21];
char second_nameserver[21];
char third_nameserver[21];
char temp_nameserver[21];

int main(void) {
int ret=0, slot=-1, i=0, suspended=0, user;
static int nfds;
long len;
char action[5];
char site[21];
char buffer[BUFSIZ];
char s[BUFSIZ];
fd_set fds_read;
fd_set fds_write;
struct timeval timeout;  /* how much time to give select() */
FILE *fp;

/* 066 will make permissions -rw------- This is the default */
/* 006 will make permissions -rw-rw---- Use this is you need the group
   to have the same permissions as the user. i.e. on talker.com */
#if defined(HAVE_UMASK)
umask(066);
#endif

	/* tell talker we are alive! */
        write_stdout(RESOLVER_SERVER_CONNECT_MSG);

	/* set up signals */
        (void)setsignal(SIGTERM, handle_sig);
        (void)setsignal(SIGSEGV, handle_sig);
	(void)setsignal(SIGBUS, handle_sig);
	(void)setsignal(SIGILL, handle_sig);
	(void)setsignal(SIGABRT, handle_sig);
	(void)setsignal(SIGSTOP, handle_sig);
	(void)setsignal(SIGINT, handle_sig);
        (void)setsignal(SIGPIPE, handle_sig);
        (void)setsignal(SIGCHLD, handle_sig);

/* get name server addys */
if (USE_FALLBACK_AS_DEFAULT > 0) {
                strcpy(nameserver,FALLBACK_DNS_SERVER);
                if((naddy = inet_addr(nameserver)) == -1L) { nameserver[0]=0; }
		else number_of_servers++;
}

        if ((fp = fopen(_PATH_RESCONF, "r")) != NULL) {
            /* read the config file */
            while (fgets(buffer, sizeof(buffer), fp) != NULL) {
		buffer[strlen(buffer)-1]=0; /* strip newline */
                /* skip comments */
                if (*buffer == ';' || *buffer == '#')
                        continue;
                /* read default domain name */
                if (MATCH(buffer, "nameserver") && number_of_servers != 3) {
			for (i=10;i<strlen(buffer);++i) {
				if (isalnum((int)buffer[i])) break;
			}
			midcpy(buffer,temp_nameserver,i,i+20);

			  ftemp=0;
			  ftemp = inet_addr(temp_nameserver);
			  if (ftemp == -1L) {
			   ftemp=0;
			  }
			  else {
			   if (number_of_servers==0) {
			    naddy = ftemp;
			    strcpy(nameserver,temp_nameserver);
			    number_of_servers++;
			   } /* if 0 */
			   else if (number_of_servers==1) {
			    second_naddy = ftemp;
			    strcpy(second_nameserver,temp_nameserver);
			    number_of_servers++;
			   } /* if 1 */
			   else if (number_of_servers==2) {
			    third_naddy = ftemp;
			    strcpy(third_nameserver,temp_nameserver);
			    number_of_servers++;
			   } /* if 2 */
			  } /* else */
		} /* if match */
	} /* end of while */
	FCLOSE(fp);
	}
	else {
		strcpy(nameserver,FALLBACK_DNS_SERVER);
		if((naddy = inet_addr(nameserver)) == -1L) return 0;
		else number_of_servers=1;
	}

	/* set up structure */
        for (i = 0 ; i < MAX_RESOLVES_IN_PROGRESS ; i++) init_structs(i);

if (naddy)
 write_log(RESOLVELOG,YESTIME,"CLIPON: Ready to answer queries with nameserver %s of %d\n",nameserver,number_of_servers);
if (second_naddy)
 write_log(RESOLVELOG,YESTIME,"CLIPON: Ready to answer queries with nameserver2 %s of %d\n",second_nameserver,number_of_servers);
if (third_naddy)
 write_log(RESOLVELOG,YESTIME,"CLIPON: Ready to answer queries with nameserver3 %s of %d\n",third_nameserver,number_of_servers);

/* start forever loop */
while (1) {

		/* under normal circumstances, we would probably want	*/
		/* to check for this, to see if our parent abandoned us	*/
		/* but the talker does soft-boots which would make us	*/
		/* lost the parent pid here. We do checkpoints instead	*/
		/* to make sure we're not orphaned			*/

/* CODE KEPT HERE IN CASE NEEDED
		ret = getppid();
                if (ret==1)
                {
			write_log(RESOLVELOG,YESTIME,"CLIPON: init has us! exiting!\n");
                        exit(0);
                }
*/

		/* These are for connection being established */
                FD_ZERO(&fds_write);
		/* These are for a reply being ready */
                FD_ZERO(&fds_read);

		/* standard input */
                FD_SET(0, &fds_read);
		if (0 >= nfds) nfds = 1;
		/* standard output */
                /* FD_SET(1, &fds_write);
		if (1 >= nfds) nfds = 2; */
		/* pending resolves */
		for (i=0;i<MAX_RESOLVES_IN_PROGRESS;++i) {
		if (resolves[i].fd != -1) {
		sprintf(mess,"CLIPON: Setting active fd %d to FD_SET!\n",resolves[i].fd);
#if defined(RESOLVER_DEBUG)
		write_log(RESOLVELOG,YESTIME,mess);
#endif
			FD_SET(resolves[i].fd,&fds_read);
			if (resolves[i].fd >= nfds)
			nfds = resolves[i].fd + 1;
			}
		} /* end of for */

                timeout.tv_sec = RESOLVE_SELECT_TIMEOUT;
                timeout.tv_usec = 0;
		
		ret = select(nfds, (void *) &fds_read, (void *) &fds_write, (void *) 0, &timeout);

		if (ret == SOCKET_ERROR) {
		if (errno != EINTR) {
		write_log(RESOLVELOG,YESTIME,"CLIPON: SELECT error %s!\n",get_error());
		exit(0);
		}
		else continue;
		} /* end of if bad select */
		else if (!ret) {
		/* select timeout */
#if defined(RESOLVER_DEBUG)
		write_log(RESOLVELOG,YESTIME,"CLIPON: Select timed out\n");
#endif
		}

if (write_stdout(RESOLVER_SERVER_SEND_CHECKPOINT) <= 0) {
        write_log(RESOLVELOG,YESTIME,"CLIPON: Can't checkpoint with talker! %s\n",get_error());
        if (!suspended) {
        write_log(RESOLVELOG,YESTIME,"CLIPON: Exiting!\n");
        exit(1);
        }
}


/* out of select */
                  for (i = 0 ; i < MAX_RESOLVES_IN_PROGRESS ; i++)
                  {
			if (resolves[i].status == STA_PENDING &&
			    !FD_ISSET(resolves[i].fd, &fds_read) &&
			    ((time(0)-resolves[i].starttime) >= RESOLVE_SELECT_TIMEOUT)) {
			write_log(RESOLVELOG,YESTIME,"CLIPON: Timing out pending fd %d (%d seconds)\n",resolves[i].fd,(int)(time(0)-resolves[i].starttime));
			SHUTDOWN(resolves[i].fd, 2);
			CLOSE(resolves[i].fd);
			restart_query(i);
			}
		  } /* end of for */

                if (FD_ISSET(0,&fds_read)) {
		/* request coming in from talker */
#if defined(RESOLVER_DEBUG)
		write_log(RESOLVELOG,YESTIME,"CLIPON: Incoming request\n");
#endif
		len = S_READ(0, s, 1000);
		  /* Check for error */
		  if (len <= 0) {
			sprintf(mess,"CLIPON: Bad read len %ld error %s\n",len,get_error());
			write_log(RESOLVELOG,YESTIME,mess);
			/* Should we really exit here?  The theorey is, that if  */
			/* the talker is gonna reboot, suspends us, and then     */
			/* fails to reboot, we might get stuck, checkpoints      */
			/* failing to no end and stdin being readble but no data */
			/* sooner or later we'll hog cpu and fill up logspace    */
			exit(1);
			/* continue; */
		  }
		s[len]=0;
		if (!strlen(s)) continue;
		  if ((slot = find_free_ns_slot()) == -1) continue;
			/* parse request */
		/* s[strlen(s)-1]=0; */
                sscanf(s,"%s ",action);
                remove_first(s);
                if (!strcmp(action,RESOLVER_CLIENT_SEND_SUSPEND)) {
                suspended=1;
                write_log(RESOLVELOG,YESTIME,"Got suspend message\n");
                continue;  
                }
                else if (!strcmp(action,RESOLVER_CLIENT_SEND_RESUME)) {
                suspended=0;
                write_log(RESOLVELOG,YESTIME,"Got resume message\n");
                continue;
                }
                else if (!strcmp(action,RESOLVER_CLIENT_SEND_REQUEST)) {
                        sscanf(s,"%d",&resolves[slot].user);
                        remove_first(s);
                        sscanf(s,"%s ",resolves[slot].site);
                        remove_first(s);
                        strcpy(resolves[slot].badlookup,s);
			if (naddy) resolves[slot].nameserver=naddy;
			else if (second_naddy) resolves[slot].nameserver=second_naddy;
			else if (third_naddy) resolves[slot].nameserver=third_naddy;
write_log(RESOLVELOG,YESTIME,"CLIPON: Starting query for ip %s user %d nameserver %s\n",
resolves[slot].site,resolves[slot].user,get_ip(resolves[slot].nameserver));
                        if (!start_query(slot, resolves[slot].site)) {
                                resolves[slot].status = STA_DONE;
                                resolves[slot].result = RES_NONAUTH;
				process_result(slot);
                                }
                        else continue;
                  }
                else if (!strcmp(action,RESOLVER_CLIENT_CANCEL_REQUEST)) {
                        /* find what slot this user request is in */
                        /* by matching up user number and site */
                        sscanf(s,"%d",&user);
                        remove_first(s);   
                        sscanf(s,"%s",site);
                        remove_first(s);
                        if ((slot = find_user_slot(user,site)) != -1) {
				SHUTDOWN(resolves[slot].fd, 2);
                                CLOSE(resolves[slot].fd);
                                resolves[slot].status = STA_DONE;
                                resolves[slot].result = RES_NONAUTH;
				process_result(slot);
                                }
                        continue;
                  }
		} /* end if FD_ISSET */

		for (i=1;i<nfds;++i) {
		  if (FD_ISSET(i,&fds_read)) {
#if defined(RESOLVER_DEBUG)
			write_log(RESOLVELOG,YESTIME,"CLIPON: Gonna read result from remote fd now\n");
#endif
			/* i is the fd, not the slot! */
			slot = get_slot(i);
			/* printf("%s",""); */
			resolves[slot].result = read_result(slot);
#if defined(RESOLVER_DEBUG)
			write_log(RESOLVELOG,YESTIME,"CLIPON: Result read\n");
#endif
			if (resolves[slot].result == RES_ERROR) restart_query(slot);
			else resolves[slot].status = STA_DONE;
		  } /* end of if FD_ISSET */
		} /* end of for */

		/* stdout ready for writing? */

                  for (i = 0 ; i < MAX_RESOLVES_IN_PROGRESS ; i++)
                  {
			if (resolves[i].status == STA_DONE)
				process_result(i);
		  } /* end of for */

} /* end of while forever */

} /* end of main */


int start_query(int slot, char *ipaddy) {
int             ip[4];
char            qbuf[256];
struct sockaddr_in sin2;

memset((char *)&sin2, 0, sizeof(struct sockaddr_in));

        sin2.sin_family  = AF_INET;
        sin2.sin_port    = htons(nsport);

                (void)bcopy((char *)&resolves[slot].nameserver,(char *)&sin2.sin_addr,sizeof(resolves[slot].nameserver));

/* Make query */
if (sscanf(ipaddy, "%d.%d.%d.%d", &ip[0], &ip[1], &ip[2], &ip[3]) == 4) {
            sprintf(qbuf, "%d.%d.%d.%d.in-addr.arpa.",
                ip[3], ip[2], ip[1], ip[0]);
}
else {
return 0;
}

resolves[slot].buflen = res_mkquery(QUERY, qbuf, C_IN, T_PTR, NULL, 0, NULL, 
		resolves[slot].buf.qb2, sizeof resolves[slot].buf);
if (resolves[slot].buflen < 0) {
        return 0;
}
#if defined(HAVE_RES_NMKQUERY)
/* because we are not using our own function, we have to set */
/* flags here */
resolves[slot].buf.qb1.rd = 1;
#endif

#if defined(RESOLVER_DEBUG)
write_log(RESOLVELOG,YESTIME,"    ----- START QUERY DEBUG INFO -----\n");
my_print_query((u_char *)&resolves[slot].buf, (u_char *)&resolves[slot].buf + resolves[slot].buflen);
write_log(RESOLVELOG,YESTIME,"    ----- END QUERY DEBUG INFO   -----\n");
#endif

/* Use datagrams - UDP */
resolves[slot].fd = socket(AF_INET, SOCK_DGRAM, 0);
/* resolves[slot].fd = socket(AF_INET, SOCK_STREAM, 0); */
if (resolves[slot].fd < 0) {
	return 0;
}

resolves[slot].status		= STA_PENDING;
resolves[slot].starttime	= time(0);

#if RES_BSD >= 43

#if defined(RESOLVER_DEBUG)
write_log(RESOLVELOG,YESTIME,"CLIPON: connecting to nameserver\n");
#endif

if (connect(resolves[slot].fd, (struct sockaddr *)&sin2, sizeof sin2) < 0) {
#if defined(RESOLVER_DEBUG)
write_log(RESOLVELOG,YESTIME,"CLIPON: connect failed!\n");
#endif
	return 0;
}

#if defined(RESOLVER_DEBUG)
write_log(RESOLVELOG,YESTIME,"CLIPON: sending to nameserver with send\n");
#endif

if (S_WRITE(resolves[slot].fd, (char *)&resolves[slot].buf, resolves[slot].buflen) != resolves[slot].buflen) {
	SHUTDOWN(resolves[slot].fd, 2);
	CLOSE(resolves[slot].fd);
#if defined(RESOLVER_DEBUG)
write_log(RESOLVELOG,YESTIME,"CLIPON: write failed!\n");
#endif
	return 0;
}
#else

#if defined(RESOLVER_DEBUG)
write_log(RESOLVELOG,YESTIME,"CLIPON: sending to nameserver with sendto\n");
#endif

if (sendto(resolves[slot].fd, (const char *)&resolves[slot].buf, resolves[slot].buflen, 0, 
	(struct sockaddr *) &sin2, sizeof sin2) != resolves[slot].buflen) {
	SHUTDOWN(resolves[slot].fd, 2);
	CLOSE(resolves[slot].fd);
#if defined(RESOLVER_DEBUG)
write_log(RESOLVELOG,YESTIME,"CLIPON: sendto failed!\n");
#endif
	return 0;
}
#endif

#if defined(RESOLVER_DEBUG)
write_log(RESOLVELOG,YESTIME,"CLIPON: sent\n");
#endif
return 1;
/* sent query, now wait for select in main loop to return a reply */
} /* end of function */


int restart_query(int slot) {

			resolves[slot].tries++;
			 if (resolves[slot].tries==number_of_servers) {
			  resolves[slot].status = STA_DONE;
			  resolves[slot].result = RES_NONAUTH;
			  process_result(slot);
			 }
			 else {
			  if (resolves[slot].tries==1) {
				resolves[slot].nameserver=second_naddy;
				write_log(RESOLVELOG,YESTIME,"CLIPON: Try 2 for %s with nameserver %s\n",
				resolves[slot].site,get_ip(resolves[slot].nameserver));
	                        if (!start_query(slot, resolves[slot].site)) {
                                 resolves[slot].status = STA_DONE;
                                 resolves[slot].result = RES_NONAUTH;
				 process_result(slot);
                                }
			  } /* if 1 */
			  else if (resolves[slot].tries==2) {
				resolves[slot].nameserver=third_naddy;
				write_log(RESOLVELOG,YESTIME,"CLIPON: Try 3 for %s with nameserver %s\n",
				resolves[slot].site,get_ip(resolves[slot].nameserver));
	                        if (!start_query(slot, resolves[slot].site)) {
                                 resolves[slot].status = STA_DONE;
                                 resolves[slot].result = RES_NONAUTH;
				 process_result(slot);
                                }
			  } /* if 2 */
			 } /* else */
return 1;
}


int read_result(int slot) {
int		n, type, origClass, class, qdcount, ancount, arcount, nscount, dlen;
int		buflen, resplen, anslen, trueLenPtr;
NET_SIZE_T salen;
Boolean		haveAnswer;
Boolean		printedAnswers;
u_char		*eom, *bp;
register HEADER	*headerPtr;
register u_char	*cp;
size_t		s;
struct sockaddr_in	sa;

printedAnswers = FALSE;
origClass = C_IN;

salen = sizeof sa;
anslen = sizeof(resolves[slot].answer);
resplen = recvfrom(resolves[slot].fd,
		   (u_char *)&resolves[slot].answer, 
                   anslen, 0, (struct sockaddr *)&sa, &salen);
if (resplen <= 0) {
SHUTDOWN(resolves[slot].fd, 2);
CLOSE(resolves[slot].fd);
write_log(RESOLVELOG,YESTIME,"CLIPON: Bad result read from nameserver %s\n",get_ip(resolves[slot].nameserver));
return(RES_ERROR);
}

#if defined(RESOLVER_DEBUG)
write_log(RESOLVELOG,YESTIME,"    ----- START ANSWER DEBUG INFO -----\n");
write_log(RESOLVELOG,YESTIME,"Got answer (%d bytes):\n",resplen);
my_print_query((u_char *) &resolves[slot].answer, (u_char *) &resolves[slot].answer + resplen);
write_log(RESOLVELOG,YESTIME,"    ----- END ANSWER DEBUG INFO   -----\n");
#endif

		SHUTDOWN(resolves[slot].fd, 2);
                CLOSE(resolves[slot].fd);
                s = -1;
                trueLenPtr = resplen;
                /*  *trueLenPtr = resplen; */

/* process answer */
    eom = (u_char *) &resolves[slot].answer + trueLenPtr;

    headerPtr = (HEADER *) &resolves[slot].answer;

/*
 * Currently defined response codes
 */
/* NOERROR         0               no error */
/* FORMERR         1               format error */
/* SERVFAIL        2               server failure */
/* NXDOMAIN        3               non existent domain */
/* NOTIMP          4               not implemented */
/* REFUSED         5               query refused */
    if (headerPtr->rcode != NOERROR) {
        return (headerPtr->rcode);
    }

/* qdcount - number of question entries */
/* ancount - number of answer entries */
/* nscount - number of authority entries */
/* arcount - number of resource entries */

    qdcount = ntohs(headerPtr->qdcount);
    ancount = ntohs(headerPtr->ancount);
    arcount = ntohs(headerPtr->arcount);
    nscount = ntohs(headerPtr->nscount);

#if defined(RESOLVER_DEBUG)
write_log(RESOLVELOG,YESTIME,"QDCOUNT: %d\n",qdcount);
write_log(RESOLVELOG,YESTIME,"ANCOUNT: %d\n",ancount);
write_log(RESOLVELOG,YESTIME,"NSCOUNT: %d\n",nscount);
write_log(RESOLVELOG,YESTIME,"ARCOUNT: %d\n",arcount);
#endif


    /*
     * If there are no answer, n.s. or additional records
     * then return with an error.
     */
      if (!ancount) {
        return (RES_NO_INFO);
	}

    bp     = hostbuf;
    buflen = sizeof(hostbuf);
    cp     = (u_char *) &resolves[slot].answer + HFIXEDSZ;
    /* Skip over question section. */

    while (qdcount-- > 0) {
            n = dn_skipname(cp, eom);
            if (n < 0) {
                    return (RES_ERROR);
                }
            cp += n + QFIXEDSZ;
            if (cp > eom) {
                      return (RES_ERROR);
                }
    }
    haveAnswer  = FALSE;

    /*
     * Scan through the answer resource records.
     * Answers for address query types are saved.
     * Other query type answers are just printed.
     */

            while (--ancount >= 0 && cp < eom) {
                n = dn_expand(resolves[slot].answer.qb2, eom, cp, (char *)bp, buflen);
                if (n < 0) {
#if defined(RESOLVER_DEBUG)
                write_log(RESOLVELOG,YESTIME,"CLIPON: Returning error in dn_expand n: %d h_errno: %d\n",n,h_errno);
#endif
                    return(RES_ERROR);
                }
                cp += n;
                if (cp + 3 * INT16SZ + INT32SZ > eom)
                        return (RES_ERROR);
                type  = GetShort(cp);
                class = GetShort(cp);
                cp   += INT32SZ;        /* skip TTL */
                dlen  = GetShort(cp);
                if (cp + dlen > eom)
                        return (RES_ERROR);
                if (type == T_CNAME) {
                    /*
                     * Found an alias.
                     */
#if defined(RESOLVER_DEBUG)
                write_log(RESOLVELOG,YESTIME,"CLIPON: Found a CNAME type %d/%d\n",class,type);
#endif
		cp += dlen;
		continue;
                } else if (type == T_PTR) {
                    /*
                     *  Found a "pointer" to the real name.
                     */
#if defined(RESOLVER_DEBUG)
                write_log(RESOLVELOG,YESTIME,"CLIPON: Found a PTR type %d/%d\n",class,type);
#endif
                    n = dn_expand(resolves[slot].answer.qb2, eom, cp, (char *)bp, buflen);
			strcpy(resolves[slot].hostname,(char *)bp);
                    if (n < 0) {
                        cp += n;
                        continue;
                    }
                    cp += n;
                    s = strlen((char *)bp) + 1;
                    haveAnswer = TRUE;
                    break;
                } else if (type != T_A) {
#if defined(RESOLVER_DEBUG)
                write_log(RESOLVELOG,YESTIME,"CLIPON: Found an A type %d/%d\n",class,type);
#endif
                    cp += dlen;
                    continue;
                }
		else {
#if defined(RESOLVER_DEBUG)
                write_log(RESOLVELOG,YESTIME,"CLIPON: Found an UNKNOWN type %d/%d\n",class,type);
#endif
		}
                    /*
                     * If we've already got 1 address, we aren't interested
                     * in addresses with a different length or class.
                     */

                    /*
                     * First address: record its length and class so we
                     * only save additonal ones with the same attributes.
                     */
            } /* end of while */

return (RES_SUCCESS);

}


void init_structs(int slot) {

resolves[slot].fd		= -1;
resolves[slot].user		= -1;
resolves[slot].tries		= 0;
resolves[slot].nameserver	= 0;
resolves[slot].status		= STA_WAITING;
resolves[slot].result		= RES_SUCCESS;
resolves[slot].buflen		= 0;
resolves[slot].site[0]		= 0;
resolves[slot].hostname[0]	= 0;
resolves[slot].badlookup[0]	= 0;
resolves[slot].buf.qb2[0]	= 0;
resolves[slot].answer.qb2[0]	= 0;
resolves[slot].starttime	= 0;
}


int find_free_ns_slot(void) {
int a;

for (a=0;a<MAX_RESOLVES_IN_PROGRESS;++a) {
if (resolves[a].fd == -1) return a;
}
return -1;

}

int find_user_slot(int user, char *site) {
int a;

for (a=0;a<MAX_RESOLVES_IN_PROGRESS;++a) {
if ((resolves[a].user == user) && (!strcmp(resolves[a].site,site))) return a;
}
return -1;
}

int get_slot(int fd) {
int b=0;

for (b=0;b<MAX_RESOLVES_IN_PROGRESS;++b) {
if (resolves[b].fd==fd) return b;
}
return -1;
}


char *Calloc(register int num, register int size)
{
    char *ptr;

    ptr = Malloc(num*size);
if (ptr == NULL) { return NULL; }
    memset(ptr, 0, num*size);
    return(ptr);
}


char *Malloc(int size)
{
    char        *ptr;

      ptr = malloc((unsigned) size);

    if (ptr == NULL) {
			write_log(RESOLVELOG,YESTIME,"CLIPON: bad malloc\n");
	exit(0);
	}
    return (ptr);
}

u_int my_ns_get16(u_char *src) {
        u_int dst;
        NS_GET16(dst, src);
        return (dst);
}

u_long my_ns_get32(u_char *src) {
        u_long dst;
        NS_GET32(dst, src);
        return (dst);
}


RETSIGTYPE handle_sig(int sig) {

/* write_log(RESOLVELOG,YESTIME,"CLIPON: Caught signal %d!\n",sig); */
switch(sig) {
	case SIGTERM:	write_log(RESOLVELOG,YESTIME,"CLIPON: Got sigterm!\n");
			exit(1);
	case SIGBUS:	write_log(RESOLVELOG,YESTIME,"CLIPON: Got sigbus!\n");
			exit(1);
	case SIGSEGV:	write_log(RESOLVELOG,YESTIME,"CLIPON: Got sigsegv!\n");
			exit(1);
	case SIGILL:	write_log(RESOLVELOG,YESTIME,"CLIPON: Got sigill!\n");
			exit(1);
	case SIGABRT:	write_log(RESOLVELOG,YESTIME,"CLIPON: Got sigabrt!\n");
			exit(1);
	case SIGINT:	write_log(RESOLVELOG,YESTIME,"CLIPON: Got sigint!\n");
			exit(1);
	case SIGCHLD:	write_log(RESOLVELOG,YESTIME,"CLIPON: Got sigchld!\n");
			exit(1);
	case SIGSTOP:	write_log(RESOLVELOG,YESTIME,"CLIPON: Got sigstop!\n"); break;
			exit(1);
	case SIGPIPE:	write_log(RESOLVELOG,YESTIME,"CLIPON: Got sigpipe!\n"); break;
	default: write_log(RESOLVELOG,YESTIME,"CLIPON: Got unknown signal!\n"); break;
	}

}

int write_stdout(char *msg) {
int n;

n = write(1,msg,strlen(msg));
return n;
}

/*** removes first word at front of string and moves rest down ***/
void remove_first(char *inpstr)
{
int newpos,oldpos;

newpos=0;  oldpos=0;
/* find first word */
while(inpstr[oldpos]==' ') {
        if (!inpstr[oldpos]) { inpstr[0]=0;  return; }
        oldpos++;
        }
/* find end of first word */
while(inpstr[oldpos]!=' ') {
        if (!inpstr[oldpos]) { inpstr[0]=0;  return; }
        oldpos++;
        }
/* find second word */
while(inpstr[oldpos]==' ') {
        if (!inpstr[oldpos]) { inpstr[0]=0;  return; }
        oldpos++;
        }
while(inpstr[oldpos]!=0)
        inpstr[newpos++]=inpstr[oldpos++];
inpstr[newpos]='\0';
}

/* Write string and arguments to a specific logging facility */
void write_log(int type, int wanttime, char *str, ...)
{
char z_mess[256*2];
char logfile[FILE_NAME_LEN];
va_list args;
FILE *fp;
/* if (!syslog_on) return; */

z_mess[0]=0;
strcpy(z_mess,LOGFILE);
strncpy(logfile,z_mess,FILE_NAME_LEN);
 if (!(fp=fopen(logfile,"a")))
   {
/*
    sprintf(z_mess,"%s LOGGING: Couldn't open file(a) \"%s\"! %s",STAFF_PREFIX,logfile,get_error());
    writeall_str(z_mess, WIZ_ONLY, -1, 0, -1, BOLD, NONE, 0);
*/
    return;
   }
 else {
    va_start(args,str);
    if (wanttime) {
    sprintf(z_mess,"%s: ",get_time(0,0));
    vsprintf(z_mess+strlen(z_mess),str,args);
    }
    else
    vsprintf(z_mess,str,args);
    va_end(args);
    fputs(z_mess,fp);
    FCLOSE(fp);
   }
}

/* Get time in a certain way and return it as a string */
/* mode 0 is to get rid of the year string and the carriage return */
/* mode 1 is to get rid of just the carriage return */
char *get_time(time_t ref,int mode)
{
time_t tm;
static char mrtime[30];
if ((int)ref==0) {
   time(&tm);
   strcpy(mrtime,ctime(&tm));
  }
else {
   strcpy(mrtime,ctime(&ref));
  }
if (mode==0)
   mrtime[strlen(mrtime)-6]=0; /* get rid of newline and year */
else if (mode==1)
   mrtime[strlen(mrtime)-1]=0; /* get rid of newline */
   return mrtime;
}


char *get_error(void)
{
static char errstr[256];
sprintf(errstr,"(%d:%s)",errno,strerror(errno));
return errstr;
}


void process_result(int slot) {

switch (resolves[slot].result) {
        case RES_SUCCESS:
            /*
             *  If the query was for an address, then the &curHostInfo
             *  variable can be used by Finger.
             *  There's no need to print anything for other query types
             *  because the info has already been printed.
             */
            break;
        /*
         * No Authoritative answer was available but we got names
         * of servers who know about the host.
         */
        case RES_NONAUTH:
#if defined(RESOLVER_DEBUG)
		write_log(RESOLVELOG,YESTIME,"CLIPON: RESULT: No auth answers!\n");
#endif
		strcpy(resolves[slot].hostname,resolves[slot].badlookup);
		break;
        case RES_NO_INFO:
#if defined(RESOLVER_DEBUG)
		write_log(RESOLVELOG,YESTIME,"CLIPON: RESULT: No records available!\n");
#endif
		strcpy(resolves[slot].hostname,resolves[slot].badlookup);
		break;
        case RES_TIME_OUT:
#if defined(RESOLVER_DEBUG)
		write_log(RESOLVELOG,YESTIME,"CLIPON: RESULT: Request timed out!\n");
#endif
		strcpy(resolves[slot].hostname,resolves[slot].badlookup);
		break;
        default:
#if defined(RESOLVER_DEBUG)
		write_log(RESOLVELOG,YESTIME,"CLIPON: RESULT: Can't find host!\n");
#endif
		strcpy(resolves[slot].hostname,resolves[slot].badlookup);
		break;
    } /* end of switch */
#if defined(HAVE_SNPRINTF)
snprintf(mess,256,"%d %s %s ",resolves[slot].user,resolves[slot].site,resolves[slot].hostname);
#else
sprintf(mess,"%d %s %s ",resolves[slot].user,resolves[slot].site,resolves[slot].hostname);
#endif
write_stdout(mess);
write_log(RESOLVELOG,YESTIME,"CLIPON: RESULT: Wrote results \"%s\" from fd %d to talker (total time %d secs)\n",mess,resolves[slot].fd,(int)(time(0)-resolves[slot].starttime));
init_structs(slot);
}


/* LIB STUFF */
/*
 * Expand compressed domain name 'comp_dn' to full domain name.
 * 'msg' is a pointer to the begining of the message,
 * 'eomorig' points to the first location after the message,
 * 'exp_dn' is a pointer to a buffer of size 'length' for the result.
 * Return size of compressed name or -1 if there was an error.
 */
int dn_expand(const u_char *msg, const u_char *eom, const u_char *src,
          char *dst, int dstsiz)
{
        int n = ns_name_uncompress(msg, eom, src, dst, (size_t)dstsiz);
        if (n > 0 && dst[0] == '.')
                dst[0] = '\0';
        return (n);
}

/*
 * Skip over a compressed domain name. Return the size or -1.
 */
int dn_skipname(const u_char *ptr, const u_char *eom) {
        const u_char *saveptr = ptr;
        if (ns_name_skip(&ptr, eom) == -1)
                return (-1);
        return (ptr - saveptr);
}

/*
 * ns_name_uncompress(msg, eom, src, dst, dstsiz)
 *      Expand compressed domain name to presentation format.
 * return:
 *      Number of bytes read out of `src', or -1 (with errno set).
 * note:
 *      Root domain returns as "." not "".
 */
int ns_name_uncompress(const u_char *msg, const u_char *eom, const u_char *src,
                   char *dst, size_t dstsiz)
{
        u_char tmp[NS_MAXCDNAME];
        int n;
        if ((n = ns_name_unpack(msg, eom, src, tmp, sizeof tmp)) == -1)
                return (-1);
        if (ns_name_ntop(tmp, dst, dstsiz) == -1)
                return (-1);
        return (n);
}

/*
 * ns_name_skip(ptrptr, eom)
 *      Advance *ptrptr to skip over the compressed name it points at.
 * return:
 *      0 on success, -1 (with errno set) on failure.
 */
int ns_name_skip(const u_char **ptrptr, const u_char *eom) {
        const u_char *cp;
        u_int n;
        cp = *ptrptr;
        while (cp < eom && (n = *cp++) != 0) {
                /* Check for indirection. */
                switch (n & NS_CMPRSFLGS) {
                case 0:                 /* normal case, n == len */
                        cp += n;
                        continue;
                case NS_CMPRSFLGS:      /* indirection */
                        cp++;
                        break;
                default:                /* illegal type */
                        errno = EMSGSIZE;
                        return (-1);
                }
                break;
        }
        if (cp > eom) {
                errno = EMSGSIZE;
                return (-1);
        }
        *ptrptr = cp;
        return (0);
}


/*
 * ns_name_ntop(src, dst, dstsiz)
 *      Convert an encoded domain name to printable ascii as per RFC1035.
 * return:
 *      Number of bytes written to buffer, or -1 (with errno set)
 * notes:
 *      The root is returned as "."
 *      All other domains are returned in non absolute form
 */
int ns_name_ntop(const u_char *src, char *dst, size_t dstsiz) {
        const u_char *cp;
        char *dn, *eom;
        u_char c;
        u_int n;
        cp = src;
        dn = dst;
        eom = dst + dstsiz;
        while ((n = *cp++) != 0) {
                if ((n & NS_CMPRSFLGS) != 0) {
                        /* Some kind of compression pointer. */
                        errno = EMSGSIZE;
                        return (-1);
                }
                if (dn != dst) {
                        if (dn >= eom) {
                                errno = EMSGSIZE;
                                return (-1);
                        }
                        *dn++ = '.';
                }
                if (dn + n >= eom) {
                        errno = EMSGSIZE;
                        return (-1);
                }
                for ((void)NULL; n > 0; n--) {
                        c = *cp++;
                        if (special(c)) {
                                if (dn + 1 >= eom) {
                                        errno = EMSGSIZE;
                                        return (-1);
                                }
                                *dn++ = '\\';
                                *dn++ = (char)c;
                        } else if (!printable(c)) {
                                if (dn + 3 >= eom) {
                                        errno = EMSGSIZE;
                                        return (-1);
                                }
                                *dn++ = '\\';
                                *dn++ = digits[c / 100];
                                *dn++ = digits[(c % 100) / 10];
                                *dn++ = digits[c % 10];
                        } else {
                                if (dn >= eom) {
                                        errno = EMSGSIZE;
                                        return (-1);
                                }
                                *dn++ = (char)c;
                        }
                }
        }
        if (dn == dst) {
                if (dn >= eom) {
                        errno = EMSGSIZE;
                        return (-1);
                }
                *dn++ = '.';
        }
        if (dn >= eom) {
                errno = EMSGSIZE;
                return (-1);
        }
        *dn++ = '\0';
        return (dn - dst);
}

/*
 * ns_name_unpack(msg, eom, src, dst, dstsiz)
 *      Unpack a domain name from a message, source may be compressed.
 * return:
 *      -1 if it fails, or consumed octets if it succeeds.
 */
int ns_name_unpack(const u_char *msg, const u_char *eom, const u_char *src,
               u_char *dst, size_t dstsiz)
{
        const u_char *srcp, *dstlim;
        u_char *dstp;
        int n, len, checked;
        len = -1;
        checked = 0;
        dstp = dst;
        srcp = src;
        dstlim = dst + dstsiz;
        if (srcp < msg || srcp >= eom) {
                errno = EMSGSIZE;
                return (-1);
        }
        /* Fetch next label in domain name. */
        while ((n = *srcp++) != 0) {
                /* Check for indirection. */
                switch (n & NS_CMPRSFLGS) {
                case 0:
                        /* Limit checks. */
                        if (dstp + n + 1 >= dstlim || srcp + n >= eom) {
                                errno = EMSGSIZE;
                                return (-1);
                        }
                        checked += n + 1;
                        *dstp++ = n;
                        memcpy(dstp, srcp, n);
                        dstp += n;
                        srcp += n;
                        break;
                case NS_CMPRSFLGS:
                        if (srcp >= eom) {
                                errno = EMSGSIZE;
                                return (-1);
                        }
                        if (len < 0)
                                len = srcp - src + 1;
                        srcp = msg + (((n & 0x3f) << 8) | (*srcp & 0xff));
                        if (srcp < msg || srcp >= eom) {  /* Out of range. */
                                errno = EMSGSIZE;
                                return (-1);
                        }
                        checked += 2;
                        /*
                         * Check for loops in the compressed name;
                         * if we've looked at the whole message,
                         * there must be a loop.
                         */
                        if (checked >= eom - msg) {
                                errno = EMSGSIZE;
                                return (-1);
                        }
                        break;
                default:
                        errno = EMSGSIZE;
                        return (-1);                    /* flag error */
                }
        }
        *dstp = '\0';
        if (len < 0)
                len = srcp - src;
        return (len);
}

/*
 * special(ch)
 *      Thinking in noninternationalized USASCII (per the DNS spec),
 *      is this characted special ("in need of quoting") ?
 * return:
 *      boolean.
 */
static int special(int ch) {
        switch (ch) {
        case 0x22: /* '"' */
        case 0x2E: /* '.' */
        case 0x3B: /* ';' */
        case 0x5C: /* '\\' */
        /* Special modifiers in zone files. */
        case 0x40: /* '@' */
        case 0x24: /* '$' */
                return (1);
        default:
                return (0);
        }
}

/*
 * printable(ch)
 *      Thinking in noninternationalized USASCII (per the DNS spec),
 *      is this character visible and not a space when printed ?
 * return:
 *      boolean.
 */
static int printable(int ch) {
        return (ch > 0x20 && ch < 0x7f);
}


int res_mkquery(int op,                     /* opcode of query */
            const char *dname,          /* domain name */
            int class, int type,        /* class and type of query */
            const u_char *data,         /* resource record data */
            int datalen,                /* length of data */
            const u_char *newrr_in,     /* new rr for modify or append */
            u_char *buf,                /* buffer to put query */
            int buflen)                 /* size of buffer */
{
/*
        if ((_res.options & RES_INIT) == 0 && res_init() == -1) {
                RES_SET_H_ERRNO(&_res, NETDB_INTERNAL);
                return (-1);
        }
*/

        return (res_nmkquery(_my_res, op, dname, class, type,
                             data, datalen,
                             newrr_in, buf, buflen));
}

#if !defined(HAVE_RES_NMKQUERY)
/*
 * Form all types of queries.
 * Returns the size of the result or -1.
 */
int res_nmkquery(struct __res_state statp,
	     int op,			/* opcode of query */
	     const char *dname,		/* domain name */
	     int class, int type,	/* class and type of query */
	     const u_char *data,	/* resource record data */
	     int datalen,		/* length of data */
	     const u_char *newrr_in,	/* new rr for modify or append */
	     u_char *buf,		/* buffer to put query */
	     int buflen)		/* size of buffer */
{
	register HEADER *hp;
	register u_char *cp;
	register int n;
	u_char *dnptrs[20], **dpp, **lastdnptr;

	/*
	 * Initialize header fields.
	 */
	if ((buf == NULL) || (buflen < HFIXEDSZ))
		return (-1);
	memset(buf, 0, HFIXEDSZ);
	hp = (HEADER *) buf;
	hp->id = htons(++statp.id);
	hp->opcode = op;
	hp->rd = 1;
/*	hp->rd = (statp.options & RES_RECURSE) != 0; */
	hp->rcode = NOERROR;
	cp = buf + HFIXEDSZ;
	buflen -= HFIXEDSZ;
	dpp = dnptrs;
	*dpp++ = buf;
	*dpp++ = NULL;
	lastdnptr = dnptrs + sizeof dnptrs / sizeof dnptrs[0];
	/*
	 * perform opcode specific processing
	 */
	switch (op) {
	case QUERY:	/*FALLTHROUGH*/
	case NS_NOTIFY_OP:
		if ((buflen -= QFIXEDSZ) < 0)
			return (-1);
		if ((n = dn_comp(dname, cp, buflen, dnptrs, lastdnptr)) < 0)
			return (-1);
		cp += n;
		buflen -= n;
		my_putshort(type, cp);
		cp += INT16SZ;
		my_putshort(class, cp);
		cp += INT16SZ;
		hp->qdcount = htons(1);
		if (op == QUERY || data == NULL)
			break;
		/*
		 * Make an additional record for completion domain.
		 */
		buflen -= RRFIXEDSZ;
/* 		n = dn_comp((char *)data, cp, buflen, dnptrs, lastdnptr); */
 		n = dn_comp((const char *)data, cp, buflen, dnptrs, lastdnptr);
		if (n < 0)
			return (-1);
		cp += n;
		buflen -= n;
		my_putshort(T_NULL, cp);
		cp += INT16SZ;
		my_putshort(class, cp);
		cp += INT16SZ;
		NS_PUT32(0, cp);
		cp += INT32SZ;
		my_putshort(0, cp);
		cp += INT16SZ;
		hp->arcount = htons(1);
		break;

	case IQUERY:
		/*
		 * Initialize answer section
		 */
		if (buflen < 1 + RRFIXEDSZ + datalen)
			return (-1);
		*cp++ = '\0';	/* no domain name */
		my_putshort(type, cp);
		cp += INT16SZ;
		my_putshort(class, cp);
		cp += INT16SZ;
		NS_PUT32(0, cp);
		cp += INT32SZ;
		my_putshort(datalen, cp);
		cp += INT16SZ;
		if (datalen) {
			memcpy(cp, data, datalen);
			cp += datalen;
		}
		hp->ancount = htons(1);
		break;

	default:
		return (-1);
	}
	return (cp - buf);
}
#endif

int dn_comp(const char *src, u_char *dst, int dstsiz,
        u_char **dnptrs, u_char **lastdnptr)
{
        return (ns_name_compress(src, dst, (size_t)dstsiz,
                                 (const u_char **)dnptrs,
                                 (const u_char **)lastdnptr));
}

/*
 * ns_name_compress(src, dst, dstsiz, dnptrs, lastdnptr)
 *      Compress a domain name into wire format, using compression pointers.
 * return:
 *      Number of bytes consumed in `dst' or -1 (with errno set).
 * notes:
 *      'dnptrs' is an array of pointers to previous compressed names.
 *      dnptrs[0] is a pointer to the beginning of the message.
 *      The list ends with NULL.  'lastdnptr' is a pointer to the end of the
 *      array pointed to by 'dnptrs'. Side effect is to update the list of
 *      pointers for labels inserted into the message as we compress the name.
 *      If 'dnptr' is NULL, we don't try to compress names. If 'lastdnptr'
 *      is NULL, we don't update the list.
 */
int ns_name_compress(const char *src, u_char *dst, size_t dstsiz,
                 const u_char **dnptrs, const u_char **lastdnptr)
{
        u_char tmp[NS_MAXCDNAME];
        if (ns_name_pton(src, tmp, sizeof tmp) == -1)
                return (-1);
        return (ns_name_pack(tmp, dst, dstsiz, dnptrs, lastdnptr));
}

/*
 * ns_name_pton(src, dst, dstsiz)
 *      Convert a ascii string into an encoded domain name as per RFC1035.
 * return:
 *      -1 if it fails
 *      1 if string was fully qualified
 *      0 is string was not fully qualified
 * notes:
 *      Enforces label and domain length limits.
 */
int ns_name_pton(const char *src, u_char *dst, size_t dstsiz) {
        u_char *label, *bp, *eom;
        int c, n, escaped;
        char *cp;
        escaped = 0;
        bp = dst;
        eom = dst + dstsiz;
        label = bp++;
        while ((c = *src++) != 0) {
                if (escaped) {
                        if ((cp = strchr(digits, c)) != NULL) {
                                n = (cp - digits) * 100;
                                if ((c = *src++) == 0 ||
                                    (cp = strchr(digits, c)) == NULL) {
                                        errno = EMSGSIZE;
                                        return (-1);
                                }
                                n += (cp - digits) * 10;
                                if ((c = *src++) == 0 ||
                                    (cp = strchr(digits, c)) == NULL) {
                                        errno = EMSGSIZE;
                                        return (-1);
                                }
                                n += (cp - digits);
                                if (n > 255) {
                                        errno = EMSGSIZE;
                                        return (-1);
                                }
                                c = n;
                        }
                        escaped = 0;
                } else if (c == '\\') {
                        escaped = 1;
                        continue;
                } else if (c == '.') {
                        c = (bp - label - 1);
                        if ((c & NS_CMPRSFLGS) != 0) {  /* Label too big. */
                                errno = EMSGSIZE;
                                return (-1);
                        }
                        if (label >= eom) {
                                errno = EMSGSIZE;
                                return (-1);
                        }
                        *label = c;
                        /* Fully qualified ? */
                        if (*src == '\0') {
                                if (c != 0) {
                                        if (bp >= eom) {
                                                errno = EMSGSIZE;
                                                return (-1);
                                        }
                                        *bp++ = '\0';
                                }
                                if ((bp - dst) > MAXCDNAME) {
                                        errno = EMSGSIZE;
                                        return (-1);
                                }
                                return (1);
                        }
                        if (c == 0 || *src == '.') {
                                errno = EMSGSIZE;
                                return (-1);
                        }
                        label = bp++;
                        continue;
                }
                if (bp >= eom) {
                        errno = EMSGSIZE;
                        return (-1);
                }
                *bp++ = (u_char)c;
        }
        c = (bp - label - 1);
        if ((c & NS_CMPRSFLGS) != 0) {          /* Label too big. */
                errno = EMSGSIZE;
                return (-1);
        }
        if (label >= eom) {
                errno = EMSGSIZE;
                return (-1);
        }
        *label = c;
        if (c != 0) {
                if (bp >= eom) {
                        errno = EMSGSIZE;
                        return (-1);
                }
                *bp++ = 0;
        }
        if ((bp - dst) > MAXCDNAME) {   /* src too big */
                errno = EMSGSIZE;
                return (-1);
        }
        return (0);
}

/*
 * ns_name_pack(src, dst, dstsiz, dnptrs, lastdnptr)
 *      Pack domain name 'domain' into 'comp_dn'.
 * return:
 *      Size of the compressed name, or -1.
 * notes:
 *      'dnptrs' is an array of pointers to previous compressed names.
 *      dnptrs[0] is a pointer to the beginning of the message. The array
 *      ends with NULL.
 *      'lastdnptr' is a pointer to the end of the array pointed to
 *      by 'dnptrs'.
 * Side effects:
 *      The list of pointers in dnptrs is updated for labels inserted into
 *      the message as we compress the name.  If 'dnptr' is NULL, we don't
 *      try to compress names. If 'lastdnptr' is NULL, we don't update the
 *      list.
 */
int ns_name_pack(const u_char *src, u_char *dst, int dstsiz,
             const u_char **dnptrs, const u_char **lastdnptr)
{
        u_char *dstp;
        const u_char **cpp, **lpp, *eob, *msg;
        const u_char *srcp;
        int n, l;
        srcp = src;
        dstp = dst;
        eob = dstp + dstsiz;
        lpp = cpp = NULL;
        if (dnptrs != NULL) {
                if ((msg = *dnptrs++) != NULL) {
                        for (cpp = dnptrs; *cpp != NULL; cpp++)
                                (void)NULL;
                        lpp = cpp;      /* end of list to search */
                }
        } else
                msg = NULL;
        /* make sure the domain we are about to add is legal */
        l = 0;
        do {
                n = *srcp;
                if ((n & NS_CMPRSFLGS) != 0) {
                        errno = EMSGSIZE;
                        return (-1);
                }
                l += n + 1;
                if (l > MAXCDNAME) {
                        errno = EMSGSIZE;
                        return (-1);
                }
                srcp += n + 1;
        } while (n != 0);
        /* from here on we need to reset compression pointer array on error */
        srcp = src;
        do {
                /* Look to see if we can use pointers. */
                n = *srcp;
                if (n != 0 && msg != NULL) {
                        l = dn_find(srcp, msg, (const u_char * const *)dnptrs,
                                    (const u_char * const *)lpp);
                        if (l >= 0) {
                                if (dstp + 1 >= eob) {
                                        goto cleanup;
                                }
                                *dstp++ = (l >> 8) | NS_CMPRSFLGS;
                                *dstp++ = l % 256;
                                return (dstp - dst);
                        }
                        /* Not found, save it. */
                        if (lastdnptr != NULL && cpp < lastdnptr - 1 &&
                            (dstp - msg) < 0x4000) {
                                *cpp++ = dstp;
                                *cpp = NULL;
                        }
                }
                /* copy label to buffer */
                if (n & NS_CMPRSFLGS) {         /* Should not happen. */
                        goto cleanup;
                }
                if (dstp + 1 + n >= eob) {
                        goto cleanup;
                }
                memcpy(dstp, srcp, n + 1);
                srcp += n + 1;
                dstp += n + 1;
        } while (n != 0);
        if (dstp > eob) {
cleanup:
                if (msg != NULL)
                        *lpp = NULL;
                errno = EMSGSIZE;
                return (-1);
        }
        return (dstp - dst);
}

/*
 * dn_find(domain, msg, dnptrs, lastdnptr)
 *      Search for the counted-label name in an array of compressed names.
 * return:
 *      offset from msg if found, or -1.
 * notes:
 *      dnptrs is the pointer to the first name on the list,
 *      not the pointer to the start of the message.
 */
static int dn_find(const u_char *domain, const u_char *msg,
        const u_char * const *dnptrs,
        const u_char * const *lastdnptr)
{
        const u_char *dn, *cp, *sp;
        const u_char * const *cpp;
        u_int n;
        for (cpp = dnptrs; cpp < lastdnptr; cpp++) {
                dn = domain;
                sp = cp = *cpp;
                while ((n = *cp++) != 0) {
                        /*
                         * check for indirection
                         */
                        switch (n & NS_CMPRSFLGS) {
                        case 0:                 /* normal case, n == len */
                                if (n != *dn++)
                                        goto next;
                                for ((void)NULL; n > 0; n--)
                                        if (mklower(*dn++) != mklower(*cp++))
                                                goto next;
                                /* Is next root for both ? */
                                if (*dn == '\0' && *cp == '\0')
                                        return (sp - msg);
                                if (*dn)
                                        continue;
                                goto next;
                        case NS_CMPRSFLGS:      /* indirection */
                                cp = msg + (((n & 0x3f) << 8) | *cp);
                                break;
                        default:        /* illegal type */
                                errno = EMSGSIZE;
                                return (-1);
                        }
                }
 next: ;
        }
        errno = ENOENT;
        return (-1);
}

static int mklower(int ch) {
        if (ch >= 0x41 && ch <= 0x5A)
                return (ch + 0x20);
        return (ch);
}

void my_putshort(unsigned short src, u_char *dst) { my_ns_put16(src, dst); }

void my_ns_put16(u_int src, u_char *dst) {
        NS_PUT16(src, dst);
}

#if !defined(HAVE_PUT32)
void putlong(u_int32_t src, u_char *dst) { ns_put32(src, dst); }
void ns_put32(u_long src, u_char *dst) {
        NS_PUT32(src, dst);
}
#endif


void my_print_query(u_char *msg, u_char *eom)
{
        register HEADER *hp;
        register u_char *cp;
        int n;
        u_int class=queryClass, type=queryType;
        /*
         * Print header fields.
         */
        hp = (HEADER *)msg;
        cp = (u_char *)msg + HFIXEDSZ;

                write_log(RESOLVELOG,YESTIME,"     HEADER:\n");
                write_log(RESOLVELOG,YESTIME,"\topcode = %s", _res_opcodes[hp->opcode]);
                write_log(RESOLVELOG,NOTIME,", id = %d", ntohs(hp->id));
                write_log(RESOLVELOG,NOTIME,", rcode = %s\n", my_p_rcode(hp->rcode));
                write_log(RESOLVELOG,YESTIME,"\theader flags: ");
                if (hp->qr) {
                        write_log(RESOLVELOG,NOTIME," response");
                } else {
                        write_log(RESOLVELOG,NOTIME," query");
                }
                if (hp->aa)
                        write_log(RESOLVELOG,NOTIME,", auth. answer");
                if (hp->tc)
                        write_log(RESOLVELOG,NOTIME,", truncation");
                if (hp->rd)
                        write_log(RESOLVELOG,NOTIME,", want recursion");
                if (hp->ra)
                        write_log(RESOLVELOG,NOTIME,", recursion avail.");
                if (hp->unused)
                        write_log(RESOLVELOG,NOTIME,", UNUSED-QUERY_BIT");
/* 
               if (hp->ad)
                        write_log(RESOLVELOG,NOTIME,", authentic data");
                if (hp->cd)
                        write_log(RESOLVELOG,NOTIME,", checking disabled");
*/
                write_log(RESOLVELOG,NOTIME,"\n");
		write_log(RESOLVELOG,YESTIME,"\tquestions = %d", ntohs(hp->qdcount));
                write_log(RESOLVELOG,NOTIME,",  answers = %d", ntohs(hp->ancount));
                write_log(RESOLVELOG,NOTIME,",  authority records = %d", ntohs(hp->nscount));
                write_log(RESOLVELOG,NOTIME,",  additional = %d\n\n", ntohs(hp->arcount));

        /*
         * Print question records.
         */
        n = ntohs(hp->qdcount);
        if (n > 0) {
                write_log(RESOLVELOG,YESTIME,"    QUESTIONS:\n");
                while (--n >= 0) {
                        write_log(RESOLVELOG,YESTIME,"\t");
                        cp = Print_cdname((const u_char *)cp, (const u_char *)msg, (const u_char *)eom);
                        if (cp == NULL)
                                return;
                	type  = GetShort(cp);
                	class = GetShort(cp);
                        write_log(RESOLVELOG,NOTIME,", type = %s", my_p_type(type));
                        write_log(RESOLVELOG,NOTIME,", class = %s\n", my_p_class(class));
                }
        }
        /*
         * Print authoritative answer records
         */
        n = ntohs(hp->ancount);
        if (n > 0) {
                write_log(RESOLVELOG,YESTIME,"    ANSWERS:\n");
                if (type == T_A && n > MAXADDRS) {
                        write_log(RESOLVELOG,YESTIME,"Limiting response to MAX Addrs = %d \n",
                               MAXADDRS);
                        n = MAXADDRS;
                }
                while (--n >= 0) {
                        write_log(RESOLVELOG,YESTIME,"    ->  ");
                        cp = Print_rr((const u_char *)cp, (const u_char *)msg, (const u_char *)eom);
                        if (cp == NULL)
                                return;
                }
        }
        /*
         * print name server records
         */
        n = ntohs(hp->nscount);
        if (n > 0) {
                write_log(RESOLVELOG,YESTIME,"    AUTHORITY RECORDS:\n");
                while (--n >= 0) {
                        write_log(RESOLVELOG,YESTIME,"    ->  ");
                        cp = Print_rr((const u_char *)cp, (const u_char *)msg, (const u_char *)eom);
                        if (cp == NULL)
                                return;
                }
        }
        /*
         * print additional records
         */
        n = ntohs(hp->arcount);
        if (n > 0) {
                write_log(RESOLVELOG,YESTIME,"    ADDITIONAL RECORDS:\n");
                while (--n >= 0) {
                        write_log(RESOLVELOG,YESTIME,"    ->  ");
                        cp = Print_rr((const u_char *)cp, (const u_char *)msg, (const u_char *)eom);
                        if (cp == NULL)
                                return;
                }
        }

write_log(RESOLVELOG,NOTIME,"\n");
}

u_char *Print_cdname(const u_char *cp, const u_char *msg, const u_char *eom)
{
        char name[MAXDNAME];
        int n;

        n = dn_expand(msg, eom, cp, name, sizeof name);
        if (n < 0)
                return (NULL);
        if (name[0] == '\0')
                strcpy(name, "(root)");

#if defined(RESOLVER_DEBUG)
	write_log(RESOLVELOG,NOTIME,"%s",name);
#endif
        return (cp + n);
}

u_char *Print_rr(const u_char *ocp, const u_char *msg, const u_char *eom) {
        u_int type, class;
        int dlen;
        u_long rrttl;
        register u_char *cp, *cp1;

        if ((cp = Print_cdname(ocp, msg, eom)) == NULL) {
                write_log(RESOLVELOG,NOTIME,"(name truncated?)\n");
                return (NULL);                  /* compression error */
        }
        BOUNDS_CHECK(cp, 3 * INT16SZ + INT32SZ);
        NS_GET16(type, cp);
        NS_GET16(class, cp);
        NS_GET32(rrttl, cp);
        NS_GET16(dlen, cp);
        BOUNDS_CHECK(cp, dlen);

	write_log(RESOLVELOG,NOTIME,"\n");
        write_log(RESOLVELOG,YESTIME,"\ttype = %s, class = %s, dlen = %d\n",
                                my_p_type(type), my_p_class(class), dlen);
        cp1 = cp;
        /*
         * Print type specific data, if appropriate
         */
        switch (type) {
        case T_NS:
                write_log(RESOLVELOG,YESTIME,"\tnameserver = ");
                goto doname;
        case T_PTR:
                write_log(RESOLVELOG,YESTIME,"\tname = ");
 doname:
                cp = Print_cdname(cp, msg, eom);
                if (cp == NULL) {
                        write_log(RESOLVELOG,NOTIME,"(name truncated?)\n");
                        return (NULL);                  /* compression error */
                }
                write_log(RESOLVELOG,NOTIME,"\n");
                break;
        case T_CNAME:
                write_log(RESOLVELOG,YESTIME,"\tname = ");
                cp = Print_cdname(cp, msg, eom);
                if (cp == NULL) {
                        write_log(RESOLVELOG,NOTIME,"(name truncated?)\n");
                        return (NULL);                  /* compression error */
                }
                write_log(RESOLVELOG,NOTIME,"\n");
                break;
	} /* end of switch */
/*
        if (cp != cp1 + dlen) {
                write_log(RESOLVELOG,NOTIME,"\n");
                write_log(RESOLVELOG,YESTIME,"*** Error: record size incorrect (%d != %d)\n\n",
                        cp - cp1, dlen);
                cp = NULL;
        }
*/
	write_log(RESOLVELOG,NOTIME,"\n");
        return (cp);
}


char *my_p_type(int mytype) {

switch(mytype) {
case T_PTR:	return "PTR";
case T_CNAME:	return "CNAME";
case T_A:	return "A";
case T_NS:	return "NS";
case T_ANY:	return "ANY";
default: return "UNKNOWN";
}
return NULL;

}

char *my_p_class(int myclass) {

switch(myclass) {
case C_IN:	return "IN";
case C_ANY:	return "ANY";
default: return "UNKNOWN";
}
return NULL;

}

char *my_p_rcode(int myrcode) {

switch(myrcode) {
case NOERROR:	return "NOERROR";
case FORMERR:	return "FORMERR";
case SERVFAIL:	return "SERVFAIL";
case NXDOMAIN:	return "NXDOMAIN";
case NOTIMP:		return "NOTIMP";
case REFUSED:	return "REFUSED";
default:	return "UNKNOWN";
}
return NULL;

}


/*
 * An os independent signal() with BSD semantics, e.g. the signal
 * catcher is restored following service of the signal.
 *
 * When sigset() is available, signal() has SYSV semantics and sigset()
 * has BSD semantics and call interface. Unfortunately, Linux does not
 * have sigset() so we use the more complicated sigaction() interface
 * there.
 *
 * Did I mention that signals suck?
 */
/* (*setsignal (int sig, RETSIGTYPE (*func)(int)))(int) */
handler_t
setsignal(int sig, handler_t handler)
{
#if defined(HAVE_SIGACTION)
        struct sigaction old, new;
#elif defined(HAVE_SIGMASK)
	struct sigvec nsv,osv;
#endif
	handler_t oldh;

#if defined(HAVE_SIGACTION)
        memset(&new, 0, sizeof(new));
        new.sa_handler = handler;
#if defined(SA_RESTART)
        new.sa_flags |= SA_RESTART;
#endif
        if (sigaction(sig, &new, &old) < 0)
                return ((handler_t)SIG_ERR);
        oldh = old.sa_handler;

#elif defined(HAVE_SIGMASK)
        memset(&nsv, 0, sizeof(nsv));
    nsv.sv_handler = handler;
    nsv.sv_mask = 0;                    /* punt */
    nsv.sv_flags = SV_INTERRUPT;        /* punt */
    sigvec(sig, &nsv, &osv);
    oldh = osv.sv_handler;
#elif defined(HAVE_SIGSET)
        oldh = (handler_t)sigset(sig, handler);
#else
        oldh = signal(sig, handler);
#endif
	return oldh;
}



#if defined(OPENBSD_SYS)
char *ctime(const time_t *mytm)   
{
struct tm *clocker;
static char buf1[32];

clocker=localtime(mytm);
#if defined(HAVE_STRFTIME)
strftime(buf1,sizeof(buf1),"%a %b %d %H:%M:%S %Y\n",clocker);
#else
#if defined(HAVE_SNPRINTF)
snprintf(buf1,sizeof(buf1),"%s %s %d %.2d:%.2d:%.2d %d\n",
clocker_daylist[clocker->tm_wday],clocker_monthlist[clocker->tm_mon],
clocker->tm_mday,clocker->tm_hour,clocker->tm_min,clocker->tm_sec,
1900+clocker->tm_year);
#else
sprintf(buf1,"%s %s %d %.2d:%.2d:%.2d %d\n",
clocker_daylist[clocker->tm_wday],clocker_monthlist[clocker->tm_mon],
clocker->tm_mday,clocker->tm_hour,clocker->tm_min,clocker->tm_sec,
1900+clocker->tm_year);
#endif
#endif

return buf1;
}
#endif

int SHUTDOWN(int sock, int how) {

#if defined(CYGWIN_SYS)
        return shutdown(sock, how);
#endif

return 1;
}

/* mid copy copies chunk from string strf to string strt */
void midcpy(char *strf, char *strt, int fr, int to)
{
int f;
for (f=fr;f<=to;++f)
  {          
   if (!strf[f])
     {
      strt[f-fr]='\0';
      return;
     }
   strt[f-fr]=strf[f];
  }
strt[f-fr]='\0';
}

char *get_ip(unsigned long addr) {
static char buf[21];

addr=ntohl(addr);
 buf[0]=0;
 sprintf(buf,"%ld.%ld.%ld.%ld", (addr >> 24) & 0xff, (addr >> 16) & 0xff,
         (addr >> 8) & 0xff, addr & 0xff);

return buf;
}


