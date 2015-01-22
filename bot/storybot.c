/* Copyright 1990 by John Ockerbloom.
   Derivative for talker by Cygnus (ncohafmuta@asteroid-b612.org), 1996

  Permission to copy this software, to redistribute it, and to use it
  for any purpose is granted, subject to the following restrictions and
  understandings.

  1. Any copy made of this software must include this copyright notice
  in full.

  2. Users of this software agree to make their best efforts (a) to
  return to the above-mentioned authors any improvements or extensions
  that they make, so that these may be included in future releases; and
  (b) to inform the authors of noteworthy uses of this software.

  3. All materials developed as a consequence of the use of this
  software shall duly acknowledge such use, in accordance with the usual
  standards of acknowledging credit in academic research.

  4. The authors have made no warrantee or representation that the
  operation of this software will be error-free, and the authors are
  under no obligation to provide any services, by way of maintenance,
  update, or otherwise.

  5. In conjunction with products arising from the use of this material,
  there shall be no use of the name of the authors, of Carnegie-Mellon
  University, nor of any adaptation thereof in any advertising,
  promotional, or sales literature without prior written consent from
  the authors and Carnegie-Mellon University in each case.

*/

/* Last changed: Jul 16th 2001 */
#if defined(HAVE_CONFIG_H)

#include "../hdrfiles/config.h"
#include "../hdrfiles/includes.h"
#include "../hdrfiles/osdefs.h"

#else

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <signal.h>
#include <dirent.h>
#include <ctype.h>
#include <stdarg.h>             /* for va_start(),va_arg(),va_end() */
/* #include <varargs.h> */
#include <time.h>
#include <sys/time.h>
#if defined(HAVE_NETDB_H)
#include <netdb.h>
#else
#include "../hdrfiles/netdb.h"
#endif
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
/* BSD 4.2 and maybe some others need these defined */
#if !defined(FD_ZERO)
#define fd_set int
#define FD_ZERO(p)       (*p = 0)
#define FD_SET(n,p)      (*p |= (1<<(n)))
#define FD_CLR(n,p)      (*p &= ~(1<<(n)))
#define FD_ISSET(n,p)    (*p & (1<<(n)))
#endif
#if !defined(INVALID_SOCKET)
#define INVALID_SOCKET	-1
#endif
#define SOCKET_ERROR	-1

#endif

#define VERSION	"1.62"
#define YES		1
#define NO		0
#define YESTIME         1       /* timestamp log entries */
#define NOTIME          0       /* don't timestamp log entries */

/*** CONFIGURATION SECTION ***/

/* REPLACE THIS with the host the robot will be connecting to */
char *host = "127.0.0.1";

/* REPLACE THIS with the talker's main login port */
int   port = 1900;

/* BOT_NAME must be all lowercase */
/* REPLACE THIS with the bot's name */
/* This also acts as the login username for the bot */
#define BOT_NAME      "spokes"

/* REPLACE THIS with the bot's login password */
#define BOT_PASSWORD  "botpassword"

/* REPLACE THIS with the bot's login sequence. The default here is */
/* <carriage-return> bot_name <carriage-return> bot_password       */
/* YOU DO NOT NEED TO *END* THIS STRING WITH \n's OR \r's	   */
/* THE PROGRAM AUTOMATICALLY SENDS 2 OF THEM TO FINISH THE LOGIN   */
#define CONNMSG       "\n %s \n %s"

/* ROOT_ID must be all lowercase */
/* REPLACE THIS with the username of the bot's owner */
#define ROOT_ID       "cygnus"

/* REPLACE THIS with the command the bot will use to disconnect */
/* YOU DO NOT NEED TO END THIS STRING WITH \n's OR \r's		   */
/* THE PROGRAM AUTOMATICALLY SENDS ONE TO FINISH THE LOGOUT	   */
#define DCONMSG       ".quit"

/* REPLACE THESE with .desc messages the bot will change to */
#define IDLEMSG       "sits silently on his dais."
#define TELLMSG       "tells a tale to someone nearby."
#define LISTENMSG     "hears a story by"
#define SHUTMSG       "is asleep. Come back later."

/* REPLACE THIS with the name of the talker's main room */
#define MAIN_ROOM     "front_gate"

/* REPLACE THIS with the room the bot will go to when he first logs in */
/* ..usually where he will live - changeable online */
#define HOME_ROOM     "pond"

/* REPLACE THIS with the directions the bot will give to get to him */
/* Last %s is the bot's home room */
#define UNKNOWNDIRS "by typing \".go meadow\", then \".go %s\" "

/* REPLACE THIS with the file you want the bot to log his recorded */
/* conversations to */
#define LOG_FILE      "log.log"

/* REPLACE THIS with the file you want the bot to log his normal debug */
/* stuff to */
#define BOTLOG_FILE      "botlog"

/* the default for paragraph formatting - changeable online		*/
#define FORMATTED	NO

/* the default for the bot .kill-ing a user who swears at him or her	*/
/* changeable online							*/
#define KILLSWEAR	YES

/* the default for conversation logging to file - changeable online	*/
/* WARNING: you should not turn this on all the time, else		*/
/* the file may get really big!						*/
#define CONVOLOG	NO

/* Should be set same as in the talker's constants.h */
#define NAME_LEN	21
#define SAYNAME_LEN	150
#define MAX_USERS	40

/* the directory the bot's stories will go in. dont change normally */
#define STORYDIR      "Stories"

/* Talker commands */
#define SAY_COMMAND	".say"		/* public talk */
#define TELL_COMMAND	".tell"		/* private talk */
#define EMOTE_COMMAND	".emote"	/* public action */
#define SEMOTE_COMMAND	".semote"	/* private action */
#define SHOUT_COMMAND	".shout"	/* global talk */
#define SHEMOTE_COMMAND	".shemote"	/* global action */

/* dont change */
#define DIRECTORY     "."

/* should not need to change unless you change in the talker code */
char *our_delimiters="!|";

/*** END OF CONFIGURATION SECTION ***/

/* Some maximum quantities */

#define MAXSTRLEN  1024
#define MAXKIDS    25
#define MAXLEVELS  512
#define MAXLINES   255
#define FORMATMAX  64
#define TOKSIZ     1024
#define MSGSIZ     512
#define BIGBUF     4096

#define ARR_SIZE   9700
#define MAX_CHAR_BUFF	2000
#define FILE_NAME_LEN	256

/* The storystate structure gives us info about where we are in the story */

struct storystate {
  char title[MAXSTRLEN];        /* Title of the story                   */
  char author[MAXSTRLEN];       /* Author of current paragraph          */
  char paragraphid[MAXLEVELS];  /* ID of current paragraph              */
  int   writing;                /* Nonzero if we're in writing mode     */
  int   editing;                /* Nonzero if we're editing this para.  */
  int   numkids;                /* Number of children of this para.     */
  int   numlines;               /* Number of lines in this para.        */
  char *kids[MAXKIDS];          /* Names of authors of various children */
  char *paragraph[MAXLINES];    /* Text of current paragraph            */
                                /* Maybe more will come later!          */
};

struct {
	char name[NAME_LEN];
	char say_name[SAYNAME_LEN];
	char room[NAME_LEN];
	int vis;
	int logon;
} ustr[MAX_USERS];
 
/* Volume of request to StoryBot */

#define WHISPER 0
#define SAY     1
#define EMOTE   2
#define SHOUT   3
#define SHEMOTE 4
#define SEMOTE  5


/* For matching strings from users that are not part of story handling */
/* Results from star matcher */
char res1[BUFSIZ], res2[BUFSIZ], res3[BUFSIZ], res4[BUFSIZ];
char res5[BUFSIZ], res6[BUFSIZ], res7[BUFSIZ], res8[BUFSIZ];
char *result[] = { res1, res2, res3, res4, res5, res6, res7, res8 };
# define MATCH(D,P)     smatch ((D),(P),result)

void load_user(void);
void add_user(void);
void read_next(char *author, struct storystate *story);
void read_previous(struct storystate *story);
void read_paragraph(struct storystate *story);
void get_paragraph(struct storystate *story);
void read_titles(char *bitmatch, int mode);
void give_help(int user, struct storystate *story);
void init_talker_stuff(void);
void quit_robot(void);
void sync_bot(void);
void reboot_robot(void);
void clear_all_users(void);
void clear_user(int user);
void remove_first(char *inpstr);
void strtolower(char *str);
void midcpy(char *strf, char *strt, int fr, int to);
void crash_n_burn(void);
void handle_page(char *pageline);
void sysud(int ud);
void handle_sig(int sig);
void write_log(int wanttime, char *str, ...);
void sendchat(char *fmt, ...);
void process_input(char *inbuf);
void terminate(char *str);
int handle_action(int user, char *command, int volume);
int smatch (register char *dat, register char *pat, register char **res);
int find_free_slot(void);
int connect_robot(void);
int get_user_num(char *name);
int handle_page2(int user, char *pageline);
int setup_directory(struct storystate *story);
int save_para(struct storystate *story, int user);
int get_input(void);
char *getline(char *inbuf, FILE *fp);
char *resolve_title(char *prefix, char *buffer, int acceptprefix);
char *get_error(void);
char *get_time(time_t ref,int mode);
char *strip_color(char *str);
struct storystate *start_story(char *title, struct storystate *story);
struct storystate *finish_story(struct storystate *story);
struct storystate *handle_command(int user, char *command, int volume, struct storystate *story);
struct storystate *handle_writing(int user, char *command, struct storystate *story);
struct storystate *abort_writing(struct storystate *story);
struct storystate *ourstory;    /* Ptr to main story structure           */

/* A couple of global vars (Bad code style! Bad code style!) */

int g_FORMATTED=FORMATTED;		/* are paras formatted to 64 lines? */
int g_CONVOLOG=CONVOLOG;		/* conversation logging flag */
int g_KILLSWEAR=KILLSWEAR;		/* .kill people swearing at me? */
int bs;					/* bot's connection socket */
int bot_user = -1;			/* bot's user number */
char *g_HOME_ROOM=HOME_ROOM;		/* the bot's home room */
char thisprog[FILE_NAME_LEN];
char directions[FILE_NAME_LEN];
char t_inpstr[ARR_SIZE];  /* functions use t_mess as a buffer    */
char mess[ARR_SIZE];
fd_set    readmask;        /* bitmap read set                */
fd_set    writemask;       /* bitmap write set               */
FILE *log_fp;		 /* file pointer for log file */


/* The main program spawns the child process, sets up the socket */
/* to the talker and then calls robot().                         */
int main(int argc, char **argv)
{
  int	fd;
  int	retval=0;
  static int nfds=0;
  char	inpstr[ARR_SIZE];  /* functions use t_mess as a buffer    */
  struct timeval sel_timeout;  /* how much time to give select() */

   /*-------------------------------------------------*/
   /* Redirect stdin, stdout, and stderr to /dev/null */
   /*-------------------------------------------------*/
    fd = open("/dev/null",O_RDWR);
    if (fd < 0) {
      perror("Unable to open /dev/null");
      exit (-1); 
      }
        
    close(0); 
    close(1); 
    close(2);
   
    if (fd != 0)
    {
     dup2(fd,0);    
     close(fd);  
    }  
        
    dup2(0,1);
    dup2(0,2);

  switch(fork())
     {
        case -1:    write_log(YESTIME,"FORK 1 FAILED! %s\n",get_error());
                    exit(1);

#if defined(__OpenBSD__) || defined(__OSF__) || defined(__osf__) || defined(__bsdi__) || defined(__FreeBSD__) || defined(__NetBSD__)
        case 0:     setpgrp(0,0);
#else
        case 0:     setpgrp();
#endif
                    break;
   
        default:    sleep(1); 
                    exit(0);
      }


#if !defined(__CYGWIN32__)

  switch(fork())
     {
        case -1:    write_log(YESTIME,"FORK 2 FAILED! %s\n",get_error());
                    exit(2);
    
        case 0:     break;  /* child becomes server */
   
        default:    exit(0);
      }

#endif

/* Copy the program binary name to memory */
strcpy(thisprog,argv[0]);

    clear_all_users();

    write_log(YESTIME,"Bot \"%s\" starting...\n",BOT_NAME);

/* initialize some signals */
signal(SIGTERM,handle_sig);
signal(SIGSEGV,handle_sig);
signal(SIGILL,handle_sig);
signal(SIGBUS,handle_sig);
signal(SIGPIPE,handle_sig);
signal(SIGINT,SIG_IGN);
signal(SIGABRT,SIG_IGN);
signal(SIGFPE,SIG_IGN);
signal(SIGTSTP,SIG_IGN);
signal(SIGCONT,SIG_IGN);
signal(SIGHUP,SIG_IGN);
signal(SIGQUIT,SIG_IGN);

    bs = connect_robot();
    if (bs < 0) {
      write_log(YESTIME,"Connect failed to %s %d, exiting! %s\n",host,port,get_error());
      exit(-1);
     }

    /* log our existance */
    sysud(1);

    /* send login info to the talker */
    sprintf(mess,CONNMSG,BOT_NAME,BOT_PASSWORD);
    sendchat("%s\n\n", mess);

ourstory = NULL;

/*--------------------------*/
/**** Main program loop *****/
/*--------------------------*/
while(1)
   {
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

	if (bs != -1) {
		FD_SET(bs,&readmask);
/*
		if (alloced_size)
			FD_SET(bs,&writemask);
*/
		if (bs >= nfds)
			nfds = bs + 1;
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
                write_log(YESTIME,"Select failed with error %s\n",get_error());
                break;
           }
         else continue;
        }

                retval=get_input();
                strcpy(inpstr,t_inpstr);
                if (retval < 0) {
                switch(retval) {
                case -1: quit_robot(); /* socket doesnt exist - just in case */ break;
                case -6: /* buffer overload */ break;
                case -7: quit_robot(); /* fatal read error */ break;
                default: break;
                } /* end of switch */
                        continue;
                } /* end of if retval */

		process_input(inpstr);

   } /* end of main while loop */

    write_log(YESTIME,"Bot broke out of main robot loop, exiting!\n");
    crash_n_burn();
    quit_robot();

return 0;
}

/* This connect_robot function sets up a socket for the bot to the talker */
int connect_robot(void)
{
 struct sockaddr_in raddr;
 int size=sizeof(struct sockaddr_in);
 unsigned long   f;
 struct hostent *hp;
 int cs;

 /* Zero out memory for address */
 memset((char *)&raddr, 0, size);

 /* Handle numeric or host name addresses */
  if (isdigit((int)*host))
  { 
                if((f = inet_addr(host)) == -1L) {
		write_log(YESTIME,"Can't get net address for ip %s\n",host);
		return -1;
		}
    (void)bcopy((char *)&f,(char *)&raddr.sin_addr,sizeof(f));
  }
  else
  {
		if((hp = gethostbyname(host)) == (struct hostent *)0) {
		write_log(YESTIME,"Can't get net address for host %s\n",host);
		return -1;
		}
    (void)bcopy(hp->h_addr,(char *)&raddr.sin_addr,hp->h_length);
  }

 raddr.sin_port = htons((unsigned short)port);
 raddr.sin_family = AF_INET;
   
  if ((cs = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET) {
	write_log(YESTIME,"socket() creation failed for %s %d! %s\n",host,port,get_error());
	return -1;
   }
  if (connect(cs,(struct sockaddr *) &raddr, sizeof(raddr)) == SOCKET_ERROR) {
	write_log(YESTIME,"connect() failed for %s %d! %s\n",host,port,get_error());
	return -1;
  }
  return cs;
}

/* We are exiting now */
void quit_robot(void)
{

 shutdown(bs, 2);
 close(bs);
 
 sysud(0);

 /* Close log file if open */
 fclose(log_fp);

 exit(0);
}

/* We are rebooting now */
void reboot_robot(void)
{
 int fd;
 char *args[]={ "./restart",NULL };

 shutdown(bs, 2);
 close(bs);
 
 write_log(YESTIME,"REBOOT: %s is rebooting\n",BOT_NAME);

 /* Sleep for some time to make sure talker is up so we can login */
 sleep(10);
        /* If someone has changed the binary or the config filename while
           this prog has been running this won't work */
        fd = open( "/dev/null", O_RDONLY );
        if (fd != 0) {
            dup2( fd, 0 );
            close(fd);
        }
        fd = open( "/dev/null", O_WRONLY );
        if (fd != 1) {
            dup2( fd, 1 );
            close(fd);
        }
        fd = open( "/dev/tty", O_WRONLY ); 
        if (fd != 2) {
            dup2( fd, 2 );
            close(fd);
        }
        execvp("./restart",args);
        /* If we get this far it hasn't worked */
        close(0);
        close(1);
        close(2);

 write_log(YESTIME,"REBOOT: Reboot for %s failed\n",BOT_NAME);

}

/* Read user input from a socket */
int get_input(void)
{
int len=0;
int complete_line;
int gotcr=0;
int buff_size;
char *dest;
char astring[ARR_SIZE];
unsigned char inpchar[2];   /* read() data from socket into this */
int char_buffer_size = 0;
char char_buffer[MAX_CHAR_BUFF];

char_buffer[0]=0;
t_inpstr[0]=0;

if (bs==-1) return -1;
if (!FD_ISSET(bs,&readmask)) return -2;

               /*--------------------------------------------*/
               /* reset the user input space                 */
               /*--------------------------------------------*/
                 astring[0]  = 0;
                 inpchar[0]  = 0;
                 dest        = astring;

len = read(bs, inpchar, 1);

                if (!len) {
		 write_log(YESTIME,"get_input: Read NULL length in read()!\n");
                 return -7;
                } /* end of if */
                else if (len==SOCKET_ERROR) {
                 if (errno!=EAGAIN && errno!=EINTR) {
		 	write_log(YESTIME,"get_input: Fatal bad read()! %s\n",get_error());
                        return -7;
			}
                 else {
			 write_log(YESTIME,"get_input: Bad read()! %s\n",get_error());
                        return -3;
			}
                } /* end of else if */


                 /*-------------------------------------------*/
                 /* if there is input pending, read it        */
                 /*  (stopping on <cr>, <EOS>, or <EOF>)      */
                 /*-------------------------------------------*/
                 complete_line = 0;

                 while ((inpchar[0] != 0)  &&
                        (len != EOF)       &&
                        (len != -1)        &&
                        (complete_line ==0 )  )
                   {
                    /*----------------------------------------------*/
                    /* process input                                */
                    /*----------------------------------------------*/
		char_buffer[char_buffer_size++] = inpchar[0];

                    if (inpchar[0] == '\015') {
                        gotcr=1;
                    }
                    if (inpchar[0] == '\012')
                        {
                         complete_line = 1;
                         if (gotcr) gotcr=0;
                         char_buffer[char_buffer_size++] = 0;
                        }
                      else
                        {
                         if (char_buffer_size > (MAX_CHAR_BUFF-4) )
                           {
                  char_buffer[char_buffer_size++] = '\n';
                  char_buffer[char_buffer_size++] = 0;
                            complete_line = 1;
                           }
                        }

               inpchar[0]=0;
                    if (complete_line == 0)
                      {
                       len = read(bs, inpchar, 1);
                        if (len==-1 && gotcr==1) {
                        /* this is just a CR line termination */
                                complete_line = 1;
                                gotcr=0;
                                char_buffer[char_buffer_size++] = 0;
                        }
                      }
                   } /* end of while */

                /*--------------------------------------*/
                /* terminate the line                   */
                /*--------------------------------------*/

		char_buffer[char_buffer_size] = 0;

                /*------------------------------------------------*/
                /* check for complete line (terminated by \n)     */
                /*------------------------------------------------*/
                if (!complete_line)
                 {
                   return -4;
                 }

                /*--------------------------------------------*/
                /* copy the user buffer to the input string   */
                /*--------------------------------------------*/
                strcpy(astring, char_buffer);
                buff_size = strlen(astring);
                char_buffer_size = 0;

                if (buff_size > 8000) {
			write_log(YESTIME,"get_input: Possible flood!\n");
			return -6;
		}

terminate(astring);
strcpy(t_inpstr,astring);
return 0;
} /* end */


/* Send string down the socket */
void sendchat(char *fmt, ...)
{
  int len;
  char buf[BIGBUF];
  va_list args;

  buf[0]=0;
  va_start(args,fmt);
  vsprintf(buf,fmt,args);
  va_end(args);
 
  len = strlen (buf);

  if (write(bs, buf, len) != len)
    {
     write_log(YESTIME,"Write failed: %s %s\n",buf,get_error());
     quit_robot();
    }

}

/* The robot routine takes care of the initial setup required for startup */
void init_talker_stuff(void)
{

  /* Set bots directions */
  sprintf(directions,UNKNOWNDIRS,HOME_ROOM);

  sync_bot();
  sendchat(";has just rebooted.\n");
  sendchat(".desc %s\n", IDLEMSG);
  sendchat(".set home %s\n", HOME_ROOM);
  sendchat(".home\n");
  sendchat(".shout I'm baaaaack!\n");
  sendchat("_who\n");
}

/* The process_input routine is the main parser of the program.
   It keeps track of the goings-on in the room, and processes 
   the various requests from people.  

   StoryBot can understand both say and whisper, though most
   commands need to be said out loud, and StoryBot will sometimes
   say or whisper different messages depending on their type.           */

void process_input(char *inbuf)
{
   char *command;                  /* Points to command player gave         */
   char command2[100];             /* Points to command player gave         */
   int volume;                     /* Was cmd a SAY or a WHISPER?           */
   int chance2;
   int u,vis;
   char room[NAME_LEN];
   char playername[ARR_SIZE];      /* Name of player associated w/ last cmd */

   /* playername = (char *) malloc(MAXSTRLEN * sizeof(char)); */

/* write_log(YESTIME,"Got inbuf: \"%s\"\n",inbuf); */

	if (!strncmp(inbuf,"+++++ command:",14)) {
	  remove_first(inbuf);
	  remove_first(inbuf);
	  sscanf(inbuf,"%s ",command2);
	  command2[strlen(command2)-1]=0;
	  if (!strcmp(command2,"who")) {
		remove_first(inbuf);
		/* read in name */
		sscanf(inbuf,"%s ",playername);
		if (!strncmp(playername,our_delimiters,2)) midcpy(playername,playername,2,strlen(playername)-2);
		/* see if we already have this user in our list */
		u=get_user_num(playername);
		if (u==-1) u=find_free_slot();
		/* get name */
		strcpy(ustr[u].say_name,playername);
		strcpy(ustr[u].name,strip_color(ustr[u].say_name));
		strtolower(ustr[u].name);
		/* get room */
		remove_first(inbuf);
		sscanf(inbuf,"%s ",ustr[u].room);
		/* get vis status */
		remove_first(inbuf);
		sscanf(inbuf,"%d",&ustr[u].vis);
		if (!strcmp(ustr[u].name,BOT_NAME)) bot_user=u;
        write_log(YESTIME,"Read in user %s (cn: %s) (rm: %s) (vs: %d)\n", strip_color(ustr[u].say_name),ustr[u].say_name,ustr[u].room,ustr[u].vis);
	    } /* end of _who command line read */
	}
     else if ( (sscanf(inbuf, "+++++ came in:%s", playername)) == 1) {
        write_log(YESTIME,"%s recorded as arrived.\n", playername);
	u=get_user_num(playername);
        if (ourstory && ourstory->numkids > 0)
        sendchat(".tell %s Hello, %s.\n.t %s I'm in the middle of telling a story.\n",
               ustr[u].name, ustr[u].say_name, ustr[u].name);
        else if (ourstory && ourstory->writing)
        sendchat(".tell %s Hello, %s.\n.t %s I'm busy listening to %s's story.\n",
               ustr[u].name, ustr[u].say_name, ustr[u].name, ourstory->author);
        else
        sendchat(".tell %s Hello, %s.\n.t %s I'm glad to have some company.\n",
               ustr[u].name, ustr[u].say_name, ustr[u].name);

        if ((ourstory == NULL) || !(ourstory->writing))
          sendchat(".tell %s Say 'help' to see commands I recognize.\n",ustr[u].name);
     }
     else if ( (sscanf(inbuf, "+++++ logon:%s %s %d", playername,room,&vis)) == 3) {
	/*
	if (!load_user(playername))
	  add_user(playername);
	*/
	if (!strncmp(playername,our_delimiters,2)) midcpy(playername,playername,2,strlen(playername)-2);
	u=get_user_num(playername);
	if (u==-1) u=find_free_slot();

	strcpy(ustr[u].say_name,playername);
	strcpy(ustr[u].name,strip_color(playername));
	strtolower(ustr[u].name);
	strcpy(ustr[u].room,room);
	ustr[u].vis=vis;
	ustr[u].logon=1;

	vis=0;
	room[0]=0;
        write_log(YESTIME,"%s recorded as logged on.\n", strip_color(ustr[u].say_name));
        chance2 = rand() % 3;
        if (chance2==1) sendchat(".tell %s Hiya %s!\n",ustr[u].name,ustr[u].say_name);
        else if (chance2==2) sendchat(".tell %s Hola %s!\n",ustr[u].name,ustr[u].say_name);
        else sendchat(".tell %s Howdy %s!\n",ustr[u].name,ustr[u].say_name);
     }
     else if ( (sscanf(inbuf, "+++++ logoff:%s", playername)) == 1) {
	u=get_user_num(playername);

       if (ourstory && ourstory->writing &&
           !strcmp(ustr[u].name, ourstory->author)) {
         sendchat(";has finished listening to %s's story.\n",ourstory->author); 
         ourstory = finish_story(ourstory);
         sendchat(".desc %s\n", IDLEMSG);
       }
        write_log(YESTIME,"%s recorded as logged off.\n", strip_color(ustr[u].say_name));

	/* Clear structure */
	clear_user(u);
     }
     else if ( (sscanf(inbuf, "+++++ left:%s", playername)) == 1) {
	u=get_user_num(playername);

       write_log(YESTIME,"%s recorded as left.\n", strip_color(ustr[u].say_name));

       if (ourstory && ourstory->writing &&
           !strcmp(ustr[u].name, ourstory->author)) {
         sendchat(";has finished listening to %s's story.\n",ourstory->author); 
         ourstory = finish_story(ourstory);
         sendchat(".desc %s\n", IDLEMSG);
       }
     } /* end of else if */
     else if (!strcmp(inbuf, "+++++ SHUTDOWN")) {
        sendchat(".desc %s\n", SHUTMSG); /* shut down bot! */
        sendchat("%s\n", DCONMSG);
        write_log(YESTIME,"Got talker shutdown message.\n");
        quit_robot();
     } 
     else if (!strcmp(inbuf, "+++++ REBOOT")) {
        sendchat(".desc %s\n", SHUTMSG); /* reboot bot! */
        sendchat("%s\n", DCONMSG);
        write_log(YESTIME,"Got talker reboot message.\n");
        reboot_robot();
     }
     else if (!strcmp(inbuf, "+++++ QUIT")) {
        write_log(YESTIME,"Got talker quit message.\n");
        quit_robot();
     }
     else if (!strcmp(inbuf, "+++++ WELCOME")) {
        write_log(YESTIME,"Got talker welcome message.\n");
        init_talker_stuff();
     }
     else if (sscanf(inbuf, "+++++ comm_say:%s %*s", playername) == 1) {
	u=get_user_num(playername);

if (!strcmp(ustr[u].name,BOT_NAME)) return;
         command = inbuf+strlen(playername)+16;
	 if (g_CONVOLOG==YES) {
	  if (strcmp(ustr[u].name,BOT_NAME)) {
		fputs(inbuf,log_fp);
		fputs("\n",log_fp);
		}
	  }
         if (command[strlen(command)-1]=='\"') command[strlen(command)-1]='\0';

         write_log(YESTIME,"SAY: \"%s\" \"%s\"\n",strip_color(ustr[u].say_name),command);

         volume = SAY;
     if (ourstory && ourstory->writing) {
       ourstory = handle_command(u, command, volume, ourstory);
       }
     else {
       if (handle_action(u, command, volume) == 1)
        ourstory = handle_command(u, command, volume, ourstory);
       else { }
      }
     }
     else if (sscanf(inbuf, "+++++ comm_tell:%s %*s", playername) == 1) {
	u=get_user_num(playername);

if (!strcmp(ustr[u].name,BOT_NAME)) return;
         command = inbuf+strlen(playername)+17;

         if (command[strlen(command)-1]=='\"') command[strlen(command)-1]='\0';

         write_log(YESTIME,"TELL: \"%s\" \"%s\"\n",strip_color(ustr[u].say_name),command);

         volume = WHISPER;
     if (ourstory && ourstory->writing) {
         if (strcmp(ustr[u].name,ourstory->author)) {
          sendchat(".tell %s I can't talk to you now, I'm busy listening to %s's story.\n", ustr[u].name, ourstory->author);
          return;
         } /* end of if strcmp */
       ourstory = handle_command(u, command, volume, ourstory);
       }
     else {
       if (handle_action(u, command, volume) == 1)
        ourstory = handle_command(u, command, volume, ourstory);
       else { }
      }
     }

     else if (sscanf(inbuf, "+++++ comm_shout:%s %*s", playername) == 1) {
	u=get_user_num(playername);

if (!strcmp(ustr[u].name,BOT_NAME)) return;
         command = inbuf+strlen(playername)+18;
         if (command[strlen(command)-1]=='\"') command[strlen(command)-1]='\0';

         write_log(YESTIME,"SHOUT: \"%s\" \"%s\"\n",strip_color(ustr[u].say_name),command);

         volume = SHOUT;
       handle_action(u, command, volume);
     }

     else if (sscanf(inbuf, "+++++ comm_semote:%s %*s", playername) == 1) {
	u=get_user_num(playername);

if (!strcmp(ustr[u].name,BOT_NAME)) return;
         command = inbuf+strlen(playername)+19;
         if (command[strlen(command)-1]=='\"') command[strlen(command)-1]='\0';

         write_log(YESTIME,"SEMOTE: \"%s\" \"%s\"\n",strip_color(ustr[u].say_name),command);

             volume = SEMOTE;
             handle_action(u, command, volume);
         }

     else if (sscanf(inbuf, "+++++ comm_shemote:%s %*s", playername) == 1) {
	u=get_user_num(playername);

if (!strcmp(ustr[u].name,BOT_NAME)) return;
         command = inbuf+strlen(playername)+20;
         if (command[strlen(command)-1]=='\"') command[strlen(command)-1]='\0';

         write_log(YESTIME,"SHEMOTE: \"%s\" \"%s\"\n",strip_color(ustr[u].say_name),command);

             volume = SHEMOTE;
             handle_action(u, command, volume);
         }
     else if (sscanf(inbuf, "+++++ comm_emote:%s %*s", playername) == 1) {
	u=get_user_num(playername);

if (!strcmp(ustr[u].name,BOT_NAME)) return;
             command = inbuf+strlen(playername)+18;
             volume = EMOTE;

	 if (g_CONVOLOG==YES) {
	  if (strcmp(ustr[u].name,BOT_NAME)) {
		fputs(inbuf,log_fp);
		fputs("\n",log_fp);
		}
	  }

             handle_action(u, command, volume);
         }

}

/* This function handle all commands sent to the bot that */
/* aren't related to story functions */
int handle_action(int user, char *command, int volume)
{
char command2[800];
char comstr[50];
char namestr[SAYNAME_LEN];
int chance=0;

comstr[0]=0;
namestr[0]=0;

if (!strcmp(ustr[user].name,BOT_NAME)) goto ENDING;

/* Copy the original command line to another buffer so we can */
/* lowercase it */
strcpy(command2,command);
strtolower(command2);

/* SAY part */
if (volume==SAY) {
 if (!strcmp(command2,BOT_NAME)) {
   sendchat(".say What do you want, %s? ;-)\n",ustr[user].say_name);
   goto ENDING;
   }
 if (strstr(command2,BOT_NAME)) {
   if (strstr (command2, "fuck you" ) ||
       strstr (command2, "screw you" )) {
       sendchat(".say Sorry, I don't do requests.\n");
	if (g_KILLSWEAR==YES) {
        sendchat(".shemote sings to %s \"Na na na na, na na na na, hey hey hey, goodbye!\"\n",ustr[user].say_name);
        sendchat(".kill %s\n",ustr[user].name);
	}
	else {
	sendchat(".say Don't make me do a lorena bobbit on that tootsie roll!\n");
	}
        goto ENDING;
      }
   else if (strstr (command2, "how are you" ) ||
            strstr (command2, "how are ya" ) ||
            strstr (command2, "how are u" ) ||
            strstr (command2, "how be you" ) ||
            strstr (command2, "how be ya" ) ||
            strstr (command2, "how goes it" )) {
        sendchat(".say I'm good %s, how are you?\n",ustr[user].say_name);
        goto ENDING;
      }
   else if (strstr (command2, "what is up" ) ||
            strstr (command2, "what's up" )) {
        sendchat(".say An adverb, %s.\n",ustr[user].say_name);
        sendchat(".emote rolls his eyes\n");
        goto ENDING;
      }
   else if (strstr (command2, "what is goin down" ) ||
            strstr (command2, "what's goin down" )) {
        sendchat(".say No %s, what's goin up!?\n",ustr[user].say_name);
        goto ENDING;
      }
   else if (strstr (command2, "que pasa" ) ||
            strstr (command2, "what is happenin" ) ||
            strstr (command2, "what are you doin" ) ||
            strstr (command2, "whatcha doin" ) ||
            strstr (command2, "what's happenin" )) {
        sendchat(".say Not much %s, you?\n",ustr[user].say_name);
        goto ENDING;
      }
   else if (strstr (command2, "you suck" ) ||
            strstr (command2, "you blow" ) ||
            strstr (command2, "you stink" ) ||
            strstr (command2, "you bite" )) {
        sendchat(".say Yeah? And you smell bad!\n");
        goto ENDING;
      }
   else if (strstr (command2, "shut up" ) ||
            strstr (command2, "shuddup" ) ||
            strstr (command2, "hush" ) ||
            strstr (command2, "be quiet" )) {
        sendchat(".say And who's gonna make me? You? HAH!\n");
        goto ENDING;
      }
  } /* end of check for bots name */

   if (strstr (command2, "i'm good" ) ||
            strstr (command2, "i am good" ) ||
            strstr (command2, "im good" ) ||
            strstr (command2, "i'm ok" ) ||
            strstr (command2, "i am ok" ) ||
            strstr (command2, "im ok" ) ||
            strstr (command2, "i'm well" ) ||
            strstr (command2, "i am well" ) ||
            strstr (command2, "im well" ) ||
            strstr (command2, "nuthin" ) ||
            strstr (command2, "not too much" ) ||
            strstr (command2, "not a lot" ) ||
            strstr (command2, "nothing" ) ||
            strstr (command2, "i'm fine" ) ||
            strstr (command2, "i am fine" ) ||
            strstr (command2, "im fine" ) ||
            strstr (command2, "i'm so-so" ) ||
            strstr (command2, "i am so-so" ) ||
            strstr (command2, "im so-so" ) ||
            strstr (command2, "i'm alright" ) ||
            strstr (command2, "i am alright" ) ||
            strstr (command2, "im alright" )) {
        chance = rand() % 2;
        if (chance==1) sendchat(".emote nods\n");
        else sendchat(".say I see\n");
        goto ENDING;
      }

   else if (strstr (command2, "lag" )) {
        chance = rand() % 3;
        if (chance==1)
         sendchat(".say Where?? WHERE!?!?\n");
        else if (chance==2)
         sendchat(".say NO!!! Anything but that!!\n");
	else
         sendchat(".say Really %s? Sucks bein' you!\n",ustr[user].say_name);

        sendchat(".emote hides\n");
        goto ENDING;
      }

   else if (strstr (command2, "taxes" )) {
         sendchat(".emote hops on his little soapbox.\n");
	 sleep(2);
         sendchat(".say Citizens!  Are you as tired as I am of these damned sales taxes?\n");
	 sleep(2);
	 sendchat(".say If you elect me as your leader, I'll eliminate ALL unjust taxes in this realm!\n");
	 sleep(2);
	 sendchat(".emote pauses and blinks brightly...\n");
	 sleep(3);
	 sendchat(".say Read my lips, No New Taxes!\n");
	 sleep(2);
	 sendchat(".emote leaps down from his soapbox.\n");
        goto ENDING;
      }

   else if (strstr (command2, "vax" ) ||
	    strstr (command2, "v a x" ) ||
	    strstr (command2, "v-a-x" ) ||
	    strstr (command2, "v_a_x" ) ||
	    strstr (command2, "v m s" ) ||
	    strstr (command2, "v-m-s" ) ||
	    strstr (command2, "v_m_s" ) ||
	    strstr (command2, "vms" )) {
	if (g_KILLSWEAR==YES) {
        sendchat(".say Such language cannot go unpunished!\n");
        sendchat(".kill %s\n",ustr[user].name);
	}
	else {
	sendchat(".emote gasps and back-hands %s across the face!\n",ustr[user].say_name);
	sendchat(".say Don't ever disrespect the family like that!\n");
	}
	goto ENDING;
	}

if (strstr(command2,BOT_NAME)) {
    if (strstr (command2, "hi" ) ||
            strstr (command2, "hello" ) ||
            strstr (command2, "yo" ) ||
            MATCH (command2, "re *" )) {
	if (!ustr[user].logon) {
        chance = rand() % 3;
        if (chance==1) sendchat(".say Hiya %s!\n",ustr[user].say_name);
        else if (chance==2) sendchat(".say Hola %s!\n",ustr[user].say_name);
        else sendchat(".say Howdy %s!\n",ustr[user].say_name);
	}
	else ustr[user].logon=0;

        goto ENDING;
      }
    else if (strstr (command2, "bye" ) ||
            strstr (command2, "ciao" ) ||
            strstr (command2, "cya" ) ||
            strstr (command2, "adios" )) {
        chance = rand() % 3;
        if (chance==1) sendchat(".say Bye %s!\n",ustr[user].say_name);
        else if (chance==2) sendchat(".say Adios %s!\n",ustr[user].say_name);
        else sendchat(".say Ciao, %s!\n",ustr[user].say_name);
        goto ENDING;
      }

    }

 return 1;
}  /* end of SAY part */

/* WHISPER part */
if (volume==WHISPER) {
     if (MATCH (command2, "fuck you*" ) ||
         MATCH (command2, "screw you*" )) {
       sendchat(".tell %s Sorry, I don't do requests.\n",ustr[user].name);
	if (g_KILLSWEAR==YES) {
       sendchat(".shemote sings to %s \"Na na na na, na na na na, hey hey hey, goodbye!\"\n",ustr[user].say_name);
       sendchat(".kill %s\n",ustr[user].name);
	}
	else {
	sendchat(".tell %s Don't make me do a lorena bobbit on that tootsie roll!\n",ustr[user].name);
	}
       goto ENDING;
      }
   else if (strstr (command2, "how are you" ) ||
            strstr (command2, "how be you" ) ||
            strstr (command2, "how are ya" ) ||
            strstr (command2, "how are u" ) ||
            strstr (command2, "how be ya" ) ||
            strstr (command2, "how goes it" )) {
        sendchat(".tell %s I'm good, how are you?\n",ustr[user].name);
        goto ENDING;
      }
   else if (strstr (command2, "what is up" ) ||
            strstr (command2, "what's up" )) {
        sendchat(".tell %s An adverb, %s.\n",ustr[user].name,ustr[user].say_name);
        sendchat(".semote %s rolls his eyes\n",ustr[user].name);
        goto ENDING;
      }
   else if (strstr (command2, "what is goin down" ) ||
            strstr (command2, "what's goin down" )) {
        sendchat(".tell %s No, what's goin up!?\n",ustr[user].name);
        goto ENDING;
      }
   else if (strstr (command2, "que pasa" ) ||
            strstr (command2, "what is happenin" ) ||
            strstr (command2, "what are you doin" ) ||
            strstr (command2, "whatcha doin" ) ||
            strstr (command2, "what's happenin" )) {
        sendchat(".tell %s Not much, you?\n",ustr[user].name);
        goto ENDING;
      }
   else if (strstr (command2, "you suck" ) ||
            strstr (command2, "you blow" ) ||
            strstr (command2, "you stink" ) ||
            strstr (command2, "you bite" )) {
        sendchat(".tell Yeah? And you smell bad!\n");
        goto ENDING;
      }
   else if (strstr (command2, "shut up" ) ||
            strstr (command2, "shuddup" ) ||
            strstr (command2, "hush" ) ||
            strstr (command2, "be quiet" )) {
        sendchat(".tell And who's gonna make me? You? HAH!\n");
        goto ENDING;
      }
   else if (strstr (command2, "i'm good" ) ||
            strstr (command2, "i am good" ) ||
            strstr (command2, "im good" ) ||
            strstr (command2, "i'm ok" ) ||
            strstr (command2, "i am ok" ) ||
            strstr (command2, "im ok" ) ||
            strstr (command2, "i'm well" ) ||
            strstr (command2, "i am well" ) ||
            strstr (command2, "im well" ) ||
            strstr (command2, "nuthin" ) ||
            strstr (command2, "not too much" ) ||
            strstr (command2, "not a lot" ) ||
            strstr (command2, "nothing" ) ||
            strstr (command2, "i'm fine" ) ||
            strstr (command2, "i am fine" ) ||
            strstr (command2, "im fine" ) ||
            strstr (command2, "i'm so-so" ) ||
            strstr (command2, "i am so-so" ) ||
            strstr (command2, "im so-so" ) ||
            strstr (command2, "i'm alright" ) ||
            strstr (command2, "i am alright" ) ||
            strstr (command2, "im alright" )) {
        chance = rand() % 2;
        if (chance==1) sendchat(".semote %s nods\n",ustr[user].name);
        else sendchat(".tell %s I see\n",ustr[user].name);
        goto ENDING;
      }
   else if (strstr (command2, "lag" )) {
        chance = rand() % 3;
        if (chance==1)
         sendchat(".tell %s Where?? WHERE!?!?\n",ustr[user].name);
        else if (chance==2)
         sendchat(".tell %s NO!!! Anything but that!!\n",ustr[user].name);
	else
         sendchat(".say Really %s? Sucks bein' you!\n",ustr[user].say_name);

        sendchat(".semote %s hides\n",ustr[user].name);
        goto ENDING;
      }
   else if (MATCH (command2, "re *" ) ||
            strstr (command2, "hello" ) ||
            MATCH (command2, "yo *" ) ||
            MATCH (command2, "hi*" )) {
	if (!ustr[user].logon) {
        chance = rand() % 3;
        if (chance==1) sendchat(".tell %s Hiya %s!\n",ustr[user].name,ustr[user].say_name);
        else if (chance==2) sendchat(".tell %s Hola %s!\n",ustr[user].name,ustr[user].say_name);
        else sendchat(".tell %s Howdy %s!\n",ustr[user].name,ustr[user].say_name);
	}
	else ustr[user].logon=0;
       goto ENDING;
      }
   else if (MATCH (command2, "bye *" ) ||
            MATCH (command2, "ciao *" ) ||
            MATCH (command2, "cya *" ) ||
            MATCH (command2, "adios *" )) {
        chance = rand() % 3;
        if (chance==1) sendchat(".tell %s Bye %s!\n",ustr[user].name,ustr[user].say_name);
        else if (chance==2) sendchat(".tell %s Adios %s!\n",ustr[user].name,ustr[user].say_name);
        else sendchat(".tell %s Ciao, %s!\n",ustr[user].name,ustr[user].say_name);
       goto ENDING;
      }
   else if (strstr (command2, "vax" ) ||
	    strstr (command2, "v a x" ) ||
	    strstr (command2, "v-a-x" ) ||
	    strstr (command2, "v_a_x" ) ||
	    strstr (command2, "v m s" ) ||
	    strstr (command2, "v-m-s" ) ||
	    strstr (command2, "v_m_s" ) ||
	    strstr (command2, "vms" )) {
	if (g_KILLSWEAR==YES) {
        sendchat(".tell %s Such language cannot go unpunished!\n",ustr[user].name);
        sendchat(".kill %s\n",ustr[user].name);
	}
	else {
	sendchat(".semote %s gasps and back-hands you across the face!\n",ustr[user].name);
	sendchat(".tell %s Don't ever disrespect the family like that!\n",ustr[user].name);
	}
	goto ENDING;
	}

  return 1;
}  /* end of WHISPER part */



/* SHOUT part */
else if (volume==SHOUT) {
  if (strstr(command2, BOT_NAME)) {
     if (strstr (command2, "fuck you*" ) ||
         strstr (command2, "screw you*" )) {
       sendchat(".shout Sorry %s, I don't do requests.\n",ustr[user].say_name);
	if (g_KILLSWEAR==YES) {
       sendchat(".shemote sings to %s \"Na na na na, na na na na, hey hey hey, goodbye!\"\n",ustr[user].say_name);
       sendchat(".kill %s\n",ustr[user].name);
	}
	else {
	sendchat(".shout Don't make me do a lorena bobbit on that tootsie roll!\n");
	}
       goto ENDING;
      }
   else if (strstr (command2, "how are you" ) ||
            strstr (command2, "how be you" ) ||
            strstr (command2, "how are ya" ) ||
            strstr (command2, "how are u" ) ||
            strstr (command2, "how be ya" ) ||
            strstr (command2, "how goes it" )) {
        sendchat(".shout I'm good %s, how are you?\n",ustr[user].say_name);
        goto ENDING;
      }
   else if (strstr (command2, "what is up" ) ||
            strstr (command2, "what's up" )) {
        sendchat(".shout An adverb, %s.\n",ustr[user].say_name);
        sendchat(".shemote rolls his eyes\n");
        goto ENDING;
      }
   else if (strstr (command2, "what is goin down" ) ||
            strstr (command2, "what's goin down" )) {
        sendchat(".shout No %s, what's goin up!?\n",ustr[user].say_name);
        goto ENDING;
      }
   else if (strstr (command2, "que pasa" ) ||
            strstr (command2, "what is happenin" ) ||
            strstr (command2, "what are you doin" ) ||
            strstr (command2, "whatcha doin" ) ||
            strstr (command2, "what's happenin" )) {
        sendchat(".shout Not much %s, you?\n",ustr[user].say_name);
        goto ENDING;
      }
   else if (strstr (command2, "you suck" ) ||
            strstr (command2, "you blow" ) ||
            strstr (command2, "you stink" ) ||
            strstr (command2, "you bite" )) {
        sendchat(".shout Yeah? And you smell bad!\n");
        goto ENDING;
      }
   else if (strstr (command2, "shut up" ) ||
            strstr (command2, "shuddup" ) ||
            strstr (command2, "hush" ) ||
            strstr (command2, "be quiet" )) {
        sendchat(".shout And who's gonna make me? You? HAH!\n");
        goto ENDING;
      }
   else if (strstr (command2, "i'm good" ) ||
            strstr (command2, "i am good" ) ||
            strstr (command2, "im good" ) ||
            strstr (command2, "i'm ok" ) ||
            strstr (command2, "i am ok" ) ||
            strstr (command2, "im ok" ) ||
            strstr (command2, "i'm well" ) ||
            strstr (command2, "i am well" ) ||
            strstr (command2, "im well" ) ||
            strstr (command2, "i'm fine" ) ||
            strstr (command2, "nuthin" ) ||
            strstr (command2, "not too much" ) ||
            strstr (command2, "not a lot" ) ||
            strstr (command2, "nothing" ) ||
            strstr (command2, "i am fine" ) ||
            strstr (command2, "im fine" ) ||
            strstr (command2, "i'm so-so" ) ||
            strstr (command2, "i am so-so" ) ||
            strstr (command2, "im so-so" ) ||
            strstr (command2, "i'm alright" ) ||
            strstr (command2, "im alright" ) ||
            strstr (command2, "i am alright" )) {
        chance = rand() % 2;
        if (chance==1) sendchat(".shemote nods\n");
        else sendchat(".shout I see\n");
        goto ENDING;
      }
   else if (MATCH (command2, "re *" ) ||
           strstr (command2, "hello" ) ||
           strstr (command2, "yo" ) ||
           strstr (command2, "hi" )) {
	if (!ustr[user].logon) {
        chance = rand() % 3;
        if (chance==1) sendchat(".shout Hiya %s!\n",ustr[user].say_name);
        else if (chance==2) sendchat(".shout Hola %s!\n",ustr[user].say_name);
        else sendchat(".shout Howdy %s!\n",ustr[user].say_name);
	}
	else ustr[user].logon=0;

      goto ENDING;
   }
   else if (strstr (command2, "bye" ) ||
           strstr (command2, "ciao" ) ||
           strstr (command2, "cya" ) ||
           strstr (command2, "adios" )) {
        chance = rand() % 3;
        if (chance==1) sendchat(".shout Bye %s!\n",ustr[user].say_name);
        else if (chance==2) sendchat(".shout Adios %s!\n",ustr[user].say_name);
        else sendchat(".shout Ciao, %s!\n",ustr[user].say_name);
      goto ENDING;
   }
 } /* end of if name check */

   if (strstr (command2, "lag" )) {
        chance = rand() % 3;
        if (chance==1)
         sendchat(".shout Where?? WHERE!?!?\n");
        else if (chance==2)
         sendchat(".shout NO!!! Anything but that!!\n");
	else 
         sendchat(".say Really %s? Sucks bein' you!\n",ustr[user].say_name);

        sendchat(".shemote hides\n");
        goto ENDING;
      }
   else if (strstr (command2, "taxes" )) {
         sendchat(".shemote hops on his little soapbox.\n");
	 sleep(2);
         sendchat(".shout Citizens!  Are you as tired as I am of these damned sales taxes?\n");
	 sleep(2);
	 sendchat(".shout If you elect me as your leader, I'll eliminate ALL unjust taxes in this realm!\n");
	 sleep(2);
	 sendchat(".shemote pauses and blinks brightly...\n");
	 sleep(3);
	 sendchat(".shout Read my lips, No New Taxes!\n");
	 sleep(2);
	 sendchat(".shemote leaps down from his soapbox.\n");
        goto ENDING;
      }
   else if (strstr (command2, "jazzin" )) {
         sendchat(".shout Hmm? Jazzin!?\n");
	 sleep(2);
         sendchat(".shemote kicks things around\n");
	 sleep(2);
         sendchat(".shout If i ever see that no-good penis wrinkle..he'll pay big!\n");
        goto ENDING;
      }
   else if (strstr (command2, "scupper" )) {
         sendchat(".shout mmm, sugar-nipples..\n");
        goto ENDING;
      }
   else if (strstr (command2, "vax" ) ||
	    strstr (command2, "v a x" ) ||
	    strstr (command2, "v-a-x" ) ||
	    strstr (command2, "v_a_x" ) ||
	    strstr (command2, "v m s" ) ||
	    strstr (command2, "v-m-s" ) ||
	    strstr (command2, "v_m_s" ) ||
	    strstr (command2, "vms" )) {
	if (g_KILLSWEAR==YES) {
        sendchat(".shout Such language cannot go unpunished!\n");
        sendchat(".kill %s\n",ustr[user].name);
	}
	else {
	sendchat(".shemote gasps and back-hands %s across the face!\n",ustr[user].say_name);
	sendchat(".shout Don't ever disrespect the family like that!\n");
	}
	goto ENDING;
	}

 return 1;
} /* end of SHOUT */
  

/* EMOTE part */
else if (volume==EMOTE) {
 if (strstr(command2,BOT_NAME)) {
  if (MATCH (command2, "kicks *" )) {
       sendchat("; kicks %s back!\n",ustr[user].say_name);
      goto ENDING;
      }
  if (MATCH (command2, "smacks *" )) {
       sendchat("; smacks %s back!\n",ustr[user].say_name);
      goto ENDING;
      }
  if (MATCH (command2, "pounds *" )) {
       sendchat("; gets up and thumbs his probiscus at %s\n",ustr[user].say_name);
      goto ENDING;
      }
  else if (MATCH (command2, "licks *" )) {
       sendchat("; ewwwwwsss, %s cooties!\n",ustr[user].say_name);
      goto ENDING;
      }
  else if (MATCH (command2, "kisses *" )) {
       sendchat("; blushes and kisses %s back on the cheek.\n",ustr[user].say_name);
      goto ENDING;
      }
  else if (MATCH (command2, "hug* *" )) {
       sendchat("; huggles back\n");
      goto ENDING;
      }
  else if (MATCH (command2, "pokes *" )) {
       sendchat("; pokies %s in some choice spots.\n",ustr[user].say_name);
      goto ENDING;
      }
  }  /* end of name check */

  if (strstr (command2, "laughs" ) ||
           strstr (command2, "laffs" ) ||
           strstr (command2, "chuckles" ) ||
           strstr (command2, "chortles" ) ||
           strstr (command2, "snickers" )) {
       sendchat("; chuckles a bit.\n");
       goto ENDING;
      }
  if (strstr (command2, "giggles" ) ||
      strstr (command2, "gigglz" )) {
      sendchat("; pinches %s's cheeks, you're so cute!\n",ustr[user].say_name);
      goto ENDING;
     }
  if (strstr (command2, "vax" ) ||
	    strstr (command2, "v a x" ) ||
	    strstr (command2, "v-a-x" ) ||
	    strstr (command2, "v_a_x" ) ||
	    strstr (command2, "v m s" ) ||
	    strstr (command2, "v-m-s" ) ||
	    strstr (command2, "v_m_s" ) ||
	    strstr (command2, "vms" )) {
	if (g_KILLSWEAR==YES) {
        sendchat(".say Such language cannot go unpunished!\n");
        sendchat(".kill %s\n",ustr[user].name);
	}
	else {
	sendchat(".emote gasps and back-hands %s across the face!\n",ustr[user].say_name);
	sendchat(".say Don't ever disrespect the family like that!\n");
	}
	goto ENDING;
	}

  return 1;
}  /* end of EMOTE */

/* SEMOTE or SHEMOTE part */
else if ((volume==SEMOTE) || (volume==SHEMOTE)) {
  if (volume==SEMOTE) {
    sprintf(comstr,".semote %s",ustr[user].name);
    strcpy(namestr,"you");
    }
  else if (volume==SHEMOTE) {
    strcpy(comstr,".shemote");
    strcpy(namestr,ustr[user].say_name);
    }

  if (strstr (command2, "kicks" )) {
       sendchat("%s kicks %s back!\n",comstr,namestr);
      goto ENDING;
      }
  if (strstr (command2, "smacks" )) {
       sendchat("%s smacks %s back!\n",comstr,namestr);
      goto ENDING;
      }
  if (strstr (command2, "pounds" )) {
       sendchat("%s gets up and thumbs his probiscus at %s\n",comstr,namestr);
      goto ENDING;
      }
  else if (strstr (command2, "licks" )) {
      sendchat("%s ewwwwwsss, cooties!\n",comstr);
      goto ENDING;
      }
  else if (strstr (command2, " pokes" )) {
      sendchat("%s pokies %s in some choice spots.\n",comstr,namestr);
      goto ENDING;
      }
  else if (strstr (command2, "kisses" )) {
      sendchat("%s blushes and kisses %s back on the cheek.\n",comstr,namestr);
      goto ENDING;
      }
  else if (strstr (command2, "hug" )) {
      sendchat("%s huggles %s back.\n",comstr,namestr);
      goto ENDING;
      }
  else if (strstr (command2, "laughs" ) ||
           strstr (command2, "laffs" ) ||
           strstr (command2, "chuckles" ) ||
           strstr (command2, "chortles" ) ||
           strstr (command2, "snickers" )) {
       sendchat("%s chuckles a bit.\n",comstr);
       goto ENDING;
      }
  else if (strstr (command2, "giggles" ) ||
      strstr (command2, "gigglz" )) {
      sendchat("%s pinches your cheeks, you're so cute!\n",comstr);
      goto ENDING;
     }
  else if (strstr (command2, "pokes" )) {
       sendchat("%s pokes %s in the ribs.\n",comstr,namestr);
      goto ENDING;
      }

  return 1;
 }  /* end of SEMOTE and SHEMOTE */


else {
     return 1;
     }

ENDING:
return 0;

}


/*****************************************************************
 * smatch: Given a data string and a pattern containing one or
 * more embedded stars (*) (which match any number of characters)
 * return true if the match succeeds, and set res[i] to the
 * characters matched by the 'i'th *.
 *****************************************************************/

int smatch (register char *dat, register char *pat, register char **res)
{
  register char *star = 0, *starend = 0, *resp = 0;
  int nres = 0;

  while (1)
  { if (*pat == '*')
    { star = ++pat; 			     /* Pattern after * */
      starend = dat; 			     /* Data after * match */
      resp = res[nres++]; 		     /* Result string */
      *resp = '\0'; 			     /* Initially null */
    }
    else if (*dat == *pat) 		     /* Characters match */
    { if (*pat == '\0') 		     /* Pattern matches */
	return (1);
      pat++; 				     /* Try next position */
      dat++;
    }
    else
    { if (*dat == '\0') 		     /* Pattern fails - no more */
	return (0); 			     /* data */
      if (star == 0) 			     /* Pattern fails - no * to */
	return (0); 			     /* adjust */
      pat = star; 			     /* Restart pattern after * */
      *resp++ = *starend; 		     /* Copy character to result */
      *resp = '\0'; 			     /* null terminate */
      dat = ++starend; 			     /* Rescan after copied char */
    }
  }
}


/* The handle_command routine calls the appropriate functions for
   the thing just said, and the context given.  If what was said
   doesn't make sense as a command, we ignore it if it was said
   (assuming it was meant for someone else in the room).          */

struct storystate *handle_command(int user, char *command, int volume, struct storystate *story)
{
  char buffer[MAXSTRLEN];
  char buffer2[MAXSTRLEN];
  char dirbuf[FILE_NAME_LEN];
  char titlebuf[FILE_NAME_LEN];
  char author[NAME_LEN+1];
  char comm_pre[80];
  char comm_pre_act[80];
  int i,found=0;
  struct dirent *dp;
  FILE *fp;
  FILE *tfp;
  DIR *dirp;
  time_t tm;


 /* Set the communication prefixes based on location */
 if (volume == SAY || volume == EMOTE) {
	sprintf(comm_pre,"%s",SAY_COMMAND);
	sprintf(comm_pre_act,"%s",EMOTE_COMMAND);
 }
 else if (volume == WHISPER || volume == SEMOTE) {
	sprintf(comm_pre,"%s %s",TELL_COMMAND,ustr[user].name);
	sprintf(comm_pre_act,"%s %s",SEMOTE_COMMAND,ustr[user].name);
 }
 else if (volume == SHOUT || volume == SHEMOTE) {
	sprintf(comm_pre,"%s",SHOUT_COMMAND);
	sprintf(comm_pre_act,"%s",SHEMOTE_COMMAND);
 }

  if (!strcasecmp(command, "help")) 
    give_help(user, story);
  else if (story && story->writing) {
    if (strcmp(ustr[user].name, story->author) && strcmp(ustr[user].name,ROOT_ID))
    { 
      if (volume == WHISPER) 
        sendchat(".tell %s Sorry, I'm busy listening to %s's story just now...\n",
                ustr[user].name, story->author);
    }
    else {
      story = handle_writing(user, command, story);
    }
  }
  else {
    if (volume == WHISPER) {
       if (!handle_page2(user, command))
       sendchat(".tell %s I can't hear you! :-P\n", ustr[user].name);
       }
    else if (!strncasecmp(command, "delete ", 7)) {
      if (story && story->numkids > 0) {
        sendchat(".tell %s I'm already in the middle of reading a story, %s.\n",
               ustr[user].name, ustr[user].say_name);
        sendchat(".tell %s Say 'stop' to abort reading this story first.\n",
               ustr[user].name);
      }
      else {
         /* Quick way to separate a spaced title into one array */
         /* since sscanf reads up to a space                    */
         strcpy(buffer,"");
         for (i=0;i<strlen(command);++i) {
          if (i<7) continue;
          else if (i==7) buffer[0]=command[i];
          else buffer[i-7]=command[i];
          }
          buffer[i-7]=0;
          i=0;
          sprintf(buffer2,"%s",STORYDIR);
          dirp=opendir((char *)buffer2);

          if (dirp == NULL) {
            sendchat(".tell %s Can't open the story directory to delete your story\n",ustr[user].name);
            sendchat(".tell %s Please tell %s about this.\n",ustr[user].name,ROOT_ID);
            write_log(YESTIME,"d1: Can't open story directory\n");
            story = NULL;
            goto END;
            }
          while (((dp = readdir(dirp)) != NULL) && (!found))
           {
            sprintf(dirbuf,"%s",dp->d_name);
            if (!strcmp(dirbuf,buffer)) found=1;
            else dirbuf[0]=0;
           }
         (void) closedir(dirp);

        /* this if should never prove true if user typed it in full */
         if (!found) {
            sendchat(".tell %s I can't find that story to delete! Sorry.\n",ustr[user].name);
            sendchat(".tell %s Make sure you type the title in full with proper capitalization\n",ustr[user].name);
            write_log(YESTIME,"d2: Can't find story to delete\n");
            story = NULL;
            goto END;
         }

         found=0;

      /* Check if user wanting to delete is author or root */

      sprintf(buffer2, "%s/%s/a.idx", STORYDIR, dirbuf);
      if ((tfp = fopen(buffer2, "r")) != NULL) {
        if (fgets(author, MAXSTRLEN, tfp) != NULL) {
          author[strlen(author)-1] = '\0';
          }
        else write_log(YESTIME,"d3: Can't get author for story \"%s\" delete\n",dirbuf);
        }
      else write_log(YESTIME,"d4: Can't open index file for story \"%s\" delete\n",dirbuf);

      fclose(tfp);
      if (!strcmp(ustr[user].name,author) || !strcmp(ustr[user].name,ROOT_ID)) { }
      else {
          sendchat(".tell %s You must be the author to delete this story!\n",ustr[user].name);
          write_log(YESTIME,"d5: No permission to delete story \"%s\".\n",dirbuf);
          story = NULL;
          goto END;
         }
      
      /* Remove all files in directory */
          sprintf(buffer2,"%s/%s",STORYDIR,dirbuf);
          dirp=opendir((char *)buffer2);

          if (dirp == NULL) {
            sendchat(".tell %s Can't open the story's directory to delete your story\n",ustr[user].name);
            sendchat(".tell %s Please tell %s about this.\n",ustr[user].name,ROOT_ID);
            write_log(YESTIME,"d6: Can't open the story directory for story \"%s\".\n",dirbuf);
            story = NULL;
            goto END;
            }
          while (((dp = readdir(dirp)) != NULL) && (!found))
           {
            sprintf(titlebuf,"%s/%s",buffer2,dp->d_name);
            if (dp->d_name[0]!='.') {
              remove(titlebuf);
              }
            titlebuf[0]=0;
           }

          (void) closedir(dirp);

          /* Remove empty directory */
          sprintf(dirbuf,"%s/%s",STORYDIR,buffer);
          if (rmdir(dirbuf) == -1) {
            sendchat(".tell %s Could not remove empty directory..\n",ustr[user].name);
            sendchat(".tell %s Please tell %s about this.\n",ustr[user].name,ROOT_ID);
            write_log(YESTIME,"d7: Can't remove empty directory for \"%s\".\n",buffer);
            story = NULL;
            goto END;
          }

          sendchat(".tell %s Ok, I deleted that story *sigh*\n",ustr[user].name);
          sendchat(".emote deletes a story for %s.\n",ustr[user].say_name);
          sendchat(".say I'm tellin' ya, that was a really good story!\n");
          write_log(YESTIME,"Story \"%s\" deleted\n",buffer);
       } /* end of if not in story in else */
     }  /* end of delete command */
    else if (!strncasecmp(command, "execute ", 8)) {
	if (strcmp(ustr[user].name,ROOT_ID)) {
    	  if (volume == WHISPER)
	   sendchat(".say Only %s can tell me to do that! :-P\n",ROOT_ID);
	  else
	   sendchat(".tell %s Only %s can tell me to do that! :-P\n",ustr[user].name,ROOT_ID);
	  goto END;
	  }
	remove_first(command);
	sendchat(".tell %s Ok, executing \"%s\"\n",ROOT_ID,command);
	sendchat("%s\n",command);
	goto END;
	}
    else if (!strcasecmp(command, "status")) {
	sendchat(".tell %s Ny name is %s and I live in the %s\n",ustr[user].name,BOT_NAME,g_HOME_ROOM);
	sendchat(".tell %s My master is %s\n",ustr[user].name,ROOT_ID);
	sendchat(".tell %s I %s feeling vindictive today\n",ustr[user].name,g_KILLSWEAR==YES?"AM":"am NOT");
	sendchat(".tell %s I am running on code version %s\n",ustr[user].name,VERSION);
	sendchat(".tell %s Paragraph formatting is: %s\n",ustr[user].name,g_FORMATTED==YES?"ON":"OFF");
	sendchat(".tell %s Conversation logging is: %s\n",ustr[user].name,g_CONVOLOG==YES?"ON":"OFF");
	goto END;
	}
    else if (!strncasecmp(command, "killswear ", 10)) {
	if (strcmp(ustr[user].name,ROOT_ID)) {
    	  if (volume == WHISPER)
	   sendchat(".tell %s Only %s can tell me to do that! :-P\n",ustr[user].name,ROOT_ID);
	  else
	   sendchat(".say Only %s can tell me to do that! :-P\n",ROOT_ID);
	  goto END;
	  }
	remove_first(command);
		if (!strcmp(command,"on")) {
		g_KILLSWEAR=YES;
	  	if (volume == WHISPER)
		sendchat(".tell %s Ok, now i'm vindictive! MUAWHAHAHA!\n",ROOT_ID);
		else
		sendchat(".say Ok, now i'm vindictive! MUAWHAHAHA!\n");
		goto END;
		}
		else if (!strcmp(command,"off")) {
		g_KILLSWEAR=NO;
	  	if (volume == WHISPER)
		sendchat(".tell %s Ok, now i'm all forgiving! DAMN IT!\n",ROOT_ID);
		else
		sendchat(".say Ok, now i'm all forgiving! DAMN IT!\n");
		goto END;
		}
	} /* killswear */
    else if (!strncasecmp(command, "logging ", 8)) {
	if (strcmp(ustr[user].name,ROOT_ID)) {
    	  if (volume == WHISPER)
	   sendchat(".say Only %s can tell me to do that! :-P\n",ROOT_ID);
	  else
	   sendchat(".tell %s Only %s can tell me to do that! :-P\n",ustr[user].name,ROOT_ID);
	  goto END;
	  }
	remove_first(command);
	if (!strcmp(command,"on")) {
	strcpy(buffer2, LOG_FILE);
	if (!(log_fp = fopen(buffer2, "a"))) {
	  if (volume == WHISPER)
	   sendchat(".tell %s I can't open my notepad, %s!\n",ustr[user].name,ustr[user].say_name);
	  else
	   sendchat(".say I can't open my notepad!\n");
	  goto END;
	  }
	if (volume == WHISPER) {
	 sendchat(".sem %s gets out his notepad and felt tip pen.\n",ustr[user].name);
	 sendchat(".tell %s Ok, i'm ready!\n",ustr[user].name);
	}
	else {
	 sendchat(".emote gets out his notepad and felt tip pen.\n");
	 sendchat(".say Ok, i'm ready!\n");
	 }
	g_CONVOLOG=YES;
	time(&tm);
	sprintf(buffer,"*** Log started on %s",ctime(&tm));
	fputs(buffer,log_fp);
	goto END;
	}
	else if (!strcmp(command,"off")) {
	time(&tm);
	sprintf(buffer,"*** Log stopped on %s",ctime(&tm));
	fputs(buffer,log_fp);
	fclose(log_fp);
	if (volume == WHISPER) {
	 sendchat(".sem %s closes his notepad and caps his pen.\n",ustr[user].name);
	}
	else {
	 sendchat(".emote closes his notepad and caps his pen.\n");
	 }
	g_CONVOLOG=NO;
	goto END;
	}
	else if (!strcmp(command,"read")) {
	strcpy(buffer2, LOG_FILE);

	/* we could fflush the log file here to write all data to disk */
	/* before we read it, but then we'd still be logging and the   */
	/* bot would log the log he just read back to us. Instead,     */
	/* we'll just close the file, read the log, and open it back   */
	/* up again.				-Cygnus		       */
	if (g_CONVOLOG==YES) {
		fflush(log_fp);
		fclose(log_fp);
	}

	if (!(fp = fopen(buffer2, "r"))) {
	  if (volume == WHISPER)
	   sendchat(".tell %s I can't open my notepad, %s!\n",ustr[user].name,ustr[user].say_name);
	  else
	   sendchat(".say I can't open my notepad!\n");

	  goto END;
	  }

	if (volume == WHISPER) {
	 sendchat(".sem %s starts from the beginning.\n",ustr[user].name);
	}
	else {
	 sendchat(".emote starts from the beginning.\n");
	 }

	while (fgets(buffer,1000,fp) != NULL) {
	buffer[strlen(buffer)-1]=0; /* strip nl */
	if (volume == WHISPER)
		sendchat(".tell %s %s\n",ustr[user].name,buffer);
	else
		sendchat(".say %s\n",buffer);
	} /* end of while */
	fclose(fp);
	buffer[0]=0;

	/* reopen log file for continued logging */
	if (g_CONVOLOG==YES) {
	  log_fp = fopen(buffer2, "a");
	  }

	goto END;
	}
	else if (!strcmp(command,"clear")) {
	strcpy(buffer2, LOG_FILE);

	if (g_CONVOLOG==YES) {
		fflush(log_fp);
		fclose(log_fp);
	}
	remove(LOG_FILE);
	if (volume == WHISPER)
	 sendchat(".semote %s rips up the log and throws it in the garbage\n",ustr[user].name);
	else
	 sendchat(".emote rips up the log and throws it in the garbage\n");

        /* reopen log file for continued logging */
        if (g_CONVOLOG==YES) {
          log_fp = fopen(buffer2, "a");
          }

	goto END;
	} /* end of logging clear */
	else {
	if (volume == WHISPER)
	 sendchat(".tell %s I don't understand what you want me to do!\n",ustr[user].name);
	else
	 sendchat(".say I don't understand what you want me to do!\n");	 	
	goto END;
	}
    } /* end of logging command */

    else if (!strncasecmp(command, "list ", 5)) {
      if (story && story->numkids > 0)  {
        sendchat(".tell %s Sorry, I'm in the middle of a story just now.\n",
                ustr[user].name);
        sendchat(".tell %s Say 'stop' if you want me to read a different one.\n",
               ustr[user].name);
        }
      else {
		read_titles(command+5,1);
      }
    } /* end of else if */
    else if (!strcasecmp(command, "list")) {
      if (story && story->numkids > 0)  {
        sendchat(".tell %s Sorry, I'm in the middle of a story just now.\n",
                ustr[user].name);
        sendchat(".tell %s Say 'stop' if you want me to read a different one.\n",
               ustr[user].name);
        }
      else {
		read_titles(NULL,0);
      }
    } /* end of else if */
    else if (!strncasecmp(command, "read ", 5)) {
      if (story && story->numkids > 0) {
        sendchat(".tell %s I'm already in the middle of reading a story, %s.\n",
               ustr[user].name, ustr[user].say_name);
        sendchat(".tell %s Say 'stop' if you want me to read a different one.\n",
               ustr[user].name);
      }
      else {
        if (resolve_title(command+5, buffer, YES) == NULL)
          sendchat(".tell %s I don't know which story you mean!\n", ustr[user].name);
        else {
          write_log(YESTIME,"%s reads \"%s\"\n", ustr[user].say_name, buffer);
          story = start_story(buffer, story);
          sendchat(".desc %s\n", TELLMSG);
          sprintf(buffer, "%s/%s/a.idx", STORYDIR, story->title);
          if ((fp = fopen(buffer, "r")) == NULL) {
            sendchat(".say I can't find that story! Sorry.\n");
            story = NULL;
          }
          else {
            fclose(fp);
            get_paragraph(story);
            read_paragraph(story);
          }
        }
      }
    }
    else if (!strncasecmp(command, "write ", 6)) {
      if (story && story->numkids > 0) {
        sendchat(".tell %s I'm already in the middle of reading a story, %s.\n",
               ustr[user].name, ustr[user].say_name);
        sendchat(".tell %s Say 'stop' if you want to tell me a new one.\n",
               ustr[user].name);
      }
      else if (!isalpha((int)command[6]) && !isdigit((int)command[6]))
        sendchat(".tell %s Titles must start with a letter or digit.\n",
                ustr[user].name);
      else {
        if (islower((int)command[6]))
          command[6] = toupper(command[6]);
        if (resolve_title(command+6, buffer, NO) == NULL) {
          story = start_story(command+6, story);
          sendchat(".desc %s %s.\n", LISTENMSG, strip_color(ustr[user].say_name));
          sendchat(";turns his attention to %s and their story.\n", ustr[user].say_name);
          strcpy(story->author, ustr[user].name);
          story->writing = YES;
          story->editing = NO;
        }
        else 
          sendchat(".tell %s That title is already taken.  Try a different one.\n",
                 ustr[user].name);
      }
    }
    else if (!strcasecmp(command, "reread")) {
      if (story) 
        read_paragraph(story);
      else
        sendchat(".tell %s I'm not reading any story at the moment.\n", 
               ustr[user].name);
    }
    else if (!strcasecmp(command, "next") || !strcasecmp(command, "n")) {
      if (story && story->numkids > 0) 
        read_next(NULL, story);
      else
        sendchat(".tell %s There is no next paragraph.\n", ustr[user].name);
    }
    else if (!strncasecmp(command, "next ", 5)) {
      if (story && story->numkids > 0) 
        read_next(command+5, story);
      else
        sendchat(".tell %s There is no next paragraph.\n", ustr[user].name);
    }
    else if (!strncasecmp(command, "n ", 2)) {
      if (story && story->numkids > 0) 
        read_next(command+2, story);
      else
        sendchat(".tell %s There is no next paragraph.\n", ustr[user].name);
    }
    else if (!strcasecmp(command, "back") || !strcasecmp(command, "b")) {
      if (story && strlen(story->paragraphid) > 1)
        read_previous(story);
      else
        sendchat(".tell %s There is no previous paragraph.\n", ustr[user].name);
    }
    else if (!strcasecmp(command, "stop")) {
      if (story) {
        story = finish_story(story);
        sendchat(";puts aside the story he was reading.\n");
        sendchat(".desc %s\n", IDLEMSG);
      }
      else
        sendchat(".tell %s But I'm not reading any story now!\n", ustr[user].name);
    }
    else if (!strcasecmp(command, "format")) {
      if (g_FORMATTED) {
        g_FORMATTED = NO;
        sendchat(".say Paragraph formatting turned off.\n");
      }
      else {
        g_FORMATTED = YES;
        sendchat(".say Paragraph formatting turned on.\n");
      }
    }
    else if (!strcasecmp(command, "add")) {
      for (i = 0; i < story->numkids && strcmp(story->kids[i], ustr[user].name); i++)
        ;
      if (i < story->numkids)
        sendchat(".tell %s You already have a branch of this paragraph.\n",
                ustr[user].name);
      else {
        strcpy(buffer, story->paragraphid);
        sprintf(buffer+strlen(buffer), "%c", 'a' + story->numkids);
        
        for (i = 0; i < story->numlines; i++)
          free(story->paragraph[i]);
        for (i = 0; i < story->numkids; i++)
          free(story->kids[i]);
        story->numlines = 0;
        story->numkids = 0;
        strcpy(story->paragraphid, buffer);
        strcpy(story->author, ustr[user].name);
        sendchat(".desc %s %s.\n", LISTENMSG, strip_color(ustr[user].say_name));
        sendchat(";turns his attention to %s and their story addition.\n",
               ustr[user].say_name);
        story->writing = YES;
        story->editing = NO;
      }
    }
    else if (!strcasecmp(command, "edit")) {
      if (strcmp(ustr[user].name, story->author)
	&& strcmp(ustr[user].name, ROOT_ID))
      {
        sendchat(".tell %s You can't edit someone else's paragraph.\n", 
                ustr[user].name);
        sendchat(".tell %s If you want to write a different version of this\n",
               ustr[user].name);
        sendchat(".tell %s paragraph, go 'back' and then 'add'.\n",
               ustr[user].name);
      }
      else {
        sendchat(".desc %s %s.\n", LISTENMSG, strip_color(ustr[user].say_name));
        sendchat(".tell %s Starting edit at end of paragraph.\n", ustr[user].name);
        sendchat(".tell %s To start the paragraph over instead, say 'clear'.\n",
               ustr[user].name);
        sendchat(";turns his attention to %s for a paragraph edit.\n",
                ustr[user].say_name);
        story->writing = YES;
        story->editing = YES;
      }
    }
  }

END:
  return(story);
}  

/* Handle_writing handles all the things an author can do while writing. */

struct storystate *handle_writing(int user, char *command, struct storystate *story)
{
  int i;
  char buffer[MAXSTRLEN];

  if (strlen(command) == 0) {
    if (story->numlines == 0) {           /* Author is done */
      sendchat(".tell %s Nice listening to you.\n", ustr[user].name);
      story = abort_writing(story);
      write_log(YESTIME,"Done with abort writing1.\n");
    }
    else {                                /* Save this paragraph */
      /* Foist, create a directory if we need one */
      write_log(YESTIME,"Setting up new directory..\n");
      if (!setup_directory(story)) {
        sendchat(".tell %s Sorry, I'm unable to start saving your story!\n",
               ustr[user].name);
        story = abort_writing(story);
       write_log(YESTIME,"Done with abort writing2.\n");
      }
      else if (!(save_para(story, user))) {
        sendchat(".tell %s This just isn't my day...\n", ustr[user].name);
        story = abort_writing(story);
      write_log(YESTIME,"Done with abort writing3.\n");
      }
      /* If we're just "editing", we're done. */
      else if (story->editing) {
        sendchat(".tell %s Paragraph edited. Thank you for making corrections.\n", 
		ustr[user].name);
        sendchat(";has finished listening to %s's paragraph.\n", ustr[user].say_name);
        story->writing = NO;
        if (story->numkids == 0) 
          sendchat(".desc %s\n", IDLEMSG);
        else 
          sendchat(".desc %s\n", TELLMSG);
      }
      /* If not, start a new para! */
      else {
        sendchat(";logs %s's paragraph.\n", ustr[user].say_name);
        sprintf(buffer, "%s%c", story->paragraphid, 'a' + story->numkids);
	strcat(buffer,"\0"); 
       /* Above line ASSUMES ASCII */
        for (i = 0; i < story->numlines; i++)
          free(story->paragraph[i]);
        for (i = 0; i < story->numkids; i++)
          free(story->kids[i]);
        story->numlines = 0;
        story->numkids = 0;
        strcpy(story->paragraphid, buffer);
      }
    }
  }
  else if (!strcasecmp(command, "delete")) {
    if (story->numlines == 0)
      sendchat(".tell %s There are no lines in this paragraph to delete.\n",
             ustr[user].name);
    else {
      sendchat(".tell %s Deleting: %s\n", 
             ustr[user].name, story->paragraph[--(story->numlines)]);
      free(story->paragraph[story->numlines]);
    }
  }
  else if (!strcasecmp(command, "abort")) {
    sendchat(".tell %s Paragraph aborted.  Nice listening to you.\n", ustr[user].name);
    story = abort_writing(story);
      write_log(YESTIME,"Done with abort writing4.\n");
  }
  else if (!strcasecmp(command, "format")) {
    sendchat(".tell %s 'Format' commands have no effect now.\n", ustr[user].name);
    sendchat(".tell %s (Formatting is always turned off while I'm writing.)\n",
            ustr[user].name);
  }
  else if (!strcasecmp(command, "clear")) {
    if (story->numlines == 0) {
      sendchat(".tell %s The paragraph is already empty.\n", ustr[user].name);
      sendchat(".tell %s If you wish to quit, say 'abort'.\n", ustr[user].name);
    }
    else {
      sendchat(".tell %s Clearing entire paragraph contents.\n", ustr[user].name);
      for (i = 0; i < story->numlines; i++)
        free(story->paragraph[i]);
      story->numlines = 0;
    }
  }
  else if (!strcasecmp(command, "reread")) {
    for (i = 0; i < story->numlines; i++)
      sendchat(";: %s\n", story->paragraph[i]);
    sendchat(";pauses.\n");
  }
  else {                           /* This is the next line of the story */
    if (story->numlines + 2 == MAXLINES) {
      sendchat(".tell %s Your paragraph is too big.  Please end it.\n",
             ustr[user].name);
      sendchat(".tell %s (An empty say (with just .say) ends a paragraph.)\n",
              ustr[user].name);
    }
    else {
      story->paragraph[story->numlines] =
                       (char *) malloc((strlen(command)+1)*sizeof(char));
      strcpy(story->paragraph[(story->numlines)++], command);
    }
  }
  return(story);
}

/* setup_directory makes sure that if this is a new story, a story
   directory exists.  Returns nonzero on success.                      */

int setup_directory(struct storystate *story)
{
  char buffer[MAXSTRLEN];

  sprintf(buffer, "%s/%s", STORYDIR, story->title);
  if (strlen(story->paragraphid) > 1 || story->editing)
    return 1;
  if (mkdir(buffer, 0700) == -1)
    return 0;
  else
    return 1;
}

/* save_para attempts to save a paragraph of the story, and then update
    the appropriate index files.  Returns positive if successful.        */
int save_para(struct storystate *story, int user)
{
  char buffer[MAXSTRLEN];
  FILE *fp;
  int i;

  sprintf(buffer, "%s/%s/%s.txt", STORYDIR, story->title, story->paragraphid);
  if (!(fp = fopen(buffer, "w"))) {
    sendchat(".tell %s Sorry, I can't save this paragraph!\n",ustr[user].name);
    write_log(YESTIME,"Paragraph save error %s\n",get_error());
    return 0;
  }
  for (i = 0; i < story->numlines; i++)
    fprintf(fp, "%s\n", story->paragraph[i]);
  fclose(fp);
  write_log(YESTIME,"%s saves paragraph in %s\n", strip_color(ustr[user].say_name), story->title);
  if (!(story->editing)) {
    sprintf(buffer, "%s/%s/%s.idx", STORYDIR, story->title, story->paragraphid);
    if (!(fp = fopen(buffer, "w"))) {
      sendchat(".tell %s I can't save this paragraph! (index creation problem)\n",
              ustr[user].name);
      write_log(YESTIME,"Index creation error for \"%s\" %s\n",story->title,get_error());
      return 0;
    }
    fprintf(fp, "%s\n", ustr[user].name);
    fclose(fp);
    if (strlen(story->paragraphid) > 1) {     /* Update previous index file */
      sprintf(buffer, "%s/%s/", STORYDIR, story->title);
      strncat(buffer, story->paragraphid, strlen(story->paragraphid)-1);
      strcat(buffer, ".idx");
      if (!(fp = fopen(buffer, "a"))) {
        sendchat(".tell %s I can't save this paragraph! (index update problem)\n",
                ustr[user].name);
        write_log(YESTIME,"Index update error for \"%s\" %s\n",story->title,get_error());
        return 0;
      }
      fprintf(fp, "%s\n", ustr[user].name);
      fclose(fp);
      write_log(YESTIME,"Index file for \"%s\" updated.\n", story->title);
    }
    else {                                    /* Make new directory file */
	/*
      sprintf(buffer, "/bin/ls %s > %s", STORYDIR, DIRECTORY);
      system(buffer);
      write_log(YESTIME,"New directory created.\n");
	*/
    }
  }
  return 1;
}

/* Abort_writing cleans up gracefully after we abort an edit */

struct storystate *abort_writing(struct storystate *story)
{
  story->writing = NO;
  write_log(YESTIME,"In abort_writing for \"%s\"\n",story->title);
  if (story->editing) {
    sendchat(";has finished listening to %s's revision.\n", story->author); 
    get_paragraph(story);
      write_log(YESTIME,"Story revision completed for \"%s\".\n",story->title);
    if (story->numkids)
      sendchat(".desc %s\n", TELLMSG);
  }
  else {
    sendchat(";has finished listening to %s's story.\n", story->author); 
    story = finish_story(NULL);
    sendchat(".desc %s\n", IDLEMSG);
    write_log(YESTIME,"New story or addition completed for \"%s\".\n",story->title);
  }
  write_log(YESTIME,"Leaving abort_writing\n");
  return story;
}

/* Give_help gives appropriate help for different story states. */

void give_help(int user, struct storystate *story)
{
  sendchat("; passes %s a note.\n", ustr[user].say_name);
  sendchat(".tell %s I am now ", ustr[user].name); 
  if (story && story->writing) {
    if (strcmp(ustr[user].name, story->author) && strcmp(ustr[user].name,ROOT_ID)) {
      sendchat("listening to %s's story.\n", story->author);
      sendchat(".tell %s I can't be of much help until they finish.\n",ustr[user].name);
    }
    else {
      sendchat("listening to your story.\n");
      sendchat(".tell %s I take anything you say as part of the story, except: \n", ustr[user].name);
      sendchat(".tell %s reread - read back the paragraph so far.\n",ustr[user].name);
      sendchat(".tell %s (paragraph formatting will be OFF in this case)\n",ustr[user].name);
      sendchat(".tell %s delete - delete last line in paragraph.\n",ustr[user].name);
      sendchat(".tell %s clear - delete entire paragraph.\n", ustr[user].name);
      sendchat(".tell %s abort - end this writing session.\n", ustr[user].name);
      sendchat(".tell %s help - give context-sensitive help\n",ustr[user].name);
      sendchat(".tell %s <blank line> (just .say)- end this paragraph.\n",ustr[user].name);
      sendchat(".tell %s (this saves the paragraph if anything's in it,\n", ustr[user].name);
      sendchat(".tell %s and ends this writing session if it's empty.\n", 
              ustr[user].name);
      sendchat(".tell %s Thus, 2 blank lines save the previous para. and quit.)\n", 
              ustr[user].name);
    }
  }
  else if (story == NULL || !(story->numkids)) { 
    sendchat("ready to tell a story or hear one of yours.\n");
    sendchat(".tell %s Useful selection commands are:\n", ustr[user].name);
    sendchat(".tell %s read <story title> - start reading a story\n",ustr[user].name);
    sendchat(".tell %s write <story title> - start recording your story\n", ustr[user].name);
    sendchat(".tell %s delete <story title> - delete a story of yours\n",ustr[user].name);
    sendchat(".tell %s list - list the stories I know\n", ustr[user].name);
    sendchat(".tell %s list <author> - list the stories I know by <author>\n", ustr[user].name);
    sendchat(".tell %s format - turn paragraph formatting %s\n",
            ustr[user].name, (g_FORMATTED==YES?"on" : "off"));
    sendchat(".tell %s help - give context-sensitive help\n", ustr[user].name);
 if (!strcmp(ustr[user].name,ROOT_ID)) {
    sendchat(".tell %s execute <command> - run a talker command\n",ustr[user].name);
    sendchat(".tell %s logging [on|off|read|clear] - log conversation i see to a file \"%s\" for later reading, etc..\n",ustr[user].name,LOG_FILE);
    }
    sendchat(".tell %s For more documentation, read 'Using %s'.\n", 
            ustr[user].name,BOT_NAME);
  }
  else {
    sendchat("in the middle of telling a story.\n");
    sendchat(".tell %s Useful reading commands are:\n",ustr[user].name);
    sendchat(".tell %s n or next - read the next paragraph\n",ustr[user].name);
    sendchat(".tell %s n or next <authorname> - read the author's next paragraph\n", ustr[user].name);
    sendchat(".tell %s b or back - read the previous paragraph\n",ustr[user].name);
    sendchat(".tell %s reread - read the current paragraph again\n",ustr[user].name);
    sendchat(".tell %s stop - don't read any more of this story\n",ustr[user].name);
    sendchat(".tell %s format - turn paragraph formatting %s\n",
            ustr[user].name, (g_FORMATTED==YES?"on" : "off"));
    sendchat(".tell %s add - add a branch or extension to this story\n", 
            ustr[user].name);
    sendchat(".tell %s edit - edit current paragraph (if yours)\n", ustr[user].name);
    sendchat(".tell %s (Add and edit halt my storytelling until you finish\n", ustr[user].name);
    sendchat(".t %s adding your part; it's polite to check with other people\n", 
            ustr[user].name);
    sendchat(".t %s in the room before you do this.)\n", ustr[user].name);
    sendchat(".t %s help - give context-sensitive help\n", ustr[user].name);
  }
}

/* read_titles simply reads the directory file, reporting the names
    and authors of the stories it finds there.                          */

void read_titles(char *bitmatch, int mode)
{
  char *upperbit = 0;
  char buffer[MAXSTRLEN], titlebuf[MAXSTRLEN], author[MAXSTRLEN]; 
  char buffer2[30], chunk1[4], chunk2[3], chunk3[3];
  char lsdir[FILE_NAME_LEN];
  static int breakup=0;
  static int abreakup=0;
  int storynum=0, total_stories=0;
  struct stat fileinfo;
  FILE *tfp;
  struct dirent *dp;
  DIR *dirp;

  if (mode==1) abreakup+=15;
  else breakup+=15;

	sprintf(lsdir,"%s/%s",DIRECTORY,STORYDIR);
	dirp=opendir((char *)lsdir);

  if (dirp == NULL) {
    sendchat(".say I'm sorry, but I can't get to my story directory!\n");
    sendchat(".say %s should know about this-- this shouldn't happen.\n",ROOT_ID);
    return;
  }
  else {
   if (mode==1) {
    if (!strlen(bitmatch)) {
      sendchat(".say Whose storylist do you want to see?\n");
      abreakup=0;
      return;
      }
    upperbit=bitmatch;
    strtolower(bitmatch);
    /* get number of stories by author first */
    while ((dp = readdir(dirp)) != NULL) {
	sprintf(titlebuf,"%s",dp->d_name);
	if (titlebuf[0]=='.') continue;
      /* titlebuf[strlen(titlebuf)-1] = '\0'; */
      sprintf(buffer, "%s/%s/a.idx", STORYDIR, titlebuf);
      if ((tfp = fopen(buffer, "r")) != NULL) {
        if (fgets(author, MAXSTRLEN, tfp) != NULL) {
          author[strlen(author)-1] = '\0';
	  if (!strcmp(bitmatch,author))
		total_stories++;
        } /* end of if fgets author */
        fclose(tfp);
      } /* end of if fopen */
    } /* end of while */
    if (!total_stories) {
      sendchat(".say I don't know any stories by ^%s^!\n", upperbit);
      abreakup=0;
      return;
     }
   } /* end of if bitmatch */
   else {
    while ((dp = readdir(dirp)) != NULL) {
	sprintf(titlebuf,"%s",dp->d_name);
	if (titlebuf[0]=='.') continue;
		total_stories++;
    }
   } /* end of bitmatch else */
  } /* end of directory read else */
  (void)closedir(dirp);

	sprintf(lsdir,"%s/%s",DIRECTORY,STORYDIR);
	dirp=opendir((char *)lsdir);

  if (dirp == NULL) {
    sendchat(".say I'm sorry, but I can't get to my story directory!\n");
    sendchat(".say %s should know about this-- this shouldn't happen.\n",ROOT_ID);
  }
  else {
   if (mode==1) {
    if (total_stories==1)
    sendchat(".say I know ^HG%d^ story by ^HM%s^\n",total_stories,
upperbit);
    else
    sendchat(".say I know ^HG%d^ stories by ^HM%s^, here are ^%d^ through ^%d^:\n",total_stories, upperbit, 
		abreakup-14, ( total_stories < abreakup ? total_stories : abreakup ) );

    while ((dp = readdir(dirp)) != NULL) {
	sprintf(titlebuf,"%s",dp->d_name);
	if (titlebuf[0]=='.') continue;
      /* titlebuf[strlen(titlebuf)-1] = '\0'; */
      sprintf(buffer, "%s/%s/a.idx", STORYDIR, titlebuf);
      if ((tfp = fopen(buffer, "r")) != NULL) {
	/* Get last changed info */
      sprintf(buffer2, "%s/%s", STORYDIR, titlebuf);
      stat(buffer2,&fileinfo);
      sprintf(buffer2,"%s",ctime(&fileinfo.st_mtime));
      midcpy(buffer2,chunk1,4,6);
      midcpy(buffer2,chunk2,8,9);
      midcpy(buffer2,chunk3,22,23);
      if (chunk2[0]==' ') midcpy(chunk2,chunk2,1,1);

        if (fgets(author, MAXSTRLEN, tfp) != NULL) {
          author[strlen(author)-1] = '\0';
          if (!strcmp(bitmatch,author)) {
		if (++storynum >= abreakup-14  &&  storynum <= abreakup )
		 sendchat(".say %s (^HM%s^)  [^HG%s-%s-%s^]\n", titlebuf,author,chunk1,chunk2,chunk3);
		chunk1[0]=0; chunk2[0]=0; buffer2[0]=0;
          }
        } /* end of if fgets author */
        fclose(tfp);
      } /* end of if fopen */
    } /* end of while */
   }
   else {
    if (total_stories==1)
    sendchat(".say I know ^HG%d^ story.\n",total_stories);
    else
    sendchat(".say I know ^HG%d^ stories, here are ^%d^ through ^%d^:\n",total_stories, 
		breakup-14, ( total_stories < breakup ? total_stories : breakup ) );

    while ((dp = readdir(dirp)) != NULL) {
	sprintf(titlebuf,"%s",dp->d_name);
	if (titlebuf[0]=='.') continue;
      /* titlebuf[strlen(titlebuf)-1] = '\0'; */
      sprintf(buffer, "%s/%s/a.idx", STORYDIR, titlebuf);
      if ((tfp = fopen(buffer, "r")) != NULL) {
	/* Get last changed info */
      sprintf(buffer2, "%s/%s", STORYDIR, titlebuf);
      stat(buffer2,&fileinfo);
      sprintf(buffer2,"%s",ctime(&fileinfo.st_mtime));
      midcpy(buffer2,chunk1,4,6);
      midcpy(buffer2,chunk2,8,9);
      midcpy(buffer2,chunk3,22,23);
      if (chunk2[0]==' ') midcpy(chunk2,chunk2,1,1);

        if (fgets(author, MAXSTRLEN, tfp) != NULL) {
          author[strlen(author)-1] = '\0';
			if (++storynum >= breakup-14  &&  storynum <= breakup )
			 sendchat(".say %s (^HM%s^)  [^HG%s-%s-%s^]\n", titlebuf,author,chunk1,chunk2,chunk3);
			chunk1[0]=0; chunk2[0]=0; buffer2[0]=0;

        } /* end of if fgets author */
        fclose(tfp);
      } /* end of if fopen */
    } /* end of while */
   } /* end of bitmatch else */
   if (mode==1) {
	if (abreakup >= total_stories) {
		sendchat(".say '^LIST %s^' to start over at the beginning of the list...\n", bitmatch);
		abreakup=0;
	} else {
		sendchat(".say '^LIST %s^' for more story titles...\n", bitmatch);
	}
     }
   else {
	if (breakup >= total_stories) {
		sendchat(".say ^LIST^ to start over at the beginning of the list...\n");
		breakup=0;
	} else {
		sendchat(".say ^LIST^ for more story titles...\n");
	}
     }
  } /* end of main else */
  (void)closedir(dirp);

}

/* resolve_title read the directory file, looking for a story which
   is uniquely identified by the prefix the user gave (if acceptprefix
   if YES), or by the exact title (if acceptprefix is NO).
   If no such story exists, or if the prefix is ambiguous, we return NULL.
   (If, however, the prefix exactly matches a title, we return that.) */

char *resolve_title(char *prefix, char *buffer, int acceptprefix)
{
  char inbuf[MAXSTRLEN];
  char tmp[FILE_NAME_LEN];
  int storymatch;
  int exactmatch;
  struct dirent *dp;
  DIR *dirp;

  exactmatch = NO;
  if (strlen(prefix) == 0)
    return NULL;
  storymatch = 0;
  sprintf(tmp,"%s/%s",DIRECTORY,STORYDIR);
	dirp=opendir((char *)tmp);

  if (dirp == NULL) {
    sendchat(".say I'm sorry, but I can't get to my story directory!\n");
    sendchat(".say %s should know about this-- this shouldn't happen.\n",ROOT_ID);
  }
  else {
    while ((dp = readdir(dirp)) != NULL && !exactmatch) {
	sprintf(inbuf,"%s",dp->d_name);
	if (inbuf[0]=='.') continue;
      /* inbuf[strlen(inbuf)-1] = '\0'; */
      if (!strncasecmp(prefix, inbuf, strlen(prefix))) {
        if (acceptprefix)
          storymatch++;
        strcpy(buffer, inbuf);
        if (!strcasecmp(buffer, prefix))
          exactmatch = YES;
      } /* end of sub if */
    } /* end of while */
  } /* end of else */
  (void)closedir(dirp);
  if (storymatch == 1 || exactmatch)
    return buffer;
  else
    return(NULL);
}

/* start_story initializes a new story structure.            */

struct storystate *start_story(char *title, struct storystate *story)
{
  int i;
  finish_story(story);
  story = (struct storystate *) malloc(sizeof(struct storystate));
  strcpy(story->title, title);
  strcpy(story->author, "");         /* Will be reset in a little while. */
  strcpy(story->paragraphid, "a");
  for (i = 0; i < MAXLINES; i++)
    story->paragraph[i] = NULL;
  story->writing = 0;                /* Will be reset if we're writing   */
  story->numkids = 0;
  story->numlines = 0;
  return story;
}

/* Get_paragraph reads in the necessary information about the paragraph
   designated by story->paragraphid, and updates the structure appropriately.

   The format for a .idx file is:
     Author of current paragraph
     Author of child paragraph a (if any)
     Author of child paragraph b (if any)
       ...
*/

void get_paragraph(struct storystate *story)
{
  FILE *fp;
  char filename[MAXSTRLEN], inbuf[MAXSTRLEN];
  int i;

  sprintf(filename, "%s/%s/%s.idx", STORYDIR, story->title, story->paragraphid);
  if (!(fp = fopen(filename, "r"))) {
    sendchat(".say Hey! I can't retrieve index file for next paragraph!\n");
    write_log(YESTIME,"idx lookup failure! %s\n",get_error());
    crash_n_burn();
    quit_robot();
  }
  else {
    for (i = 0; i < story->numkids; i++)
      free(story->kids[i]);
    story->numkids = 0;
    fgets(inbuf, MAXSTRLEN, fp);
    inbuf[strlen(inbuf)-1] = '\0';
    if (strcmp(story->author, inbuf)) {
      sendchat(".say (%s wrote this part of the story.)\n", inbuf);
      strcpy(story->author, inbuf);
    }
    for (i = 0; fgets(inbuf, MAXSTRLEN, fp) != NULL; i++) {
      inbuf[strlen(inbuf)-1] = '\0';
      story->kids[i] = (char *) malloc((strlen(inbuf)+1) * sizeof(char));
      strcpy(story->kids[i], inbuf);
    }
    story->kids[i] = NULL;
    story->numkids = i;
  }
  fclose(fp);
  if (story->numkids == 0) 
    sendchat(".desc %s\n", IDLEMSG);

  sprintf(filename, "%s/%s/%s.txt", STORYDIR, story->title, story->paragraphid);
  if (!(fp = fopen(filename, "r"))) {
    sendchat(".say Hey! I can't retrieve text file for next paragraph!\n");
    write_log(YESTIME,"txt lookup failure! %s\n",get_error());
    crash_n_burn();
    quit_robot();
  }
  else {
    for (i = 0; i < story->numlines; i++)
      free(story->paragraph[i]);
    for (i = 0; fgets(inbuf, MAXSTRLEN, fp) != NULL; i++) {
      inbuf[strlen(inbuf)-1] = '\0';
      story->paragraph[i] = (char *) malloc((strlen(inbuf)+1) * sizeof(char));
      strcpy(story->paragraph[i], inbuf);
    }
    story->paragraph[i] = NULL;
    story->numlines = i;
  }
  fclose(fp);
}

/* read_next(author, story) goes on to the 'next' paragraph by 
   that author.  If author is NULL, it goes on to the next
   paragraph by the same author as the current paragraph, or
   if there is no such paragraph, to the first paragraph
   which was written to follow this one.

   If the author doesn't match, we pretend we never heard the cmd       */

void read_next(char *author, struct storystate *story)
{
  int i;
  char c = '\0';
  int exactmatch;

  if (author == NULL) {
    for (i = 0; i < story->numkids; i++) {
      if (!strcasecmp(story->author, story->kids[i]))
        c = 'a' + i;                               /* ASSUMES ASCII */
    }
    if (c == '\0')
        c = 'a';
  }
  else {
    exactmatch = 0;
    for (i = 0; i < story->numkids && !exactmatch; i++) {
      if (!strncasecmp(author, story->kids[i], strlen(author)))
        c = 'a' + i;                               /* ASSUMES ASCII */
      if (!strcasecmp(author, story->kids[i]))
        exactmatch = YES;
    }
  }
  if (c != '\0') {
	/*** NULL ***/
    sprintf(story->paragraphid+strlen(story->paragraphid), "%c", c);
    get_paragraph(story);
    read_paragraph(story);
  }
}

/* read_previous(story) backs up a paragraph.  We've already
   ascertained that there is a paragraph to back up onto.     */

void read_previous(struct storystate *story)
{
  story->paragraphid[strlen(story->paragraphid)-1] = '\0';
  get_paragraph(story);
  read_paragraph(story);
}


/* read_paragraph reads the paragraph currently in memory. */
/* Formatting, if asked for, is applied.                   */

void read_paragraph(struct storystate *story)
{
  int i, j;
  int col;
  int tokenlen;
  char *token;

  col = 0;	
  if (!(story->writing) && (g_FORMATTED==YES)) {
    for (i = 0; i < story->numlines; i++) {
      token = story->paragraph[i];
      while (*token != '\0') {
        if (isspace((int)*token))
          token++;
        else {
          if (!col)
            sendchat(";: ");
          tokenlen = 0;
          while (token[tokenlen] != '\0' && !isspace((int)token[tokenlen]))
            tokenlen++;
          if ((col + tokenlen) > FORMATMAX) {
            col = 0;
            sendchat("\n;: ");
          }
          for (j = 0; j < tokenlen; j++) {
           /* putchar(token[j]); */
           sprintf(mess,"%c",token[j]);
           sendchat(mess);
           }
          /* putchar(' '); */
          strcpy(mess," ");
          sendchat(mess);
          col += (tokenlen + 1);
          token += tokenlen;
        }
      }
    }
    sendchat("\n");
  }
  else {
    for (i = 0; i < story->numlines; i++) 
        sendchat(";: %s\n", story->paragraph[i]); 
  }
  if (story->numkids > 0) {
    if (story->numkids > 1 || strcasecmp(story->author, story->kids[0])) {
      sendchat(".say Following paragraphs by: ");
      for (i = 0; i < story->numkids; i++) {
        sendchat("%s", story->kids[i]); 
        if (i == story->numkids - 2)
          sendchat(" and ");
        else if (i < story->numkids - 2)
          sendchat(", ");
      }
      sendchat("\n"); 
    }
    sendchat(";pauses.\n");
  }
  else
    sendchat(";has finished the story.\n");
}

/* Finishstory frees up the story data structure */

struct storystate *finish_story(struct storystate *story)
{
  int i;

  if (story != NULL) {
    for (i = 0; i < story->numlines; i++) {
      if (story->paragraph[i] != NULL)
        free(story->paragraph[i]);
    }
    for (i = 0; i < story->numkids; i++) {
      if (story->kids[i] != NULL)
        free(story->kids[i]);
    }
    free(story);
  }
  return NULL;
}

/* sync_bot sends a garbage message and waits for the HUH? to     */
/* come back. This is useful for synchronizing dialogue.          */

void sync_bot(void)
{

}

/* handle_page2 takes a paging line, and pages back a message giving
    StoryBot's location.                                           */
int handle_page2(int user, char *pageline)
{
  int chance;

  if ((!strcmp(pageline, ""))  ||
      (!strcmp(pageline, "directions")) ||
      (!strcmp(pageline, "Directions")) ||
      (!strcmp(pageline, "Where are you?")) ||
      (!strcmp(pageline, "Where are you")) ||
      (!strcmp(pageline, "where are you?")) ||
      (!strcmp(pageline, "where are you")) ||
      (!strcmp(pageline, "Where")) ||
      (!strcmp(pageline, "where")) ) {
      sendchat(".tell %s Hello, %s! You can find me from the %s %s\n",
              ustr[user].name, ustr[user].say_name, MAIN_ROOM, directions);
     return 1;
    }
  else if (!strcmp(pageline,"shutdown") && !strcmp(ustr[user].name,ROOT_ID)) {
     sendchat(".tell %s Shutting down..\n",ustr[user].name);
     chance = rand() % 2;
     if (chance==1)
     sendchat(".shout See ya all! %s told me to go home..the nerve!\n",ustr[user].say_name);
     else
     sendchat(".shemote moons everyone!\n");

     sendchat(".shemote spins around in a daze, singing 'Daisy'.\n");
     sendchat(".shemote sings more and more slowly, till he finally shuts down.\n");
     sendchat("%s\n", DCONMSG);
     quit_robot();
    }
  else if (!strcmp(pageline,"reboot") && !strcmp(ustr[user].name,ROOT_ID)) {
     sendchat(".tell %s Rebooting..\n",ustr[user].name);
     sendchat(".shout See ya all in a bit! %s told me to reboot\n",ustr[user].say_name);
     sendchat(".shemote spins around in a daze, singing 'Daisy'.\n");
     sendchat(".shemote sings more and more slowly, till he finally shuts down.\n");
     sendchat("%s\n", DCONMSG);
     reboot_robot();
    }
 else {
  return 0;
  }

return 1;
}

void handle_page(char *pageline)
{
  char player[MAXSTRLEN];
  char c;

  player[0] = '\0';

  if ((sscanf(pageline, "%s", player) == 1)  ||
      (sscanf(pageline, "%s tells you%c", player,  &c) == 2)) {
    write_log(YESTIME,"Paged by %s\n", player);
    if (strcasecmp(player, "Gardener")) {
      sendchat(".tell %s Hello, %s! You can find me from the %s %s\n",
              player, player, MAIN_ROOM, directions);
    }
  }
}


/* Getline takes a line from specified input and stuffs it into inbuf. */
/* Inbuf is NULL if EOF reached.                                       */

char *getline(char *inbuf, FILE *fp)
{
  char c;
  int i = 0;

  while ((c = getc(fp)) != '\n' &&  c != EOF && i < MAXSTRLEN - 2)
    inbuf[i++] = c;
  inbuf[i] = '\0';
  if (c == EOF)
    inbuf = NULL;
  else if ((strncmp(inbuf, "ifejf", 5)) ||
           (sscanf(inbuf, "%*s fakes you --%c", &c) ==1)) 
    /* handle_page(inbuf); */
  return(inbuf);

return(inbuf);
}

/* crash_n_burn handles panics.  We hope this never gets called.  */
void crash_n_burn(void)
{
   sendchat(".desc %s\n", SHUTMSG);
   sendchat(".go\n");
   sendchat(";suddenly starts to panic!\n");
   sendchat(";spins around in a daze, singing 'Daisy'.\n");
   sendchat(";sings more and more slowly, till it finally shuts down.\n");
   sendchat(";is broken.  %s should hear about this.\n",ROOT_ID);
   sendchat("%s\n", DCONMSG);
}


/*** convert string to lower case ***/
void strtolower(str)
char *str;
{
while(*str)
  {
    *str=tolower(*str);
    str++;
  }
}

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

/*** removes first word at front of string and moves rest down ***/
void remove_first(inpstr)
char *inpstr;
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


void load_user(void)
{
}

void add_user(void)
{
}

void clear_all_users(void)
{
int u;

for (u=0;u<MAX_USERS;++u) {
	ustr[u].name[0]=0;
	ustr[u].say_name[0]=0;
	ustr[u].room[0]=0;
	ustr[u].vis=0;
	ustr[u].logon=0;
	}
}

void clear_user(int user)
{
ustr[user].name[0]=0;
ustr[user].say_name[0]=0;
ustr[user].room[0]=0;
ustr[user].vis=0;
ustr[user].logon=0;
}

int get_user_num(char *name)
{
int u;
char tempname[SAYNAME_LEN];

strcpy(tempname,strip_color(name));
strtolower(tempname);

for (u=0;u<MAX_USERS;++u) {
        if (!strcmp(ustr[u].name,tempname))
        return u;
	}

return(-1);

}

int find_free_slot(void)
{
int u;

   for (u=0;u<MAX_USERS;++u)
     {
      if (!strlen(ustr[u].name)) {
        return u;
	}
     }
  return -1;

}

char *get_error(void)
{
static char errstr[FILE_NAME_LEN];

sprintf(errstr,"(%d:%s)",errno,strerror(errno));
return errstr;
}


/* Strip colors from string */
char *strip_color(char *str)
{
char tp[3];
static char tempst[500];
int  left=strlen(str);
int  i, count=0;

tempst[0]=0;

for(i=0; i<left; i++) {
        if (str[i]==' ') {
                strcat(tempst, " ");
                continue;
                }
      if (str[i]=='@') { 
                i++;
                if (str[i]=='@') {
                 count=0; continue;
                }
                else { i--;
                       tp[0]=str[i]; tp[1]=0;
                       strcat(tempst,tp);
                       continue;
                     }
                }
        if (str[i]=='^') {
                if (count) {
                        count=0;
                        continue;
                        }
                else {
                        count=1;
                        i++;
                         if (i == left) {
                            count=0;
                            break;
                           }
                 if (str[i]=='H') {
                    i++;
                     if (i == left) {
                            count=0;
                            break;
                           }
                     if (str[i]=='R') {
                       continue;
                      }
                     else if (str[i]=='G') {
                       continue;
                       }
                     else if (str[i]=='Y') {
                       continue;
                       }
                     else if (str[i]=='B') {
                       continue;
                       }
                     else if (str[i]=='M') {
                       continue;
                       }
                     else if (str[i]=='C') {
                       continue;
                       }
                     else if (str[i]=='W') {
                       continue;
                       }
                     else { i--; tp[0]=str[i]; tp[1]=0;
                            strcat(tempst,tp);
                            }
                   }
                 else if (str[i]=='L') {
                    i++;
                     if (i == left) {
                            count=0;
                            break;
                           }
                     if (str[i]=='R') {
                        continue;
                      }
                     else if (str[i]=='G') {
                        continue;
                       }
                     else if (str[i]=='Y') {
                        continue;
                       }
                     else if (str[i]=='B') {
                        continue;
                       }
                     else if (str[i]=='M') {
                        continue;
                       }
                     else if (str[i]=='C') {
                        continue;
                       }
                     else if (str[i]=='W') {
                        continue;
                       }
                     else { i--; tp[0]=str[i]; tp[1]=0;
                            strcat(tempst,tp);
                            }
                   }
                 else if (str[i]=='B') {
                    i++;
                     if (i == left) {
                            count=0;
                            break;
                           }
                     if (str[i]=='L') {
                        continue;
                      }
                     else { i--; tp[0]=str[i]; tp[1]=0;
                            strcat(tempst,tp);
                            }
                   }
                 else if (str[i]=='U') {
                    i++;
                     if (i == left) {
                            count=0;
                            break;
                           }
                     if (str[i]=='L') {
                        continue;
                      }
                     else { i--; tp[0]=str[i]; tp[1]=0;
                            strcat(tempst,tp);
                            }
                   }
                 else if (str[i]=='R') {
                    i++;
                     if (i == left) {
                            count=0;
                            break;
                           }
                     if (str[i]=='V') {
                        continue;
                      }
                     else { i--; tp[0]=str[i]; tp[1]=0;
                            strcat(tempst,tp);
                            }
                    }
                 else { tp[0]=str[i]; tp[1]=0;
                        strcat(tempst,tp);
                        }
                        continue;
                    }
                }
        tp[0]=str[i];
        tp[1]=0;
        strcat(tempst, tp);
        } 
count=0;
i=0;

return tempst;
}

void sysud(int ud)
{
char filename[FILE_NAME_LEN];
FILE *tfp;

if (ud)
 write_log(YESTIME,"Bot \"%s\" started with PID %u\n", BOT_NAME, (unsigned int)getpid());
else
 write_log(YESTIME,"Bot \"%s\" is exiting\n",BOT_NAME);

sprintf(filename,"%s.pid",thisprog);

if (ud) {
  if (!(tfp=fopen(filename,"w"))) return;
  fprintf(tfp,"%u",(unsigned int)getpid());
  fclose(tfp);
  }
else {
  remove(filename);
}

}

void handle_sig(int sig)
{

switch(sig) {
        case SIGTERM:
                write_log(YESTIME,"Caught SIGTERM! Exiting..\n");
		crash_n_burn();
		quit_robot();
        case SIGSEGV:
                write_log(YESTIME,"Caught SIGSEGV! Exiting..\n");
		crash_n_burn();
		quit_robot();
        case SIGILL:
                write_log(YESTIME,"Caught SIGILL! Exiting..\n");
		crash_n_burn();
		quit_robot();
        case SIGBUS:
                write_log(YESTIME,"Caught SIGBUS! Exiting..\n");
		crash_n_burn();
		quit_robot();
        case SIGPIPE:
                write_log(YESTIME,"Caught SIGPIPE! Continuing..\n");
  }
}


/* Write string and arguments to a specific logging facility */
void write_log(int wanttime, char *str, ...)
{
char z_mess[ARR_SIZE*2];
char logfile[FILE_NAME_LEN];
va_list args;
FILE *fp;

z_mess[0]=0;
strncpy(logfile,BOTLOG_FILE,FILE_NAME_LEN);

 if (!(fp=fopen(logfile,"a")))
   {
    sendchat(".sos I can't open my log file! Help!\n");
    quit_robot();
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

/*    strcpy(z_mess, strip_color(z_mess)); */
    fputs(z_mess,fp);
    fclose(fp);
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

/*** put string terminate char. at first char < 32 ***/
void terminate(char *str)
{
int u;
int bell = 7;
int tab  = 9;

for (u = 0; u<ARR_SIZE; ++u)
  {
   if ((*(str+u) == 13 &&       /* terminate line on first control char */
       *(str+u) != bell &&     /* except for bell                      */
       *(str+u) != tab) ||     /* and tab                              */
       *(str+u) > 126  )       /* special chars over 126               */
     {
	/* write_log(YESTIME,"terminating on \"%c\" %d\n",*(str+u),*(str+u)); */
      *(str+u)=0;
      u=ARR_SIZE;
     }
  }
}

