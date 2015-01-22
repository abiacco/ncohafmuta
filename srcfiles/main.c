#if defined(HAVE_CONFIG_H)
#include "../hdrfiles/config.h"
#endif

#include "../hdrfiles/includes.h"

/*--------------------------------------------------------*/
/* Talker-related include files                           */ 
/*--------------------------------------------------------*/
#define _DEFINING_OSDEFS
#include "../hdrfiles/osdefs.h"
/*
#include "../hdrfiles/authuser.h" 
#include "../hdrfiles/text.h"
*/
#define _DEFINING_CONSTANTS
#include "../hdrfiles/constants.h"

/*--------------------------------------------------------*/
/* Initialize functions we need too                       */
/* Almost all, including all command functions            */
/*--------------------------------------------------------*/
#include "../hdrfiles/protos.h"
#include "../hdrfiles/resolver_clipon.h"

extern int resolver_toclient_pipes[2];
extern int resolver_toserver_pipes[2];
extern int resolver_clipon_pid;

extern int treboot;
extern char *our_delimiters;


/** Far too many bloody global declarations **/
/*---------------------------------------------------------*/
/* port definitions                                        */
/*---------------------------------------------------------*/
          
struct {
         int  total_connections_allowed;
         int  users;
         int  wizes;
         int  who;
         int  www;
         int  interconnect;
         int  cypherconnect;
        } range =
          {  MAX_USERS + MAX_WHO_CONNECTS + MAX_WWW_CONNECTS + MAX_INTERCONNECTS + MAX_CYPHERCONNECTS,
             NUM_USERS,
             NUM_USERS + NUM_WIZES,
             NUM_USERS + NUM_WIZES + MAX_WHO_CONNECTS,
             NUM_USERS + NUM_WIZES + MAX_WHO_CONNECTS + MAX_WWW_CONNECTS,
             NUM_USERS + NUM_WIZES + MAX_WHO_CONNECTS + MAX_WWW_CONNECTS + MAX_INTERCONNECTS,
             NUM_USERS + NUM_WIZES + MAX_WHO_CONNECTS + MAX_WWW_CONNECTS + MAX_INTERCONNECTS + MAX_CYPHERCONNECTS
           };
         
int PORT;                  /* main login port for incoming   */

int listen_sock[4];        /* 32 bit listening sockets       */
fd_set    readmask;        /* bitmap read set                */
fd_set    writemask;       /* bitmap write set               */
int inter;                 /* inter talker connections       */
int cypher;                /* caller id port                 */
             
int last_user;
static int nfds=0;
int restarting=0;

/** Big Letter array map **/
int biglet[26][5][5] =
     {{{0,1,1,1,0},{1,0,0,0,1},{1,1,1,1,1},{1,0,0,0,1},{1,0,0,0,1}},
      {{1,1,1,1,0},{1,0,0,0,1},{1,1,1,1,0},{1,0,0,0,1},{1,1,1,1,0}},
      {{0,1,1,1,1},{1,0,0,0,0},{1,0,0,0,0},{1,0,0,0,0},{0,1,1,1,1}},
      {{1,1,1,1,0},{1,0,0,0,1},{1,0,0,0,1},{1,0,0,0,1},{1,1,1,1,0}},
      {{1,1,1,1,1},{1,0,0,0,0},{1,1,1,1,0},{1,0,0,0,0},{1,1,1,1,1}},
      {{1,1,1,1,1},{1,0,0,0,0},{1,1,1,1,0},{1,0,0,0,0},{1,0,0,0,0}},
      {{0,1,1,1,0},{1,0,0,0,0},{1,0,1,1,0},{1,0,0,0,1},{0,1,1,1,0}},
      {{1,0,0,0,1},{1,0,0,0,1},{1,1,1,1,1},{1,0,0,0,1},{1,0,0,0,1}},
      {{0,1,1,1,0},{0,0,1,0,0},{0,0,1,0,0},{0,0,1,0,0},{0,1,1,1,0}},
      {{0,0,0,0,1},{0,0,0,0,1},{0,0,0,0,1},{1,0,0,0,1},{0,1,1,1,0}},
      {{1,0,0,0,1},{1,0,0,1,0},{1,0,1,0,0},{1,0,0,1,0},{1,0,0,0,1}},
      {{1,0,0,0,0},{1,0,0,0,0},{1,0,0,0,0},{1,0,0,0,0},{1,1,1,1,1}},
      {{1,0,0,0,1},{1,1,0,1,1},{1,0,1,0,1},{1,0,0,0,1},{1,0,0,0,1}},
      {{1,0,0,0,1},{1,1,0,0,1},{1,0,1,0,1},{1,0,0,1,1},{1,0,0,0,1}},
      {{0,1,1,1,0},{1,0,0,0,1},{1,0,0,0,1},{1,0,0,0,1},{0,1,1,1,0}},
      {{1,1,1,1,0},{1,0,0,0,1},{1,1,1,1,0},{1,0,0,0,0},{1,0,0,0,0}},
      {{0,1,1,1,0},{1,0,0,0,1},{1,0,1,0,1},{1,0,0,1,1},{0,1,1,1,0}},
      {{1,1,1,1,0},{1,0,0,0,1},{1,1,1,1,0},{1,0,0,1,0},{1,0,0,0,1}},
      {{0,1,1,1,1},{1,0,0,0,0},{0,1,1,1,0},{0,0,0,0,1},{1,1,1,1,0}},
      {{1,1,1,1,1},{0,0,1,0,0},{0,0,1,0,0},{0,0,1,0,0},{0,0,1,0,0}},
      {{1,0,0,0,1},{1,0,0,0,1},{1,0,0,0,1},{1,0,0,0,1},{1,1,1,1,1}},
      {{1,0,0,0,1},{1,0,0,0,1},{0,1,0,1,0},{0,1,0,1,0},{0,0,1,0,0}},
      {{1,0,0,0,1},{1,0,0,0,1},{1,0,1,0,1},{1,1,0,1,1},{1,0,0,0,1}},
      {{1,0,0,0,1},{0,1,0,1,0},{0,0,1,0,0},{0,1,0,1,0},{1,0,0,0,1}},
      {{1,0,0,0,1},{0,1,0,1,0},{0,0,1,0,0},{0,0,1,0,0},{0,0,1,0,0}},
      {{1,1,1,1,1},{0,0,0,1,0},{0,0,1,0,0},{0,1,0,0,0},{1,1,1,1,1}}};

char *syserror="Sorry - a system error has occured";
char mailgateway_ip[16];
unsigned int mailgateway_port=0;
char *atpos;

char area_nochange[MAX_AREAS]; 
char mess[ARR_SIZE+25];    /* functions use mess to send output   */ 
char t_mess[ARR_SIZE+25];  /* functions use t_mess as a buffer    */ 
char t_inpstr[ARR_SIZE];  /* functions use t_mess as a buffer    */ 
char datadir[255];	/* config directory                       */
char thishost[101];	/* FQDN were running on                   */
char thisos[101];       /* operating system were running on       */
char thisprog[255];	/* the binary the program is run as	  */

int noprompt;		/* talker waiting for user input?         */
int signl;
int atmos_on;		/* all room atmospherics on?              */
int syslog_on;		/* are we logging system stuff to a file  */
int allow_new;		/* can new users be created?              */
int dobackups;		/* are we archiving log files @ midnight? */

int tells;		/* tells in defined time period           */
int commands;		/* total commands in defined time period  */
int says;		/* says in defined time period            */
int says_running;
int tells_running;
int commands_running;

int shutd= -1;		/* talker waiting for confirm on a shutdown?    */
int delete_sent=0;

int sys_access=1;	/* is the system open for user connections?      */
int wiz_access=1;	/* is the system open for wizard connections?    */
int who_access=1;	/* is the system open for external who listings? */
int www_access=1;	/* is the system open for external web requests? */

int checked=0;		/* see if messages have been checked at midnight */
int new_room;			/* room new users log into               */
int debug	= 0;

/*** END OF GLOBAL DELCARATIONS ***/

/****************************************************************************
     Main function - 
     Sets up network data, signals, accepts user input and acts as 
     the switching center for speach output.
*****************************************************************************/ 
int main(int argc, char **argv)
{
struct sockaddr_in acc_addr; /* this is the socket for accepting */

int       as=0;             /* as = accept socket - 32 bit */
NET_SIZE_T       size;
int       user;
int       com_num;
int       new_user;	     /* new user's index number */
int       imotd=0;	     /* random MOTD placer */
int       i=0;
int	  retval=0;
int       fd;                /* file desc. for stdin,out,err    */

char      port;
char      filename[1024];
char      inpstr[ARR_SIZE];
char      smtpcode[4];

unsigned  long ip_address;   /* connector's net address actually  */
#if defined(WINDOWS)
unsigned  long arg = 1;      /* for a WIN32 nonblocking set       */
#endif

struct timeval sel_timeout;  /* how much time to give select() */

#if defined(WINDOWS)
WSADATA wsaData; /* WinSock details */
DWORD dwThreadId, dwThrdParam = 1;
DWORD alarm_thread(LPDWORD lpdwParam);
OSVERSIONINFO myinfo;
unsigned short wVersionRequested = MAKEWORD(1, 1); /* v1.1 requested */
int err; /* error status */
#endif

#if defined(HAVE_UNAME)
struct utsname uname_info;
#endif

fd=-1;
/* It's showtime!!!!! */
#if defined(WINDOWS)
sprintf(mess,"WIN%s - WIN32 VERSION -",VERSION);
SetConsoleTitle(mess);
puts(mess);
#else
puts(VERSION);
#endif

puts("Copyrighted software                      ");
puts("                                          ");

#if defined(WINDOWS)
    /* Need to include library: wsock32.lib for Windows Sockets */
    err = WSAStartup(wVersionRequested, &wsaData);
    if (err != 0) {
      printf("Error in WSAStartup %s\n", get_error());
      WSACleanup();
      exit(1);
    }

     /* Check the WinSock version information */
     if ( (LOBYTE(wsaData.wVersion) != LOBYTE(wVersionRequested)) ||
        (HIBYTE(wsaData.wVersion) != HIBYTE(wVersionRequested)) ) {
      printf("WinSock version %d found doesn't live up to WinSock version %d needed\n",wsaData.wHighVersion,wVersionRequested);
      WSACleanup();
      exit(1);
    }

     /* Print WinSock info */
     printf("%s\n%s\n",wsaData.szDescription,wsaData.szSystemStatus);
     printf("Max sockets program can open: %d\n",wsaData.iMaxSockets);
#endif                          /* WIN32 */

 if (argc==1)       /* check comline input */
    {
     puts("NOTE: Running with config data directory");
     strcpy(datadir,"config");
     debug=0;
    }
 else if (argc==2)
    {
     if (!strcmp(argv[1],"-d")) {
      puts("NOTE: Running in debug mode, I wont fork to the background.");
      strcpy(datadir,"config");
      debug=1;
     }
     else
      strcpy(datadir,argv[1]);
    }
 else if (argc==3)
    {
     strcpy(datadir,argv[1]);
     if (!strcmp(argv[2],"-d")) {
      puts("NOTE: Running in debug mode, I wont fork to the background.");
      debug=1;
     }
     else {
      puts("Invalid option. Valid options are:");
      puts(" -d  (dont fork to background)");
      exit(0);
     }
    }

/* Copy the program binary name to memory */
strcpy(thisprog,argv[0]);

/* Leave a buffer of 3 fds for open files */
if (range.total_connections_allowed > FD_SETSIZE-3) {
 printf("*** Max connections defined higher than number of file ***\n");
 printf("*** descriptors allowed by host machine..lower them    ***\n");
 printf("Connections tried to allocate: %d\n",
        range.total_connections_allowed);
 printf("Connections allowed to allocate: %d\n",
        FD_SETSIZE-3);
#if defined(WINDOWS)
	WSACleanup();
#endif
 exit(0);
 }

/* 066 will make permissions -rw------- This is the default */
/* 006 will make permissions -rw-rw---- Use this is you need the group
   to have the same permissions as the user. i.e. on talker.com */
#if defined(HAVE_UMASK)
umask(066);
#endif

do_timeset(TZONE);

printf("Checking abbreviation count..\n");
abbrcount();

/* read system data */
printf("Reading area data from dir. %s ...\n",datadir);
read_init_data(0);

/* read exempt users */
printf("Reading user exempt data from file  %s ...\n",EXEMFILE);
read_exem_data();

/* read banned names */
printf("Reading banned user names from file  %s ...\n",NBANFILE);
read_nban_data();

/* Initialize functions */
init_user_struct();
init_area_struct();
init_misc_struct();

/* Clear out login-limiting struct */
if (LOGIN_LIMITING) check_connlist_entries(-1);

/* see if there's a colon in MAILPROG, if so, they want to use a SMTP gateway */
atpos=strchr(MAILPROG,':');
if (atpos) {
	/* yeah, there's a colon */
	atpos++; /* move past the : */
	if ((strlen(MAILPROG)-((MAILPROG+strlen(MAILPROG)-atpos)+1)) > (sizeof(mailgateway_ip)-1)) {
		printf("\nBad length for SMTP gateway ip in MAILPROG variable. Size: %d Max: %d\n\n",
		(int)strlen(MAILPROG)-((MAILPROG+strlen(MAILPROG)-atpos)+1),(int)sizeof(mailgateway_ip)-1);
		exit(0);
	}
	if ((MAILPROG+strlen(MAILPROG)-atpos) > 5) {
		printf("\nBad length for SMTP gateway port in MAILPROG variable. Size: %d Max: %d\n\n",
		MAILPROG+strlen(MAILPROG)-atpos,5);
		exit(0);
	}
	midcpy(MAILPROG,mess, strlen(MAILPROG)-(MAILPROG+strlen(MAILPROG)-atpos), 
		strlen(MAILPROG));
	mailgateway_port=atoi(mess);
	midcpy(MAILPROG, mailgateway_ip, 0, 
		strlen(MAILPROG)-((MAILPROG+strlen(MAILPROG)-atpos)+2));
	if (!(*mailgateway_ip) || mailgateway_port<=0) {
		printf("\nBad format for SMTP gateway (host:port) in MAILPROG variable!\n\n");
		exit(0);
	}

	printf("Using SMTP gateway: %s on port %d\n",mailgateway_ip,mailgateway_port);
}

/* Reset meter variables */
tells         = 0;
commands      = 1;
says          = 0;
says_running  = 0;
tells_running = 0;
commands_running = 0;

puts("Resetting login numbers...");
system_stats.quota              = MAX_NEW_PER_DAY;
system_stats.logins_today       = 0;
system_stats.logins_since_start = 0;
system_stats.new_since_start    = 0;
system_stats.new_users_today    = 0;
system_stats.tot_users          = 0;
system_stats.tot_expired        = 0;
system_stats.cache_hits         = 0;
system_stats.cache_misses       = 0;

puts("Checking for out of date messages...");
check_mess(1);

puts("Counting users in user directory...");
check_total_users(1);

puts("Updating staff list...");
do_stafflist();

puts("Counting messages...");
messcount();

/* Clear fights */
reset_chal(0,"");

/* If there are somehow any emails in the SMTP active queue */
/* move them back to the hold queue for re-sending          */
requeue_smtp(-1);

#if defined(WINDOWS)
  /* Get Windows version */
myinfo.dwOSVersionInfoSize=sizeof(OSVERSIONINFO);
GetVersionEx(&myinfo);
sprintf(thisos,"Windows v%d.%d",myinfo.dwMajorVersion,myinfo.dwMinorVersion);
#else
  /* not windows, unix-like, cygwin, possibly mac */
# if defined(HAVE_UNAME)
	uname(&uname_info);
	strcpy(thisos,uname_info.sysname);
	strcat(thisos," ");
	strcat(thisos,uname_info.release);
# else
	strcpy(thisos,"Unknown");
# endif
#endif

if (!check_for_file(REBOOTFILE)) restarting=0;
else restarting=1;

if (restarting) read_rebootdb();

/*----------------------------*/
/* dissociate from tty device */
/*----------------------------*/
if (!debug) {

#if !defined(WINDOWS)
/* what about mac?? */

   /*-------------------------------------------------------*/
   /* Fork the process away from the foreground space       */
   /*-------------------------------------------------------*/  
   switch(fork()) 
      {
        case -1:    write_log(BOOTLOG,YESTIME,"FORK: FORK 1 FAILED! %s\n",get_error()); 
	            exit(1);
/*
#if defined(FREEBSD_SYS) || defined(NETBSD_SYS) || defined(OPENBSD_SYS) || defined(__OSF__) || defined(__osf__) || defined(__bsdi__)
*/
#if defined(HAVE_SETPGRP_ARGS)
        case 0:     setpgrp(0,0);
#else
        case 0:     setpgrp();
#endif
		    break;
        default:    sleep(1);
                    exit(0);  /* kill parent */
      }

   /*-------------------------------------------------------*/
   /* Fork the process away from the tty terminal           */
   /* We don't do this on cygwin because it doesn't work!   */
   /*-------------------------------------------------------*/  
   switch(fork()) 
      {
        case -1:    write_log(BOOTLOG,YESTIME,"FORK: FORK 2 FAILED! %s\n",get_error()); 
	            exit(2);
        case 0:     break;  /* child becomes server */
        default:    sleep(1);
                    exit(0);  /* kill parent */
      }

	/*------------------------------------------------*/
	/* set up alarm, signals and signal handlers      */
	/*------------------------------------------------*/
	reset_alarm();

#else

/* Windows thread startup */
/* Start timer thread */
hThread = CreateThread(
        NULL, /* no security attributes */
        0,    /* use default stack size */
        (LPTHREAD_START_ROUTINE) alarm_thread, /* thread function */
        &dwThrdParam, /* argument to thread function   */
        0,            /* use default creation flags    */
        &dwThreadId); /* returns the thread identifier */

/* Check the return value for success. */
if (hThread == NULL) {
   WSACleanup();
   printf("Can't create timer thread! Exiting!\n");
   exit(7);
   }

#endif
} /* !debug */

/* We have to come out of the check to make the */
/* sockets, no matter if we fork of not */
/*---------------------*/
/* Initialize sockets  */
/*---------------------*/
if (!restarting) printf("Initializing sockets...\n");
make_sockets();

/* We can't FD_ISSET on sockets set to -1 or program will segfault */ 
for (i=0;i<4;++i) {
 if (listen_sock[i]==-1) listen_sock[i]=0;
}

if (!debug) {
puts("** System running **");

   /*-------------------------------------------------*/
   /* Redirect stdin, stdout, and stderr to /dev/null */
   /*-------------------------------------------------*/
    fd = open("/dev/null",O_RDWR);
    if (fd < 0) {
      perror("Unable to open /dev/null");
      exit (-1);
      }

    CLOSE(0);
    CLOSE(1);
    CLOSE(2);

    if (fd != 0)
    {
     dup2(fd,0);
     CLOSE(fd);
    }

    dup2(0,1);
    dup2(0,2);

} /* end of !debug */
else puts("** System running **");

/* Log startup */
sysud(1,0);

/* Get local hostname */
if (strlen(DEF_HOSTNAME)) {
	strcpy(thishost,DEF_HOSTNAME);
}
else {
#if defined(HAVE_UNAME)
    strcpy(thishost,uname_info.nodename);
#elif defined(HAVE_GETHOSTNAME)
 if (gethostname(thishost,100) != 0) {
    printf("\n   Cannot get local hostname!\n");
    strcpy(thishost,"");
   }
#else
    strcpy(thishost,"");
#endif
}

if (!strstr(thishost,".")) {
/* if we do not already have a domain in the hostname */
mess[0]=0;
/* i'd like to get the domain from uname_info, but it seems */
/* a lot of OSes don't define it in the structure. I have only */
/* seen it in Linux so far, and even that is conditional */
#if defined(HAVE_GETDOMAINNAME)
if (!getdomainname(mess,80)) {
#else
if (TRUE==FALSE) {
#endif
  if (strcmp(mess,"(none)") && strlen(mess)) {
  /* we found a domain name */
   /* we only have a hostname, add the domain we found */
    if (strlen(thishost))
     strcat(thishost,".");
    strcat(thishost,mess);
    if (thishost[strlen(thishost)-1]=='.')
	midcpy(thishost,thishost,0,strlen(thishost)-2);
  }
  else {
  /* we didn't find a domain */
   /* we only have a hostname, add the default domain from constants.h */
   if (strlen(thishost))
    strcat(thishost,".");
   strcat(thishost,DEF_DOMAIN);
  }
} /* end of if getdomainname */
else {
   /* getdomainname failed, put the default on the end */
   if (strlen(thishost))
    strcat(thishost,".");
   strcat(thishost,DEF_DOMAIN);
}

mess[0]=0;
} /* !strstr */

do_tracking(1, NULL);

/* Initalize signal handlers                           */  
init_signals();

/*-----------------------------------------------------*/
/* clear the btell and shout buffers                   */
/*-----------------------------------------------------*/

cbtbuff();
cshbuff();

if (restarting) {
/* CYGRESOLVE */
/* ok, we are in a soft boot, does the admin want the clip-on in this */
/* boot if not, try and kill the clip-on */
if (resolve_names!=2 && resolve_names!=3) kill_resolver_clipon();
else {
/* the clipon wasn't on, but now we do want it on */
if (FROM_CLIENT_READ==-1) init_resolver_clipon();
else send_resolver_request(-2,NULL,NULL);
}

check_sockets();

restarting=0;
for (i=0; i<MAX_USERS; ++i) {
if (ustr[i].sock==-1 || ustr[i].logging_in) continue;
if (SHOW_SREBOOT==2) write_str(i," ^*** Soft-reboot complete!  Carry on ***^");
}
i=0;
if (SHOW_SREBOOT==1) {
sprintf(mess,"%s *** Soft-reboot complete!  Carry on ***",STAFF_PREFIX);
writeall_str(mess, WIZ_ONLY, -1, 0, -1, BOLD, WIZT, 0);
}
write_log(BOOTLOG,YESTIME,"SOFT-REBOOT Complete!\n");
}
else {
/* CYGRESOLVE */
/* Normal boot, start the resolver clip-on if we want it */
if (resolve_names==2 || resolve_names==3) init_resolver_clipon();
}

/*--------------------------*/
/**** Main program loop *****/
/*--------------------------*/

LOOP_FOREVER
   {
	noprompt=0;  
	signl=0;
	
	/*-----------------------------------------------------------*/
	/* Set up bitmap readmask and writemask by clearing them out */
	/*-----------------------------------------------------------*/
	FD_ZERO(&readmask);
	FD_ZERO(&writemask);
	nfds = 0;

	/*----------------------------------*/
	/* Set up timeout for select()      */
	/* what is really a good number???? */
	/*----------------------------------*/
	sel_timeout.tv_sec  = 0;   /* number of seconds      */
	sel_timeout.tv_usec = 0;   /* number of microseconds */

        /* Is there a who port socket in use? */
        /* If so, add it to our mask          */
	for (user = 0; user < MAX_WHO_CONNECTS; ++user)
	  {
            if (whoport[user].sock != -1) {
             FD_SET(whoport[user].sock,&readmask);
	     if (whoport[user].sock >= nfds)
	     nfds = whoport[user].sock + 1;
	    }
	  }
	user=0;

        /* Is there a www port socket in use? */
        /* If so, add it to our mask          */
	for (user = 0; user < MAX_WWW_CONNECTS; ++user)
	  {
            if (wwwport[user].sock != -1) {
             FD_SET(wwwport[user].sock,&readmask);
             if (wwwport[user].alloced_size)
             FD_SET(wwwport[user].sock,&writemask);
	     if (wwwport[user].sock >= nfds)
	     nfds = wwwport[user].sock + 1;
	    }
	  }
	user=0;

        /* Is there a misc socket in use?     */
        /* If so, add it to our mask          */
	for (user = 0; user < MAX_MISC_CONNECTS; ++user)
	  {
            if (miscconn[user].sock != -1) {
#if defined(MISC_DEBUG)
		write_log(DEBUGLOG,YESTIME,"Adding miscconn %d:%d to fdset\n",user,miscconn[user].sock);
#endif
             FD_SET(miscconn[user].sock,&readmask);
             FD_SET(miscconn[user].sock,&writemask);
	     if (miscconn[user].sock >= nfds)
	     nfds = miscconn[user].sock + 1;
	    }
	  }
	user=0;

	for (user = 0; user < MAX_USERS; ++user) 
	  {
	   /* A user slot is defined, but there is no */
	   /* user filling the slot yet               */
           /* if (ustr[user].area == -1 && !ustr[user].logging_in) */
           /* continue; */

	   /* If a connection exists, add the online users socket */
	   /* to the read mask set                                */
	    if (ustr[user].sock != -1) {
	     /* bytes_read == -1 is user suspension by flood protection */
	     if (ustr[user].bytes_read != -1)
             FD_SET(ustr[user].sock,&readmask);
             if (ustr[user].alloced_size)
             FD_SET(ustr[user].sock,&writemask);
	     if (ustr[user].sock >= nfds)
	     nfds = ustr[user].sock + 1;
            }
          }

       for (i=0;i<4;++i) {
	/* 1 = wiz 2 = who 3 = www */
	if (i==1) { if (WIZ_OFFSET==0) continue; }
	else if (i==2) { if (WHO_OFFSET==0) continue; }
	else if (i==3) { if (WWW_OFFSET==0) continue; }
	if (!listen_sock[i]) continue;
	/* Add the listening sockets to the read mask set */
	FD_SET(listen_sock[i],&readmask);
	if (listen_sock[i] >= nfds)
	nfds = listen_sock[i] + 1;
        }

	/* CYGRESOLVE */
if (resolve_names==2 || resolve_names==3) {
	if (FROM_CLIENT_READ != -1) {
		FD_SET(FROM_CLIENT_READ, &readmask);
		if (FROM_CLIENT_READ >= nfds)
		nfds = FROM_CLIENT_READ + 1;
		}
}

	/*--------------------------------------------------------------*/
	/* Wait for input on the ports                                  */
	/*                                                              */
        /* We declare the args as (void*) because HP/UX for example has */
        /* a select() prototype declaring the args as (int*) rather     */
        /* than (fd_set*), POSIX or no POSIX. By casting to (void*) we  */
        /* avoid compiletime warnings about these args                  */
	/*--------------------------------------------------------------*/

	if (select(nfds, (void *) &readmask, (void *) &writemask, (void *) 0, 0) == SOCKET_ERROR) {
	 if (errno != EINTR) {
	 	write_log(ERRLOG,YESTIME,"SELECT: Select failed with error %s on pid %u\n",get_error(),(unsigned int)getpid());
		if (REBOOT_A_CRASH==1)
                 treboot=1;
		shutdown_error(log_error(9));
	   }
	 else continue;
	}

	if (signl) continue; 

        port = ' ';

	/*---------------------------------------*/
	/* Check for connection to who socket    */
	/*---------------------------------------*/
	if (WHO_OFFSET != 0) {
	if (FD_ISSET(listen_sock[2],&readmask)) 
	  {
           size=sizeof(acc_addr);
           if ( ( as = accept(listen_sock[2], (struct sockaddr *)&acc_addr, &size) ) == INVALID_SOCKET )
             {
	       /* we can not open a new file descriptor */
	       write_log(ERRLOG,YESTIME,"ACCEPT: ERROR creating who socket %s\n",get_error());
	      if (errno_ok(errno)) {
                FD_CLR(listen_sock[2],&readmask);
		continue;
		}
	      else {
		if (REBOOT_A_CRASH==1)
                 treboot=1;
                shutdown_error(log_error(1));
		}
             }
            else
             {
              port='3';
              /* Set socket to non-blocking */
/*
#if defined(WINDOWS)
	      if (ioctlsocket(as, FIONBIO, &arg) == -1) {
#else
              if (fcntl(as,F_SETFL,NBLOCK_CMD)== -1) {
#endif
*/
              if (MY_FCNTL(as,MY_F_SETFL,NBLOCK_CMD)==SOCKET_ERROR) {
	       write_log(ERRLOG,YESTIME,"BLOCK: ERROR setting who socket to non-blocking %s\n",get_error());
		if (REBOOT_A_CRASH==1)
                 treboot=1;
               shutdown_error(log_error(2));
              }

             /*---------------------------------*/
	     /* Get who port connect and log it */
	     /*---------------------------------*/

              ip_address = acc_addr.sin_addr.s_addr;
              if ( (new_user = find_free_slot(port) ) == -1 ) {
		 SHUTDOWN(as, 2);
	         while (CLOSE(as) == -1 && errno == EINTR)
			; /* empty while */
                 continue;
                }

              whoport[new_user].sock=as;

	      resolve_add(new_user,ip_address,RESOLVE_TO_OTHER,RESOLVE_WHO);

              log_misc_connect(new_user,1);

	      if (check_misc_restrict(whoport[new_user].sock,whoport[new_user].site,whoport[new_user].net_name) == 1) {
		write_log(BANLOG,YESTIME,"WHO : IN : Connection attempt, RESTRICTed site %s:%s:sck#%d:slt#%d\n",
		whoport[new_user].site,whoport[new_user].net_name,whoport[new_user].sock,new_user);
		free_sock(new_user,port);
		continue;
                }

              if (who_access) {
		/* Send out who list to person or remote who connected to who port */
		external_who(whoport[new_user].sock);
		}
              else {
	       strcpy(mess,WHO_CLOSED);
	       write_it(whoport[new_user].sock,mess);
               }
              free_sock(new_user,port);
              continue;
             } /* end of accept else */
           } /* end of FD_ISSET */
	} /* end of WHO_OFFSET if */

	/*--------------------------------------------*/
	/* Check for connection to mini www socket    */
	/*--------------------------------------------*/
	if (WWW_OFFSET != 0) {
	if (FD_ISSET(listen_sock[3], &readmask)) 
	  {
           size=sizeof(acc_addr);
           if ( ( as = accept(listen_sock[3], (struct sockaddr *)&acc_addr, &size) ) == INVALID_SOCKET )
             {
	       /* we can not open a new file descriptor */
	       write_log(ERRLOG,YESTIME,"ACCEPT: ERROR creating www socket %s\n",get_error());
	      if (errno_ok(errno)) {
                FD_CLR(listen_sock[3],&readmask);
		continue;
		}
	      else {
		if (REBOOT_A_CRASH==1)
                 treboot=1;
                shutdown_error(log_error(3));
		}
             }
            else
             {
              port='4';

              /* Set socket to non-blocking */
/*
#if defined(WINDOWS)
              if (ioctlsocket(as, FIONBIO, &arg) == -1) {
#else
              if (fcntl(as,F_SETFL,NBLOCK_CMD)== -1) {
#endif
*/
              if (MY_FCNTL(as,MY_F_SETFL,NBLOCK_CMD)==SOCKET_ERROR) {
	       write_log(ERRLOG,YESTIME,"BLOCK: ERROR setting www socket to non-blocking %s\n",get_error());
		if (REBOOT_A_CRASH==1)
                 treboot=1;
               shutdown_error(log_error(4));
              }

             /*---------------------------------*/
	     /* Get www port connect and log it */
	     /*---------------------------------*/

              ip_address = acc_addr.sin_addr.s_addr;
              if ( (new_user = find_free_slot(port) ) == -1 ) {
		 SHUTDOWN(as, 2);
	         while (CLOSE(as) == -1 && errno == EINTR)
			; /* empty while */
		 write_log(ERRLOG,YESTIME,"WWW: Web port connections maxed out!\n");
                 continue;
                }

              wwwport[new_user].sock=as;

	      resolve_add(new_user,ip_address,RESOLVE_TO_OTHER,RESOLVE_WWW);

              log_misc_connect(new_user,2);

	      if (check_misc_restrict(wwwport[new_user].sock,wwwport[new_user].site,wwwport[new_user].net_name) == 1) {
		write_log(BANLOG,YESTIME,"WWW : IN : Connection attempt, RESTRICTed site %s:%s:sck#%d:slt#%d\n",
		wwwport[new_user].site,wwwport[new_user].net_name,wwwport[new_user].sock,new_user);
		free_sock(new_user,port);
		continue;
                }

              if (!www_access) {
	       strcpy(mess,WWW_CLOSED);
	       write_it(wwwport[new_user].sock,mess);
               free_sock(new_user,port);
               continue;
               }
             } /* end of accept else */
           } /* end of FD_ISSET */
	} /* end of WWW_OFFSET if */

	/*---------------------------------------*/
	/* Check for connection to listen socket */
	/*---------------------------------------*/

	if (FD_ISSET(listen_sock[0],&readmask) || FD_ISSET(listen_sock[1],&readmask)) 
	  {
           if ( FD_ISSET(listen_sock[0],&readmask))
             {
               size=sizeof(acc_addr);
               if ( ( as = accept(listen_sock[0], (struct sockaddr *)&acc_addr,&size) ) == INVALID_SOCKET )
                 {
	       /* we can not open a new file descriptor */
	       write_log(ERRLOG,YESTIME,"ACCEPT: ERROR creating user socket %s\n",get_error());
	          if (errno_ok(errno)) {
                   FD_CLR(listen_sock[0],&readmask);
		   continue;
		   }
		  else {
		   if (REBOOT_A_CRASH==1)
                    treboot=1;
                   shutdown_error(log_error(5));
		   }
                }
              port = '1';
            }

	if (WIZ_OFFSET != 0) {        
           if ( FD_ISSET(listen_sock[1],&readmask))
             {
               size=sizeof(acc_addr);
               if ( ( as = accept(listen_sock[1], (struct sockaddr *)&acc_addr,&size) ) == INVALID_SOCKET )
                 {
	       /* we can not open a new file descriptor */
	       write_log(ERRLOG,YESTIME,"ACCEPT: ERROR creating wiz socket %s\n",get_error());
	          if (errno_ok(errno)) {
                   FD_CLR(listen_sock[1],&readmask);
		   continue;
		   }
		  else {
		   if (REBOOT_A_CRASH==1)
                    treboot=1;
                   shutdown_error(log_error(6));
		   }
                 }
               port = '2';
             }
	} /* end of WIZ_OFFSET if */
             
              /* Set socket to non-blocking */
/*
#if defined(WINDOWS)
              if (ioctlsocket(as, FIONBIO, &arg) == -1) {
#else
              if (fcntl(as,F_SETFL,NBLOCK_CMD)== -1) {
#endif
*/
              if (MY_FCNTL(as,MY_F_SETFL,NBLOCK_CMD)==SOCKET_ERROR) {
	       write_log(ERRLOG,YESTIME,"BLOCK: ERROR setting user/wiz socket to non-blocking %s\n",get_error());
		if (REBOOT_A_CRASH==1)
                 treboot=1;
               shutdown_error(log_error(7));
              }

	   
	   /*----------------------------------------------*/
	   /* No system access is allowed                  */
	   /*----------------------------------------------*/

           if (port == '1') {
	   if (!sys_access) 
	     {
	       strcpy(mess,SYS_CLOSED);
	       S_WRITE(as,mess,strlen(mess));
		 SHUTDOWN(as, 2);
	         while (CLOSE(as) == -1 && errno == EINTR)
			; /* empty while */
	       continue;
	     }
           }
          else if (port == '2') {
	   if (!wiz_access) 
	     {
	       strcpy(mess,WIZ_CLOSED);
	       S_WRITE(as,mess,strlen(mess));
		 SHUTDOWN(as, 2);
	         while (CLOSE(as) == -1 && errno == EINTR)
			; /* empty while */
	       continue;
	     }
           }
			
            if ( (new_user = find_free_slot(port) ) == -1 ) 
              {                      
		sprintf(mess,"%s",SYSFULLFILE);
		cat_to_sock(mess,as);
		 SHUTDOWN(as, 2);
	         while (CLOSE(as) == -1 && errno == EINTR)
			; /* empty while */
		continue;
	      }
	     
	     ustr[new_user].sock           = as;		
	     ustr[new_user].last_input     = time(0);
	     ustr[new_user].logging_in     = 3;
             ustr[new_user].attleft        = 3;
	     ustr[new_user].warning_given  = 0;
	     ustr[new_user].afk            = 0;
	     ustr[new_user].lockafk        = 0;
	     ustr[new_user].attach_port    = port;

             /*------------------------*/
	     /* get internet site info */
	     /*------------------------*/

  ustr[new_user].site[0]=0;
  ustr[new_user].net_name[0]=0;

  ip_address = acc_addr.sin_addr.s_addr;

  /* Get ip address of new user..we do this no matter what */
  resolve_add(new_user,ip_address,RESOLVE_TO_IP,RESOLVE_USER);

	     /*---------------------------------*/
	     /* Check for totally restricted ip */
	     /*---------------------------------*/
	     
             if (check_restriction(new_user, ANY, THEIR_IP) == 1) 
               {
                write_log(BANLOG,YESTIME,"MAIN: Connection attempt, RESTRICTed IP %s:%s:sck#%d:slt#%d\n",ustr[new_user].site,ustr[new_user].net_name,ustr[new_user].sock,new_user);
                user_quit(new_user,1);
		mess[0]=0;
                continue;
               }     

		if (LOGIN_LIMITING) {
			if (check_connlist(new_user) == 1) {
				write_log(BANLOG,YESTIME,"MAIN: Banned user for login hammering! %s:%s:sck#%d:slt#%d\n",ustr[new_user].site,ustr[new_user].net_name,ustr[new_user].sock,new_user);
				user_quit(new_user,1);
				mess[0]=0;
				continue;
			}
		}

/* Telopt negotiation for terminal type */
telnet_neg_ttype(new_user, 0);

             if (checked)
              write_str(new_user,"System in check or in progress of booting, please wait..");

/*--------------------------------------------------*/
/* Randomize motds, by the NUM_MOTDS variable       */
/*  First motd is motd0, then motd1, and so on      */
/* Ditto on the wiz port with wizmotd0 and wizmotd1 */
/*--------------------------------------------------*/

     imotd = rand() % NUM_MOTDS;

	if (ustr[new_user].attach_port=='1')
		sprintf(filename, "%s/motd%d", LIBDIR, imotd);
	else if (ustr[new_user].attach_port=='2')
		sprintf(filename,"%s/wizmotd%d", LIBDIR, imotd);

             cat( filename, new_user, -1);
             sprintf(mess,TOTAL_USERS_MESS,system_stats.tot_users);
             write_str(new_user,mess);

  /* If global to resolve address, resolve address to hostname */
  if (resolve_names >= 1) resolve_add(new_user,ip_address,RESOLVE_TO_NAME,RESOLVE_USER);
  else strcpy(ustr[new_user].net_name,SYS_RES_OFF);

  /* Send connection info to staff who ask to get it */
  sprintf(mess," [INCOMING LOGIN from %s (%s) on line %d (%c)]",ustr[new_user].net_name,ustr[new_user].site,ustr[new_user].sock,port);
  writeall_str(mess, -2, new_user, 0, new_user, BOLD, NONE, 0);
  mess[0]=0;

	     /*-------------------------------*/
	     /* Check for restricted hostname */
	     /*-------------------------------*/
	     
             if (check_restriction(new_user, ANY, THEIR_HOST) == 1) 
               {
                write_log(BANLOG,YESTIME,"MAIN: Connection attempt, RESTRICTed host %s:%s:sck#%d:slt#%d\n",ustr[new_user].site,ustr[new_user].net_name,ustr[new_user].sock,new_user);
                user_quit(new_user,1);
		mess[0]=0;
                continue;
               }     

	     telnet_echo_on(new_user);
	     write_str(new_user,"");

	     /* Write out login prompt */
             write_str_nr(new_user,SYS_LOGIN);
	     /* Write out the generic terminator of GA */
	     /* until we find out if we can use EORs */
	     /* telnet_write_eor(new_user); */

	     /* Telopt negotiation for prompt terminator    */
	     /* ask if we can send EORs for clients like TF */
	     telnet_ask_eor(new_user);
 
	   } /** end of last FD_ISSET **/

	/* CYGRESOLVE */
if (resolve_names==2 || resolve_names==3) {
	if (FROM_CLIENT_READ != -1)
		if (FD_ISSET(FROM_CLIENT_READ, &readmask)) {
#if defined(RESOLVER_DEBUG)
			write_log(RESOLVELOG,YESTIME,"Ready to read result from resolver\n");
#endif
			read_resolver_reply();
			}
}

	/** cycle through user list, getting input and taking actions	**/
	/** based on their status					**/
	for (user=0; user < MAX_USERS; ++user) {

		retval=get_input(user,ustr[user].attach_port,0);
		if (ustr[user].sock==-1) continue;
		strcpy(inpstr,t_inpstr);
		if (retval < 0) {
		switch(retval) {
		case -1: /* socket doesnt exist - just in case */ break;
		case -6: /* buffer overload */ break;
		case -7: /* fatal read error */ break;
		default:
		if (FD_ISSET(ustr[user].sock,&writemask)) {
			if (!queue_flush(user)) {
#if defined(QFLUSH_DEBUG)
			write_log(DEBUGLOG,YESTIME,"Calling user_quit from !queue_flush retval %d\n",retval);
#endif
			user_quit(user,0);
			}
		} /* end of FD_ISSET */
		break;
		} /* end of switch */
			continue;
		} /* end of if retval */
		else {
		if (FD_ISSET(ustr[user].sock,&writemask)) {
			if (!queue_flush(user)) {
#if defined(QFLUSH_DEBUG)
			write_log(DEBUGLOG,YESTIME,"Calling user_quit from !queue_flush2\n");
#endif
			user_quit(user,0);
			}
		} /* end of FD_ISSET */
		} /* end of else */

		ustr[user].last_input    = time(0);  /* ie now        */
		ustr[user].warning_given = 0;        /* reset warning */
		
		/*---------------------------------*/
		/* user wakes up from afk or bafk  */
		/*---------------------------------*/
		
		if (ustr[user].afk)
		  {
                    if (ustr[user].lockafk) {
                        strtolower(inpstr);
                        st_crypt(inpstr);
                        if (strcmp(ustr[user].password,inpstr)) {
                          telnet_echo_on(user);
                          write_str(user,"Password incorrect.");
                          write_str_nr(user,"Enter password: ");
                          telnet_echo_off(user);
			  telnet_write_eor(user);
                          continue;
                          }
                        else { telnet_echo_on(user); }
                       }
		    if (!ustr[user].vis)
		     sprintf(mess,AFK_BACK,INVIS_ACTION_LABEL);
		    else
		     sprintf(mess,AFK_BACK,ustr[user].say_name);
                    writeall_str(mess,1,user,0,user,NORM,AFK_TYPE,0);
		    check_alert(user,1);
                    if (ustr[user].lockafk==0) {
                      if (ustr[user].afk==1)
                       sprintf(mess,AFK_BACK3, "AFK");
                      else if (ustr[user].afk==2)
                       sprintf(mess,AFK_BACK3, "BAFK");
                      write_str(user,mess);
                     }
                    ustr[user].afk = 0;
                    ustr[user].afkmsg[0] = 0;
                    if (ustr[user].lockafk) {
                      write_str(user,AFK_BACK2);
                      ustr[user].lockafk = 0;
                      continue;
                      }
		  }
		  
		/*-------------------------------*/
		/* see if user is logging in     */
		/*-------------------------------*/
		if (ustr[user].logging_in) 
		  { 
		   my_login(user,inpstr);  
		   continue; 
		  }

		/*-------------------------------*/
		/* see if user is reading a file */
		/*-------------------------------*/
		if (ustr[user].file_posn) 
		  {
		   if (inpstr[0] == 'q' || inpstr[0] == 'Q') 
		     {
			/* Quit reading */
		      ustr[user].file_posn=0;  
		      ustr[user].line_count=0; 
		      ustr[user].number_lines=0; 
		      ustr[user].numbering=0;
			if (ustr[user].log_stage) {
			/* User ended the log read, reset ourselves */
				ustr[user].log_stage=0;
				ustr[user].temp_buffer[0]=0;
			}
                      continue;
		     }
		   else if (inpstr[0] == 'p' || inpstr[0] == 'P')
		     {
			/* Display previous page */
			/* we should count bytes from 2 screens back then subtract */
			/* that from the current file position */
			i = ustr[user].line_count - (ustr[user].rows*2);
			if (i<0) i=0;
			if (i) {
			sprintf(mess,"%d - %d bytes for rows %d to %d (start %d)",
ustr[user].file_posn,file_count_bytes(ustr[user].page_file, i, ustr[user].line_count),
i, ustr[user].line_count,
ustr[user].file_posn-file_count_bytes(ustr[user].page_file, i, ustr[user].line_count));
			/* write_str(user,mess); */
			ustr[user].file_posn=ustr[user].file_posn-
			 file_count_bytes(ustr[user].page_file, i, ustr[user].line_count);
			ustr[user].line_count=i;
			ustr[user].numbering=i;
			}
			else {
			ustr[user].file_posn=0;
			ustr[user].line_count=0;
			ustr[user].numbering=0;
			}
		     }
		   else if (inpstr[0] == 'r' || inpstr[0] == 'R')
		     {
			/* Re-display current page */
			/* we should count bytes from 1 screen back then subtract */
			/* that from the current file position */
			i = ustr[user].line_count - ustr[user].rows;
			if (i<0) i=0;
			if (i) {
			sprintf(mess,"%d - %d bytes for rows %d to %d (start %d)",
ustr[user].file_posn,file_count_bytes(ustr[user].page_file, i, ustr[user].line_count),
i, ustr[user].line_count,
ustr[user].file_posn-file_count_bytes(ustr[user].page_file, i, ustr[user].line_count));
			/* write_str(user,mess); */
			ustr[user].file_posn=ustr[user].file_posn-
			 file_count_bytes(ustr[user].page_file, i, ustr[user].line_count);
			ustr[user].line_count=i;
			ustr[user].numbering=i;
			}
			else {
			ustr[user].file_posn=0;
			ustr[user].line_count=0;
			ustr[user].numbering=0;
			}
		     }
		   else if (inpstr[0] == 'f' || inpstr[0] == 'F')
		     {
			ustr[user].file_posn=0;
			ustr[user].line_count=0;
			ustr[user].numbering=0;
		     }
		   else if (inpstr[0] == 'l' || inpstr[0] == 'L')
		     {
			ustr[user].file_posn=
			 file_count_bytes(ustr[user].page_file, 0, 0) -
			 file_count_bytes(ustr[user].page_file, ustr[user].rows, 0);
			ustr[user].line_count=
			file_count_lines(ustr[user].page_file)-ustr[user].rows;
			ustr[user].numbering=
			file_count_lines(ustr[user].page_file)-ustr[user].rows;
		     }

		   cat(ustr[user].page_file,user, ustr[user].number_lines); 
			/* If we've hit the end of the file and are in	*/
			/* an ALL read of logfiles, keep going		*/
			if (ustr[user].file_posn==0 && ustr[user].log_stage) {
			readlog(user,"");
			}
		   continue;
		  }

              /*---------------------------------------*/
              /*  See if user is entering a profile    */
              /*---------------------------------------*/
              if (ustr[user].pro_enter) {
               if (ustr[user].pro_enter > PRO_LINES) strtolower(inpstr);
               set_profile(user,inpstr);  continue;
               }

              /*---------------------------------------*/
              /*  See if user is adding a talker       */
              /*---------------------------------------*/
              if (ustr[user].t_ent) {
                 talker(user,inpstr);  continue;
                 }

              /*---------------------------------------*/
              /*  See if user is entering a vote topic */
              /*---------------------------------------*/
              if (ustr[user].vote_enter) {
               if (ustr[user].vote_enter > VOTE_LINES) strtolower(inpstr);
               enter_votedesc(user,inpstr);  continue;
               }

              /*---------------------------------------*/
              /*  See if user is writing a room desc   */
              /*---------------------------------------*/
              if (ustr[user].roomd_enter) {
               if (ustr[user].roomd_enter > ROOM_DESC_LINES) strtolower(inpstr);
               descroom(user,inpstr);  continue;  
               }

              /* make sure users can't send bot commands */
               if (strstr(inpstr,"+++++") || strstr(inpstr,our_delimiters)) continue;

	      /* did the user enter a macro of theirs? */
	       if (check_macro(user,inpstr) == -1) continue;

               /*----------------------------------------------*/
               /* if user did nothing, return                  */
               /*----------------------------------------------*/
               
	       if (!inpstr[0] || nospeech(inpstr)) continue; 

	       /*------------------------*/
	       /* deal with any commands */
	       /*------------------------*/
	       com_num=get_com_num(user,inpstr);
		
		if ((com_num == -1) && 
                    (inpstr[0] == '.' || !strcmp(ustr[user].name,BOT_ID))) 
		  {
		   write_str(user,SYNTAX_ERROR);
		   continue;
		  }
		  
		if (com_num != -1) 
		  {
		   last_user=user;
                   if ((!strcmp(ustr[user].name,BOT_ID) || !strcmp(ustr[user].name,ROOT_ID)) && inpstr[0]=='_')
                    exec_bot_com(com_num,user,inpstr);
                   else
		    exec_com(com_num,user,inpstr);
			
		   last_user= -1;
		   continue;
		  }


		/*--------------------------------------------*/
		/* see if input is answer to clear_mail query */
		/*--------------------------------------------*/
		if (ustr[user].clrmail==user && inpstr[0]!='y') 
		  {
		   ustr[user].clrmail= -1; 
		   continue;
		  }
		  
		if (ustr[user].clrmail==user && inpstr[0]=='y') 
		  {
                     if (delete_sent) {
                        clear_sent(user, "");
                        ustr[user].clrmail= -1;
                        continue;
                        }
                     else {
		        clear_mail(user, "");
		        ustr[user].clrmail= -1;
		        continue;
                        }
		  } 

		/*------------------------------------------*/
		/* see if input is answer to shutdown query */
		/*------------------------------------------*/
		
		if (shutd==user && inpstr[0]!='y') 
		  {
		    shutd= -1;  
		    continue;
		   }
		   
		if (shutd==user && inpstr[0]=='y') 
		  shutdown_d(user,"");

		/*-----------------------------------------------------*/
		/* send speech to speaker & everyone else in same area */
		/*-----------------------------------------------------*/
		commands++;
                say(user, inpstr, 0);
	      } /* end of MAIN USER FOR */

	user=0;

	/** cycle through web server users **/
	for (user=0; user < MAX_WWW_CONNECTS; ++user) {

		retval=get_input(user,'4',1);
/*
write_log(DEBUGLOG,YESTIME,"after get_input\n");
write_log(DEBUGLOG,YESTIME,"Pos: %d Retval: %d sock: %d wwwsock: %d input: \"%s\"\n",user,retval,ustr[user].sock,wwwport[user].sock,t_inpstr);
*/
		if (wwwport[user].sock==-1) continue;
		strcpy(inpstr,t_inpstr);
                if (retval < 0) {
                switch(retval) {
		case -1: /* socket doesnt exist - jsut in case */ break;
		case -6: /* buffer overload */ break;
		case -7: /* fatal read error */ break;
                default:
		if (FD_ISSET(wwwport[user].sock,&writemask)) {
			queue_flush_www(user);
		} /* end of FD_ISSET */
		break;
		} /* end of switch */
		continue;
		} /* end of if retval */
		else {
		if (FD_ISSET(wwwport[user].sock,&writemask)) {
			queue_flush_www(user);
		} /* end of FD_ISSET */
		} /* end of else */

		parse_input(user,inpstr);
		continue;
	   } /* end of web server users for */

	user=0;

	/** cycle through misc connections **/
	for (user=0; user < MAX_MISC_CONNECTS; ++user) {

		retval=get_input(user,'5',2);
#if defined(MISC_DEBUG)
 write_log(DEBUGLOG,YESTIME,"Pos: %d Retval: %d sock: %d miscconnsock: %d input: \"%s\"\n",user,retval,ustr[user].sock,miscconn[user].sock,t_inpstr);
#endif
		if (miscconn[user].sock==-1) continue;
		strcpy(inpstr,t_inpstr);
                if (retval < 0) {
                switch(retval) {
		case -1: /* socket doesnt exist - jsut in case */ break;
		case -6: /* buffer overload */ break;
		case -7: /* fatal read error */ break;
                default:
		break;
		} /* end of switch */
		continue;
		} /* end of if retval */
		else {
		  if (miscconn[user].type==1) {
			write_str(miscconn[user].user,inpstr);
		  }
		  else if (miscconn[user].type==2) {
			midcpy(inpstr,smtpcode,0,2);
			if (miscconn[user].stage==0 && !strcmp(smtpcode,"220")) {
			 /* got SMTP banner */
#if defined(SMTP_DEBUG)
 write_log(DEBUGLOG,YESTIME,"SMTP: Got banner: %s\n",inpstr);
#endif
			 miscconn[user].stage=1;
			 miscconn[user].ready=1;
			} 
			else if (miscconn[user].stage==1) {
				if (!strcmp(smtpcode,"250")) {
				 /* our HELO message was good */
#if defined(SMTP_DEBUG)
 write_log(DEBUGLOG,YESTIME,"SMTP: Got good HELO reply: %s\n",inpstr);
#endif
				 miscconn[user].stage=2;
				 miscconn[user].ready=1;
				}
				else if (!strcmp(smtpcode,"501")) {
				 /* our HELO message was bad */
				 write_log(ERRLOG,YESTIME,"SMTP: OUT: sck#%d:slt#%d:NA:bad HELO:server said \"%s\"\n",
				 miscconn[user].sock,user,inpstr);
				 free_sock(user,'5');
				 continue;
				}
			} /* else if stage 1 */
			else if (miscconn[user].stage==2) {
				if (!strcmp(smtpcode,"250")) {
				 /* our MAIL FROM message was good */
#if defined(SMTP_DEBUG)
 write_log(DEBUGLOG,YESTIME,"SMTP: Got good MAIL FROM reply: %s\n",inpstr);
#endif
				 miscconn[user].stage=3;
				 miscconn[user].ready=1;
				}
				else if (atoi(smtpcode) >= 500) {
				 /* our MAIL FROM message was bad */
				 /* 500 error, 501 syntax, 571 relay */
				 FCLOSE(miscconn[user].fd);
				 write_log(ERRLOG,YESTIME,"SMTP: OUT: sck#%d:slt#%d:%s:bad MAIL FROM:server said \"%s\"\n",
				 miscconn[user].sock,user,miscconn[user].queuename,inpstr);
				 requeue_smtp(user);
				 free_sock(user,'5');
				 continue;
				}
			} /* else if stage 2 */
			else if (miscconn[user].stage==3) {
				if (!strcmp(smtpcode,"250")) {
				 /* our RCPT TO message was good */
#if defined(SMTP_DEBUG)
 write_log(DEBUGLOG,YESTIME,"SMTP: Got good RCPT TO reply: %s\n",inpstr);
#endif
				 miscconn[user].stage=4;
				 miscconn[user].ready=1;
				}
				else if (atoi(smtpcode) >= 500) {
				 /* our RCPT TO message was bad */
				 /* 500 error, 501 syntax, 571 relay */
				 FCLOSE(miscconn[user].fd);
				 write_log(ERRLOG,YESTIME,"SMTP: OUT: sck#%d:slt#%d:%s:bad RCPT TO:server said \"%s\"\n",
				 miscconn[user].sock,user,miscconn[user].queuename,inpstr);
				 requeue_smtp(user);
				 free_sock(user,'5');
				 continue;
				}
			} /* else if stage 3 */
			else if (miscconn[user].stage==4) {
				if (!strcmp(smtpcode,"354")) {
				 /* our DATA message was good */
#if defined(SMTP_DEBUG)
 write_log(DEBUGLOG,YESTIME,"SMTP: Got good DATA reply: %s\n",inpstr);
#endif
				 miscconn[user].stage=5;
				 miscconn[user].ready=1;
				}
				else if (atoi(smtpcode) >= 500) {
				 /* our DATA message was bad */
				 /* 500 error, 501 syntax, 571 relay */
				 FCLOSE(miscconn[user].fd);
				 write_log(ERRLOG,YESTIME,"SMTP: OUT: sck#%d:slt#%d:%s:bad DATA:server said \"%s\"\n",
				 miscconn[user].sock,user,miscconn[user].queuename,inpstr);
				 requeue_smtp(user);
				 free_sock(user,'5');
				 continue;
				}
			} /* else if stage 4 */
			else if (miscconn[user].stage==5) {
				if (!strcmp(smtpcode,"250")) {
				 /* our message body was good */
#if defined(SMTP_DEBUG)
 write_log(DEBUGLOG,YESTIME,"SMTP: Got good BODY reply: %s\n",inpstr);
#endif
				 sprintf(filename,"%s/%s",MAILDIR_SMTP_ACTIVE,miscconn[user].queuename);
				 remove(filename);
				 write_log(SYSTEMLOG,YESTIME,"SMTP: OUT: sck#%d:slt#%d:%s:good BODY:server said \"%s\" - Mail successfully sent\n",
				 miscconn[user].sock,user,miscconn[user].queuename,inpstr);
				 miscconn[user].stage=2;
				 miscconn[user].ready=1;
				 miscconn[user].fd=NULL;
				 miscconn[user].queuename[0]=0;
				}
				else if (atoi(smtpcode) >= 500) {
				 /* our message body was bad */
				 /* 500 error, 501 syntax, 571 relay */
				 /* we dont close the fd here, because writing */
				 /* the body out already closes it */
				 write_log(ERRLOG,YESTIME,"SMTP: OUT: sck#%d:slt#%d:%s:bad BODY:server said \"%s\"\n",
				 miscconn[user].sock,user,miscconn[user].queuename,inpstr);
				 requeue_smtp(user);
				 free_sock(user,'5');
				 continue;
				}
			} /* else if stage 5 */
			else { }
/* write_log(DEBUGLOG,YESTIME,"SMTP: stage check done\n"); */
		  /* check for writability */
		   if (FD_ISSET(miscconn[user].sock,&writemask)) {
/* write_log(DEBUGLOG,YESTIME,"SMTP: sock %d is writeable\n",miscconn[user].sock); */
			if (miscconn[user].ready) {
/* write_log(DEBUGLOG,YESTIME,"SMTP: sock %d is ready\n",miscconn[user].sock); */
				if (miscconn[user].stage==1) {
					/* write out hello - HELO */
					if (write_smtp_data(user,0) == -1) {
					 free_sock(user,'5');
					 continue;
					}
					else {
#if defined(SMTP_DEBUG)
 write_log(DEBUGLOG,YESTIME,"SMTP: Wrote out HELO\n");
#endif
					 miscconn[user].ready=0;
					}
				} /* if stage 1 */
				else if (miscconn[user].stage==2) {
					/* get an email from the queue and put it */
					/* in the active queue for processing     */
					if (queuetoactive_smtp(user) == -1) {
					 free_sock(user,'5');
					 continue;
					}
					else {
					 sprintf(filename,"%s/%s",MAILDIR_SMTP_ACTIVE,miscconn[user].queuename);
					 if (!(miscconn[user].fd=fopen(filename,"r"))) {
					  write_log(ERRLOG,YESTIME,"SMTP: OUT: sck#%d:slt#%d:%s:bad queue fopen:%s\n",
					  miscconn[user].sock,user,miscconn[user].queuename,get_error());
					  requeue_smtp(user);
					  free_sock(user,'5');
					  continue;
					 }
					 else {
					  /* write out sender - MAIL FROM */
					  if (write_smtp_data(user,1) == -1) {
					   requeue_smtp(user);
					   free_sock(user,'5');
					   continue;
					  }
					  else {
#if defined(SMTP_DEBUG)
 write_log(DEBUGLOG,YESTIME,"SMTP: Wrote out MAIL FROM\n");
#endif
					   miscconn[user].ready=0;
					   }
					 } /* good fd else */
					} /* good queuetoactive else */
				} /* else if stage 2 */
				else if (miscconn[user].stage==3) {
					/* write out recipient - RCPT TO */
					if (write_smtp_data(user,2) == -1) {
					 requeue_smtp(user);
					 free_sock(user,'5');
					 continue;
					}
					else {
#if defined(SMTP_DEBUG)
 write_log(DEBUGLOG,YESTIME,"SMTP: Wrote out RCPT TO\n");
#endif
					 miscconn[user].ready=0;
					}
				} /* else if stage 3 */
				else if (miscconn[user].stage==4) {
					/* write out data header - DATA */
					if (write_smtp_data(user,3) == -1) {
					 requeue_smtp(user);
					 free_sock(user,'5');
					 continue;
					}
					else {
#if defined(SMTP_DEBUG)
 write_log(DEBUGLOG,YESTIME,"SMTP: Wrote out DATA\n");
#endif
					 miscconn[user].ready=0;
					}
				} /* else if stage 4 */
				else if (miscconn[user].stage==5) {
					/* write out body of message */
					if (write_smtp_data(user,4) == -1) {
					 requeue_smtp(user);
					 free_sock(user,'5');
					 continue;
					}
					else {
#if defined(SMTP_DEBUG)
 write_log(DEBUGLOG,YESTIME,"SMTP: Wrote out BODY\n");
#endif
					 miscconn[user].ready=0;
					}
				} /* else if stage 5 */
/* write_log(DEBUGLOG,YESTIME,"SMTP: sock %d is done with writeable stages\n",miscconn[user].sock); */
			} /* if ready */
/* write_log(DEBUGLOG,YESTIME,"SMTP: sock %d is done with writeable is ready\n",miscconn[user].sock); */
		   } /* end of FD_ISSET */
/* write_log(DEBUGLOG,YESTIME,"SMTP: sock %d is done with writeable check\n",miscconn[user].sock); */

		  } /* else if SMTP */

		} /* else */
		/* parse_misc_input(user,inpstr); */
		continue;
	   } /* end of misc connection for */

	user=0;

	} /* end while LOOP_FOREVER */
	
    if (REBOOT_A_CRASH==1)
     treboot=1;
    shutdown_error(log_error(8));

}
