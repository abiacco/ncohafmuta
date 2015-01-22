/*----------------------------------------------------------------------*/
/* Now Come on Over Here And Fuck Me Up The Ass - Ncohafmuta V 1.4.x    */
/*----------------------------------------------------------------------*/
/*  This code is a collection of software that originally started       */
/*  as a system called:                                                 */
/*                       IFORMS V 1.0                                   */
/*            Interactive FORum Multiplexor Software - (C) Deep         */
/*                 Last update 25/9/94                                  */
/*                                                                      */
/* As a result of extensive changes, it can no longer be considered     */
/* the same code                                                        */
/*                     -Cygnus (Anthony J. Biacco - Ncohafmuta Ent.)    */
/*                                                                      */
/* Legal note:  This code may NOT be freely distributed.  Doing so may  */
/*              be in violation of the US Munitions laws which cover    */
/*              exportation of encoding technology.                     */
/*----------------------------------------------------------------------*/

 /* SPECIAL PINWHEELS USER EXCEPTIONS IN THE CODE */
 /* llo doesn't get blinking */
	
 /* Major things to do:
                intertalker connectivity
                investigate using a database
                investigate using threads or async IO for connections
                investigate creating an email gateway
                multiline mail, board writes
                add command initialization from a file
                create standards for diffent term type 
                allocate structure dynamically
                break code up into multiple C files (in progress)
                port code to Win95 and WinNT (in progress)
 */
 
/*--------------------------------------------------------------*/
/* NOTE: For AIX users the getrlimit is not supported by the OS */
/*                                                              */
/*   For sun users the cpp is not ansi standard so preprocessor */
/*   strings are not supported properly                         */
/*                                                              */
/*--------------------------------------------------------------*/

#if defined(HAVE_CONFIG_H)
#include "../hdrfiles/config.h"
#endif

#include "../hdrfiles/includes.h"

/*--------------------------------------------------------*/
/* Talker-related include files                           */
/*--------------------------------------------------------*/
#include "../hdrfiles/osdefs.h"
#include "../hdrfiles/authuser.h"
#define _DEFINING_TEXT
#include "../hdrfiles/text.h"
#include "../hdrfiles/constants.h"

/*--------------------------------------------------------*/
/* Initialize functions we need too			  */
/* Almost all, including all command functions		  */
/*--------------------------------------------------------*/
#include "../hdrfiles/protos.h"
#include "../hdrfiles/resolver_clipon.h"

extern int resolver_toclient_pipes[2];
extern int resolver_toserver_pipes[2];
extern int resolver_clipon_pid;
extern char *our_delimiters;

/** Variables declared somewhere else **/

extern int PORT;                  /* main login port for incoming   */

extern int listen_sock[4];        /* 32 bit listening sockets       */
extern fd_set readmask;           /* bitmap read set                */
extern fd_set writemask;          /* bitmap write set               */
extern int inter;                 /* inter talker connections       */
extern int cypher;                /* caller id port                 */

extern int last_user;
extern int restarting;

/* Big Letter array map */
extern int biglet[26][5][5];
extern char *syserror;

extern char area_nochange[MAX_AREAS]; 
extern char mess[ARR_SIZE+25];    /* functions use mess to send output   */ 
extern char t_mess[ARR_SIZE+25];  /* functions use t_mess as a buffer    */ 
extern char t_inpstr[ARR_SIZE];   /* functions use t_mess as a buffer    */ 
extern char datadir[255];	  /* config directory                    */
extern char thishost[101];	  /* FQDN were running on                */
extern char thisos[101];          /* operating system were running on    */
extern char thisprog[255];	  /* the binary the program is run as	 */
extern int noprompt;		  /* talker waiting for user input?      */
extern int signl;
extern int tells;		  /* tells in defined time period        */
extern int commands;		  /* total commands in defined time period  */
extern int says;		  /* says in defined time period         */
extern int says_running;
extern int tells_running;
extern int commands_running;
extern int shutd;		/* talker waiting for confirm on a shutdown? */
extern int delete_sent;
extern int sys_access;	/* is the system open for user connections?      */
extern int wiz_access;	/* is the system open for wizard connections?    */
extern int who_access;	/* is the system open for external who listings? */
extern int www_access;	/* is the system open for external web requests? */
extern int checked;	/* see if messages have been checked at midnight */
extern int debug;

extern struct command_struct sys[];          /* the user command structure */
extern struct botcommand_struct botsys[];    /* the bot command structure  */
extern char sh_conv[NUM_LINES][MAX_LINE_LEN+1];     /* store review shouts */
extern int sh_count;			      /* shout count in the buffer */
extern char mailgateway_ip[16];		     	   /* SMTP gateway address */
extern unsigned int mailgateway_port;		      /* SMTP gateway port */

/*** GLOBAL DECLARATIONS ***/

/* Definitions for authuser info from a remote identd */
struct timeval timeout={10, 0};
char l_mess[ARR_SIZE+25];  /* log functions use l_mess as buffer  */ 
char bt_conv[NUM_LINES][MAX_LINE_LEN+1]; /* stores lines of conversation in wiztell buffer */
char web_opts[11][64];	/* web configuration options from file    */
time_t start_time;	/* startup time                           */
int NUM_AREAS;		/* number of areas defined in config file */
int num_of_users=0;	/* total number of users online           */
int MESS_LIFE=0;	/* message lifetime in days               */
int atmos_on;		/* all room atmospherics on?              */
int syslog_on;		/* are we logging system stuff to a file  */
int allow_new;		/* can new users be created?              */
int dobackups;		/* are we archiving log files @ midnight? */
int average_tells;	/* average tells                          */
int down_time=0;	/* countdown to an auto shutdown or auto reboot */
int bt_count;		/* wiztell count in the buffer                  */
int treboot=0;		/* is an auto-reboot started?                   */

int autopromote = AUTO_PROMOTE; /* allowing users to promote themselves? */
int autonuke    = NUKE_NOSET;	/* nuking users on quit without a desc.? */
int autoexpire  = AUTO_EXPIRE;	/* expiring users at midnight on auto?   */
int bot         = -5;           /* this will hold the bots user number   */
int new_room;			/* room new users log into               */

/*** END OF GLOBAL DELCARATIONS ***/

/*** START OF FUNCTIONS ***/

/* list global revoke (mode 0) or grant (mode 1) permissions */
void listall_gravokes(int user, int mode)
{
int aa=0,ii=0,found2=0,t=0;
char name2[ARR_SIZE];
char small_buff2[64];
char filerid2[FILE_NAME_LEN];
struct dirent *dp2;
DIR  *dirp2;

if (mode==0)
write_str(user," Global revoke list");
else if (mode==1)
write_str(user," Global grant list");

write_str(user,"+-------------------------------------------------+");

  sprintf(t_mess,"%s",USERDIR);
        
  strncpy(filerid2,t_mess,FILE_NAME_LEN);
    
  dirp2=opendir((char *)filerid2);
       
 if (dirp2 == NULL)
   {write_str(user,"Directory information not found.");
    write_str(user,"+-------------------------------------------------+");
    write_log(ERRLOG,YESTIME,"Directory information not found for directory \"%s\" in listall_gravokes %s\n",
    filerid2,get_error());
    return;
   }

 while ((dp2 = readdir(dirp2)) != NULL)   
   {
    sprintf(small_buff2,"%s",dp2->d_name);
       if (small_buff2[0]=='.') continue;

	found2=0; aa=0; ii=0;
        read_user(small_buff2);

		/* first see if user has at least 1 to print their name */
                for (aa=0; aa < MAX_GRAVOKES; ++aa) {
		if (mode==0) {
                  if (!is_revoke(t_ustr.revokes[aa])) continue;
		  }
		else if (mode==1) {
                  if (!is_grant(t_ustr.revokes[aa])) continue;
		  }
                  found2=1; break;
                } /* end of for */
		if (!found2) continue;

sprintf(name2,"%-*s",NAME_LEN+count_color(t_ustr.say_name,0),t_ustr.say_name);
		/* print out all permissions */
                for (aa=0; aa < MAX_GRAVOKES; ++aa) {
		if (mode==0) {
                  if (!is_revoke(t_ustr.revokes[aa])) continue;
		  }
		else if (mode==1) {
                  if (!is_grant(t_ustr.revokes[aa])) continue;
		  }
		  if (ii>0) strcat(name2,",");
		  for (t=0;sys[t].jump_vector != -1;++t) {
			if (strip_com(t_ustr.revokes[aa]) == sys[t].jump_vector) {
				  strcat(name2,sys[t].command);
				break;
				}
		     }
			t=0;
		  ii=1;
                } /* end of for */
write_str(user,name2);
continue;
} /* end of while */
(void) closedir(dirp2);
write_str(user,"+-------------------------------------------------+");

}


/*------------------------------------------------------*/
/* write time system went up or down to syslog file     */
/*------------------------------------------------------*/
void sysud(int ud, int user)
{
char filename[FILE_NAME_LEN];
FILE *tfp;

/* if ud=1 system is coming up, else if 0, going down */

if (!syslog_on) return;

if (ud) puts("Logging startup...");

/* write to file */
if (ud)  {
  time(&start_time);
  write_log(BOOTLOG,NOTIME,"%s: BOOT on port %d using datadir: %s with pid %u\n",get_time(start_time,0),PORT,datadir,(unsigned int)getpid());
  }
else {
      if (user==-1) {
      if (treboot==1)
       write_log(BOOTLOG,YESTIME,"SOFT-REBOOT by the talker\n");
      else if (treboot==2)
       write_log(BOOTLOG,YESTIME,"HARD-REBOOT by the talker\n");
      else
       write_log(BOOTLOG,YESTIME,"SHUTDOWN by the talker complete\n");
      }
      else {
      if (treboot==1)
       write_log(BOOTLOG,YESTIME,"SOFT-REBOOT by %s\n",ustr[user].say_name);
      else if (treboot==2)
       write_log(BOOTLOG,YESTIME,"HARD-REBOOT by %s\n",ustr[user].say_name);
      else
       write_log(BOOTLOG,YESTIME,"SHUTDOWN by %s complete\n",ustr[user].say_name);
      }
     }

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


/*** Handle macros... if any ***/
/* returning 0 is an error, but the command will continue through the */
/* main loop, returning -1 is an error and the command is discarded   */
int check_macro(int user, char *inpstr)
{
int a=0;
int arg1=0;
int arg2=0;
int arg3=0;
int i=0;
int j=0;
int found=0;
int noarg=0;
char line[ARR_SIZE];
char outstr[ARR_SIZE];
char tp[3];
char argu[3][ARR_SIZE];
MacroPtr tmpMacros = ustr[user].Macros;

tp[0]=0;

if (!tmpMacros) return 0;

sscanf(inpstr,"%s ",line);
for (i=0;i<NUM_MACROS;++i) {
   if (!strcmp(tmpMacros->name[i],line)) {
     found=1;
     break;
    }
  }
if (!found) return 0;
found=0;

outstr[0]=0;
/* strcpy(outstr,line); */
remove_first(inpstr);
if (!strlen(inpstr)) noarg=1;
else {
 argu[0][0]=0;
 argu[1][0]=0;
 argu[2][0]=0;
 argu[3][0]=0;  /* last input */

 /* scan arguments into seperate arrays */
 while (strlen(inpstr)) {
   if (a<3) {
    sscanf(inpstr,"%s ",argu[a]);
    argu[a][80]=0;
    }
   else {
    strcpy(line,inpstr);
    break;
    }
   if (a<3) line[0]=0;
   remove_first(inpstr); 
   a++;
  } /* end of while */
 a=0;
} /* end of else */


/* go through the input string character by character */
for (j=0;j<strlen(tmpMacros->body[i]);++j) {
  if (tmpMacros->body[i][j]=='$') {
     j++;
     if (tmpMacros->body[i][j]=='1') {
        if (noarg) {
          write_str(user,"Macro was made to accept an argument. None was given.");
          return -1;
          }
        /* only set if coming from a lower argument substition */
        if (a<1 && strlen(argu[0])) a=1;
        arg1++;
         /* only can use same substitution 3 times in same macro */
        if (arg1<=3)
         strcat(outstr,argu[0]);
        else { }
        continue;
       }
     else if (tmpMacros->body[i][j]=='2') {
        if (noarg) {
          write_str(user,"Macro was made to accept an argument. None was given.");
          return -1;
          }
        /* only set if coming from a lower argument substition */
        if (a<2 && strlen(argu[1])) a=2;
        arg2++;
         /* only can use same substitution 3 times in same macro */
        if (arg2<=3)
         strcat(outstr,argu[1]);
        else { }
        continue;     
       }
     else if (tmpMacros->body[i][j]=='3') {
        if (noarg) {
          write_str(user,"Macro was made to accept an argument. None was given.");
          return -1;
          }
        /* only set if coming from a lower argument substition */
        if (a<3 && strlen(argu[2])) a=3;
        arg3++;
         /* only can use same substitution 3 times in same macro */
        if (arg3<=3)
         strcat(outstr,argu[2]);
        else { }
        continue;     
       }
     else {
        /* $ is not a substitution, just a $ */
        j--;
        tp[0]=tmpMacros->body[i][j];
        tp[1]=0;
        strcat(outstr,tp);
        continue;
       }
    } /* end of if $ */
  else {
   /* just a normal character */
   tp[0]=tmpMacros->body[i][j];
   tp[1]=0;
   strcat(outstr,tp);
   tp[0]=0;
   continue;
  } /* end of else */
 } /* end of for */
j=0;

/* copy extra arguments onto end depending  */
/* on how many substitions were given       */
if (!a && !noarg) {
  /* none given to match macro, copy whole inpstr to end */
  strcat(outstr," ");
  strcat(outstr,argu[0]);
  if (strlen(argu[1])) strcat(outstr," ");
  strcat(outstr,argu[1]);
  if (strlen(argu[2])) strcat(outstr," ");
  strcat(outstr,argu[2]);
  if (strlen(line)) strcat(outstr," ");
  j = ARR_SIZE - (strlen(outstr) + 1);
  line[j]=0;
  strcat(outstr,line);
  }
else if (a==1 && !noarg) {
  /* 1 given to match macro, copy last 3 to end */
  if (strlen(argu[1])) strcat(outstr," ");
  strcat(outstr,argu[1]);
  if (strlen(argu[2])) strcat(outstr," ");
  strcat(outstr,argu[2]);
  if (strlen(line)) strcat(outstr," ");
  j = ARR_SIZE - (strlen(outstr) + 1);
  line[j]=0;
  strcat(outstr,line);
  }
else if (a==2 && !noarg) {
  /* 2 given to match macro, copy last 2 to end */
  if (strlen(argu[2])) strcat(outstr," ");
  strcat(outstr,argu[2]);
  if (strlen(line)) strcat(outstr," ");
  j = ARR_SIZE - (strlen(outstr) + 1);
  line[j]=0;
  strcat(outstr,line);
  }
else if (a==3 && !noarg) {
  /* 3 given to match macro, copy last 1 to end */
  if (strlen(line)) strcat(outstr," ");
  j = ARR_SIZE - (strlen(outstr) + 1);
  line[j]=0;
  strcat(outstr,line);
  }

/* copy final processed string back to main input string */
strcpy(inpstr,outstr);

 argu[0][0]=0;
 argu[1][0]=0;
 argu[2][0]=0;
 argu[3][0]=0;  /* last input */
 line[0]=0; outstr[0]=0; tp[0]=0;
 arg1=0; arg2=0; arg3=0; a=0; i=0; j=0; found=0; noarg=0;

return 1;
}


/*---------------------------------------------------------*/
/* keep an audit trail of user logins and logoffs          */
/* in the system log file                                  */
/*---------------------------------------------------------*/
void syssign(int user, int onoff)
{
char stm[30];
char filename[FILE_NAME_LEN];
FILE *fp;

/* write to file */
write_log(LOGINLOG,YESTIME,"%s:%s:%s:%s:sck#%d:slt#%d\n",onoff == 1 ? "IN " : "OUT",ustr[user].name,ustr[user].site,ustr[user].net_name,ustr[user].sock,user);

 if (onoff==1) {

   /* the name does not get the color stripped out of it here */
   /* because this is the first time the bot will get the user name */
   /* to put in its list */
    sprintf(mess,"+++++ logon:%s %s %d", ustr[user].say_name,
		astr[ustr[user].area].name, ustr[user].vis);
    write_bot(mess);

  /* If user coming in in bot room, tell bot that user is here */
  if (ustr[user].area == ustr[bot].area) {
    sprintf(mess,"+++++ came in:%s", strip_color(ustr[user].say_name));
    write_bot(mess);
    }

  sprintf(filename,"%s/%s",LOGDIR,LASTLOGS);
  if (!(fp=fopen(filename,"a"))) {
       /* write_str(user,BAD_FILEIO); */
       /* writeall_str(mess, WIZ_ONLY, -1, 0, -1, BOLD, NONE, 0); */
       write_log(ERRLOG,YESTIME,"Couldn't open file(a) \"%s\" in syssign! %s\n",filename,get_error()); 
       return;
       }

strcpy(stm,get_time(0,0));
fprintf(fp,"%s  %s ",stm,strip_color(ustr[user].say_name));
fprintf(fp,"%s\n",strip_color(ustr[user].desc));
fclose(fp);

midcpy(stm,stm,11,12);
  if (!strcmp(stm,"00"))
   logstat[0].logins++;
  else if (!strcmp(stm,"01"))
   logstat[1].logins++;
  else if (!strcmp(stm,"02"))
   logstat[2].logins++;
  else if (!strcmp(stm,"03"))
   logstat[3].logins++;
  else if (!strcmp(stm,"04"))
   logstat[4].logins++;
  else if (!strcmp(stm,"05"))
   logstat[5].logins++;
  else if (!strcmp(stm,"06"))
   logstat[6].logins++;
  else if (!strcmp(stm,"07"))
   logstat[7].logins++;
  else if (!strcmp(stm,"08"))
   logstat[8].logins++;
  else if (!strcmp(stm,"09"))
   logstat[9].logins++;
  else if (!strcmp(stm,"10"))
   logstat[10].logins++;
  else if (!strcmp(stm,"11"))
   logstat[11].logins++;
  else if (!strcmp(stm,"12"))
   logstat[12].logins++;
  else if (!strcmp(stm,"13"))
   logstat[13].logins++;
  else if (!strcmp(stm,"14"))
   logstat[14].logins++;
  else if (!strcmp(stm,"15"))
   logstat[15].logins++;
  else if (!strcmp(stm,"16"))
   logstat[16].logins++;
  else if (!strcmp(stm,"17"))
   logstat[17].logins++;
  else if (!strcmp(stm,"18"))
   logstat[18].logins++;
  else if (!strcmp(stm,"19"))
   logstat[19].logins++;
  else if (!strcmp(stm,"20"))
   logstat[20].logins++;
  else if (!strcmp(stm,"21"))
   logstat[21].logins++;
  else if (!strcmp(stm,"22"))
   logstat[22].logins++;
  else if (!strcmp(stm,"23"))
   logstat[23].logins++;

 }
 else {
  /* If user leaving from bot room, tell bot that user left */
  if (ustr[user].area == ustr[bot].area) {
    sprintf(mess,"+++++ left:%s", strip_color(ustr[user].say_name));
    write_bot(mess);
    }
 }

}


/*---------------------------------------------------------*/
/* keep an audit trail of user logins to the who and www   */
/* ports in the system log file                            */
/*---------------------------------------------------------*/
int log_misc_connect(int user, unsigned long addr, int type)
{
char stm[30];
static char buf[256];
static char namebuf[256];
struct hostent *he;
time_t tm;

/* write to file */
time(&tm);
strcpy(stm,ctime(&tm));
stm[strlen(stm)-6]=0; /* get rid of nl at end */

/* Resolve sock address to hostname, if cant copy failed message */
 if (type != 3 && type != 4) {
 he = gethostbyaddr((char *)&addr, sizeof(addr), AF_INET);
 if (he && he->h_name)
    strcpy(namebuf, he->h_name);
 else
    strcpy(namebuf, SYS_LOOK_FAILED);
 }

/* Resolve to ip */
addr = ntohl(addr);
sprintf(buf,"%ld.%ld.%ld.%ld", (addr >> 24) & 0xff, (addr >> 16) & 0xff,
         (addr >> 8) & 0xff, addr & 0xff);

if (type==1) {
  write_log(LOGINLOG,YESTIME,"WHO: Connection %s:%s:sck#%d:slt#%d\n",buf,namebuf,whoport[user].sock,user);
  strcpy(whoport[user].site,buf);
  strcpy(whoport[user].net_name,namebuf);
  }
else if (type==2) {
  write_log(LOGINLOG,YESTIME,"WWW: Connection %s:%s:sck#%d:slt#%d\n",buf,namebuf,wwwport[user].sock,user);
  strcpy(wwwport[user].site,buf);
  strcpy(wwwport[user].net_name,namebuf);
  }
else if (type==3) {
  strcpy(namebuf,buf);
  write_log(LOGINLOG,YESTIME,"RWHO: Connection %s:%s:sck#%d:slt#%d:%s\n",buf,namebuf,miscconn[user].sock,user,ustr[miscconn[user].user].name);
  strcpy(miscconn[user].site,buf);
  }
else if (type==4) {
  strcpy(namebuf,buf);
  write_log(LOGINLOG,YESTIME,"SMTP: Connection %s:%s:sck#%d:slt#%d\n",buf,namebuf,miscconn[user].sock,user);
  strcpy(miscconn[user].site,buf);
  }

if (type==1) {
  if (check_misc_restrict(whoport[user].sock,buf,namebuf) == 1) {
   write_log(BANLOG,YESTIME,"WHO : Connection attempt, RESTRICTed site %s:%s:sck#%d:slt#%d\n",buf,namebuf,whoport[user].sock,user);
   return -1;
   }
  else
   return 0;
  }
else if (type==2) {
  if (check_misc_restrict(wwwport[user].sock,buf,namebuf) == 1) {
   write_log(BANLOG,YESTIME,"WWW : Connection attempt, RESTRICTed site %s:%s:sck#%d:slt#%d\n",buf,namebuf,wwwport[user].sock,user);
   return -1;
   }
  else
   return 0;
  }

return 1;
}


/*** closes socket & does relevant output to other users & files ***/
void user_hot_quit(int user)
{
time_t tm;

time(&tm);
strcpy(ustr[user].last_date,  ctime(&tm));
  ustr[user].last_date[24]=0;
strcpy(ustr[user].last_site, ustr[user].site);
strcpy(ustr[user].last_name, ustr[user].net_name);
ustr[user].rawtime = tm;

/* Fix a user that has quit or has been killed while stuck in editing */
if (ustr[user].pro_enter) {
  free(ustr[user].pro_start);
  ustr[user].pro_end='\0';
  strcpy(ustr[user].flags,ustr[user].mutter);
  }
else if (ustr[user].vote_enter) {
  free(ustr[user].vote_start);
  ustr[user].vote_end='\0';
  strcpy(ustr[user].flags,ustr[user].mutter);
  }
else if (ustr[user].roomd_enter) {
  free(ustr[user].roomd_start);
  ustr[user].roomd_end='\0';
  strcpy(ustr[user].flags,ustr[user].mutter);
  }

queue_flush(user);
if (ustr[user].logging_in) {
        /* Close socket and clear from readmask */
	SHUTDOWN(ustr[user].sock, 2);
        while (CLOSE(ustr[user].sock) == -1 && errno == EINTR)
                ; /* empty while */
}
else {
copy_from_user(user);
write_user(ustr[user].name);
write_rebootdb(user);
}

        FD_CLR(ustr[user].sock,&readmask);
        FD_CLR(ustr[user].sock,&writemask);
	reset_user_struct(user,1);
}

/*** closes socket & does relevant output to other users & files ***/
/*** If mode is 0 we run silent and dont output/flush anything   ***/
void user_quit(int user, int mode)
{
int area=ustr[user].area;
int min;
time_t tm;

/* see is user has quit before he logged in */
if (ustr[user].logging_in) {
       min=(int)((time(0) - ustr[user].last_input)/60);
       if (min >= LOGIN_TIMEOUT)
        sprintf(mess," [LOGIN TIMED OUT from %s (%s)]",ustr[user].net_name,ustr[user].site);
       else
        sprintf(mess," [LOGIN ABORTED from %s (%s)]",ustr[user].net_name,ustr[user].site);
        writeall_str(mess, -2, user, 0, user, BOLD, NONE, 0);
        mess[0]=0;
        min=0;
   if (!strcmp(ustr[user].desc,DEF_DESC) && (ustr[user].super==0)) {
	if (mode) {
         write_str(user,"");
         write_str(user,"Description not set. GONNA HAFTA NUKE YOU. Sorry.");
         write_str(user,"");
	}
        remove_exem_data(ustr[user].login_name);
        remove_user(ustr[user].login_name);
        write_log(SYSTEMLOG,YESTIME,"AUTO-NUKE of %s\n",ustr[user].login_name);
        }

	/* Flush data to socket */
	if (mode) queue_flush(user);

	/* Close socket and clear from readmask */
	SHUTDOWN(ustr[user].sock, 2);
        while (CLOSE(ustr[user].sock) == -1 && errno == EINTR)
		; /* empty while */
        FD_CLR(ustr[user].sock,&readmask);
        FD_CLR(ustr[user].sock,&writemask);

	/* reset user structure */
	reset_user_struct(user,1);
	return;
	}

if (mode) write_str(user,BYE_MESS); 

time(&tm);
strcpy(ustr[user].last_date,  ctime(&tm));
  ustr[user].last_date[24]=0;
strcpy(ustr[user].last_site, ustr[user].site);
strcpy(ustr[user].last_name, ustr[user].net_name);
ustr[user].rawtime = tm;

min=(tm-ustr[user].time)/60;
if (min<0) min=0;

if (mode) {
sprintf(mess,"%s %s",
             ustr[user].say_name,  ustr[user].desc);
write_str(user,mess);

sprintf(mess,"From site %s - %s",
             ustr[user].last_site, ustr[user].last_name);
write_str(user,mess);
                                      
sprintf(mess,USE_MESS,ustr[user].last_date,converttime((long)min));
write_str(user,mess);
} /* end of mode */

/* kind of a fudge factor always have at least one */
if (ustr[user].aver==0) ustr[user].aver = 1;  

ustr[user].times_on++;

ustr[user].totl += (long)min;

ustr[user].aver = (int)(ustr[user].totl / (long)ustr[user].times_on);

/* Fix a user that has quit or has been killed while stuck in editing */
if (ustr[user].pro_enter) {
  free(ustr[user].pro_start);
  ustr[user].pro_end='\0';
  strcpy(ustr[user].flags,ustr[user].mutter);
  }
else if (ustr[user].vote_enter) {
  free(ustr[user].vote_start);
  ustr[user].vote_end='\0';
  strcpy(ustr[user].flags,ustr[user].mutter);
  }
else if (ustr[user].roomd_enter) {
  free(ustr[user].roomd_start);
  ustr[user].roomd_end='\0';
  strcpy(ustr[user].flags,ustr[user].mutter);
  }

copy_from_user(user);
write_user(ustr[user].name);

/* send message to other users & conv file */
if (ustr[user].super >= WIZ_LEVEL) 
  {
   sprintf(mess,SYS_DEPART_HI,ustr[user].say_name,ustr[user].desc);
   writeall_str(mess, 0, user, 0, user, NORM, LOGIO, 0);
  }
else 
  {
   sprintf(mess,SYS_DEPART_LO,ustr[user].say_name,ustr[user].desc);
   writeall_str(mess, 0, user, 0, user, NORM, LOGIO, 0);
  }

if (astr[area].private && (find_num_in_area(area) <= PRINUM) && (mode!=2))
   {
    strcpy(mess,NOW_PUBLIC);
    writeall_str(mess, 1, user, 0, user, NORM, NONE, 0);
    astr[area].private=0;
    cbuff(user);
   }


/* store signoff in log file & set some vars. */
num_of_users--;
syssign(user,0);

sprintf(mess,"+++++ logoff:%s",strip_color(ustr[user].say_name));
write_bot(mess);

if (ustr[user].ttt_opponent != -3)
        ttt_abort(user); 

if (user == fight.first_user || user == fight.second_user) reset_chal(0,"");

 if (autonuke) {
   if (!strcmp(ustr[user].desc,DEF_DESC) && (ustr[user].super==0)) {
	if (mode) {
        write_str(user,"");
        write_str(user,"Description not set. GONNA HAFTA NUKE YOU. Sorry.");
        write_str(user,"");
	}
        remove_exem_data(ustr[user].login_name);
        remove_user(ustr[user].login_name);
	write_log(SYSTEMLOG,YESTIME,"AUTO-NUKE of %s\n",ustr[user].say_name);

        sprintf(mess,"%s %s has been AUTO-NUKED",STAFF_PREFIX,strip_color(ustr[user].say_name));
        writeall_str(mess, WIZ_ONLY, user, 0, user, BOLD, WIZT, 0);
      }
   }

/* If bot logging off, clear bot id */
if (user==bot) {
  write_bot("+++++ QUIT");
  bot=-5;
  }

if (ustr[user].rwho > 1) {
/*
        write_str(user,"");
        write_str(user,"You're in the middle of a remote who. Your connection will not close until the task is done. Please wait.");
*/
for (min=0;min<MAX_MISC_CONNECTS;++min) {
        if (miscconn[min].user==user) {
        free_sock(min,'5');
	break;
	} /* if */
} /* for */
	write_log(SYSTEMLOG,YESTIME,"RWHO: Cancelled remote who for %s\n",strip_color(ustr[user].say_name));
	ustr[user].rwho=1;
}

 /* Flush data to socket */
if (mode) queue_flush(user);

 /* Close user's connection socket */
  SHUTDOWN(ustr[user].sock, 2);
  while (CLOSE(ustr[user].sock) == -1 && errno == EINTR)
	; /* empty while */
  FD_CLR(ustr[user].sock,&readmask);
  FD_CLR(ustr[user].sock,&writemask);

 /* reset user structure */
  reset_user_struct(user,1);

}


/*** prints who is on the system to requesting user ***/
void t_who(int user, char *inpstr, int mode)
{
FILE   *fp=NULL;
int    s,u,v,min,idl,invis=0,count=0,ranklen=0;
int    z=0;
int    vi=' ';
int    with,num=0;
char   ud[100],un[SAYNAME_LEN+5],an[ROOM_LEN+5],und[SAYNAME_LEN*2],rank[50];
char   temp[256];
char   check[NAME_LEN+1];
char   i_buff[5];
char   filename[FILE_NAME_LEN];
time_t tm;

/*------------------------------------------------------------*/
/* Check if inpstr length is greater than the max name length */
/*------------------------------------------------------------*/
if (strlen(inpstr) > NAME_LEN) {
   write_str(user,"Name too long.");
   return;
   }

if (check_rwho(user,inpstr) == 1) return;

if (mode==3) {
  if (ustr[user].friend_num==0) {
     write_str(user,"You have no friends!");
     return;
     }
  }  

/*-------------------------------------------------------*/
/* process the with command                              */
/*-------------------------------------------------------*/
with = user;
if (mode == 2)
  {
    if(strlen(inpstr) == 0) 
      {
       with = ustr[user].area;
      }
     else
      {
       sscanf(inpstr,"%s ",temp);
       strtolower(temp);

       if ((u=get_user_num(temp,user))== -1) 
         {
           not_signed_on(user,temp);  
           return;
         }
       with = ustr[u].area;
      }
  }

if (ustr[user].pbreak) {
   strcpy(filename,get_temp_file());
   if (!(fp=fopen(filename,"w"))) {
     write_str(user,BAD_FILEIO);
     return;
     }
   }

time(&tm);
/* display current time */
if (mode==1)
 sprintf(mess,WWHO_COLOR,get_time(0,1));
else if (mode==3)
 sprintf(mess,FWHO_COLOR,get_time(0,1));
else
 sprintf(mess,WHO_COLOR,get_time(0,1));

if (ustr[user].pbreak) {
   strcat(mess,"\n\r\n\r");
   fputs(mess,fp);
   }
else {
write_str(user,mess);
write_str(user,"");
}

/* Give Display format */
if (ustr[user].who==0) { /* OURS */
/* sprintf(mess,"^LCRoom^             "); */
sprintf(mess,"%-*s",ROOM_LEN+6,"^LCRoom^");
strcat(mess,"^HGTime^ "); 
strcat(mess,"^HRStat^ "); 
strcat(mess,"^Idl^   ");
strcat(mess,"^HMName^/^HBDescription^");
if (ustr[user].pbreak) {
   strcat(mess,"\n");
   fputs(mess,fp);
   }
else write_str(user,mess);
} /* end of if */
else if (ustr[user].who==2) { /* IFORMS */
/* sprintf(mess,"^LCRoom^              "); */
sprintf(mess,"%-*s",ROOM_LEN+7,"^LCRoom^");
strcat(mess,"^HMName^/^HBDescription^                           ");
strcat(mess,"^HGTime^-"); 
strcat(mess,"^HRStat^-"); 
strcat(mess,"^Idle^");
if (ustr[user].pbreak) {
   strcat(mess,"\n");
   fputs(mess,fp);
   }
else write_str(user,mess);
} /* end of if */
else if (ustr[user].who==3) { /* NEW */
sprintf(mess,"%-*s %-*s ",NAME_LEN,"Name",ROOM_LEN,"Room");
strcat(mess,"    On For Idle");
if (ustr[user].pbreak) {
   strcat(mess,"\n");
   fputs(mess,fp);
   }
else write_str(user,mess);
} /* end of if */


if (mode!=3) {
for (v=0;v<NUM_AREAS;++v) {
  for (u=0;u<MAX_USERS;++u) {
	if ((ustr[u].area!= -1) && (ustr[u].area == v) && (!ustr[u].logging_in))
	  {
		if (!ustr[u].vis && ustr[user].tempsuper < MIN_HIDE_LEVEL)
	          {
	            invis++;
	            continue; 
	          }
	   
       	        min=(int)((time(0)-ustr[u].time)/60);
		idl=(int)((time(0)-ustr[u].last_input)/60);
		if (min<0) min=0;
		if (idl<0) idl=0;

		if (ustr[user].who==1)
		sprintf(un," %s",ustr[u].say_name);
		else
		strcpy(un,ustr[u].say_name);

                if (ustr[u].afk==0)
		 strcpy(ud,ustr[u].desc);
		else if ((strlen(ustr[u].afkmsg) > 1) && (ustr[u].afk>=1))
		 strcpy(ud,ustr[u].afkmsg);
                else if (!strlen(ustr[u].afkmsg) && (ustr[u].afk>=1))
                 strcpy(ud,ustr[u].desc);

		strcpy(und,un);
		strcat(und," ");
		strcat(und,ud);
		
		if (!astr[ustr[u].area].hidden)
		  {
		    strcpy(an,astr[ustr[u].area].name);
		  }
		 else
		  { 
		    if ((ustr[user].tempsuper >= ROOMVIS_LEVEL) && SHOW_HIDDEN)
		      {
		        strcpy(an, "<");
		        strcat(an, astr[ustr[u].area].name);
		        strcat(an, ">");
		      }
		     else
		      {
		       strcpy(an, "        ");
		      }
		  }
		  
		
		if (ustr[user].tempsuper >= WHO_LEVEL) 
		   { 
		    s=ranks[ustr[u].super].abbrev;
		   }
		  else
		   s=' ';
		   
		if (ustr[u].afk == 1) {
		   if (ustr[u].lockafk)
		    strcpy(i_buff,"^HYLAFK^");
		   else
		    strcpy(i_buff,"^HYAFK^ ");
                  }
		 else if (ustr[u].afk == 2) {
		      strcpy(i_buff,"^HRBAFK^");
                     }
                 else if (ustr[u].pro_enter) {
		      strcpy(i_buff,"^HGPROF^");
                     }
                 else if (ustr[u].vote_enter) {
		      strcpy(i_buff,"^HGVOTE^");
                     }
                 else if (ustr[u].roomd_enter) {
		      strcpy(i_buff,"^HGDESC^");
                     }
		 else {
		if ((ustr[user].who==1) && (ustr[user].tempsuper >= WHO_LEVEL))
		 strcpy(i_buff,"");
		else {
                  if (idl < 3)
                    strcpy(i_buff,"Actv");
                  else if (idl >= 3 && idl < 60)
                    strcpy(i_buff,"Awke");
                  else if (idl >= 60 && idl < 180)
		    strcpy(i_buff,"Idle"); 
                  else if (idl >= 180)
                    strcpy(i_buff,"Coma");
		  }
                    } 

	if (ustr[user].who==1) {
	if (ustr[user].tempsuper >= WHO_LEVEL) {
	strcpy(rank,ranks[ustr[u].super].sname);
	ranklen=RANK_LEN;
	}
	else {
	strcpy(rank,"  ");
	strcat(rank,i_buff);
	strcat(rank,"  ");
	strcpy(i_buff,"");
	ranklen=strlen(rank);
	}
	}

	if (ustr[user].who==1) {
		if (!ustr[u].vis)
		 und[0]='*';
	}
	else {
                if (!ustr[u].vis) vi='_';
                else vi=' ';
	}

count=DESC_LEN+count_color(und,0);
/*
sprintf(mess,"len without mod        : %d\nlen of colors          : %d\nlen of replacing colors: %d\ncount                  : %d\n",strlen(und),count_color(und,0),count_color(und,1),count);
write_str(user,mess);
*/
if (ustr[user].who==1)
        sprintf(mess,"%-*s :%-*s:%-*s:%-3.3d m %s",count,und,ranklen,rank,ROOM_LEN,an,min,i_buff);
else if (ustr[user].who==2)
        sprintf(mess,"%-*s %c%c%-*s %-5.5d %s %3.3d",ROOM_LEN,an,s,vi,count,und,min,i_buff,idl);
else if (ustr[user].who==3) {
        sprintf(mess,"%-*s %-*s %s %s",NAME_LEN+count_color(ustr[u].say_name,0),
ustr[u].say_name,ROOM_LEN,an,time_format_1((time_t)time(0)-ustr[u].time),
time_format_2((time_t)time(0)-ustr[u].last_input));
}
else
        sprintf(mess,"%-*s %-5.5d %s %3.3d %c%c%s",ROOM_LEN,an,min,i_buff,idl,s,vi,und);

count=0;
		
		strncpy(temp,strip_color(mess),256);
		strtolower(temp);
		
		if (!strlen(inpstr) ||
		     instr2(0,mess,inpstr,0)!= -1 ||
		     instr2(0,temp,inpstr,0)!= -1 ||
		     mode > 0) 
		 {
		   if ((mode == 1 && ustr[u].tempsuper >= WIZ_LEVEL) || 
		        mode == 0 ||
		       (mode == 2 && ustr[u].area == with))
		     {
		      mess[0]=toupper((int)mess[0]);
			if (ustr[user].pbreak) {
			   strcat(mess,"\n");
  			   fputs(mess,fp);
  			 }
		      else write_str(user,mess);
                      num++;
		     }  /* inpstr check */
                 }  /* mode check */
	   }  /* if user in area check */
	}  /* user for */
 }  /* area for */
}  /* end of mode if */

/* Friends check */
if (mode==3) {
for (z=0; z<MAX_ALERT; ++z) {
  strcpy(check,ustr[user].friends[z]);
  strtolower(check);
for (v=0;v<NUM_AREAS;++v) {
  for (u=0;u<MAX_USERS;++u) {
	if ((ustr[u].area!= -1) && (ustr[u].area == v) && (!ustr[u].logging_in))
	  {
            if (!strcmp(ustr[u].name,check)) {
		if (!ustr[u].vis && ustr[user].tempsuper < MIN_HIDE_LEVEL)
	          {
	            invis++;
	            continue; 
	          }
	   
       	        min=(int)((time(0)-ustr[u].time)/60);
		idl=(int)((time(0)-ustr[u].last_input)/60);
		if (min<0) min=0;
		if (idl<0) idl=0;

		if (ustr[user].who==1)
		sprintf(un," %s",ustr[u].say_name);
		else
		strcpy(un,ustr[u].say_name);

                if (ustr[u].afk==0)
		 strcpy(ud,ustr[u].desc);
		else if ((strlen(ustr[u].afkmsg) > 1) && (ustr[u].afk>=1))
		 strcpy(ud,ustr[u].afkmsg);
                else if (!strlen(ustr[u].afkmsg) && (ustr[u].afk>=1))
                 strcpy(ud,ustr[u].desc);

		strcpy(und,un);
		strcat(und," ");
		strcat(und,ud);
		
		if (!astr[ustr[u].area].hidden)
		  {
		    strcpy(an,astr[ustr[u].area].name);
		  }
		 else
		  { 
		    if ((ustr[user].tempsuper >= ROOMVIS_LEVEL) && SHOW_HIDDEN)
		      {
		        strcpy(an, "<");
		        strcat(an, astr[ustr[u].area].name);
		        strcat(an, ">");
		      }
		     else
		      {
		       strcpy(an, "        ");
		      }
		  }
		  
		
		if (ustr[user].tempsuper >= WHO_LEVEL) 
		   { 
		    s=ranks[ustr[u].super].abbrev;
		   }
		  else
		   s=' ';
		   
		if (ustr[u].afk == 1) {
		   if (ustr[u].lockafk)
		    strcpy(i_buff,"^HYLAFK^");
		   else
		    strcpy(i_buff,"^HYAFK^ ");
                  }
		 else if (ustr[u].afk == 2) {
		      strcpy(i_buff,"^HRBAFK^");
                     }
                 else if (ustr[u].pro_enter) {
		      strcpy(i_buff,"^HGPROF^");
                     }
                 else if (ustr[u].vote_enter) {
		      strcpy(i_buff,"^HGVOTE^");
                     }
                 else if (ustr[u].roomd_enter) {
		      strcpy(i_buff,"^HGDESC^");
                     }
		 else {
		if ((ustr[user].who==1) && (ustr[user].tempsuper >= WHO_LEVEL))
		  strcpy(i_buff,"");
		else {
                  if (idl < 3)
                    strcpy(i_buff,"Actv");
                  else if (idl >= 3 && idl < 60)
                    strcpy(i_buff,"Awke");
                  else if (idl >= 60 && idl < 180)
		    strcpy(i_buff,"Idle"); 
                  else if (idl >= 180)
                    strcpy(i_buff,"Coma");
		  }
                    } 

	if (ustr[user].who==1) {
	if (ustr[user].tempsuper >= WHO_LEVEL) {
	strcpy(rank,ranks[ustr[u].super].sname);
	ranklen=RANK_LEN;
	}
	else {
	strcpy(rank,"  ");
	strcat(rank,i_buff);
	strcat(rank,"  ");
	strcpy(i_buff,"");
	ranklen=strlen(rank);
	}
	}

	if (ustr[user].who==1) {
		if (!ustr[u].vis)
		 und[0]='*';
	}
	else {
                if (!ustr[u].vis) vi='_';
                else vi=' ';
	}

count=DESC_LEN+count_color(und,0);
if (ustr[user].who==1)
        sprintf(mess,"%-*s :%-*s:%-*s:%-3.3d min %s",count,und,ranklen,rank,ROOM_LEN,an,min,i_buff);
else if (ustr[user].who==2)
        sprintf(mess,"%-*s %c%c%-*s %-5.5d %s %3.3d",ROOM_LEN,an,s,vi,count,und,min,i_buff,idl);
else if (ustr[user].who==3) {
        sprintf(mess,"%-*s %-*s %s %s",NAME_LEN+count_color(ustr[u].say_name,0),
ustr[u].say_name,ROOM_LEN,an,time_format_1((time_t)time(0)-ustr[u].time),
time_format_2((time_t)time(0)-ustr[u].last_input));
}
else
        sprintf(mess,"%-*s %-5.5d %s %3.3d %c%c%s",ROOM_LEN,an,min,i_buff,idl,s,vi,und);

count=0;
                 
		      mess[0]=toupper((int)mess[0]);
			if (ustr[user].pbreak) {
			   strcat(mess,"\n");
  			   fputs(mess,fp);
  			 }
		      else write_str(user,mess);
                      num++;
              }  /* end of if found */
	   }  /* if user in area check */
	}  /* user for */
  }  /* area for */
 }  /* end of friends list for */
}  /* end of mode if */


if (invis) {
sprintf(mess,SHADOW_COLOR,invis == 1 ? "is" : "are",invis,invis == 1 ? " " : "s");

	if (ustr[user].pbreak) {
	   strcat(mess,"\n");
	   fputs(mess,fp);
  	 }
	else write_str(user,mess);

	}

if (ustr[user].pbreak) {
   fputs("\n",fp);
   }	
else write_str(user," ");

 if (mode==1)
  sprintf(mess,WUSERCNT_COLOR,num,num == 1 ? "" : "s");
 else if (mode==3)
  sprintf(mess,FUSERCNT_COLOR,num,num == 1 ? "" : "s");
 else
  sprintf(mess,USERCNT_COLOR,num_of_users,num_of_users == 1 ? "" : "s");

	if (ustr[user].pbreak) {
	   strcat(mess,"\n");
	   fputs(mess,fp);
           fputs("\n",fp);
  	 }
  else {
  write_str(user,mess);
  write_str(user," ");
  }

	if (ustr[user].pbreak) {
	   fclose(fp);
           cat(filename,user,0);
  	 }

}


/*** prints who is on the system to requesting user ***/
void swho(int user, char *inpstr)
{
int u;
int found = 0;

/* Give Display format */
if ( !strlen(inpstr) )
  {
   write_str(user,"----------------------------------------------------------------");
   write_str(user,"User          IP address    User#:Sock  Hostname");
   write_str(user,"----------------------------------------------------------------");
  }
else strtolower(inpstr);

/* display who port connections */
for (u=0; u<MAX_WHO_CONNECTS; ++u) 
  {
   if (strlen(whoport[u].site) > 1) {
	   sprintf(mess, SYS_SITE_LINE, " [who port] ", 
	                                whoport[u].site, 
	                                u, whoport[u].sock,
		                        whoport[u].net_name );

	   if (!strlen(inpstr) || instr2(0,mess,inpstr,0) != -1) 
	     {
	       mess[0]=toupper((int)mess[0]);
	       write_str(user,mess);
               found=1;
             }
     } /* end of if */
  } /* end of for */
u=0;

/* display www port connections */
for (u=0; u<MAX_WWW_CONNECTS; ++u) 
  {
   if (strlen(wwwport[u].site) > 1) {
	   sprintf(mess, SYS_SITE_LINE, " [www port] ", 
	                                wwwport[u].site, 
	                                u, wwwport[u].sock,
		                        wwwport[u].net_name );

	   if (!strlen(inpstr) || instr2(0,mess,inpstr,0) != -1) 
	     {
	       mess[0]=toupper((int)mess[0]);
	       write_str(user,mess);
               found=1;
             }
     } /* end of if */
  } /* end of for */
u=0;
/* display misc connections */
for (u=0; u<MAX_MISC_CONNECTS; ++u) 
  {
   if (miscconn[u].sock != -1) {
	if (miscconn[u].type==1) {
	   sprintf(mess, SYS_SITE_LINE, " [ rwho ] ", 
	                                miscconn[u].site, 
	                                u, miscconn[u].sock,
		                        itoa(miscconn[u].port) );
	}
	else if (miscconn[u].type==2) {
	   sprintf(t_mess, "Stage: %d Queue: %s",miscconn[u].stage,miscconn[u].queuename);
	   sprintf(mess, SYS_SITE_LINE, " [ smtp ] ", 
	                                miscconn[u].site, 
	                                u, miscconn[u].sock,
		                        t_mess );
	}

	   if (!strlen(inpstr) || instr2(0,mess,inpstr,0) != -1) 
	     {
	       mess[0]=toupper((int)mess[0]);
	       write_str(user,mess);
               found=1;
             }
     } /* end of if */
  } /* end of for */
u=0;
/* display user list */
for (u=0; u<MAX_USERS; ++u) 
  {
    if (!strcmp(ustr[u].name,inpstr)) found = 2;

    if (ustr[u].area != -1)  
      {
	sprintf(mess, SYS_SITE_LINE, ustr[u].name,
	                             ustr[u].site,
	                             u, ustr[u].sock,
		                     ustr[u].net_name );

        if (strlen(inpstr) && (found==2)) {
	   mess[0]=toupper((int)mess[0]);
	   write_str(user,mess);
           write_str(user," ");
           return;
           }
	else if (!strlen(inpstr) || instr2(0,mess,inpstr,0) != -1) 
	  {
	   mess[0]=toupper((int)mess[0]);
	   write_str(user,mess);
           found=1;
          }
       } /* end of area if */
      else if (ustr[u].logging_in) 
         {
	   sprintf(mess, SYS_SITE_LINE, " [login] ", 
	                                ustr[u].site, 
	                                u, ustr[u].sock,
		                        ustr[u].net_name );

	   if (!strlen(inpstr) || instr2(0,mess,inpstr,0) != -1) 
	     {
	       mess[0]=toupper((int)mess[0]);
	       write_str(user,mess);
               found=1;
             }
	  } /* end of else if */
  } /* end of for */
/*
for (u=0;u<MAX_CONNLIST_ENTRIES;++u) {
	sprintf(mess,"%14s %2d %ld\n",connlist[u].site,connlist[u].connections,
	connlist[u].starttime);
	write_str(user,mess);
}
*/

if (!found) {
/* plug security hole */
if (check_fname(inpstr,user))
  {                                     
   write_str(user,"Illegal name.");
   return;
  }

   if (!check_for_user(inpstr)) {
     write_str(user,NO_USER_STR);
     return;
     }
   read_user(inpstr);
   sprintf(mess,"%s, in from %s %s",t_ustr.say_name,t_ustr.last_site,t_ustr.last_name);
   write_str(user,mess);
   }

write_str(user," ");
}


/*** not signed on - subsid func ***/
void not_signed_on(int user, char *name)
{
sprintf(mess,"%s is not signed on",name);
mess[0]=toupper((int)mess[0]);
write_str(user,mess);
return;
}


/*** go moves user into different room ***/
void go(int user, char *inpstr, int user_knock)
{
int f;
int new_area=0;
int teleport=0;
int beep=0;
int area=ustr[user].area;
int found = 0;
char room_char;
char room_name[ARR_SIZE];
char entmess[80];
char exitmess[80];

if (ustr[user].anchor) {
  write_str(user,ANCHORED_DOWN);
  return;
  }

if (!strlen(inpstr)) 
  {
   if (user_knock==1) 
     {
      write_str(user,"Knock where?");
      return;
     }
    else if (!user_knock)
     {
      if ((ustr[user].tempsuper==0) && (!strcmp(ustr[user].desc,DEF_DESC))
          && (area==new_room)) {
          write_str(user,"You can't leave this room until you set a description with .desc");
          return;
         }
      write_str(user,"*** warp to main room ***");
      new_area = INIT_ROOM;
      teleport = 1;
     }
  }
 else
  {
   sscanf(inpstr,"%s ",room_name);

      if ((ustr[user].tempsuper==0) && (!strcmp(ustr[user].desc,DEF_DESC))
          && (area==new_room)) {
          write_str(user,"You can't leave this room until you set a description with .desc");
          return;
         }

   /*--------------------*/
   /* see if area exists */
   /*--------------------*/

   found = FALSE;
   for (new_area=0; new_area < NUM_AREAS; ++new_area)
    { 
     if (! instr2(0, astr[new_area].name, room_name, 0) )
       { 
         found = TRUE;
         break;
       }
    }
 
   if (!found)
     {
      write_str(user, NO_ROOM);
      return;
     }
  }
  
/*----------------------------------------------*/
/* check to see if the user is in that room     */
/*----------------------------------------------*/

if (ustr[user].area == new_area) 
  {
    write_str(user,"You are in that room now!");  
    return;
  }

/*----------------------------------------------*/
/* check for secure room                        */
/*----------------------------------------------*/

if (ustr[user].security[new_area] == 'X' && user_knock!=1) 
  {
   /* don't let in unless invited */
   if (ustr[user].invite != new_area ) {
   write_str(user,"Your security clearance does not let you enter there");
   return;
   }
  }
  
/*-----------------------------------------------*/
/* see if user can get to area from current area */
/*-----------------------------------------------*/

room_char = new_area + 'A';                /* get char. repr. of room to move to */

  strcpy(entmess,ustr[user].entermsg);
  strcpy(exitmess,ustr[user].exitmsg);

/*------------------------------------------*/
/* see if new room is joined to current one */
/*------------------------------------------*/

found = FALSE;
for (f=0; f<strlen(astr[area].move); ++f) 
 {
  if ( astr[area].move[f] == room_char )  
    {
     found = TRUE;
     break;
    }
 }

/*--------------------------------------------------------------*/
/* anyone equal to the TELEP_LEVEL or higher can teleport to    */
/* non-connected rooms                                          */
/*--------------------------------------------------------------*/

if (!found)
  {
    if ((ustr[user].tempsuper >= TELEP_LEVEL) || (ustr[user].security[new_area] == 'Y')) 
      {
        strcpy(entmess,COME_TELEMESS);  
        beep=1;
        teleport=1;  
        found = TRUE;
      }
  }
  
if (!strlen(inpstr) && strcmp(astr[ustr[user].area].name,ARREST_ROOM)) {
     found = TRUE;
     }

if ((user_knock==2)
    && strcmp(astr[ustr[user].area].name,ARREST_ROOM)
    && FOLLOWIS_JOIN) {
    found = TRUE;
    }

if (!found)
  { 
   write_str(user,"That room is not adjoined to here");
   return;
  }

/*-----------------------------------------------------------*/
/* check for a user knock                                    */
/*-----------------------------------------------------------*/
if (user_knock==1) 
  {
   knock(user,new_area);  
   return;
  }
  
/*-----------------------------------------------------------*/
/* if the room is private abort move...inform user           */
/*-----------------------------------------------------------*/

if (astr[new_area].private && ustr[user].invite != new_area ) 
  {
   write_str(user,"Sorry - that room is currently private");
   return;
  }

/* record movement */
if (teleport || astr[new_area].hidden)
  sprintf(mess,GO_TELEMESS,ustr[user].say_name);
 else {
  if (!strcmp(exitmess,DEF_EXIT))
   sprintf(mess,"%s %s %s",ustr[user].say_name,exitmess,astr[new_area].name);
  else  
   sprintf(mess,"%s %s",ustr[user].say_name,exitmess);
  }

/* send output to old room & to conv file */
if (!ustr[user].vis) {
   if (area == ustr[bot].area) {
    sprintf(mess,"+++++ left:%s", strip_color(ustr[user].say_name));
    write_bot(mess);
    }
   strcpy(mess,INVIS_MOVES);
   }
	
writeall_str(mess, 1, user, 0, user, NORM, NONE, 0);

if (ustr[user].vis) {
   sprintf(mess,"%s has left the room.",ustr[user].say_name);
   writeall_str(mess, 1, user, 0, user, NORM, NONE, 0);
   if (area == ustr[bot].area) {
    sprintf(mess,"+++++ left:%s", strip_color(ustr[user].say_name));
    write_bot(mess);
    }
   }

/*-----------------------------------------------------------*/
/* return room to public     (if needed)                     */
/*-----------------------------------------------------------*/

if (astr[area].private && (find_num_in_area(area) <= PRINUM))
  {
   strcpy(mess,NOW_PUBLIC);
   writeall_str(mess, 1, user, 0, user, NORM, NONE, 0);
   cbuff(user); 
   astr[area].private=0;
  }

/* record movement to new room */ 
sprintf(mess,"%s %s",ustr[user].say_name,entmess);

/* send output to new room */
if (!ustr[user].vis) 
	strcpy(mess,INVIS_MOVES);

if (!strcmp(astr[new_area].name,HEAD_ROOM)) {
   write_str(user,"A portal appears from nowhere as you step in it.");
   write_str_nr(user,"Identify for retina scan.");
	telnet_write_eor(user);
      if (!strcmp(ustr[user].name,ROOT_ID) || !strcmp(ustr[user].name,"matrix")
          || !strcmp(ustr[user].name,"sirkrake") || !strcmp(ustr[user].name,"pixie")
          || !strcmp(ustr[user].name,"shadowlord") || !strcmp(ustr[user].name,"sauron") 
          || !strcmp(ustr[user].name,"shadowscragger") || !strcmp(ustr[user].name,"stauf")
          || !strcmp(ustr[user].name,"wil") || !strcmp(ustr[user].name,"ladybug")
          || !strcmp(ustr[user].name,"damia") || !strcmp(ustr[user].name,"lazarus")
          || !strcmp(ustr[user].name,"krystal") || !strcmp(ustr[user].name,"dagny")
          || !strcmp(ustr[user].name,"weasal") || !strcmp(ustr[user].name,"scupper")
          || !strcmp(ustr[user].name,"necros") || !strcmp(ustr[user].name,"hecubus")
          || !strcmp(ustr[user].name,"cygnus"))
        {
         if (user_wants_message(user,BEEPS)) {
          write_str_nr(user,".\07");
	telnet_write_eor(user);
          write_str_nr(user,".\07");
	telnet_write_eor(user);
          write_str_nr(user,".\07\07");
	telnet_write_eor(user);
          }
         else {
          write_str_nr(user,".");
	telnet_write_eor(user);
          write_str_nr(user,".");
	telnet_write_eor(user);
          write_str_nr(user,".");
	telnet_write_eor(user);
          }
         write_str(user,"Access granted!");
         write_str(user,"\nThe portal closes behind you...\n");
        }
      else { 
       if (user_wants_message(user,BEEPS)) {
        write_str_nr(user,".\07");
	telnet_write_eor(user);
        write_str_nr(user,".\07");
	telnet_write_eor(user);
        }
       else {
        write_str_nr(user,".");
	telnet_write_eor(user);
        write_str_nr(user,".");
	telnet_write_eor(user);
        }
       write_str(user,"Identification not recognized..sorry.");
       return;
       }
  }
    
ustr[user].area = new_area;

if (beep==1)
 writeall_str(mess, 1, user, 0, user, BEEPS, NONE, 0);
else
 writeall_str(mess, 1, user, 0, user, NORM, NONE, 0);
beep=0;

   if (new_area == ustr[bot].area) {
    sprintf(mess,"+++++ came in:%s", strip_color(ustr[user].say_name));
    write_bot(mess);
    }

/* deal with user */
if (ustr[user].invite == new_area)  
  ustr[user].invite= -1;
  
look(user,"");
}



/*** knock - subsid func of go ***/
void knock(int user, int new_area)
{
int temp;

if (!astr[new_area].private) 
  {
   write_str(user,"That room is public anyway");
   return;
  }
  
write_str(user,"You knock on the door");
sprintf(mess,"%s knocks on the door",ustr[user].say_name);
if (!ustr[user].vis)
 sprintf(mess,"%s knocks on the door",INVIS_ACTION_LABEL);

/* swap user area 'cos of way output func. works */
temp=ustr[user].area;
ustr[user].area=new_area;
writeall_str(mess, 1, user, 0, user, NORM, KNOCK, 0);
ustr[user].area=temp;

/* send message to users in current room */
sprintf(mess,"%s knocks on the %s door",ustr[user].say_name,astr[new_area].name);
writeall_str(mess, 1, user, 0, user, NORM, KNOCK, 0);
}


/* Edit a user profile from the command line */
/* op_mode = 0       Straight editing of a line               */
/* op_mode = 1       Clearing of a line                       */
/* op_mode = 2       Blanking of a line                       */
/* op_mode = 3       Inserting a line at end of file          */
/* op_mode = 4       Inserting a blank line at end of file    */
/* op_mode = 5       Inserting a line in middle of file       */
/* op_mode = 6       Inserting a blank line in middle of file */

void inedit_file(int user, char *inpstr, int line_num, int mode)
{
int a=0;
int lines=0;
int diff=0;
char buffer[ARR_SIZE];
char filename[FILE_NAME_LEN];
char filename2[FILE_NAME_LEN];
FILE *fp;
FILE *fp2;

sprintf(filename,"%s/%s",PRO_DIR,ustr[user].name);
lines = file_count_lines(filename);
fp=fopen(filename,"r");

/* Put these mode checks in first so we know if we can insert anymore */
/* lines in the users profile.                                        */
if ((mode==3) || (mode==4) || (mode==5) || (mode==6)) {
   if (lines==PRO_LINES) {
     write_str(user,"You cant insert anymore lines to an already full profile.");
     FCLOSE(fp);
     return;
    }
  }

/* Put these modes first since we know they contain an input string */
/* and only need to append to the end of the original file          */
if ((mode==3) || (mode==4)) {
FCLOSE(fp);
if (!(fp=fopen(filename,"a"))) {
        write_str(user,BAD_FILEIO);
        write_log(ERRLOG,YESTIME,"Couldn't open file(a) \"%s\" in inedit_file! %s\n",filename,get_error());
	return;
        }
   if (mode==3) { fputs(inpstr,fp); }
   else if (mode==4) { }

   fputs("\n",fp);
   if (mode==3)
    write_str(user,"Inserted text to end.");
   else if (mode==4)
    write_str(user,"Inserted blank line to end.");
   FCLOSE(fp);
   if (mode==3) {
        if (autopromote == 1)
         check_promote(user,9);
     }
   return;
  }

if (!strlen(inpstr)) {
  if (line_num > lines) {
    write_str(user,"You dont have anything in that line.");
    FCLOSE(fp);
    return;
    }
   for (a=1;a<PRO_LINES+1;++a) {
      fgets(buffer,1000,fp);
      buffer[strlen(buffer)-1]=0;
      if (a==line_num) {
         if (strlen(buffer) > 1)
          sprintf(mess,"Line ^%d^ is: %s",line_num,buffer);
         else
          sprintf(mess,"Line ^%d^ is blank.",line_num);
         write_str(user,mess);
         FCLOSE(fp);
         return;
        }
      strcpy(buffer,"");
     } /* end of for */
  } /* end of no input string */
else {
strcpy(filename2,get_temp_file());
if (!(fp2=fopen(filename2,"a"))) {
        write_str(user,BAD_FILEIO);
        write_log(ERRLOG,YESTIME,"Couldn't open tempfile(a) for %s to append to in inedit_file()! %s\n",ustr[user].name,get_error());
        FCLOSE(fp); return;
        }
 if (mode==1) {
   if (line_num > lines) {
     write_str(user,"That line doesnt exist to clear.");
     FCLOSE(fp);
     FCLOSE(fp2);
     return;
    }
   for (a=1;a<lines+1;++a) {
      fgets(buffer,1000,fp);
      if (a==line_num) { }
      else fputs(buffer,fp2);
      strcpy(buffer,"");
     } /* end of for */
   sprintf(mess,"Line ^%d^ cleared!",line_num);
   write_str(user,mess);
   FCLOSE(fp);
   FCLOSE(fp2);
   remove(filename);
   rename(filename2,filename);
   return;
  } /* end of clear line mode */
 else if (mode==2) {
   if (line_num > lines) {
     write_str(user,"That line doesnt exist to blank.");
     FCLOSE(fp);
     FCLOSE(fp2);
     return;
    }
   for (a=1;a<lines+1;++a) {
      fgets(buffer,1000,fp);
      if (a==line_num) fputs("\n",fp2);
      else fputs(buffer,fp2);
      strcpy(buffer,"");
     } /* end of for */
   sprintf(mess,"Line ^%d^ blanked!",line_num);
   write_str(user,mess);
   FCLOSE(fp);
   FCLOSE(fp2);
   remove(filename);
   rename(filename2,filename);
   return;
  } /* end of blank line mode */

 else if (mode==5 || mode==6) {
   if (line_num > lines) {
     write_str(user,"That line doesnt exist to insert after.");
     FCLOSE(fp);
     FCLOSE(fp2);
     return;
    }
   for (a=1;a<lines+1;++a) {
      fgets(buffer,1000,fp);
      if (a==line_num) {
        fputs(buffer,fp2);
        if (mode==5) {
         fputs(inpstr,fp2);
         fputs("\n",fp2);
         }
        else if (mode==6) fputs("\n",fp2);
        }
      else fputs(buffer,fp2);
      strcpy(buffer,"");
     } /* end of for */
   if (mode==5)
    sprintf(mess,"Inserted your text after line ^%d^",line_num);
   else if (mode==6)
    sprintf(mess,"Inserted a blank line after line ^%d^",line_num);
   write_str(user,mess);
   FCLOSE(fp);
   FCLOSE(fp2);
   remove(filename);
   rename(filename2,filename);
   if (mode==5) {
        if (autopromote == 1)
         check_promote(user,9);
     }
   return;
  } /* end of insert line in middle mode */

 if (line_num > lines) {
   diff = line_num - lines;

   for (a=1;a<lines+1;++a) {
      fgets(buffer,1000,fp);
      fputs(buffer,fp2);
      strcpy(buffer,"");
     } /* end of for */
   a=0;
   if (diff!=1) {
     for (a=0;a<diff-1;++a) {
      fputs("\n",fp2);
       }
     } /* end of if diff */
   fputs(inpstr,fp2);
   fputs("\n",fp2);
   sprintf(mess,"Added line ^%d^",line_num);
   write_str(user,mess);
   FCLOSE(fp);
   FCLOSE(fp2);
   remove(filename);
   rename(filename2,filename);

        if (autopromote == 1)
         check_promote(user,9);

   return;
   } /* end of greater than if */
 else {
   for (a=1;a<lines+1;++a) {
      fgets(buffer,1000,fp);
      if (a==line_num) {
        fputs(inpstr,fp2);
        fputs("\n",fp2);
        }
      else fputs(buffer,fp2);
      strcpy(buffer,"");
     } /* end of for */
   sprintf(mess,"Modified line ^%d^",line_num);
   write_str(user,mess);
   FCLOSE(fp);
   FCLOSE(fp2);
   remove(filename);
   rename(filename2,filename);

        if (autopromote == 1)
         check_promote(user,9);

   return;
  } /* end of other line edit else */
 } /* end of main else */

}

/** Write profile in buffer to file **/
int write_pro(int user)
{
char *c,filename[FILE_NAME_LEN];
FILE *fp;

sprintf(filename,"%s/%s",PRO_DIR,ustr[user].name);
if (!(fp=fopen(filename,"w"))) {
        write_str(user,BAD_FILEIO);
        write_log(ERRLOG,YESTIME,"Couldn't open file(w) \"%s\" in write_pro! %s\n",filename,get_error());
	return 0;
        }
for (c=ustr[user].pro_start;c<ustr[user].pro_end;++c) putc(*c,fp);
fclose(fp);
return 1;
}


/** Enter room description ***/
void descroom(int user, char *inpstr)
{
char *c;
int ret_val;
int redo=0;
int i=0; /*******/
int op_mode=0; /******/
char option[ARR_SIZE]; /******/
char filename[FILE_NAME_LEN];
char filename2[FILE_NAME_LEN];
FILE *fp;

if ((!strcmp(astr[ustr[user].area].name,HEAD_ROOM)) ||
    (!strcmp(astr[ustr[user].area].name,"sky_palace"))) {
    write_str(user,"Can't make a description for this room, sorry.");
    return;
    }

/* get memory */
STARTROOMD:

if (!ustr[user].roomd_enter) {
       option[0]=0;
       sscanf(inpstr,"%s ",option);
       if (!strcmp(option,"-c") || !strcmp(option,"clear")) {
       sprintf(filename,"%s/%s",datadir,astr[ustr[user].area].name);
       remove(filename);
       write_str(user,"Room description deleted.");
       return;
       }
        if (!(ustr[user].roomd_start=(char *)malloc(82*ROOM_DESC_LINES))) {
        write_str(user,BAD_MALLOC);
	write_log(ERRLOG,YESTIME,"MALLOC: Can't malloc memory in descroom! %s\n",get_error());
        return;
        }

       if (strlen(inpstr) && (!redo)) {
         if (!strncmp(option,"-i",2)) {
          if (strlen(option)==2) op_mode=3;
          else {
           for (i=2;i<strlen(option);++i) {
              if (!isdigit((int)option[i])) {
                write_str(user,"Line number given was not a number!");
                return;
                }
             } /* end of for */
           i=0;
           midcpy(option,option,2,7);
           i=atoi(option);
           if (i == 0) {
             write_str(user,"Description lines start at 1. Not 0. Try again.");
             return;
             }
           op_mode=5;
          } /* end of else */

          remove_first(inpstr);
          if (op_mode==3) {
            if (!strlen(inpstr)) {
              write_str(user,"You must have text or a -b after this option");
              return;
              }
            if (!strcmp(inpstr,"-b")) op_mode=4;
            inedit_file2(user,inpstr,1,op_mode);
            return;
           }
          else if (op_mode==5) {
            if (!strlen(inpstr)) {
              write_str(user,"You must have text or a -b after this option");
              return;
              }
            if (!strcmp(inpstr,"-b")) op_mode=6;
            inedit_file2(user,inpstr,i,op_mode);
            return;
           }
         } /* end of IF INSERT OPTION */

       for (i=0;i<strlen(option);++i) {
          if (!isdigit((int)option[i])) {
            write_str(user,"Line number given was not a number!");
            return;
            }
         }
       i=0;
       i=atoi(option);
       if (i == 0) {
         write_str(user,"Description lines start at 1. Not 0. Try again.");
         return;
         }
       if (i > PRO_LINES) {
         sprintf(mess,"The line number can't be higher than the max desc. lines allowed, which is currently %d",PRO_LINES);
         write_str(user,mess);
         return;
         }
       remove_first(inpstr);
       if (!strcmp(inpstr,"-c")) op_mode=1;
       else if (!strcmp(inpstr,"-b")) op_mode=2;
       else op_mode=0;
       inedit_file2(user,inpstr,i,op_mode);
       return;
      } /* end of inedit */         
    ustr[user].roomd_enter=1;
    ustr[user].roomd_end=ustr[user].roomd_start;
    if (!redo) {
    sprintf(mess,"%s is writing a room description..",ustr[user].say_name);
    writeall_str(mess, 1, user, 0, user, NORM, NONE, 0);
    }
    if (!redo) strcpy(ustr[user].mutter,ustr[user].flags);
    user_ignore(user,"all");
    write_str(user,"");
    write_str(user,"** Decorating the room, finish with a '.' on a line by itself **");
    sprintf(mess,"** Max lines you can write is %d",ROOM_DESC_LINES);
    write_str(user,mess);
    write_str(user,"");
    write_str_nr(user,"1: ");
	 telnet_write_eor(user);
    noprompt=1;
    return;
    }
inpstr[80]=0;  c=inpstr;

/* check for dot terminator */
ret_val=0;

if (ustr[user].roomd_enter > ROOM_DESC_LINES) {
   if (*c=='s' && *(c+1)==0) {
     ret_val=write_room(user);
        if (ret_val) {
	write_str(user,"");
	write_str(user,"Room description stored");
	}
        else {
	write_str(user,"");
	write_str(user,"Room description not stored");
	}
        free(ustr[user].roomd_start);  ustr[user].roomd_enter=0;
        ustr[user].roomd_end='\0';
        noprompt=0;
        sprintf(mess,"%s finishes the room description.",ustr[user].say_name);
        writeall_str(mess, 1, user, 0, user, NORM, NONE, 0);
        strcpy(ustr[user].flags,ustr[user].mutter);
        ustr[user].mutter[0]=0;
        return;
     }
   else if (*c=='v' && *(c+1)==0) {
write_str(user,"+-----------------------------------------------------------------------------+");
c='\0';
strcpy(filename2,get_temp_file());
fp=fopen(filename2,"w");
for (c=ustr[user].roomd_start;c<ustr[user].roomd_end;++c) { 
    putc(*c,fp);
    }
    fclose(fp);
    cat(filename2,user,0);
    remove(filename2);
c='\0';
write_str(user,"+-----------------------------------------------------------------------------+");
            write_str_nr(user,PROFILE_PROMPT);
		 telnet_write_eor(user);
            noprompt=1;  return;
        }
   else if (*c=='r' && *(c+1)==0) {
        free(ustr[user].roomd_start); ustr[user].roomd_enter=0;
        ustr[user].roomd_end='\0';
        redo=1;
        goto STARTROOMD;
        }             
   else if (*c=='a' && *(c+1)==0) {
        free(ustr[user].roomd_start); ustr[user].roomd_enter=0;
        ustr[user].roomd_end='\0';
	write_str(user,"");
        write_str(user,"Room description not stored");
        noprompt=0;
        sprintf(mess,"%s finishes the room description.",ustr[user].say_name);
        writeall_str(mess, 1, user, 0, user, NORM, NONE, 0);
        strcpy(ustr[user].flags,ustr[user].mutter);
        ustr[user].mutter[0]=0;
        return;
        }             
   else {
    write_str_nr(user,PROFILE_PROMPT);
	 telnet_write_eor(user);
    return;
   } 
  }

if (*c=='.' && *(c+1)==0) {
        if (ustr[user].roomd_enter!=1)   {
            ustr[user].roomd_enter= ROOM_DESC_LINES + 1;
            write_str_nr(user,PROFILE_PROMPT);
		 telnet_write_eor(user);
            noprompt=1;  return;
            }
        else {
	write_str(user,"");
	write_str(user,"Room description not stored");
	}
        free(ustr[user].roomd_start);  ustr[user].roomd_enter=0;
        noprompt=0;
        sprintf(mess,"%s finishes the room description.",ustr[user].say_name);
        writeall_str(mess, 1, user, 0, user, NORM, NONE, 0);
        strcpy(ustr[user].flags,ustr[user].mutter);
        ustr[user].mutter[0]=0;
        return;
        }

/* write string to memory */
while(*c) *ustr[user].roomd_end++=*c++;
*ustr[user].roomd_end++='\n';

/* end of lines */
if (ustr[user].roomd_enter==ROOM_DESC_LINES) {
            ustr[user].roomd_enter= ROOM_DESC_LINES + 1;
            write_str_nr(user,PROFILE_PROMPT);
		 telnet_write_eor(user);
            noprompt=1;  return;
        }
sprintf(mess,"%d: ",++ustr[user].roomd_enter);
write_str_nr(user,mess);
telnet_write_eor(user);
}


/* Edit a room description from the command line */
/* op_mode = 0       Straight editing of a line               */
/* op_mode = 1       Clearing of a line                       */
/* op_mode = 2       Blanking of a line                       */
/* op_mode = 3       Inserting a line at end of file          */
/* op_mode = 4       Inserting a blank line at end of file    */
/* op_mode = 5       Inserting a line in middle of file       */
/* op_mode = 6       Inserting a blank line in middle of file */

void inedit_file2(int user, char *inpstr, int line_num, int mode)
{
int a=0;
int lines=0;
int diff=0;
char buffer[ARR_SIZE];
char filename[FILE_NAME_LEN];
char filename2[FILE_NAME_LEN];
FILE *fp;
FILE *fp2;

sprintf(filename,"%s/%s",datadir,astr[ustr[user].area].name);
lines = file_count_lines(filename);
fp=fopen(filename,"r");

/* Put these mode checks in first so we know if we can insert anymore */
/* lines in the users profile.                                        */
if ((mode==3) || (mode==4) || (mode==5) || (mode==6)) {
   if (lines==PRO_LINES) {
     write_str(user,"You cant insert anymore lines to an already full room description.");
     FCLOSE(fp);
     return;
    }
  }

/* Put these modes first since we know they contain an input string */
/* and only need to append to the end of the original file          */
if ((mode==3) || (mode==4)) {
FCLOSE(fp);
if (!(fp=fopen(filename,"a"))) {
        write_str(user,BAD_FILEIO);
        write_log(ERRLOG,YESTIME,"Couldn't open file(a) \"%s\" in inedit_file2! %s\n",filename,get_error());
	return;
        }
   if (mode==3) { fputs(inpstr,fp); }
   else if (mode==4) { }

   fputs("\n",fp);
   if (mode==3)
    write_str(user,"Inserted text to end.");
   else if (mode==4)
    write_str(user,"Inserted blank line to end.");
   FCLOSE(fp);
   return;
  }

if (!strlen(inpstr)) {
  if (line_num > lines) {
    write_str(user,"You dont have anything in that line.");
    FCLOSE(fp);
    return;
    }
   for (a=1;a<PRO_LINES+1;++a) {
      fgets(buffer,1000,fp);
      buffer[strlen(buffer)-1]=0;
      if (a==line_num) {
         if (strlen(buffer) > 1)
          sprintf(mess,"Line ^%d^ is: %s",line_num,buffer);
         else
          sprintf(mess,"Line ^%d^ is blank.",line_num);
         write_str(user,mess);
         FCLOSE(fp);
         return;
        }
      strcpy(buffer,"");
     } /* end of for */
  } /* end of no input string */
else {
strcpy(filename2,get_temp_file());
if (!(fp2=fopen(filename2,"a"))) {
        write_str(user,BAD_FILEIO);
        write_log(ERRLOG,YESTIME,"Couldn't open tempfile(a) for %s to append to in inedit_file2! %s\n",ustr[user].name,get_error());
        FCLOSE(fp); return;
        }
 if (mode==1) {
   if (line_num > lines) {
     write_str(user,"That line doesnt exist to clear.");
     FCLOSE(fp);
     FCLOSE(fp2);
     return;
    }
   for (a=1;a<lines+1;++a) {
      fgets(buffer,1000,fp);
      if (a==line_num) { }
      else fputs(buffer,fp2);
      strcpy(buffer,"");
     } /* end of for */
   sprintf(mess,"Line ^%d^ cleared!",line_num);
   write_str(user,mess);
   FCLOSE(fp);
   FCLOSE(fp2);
   remove(filename);
   rename(filename2,filename);
   return;
  } /* end of clear line mode */
 else if (mode==2) {
   if (line_num > lines) {
     write_str(user,"That line doesnt exist to blank.");
     FCLOSE(fp);
     FCLOSE(fp2);
     return;
    }
   for (a=1;a<lines+1;++a) {
      fgets(buffer,1000,fp);
      if (a==line_num) fputs("\n",fp2);
      else fputs(buffer,fp2);
      strcpy(buffer,"");
     } /* end of for */
   sprintf(mess,"Line ^%d^ blanked!",line_num);
   write_str(user,mess);
   FCLOSE(fp);
   FCLOSE(fp2);
   remove(filename);
   rename(filename2,filename);
   return;
  } /* end of blank line mode */

 else if (mode==5 || mode==6) {
   if (line_num > lines) {
     write_str(user,"That line doesnt exist to insert after.");
     FCLOSE(fp);
     FCLOSE(fp2);
     return;
    }
   for (a=1;a<lines+1;++a) {
      fgets(buffer,1000,fp);
      if (a==line_num) {
        fputs(buffer,fp2);
        if (mode==5) {
         fputs(inpstr,fp2);
         fputs("\n",fp2);
         }
        else if (mode==6) fputs("\n",fp2);
        }
      else fputs(buffer,fp2);
      strcpy(buffer,"");
     } /* end of for */
   if (mode==5)
    sprintf(mess,"Inserted your text after line ^%d^",line_num);
   else if (mode==6)
    sprintf(mess,"Inserted a blank line after line ^%d^",line_num);
   write_str(user,mess);
   FCLOSE(fp);
   FCLOSE(fp2);
   remove(filename);
   rename(filename2,filename);
   return;
  } /* end of insert line in middle mode */

 if (line_num > lines) {
   diff = line_num - lines;

   for (a=1;a<lines+1;++a) {
      fgets(buffer,1000,fp);
      fputs(buffer,fp2);
      strcpy(buffer,"");
     } /* end of for */
   a=0;
   if (diff!=1) {
     for (a=0;a<diff-1;++a) {
      fputs("\n",fp2);
       }
     } /* end of if diff */
   fputs(inpstr,fp2);
   fputs("\n",fp2);
   sprintf(mess,"Added line ^%d^",line_num);
   write_str(user,mess);
   FCLOSE(fp);
   FCLOSE(fp2);
   remove(filename);
   rename(filename2,filename);
   return;
   } /* end of greater than if */
 else {
   for (a=1;a<lines+1;++a) {
      fgets(buffer,1000,fp);
      if (a==line_num) {
        fputs(inpstr,fp2);
        fputs("\n",fp2);
        }
      else fputs(buffer,fp2);
      strcpy(buffer,"");
     } /* end of for */
   sprintf(mess,"Modified line ^%d^",line_num);
   write_str(user,mess);
   FCLOSE(fp);
   FCLOSE(fp2);
   remove(filename);
   rename(filename2,filename);
   return;
  } /* end of other line edit else */
 } /* end of main else */

}


/** Write room description in buffer to file **/
int write_room(int user)
{
char *c,filename[FILE_NAME_LEN];
FILE *fp;

sprintf(filename,"%s/%s",datadir,astr[ustr[user].area].name);
if (!(fp=fopen(filename,"w"))) {
        write_str(user,BAD_FILEIO);
        write_log(ERRLOG,YESTIME,"Couldn't open file(w) \"%s\" in write_room! %s\n",filename,get_error());
	return 0;
        }
for (c=ustr[user].roomd_start;c<ustr[user].roomd_end;++c) putc(*c,fp);
fclose(fp);
return 1;
}

/** Write vote topic in buffer to file **/
int write_vote(int user)
{
char *c,filename[FILE_NAME_LEN];
FILE *fp;

sprintf(filename,"%s/%s",LIBDIR,"votefile");
if (!(fp=fopen(filename,"w"))) {
        write_str(user,BAD_FILEIO);
        write_log(ERRLOG,YESTIME,"Couldn't open file(w) \"%s\" in write_vote! %s\n",filename,get_error());
	return 0;
        }
for (c=ustr[user].vote_start;c<ustr[user].vote_end;++c) putc(*c,fp);
fclose(fp);
return 1;
}

/** show a newbie how to type a command **/
void show(int user, char *inpstr)
{
  if (!strlen(inpstr)) {
    write_str(user,"Show what?");  return;
    }

if (ustr[user].frog) strcpy(inpstr,FROG_TALK);

sprintf(mess,"Type ---> ^%s^\n",inpstr);
write_str(user,mess);
writeall_str(mess, 1, user, 0, user, NORM, ECHOM, 0);
}

/** clears a socket for use **/
void cline(int user, char *inpstr)
{
int u=0,uu,found=0;

if (!strlen(inpstr)) {
        write_str(user,"Clear which line?\n");
        return;
        }

/* plug security hole */
if (check_fname(inpstr,user)) 
  {
   write_str(user,"Illegal name.");
   return;
  }

u=atoi(inpstr);

for (uu=0;uu<MAX_WWW_CONNECTS;++uu) {
        if (wwwport[uu].sock==u) {
          free_sock(uu,'4');
          sprintf(mess,"Web line %d cleared\n",uu);
          write_str(user,mess);
          found=1;
          break;
          }
   }
   
uu=0;
if (found) return;

for (uu=0;uu<MAX_WHO_CONNECTS;++uu) {
        if (whoport[uu].sock==u) {
          free_sock(uu,'3');
          sprintf(mess,"Who line %d cleared\n",uu);
          write_str(user,mess);
          found=1;
          break;
          }
   }
           
uu=0;
if (found) return;

for (uu=0;uu<MAX_MISC_CONNECTS;++uu) {
        if (miscconn[uu].sock==u) {
          free_sock(uu,'5');
          sprintf(mess,"Rwho line %d cleared\n",uu);
          write_str(user,mess);
          found=1;
          break;
          }
   }
   
uu=0;
if (found) return;

for (uu=0;uu<MAX_USERS;++uu) {
	if (ustr[uu].sock==u && ustr[uu].logging_in) {
        user_quit(uu,1);
        sprintf(mess,"Line %d cleared\n",uu);
        write_str(user,mess);
	found=1; break;
        }
	else if (ustr[uu].sock==u && !ustr[uu].logging_in) {
        sprintf(mess,"Line %d is in use\n",uu);
        write_str(user,mess);
	found=1; break;
	}
   } /* end of for */
if (!found) {
	write_str(user,"Line not found!");
  }

}

/** blinking broadcast **/
void bbcast(int user, char *inpstr)
{
if (!strlen(inpstr)) {
  write_str(user,"Broadcast what?");
  return;
  }
sprintf(mess,"^BL *** [ %s ] ***^",inpstr);
writeall_str(mess, 0, user, 1, user, BEEPS, BCAST, 0);
write_log(SYSTEMLOG,YESTIME,"BBCAST: by %s\n",ustr[user].say_name);
}

/** version command **/
void version(int user)
{
int i=0;
struct stat fileinfo2;
time_t lastmod;

write_str(user,VERSION);

/* See when the program was last updated */
stat(thisprog, &fileinfo2);
lastmod = fileinfo2.st_ctime;
sprintf(mess,"Updated: %s",ctime(&lastmod));
mess[strlen(mess)-1]=0;
write_str(user,mess);

sprintf(mess,"Hostname & OS: %s (%s)",thishost,thisos);
write_str(user,mess);

if ((TELEP_LEVEL==0) && (FOLLOWIS_JOIN==1))
 write_str(user,"Teleportation is NOT restricted.");
else if ((TELEP_LEVEL==0) && (FOLLOWIS_JOIN==0))
 write_str(user,"Teleportation is loosely restricted.");
else if ((TELEP_LEVEL > 0) && (FOLLOWIS_JOIN==0)) {
 sprintf(mess,"Teleportation is reserved for ranks >= %d",TELEP_LEVEL);
 write_str(user,mess);
 }
else if ((TELEP_LEVEL > 0) && (FOLLOWIS_JOIN==1)) {
 sprintf(mess,"Teleportation is reserved for ranks >= %d (except .follow)",TELEP_LEVEL);
 write_str(user,mess);
 }

if (allow_new > 1)
 write_str(user,"Registration is NOT needed for new players.");
else if (allow_new==1)
 write_str(user,"Email verification IS required for new players.");
else
 write_str(user,"New players are not allowed at this time.");

i=DAYS_OFF;
if ((i < 1) || (i > 1))
 sprintf(mess,"Expiration grace period for users is %d days.",i);
else
 sprintf(mess,"Expiration grace period for users is %d day.",i);
write_str(user,mess);

if ((autoexpire==1) || (autoexpire==2)) {
i=TIME_TO_GO;
if (i > 1)
 sprintf(mess,"Expiration warning period for users is %d days.",i);
else if (i==1)
 sprintf(mess,"Expiration warning period for users is %d day.",i);
else
 strcpy(mess,"Expiration warning period for users is DISABLED");
write_str(user,mess);
}
else {
 write_str(user,"Expiration warning period for users is DISABLED");
}

write_str(user,"An administrative hierarchy exists.");
if (resolve_names==1)
 write_str(user,"Resolver is                 configured for cache and gethostbyaddr");
else if (resolve_names==2)
 write_str(user,"Resolver is                 configured for cache and resolver clip-on");
else if (resolve_names==3)
 write_str(user,"Resolver is                 configured for just resolver clip-on");
else if (resolve_names==4)
 write_str(user,"Resolver is                 configured for just gethostbyaddr");
else
 write_str(user,"Resolver is                 disabled.");
write_str(user,"The robot interface is      enabled.");
if (down_time == 1)
 sprintf(mess,"Auto-Shutdown/Reboot is     enabled.  (%d min. left)",down_time);
else if (down_time > 0)
 sprintf(mess,"Auto-Shutdown/Reboot is     enabled.  (%d mins. left)",down_time);
else
 sprintf(mess,"Auto-Shutdown/Reboot is     disabled.");
write_str(user,mess);
sprintf(mess,"Max users                   : %3d       Max atmospheres p/room : %-3d",MAX_USERS,MAX_ATMOS);
write_str(user,mess);
sprintf(mess,"Max areas                   : %3d       Max length of atmoses  : %-3d",MAX_AREAS,ATMOS_LEN);
write_str(user,mess);
sprintf(mess,"Max name length for players : %3d       Max mail forwards a day: %-3d",NAME_LEN,MAX_AUTOFORS);
write_str(user,mess);
sprintf(mess,"Max description length      : %3d       Max mailfile size      : %-6d",DESC_LEN,MAX_MAILSIZE);
write_str(user,mess);
sprintf(mess,"Max macro length            : %3d       Max sent-mailfile size : %-6d",MACRO_LEN,MAX_SMAILSIZE);
write_str(user,mess);
sprintf(mess,"Max profile lines           : %3d",PRO_LINES);
write_str(user,mess);
sprintf(mess,"Max room description lines  : %3d",ROOM_DESC_LINES);
write_str(user,mess);
sprintf(mess,"Max entermessage length     : %3d",MAX_ENTERM-2);
write_str(user,mess);
sprintf(mess,"Max exitmessage length      : %3d",MAX_EXITM-2);
write_str(user,mess);
sprintf(mess,"Max success and fail lengths: %3d",MAX_ENTERM-2);
write_str(user,mess);
sprintf(mess,"Max topic length            : %3d",TOPIC_LEN);
write_str(user,mess);
}


/** superusers changing user_name **/
void suname(int user, char *inpstr)
{
int f,u;
char other_user[ARR_SIZE],newname[ARR_SIZE],lowername[NAME_LEN+1];
char filename[80];
char filename2[80];


if (!strlen(inpstr)) {
   write_str(user,"Whose name do you want to change?");
   return;
   }

sscanf(inpstr,"%s ",other_user);

/* plug security hole */
if (check_fname(other_user,user)) 
  {
   write_str(user,"Illegal name.");
   return;
  }

strtolower(other_user);
CHECK_NAME(other_user);

if (!read_user(other_user))
  {
   write_str(user,NO_USER_STR);
   return;
  }

 if ((t_ustr.super >= ustr[user].tempsuper) && strcmp(ustr[user].name,ROOT_ID))
  {
    strcpy(t_mess,"You cannot rename a user of same or higher rank.");
    write_str(user,t_mess);
    return;
  }
  
remove_first(inpstr);  
   
sscanf(inpstr,"%s",newname);

 if (newname[0]<32 || strlen(newname)< 3) 
    {
     write_str(user,"Invalid name given [must be at least 3 letters].");  
     return;
    }

 if (strstr(newname,"^")) {
    write_str(user,"Name cannot have color or hilites in it.");
    return;
    }

  if (strlen(newname)>NAME_LEN-1) 
    {
     write_str(user,"Name too long");  
     return;
    }

	/* see if only letters in login */
     for (f=0; f<strlen(newname); ++f) 
       {
         if (!isalpha((int)newname[f]) || newname[f]<'A' || newname[f] >'z') 
           {
	     write_str(user,"Name can only contain letters.");
	     return;
	   }
       }
        

  /*----------------------------------------------------------------*/
  /* copy capitalized name to a temp array and convert to lowercase */
  /*----------------------------------------------------------------*/
  
  strcpy(lowername,newname);
  strtolower(lowername);
 
 if (!strcmp(other_user, lowername))
    {
        write_str(user,"\nNew name cannot be the login name. \nName not changed."); 
        return;  
    }

if (check_for_user(lowername))
    {
      write_str(user,"Sorry, that name is already used.");
      return;
    }
  
  sprintf(mess,"changed %s\'s name to %s",other_user,newname);
  btell(user,mess); 
  strcpy(t_ustr.name,lowername);   
  strcpy(t_ustr.login_name,lowername);   
  strcpy(t_ustr.say_name,newname);   
  write_user(lowername);
  if ((u=get_user_num_exact(other_user,user)) != -1) 
    {
     strcpy(ustr[u].name,lowername);
     strcpy(ustr[u].login_name,lowername);
     sprintf(mess,"^*^ ^HY%s^ has had their name changed to ^HY%s^ ^*^",strip_color(ustr[u].say_name),newname);
     writeall_str(mess, 0, user, 1, user, NORM, BCAST, 0);
     strcpy(ustr[u].say_name,newname);
    }

/* change exempt file */
/* first arguement is to check against names in file for user */
/* second is the new name we're changing to */
change_exem_data(other_user,newname);

   /* move user's INBOX mail file to new name */
  sprintf(filename,"%s/%s",MAILDIR,other_user);
  sprintf(filename2,"%s/%s",MAILDIR,lowername);
  rename(filename,filename2);

   /* move user's SENT mail file to new name */
  sprintf(filename,"%s/%s.sent",MAILDIR,other_user);
  sprintf(filename2,"%s/%s.sent",MAILDIR,lowername);
  rename(filename,filename2);

   /* move user's EMAIL file to new name */
  sprintf(filename,"%s/%s.email",MAILDIR,other_user);
  sprintf(filename2,"%s/%s.email",MAILDIR,lowername);
  rename(filename,filename2);

   /* move user's PROFILE file to new name */
  sprintf(filename,"%s/%s",PRO_DIR,other_user);
  sprintf(filename2,"%s/%s",PRO_DIR,lowername);
  rename(filename,filename2);

   /* move user's WARNING file to new name */
  sprintf(filename,"%s/%s",WLOGDIR,other_user);
  sprintf(filename2,"%s/%s",WLOGDIR,lowername);
  rename(filename,filename2);

  nuke(user,other_user,0);
  sprintf(mess,"%s Had the name ^%s^, but is renamed now to ^%s^ by ^HR%s^"
              ,lowername,other_user,newname,ustr[user].say_name);
  warning(user,mess,0);
  newname[0]=0;
  lowername[0]=0;
}

/** superusers changing user's passwords **/
void supass(int user, char *inpstr)
{
int u;
char other_user[ARR_SIZE],newpass[ARR_SIZE];


if (!strlen(inpstr)) {
   write_str(user,"Whose password do you want to change?");
   return;
   }

sscanf(inpstr,"%s ",other_user);

/* plug security hole */
if (check_fname(other_user,user)) 
  {
   write_str(user,"Illegal name.");
   return;
  }

strtolower(other_user);
CHECK_NAME(other_user);

if (!read_user(other_user))
  {
   write_str(user,NO_USER_STR);
   return;
  }

 if ((t_ustr.super >= ustr[user].tempsuper) && strcmp(ustr[user].name,ROOT_ID))
  {
    strcpy(t_mess,"You cannot repassword a user of same or higher rank.");
    write_str(user,t_mess);
    return;
  }
  
remove_first(inpstr);  
   
sscanf(inpstr,"%s",newpass);

 if (newpass[0]<32 || strlen(newpass)< 3) 
    {
     write_str(user,SYS_PASSWD_INVA);  
     return;
    }

 if (strstr(newpass,"^")) {
    write_str(user,"Password cannot have color or hilites in it.");
    return;
    }
        
  if (strlen(newpass)>NAME_LEN-1) 
    {
     write_str(user,SYS_PASSWD_LONG);  
     return;
    }

  /*-------------------------------------------------------------*/
  /* convert password to lowercase and encrypt the password      */
  /*-------------------------------------------------------------*/
  
  strtolower(newpass);  
 
 if (!strcmp(other_user, newpass))
    {
        write_str(user,"\nPassword cannot be the login name. \nPassword not changed."); 
        return;  
    }
  
  sprintf(mess,"Password for %s changed to %s",t_ustr.say_name,newpass);
  write_str(user,mess); 
  
  st_crypt(newpass);                                   
  strcpy(t_ustr.password,newpass);   
  strcpy(t_ustr.login_pass,newpass);   
  write_user(other_user);
  if ((u=get_user_num_exact(other_user,user)) != -1) 
    {
     strcpy(ustr[u].password,newpass);
     strcpy(ustr[u].login_pass,newpass);
    }
  newpass[0]=0; 
} 


/** list abbreviations **/
void abbrev(int user, char *inpstr)
{
int i=0;
int c=0;
int num=0;
int found=0;
char abbrcom[ARR_SIZE];

if (!strlen(inpstr)) {
 write_str(user,"Command           Yours  Default");
 write_str(user,"-------------     -----  -------");

  for (i=0;i<NUM_ABBRS;++i) {

     for (c=0; sys[c].su_com != -1; ++c) {
      if (!strcmp(sys[c].command,ustr[user].custAbbrs[i].com)) {
     sprintf(mess,"%-12s <-->   %s      %s", ustr[user].custAbbrs[i].com,
	ustr[user].custAbbrs[i].abbr,sys[c].cabbr);
     write_str(user,mess);
     break;
     }
    } /* end of c for */

   } /* end of i for */

  write_str(user,"Ok.");
  return;
 }

/* User wants the defaults back */
if (!strcmp(inpstr,"-d")) {
    write_str(user,"Resetting abbreviations to defaults..");
    initabbrs(user);
    write_str(user,"Ok.");
    return;
    }

sscanf(inpstr,"%s ",abbrcom);
strtolower(abbrcom);
i=0;

/* Make sure command name is one that is available for abbreviation */
 for (i=0;i<NUM_ABBRS;++i) {
  if (!strcmp(abbrcom,ustr[user].custAbbrs[i].com)) {
     found=1;
     num=i;
     break;
     }
  }

 if (!found) {
   write_str(user,"That command is not available for abbreviation.");
   return;
   }

 remove_first(inpstr);

 if (!strlen(inpstr)) {
    sprintf(mess,"Your abbreviation for %s is %s",ustr[user].custAbbrs[num].com,ustr[user].custAbbrs[num].abbr);
    write_str(user,mess);
    return;
    }

 if (strlen(inpstr) > 1) {
   write_str(user,"Abbreviation too long.");
   return;
   }

/* Check for letters, numbers, and a period */
 if (inpstr[0] == '.' || isalnum((int)inpstr[0])) {
   write_str(user,"You can't use that key for a abbreviation.");
   return;
   }

i=0;
found=0;

/* Check if abbreviation is being used by another command already */
 for (i=0;i<NUM_ABBRS;++i) {
       if (!strcmp(ustr[user].custAbbrs[i].abbr,inpstr)) {
        found=1;
        break;
       }
     }

 if (found) {
    write_str(user,"That abbreviation is already being used by another command.");
    return;
    }
       
 if (inpstr[0]=='\\')
     strcpy(ustr[user].custAbbrs[num].abbr,"\\");
 else if (inpstr[0]=='\'')
     strcpy(ustr[user].custAbbrs[num].abbr,"\'");
 else if (inpstr[0]=='\"')
     strcpy(ustr[user].custAbbrs[num].abbr,"\"");
 else if (inpstr[0]=='\?')
     strcpy(ustr[user].custAbbrs[num].abbr,"\?");
 else
     strcpy(ustr[user].custAbbrs[num].abbr,inpstr);
 
/*
 strcpy(ustr[user].custAbbrs[num].abbr,inpstr);
*/

 copy_from_user(user);
 write_user(ustr[user].name);
 write_str(user,"Abbreviation changed.");
}


/** short who with new mud form **/
void newwho(int user)
{
int u,v,idl,invis=0;
char un[SAYNAME_LEN+10];
char amess[5000];
char bmess[5000];
char cmess[5000];
char dmess[5000];
char emess[500];
char fmess[100];

strcpy(amess,"");
strcpy(bmess,"");
strcpy(cmess,"");
strcpy(dmess,"");
strcpy(emess,"");
strcpy(fmess,"");

for (v=0;v<NUM_AREAS;++v) 
     {
       for (u=0;u<MAX_USERS;++u) {
	if ((ustr[u].area!= -1) && (ustr[u].area == v) && (!ustr[u].logging_in))
	        {
	   	  if (!ustr[u].vis && ustr[user].tempsuper < MIN_HIDE_LEVEL)
	             { 
	              invis++;  
	              continue; 
	              }
			 
		 idl=(int)((time(0)-ustr[u].last_input)/60);
		
		 strcpy(un,ustr[u].say_name);
                 strcat(un," ");

                 if (idl < 3) {  
                    strcat(amess,un);
                    } 
                 if (idl >= 3 && idl < 60) {
                    strcat(bmess,un);
                    }
                 if (idl >= 60 && idl < 180) {
                    strcat(cmess,un);
                    }
                 if (idl >= 180) {
                    strcat(dmess,un);
                    }
                 }
        }
     }
 if (strlen(amess) >= 3) {
    amess[strlen(amess)]=0;
    sprintf(mess,"  Active: %s",amess);
    write_str(user,mess);
    }
 if (strlen(bmess) >= 3) {
    bmess[strlen(bmess)]=0;
    sprintf(mess,"   Awake: %s",bmess);
    write_str(user,mess);
    }
 if (strlen(cmess) >= 3) {
    cmess[strlen(cmess)]=0;
    sprintf(mess,"    Idle: %s",cmess);
    write_str(user,mess);
    }
 if (strlen(dmess) >= 3) {
    dmess[strlen(dmess)]=0;
    sprintf(mess,"Comatose: %s",dmess);
    write_str(user,mess);
    }

sprintf(emess,SHORT_WHO,num_of_users,num_of_users == 1 ? "" : "s",num_of_users == 1 ? "is" : "are");

if (invis) {
   sprintf(fmess," (%d invis)",invis);
   strcat(emess,fmess);
   } 
write_str(user,emess);

}


/** check last login for specified user **/
void last_u(int user, char *inpstr)
{
int on_now=0;
int u;
char other_user[ARR_SIZE],ldate[20];
time_t tm_then;

if (!strlen(inpstr)) {
   write_str(user,"Who do you want to see the lastlog of?");
   return;
   } 

sscanf(inpstr,"%s",other_user);

strtolower(other_user);
CHECK_NAME(other_user);

   if ((u = get_user_num(other_user,user)) != -1) {
      if (!strcmp(ustr[u].name,other_user))
       on_now = 1;
      }

if (!on_now) {
if (!read_user(other_user))
   {
    write_str(user,NO_USER_STR);
    return;
   }
midcpy(t_ustr.last_date,ldate,0,15);
write_str(user,"");
sprintf(mess,"^LR-->^    %s was last here on %s",t_ustr.say_name,ldate);
write_str(user,mess);
tm_then=((time_t) t_ustr.rawtime);
sprintf(mess,"^LR-->^    That was %s ago.",converttime((long)((time(0)-tm_then)/60)));
write_str(user,mess);
write_str(user,"");
}
else { 
  write_str(user,"");
  if (!ustr[u].vis) {
	if (ustr[user].tempsuper >= MIN_HIDE_LEVEL)
	 sprintf(mess,"^LR-->^    %s is online right now!! (*)   ^LR<--^",ustr[u].say_name);
	else {
	 /* target user is invisible and doing user does not have the */
	 /* power to see them */
	 midcpy(ustr[u].last_date,ldate,0,15);
	 sprintf(mess,"^LR-->^    %s was last here on %s",ustr[u].say_name,ldate);
	 write_str(user,mess);
	 tm_then=((time_t) ustr[u].rawtime);
	 sprintf(mess,"^LR-->^    That was %s ago.",converttime((long)((time(0)-tm_then)/60)));
	}
  } /* !vis */
  else {
  sprintf(mess,"^LR-->^    %s is online right now!!    ^LR<--^",ustr[u].say_name);
  } /* vis */
  write_str(user,mess);
  write_str(user,"");
 } /* online */
}

/*** search for specific talker in the talker list, add a talker ***/
/*** modify a talker, or delete a talker                         ***/
void talker(int user, char *inpstr)
{
int occured=0;
int i=0;
int num=0;
int num2=0;
int alpha=0;
int lasthost=0;
char filename[FILE_NAME_LEN],filename2[FILE_NAME_LEN];
char line[ARR_SIZE],line2[ARR_SIZE];
FILE *fp;
FILE *fp2;

if (!ustr[user].t_ent) {
 ustr[user].t_num        = 0;
 ustr[user].t_name[0]    = 0;
 ustr[user].t_host[0]    = 0;
 ustr[user].t_ip[0]      = 0;
 ustr[user].t_port[0]    = 0;

 if (!strlen(inpstr)) 
   {
	sprintf(t_mess,"%s",TALKERLIST);
	strncpy(filename,t_mess,FILE_NAME_LEN);
        if (!check_for_file(filename)) {
           write_str(user,NO_TLIST);
           return;
          }
        write_str(user,"    Name                    Hostname                       Number Address  Port");
        write_str(user,"    ----------------------- ------------------------------ --------------- ----");
        cat(filename,user,1);
   }
 else {
	sprintf(t_mess,"%s",TALKERLIST);
	strncpy(filename,t_mess,FILE_NAME_LEN);

 if (!strcmp(inpstr,"-a")) {
   START:
    ustr[user].t_ent=1;
    write_str(user,"Enter a \"q\" by itself on any line to abort");
    write_str(user,"");
    write_str_nr(user,"Enter the name of the talker: ");
	 telnet_write_eor(user);
    noprompt=1; return;
   } /* end of -a if */
 else if (!strncmp(inpstr,"-d",2)) {
   if (ustr[user].tempsuper < WIZ_LEVEL) {
     write_str(user,NOT_WORTHY);
     return;
     }
    remove_first(inpstr);
   if (!strlen(inpstr) || (strlen(inpstr)>3)) {
     write_str(user,"You must enter the number of the talker you wish to delete.");
     return;
    }
   num=atoi(inpstr);
   num2=file_count_lines(filename);
   if ((num==0) || (num>num2)) {
     write_str(user,"That is not a valid number.");
     return;
    }
   if (!(fp=fopen(filename,"r"))) { 
     write_str(user,"Can't open talker list! May not exist");
     return;
    }
   strcpy(filename2,get_temp_file());
   if (!(fp2=fopen(filename2,"a"))) { 
        write_str(user,BAD_FILEIO);
	write_log(ERRLOG,YESTIME,"Couldn't open tempfile(a) in talker! %s\n",get_error());
     return;
    }
   strcpy(line,"");
   line[0]=0;
   while (fgets(line,80,fp)!=NULL) {
    line[strlen(line)-1]=0;
    i++;
    if (i==num) { strcpy(line,""); line[0]=0; continue; }
    else {
     fputs(line,fp2); fputs("\n",fp2);
     strcpy(line,""); line[0]=0;
     }
   }
   i=0;
   write_str(user,"Talker entry deleted.");
   FCLOSE(fp);
   FCLOSE(fp2);
   remove(filename);
   rename(filename2,filename);
   ustr[user].t_num        = 0;
   ustr[user].t_name[0]    = 0;
   ustr[user].t_host[0]    = 0;
   ustr[user].t_ip[0]      = 0;
   ustr[user].t_port[0]    = 0;
   return;
  } /* end of -d if */
 else if (isdigit((int)inpstr[0]) && (strlen(inpstr)<4)) {
   if (ustr[user].tempsuper < WIZ_LEVEL) {
     write_str(user,NOT_WORTHY);
     return;
     }
   num=atoi(inpstr);
   num2=file_count_lines(filename);
   if ((num==0) || (num>num2)) {
     write_str(user,"That is not a valid number.");
     return;
    }
   write_str(user,"Modifying talker info..");
   ustr[user].t_num=num;
   goto START;
   } /* end of modify if */
 else {
	strtolower(inpstr);
        /* look through list */
	if (!(fp=fopen(filename,"r"))) { 
           write_str(user,"Can't open talker list! May not exist");
           return;
           }
        strcpy(line,""); line[0]=0;
	while(fgets(line,80,fp)!=NULL) {
                num++;
	        line[strlen(line)-1]=0;
                strcpy(line2,line);
                strtolower(line2);
		if (instr2(0,line2,inpstr,0)== -1) goto NEXT;
                if (!occured) {
        write_str(user,"    Name                    Hostname                       Number Address  Port");
        write_str(user,"    ----------------------- ------------------------------ --------------- ----");
                 }
                 if (ustr[user].tempsuper >= LINENUM_LEVEL) {
		  sprintf(mess,"%-3d ",num);
		  write_str_nr(user,mess);
                  }
		  write_str(user,line);	
		  ++occured;
		NEXT:
                strcpy(line,""); line[0]=0;
		continue;
		}
	FCLOSE(fp);

   if (!occured) write_str(user,"No occurences found");
   else {
        write_str(user," ");
	sprintf(mess,"%d occurence%s found",occured,occured == 1 ? "" : "s");
	write_str(user,mess);
	}
   num=0;
   } /* end of search else */
  } /* end of strlen inpstr else */
 } /* end of if t_ent */
else {
    if ((strlen(inpstr) < 5) && ustr[user].t_ent>0 && ustr[user].t_ent<4) {
	if (is_quit(user,inpstr)) return;
      if (ustr[user].t_ent==1) {
       write_str(user,"Name too short.");
       write_str_nr(user,"Enter the name of the talker: ");
	telnet_write_eor(user);
       noprompt=1; return;
       }
      else if (ustr[user].t_ent==2) {
       write_str(user,"Hostname too short.");
       write_str(user,"Use \"unknown\" if you dont know the name address");
       write_str_nr(user,"Enter the hostname address (w/o port): ");
	telnet_write_eor(user);
       noprompt=1; return;
       }
      else if (ustr[user].t_ent==3) {
        write_str(user,"Address too short.");
        write_str(user,"Use \"unknown\" if you dont know the numeric address");
        write_str_nr(user,"Enter the talker numeric address (w/o port): ");
	telnet_write_eor(user);
        noprompt=1; return;
       }
     } /* end of if strlen inpstr */
   if (ustr[user].t_ent==1) {
      inpstr[23]=0;
      strcpy(ustr[user].t_name,inpstr);
      if (isalpha((int)ustr[user].t_name[0])) ustr[user].t_name[0]=toupper((int)ustr[user].t_name[0]);
      write_str_nr(user,"Enter hostname address (w/o port): ");
	telnet_write_eor(user);
      ustr[user].t_ent=2;
      noprompt=1; return;
     }
   else if (ustr[user].t_ent==2) {
      inpstr[30]=0;
      strcpy(ustr[user].t_host,inpstr);
      write_str_nr(user,"Enter the talker numeric address (w/o port): ");
	telnet_write_eor(user);
      ustr[user].t_ent=3;
      noprompt=1; return;
     }
   else if (ustr[user].t_ent==3) {
      inpstr[15]=0;
      strcpy(ustr[user].t_ip,inpstr);
      write_str_nr(user,"Enter the talker port number: ");
	telnet_write_eor(user);
      ustr[user].t_ent=4;
      noprompt=1; return;
     }
   else if (ustr[user].t_ent==4) {
      inpstr[5]=0;
      strcpy(ustr[user].t_port,inpstr);
      sprintf(t_mess,"%s",TALKERLIST);
      strncpy(filename,t_mess,FILE_NAME_LEN);
      if (!(fp=fopen(filename,"r"))) {
        fp=fopen(filename,"a");
        sprintf(mess,"%-23s %-30s %-15s %s\n",ustr[user].t_name,
                ustr[user].t_host,ustr[user].t_ip,ustr[user].t_port);      
        ustr[user].t_ent=0;
        write_str(user,"Talker added.");
        ustr[user].t_num        = 0;
        ustr[user].t_name[0]    = 0;
        ustr[user].t_host[0]    = 0;
        ustr[user].t_ip[0]      = 0;
        ustr[user].t_port[0]    = 0;
        fputs(mess,fp);
        FCLOSE(fp);
        return;
       }
      else {
       if (isalpha((int)ustr[user].t_name[0]) ||
           isdigit((int)ustr[user].t_name[0])) alpha=1;
       else alpha=2;

       strcpy(filename2,get_temp_file());
   if (!(fp2=fopen(filename2,"a"))) { 
        write_str(user,BAD_FILEIO);
	write_log(ERRLOG,YESTIME,"Couldn't open tempfile(a) in talker! %s\n",get_error());
        alpha=0;
        ustr[user].t_ent        = 0;
        ustr[user].t_num        = 0;
        ustr[user].t_name[0]    = 0;
        ustr[user].t_host[0]    = 0;
        ustr[user].t_ip[0]      = 0;
        ustr[user].t_port[0]    = 0;
     FCLOSE(fp);
     return;
    }
   if (ustr[user].t_num) {
    strcpy(line,""); line[0]=0;
    while (fgets(line,80,fp)!=NULL) {
     line[strlen(line)-1]=0;
     i++;
     if (i==ustr[user].t_num) {
       if (alpha!=3) lasthost=1;
       continue;
      }
     else {
      if (alpha==3) {
         fputs(line,fp2); fputs("\n",fp2);
         lasthost=0;
         }
      else if ((alpha==2) || (alpha==1)) {
        if (line[0]<ustr[user].t_name[0]) {
         fputs(line,fp2); fputs("\n",fp2);
         lasthost=1;
         }
        else if (line[0]==ustr[user].t_name[0]) {
         if (line[1]<ustr[user].t_name[1]) {
          fputs(line,fp2); fputs("\n",fp2);
          lasthost=1;
          }
         else if (line[1]==ustr[user].t_name[1]) {
          fputs(line,fp2); fputs("\n",fp2);
          sprintf(mess,"%-23s %-30s %-15s %s\n",ustr[user].t_name,
                ustr[user].t_host,ustr[user].t_ip,ustr[user].t_port);      
          fputs(mess,fp2);
          alpha=3;
          lasthost=0;
          }
         else if (line[1]>ustr[user].t_name[1]) {
          sprintf(mess,"%-23s %-30s %-15s %s\n",ustr[user].t_name,
                ustr[user].t_host,ustr[user].t_ip,ustr[user].t_port);      
          fputs(mess,fp2);
          fputs(line,fp2); fputs("\n",fp2);
          alpha=3;
          lasthost=0;
          }
         } /* end of else if line equals */
        else if (line[0]>ustr[user].t_name[0]) {
         sprintf(mess,"%-23s %-30s %-15s %s\n",ustr[user].t_name,
                ustr[user].t_host,ustr[user].t_ip,ustr[user].t_port);      
         fputs(mess,fp2);
         fputs(line,fp2); fputs("\n",fp2);
         lasthost=0;
         alpha=3;
         }
       } /* end of else if alpha */
      } /* end of else */
     strcpy(line,""); line[0]=0;
     } /* end of while */
     if (lasthost) {
          sprintf(mess,"%-23s %-30s %-15s %s\n",ustr[user].t_name,
                ustr[user].t_host,ustr[user].t_ip,ustr[user].t_port);      
          fputs(mess,fp2);
          lasthost=0;
       }
     i=0;
     alpha=0;
     ustr[user].t_ent=0;
     ustr[user].t_num        = 0;
     ustr[user].t_name[0]    = 0;
     ustr[user].t_host[0]    = 0;
     ustr[user].t_ip[0]      = 0;
     ustr[user].t_port[0]    = 0;
     write_str(user,"Talker entry modified.");
     FCLOSE(fp);
     FCLOSE(fp2);
     remove(filename);
     rename(filename2,filename);
     return;
    } /* end of if t_num */
    strcpy(line,""); line[0]=0;
   while (fgets(line,80,fp)!=NULL) {
    line[strlen(line)-1]=0;
      if (alpha==3) {
         fputs(line,fp2); fputs("\n",fp2);
         lasthost=0;
         }
      else if ((alpha==2) || (alpha==1)) {
        if (line[0]<ustr[user].t_name[0]) {
         fputs(line,fp2); fputs("\n",fp2);
         lasthost=1;
         }
        else if (line[0]==ustr[user].t_name[0]) {
         if (line[1]<ustr[user].t_name[1]) {
          fputs(line,fp2); fputs("\n",fp2);
          lasthost=1;
          }
         else if (line[1]==ustr[user].t_name[1]) {
          fputs(line,fp2); fputs("\n",fp2);
          sprintf(mess,"%-23s %-30s %-15s %s\n",ustr[user].t_name,
                ustr[user].t_host,ustr[user].t_ip,ustr[user].t_port);      
          fputs(mess,fp2);
          alpha=3;
          lasthost=0;
          }
         else if (line[1]>ustr[user].t_name[1]) {
          sprintf(mess,"%-23s %-30s %-15s %s\n",ustr[user].t_name,
                ustr[user].t_host,ustr[user].t_ip,ustr[user].t_port);      
          fputs(mess,fp2);
          fputs(line,fp2); fputs("\n",fp2);
          alpha=3;
          lasthost=0;
          }
         } /* end of else if line equals */
        else if (line[0]>ustr[user].t_name[0]) {
         sprintf(mess,"%-23s %-30s %-15s %s\n",ustr[user].t_name,
                ustr[user].t_host,ustr[user].t_ip,ustr[user].t_port);      
         fputs(mess,fp2);
         fputs(line,fp2); fputs("\n",fp2);
         lasthost=0;
         alpha=3;
         }
       } /* end of else if alpha */
     strcpy(line,""); line[0]=0;
    } /* end of while */
    if (lasthost) {
          sprintf(mess,"%-23s %-30s %-15s %s\n",ustr[user].t_name,
                ustr[user].t_host,ustr[user].t_ip,ustr[user].t_port);      
          fputs(mess,fp2);
          lasthost=0;
      }
    ustr[user].t_ent=0;
    ustr[user].t_num        = 0;
    ustr[user].t_name[0]    = 0;
    ustr[user].t_host[0]    = 0;
    ustr[user].t_ip[0]      = 0;
    ustr[user].t_port[0]    = 0;
    alpha=0;
    write_str(user,"Talker added.");
    FCLOSE(fp);
    FCLOSE(fp2);
    remove(filename);
    rename(filename2,filename);
  } /* end of filename else */
 } /* end of else if */
 } /* end of t_ent else */
strcpy(line,""); line[0]=0; i=0;
}

/** bbbbbbbuuubbles for eve **/
void bubble(int user)
{
sprintf(mess,"OoOoo Ooo oOOo oOo oOooOOo OooOOooo oOoOooO ooo Ooo");
write_str(user,mess);
writeall_str(mess, 1, user, 0, user, NORM, ECHOM, 0);
}


/** locate a user, online or not **/
void where(int user, char *inpstr)
{
char other_user[ARR_SIZE];
int u;

if  (!strlen(inpstr)) {
      for (u=0; u < MAX_USERS; ++u) {
            if (ustr[u].area!= -1) {
           if (!ustr[u].vis && ustr[user].tempsuper < MIN_HIDE_LEVEL) continue;
           if (astr[ustr[u].area].hidden) {
                if ((ustr[user].tempsuper >= ROOMVIS_LEVEL) && SHOW_HIDDEN)
                 sprintf(mess,"%s is in <%s>",ustr[u].say_name,astr[ustr[u].area].name);
                else
                 sprintf(mess,"%s is in ??",ustr[u].say_name);
               }
           else
              sprintf(mess,"%s is in %s",ustr[u].say_name,astr[ustr[u].area].name);
           write_str(user,mess);
         }
       }
    return;
    }

sscanf(inpstr,"%s ",other_user);
strtolower(other_user);
CHECK_NAME(other_user);

if (!check_for_user(other_user)) 
  {
   write_str(user,NO_USER_STR);
   return;
  }

 if ((u=get_user_num(other_user,user))== -1)
    {
     read_user(other_user);
     if (astr[t_ustr.area].hidden)
    sprintf(mess,"%s was last seen in ??",t_ustr.say_name);
     else
    sprintf(mess,"%s was last seen in %s",t_ustr.say_name,astr[t_ustr.area].name);
    write_str(user,mess);
    }
   else {  
     if (astr[ustr[u].area].hidden) {
       if ((ustr[user].tempsuper >= ROOMVIS_LEVEL) &&  SHOW_HIDDEN)
        sprintf(mess,"%s is in <%s>",ustr[u].say_name,astr[ustr[u].area].name);
      else
        sprintf(mess,"%s is in ??",ustr[u].say_name);
     }
     else
    sprintf(mess,"%s is in %s",ustr[u].say_name,astr[ustr[u].area].name);
    write_str(user,mess);
    }
}


/** forward mail to an email address **/
void fmail(int user, char *inpstr)
{
char mail_addr[ARR_SIZE];
char temp[EMAIL_LENGTH+1];
char filename2[FILE_NAME_LEN];

if (!strlen(inpstr)) {
    write_str(user,"Where do you want to forward your mail?");
    write_str(user,"Specify a valid email address or use *mine* to use your set address.");
    return;
    }

/* Check for illegal characters in email addy */
if (strpbrk(inpstr,";/[]\\") ) {
   write_str(user,"Illegal email address");
   return;
   }

if (strstr(inpstr,"^")) {
   write_str(user,"Email can't have color or hilite codes in it");
   return;
   }

 inpstr[EMAIL_LENGTH-1]=0;
 strcpy(temp,inpstr);
 strtolower(temp);

 if (strstr(temp,"whitehouse.gov"))
      {
       write_str(user,"Email address not valid.");
       return;
      }

sprintf(filename2,"%s/%s",MAILDIR,ustr[user].name);

if (!check_for_file(filename2)) {
    write_str(user,"You don't have a mailfile to forward!");
    return;
    }

/* set recipient */
if (!strcmp(inpstr,"mine") || !strcmp(inpstr,"Mine")) {
    if (!strcmp(ustr[user].email_addr,DEF_EMAIL)) {
       write_str(user,"Email address not valid.");
       return;
      }
    else
     strcpy(mail_addr,ustr[user].email_addr);
  }
else if (!strstr(inpstr,".") || !strstr(inpstr,"@")) {
       write_str(user,"Email address not valid.");
       return;
      }
else strcpy(mail_addr,inpstr);

/* send external email */
if (send_ext_mail(-2, -2, 2, NULL, filename2, DATA_IS_FILE, mail_addr)==-1) {
	write_log(ERRLOG,YESTIME,"FMAIL: Couldn't send external email in fmail()\n");
	return;
}

write_str(user,"Mail forwarded.");
write_str(user,"**Note: If email address is not valid, mail will bounce.");

}


/*** wipe suggestionboard (erase file) ***/
void swipe(int user, char *inpstr)
{
char filename[FILE_NAME_LEN];
FILE *bfp;
int lower=-1;
int upper=-1;
int mode=0;

   write_str(user,"***Suggestion Wipe***");
   sprintf(t_mess,"%s/suggs",MESSDIR);
  
strncpy(filename,t_mess,FILE_NAME_LEN);

/*---------------------------------------------*/
/* check if there is any mail                  */
/*---------------------------------------------*/

if (!(bfp=fopen(filename,"r"))) 
  {
   write_str(user,"There are no messages to Swipe off the board."); 
   return;
  }
FCLOSE(bfp);

/*---------------------------------------------*/
/* get the delete parameters                   */
/*---------------------------------------------*/

get_bounds_to_delete(inpstr, &lower, &upper, &mode);
 
if (upper == -1 && lower == -1)
  {
   write_str(user,"No messages wiped.  Specification of what to ");
   write_str(user,"wipe did not make sense.  Type: .help wipe ");
   write_str(user,"for detailed instructions on use. ");
   return;
  }
    
   switch(mode)
    {
     case 0: return;
             break;
        
     case 1: 
            sprintf(mess,"SWiped all messages.");
            upper = -1;
            lower = -1;
            break;
        
     case 2: 
            sprintf(mess,"SWiped line %d.", lower);
            
            break;
        
     case 3: 
            sprintf(mess,"SWiped from line %d to the end.",lower);
            break;
        
     case 4: 
            sprintf(mess,"SWiped from begining of board to line %d.",upper);
            break;
        
     case 5: 
           if (lower == -1)
            sprintf(mess,"SWiped all except line %d.",upper);
           else
            sprintf(mess,"SWiped all except lines %d to %d.",upper, lower);

            break;
        
     case 6: 
            sprintf(mess,"SWiped from line %d to %d.", lower, upper);
            break;

     default: return;
              break;
    }


remove_lines_from_file(user, 
                       filename, 
                       lower, 
                       upper);

write_str(user,mess);
if (!file_count_lines(filename)) remove(filename);

}

/*-----------------------------------------------*/
/* anchor a user down                            */
/*-----------------------------------------------*/
void anchor_user(int user, char *inpstr)
{
char other_user[ARR_SIZE];
int u,inlen;
unsigned int i;

if (!strlen(inpstr)) 
  {
   write_str(user,"Users Anchored & logged on     Time left"); 
   write_str(user,"--------------------------     ---------"); 
   for (u=0;u<MAX_USERS;++u) 
    {
     if (ustr[u].anchor  && ustr[u].area > -1) 
       {
        if (ustr[u].anchor_time == 0)
           sprintf(mess,"%-*s   %s",NAME_LEN+count_color(ustr[u].say_name,0),ustr[u].say_name,"Perm");
        else
           sprintf(mess,"%-*s   %s",NAME_LEN+count_color(ustr[u].say_name,0),ustr[u].say_name,converttime((long)ustr[u].anchor_time));
        write_str(user, mess);
       };
    }
   write_str(user,"(end of list)");
   return;
  }

sscanf(inpstr,"%s ",other_user);
strtolower(other_user);

if ((u=get_user_num(other_user,user))== -1) 
  {
   not_signed_on(user,other_user);
   return;
  }
  
if (u == user)
  {   
   write_str(user,"You are definitly wierd! Trying to anchor yourself, geesh."); 
   return;
  }

 if ((!strcmp(ustr[u].name,ROOT_ID)) || (!strcmp(ustr[u].name,BOT_ID)
      && strcmp(ustr[user].name,ROOT_ID))) {
    write_str(user,"Yeah, right!");
    return;
    }

if ((ustr[user].tempsuper <= ustr[u].super) && strcmp(ustr[user].name,ROOT_ID)) 
  {
   write_str(user,"That would not be wise...");
   sprintf(mess,ANCHOR_CANT,ustr[user].say_name);
   write_str(u,mess);
   return;
  }

if (ustr[u].anchor == 0) {
remove_first(inpstr);
if (strlen(inpstr) && strcmp(inpstr,"0")) {
   if (strlen(inpstr) > 5) {
      write_str(user,"Minutes cant exceed 5 digits.");
      return;
      }
   inlen=strlen(inpstr);
   for (i=0;i<inlen;++i) {
     if (!isdigit((int)inpstr[i])) {
        write_str(user,"Numbers only!");
        return;
        }
     }
    i=0;
    i=atoi(inpstr);
    if ( i > 32767) {
       write_str(user,"Minutes cant exceed 32767.");
       i=0;
       return;
      }
  i=0;
  ustr[u].anchor_time=atoi(inpstr);
  ustr[u].anchor = 1;
    write_str(u,ANCHORON_MESS);
    sprintf(mess,"ANCHOR: ON for %s by %s for %s\n",ustr[u].say_name, ustr[user].say_name, converttime((long)ustr[u].anchor_time));
}
else {
    ustr[u].anchor = 1;
    ustr[u].anchor_time=0;
    write_str(u,ANCHORON_MESS);
    sprintf(mess,"ANCHOR: ON for %s by %s\n",ustr[u].say_name, ustr[user].say_name);
  }
}     /* end of if anchored */

else {
    ustr[u].anchor = 0;
    ustr[u].anchor_time=0;
    write_str(u,ANCHOROFF_MESS);
    sprintf(mess,"ANCHOR: OFF for %s by %s\n",ustr[u].say_name, ustr[user].say_name);
  } 

btell(user, mess);

write_log(SYSTEMLOG,YESTIME,mess);
write_str(user,"Ok");
}


/** turn welcome quote on or off **/
void quote_op(int user, char *inpstr)
{
char line[257];
FILE *pp;

if (!strlen(inpstr)) {
  if (ustr[user].quote)
    {
      write_str(user, "Quote feature now  off.");
      ustr[user].quote = 0;
    }
   else
    {
      write_str(user,"Quote feature now  on.");
      ustr[user].quote = 1;
    }
    
  read_user(ustr[user].login_name);
  t_ustr.quote = ustr[user].quote;
  write_user(ustr[user].login_name);
 }
else {

if (!strcmp(inpstr,"-l") || !strcmp(inpstr,"-s")) {
write_str(user,"+---------------------------------------------------------------------------+");
   if (!strcmp(inpstr,"-l"))
    sprintf(t_mess,"%s 2> /dev/null",FORTPROG);
   else if (!strcmp(inpstr,"-s"))
    sprintf(t_mess,"%s -s 2> /dev/null",FORTPROG);

 if (!(pp=popen(t_mess,"r"))) {
        write_str(user,"No quote.");
        return;
        }
while (fgets(line,256,pp) != NULL) {
        line[strlen(line)-1]=0;
        write_str(user,line);
      } /* end of while */
pclose(pp);

write_str(user,"+---------------------------------------------------------------------------+");
 }
 else write_str(user,"Option not understood.");

 } /* end of else */

}


/** list last current logins **/
void list_last(int user, char *inpstr)
{
  int num=0;
  int lines=0;
  char line[257];
  char filename[FILE_NAME_LEN];
  char filename2[FILE_NAME_LEN];
  FILE *fp;
  FILE *fp2;


sprintf(filename2,"%s/%s",LOGDIR,LASTLOGS);

if (!strlen(inpstr)) {
write_str(user,"^HG+----------------------------------------------------------+^");
sprintf(mess,"    ^HYLast 10 logins to %s^",SYSTEM_NAME);
write_str(user,mess);
write_str(user,"^HG+----------------------------------------------------------+^");
write_str(user,"");

  strcpy(filename,get_temp_file());
  lines=file_count_lines(filename2);

  if (!(fp=fopen(filename2,"r"))) {
    write_str(user," No logins");
    write_str(user,"");
    return;
  }
 if (!(fp2=fopen(filename,"w"))) {
	fclose(fp);
        write_str(user,BAD_FILEIO);
	write_log(ERRLOG,YESTIME,"Couldn't open tempfile(w) in list_last! %s\n",get_error());
	return;
	}
  if (lines > 10) file_skip_lines(fp, (lines-10)+1);

  while (fgets(line,256,fp) != NULL) {
   fputs(line, fp2);
  } /* end of while */
  fclose(fp);
  fclose(fp2);

  cat(filename,user,0);
  write_str(user,"");
  return;
}

  for (num=0;num<strlen(inpstr);num++) {
  if (!isdigit((int)inpstr[num])) {
     write_str(user,"Lines from bottom must be a number.");
     return;
     }
  }
  num=0;
  num=atoi(inpstr);

  if (num > 100) {
   write_str(user,"Input number too big. No greater than 100");
   return;
   }

write_str(user,"^HG+----------------------------------------------------------+^");
sprintf(mess,"    ^HYLast %2d logins to %s^",num,SYSTEM_NAME);
write_str(user,mess);
write_str(user,"^HG+----------------------------------------------------------+^");
write_str(user,"");

  strcpy(filename,get_temp_file());
  lines=file_count_lines(filename2);

  if (!(fp=fopen(filename2,"r"))) {
    write_str(user," No logins");
    write_str(user,"");
    return;
  }
 if (!(fp2=fopen(filename,"w"))) {
	fclose(fp);
        write_str(user,BAD_FILEIO);
	write_log(ERRLOG,YESTIME,"Couldn't open tempfile(w) in list_last! %s\n",get_error());
	return;
	}
  if (lines > num) file_skip_lines(fp, (lines-num)+1);

  while (fgets(line,256,fp) != NULL) {
   fputs(line, fp2);
  } /* end of while */
  fclose(fp);
  fclose(fp2);

  cat(filename,user,0);
  write_str(user,"");
}

 
/*-----------------------------------------------------------------*/
/*  The realuser command. returns the real login of a user if the  */
/*  site the user telnet from runs an ident daemon port 113 tcp	   */
/*-----------------------------------------------------------------*/
void real_user(int user, char *inpstr)
{
 char           other_user[ARR_SIZE];
 int	        u;
 unsigned long  inlocal;
 unsigned long  inremote;
 unsigned short local;
 unsigned short remote;
 char         * real_name;

 if (!strlen(inpstr)) 
   {
    write_str(user,"usage: .realuser user_id     - returns the actual login user id if it is available  "); 
    return;
   }
 
 sscanf(inpstr, "%s ", other_user);
 strtolower(other_user);
 
 remove_first(inpstr);
 
 if ((u = get_user_num(other_user, user))== -1) 
   {
    not_signed_on(user, other_user);  
    write_str(user,"- done -");
    return;
   }
   
 /*------------------------------------------*/
 /* if ident has not been run on this user,  */
 /* or if use has no ident, force a run now  */
 /*------------------------------------------*/
 
 if ((ustr[u].real_id[0] == 0) || !strcmp(ustr[u].real_id,"NO IDENT"))
   {
    if (auth_fd2(ustr[u].sock, &inlocal, &inremote, &local, &remote) == -1) {
	write_str(user,"Ident failed, initial socket error");
	return;
    }
 
    if ( (real_name = auth_tcpuser2(inlocal, inremote, local, remote)) == NULL ) 
      {
       sprintf(t_mess,"- NO IDENT @%s -", ustr[u].net_name);
       strcpy(ustr[u].real_id, "NO IDENT");
      } 
     else 
      {
       sprintf(t_mess,"- %s@%s -", real_name, ustr[u].net_name);
       strcpy(ustr[u].real_id, real_name);
      }
   
    write_str(user, t_mess);
 
    write_str(user,"- done (non-cached) -");
   }
  else
   /*----------------------------------------------*/
   /* ident was already run, use the cached info   */
   /*----------------------------------------------*/
   {
    sprintf(t_mess,"- %s@%s -", ustr[u].real_id, ustr[u].net_name);
    write_str(user, t_mess);
    write_str(user,"- done (cached) -");
   }
 
 }

/*** Use system's nslookup command to resolve an ip address ***/
void pukoolsn(int user, char *inpstr)
{
   char line[257];
   char filename[FILE_NAME_LEN];
   FILE *fp;
   FILE *pp;

   if (strpbrk(inpstr,";$/+*[]\\") ) {
        write_str(user,"Illegal character in ip address");
        return;
        }

   if ((!strlen(inpstr)) || (strlen(inpstr) < 7)) {
       write_str(user,"You need to specify a valid ip address");
       return;
       }
   if (strlen(inpstr) > 25) {
       write_str(user,"Address specified too long. No greater than 25 chars.");
       return;
       }
  strcpy(filename,get_temp_file());
  sprintf(mess,"nslookup %s 2> /dev/null",inpstr);
 
 if (!(pp=popen(mess,"r"))) {
	write_str(user,"Can't open pipe to do an nslookup!");
	write_log(ERRLOG,YESTIME,"Couldn't open popen(r) \"%s\" in pukoolsn! %s\n",mess,get_error());
	return;
	}
 if (!(fp=fopen(filename,"w"))) {
        write_str(user,BAD_FILEIO);
	write_log(ERRLOG,YESTIME,"Couldn't open tempfile(w) in pukoolsn! %s\n",get_error());
	return;
	}
while (fgets(line,256,pp) != NULL) {
	fputs(line,fp);
      } /* end of while */
fclose(fp);
pclose(pp);

if (!cat(filename,user,0))
    write_str(user,"No info.");
write_log(SYSTEMLOG,YESTIME,"NSLOOKUP: Query for host %s\n",inpstr);
write_str(user,"Done.");
}

/*** Use system's finger command to get info on troublesome user ***/
void regnif(int user, char *inpstr)
{
   char line[257];
   char filename[FILE_NAME_LEN];
   FILE *fp;
   FILE *pp;

   if (strpbrk(inpstr,";$/+*[]\\") ) {
        write_str(user,"Illegal character in address");
        return;
        }
   if ((!strlen(inpstr)) || (strlen(inpstr) < 8) || (!strstr(inpstr,"@"))) {
       write_str(user,"You need to specify address in form: user@host");
       return;
       }
   if (strlen(inpstr) > 60) {
       write_str(user,"Address specified too long. No greater than 60 chars.");
       return;
       }
strcpy(filename,get_temp_file());
sprintf(mess,"finger %s 2> /dev/null",inpstr);
 if (!(pp=popen(mess,"r"))) {
	write_str(user,"Can't open pipe to do a finger!");
	write_log(ERRLOG,YESTIME,"Couldn't open popen(r) \"%s\" in regnif! %s\n",mess,get_error());
	return;
	}
 if (!(fp=fopen(filename,"w"))) {
        write_str(user,BAD_FILEIO);
	write_log(ERRLOG,YESTIME,"Couldn't open tempfile(w) in regnif! %s\n",get_error());
	return;
	}
while (fgets(line,256,pp) != NULL) {
	fputs(line,fp);
      } /* end of while */
fclose(fp);
pclose(pp);

if (!cat(filename,user,0))
    write_str(user,"No info.");
write_log(ERRLOG,YESTIME,"FINGER: Query for user %s\n",inpstr);
write_str(user,"Done.");
}

/*** Use system's whois command to find what a domain name is ***/
void siohw(int user, char *inpstr)
{
   int i=0;
   char line[257];
   char filename[FILE_NAME_LEN];
   FILE *fp;
   FILE *pp;
   
   if (strpbrk(inpstr,";:$/+*[]\\") ) {
        write_str(user,"Illegal character in search string");
        return;
        }

   if ((!strlen(inpstr)) || (strlen(inpstr) < 3)) {
       write_str(user,"You need to specify a valid search string");
       return;
       }
   if (strlen(inpstr) > 45) {
       write_str(user,"String specified too long. No greater than 45 chars.");
       return;
       }
strcpy(filename,get_temp_file());
i=0;
#if defined(LINUX_SYS)
if (strstr(inpstr,"@"))
 sprintf(mess,"fwhois %s@whois.networksolutions.com 2> /dev/null",inpstr);
else
 sprintf(mess,"fwhois %s 2> /dev/null",inpstr);
#else
if (strstr(inpstr,"@")) {
 for (i=0;i<strlen(inpstr);++i) {
	if (inpstr[i]=='@') break;
 }
 midcpy(inpstr,line,i+1,255);
 inpstr[i]=0;
 sprintf(mess,"whois -h %s %s 2> /dev/null",line,inpstr);
}
else
 sprintf(mess,"whois -h whois.networksolutions.com %s 2> /dev/null",inpstr);
#endif

 if (!(pp=popen(mess,"r"))) {
	write_str(user,"Can't open pipe to do a whois!");
	write_log(ERRLOG,YESTIME,"Couldn't open popen(r) \"%s\" in siohw! %s\n",mess,get_error());
	return;
	}
 if (!(fp=fopen(filename,"w"))) {
        write_str(user,BAD_FILEIO);
	write_log(ERRLOG,YESTIME,"Couldn't open tempfile(w) in siohw! %s\n",get_error());
	return;
	}
while (fgets(line,256,pp) != NULL) {
	fputs(line,fp);
      } /* end of while */
fclose(fp);
pclose(pp);

if (!cat(filename,user,0))
    write_str(user,"No info.");
write_log(ERRLOG,YESTIME,"WHOIS: Query for host %s\n",inpstr);
write_str(user,"Done.");
}

/*** Clear Sent Mail ***/
void clear_sent(int user, char *inpstr)
{
char filename[FILE_NAME_LEN];
FILE *bfp;
int lower=-1;
int upper=-1;
int mode=0;

/*--------------------------------------------------*/
/* check if there is any sent mail                  */
/*--------------------------------------------------*/

sprintf(t_mess,"%s/%s.sent",MAILDIR,ustr[user].name);
strncpy(filename,t_mess,FILE_NAME_LEN);

if (!(bfp=fopen(filename,"r"))) 
  {
   write_str(user,"You have no sent mail."); 
   return;
  }
FCLOSE(bfp);

/* remove the mail file */
if (ustr[user].clrmail== -1) 
  {
   /*---------------------------------------------*/
   /* get the delete parameters                   */
   /*---------------------------------------------*/

   get_bounds_to_delete(inpstr, &lower, &upper, &mode);
 
   if (upper == -1 && lower == -1)
     {
      write_str(user,"No sent mail deleted.  Specification of what to ");
      write_str(user,"delete did not make sense.  Type: .help csent ");
      write_str(user,"for detailed instructions on use. ");
      return;
     }
    
   switch(mode)
    {
     case 0: return;
             break;
        
     case 1: 
            sprintf(mess,"Csent: Delete all sent mail messages? ");
            upper = -1;
            lower = -1;
            break;
        
     case 2: 
            sprintf(mess,"Csent: Delete line %d? ", lower);            
            break;
        
     case 3: 
            sprintf(mess,"Csent: Delete from line %d to the end?",lower);
            break;
        
     case 4: 
            sprintf(mess,"Csent: Delete from begining to line %d?",upper);
            break;
        
     case 5: 
           if (lower == -1)
            sprintf(mess,"Csent: Delete all except line %d?",upper);
           else
            sprintf(mess,"Csent: Delete all except lines %d to %d?",upper, lower);

            break;
        
     case 6: 
            sprintf(mess,"Csent: Delete from line %d to %d?", lower, upper);
            break;

     default: return;
              break;
    }

   ustr[user].lower = lower;
   ustr[user].upper = upper;

   ustr[user].clrmail=user; 
   noprompt=1;
   delete_sent=1;
   write_str(user,mess);
   write_str_nr(user,"Do you wish to do this? (y/n) ");
	telnet_write_eor(user);
   return;
  }
  
remove_lines_from_file(user, 
                       filename, 
                       ustr[user].lower, 
                       ustr[user].upper);

sprintf(mess,"You deleted specified sent mail messages.");
write_str(user,mess);

if (!file_count_lines(filename))  remove(filename);
delete_sent=0;

}

/*** Read Mail ***/
void read_sent(int user, char *inpstr)
{
int b=0;
int a=0;
int lines=0;
int num_lines=0;
char junk[1001];
char filename[FILE_NAME_LEN];
char filename2[FILE_NAME_LEN];
FILE *fp;
FILE *tfp;

if (strlen(inpstr)) {
   if ((strlen(inpstr) <3) && (strlen(inpstr) > 0) &&
      (!isalpha((int)inpstr[0])))
       {
       num_lines=atoi(inpstr);
       a=1;
       }
   else {
       write_str(user,"Number invalid.");
       return;
       }
  }

/* Send output to user */
sprintf(t_mess,"%s/%s.sent",MAILDIR,ustr[user].name);
strncpy(filename,t_mess,FILE_NAME_LEN);

sprintf(mess,"\n** Your Private SENT Mail Console **");
write_str(user,mess);

if (a==1) {
   if (num_lines < 1) {
      if (!cat(filename,user,1))
          {
           write_str(user,"Your sent mail box is empty.");
          }
     goto DONE; 
    }
lines = file_count_lines(filename);
if (num_lines <= lines) {
   ustr[user].numbering = (lines - num_lines) +1;
   }
 else {
      num_lines=0;
      }
num_lines = lines - num_lines;

if (!(fp=fopen(filename,"r"))) {
    write_str(user,"Your sent mail box is empty.");
    num_lines=0; a=0; lines=0;
    ustr[user].numbering = 0;
    return;
    }
strcpy(filename2,get_temp_file());
tfp=fopen(filename2,"w");

while (!feof(fp)) {
         fgets(junk,1000,fp);
         b++;
         if (b <= num_lines)  {
             junk[0]=0;
             continue;
            }
          else {
             fputs(junk,tfp);
             junk[0]=0;
             }
       }
FCLOSE(fp);
FCLOSE(tfp);
num_lines=0;  lines=0;
if (!cat(filename2,user,1))
    write_str(user,"Your sent mail box is empty.");

DONE:
b=0;
}

if (a==0) {
ustr[user].numbering = 0;
if (!cat(filename,user,1))
   write_str(user,"Your sent mail box is empty.");
 }

/* ustr[user].numbering= 0; */
}

/*--------------------------------------------------------------------*/
/* this command basically takes a ip address OR hostname string       */
/* that's inputted by the user and checks for all users with that     */
/* string in their last site.     BY REQUEST *grin*                   */
/*--------------------------------------------------------------------*/
void same_site(int user, char *inpstr)
{
int a=0;
int num,netname=0,foundnum=0;
int len=strlen(inpstr);
char buffer[ARR_SIZE];
char buffer2[ARR_SIZE];
char small_buff[ARR_SIZE];
char filerid[FILE_NAME_LEN];
char filename[FILE_NAME_LEN];
struct dirent *dp;
FILE *fp;
DIR  *dirp;
 
 if (strlen(inpstr) < 3) {
    write_str(user,"Search string too short..spam precaution *grin*");
    return;
    }

num=0;
foundnum=0;
for (a=0;a<len;++a) {
    if (isdigit((int)inpstr[a]) && (num==-2)) {
      num=-1; break;
      }
    else if (isdigit((int)inpstr[a])) {
     foundnum=1;
     num=-3;
     }
    else
     num=-3;

    if (isalpha((int)inpstr[a])) {
      if (strstr(inpstr,".")) {
        num=-1; break;
        }
      else {
        if (foundnum!=1) num=-2;
        else { num=-1; break; }
        } /* end of else */
      } /* end of sub if */

   }  /* end of for */
a=0;

/* Is user search */
 if (num==-2) {
     strtolower(inpstr);
     if (!read_user(inpstr)) {
       write_str(user,NO_USER_STR);
       return;
       }
     strcpy(buffer,t_ustr.last_site);
     /* write_user(inpstr); */
     strtolower(buffer);
     }

/* Check for hostname search string */
 else if (num==-1) {
     netname=1;
     strcpy(buffer,inpstr);
     strtolower(buffer);
     }

/* Check for IP search string */
 else if (num==-3) {
     strcpy(buffer,inpstr);
     strtolower(buffer);
     }

 else {
     write_str(user,"Invalid search string or user.");
     return;
     }

 sprintf(t_mess,"%s",USERDIR);
 strncpy(filerid,t_mess,FILE_NAME_LEN);
 
 num=0;
 dirp=opendir((char *)filerid);
  
 if (dirp == NULL)
   {write_str(user,"Directory information not found.");
    write_log(ERRLOG,YESTIME,"Directory information not found for directory \"%s\" in same_site %s\n",
    filerid,get_error());
    return;
   }

   strcpy(filename,get_temp_file());
   if (!(fp=fopen(filename,"w"))) {
     write_str(user,BAD_FILEIO);
     write_log(ERRLOG,YESTIME,"Couldn't open tempfile(w) in same_site! %s\n",get_error());
     (void) closedir(dirp);
     return;
     }
   
 while ((dp = readdir(dirp)) != NULL) 
   { 
    sprintf(small_buff,"%s",dp->d_name);
    if (small_buff[0]=='.') continue;
    read_user(small_buff);     /* Read user's profile */
      if (!strcmp(t_ustr.last_site,"127.0.0.1")) {
          t_ustr.last_site[0]=0; continue; }
      if (!netname) {
       strcpy(buffer2,t_ustr.last_site);
       if (strstr(buffer2,buffer))  /* Search for string in user's last site */
        {
            sprintf(mess,"%-*s from %14s, %s ago",NAME_LEN+count_color(t_ustr.say_name,0),t_ustr.say_name,t_ustr.last_site,converttime((long)((time(0)-t_ustr.rawtime)/60)) );
            fputs(mess,fp);
            fputs("\n",fp);
            t_ustr.last_site[0]=0;
            num++;
        } 
       t_ustr.last_site[0]=0;
       buffer2[0]=0;
      }
     else {
       strcpy(buffer2,t_ustr.last_name);
       strtolower(buffer2);
       if (strstr(buffer2,buffer))  /* Search for string in user's last hostname */
        {
            sprintf(mess,"%-*s from %s\n\r%s ago\n\r",NAME_LEN+count_color(t_ustr.say_name,0),t_ustr.say_name,t_ustr.last_name,converttime((long)((time(0)-t_ustr.rawtime)/60)) );
            fputs(mess,fp);
            fputs("\n",fp);
            t_ustr.last_name[0]=0;
            num++;
        } 
       t_ustr.last_name[0]=0;
       buffer2[0]=0;
      }
   }       /* End of while */
 
 t_ustr.last_site[0]=0;
 t_ustr.last_name[0]=0;
 buffer2[0]=0;
 netname=0;
 fputs("\n",fp);
 sprintf(mess,"Displayed %d user%s",num,num == 1 ? "" : "s");
 fputs(mess,fp);
 fputs("\n",fp);

 fclose(fp);
 (void) closedir(dirp);

 if (!cat(filename,user,0)) {
     write_str(user,BAD_FILEIO);
     write_log(ERRLOG,YESTIME,"Couldn't cat file \"%s\" in same_site! %s",filename,get_error());
    }

 return;
}

/*---------------------------------------------------------------------*/
/* Gags user from .telling, .semote, .sthink, .ptell .beep, and .smail */
/*---------------------------------------------------------------------*/
void gag(int user, char *inpstr)
{
int i=0,found=0,u;
char other_user[ARR_SIZE];
char check[50];

if (!strlen(inpstr)) {
    write_str(user,"Users that you have gagged");
    write_str(user,"+-----------------------------------------------------------+");
     for (i=0;i<MAX_GAG;++i) {
       if (strlen(ustr[user].gagged[i])) {
             strcpy(check,ustr[user].gagged[i]);
             strtolower(check);

            if ((u=get_user_num_exact(check,user))==-1) {
             if (!read_user(check)) {
              sprintf(mess,"%-*s doesn't exist yet",NAME_LEN,ustr[user].gagged[i]);
              write_str(user,mess);
              found=1; continue;
              }
             sprintf(mess,"%-*s last on %s ago",NAME_LEN,strip_color(t_ustr.say_name),converttime((long)((time(0)-t_ustr.rawtime)/60)));
             write_str(user,mess);
             t_ustr.rawtime=0;
             }
             else {
		if (!ustr[u].vis) {
		  if (ustr[user].tempsuper >= MIN_HIDE_LEVEL)
                   sprintf(mess,"^HY%-*s^ is online right NOW!",NAME_LEN,ustr[user].gagged[i]);
		  else {
                    sprintf(mess,"%-*s last on %s ago",NAME_LEN,ustr[user].gagged[i],converttime((long)((time(0)-ustr[u].rawtime)/60)));
		   }
		  } /* end of vis if */
		else {
                 sprintf(mess,"^HY%-*s^ is online right NOW!",NAME_LEN,ustr[user].gagged[i]);
		}
              write_str(user,mess);
             }
         found=1;
        } /* end of if strlen */
      } /* end of for */
     i=0;
    if (!found) write_str(user,"You have noone gagged.");
    write_str(user,"+-----------------------------------------------------------+");
    return;
    }

if (strlen(inpstr) > NAME_LEN) {
   write_str(user,"Name too long. Sorry.");
   return;
   }

/* plug security hole */
if (check_fname(inpstr,user)) 
  {
   write_str(user,"Illegal name.");
   return;
  }

sscanf(inpstr,"%s ",other_user);
strtolower(other_user);

if (!strcmp(ustr[user].name,other_user)) {
    write_str(user,"Why would you want to add yourself?!");
    return;
    }

      for (i=0; i<MAX_GAG; ++i) {
        strcpy(check,ustr[user].gagged[i]);
        strtolower(check);
        if (!strcmp(other_user,check)) { 
            found=1;
            break;
           }
        check[0]=0;
       } /* end of for */

/* If not found, add user to list */
if (!found) {
       if (ustr[user].gag_num < MAX_GAG) {
        if (!read_user(other_user)) {
            write_str(user,NO_USER_STR);
            return;
            }
       if ((t_ustr.super >= WIZ_LEVEL) && (ustr[user].tempsuper < t_ustr.super)) {
	   if (strcmp(t_ustr.name,BOT_ID)) {
           write_str(user,"You can't gag users of that level.");
           return;
	   }
          }
        /* User exists, so add to a free slot */
        i=0;
        for (i=0; i<MAX_GAG; ++i) {
           if (!strlen(ustr[user].gagged[i])) break;
          }
        strcpy(ustr[user].gagged[i],strip_color(t_ustr.say_name));
        write_str(user,"User gagged.");
        ustr[user].gag_num++;
        }
       else {
        write_str(user,"You can't add any more users to your list.");
        }
   }
else {
        write_str(user,"User UN-gagged.");
        ustr[user].gagged[i][0]=0;
        ustr[user].gag_num--;
     }

copy_from_user(user);
write_user(ustr[user].name);
}


/*---------------------------------------------------------------------*/
/* Alert you with beep when specified user logs into the talker        */
/*---------------------------------------------------------------------*/
void alert(int user, char *inpstr)
{
int i=0,found=0,u;
char other_user[ARR_SIZE];
char check[50];

if (!strlen(inpstr)) {
    write_str(user,"Users that you are being alerted of");
    write_str(user,"+-----------------------------------------------------------+");
     for (i=0;i<MAX_ALERT;++i) {
       if (strlen(ustr[user].friends[i])) {
             strcpy(check,ustr[user].friends[i]);
             strtolower(check);

            if ((u=get_user_num_exact(check,user))==-1) {
             if (!read_user(check)) {
              sprintf(mess,"%-*s doesn't exist yet",NAME_LEN,ustr[user].friends[i]);
              write_str(user,mess);
              found=1; continue;
              }
             sprintf(mess,"%-*s last on %s ago",NAME_LEN,strip_color(t_ustr.say_name),converttime((long)((time(0)-t_ustr.rawtime)/60)));
             write_str(user,mess);
             t_ustr.rawtime=0;
             }
             else {
		if (!ustr[u].vis) {
		  if (ustr[user].tempsuper >= MIN_HIDE_LEVEL)
                   sprintf(mess,"^HY%-*s^ is online right NOW! (*)",NAME_LEN,ustr[user].friends[i]);
		  else {
                    sprintf(mess,"%-*s last on %s ago",NAME_LEN,ustr[user].friends[i],converttime((long)((time(0)-ustr[u].rawtime)/60)));
		   }
		  } /* end of vis if */
		else {
                 sprintf(mess,"^HY%-*s^ is online right NOW!",NAME_LEN,ustr[user].friends[i]);
		}
              write_str(user,mess);
             }
         found=1;
        } /* end of if strlen */
      } /* end of for */
     i=0;
    if (!found) write_str(user,"You are not being alerted of anyone");
    write_str(user,"+-----------------------------------------------------------+");
    check[0]=0;
    return;
    }

if (strlen(inpstr) > NAME_LEN) {
   write_str(user,"Name too long. Sorry.");
   return;
   }

/* plug security hole */
if (check_fname(inpstr,user)) 
  {
   write_str(user,"Illegal name.");
   return;
  }

sscanf(inpstr,"%s ",other_user);
strtolower(other_user);

if (!strcmp(ustr[user].name,other_user)) {
    write_str(user,"Why would you want to add yourself?!");
    return;
    }

      for (i=0; i<MAX_ALERT; ++i) {
        strcpy(check,ustr[user].friends[i]);
        strtolower(check);
        if (!strcmp(other_user,check)) { 
            found=1;
            break;
           }
        check[0]=0;
       } /* end of for */

/* If not found, add user to list */
if (!found) {
       if (ustr[user].friend_num < MAX_ALERT) {
        if (!read_user(other_user)) {
            write_str(user,NO_USER_STR);
            return;
            }
        /* User exists, so add to a free slot */
        i=0;
        for (i=0; i<MAX_ALERT; ++i) {
           if (!strlen(ustr[user].friends[i])) break;
          }
        strcpy(ustr[user].friends[i],strip_color(t_ustr.say_name));
        write_str(user,"User added to Alert list.");
        ustr[user].friend_num++;
        }
       else {
        write_str(user,"You can't add any more users to your list.");
        }
   }
else {
        write_str(user,"User removed from Alert list.");
        ustr[user].friends[i][0]=0;
        ustr[user].friend_num--;
     }

copy_from_user(user);
write_user(ustr[user].name);
}


/** read the monthly schedule file **/
void schedule(int user)
{
char filename[FILE_NAME_LEN];

write_str(user," ");
sprintf(filename,"%s",SCHEDFILE);
if (!cat(filename,user,0))
  write_str(user,"No schedule exists for the month!");
write_str(user," ");

}

/** sing function for music heads **/
void sing(int user, char *inpstr)
{

if (!strlen(inpstr)) {
    write_str(user,NO_SINGSTR);
    return;
    }

sprintf(t_mess,"^<singing along>^ %s",inpstr);
strcat(t_mess,"\0");
emote(user,t_mess);

}


/*--------------------------------------------------*/
/* Show users that have been gone one month or more */
/* Nuke them if  -n is specified                    */
/*--------------------------------------------------*/
void show_expire(int user, char *inpstr)
{
int i,num,u,nuke_user=0;
int found=0;
int add=0;
unsigned long diff=0;
unsigned long limit=0;
char small_buff[64],n_option[80];
char tempname[NAME_LEN+1];
char now_date[30];
char filerid[FILE_NAME_LEN];
time_t tm;
struct dirent *dp;
DIR  *dirp;

if (!strlen(inpstr)) goto SHOW;

sscanf(inpstr,"%s ",n_option);
if (!strcmp(n_option,"exempt") || !strcmp(n_option,"-e")) {
    remove_first(inpstr);
    if (!strlen(inpstr)) {
        write_str(user," Users exempted from .expire");
        write_str(user,"+-----------------------------------------------------------+");
        time(&tm);
         for (i=0;i<NUM_EXPIRES;++i) {
            if (!strcmp(expired[i],"name"))
             write_str(user,"< not in use >");
            else {
             strcpy(tempname,expired[i]);
             strtolower(tempname);
            if ((u=get_user_num_exact(tempname,user))==-1) {
             if (!read_user(tempname)) {
              sprintf(mess,"%-*s doesn't exist yet",NAME_LEN,expired[i]);
              write_str(user,mess);
              continue;
              }
             diff=tm-t_ustr.rawtime;
             sprintf(mess,"%-*s last on %s ago",NAME_LEN,strip_color(t_ustr.say_name),converttime((long)(diff/60)));
             write_str(user,mess);
             diff=0; t_ustr.rawtime=0;
             }
             else {
              sprintf(mess,"^HY%-*s^ is online right NOW!",NAME_LEN,expired[i]);
              write_str(user,mess);
             }
            } /* end of else */
           } /* end of for */
        i=0;
        write_str(user,"+-----------------------------------------------------------+");
        return;
        }
        
else if (((strlen(inpstr) < 3) && (strlen(inpstr) > 0)) || (strlen(inpstr) > NAME_LEN))
       {
        write_str(user,"Name length invalid.");
        return;
       }
else strtolower(inpstr);

          if ((remove_exem_data(inpstr))==1) {
           write_str(user,"User un-exempted.");
           return;
           }

 i=0;

     for (i=0;i<NUM_EXPIRES;++i) {
      if (!strcmp(expired[i],"name")) {
        if (!read_user(inpstr)) {
          write_str(user,NO_USER_STR);
          return;
         }
        strcpy(expired[i],strip_color(t_ustr.say_name));
        write_str(user,"User exempted.");
        write_exem_data();
        i=0;
        return;
        }
      }

  i=0;

  write_str(user,"Max users are exempted. Un-exempt someone first.");
  return;
      
 }

else if (!strcmp(n_option,"-n")) {
   nuke_user=1;
   remove_first(inpstr);

   if (!strlen(inpstr)) goto SHOW;

    if (((strlen(inpstr) < 3) && (strlen(inpstr) > 0)) || (strlen(inpstr) > NAME_LEN)) 
       {
        write_str(user,"Name length invalid.");
        return;
       }
   strtolower(inpstr);
   }

 else {
      write_str(user,"Option not understood.");
      return;
      }

 SHOW:
 sprintf(t_mess,"%s",USERDIR);
 strncpy(filerid,t_mess,FILE_NAME_LEN);
 
 num=0;
 dirp=opendir((char *)filerid);
  
 if (dirp == NULL)
   {write_str(user,"Directory information not found.");
    return;
    write_log(ERRLOG,YESTIME,"Directory information not found for directory \"%s\" in show_expire %s\n",
    filerid,get_error());
   }
    
  time(&tm);       
  strcpy(now_date,ctime(&tm));
  sprintf(t_mess,"Today is  %s",now_date);
  write_str(user,t_mess);

 while ((dp = readdir(dirp)) != NULL) 
   { 
    sprintf(small_buff,"%s",dp->d_name);
     if (!strcmp(small_buff,inpstr)) {
        write_str(user,"Excepted user found.. skipping.");
        continue;
        }       
     for (i=0;i<NUM_EXPIRES;++i) {
        strcpy(tempname,expired[i]);
        strtolower(tempname);
        if (!strcmp(small_buff,tempname)) {
            write_str(user,"Excepted user found.. skipping.");
            found=1; break;
            }
       }

    if (found) { found=0; continue; }

    if (small_buff[0]=='.') continue;
    read_user(small_buff);     /* Read user's profile */
      if (!strcmp(t_ustr.last_site,"127.0.0.1")) {
          t_ustr.last_site[0]=0;
          continue;
          }
    /* Wizards get a month extra before expiration, if set */
    if (EXPIRE_EXEMPT > -1) {
      if (t_ustr.super >= EXPIRE_EXEMPT) add=30;
      else add=0;
      }

 /* difference between user last login and now AND over time if goes here */
    diff  = tm-t_ustr.rawtime;
    limit = 3600*24*(DAYS_OFF+add);

    if (diff >= limit)
        {
           if (nuke_user!=1) {
            sprintf(mess,"%-*s on %s ago from %s",NAME_LEN,strip_color(t_ustr.say_name),converttime((long)(diff/60)),t_ustr.last_site);
            write_str(user,mess);
            num++;
            }
            t_ustr.last_site[0]=0;
            t_ustr.rawtime=0;
             if (nuke_user==1) {
                nuke(user,small_buff,1);
                num++;
                system_stats.tot_expired++;
                }
        } 
   t_ustr.last_site[0]=0;
   t_ustr.rawtime=0;
   }       /* End of while */
 
 t_ustr.last_site[0]=0;
 t_ustr.rawtime=0;
 if (num>0) write_str(user,"");
 if (nuke_user==1) {
  sprintf(mess,"did a user expire, %d user%s %s nuked.",num,num == 1 ? "" : "s", num == 1 ? "was" : "were");
  btell(user,mess);
  write_log(SYSTEMLOG,YESTIME,"EXPIRE: User MANUAL-EXPIRE by %s: %d user%s %s nuked\n",ustr[user].say_name,num,num == 1 ? "" : "s", num == 1 ? "was" : "were");
  sprintf(mess,"Nuked %d user%s",num,num == 1 ? "" : "s");
  }
 else {
  sprintf(mess,"Displayed %d user%s",num,num == 1 ? "" : "s");
  }
 write_str(user,mess);
 nuke_user=0;
 i=0;

 (void) closedir(dirp);

}


/* Auto expire users at midnight */
void auto_expire(void)
{
int i=0,num=0,warns=0;
int found=0;
int add=0;
unsigned long diff=0;
unsigned long limit=0;
unsigned long to_go=0;
char small_buff[64];
char tempname[NAME_LEN+1];
char filerid[FILE_NAME_LEN];
time_t tm;
struct dirent *dp;
DIR  *dirp;

/* check to see if we need to do anything */
if (autoexpire==0) return;

 sprintf(t_mess,"%s",USERDIR);
 strncpy(filerid,t_mess,FILE_NAME_LEN);
 
 num=0;
 dirp=opendir((char *)filerid);
  
 if (dirp == NULL) return;

 time(&tm);       

 while ((dp = readdir(dirp)) != NULL) 
   { 
    sprintf(small_buff,"%s",dp->d_name);
    for (i=0;i<NUM_EXPIRES;++i) {
        strcpy(tempname,expired[i]);
        strtolower(tempname);
        if (!strcmp(small_buff,tempname)) {
            found=1; break;
            }
       }

    if (found) { found=0; continue; }

    if (small_buff[0]=='.') continue;
    read_user(small_buff);     /* Read user's profile */
      if (!strcmp(t_ustr.last_site,"127.0.0.1")) {
          t_ustr.last_site[0]=0;
          continue;
          }
 /* Wizards get a month extra before expiration, if set */
    if (EXPIRE_EXEMPT > -1) {
      if (t_ustr.super >= EXPIRE_EXEMPT) add=30;
      else add=0;
     }

 /* difference between user last login and now AND over time if goes here */
    diff  = tm-t_ustr.rawtime;
    limit = 3600*24*(DAYS_OFF+add);

    if ((diff >= limit) && ((autoexpire==2) || (autoexpire==3)))
        {
	t_ustr.last_site[0]=0;
	t_ustr.rawtime=0;
	remove_exem_data(small_buff);
	remove_user(small_buff);
	write_log(SYSTEMLOG,YESTIME,"EXPIRE: AUTO-EXPIRE-NUKE of %s\n",small_buff);   
	system_stats.tot_expired++;
	num++;
        }  /* end of if */
    else {
	if ((TIME_TO_GO > 0) && ((autoexpire==1) || (autoexpire==2))) {
		/* user not at limit, but if we have a TIME_TO_GO set */
		/* mail the user to tell them they have TIME_TO_GO to */
		/* logon before their account gets nuked	      */
	    to_go = limit - diff; /* seconds left til nuking */
/*	    if (to_go <= (TIME_TO_GO*86400)) {  MAIL EVERY DAY CODE */
if ((to_go <= (TIME_TO_GO*86400)) && (to_go >= ((TIME_TO_GO-1)*86400))) {
		/* user is over limit, if they have an email addy set */
		/* mail them about it, but only ONCE */
	    if ((strlen(t_ustr.email_addr) < 8) || !strcmp(t_ustr.email_addr,DEF_EMAIL)) {
		  t_ustr.last_site[0]=0;
		  t_ustr.rawtime=0;
		  continue;
	    } /* email not set correctly or at all */
	    else {
		/* send external email */
		if (send_ext_mail(-2, -1, 3, NULL, NUKEWARN, DATA_IS_FILE, NULL)==-1) {
		  write_log(ERRLOG,YESTIME,"EXPIRE: Couldn't send external email in auto_expire!\n");
		  t_ustr.last_site[0]=0;
		  t_ustr.rawtime=0;
		  continue;
		}

		write_log(SYSTEMLOG,YESTIME,"EXPIRE: Sent out nukewarn to %s at %s\n",t_ustr.say_name,t_ustr.email_addr);
		warns++;
	    } /* end of else email address ok */
	    } /* end of if over limit */
	  } /* end of main if */
	} /* end of else */
   t_ustr.last_site[0]=0;
   t_ustr.rawtime=0;
   }       /* End of while */
 
 t_ustr.last_site[0]=0;
 t_ustr.rawtime=0;

  sprintf(mess,"%s Talker user expire: %d user%s %s nuked, %d %s warned",STAFF_PREFIX,num,num == 1 ? "" : "s", num == 1 ? "was" : "were", warns, warns == 1 ? "was" : "were");
  writeall_str(mess, WIZ_ONLY, -1, 0, -1, BOLD, NONE, 0);
  write_log(SYSTEMLOG,YESTIME,"EXPIRE: Talker AUTO-EXPIRE: %d user%s %s nuked, %d %s warned\n",num,num == 1 ? "" : "s", num == 1 ? "was" : "were", warns, warns == 1 ? "was" : "were");
 num=0;
 warns=0;
 i=0;
}

/* Ban a name from logging in or being created */
void banname(int user, char *inpstr)
{
int i=0;
char tempname[NAME_LEN+1];

if (!strlen(inpstr)) {
        write_str(user," Names banned from use");
        write_str(user,"+----------------------------+");
         for (i=0;i<NUM_NAMEBANS;++i) {
            if (!strcmp(nbanned[i],"name"))
             write_str(user,"< not in use >");
            else 
             write_str(user,nbanned[i]);
           }
        i=0;
        write_str(user,"+----------------------------+");
        return;
        }
        
else if (((strlen(inpstr) < 3) && (strlen(inpstr) > 0)) || (strlen(inpstr) > NAME_LEN))
       {
        write_str(user,"Name length invalid.");
        return;
       }
else strtolower(inpstr);

if (!strcmp(inpstr,ROOT_ID)) {
   write_str(user,"Yeah, right!");
   return;
   }

     for (i=0;i<NUM_NAMEBANS;++i) {
         strcpy(tempname,nbanned[i]);
         strtolower(tempname);
       if (!strcmp(tempname,inpstr)) {
          strcpy(nbanned[i],"name");
          write_str(user,"Name un-banned.");
          write_nban_data();
          i=0;
          return;
          }
       }

 i=0;

     for (i=0;i<NUM_NAMEBANS;++i) {
      if (!strcmp(nbanned[i],"name")) {
        strcpy(nbanned[i],strip_color(inpstr));
        write_str(user,"Name banned from use.");
        write_nban_data();
        i=0;
        return;
        }
      }

  i=0;

  write_str(user,"Max number of names are banned. Un-ban one first.");
  return;
      
}


/*----------------------------------------------------*/
/* Create a new player with defaults settings         */
/*----------------------------------------------------*/
void player_create(int user, char *inpstr)
{
int new_user,f=0,level=0;
char newname[ARR_SIZE];
char lowername[ARR_SIZE];
char newpass[ARR_SIZE];
char levelstr[ARR_SIZE];

if (!strlen(inpstr)) {
   write_str(user,"Syntax: .pcreate <user_name> <level> <password>");
   return;
  }

sscanf(inpstr,"%s ",newname);

 if (newname[0]<32 || strlen(newname)< 3) 
    {
     write_str(user,"Invalid name given [must be at least 3 letters].");  
     return;
    }

 if (strstr(newname,"^")) {
    write_str(user,"Name cannot have color or hilites in it.");
    return;
    }

  if (strlen(newname)>NAME_LEN-1) 
    {
     write_str(user,"Name too long");  
     return;
    }

	/* see if only letters in login */
     for (f=0; f<strlen(newname); ++f) 
       {
         if (!isalpha((int)newname[f]) || newname[f]<'A' || newname[f] >'z') 
           {
	     write_str(user,"Name can only contain letters.");
	     return;
	   }
       }
        

  /*----------------------------------------------------------------*/
  /* copy capitalized name to a temp array and convert to lowercase */
  /*----------------------------------------------------------------*/
  
  strcpy(lowername,newname);
  strtolower(lowername);
 
if (check_for_user(lowername))
    {
      write_str(user,"Sorry, that name is already used.");
      return;
    }

/* Check for level input string */
remove_first(inpstr);

if (!strlen(inpstr)) {
  write_str(user,"Missing required field(s).");
   write_str(user,"Syntax: .pcreate <user_name> <level> [<password>]");
  return;
  }

sscanf(inpstr,"%s ",levelstr);

if (strlen(levelstr) > 1) {
  write_str(user,"Level number length cannot be greater than 1");
  return;
  }

if (!isdigit((int)levelstr[0])) {
  write_str(user,"Level given is not a number.");
  return;
  }
level=atoi(levelstr);

if (level > MAX_LEVEL) {
  write_str(user,"Level does not exist.");
  return;
  }

if (PROMOTE_TO_ABOVE)
  {
  }
else
  {
    if ((level > ustr[user].tempsuper) && strcmp(ustr[user].name,ROOT_ID)) {
     write_str(user,"You can't make a player with a level that high.");
     return;
    }
  }

if (PROMOTE_TO_SAME)
  {
    if ((level > ustr[user].tempsuper) && strcmp(ustr[user].name,ROOT_ID)) {
     write_str(user,"You can't make a player with a level that high.");
     return;
    }
  }
else
  {
    if ((level >= ustr[user].tempsuper) && strcmp(ustr[user].name,ROOT_ID)) {
     write_str(user,"You can't make a player with a level that high.");
     return;
    }
  }


/* Check for password */
remove_first(inpstr);

sscanf(inpstr,"%s",newpass);

 if (newpass[0]<32 || strlen(newpass)< 3) 
    {
     write_str(user,"Invalid password given [must be at least 3 letters].");  
     return;
    }

 if (strstr(newpass,"^")) {
    write_str(user,"Password cannot have color or hilites in it.");
    return;
    }
        
  if (strlen(newpass)>NAME_LEN-1) 
    {
     write_str(user,"Password too long");  
     return;
    }

  /*-------------------------------------------------------------*/
  /* convert password to lowercase and encrypt the password      */
  /*-------------------------------------------------------------*/
  
  strtolower(newpass);  
 
 if (!strcmp(lowername, newpass))
    {
        write_str(user,"Password cannot be the login name."); 
        return;  
    }
  
  st_crypt(newpass);                                   

  if ( (new_user = find_free_slot('1') ) == -1 )
    {
     write_str(user,"Could not get free user slot to write user details! Aborted!");
     write_log(ERRLOG,YESTIME,"PCREATE: Could not get free user slot to write user details for \"%s\"\n",newname);
     return;
    }

/* We got free slot, let's reset the struct for this slot */
reset_user_struct(new_user,1);

/* Now to be safe set logging_in to 1 so this slot can't */
/* taken from us while we're using it			 */
ustr[new_user].logging_in=1;

/* Get init_user to set default stuff for us */
init_user(new_user);

/* Set some stuff that it doesn't set for our situation */

   strcpy(ustr[new_user].name,         lowername);
   strcpy(ustr[new_user].say_name,     newname);
   strcpy(ustr[new_user].password,     newpass);   

   strcpy(ustr[new_user].desc,         "was just created");
   strcpy(ustr[new_user].init_site,    "127.0.0.1");
   strcpy(ustr[new_user].last_site,    "127.0.0.1");
   strcpy(ustr[new_user].last_name,    "localhost");
   strcpy(ustr[new_user].init_netname, "localhost");

  if (level > 0) {
   ustr[new_user].promote   = 1;
   ustr[new_user].super     = level;
  }

/* Ok, we're done, copy over to temp struct */
copy_from_user(new_user);

/* Write out data to new file */
write_user(lowername);

write_str(user,"User created.");

/* update staff list file */
if (ustr[new_user].super >= WIZ_LEVEL) do_stafflist();

/* Reset user structure */
reset_user_struct(new_user,1);

}


/*----------------------------------------*/
/* REVOKE command from user, rank, or all */
/*----------------------------------------*/
void revoke_com(int user, char *inpstr)
{
int i=0,found=0,num=0,a=0,u,type=0;
int was_granted=-1;
char name[SAYNAME_LEN+1];
char command[81];
char other_user[81];
char small_buff[64];
char filerid[FILE_NAME_LEN];
struct dirent *dp;
DIR  *dirp;

inpstr[80]=0;

sscanf(inpstr,"%s ",command);

if (!strlen(inpstr)) {
  write_str(user,"No command specified!");
  return;
  }

strtolower(command);

if (!strcmp(command,"-r")) {
if (strcmp(ustr[user].name,ROOT_ID)) return;

  sprintf(t_mess,"%s",USERDIR);

  strncpy(filerid,t_mess,FILE_NAME_LEN);
 
  dirp=opendir((char *)filerid);
  
 if (dirp == NULL)
   {write_str(user,"Directory information not found.");
    a=0;
    return;
   }

 while ((dp = readdir(dirp)) != NULL) 
   { 
    sprintf(small_buff,"%s",dp->d_name);
       if (small_buff[0]=='.') continue;

    	a=0;
	read_user(small_buff);

        	for (a=0; a < MAX_GRAVOKES; ++a) {
		  t_ustr.revokes[a][0]=0;
          	} /* end of for */
		  t_ustr.revokes_num=0;
		  write_user(t_ustr.name);

	 small_buff[0]=0; continue;
    } /* end of directory while */
	(void) closedir(dirp);

a=0;
write_str(user,"Reset.");
return;
} /* end of if root option -r */

/* Check for list option and list commands user cant use */
if (!strcmp(command,"-l")) {
   remove_first(inpstr);
   if (!strlen(inpstr)) {
	write_str(user,"Whose revoke list do you want to see?");
	return;
	}
   if (!strcmp(inpstr,"all")) {
	listall_gravokes(user,0);
	return;
	}

   sscanf(inpstr,"%s",other_user);

	   if (!read_user(other_user)) {
		write_str(user,NO_USER_STR);
		return;
		}

    write_str(user,"Commands that this user can't use");
    write_str(user,"+---------------------------------+");

        	for (a=0; a < MAX_GRAVOKES; ++a) {
		if (!is_revoke(t_ustr.revokes[a])) continue;

		  	found=1;
			for (i=0; sys[i].jump_vector != -1 ;++i) {
			if (strip_com(t_ustr.revokes[a]) == sys[i].jump_vector) break;
			}
		  write_str(user,sys[i].command);
		  i=0;
          	}
	if (!found) {
	  write_str(user,"None.");
	  }

    write_str(user,"+----------------------------------+");

   i=0; found=0; a=0;
   return;
  } /* end of list if */

/* Check if command really exists */
for (i=0; sys[i].jump_vector != -1 ;++i) {
   if (!strcmp(sys[i].command,command)) {
     found=1;
     break;
     } /* end of if */
   } /* end of for */

if (!found) {
  write_str(user,"Command does not exist!");
  found=0; i=0;
  return;
  } /* end of if */

found=0;
remove_first(inpstr);

if (!strlen(inpstr)) {
  write_str(user,"Who do you want to take it away from?");
  return;
  }

sscanf(inpstr,"%s",other_user);
strtolower(other_user);

/* Types */
/* -2        , all users */
/* -1        , by user   */
/* 0 or above, by rank   */

if (!strcmp(other_user,"all")) {
  type=-2;
  }
else {
  for (a=0;a<strlen(other_user);++a) {
     if (!isdigit((int)other_user[a])) { found=1; break; }
     }
  if (!found) {
     /* we're a true number, but are we a existing level? */
     other_user[4]=0; /* strip to 4 digit number to prevent buffer */
                      /* overrun to int max size */
     num=atoi(other_user);
     if (num > MAX_LEVEL) {
       write_str(user,"That level doesn't exist!");
       return;
       }
     if (num == ustr[user].tempsuper && GRANT_TO_SAME == FALSE && strcmp(ustr[user].name,ROOT_ID)) {
	  write_str(user,"You dont have that much power!");
	  found=0; a=0; num=0; i=0;
	  return;
	  }
     if (num > ustr[user].tempsuper) {
	  write_str(user,"You dont have that much power!");
	  found=0; a=0; num=0; i=0;
	  return;
	  }
	type=num;
    } /* end of sub-if */
  else {
	type=-1;
    } /* end of sub-else */
  } /* end of main type else */

if (type==-1) {

	/* we're either a alpha-numeric mix or all alpha */
	/* either way, check as a username               */
	   if (!read_user(other_user)) {
		write_str(user,NO_USER_STR);
		return;
		}

	if (t_ustr.super == ustr[user].tempsuper && GRANT_TO_SAME == FALSE && strcmp(ustr[user].name,ROOT_ID)) {
	  write_str(user,"You dont have that much power!");
	  found=0; a=0; num=0; i=0;
	  return;
	  }
	if (t_ustr.super > ustr[user].tempsuper) {
	  write_str(user,"You dont have that much power!");
	  found=0; a=0; num=0; i=0;
	  return;
	  }

	/* Check to see if user has had this command revoked */
	/* if so, nothing changes */
        	for (a=0; a < MAX_GRAVOKES; ++a) {
		if (!is_revoke(t_ustr.revokes[a])) continue;
           	if (strip_com(t_ustr.revokes[a]) == sys[i].jump_vector) {
		  write_str(user,"That user already has that command revoked!");
		  found=0; a=0; num=0; i=0;
		  return;
		  }
		}
	 	a=0;


	/* Check to see if user was granted the command    */
	/* if so take the grant away by blanking the entry */
        	for (a=0; a < MAX_GRAVOKES; ++a) {
		if (!is_grant(t_ustr.revokes[a])) continue;
           	if (strip_com(t_ustr.revokes[a]) == sys[i].jump_vector) {
		  was_granted=a;
		  break;
		  }
		}


	/* Check to see if user has that command to begin with */
	/* if it wasn't granted                                */
	if (was_granted==-1) {
	 if (t_ustr.super < sys[i].su_com) {
	   sprintf(mess,"%s doesn't even have that command!",t_ustr.say_name);
	   write_str(user,mess);
	   found=0; a=0; num=0; i=0;
	   return;
	   }
	}

if (was_granted!=-1) {
/* only clear the grant entry to give normal permissions back */
t_ustr.revokes[a][0]=0;
t_ustr.revokes_num--;
	   strcpy(name,t_ustr.say_name);
	   write_user(other_user);
	   sprintf(mess,"%s has taken the %s command BACK from you",ustr[user].say_name,sys[i].command);
	   if ((u=get_user_num_exact(other_user,user)) != -1) {
		ustr[u].revokes_num--;
		ustr[u].revokes[a][0] = 0;
		write_str(u,mess);
	     } /* end of if user online */
		else {
			sprintf(t_mess,"%s %s",other_user,mess);
			send_mail(user,t_mess,1);
		   }
		sprintf(mess,"You took the %s command BACK from %s",sys[i].command,name);
		write_str(user,mess);
			sprintf(t_mess,"%s %s",ustr[user].name,mess);
			send_mail(user,t_mess,1);
			goto END;
} /* end of if was_granted */

	if (t_ustr.revokes_num < MAX_GRAVOKES) {
	        /* Add to a free slot */
        	for (a=0; a < MAX_GRAVOKES; ++a) {
           	if (!strlen(t_ustr.revokes[a])) break;
          	}
	   t_ustr.revokes_num++;
	   sprintf(t_ustr.revokes[a],"%d -",sys[i].jump_vector);
	   strcpy(name,t_ustr.say_name);
	   write_user(other_user);
	   sprintf(mess,"%s has taken the %s command from you",ustr[user].say_name,sys[i].command);
	   if ((u=get_user_num_exact(other_user,user)) != -1) {
		ustr[u].revokes_num++;
		sprintf(ustr[u].revokes[a],"%d -",sys[i].jump_vector);
		write_str(u,mess);
	     } /* end of if user online */
		else {
			sprintf(t_mess,"%s %s",other_user,mess);
			send_mail(user,t_mess,1);
		   }
		sprintf(mess,"You took the %s command from %s",sys[i].command,name);
		write_str(user,mess);
			sprintf(t_mess,"%s %s",ustr[user].name,mess);
			send_mail(user,t_mess,1);
	  } /* end of num revokes if */
	  else {
	  sprintf(mess,"You can't revoke any more commands from %s until you revoke one that was granted",t_ustr.say_name);
	  write_str(user,mess);
	  found=0; a=0; num=0; i=0;
	  return;
	  }
END:
	/* wiztell here */
	if (was_granted!=-1)
	sprintf(mess,"has taken the %s command BACK from %s",sys[i].command,name);
	else
	sprintf(mess,"has taken the %s command away from %s",sys[i].command,name);
	btell(user,mess);
	found=0; a=0; num=0; i=0;
	return;
  } /* end of type -1 */
else if (type==-2) {
	/* ALL USERS */
  sprintf(t_mess,"%s",USERDIR);

  strncpy(filerid,t_mess,FILE_NAME_LEN);
 
  dirp=opendir((char *)filerid);
  
 if (dirp == NULL)
   {write_str(user,"Directory information not found.");
    found=0; a=0; num=0; i=0;
    return;
   }

 while ((dp = readdir(dirp)) != NULL) 
   { 
    sprintf(small_buff,"%s",dp->d_name);
       if (small_buff[0]=='.') continue;

    	found=0; a=0; num=0; was_granted=-1;
	read_user(small_buff);

	/* Check to see if we want to clear it or take it */
	
	/* cant take it from own level or above */
	if (t_ustr.super == ustr[user].tempsuper && GRANT_TO_SAME == FALSE && strcmp(ustr[user].name,ROOT_ID)) continue;
	if (t_ustr.super > ustr[user].tempsuper) continue;

	/* does the user already have this command revoked */
        	for (a=0; a < MAX_GRAVOKES; ++a) {
		if (!is_revoke(t_ustr.revokes[a])) continue;
           	if (strip_com(t_ustr.revokes[a]) == sys[i].jump_vector) {
		  found=1; break;
		  }
		}
		if (found==1) continue;
	 	a=0;

	/* Check to see if user was granted the command    */
	/* if so take the grant away by blanking the entry */
        	for (a=0; a < MAX_GRAVOKES; ++a) {
		if (!is_grant(t_ustr.revokes[a])) continue;
           	if (strip_com(t_ustr.revokes[a]) == sys[i].jump_vector) {
		  was_granted=a;
			break;
		  }
		}

	/* Check to see if user has that command to begin with */
	if (was_granted==-1) {
	 if (t_ustr.super < sys[i].su_com) {
	   sprintf(mess,"%s doesn't even have that command!",t_ustr.say_name);
	   write_str(user,mess);
	   continue;
	   }
	}

if (was_granted!=-1) {
/* only clear the grant entry to give normal permissions back */
t_ustr.revokes[a][0]=0;
t_ustr.revokes_num--;
		  write_user(t_ustr.name);
		  sprintf(mess,"%s has taken the %s command BACK from you and everyone",ustr[user].say_name,sys[i].command);
	   	if ((u=get_user_num_exact(small_buff,user)) != -1) {
			ustr[u].revokes_num--;
			ustr[u].revokes[a][0] = 0;
		  write_str(u,mess);
	     	  } /* end of if user online */		  
		else {
			sprintf(t_mess,"%s %s",small_buff,mess);
			send_mail(user,t_mess,1);
		   }
	 small_buff[0]=0; continue;
} /* end of if was_granted */
	a=0;

	if (t_ustr.revokes_num < MAX_GRAVOKES) {
	        /* Add to a free slot */
        	for (a=0; a < MAX_GRAVOKES; ++a) {
           	if (!strlen(t_ustr.revokes[a])) break;
          	}
	   t_ustr.revokes_num++;
	   sprintf(t_ustr.revokes[a],"%d -",sys[i].jump_vector);
	   write_user(t_ustr.name);
		sprintf(mess,"%s has taken the %s command from you and everyone",ustr[user].say_name,sys[i].command);
	   if ((u=get_user_num_exact(small_buff,user)) != -1) {
		ustr[u].revokes_num++;
		sprintf(ustr[u].revokes[a],"%d -",sys[i].jump_vector);
		write_str(u,mess);
	     } /* end of if user online */
		else {
			sprintf(t_mess,"%s %s",small_buff,mess);
			send_mail(user,t_mess,1);
		   }
	  } /* end of num revokes if */
	  else {
	  sprintf(mess,"You can't revoke any more commands from %s until you revoke one that was granted.",t_ustr.say_name);
	  write_str(user,mess);
	  small_buff[0]=0; continue;
	  }

    } /* end of directory while */
	(void) closedir(dirp);
	/* wiztell here */
	sprintf(mess,"You took the %s command from everyone",sys[i].command);
	write_str(user,mess);
	sprintf(t_mess,"%s %s",ustr[user].name,mess);
	send_mail(user,t_mess,0);
	sprintf(mess,"has taken the %s command away from everyone",sys[i].command);
	btell(user,mess);
  } /* end of type -2 */
else {

  /* USERS OF RANK <TYPE> */
  sprintf(t_mess,"%s",USERDIR);

  strncpy(filerid,t_mess,FILE_NAME_LEN);
 
  dirp=opendir((char *)filerid);
  
 if (dirp == NULL)
   {write_str(user,"Directory information not found.");
    found=0; a=0; num=0; i=0;
    return;
   }

 while ((dp = readdir(dirp)) != NULL) 
   { 
    sprintf(small_buff,"%s",dp->d_name);
       if (small_buff[0]=='.') continue;

    	found=0; a=0; num=0;
	read_user(small_buff);

	/* Is user of the rank we're taking or giving back to? */
	if (t_ustr.super != type) continue;

	/* cant take it from own level or above */
	if (t_ustr.super == ustr[user].tempsuper && GRANT_TO_SAME == FALSE && strcmp(ustr[user].name,ROOT_ID)) continue;
	if (t_ustr.super > ustr[user].tempsuper) continue;

	/* does the user already have this command revoked */
                for (a=0; a < MAX_GRAVOKES; ++a) {
                if (!is_revoke(t_ustr.revokes[a])) continue;
                if (strip_com(t_ustr.revokes[a]) == sys[i].jump_vector) {
                  found=1; break;
                  }
                }
                if (found==1) continue;
                a=0;

        /* Check to see if user was granted the command    */
        /* if so take the grant away by blanking the entry */
                for (a=0; a < MAX_GRAVOKES; ++a) {
                if (!is_grant(t_ustr.revokes[a])) continue;
                if (strip_com(t_ustr.revokes[a]) == sys[i].jump_vector) {
                  was_granted=a;
			break;
                  }
                }

        /* Check to see if user has that command to begin with */
        if (was_granted==-1) {
         if (t_ustr.super < sys[i].su_com) {
           sprintf(mess,"%s doesn't even have that command!",t_ustr.say_name);
           write_str(user,mess);
           continue;
           }
        }

if (was_granted!=-1) {
/* only clear the grant entry to give normal permissions back */ 
t_ustr.revokes[a][0]=0;
t_ustr.revokes_num--;
                  write_user(t_ustr.name);
		  sprintf(mess,"%s has taken the %s command BACK from you and all level %d's",ustr[user].say_name,sys[i].command,type);
	   	if ((u=get_user_num_exact(small_buff,user)) != -1) {
			ustr[u].revokes_num--;
			ustr[u].revokes[a][0] = 0;
		  write_str(u,mess);
	     	  } /* end of if user online */		  
		else {
			sprintf(t_mess,"%s %s",small_buff,mess);
			send_mail(user,t_mess,1);
		   }
	 small_buff[0]=0; continue;
} /* end of if was_granted */
	a=0;

	if (t_ustr.revokes_num < MAX_GRAVOKES) {
	        /* Add to a free slot */
        	for (a=0; a < MAX_GRAVOKES; ++a) {
           	if (!strlen(t_ustr.revokes[a])) break;
          	}
	   t_ustr.revokes_num++;
	   sprintf(t_ustr.revokes[a],"%d -",sys[i].jump_vector);
	   write_user(t_ustr.name);
		sprintf(mess,"%s has taken the %s command from you and all level %d's",ustr[user].say_name,sys[i].command,type);
	   if ((u=get_user_num_exact(small_buff,user)) != -1) {
		ustr[u].revokes_num++;
		sprintf(ustr[u].revokes[a],"%d -",sys[i].jump_vector);
		write_str(u,mess);
	     } /* end of if user online */
		else {
			sprintf(t_mess,"%s %s",small_buff,mess);
			send_mail(user,t_mess,1);
		   }
	  } /* end of num revokes if */
	  else {
	  sprintf(mess,"You can't revoke any more commands from %s until you revoke one that was granted",t_ustr.say_name);
	  write_str(user,mess);
	  small_buff[0]=0; continue;
	  }

    } /* end of directory while */
	(void) closedir(dirp);
	/* wiztell here */
	sprintf(mess,"You took the %s command from level %d's",sys[i].command,type);
	write_str(user,mess);
	sprintf(t_mess,"%s %s",ustr[user].name,mess);
	send_mail(user,t_mess,0);
	sprintf(mess,"has taken the %s command away from level %d's",sys[i].command,type);
	btell(user,mess);

  } /* end of type rank */

found=0; a=0; num=0; i=0;
}


/*----------------------------------------*/
/* GRANT command from user, rank, or all  */
/*----------------------------------------*/
void grant_com(int user, char *inpstr)
{
int i=0,found=0,num=0,a=0,u,type=0,level=0;
int was_revoked=-1;
char name[SAYNAME_LEN+1];
char opt[81];
char command[81];
char other_user[81];
char small_buff[64];
char filerid[FILE_NAME_LEN];
struct dirent *dp;
DIR  *dirp;

inpstr[80]=0;

sscanf(inpstr,"%s ",command);

if (!strlen(inpstr)) {
  write_str(user,"No command specified!");
  return;
  }

strtolower(command);

if (!strcmp(command,"-r")) {
if (strcmp(ustr[user].name,ROOT_ID)) return;

  sprintf(t_mess,"%s",USERDIR);

  strncpy(filerid,t_mess,FILE_NAME_LEN);
 
  dirp=opendir((char *)filerid);
  
 if (dirp == NULL)
   {write_str(user,"Directory information not found.");
    a=0;
    return;
   }

 while ((dp = readdir(dirp)) != NULL) 
   { 
    sprintf(small_buff,"%s",dp->d_name);
       if (small_buff[0]=='.') continue;

    	a=0;
	read_user(small_buff);

        	for (a=0; a < MAX_GRAVOKES; ++a) {
		  t_ustr.revokes[a][0]=0;
          	} /* end of for */
		  t_ustr.revokes_num=0;
		  write_user(t_ustr.name);

	 small_buff[0]=0; continue;
    } /* end of directory while */
	(void) closedir(dirp);

a=0;
write_str(user,"Reset.");
return;
} /* end of if root option -r */

/* Check for list option and list commands user cant use */
if (!strcmp(command,"-l")) {
   remove_first(inpstr);
   if (!strlen(inpstr)) {
	write_str(user,"Whose grant list do you want to see?");
	return;
	}
   if (!strcmp(inpstr,"all")) {
	listall_gravokes(user,1);
	return;
	}

   sscanf(inpstr,"%s",other_user);

	   if (!read_user(other_user)) {
		write_str(user,NO_USER_STR);
		return;
		}

    write_str(user,"Commands that this user is granted");
    write_str(user,"+----------------------------------+");

        	for (a=0; a < MAX_GRAVOKES; ++a) {
		if (!is_grant(t_ustr.revokes[a])) continue;

		  	found=1;
			for (i=0; sys[i].jump_vector != -1 ;++i) {
			if (strip_com(t_ustr.revokes[a]) == sys[i].jump_vector) break;
			}
		  write_str(user,sys[i].command);
		  i=0;
          	}
	if (!found) {
	  write_str(user,"None.");
	  }

    write_str(user,"+----------------------------------+");

   i=0; found=0; a=0;
   return;
  } /* end of list if */

/* Check if command really exists */
for (i=0; sys[i].jump_vector != -1 ;++i) {
   if (!strcmp(sys[i].command,command)) {
     found=1;
     break;
     } /* end of if */
   } /* end of for */

if (!found) {
  write_str(user,"Command does not exist!");
  found=0; i=0;
  return;
  } /* end of if */

found=0;
remove_first(inpstr);

if (sys[i].su_com == ustr[user].tempsuper && GRANT_TO_SAME == FALSE && strcmp(ustr[user].name,ROOT_ID)) {
	write_str(user,"You dont have that much power to give that command!");
	return;
  }
if (sys[i].su_com > ustr[user].tempsuper) {
	write_str(user,"You dont have that much power to give that command!");
	return;
  }

if (!strlen(inpstr)) {
  write_str(user,"Who do you want to give it to?");
  return;
  }

sscanf(inpstr,"%s",other_user);
strtolower(other_user);

/* Types */
/* -2        , all users */
/* -1        , by user   */
/* 0 or above, by rank   */

if (!strcmp(other_user,"all")) {
  remove_first(inpstr);
  if (strlen(inpstr)) {
  sscanf(inpstr,"%s",opt);
  for (a=0;a<strlen(opt);++a) {
     if (!isdigit((int)opt[a])) { found=1; break; }
     }
  if (!found) {
     /* we're a true number, but are we a existing level? */
     opt[4]=0; /* strip to 4 digit number to prevent buffer */
                      /* overrun to int max size */
     num=atoi(opt);
     if (num > MAX_LEVEL) {
       write_str(user,"That level doesn't exist!");
       return;
       }
	level=num;
      }
  else {
       write_str(user,"That level doesn't exist!");
       return;
       } /* non-numbers in level field */
   } /* end of if strlen inpstr */
	else level=sys[i].su_com;

  type=-2;
  }
else {
  for (a=0;a<strlen(other_user);++a) {
     if (!isdigit((int)other_user[a])) { found=1; break; }
     }
  if (!found) {
     /* we're a true number, but are we a existing level? */
     other_user[4]=0; /* strip to 4 digit number to prevent buffer */
                      /* overrun to int max size */
     num=atoi(other_user);
     if (num > MAX_LEVEL) {
       write_str(user,"That level doesn't exist to affect!");
       return;
       }
     if (num == ustr[user].tempsuper && GRANT_TO_SAME == FALSE && strcmp(ustr[user].name,ROOT_ID)) {
	  write_str(user,"You dont have that much power to affect that level!");
	  found=0; a=0; num=0; i=0;
	  return;
	  }
     if (num > ustr[user].tempsuper) {
	  write_str(user,"You dont have that much power to affect that level!");
	  found=0; a=0; num=0; i=0;
	  return;
	  }
	type=num;
	} /* end of sub-if */
  else {
	type=-1;
       } /* non-numbers in level field */

	num=0; found=0;

	/* now check if we can give out this much power */
  remove_first(inpstr);
  if (strlen(inpstr)) {
  sscanf(inpstr,"%s",opt);
  for (a=0;a<strlen(opt);++a) {
     if (!isdigit((int)opt[a])) { found=1; break; }
     }
  if (!found) {
     /* we're a true number, but are we a existing level? */
     opt[4]=0; /* strip to 4 digit number to prevent buffer */
                      /* overrun to int max size */
     num=atoi(opt);
     if (num > MAX_LEVEL) {
       write_str(user,"That level doesn't exist!");
       return;
       }
        level=num;
    } /* end of sub-if */
   else {
       write_str(user,"That level doesn't exist!");
       return;
       } /* end of sub-else */
    } /* end of if strlen inpstr */
    else level=sys[i].su_com;

  } /* end of main type else */

if (type==-1) {

	/* we're either a alpha-numeric mix or all alpha */
	/* either way, check as a username               */
	   if (!read_user(other_user)) {
		write_str(user,NO_USER_STR);
		return;
		}

	/* cant give from own level or above */
	if (t_ustr.super == ustr[user].tempsuper && GRANT_TO_SAME == FALSE && strcmp(ustr[user].name,ROOT_ID)) {
		write_str(user,"You dont have that much power!");
		found=0; a=0; num=0; i=0;
		return;
		}
	if (t_ustr.super > ustr[user].tempsuper) {
		write_str(user,"You dont have that much power!");
		found=0; a=0; num=0; i=0;
		return;
		}

	/* Check to see if user has had this command granted */
	/* if so, nothing changes */
                for (a=0; a < MAX_GRAVOKES; ++a) {
                if (!is_grant(t_ustr.revokes[a])) continue;
                if (strip_com(t_ustr.revokes[a]) == sys[i].jump_vector) {
                  write_str(user,"That user already has that command granted to them!");
                  found=0; a=0; num=0; i=0;
                  return;
                  }
                }
                a=0;

        /* Check to see if user was revoked the command     */
        /* if so take the revoke away by blanking the entry */
                for (a=0; a < MAX_GRAVOKES; ++a) {
                if (!is_revoke(t_ustr.revokes[a])) continue;
                if (strip_com(t_ustr.revokes[a]) == sys[i].jump_vector) {
                  was_revoked=a;
			break;
                  }
                }   

/* if command was revoked check to see if we're allowed to give it */
/* back based on our level against the commands level              */
if (was_revoked==-1) {
	/* can we give them this much access to that command */
	if (level > sys[i].su_com) {
		if (level == ustr[user].tempsuper && GRANT_TO_SAME == FALSE && strcmp(ustr[user].name,ROOT_ID)) {
		/* we cant give out access at or above our level */
		write_str(user,"You cant give that much power!");
		found=0; a=0; num=0; i=0;
		return;
		}
		if (level > ustr[user].tempsuper) {
		/* we cant give out access at or above our level */
		write_str(user,"You cant give that much power!");
		found=0; a=0; num=0; i=0;
		return;
		}
	} /* end of if level */
	else if (level < sys[i].su_com) {
	write_str(user,"You cant give out less access than the command normally gives!");
	found=0; a=0; num=0; i=0;
	return;	
	} /* end of else if level */
	else {
		if (t_ustr.super>=level) {
		/* user already has that much power, sheesh */
		write_str(user,"The user already has that much power!");
		found=0; a=0; num=0; i=0;
		return;		
		}
	} /* end of level else */
} /* end of if was_revoked */

if (was_revoked!=-1) {
/* only clear the revoke entry to give normal permissions back */
t_ustr.revokes[a][0]=0;
t_ustr.revokes_num--;
           strcpy(name,t_ustr.say_name);
           write_user(other_user);
           sprintf(mess,"%s has given the %s command BACK to you",ustr[user].say_name,sys[i].command);
           if ((u=get_user_num_exact(other_user,user)) != -1) {
                ustr[u].revokes_num--;
                ustr[u].revokes[a][0] = 0;
                write_str(u,mess);
             } /* end of if user online */
                else {
                        sprintf(t_mess,"%s %s",other_user,mess);
                        send_mail(user,t_mess,1);
                   }
                        sprintf(mess,"You gave the %s command BACK to %s",sys[i].command,name);
			write_str(user,mess);
                        sprintf(t_mess,"%s %s",ustr[user].name,mess);
                        send_mail(user,t_mess,1);
                        goto END;
} /* end of if was_revoked */

        if (t_ustr.revokes_num < MAX_GRAVOKES) {
                /* Add to a free slot */
                for (a=0; a < MAX_GRAVOKES; ++a) {
                if (!strlen(t_ustr.revokes[a])) break;
                }
           t_ustr.revokes_num++;
           sprintf(t_ustr.revokes[a],"%d + %d",sys[i].jump_vector,level);
           strcpy(name,t_ustr.say_name);
           write_user(other_user);
           sprintf(mess,"%s has given the %s command to you",ustr[user].say_name,sys[i].command);
           if ((u=get_user_num_exact(other_user,user)) != -1) {
                ustr[u].revokes_num++;
                sprintf(ustr[u].revokes[a],"%d + %d",sys[i].jump_vector,level);
                write_str(u,mess);
             } /* end of if user online */
                else {
                        sprintf(t_mess,"%s %s",other_user,mess);
                        send_mail(user,t_mess,1);
                   }
                        sprintf(mess,"You gave the %s command to %s",sys[i].command,name);
			write_str(user,mess);
                        sprintf(t_mess,"%s %s",ustr[user].name,mess);
                        send_mail(user,t_mess,1);
          } /* end of num revokes if */
          else {
          sprintf(mess,"You can't grant any more commands to %s until you grant one that was revoked",t_ustr.say_name);
          write_str(user,mess);
          found=0; a=0; num=0; i=0;
          return;
          }
END:
        /* wiztell here */
        if (was_revoked!=-1)
        sprintf(mess,"has given the %s command BACK to %s",sys[i].command,name);
        else
        sprintf(mess,";has given the %s command to %s",sys[i].command,name);
        btell(user,mess);
        found=0; a=0; num=0; i=0;
        return;
  } /* end of type -1 */
else if (type==-2) {
	/* ALL USERS */
  sprintf(t_mess,"%s",USERDIR);

  strncpy(filerid,t_mess,FILE_NAME_LEN);
 
  dirp=opendir((char *)filerid);
  
 if (dirp == NULL)
   {write_str(user,"Directory information not found.");
    found=0; a=0; num=0; i=0;
    return;
   }

 while ((dp = readdir(dirp)) != NULL) 
   { 
    sprintf(small_buff,"%s",dp->d_name);
       if (small_buff[0]=='.') continue;

    	found=0; a=0; num=0; was_revoked=-1;
	read_user(small_buff);

	/* cant give from own level or above */
	if ((t_ustr.super == ustr[user].tempsuper) && GRANT_TO_SAME == FALSE && strcmp(ustr[user].name,ROOT_ID)) continue;
	if (t_ustr.super > ustr[user].tempsuper) continue;

        /* Check to see if user has had this command granted */
        /* if so, nothing changes */
                for (a=0; a < MAX_GRAVOKES; ++a) {
                if (!is_grant(t_ustr.revokes[a])) continue; 
                if (strip_com(t_ustr.revokes[a]) == sys[i].jump_vector) {
		  found=1; break;
                  }
                }   
		if (found==1) continue;
                a=0;

        /* Check to see if user was revoked the command     */
        /* if so take the revoke away by blanking the entry */
                for (a=0; a < MAX_GRAVOKES; ++a) {
                if (!is_revoke(t_ustr.revokes[a])) continue;
                if (strip_com(t_ustr.revokes[a]) == sys[i].jump_vector) {
                  was_revoked=a;
			break;
                  }
                }

if (was_revoked==-1) {
        if (level > sys[i].su_com) {
                if (level == ustr[user].tempsuper && GRANT_TO_SAME == FALSE && strcmp(ustr[user].name,ROOT_ID)) {
                /* we cant give out access at or above our level */
		continue;
                }
                if (level > ustr[user].tempsuper) {
                /* we cant give out access at or above our level */
		continue;
                }
        } /* end of if level */
        else if (level < sys[i].su_com) {
		/* dont give out less access than the command */
		/* normally has				      */
		continue;
        } /* end of else if level */
        else {
                if (t_ustr.super==level) {
                /* user already has that much power, sheesh */
		continue;
                }
        } /* end of level else */   
} /* end of if was_revoked */

if (was_revoked!=-1) {
/* only clear the grant entry to give normal permissions back */ 
t_ustr.revokes[a][0]=0;
t_ustr.revokes_num--;
                  write_user(t_ustr.name);
                  sprintf(mess,"%s has given you and everyone BACK the %s command",ustr[user].say_name,sys[i].command);
                if ((u=get_user_num_exact(small_buff,user)) != -1) {
                        ustr[u].revokes_num--;
                        ustr[u].revokes[a][0] = 0;
                  write_str(u,mess);
                  } /* end of if user online */
                else {
                        sprintf(t_mess,"%s %s",small_buff,mess);
                        send_mail(user,t_mess,1);
                   }
         small_buff[0]=0; continue;
} /* end of if was_revoked */
        a=0;

        if (t_ustr.revokes_num < MAX_GRAVOKES) {
                /* Add to a free slot */
                for (a=0; a < MAX_GRAVOKES; ++a) {
                if (!strlen(t_ustr.revokes[a])) break;
                }
           t_ustr.revokes_num++;
           sprintf(t_ustr.revokes[a],"%d + %d",sys[i].jump_vector,level);
           write_user(t_ustr.name);
                sprintf(mess,"%s has given you and everyone the %s command",ustr[user].say_name,sys[i].command);
           if ((u=get_user_num_exact(small_buff,user)) != -1) {
                ustr[u].revokes_num++;
                sprintf(ustr[u].revokes[a],"%d + %d",sys[i].jump_vector,level);
                write_str(u,mess);
             } /* end of if user online */
                else {
                        sprintf(t_mess,"%s %s",small_buff,mess);
                        send_mail(user,t_mess,1);
                   }
          } /* end of num revokes if */
          else {
          sprintf(mess,"You can't grant any more commands to %s until you grant one that was revoked.",t_ustr.say_name);
          write_str(user,mess);
	 small_buff[0]=0; continue;
	 }

    } /* end of directory while */
	(void) closedir(dirp);
	/* wiztell here */
	sprintf(mess,"You gave the %s command back to everyone",sys[i].command);
	write_str(user,mess);
	sprintf(t_mess,"%s %s",ustr[user].name,mess);
	send_mail(user,t_mess,0);
	sprintf(mess,"has given the %s command back to everyone",sys[i].command);
	btell(user,mess);
  } /* end of type -2 */
else {

  /* USERS OF RANK <TYPE> */
  sprintf(t_mess,"%s",USERDIR);

  strncpy(filerid,t_mess,FILE_NAME_LEN);
 
  dirp=opendir((char *)filerid);
  
 if (dirp == NULL)
   {write_str(user,"Directory information not found.");
    found=0; a=0; num=0; i=0;
    return;
   }

 while ((dp = readdir(dirp)) != NULL) 
   { 
    sprintf(small_buff,"%s",dp->d_name);
       if (small_buff[0]=='.') continue;

    	found=0; a=0; num=0; was_revoked=-1;
	read_user(small_buff);

	/* Is user of the rank we're taking or giving back to? */
	if (t_ustr.super != type) continue;

        /* Check to see if user has had this command granted */
        /* if so, nothing changes */
                for (a=0; a < MAX_GRAVOKES; ++a) {
                if (!is_grant(t_ustr.revokes[a])) continue;
                if (strip_com(t_ustr.revokes[a]) == sys[i].jump_vector) {
                  found=1; break;
                  }
                }
                if (found==1) continue;
                a=0;

        /* Check to see if user was revoked the command     */
        /* if so take the revoke away by blanking the entry */
                for (a=0; a < MAX_GRAVOKES; ++a) {
                if (!is_revoke(t_ustr.revokes[a])) continue;
                if (strip_com(t_ustr.revokes[a]) == sys[i].jump_vector) {
                  was_revoked=a;
			break;
                  }
                }

if (was_revoked==-1) {
        if (level > sys[i].su_com) {
                if (level == ustr[user].tempsuper && GRANT_TO_SAME == FALSE && strcmp(ustr[user].name,ROOT_ID)) {
                /* we cant give out access at or above our level */
                continue;
                }
                if (level > ustr[user].tempsuper) {
                /* we cant give out access at or above our level */
                continue;
                }
        } /* end of if level */
        else if (level < sys[i].su_com) {
                /* dont give out less access than the command */
                /* normally has                               */
                continue;
        } /* end of else if level */
        else {
                if (t_ustr.super==level) {
                /* user already has that much power, sheesh */
                continue;
                }
        } /* end of level else */
} /* end of if was_revoked */
                
if (was_revoked!=-1) {   
/* only clear the grant entry to give normal permissions back */
t_ustr.revokes[a][0]=0;
t_ustr.revokes_num--;
                  write_user(t_ustr.name);
                  sprintf(mess,"%s has given you and all level %d's BACK the %s command",ustr[user].say_name,type,sys[i].command);
                if ((u=get_user_num_exact(small_buff,user)) != -1) {
                        ustr[u].revokes_num--;
                        ustr[u].revokes[a][0] = 0;
                  write_str(u,mess);
                  } /* end of if user online */
                else {
                        sprintf(t_mess,"%s %s",small_buff,mess);
                        send_mail(user,t_mess,1);
                   }
         small_buff[0]=0; continue;
} /* end of if was_revoked */
        a=0;
        if (t_ustr.revokes_num < MAX_GRAVOKES) {
                /* Add to a free slot */
                for (a=0; a < MAX_GRAVOKES; ++a) {
                if (!strlen(t_ustr.revokes[a])) break;
                }
           t_ustr.revokes_num++;
           sprintf(t_ustr.revokes[a],"%d + %d",sys[i].jump_vector,level);
           write_user(t_ustr.name);
                sprintf(mess,"%s has given you and all level %d's the %s command with permissions of level %d",ustr[user].say_name,type,sys[i].command,level);
           if ((u=get_user_num_exact(small_buff,user)) != -1) {
                ustr[u].revokes_num++;
                sprintf(ustr[u].revokes[a],"%d + %d",sys[i].jump_vector,level);
                write_str(u,mess);
             } /* end of if user online */
                else {
                        sprintf(t_mess,"%s %s",small_buff,mess);
                        send_mail(user,t_mess,1);
                   }
          } /* end of num revokes if */
          else {
          sprintf(mess,"You can't grant any more commands to %s until you grant one that was revoked.",t_ustr.say_name);
          write_str(user,mess);
         small_buff[0]=0; continue;
         }      

    } /* end of directory while */
        (void) closedir(dirp);
        /* wiztell here */
        sprintf(mess,"You gave the %s command back to level %d's with permissions of level %d",sys[i].command,type,level);
	write_str(user,mess);
        sprintf(t_mess,"%s %s",ustr[user].name,mess);
	send_mail(user,t_mess,0);
        sprintf(mess,"has given the %s command back to level %d's with permissions of level %d",sys[i].command,type,level);
        btell(user,mess);
  } /* end of type rank */

found=0; a=0; num=0; i=0;
}


/*----------------------------------------------------*/
/* Show to user various info specified on other users */
/*----------------------------------------------------*/
void clist(int user, char *inpstr)
{
int num;
int len=0;
int type=0;
int n_option=0,timenum,search=0;
char list[20][SAYNAME_LEN+5];
int i=0,j=0,MAXPOS=0;
long value=0,diff=0;
char day[3],month[4],year[5],datetime[10];
char small_buff[64];
char option[7];
char timebuf[23];
char comment[ARR_SIZE];
char filerid[FILE_NAME_LEN];
char filename[FILE_NAME_LEN];
char filename2[FILE_NAME_LEN];
char line[ARR_SIZE];
char line2[ARR_SIZE];
time_t tm;
time_t tm_then;
struct dirent *dp;
struct tm tmBuf;
FILE *fp;
FILE *fp2;
DIR  *dirp;

 if (!strlen(inpstr)) {
    write_str(user,"These are the topics you can search under..");
    write_str(user,"+-----------------------------------------+");
    write_hilite(user,"  rank    email    homepage    desc");
    write_hilite(user,"  bans    newbans  top");
    return;
    }

sscanf(inpstr,"%s ",option);
if (!strcmp(option,"rank")) {
    remove_first(inpstr);
    if (!strlen(inpstr)) {
        type=0;
        search=0;
        }
    else if ((strlen(inpstr)==1) && isdigit((int)inpstr[0])) {
        n_option=atoi(inpstr);
        type=0;
        search=1;
        }
    else { write_str(user,"Search string not valid.");  return; }
   }
else if (!strcmp(option,"email")) {
    remove_first(inpstr);
    if (!strlen(inpstr)) {
        type=1;
        search=0;
        }
    else if (strlen(inpstr)<=EMAIL_LENGTH) {
        type=1;
        search=1;
        }
    else { 
       sprintf(mess,"Search string too long. Max %d chars.",EMAIL_LENGTH);
       write_str(user,mess);
       return; 
       }
   }
else if (!strcmp(option,"top")) {
    remove_first(inpstr);
    if (!strlen(inpstr)) {
	write_str(user,"You can see the following highest things:");
	write_str(user," age        (oldest on the talker)");
	write_str(user," commands   (number of commands)");
	write_str(user," hangman    (hangman win %)");
	write_str(user," logavg     (login average)");
	write_str(user," logins     (most logins)");
	write_str(user," time       (cumulative time online)");
	write_str(user," ttt        (tic-tac-toe win %)");
	return;
        }
    else if (!strcmp(inpstr,"age")) search=1;
    else if (!strcmp(inpstr,"commands")) search=2;
    else if (!strcmp(inpstr,"logins")) search=3;
    else if (!strcmp(inpstr,"time")) search=4;
    else if (!strcmp(inpstr,"logavg")) search=5;
    else if (!strcmp(inpstr,"hangman")) search=6;
    else if (!strcmp(inpstr,"ttt")) search=7;
    else { 
	write_str(user,"Invalid option!");
	write_str(user,"You can see the following highest things:");
	write_str(user," age        (oldest on the talker)");
	write_str(user," commands   (number of commands)");
	write_str(user," hangman    (hangman win %)");
	write_str(user," logavg     (login average)");
	write_str(user," logins     (most logins)");
	write_str(user," time       (cumulative time online)");
	write_str(user," ttt        (tic-tac-toe win %)");
	return;
       }
	type=6;
	MAXPOS=sizeof(list)/sizeof(list[0]);
	for (i=0;i<20;++i) strcpy(list[i],"0 ");
   }
else if (!strcmp(option,"homepage")) {
    remove_first(inpstr);
    if (!strlen(inpstr)) {
        type=2;
        search=0;
        }
    else if (strlen(inpstr)<=HOME_LEN) {
        type=2;
        search=1;
        }
    else { 
       sprintf(mess,"Search string too long. Max %d chars.",HOME_LEN);
       write_str(user,mess);
       return; 
       }
   }
else if (!strcmp(option,"desc")) {
    remove_first(inpstr);
    if (!strlen(inpstr)) {
        type=3;
        search=0;
        }
    else if (strlen(inpstr)<=DESC_LEN) {
        type=3;
        search=1;
        }
    else { 
       sprintf(mess,"Search string too long. Max %d chars.",DESC_LEN);
       write_str(user,mess);
       return; 
       }
   }
else if (!strcmp(option,"bans")) {
    remove_first(inpstr);
    if (!strlen(inpstr)) {
        write_str(user,"IP or hostname search string must be given");
        return;
        }
    else if (strlen(inpstr)<=FILE_NAME_LEN) {
        type=4;
        }
    else { 
       sprintf(mess,"Search string too long. Max %d chars.",FILE_NAME_LEN);
       write_str(user,mess);
       return; 
       }

/* Check for hostname search string */
 if (isalpha((int)inpstr[strlen(inpstr)-1])) {
     }       
/* Check for IP search string */
 else if (!isalpha((int)inpstr[strlen(inpstr)-1]) && strstr(inpstr,".")) {
     }
 else {
     write_str(user,"Invalid search string.");     
     return;
     }
   }
else if (!strcmp(option,"newbans")) {
    remove_first(inpstr);
    if (!strlen(inpstr)) {
        write_str(user,"IP or hostname search string must be given");
        return;
        }
    else if (strlen(inpstr)<=FILE_NAME_LEN) {
        type=5;
        }
    else { 
       sprintf(mess,"Search string too long. Max %d chars.",FILE_NAME_LEN);
       write_str(user,mess);
       return; 
       }

/* Check for hostname search string */
 if (isalpha((int)inpstr[strlen(inpstr)-1])) {
     }
/* Check for IP search string */
 else if (!isalpha((int)inpstr[strlen(inpstr)-1]) && strstr(inpstr,".")) {
     }
 else {
     write_str(user,"Invalid search string.");     
     return;
     }
   }

else {
    write_str(user,"Unknown option");
    write_str(user,"These are the topics you can search under..");
    write_str(user,"+-----------------------------------------+");
    write_hilite(user,"  rank    email    homepage    desc");
    write_hilite(user,"  bans    newbans  top");
    return;
   }

 if (type == 4)
  sprintf(t_mess,"%s",RESTRICT_DIR);
 else if (type == 5)
  sprintf(t_mess,"%s",RESTRICT_NEW_DIR);
 else
  sprintf(t_mess,"%s",USERDIR);

 strncpy(filerid,t_mess,FILE_NAME_LEN);
 
 num=0;
 dirp=opendir((char *)filerid);
  
 if (dirp == NULL)
   {write_str(user,"Directory information not found.");
    write_log(ERRLOG,YESTIME,"Directory information not found for directory \"%s\" in clist %s\n",
    filerid,get_error());
    return;
   }

 time(&tm);

 strcpy(filename,get_temp_file());
 if (!(fp=fopen(filename,"w"))) {
     write_str(user,BAD_FILEIO);
     write_log(ERRLOG,YESTIME,"Couldn't open tempfile(w) \"%s\" in clist! %s\n",filename,get_error());
     (void) closedir(dirp);
     return;
     }

 while ((dp = readdir(dirp)) != NULL) 
   { 
    sprintf(small_buff,"%s",dp->d_name);
    if ((type==4) || (type==5)) {
           len=strlen(small_buff);
       if ((small_buff[0]=='.') ||
           ( (small_buff[len-2]=='.') &&
             ((small_buff[len-1]=='c') ||
              (small_buff[len-1]=='r')) ) ) {
         small_buff[0]=0;
         len=0;
         continue;
      }
    }
    else {
     if (small_buff[0]=='.') continue;
     read_user(small_buff);     /* Read user's profile */
    }
       if (type==0) {
         if (search) {
            if (t_ustr.super==n_option) {
             sprintf(mess,"%-*s with rank  %d",NAME_LEN+count_color(t_ustr.say_name,0),t_ustr.say_name,t_ustr.super);
             fputs(mess,fp);
             fputs("\n",fp);
             t_ustr.super=0;
             num++;
             }
            else { t_ustr.super=0; continue; }
            }
         else {
            sprintf(mess,"%-*s with rank  %d",NAME_LEN+count_color(t_ustr.say_name,0),t_ustr.say_name,t_ustr.super);
            fputs(mess,fp);
            fputs("\n",fp);
            t_ustr.super=0;
            num++;
           }
      }  /* end of type if */
    else if (type==1) {
       if (search) {
       strcpy(line2,t_ustr.email_addr);
       strtolower(line2);
       strcpy(line,inpstr);
       strtolower(line);
       if (strstr(line2,line))
        {
            sprintf(mess,"%-*s w/email  %s",NAME_LEN+count_color(t_ustr.say_name,0),t_ustr.say_name,t_ustr.email_addr);
            fputs(mess,fp);
            fputs("\n",fp);
            t_ustr.email_addr[0]=0;
            num++;
        } 
       else {
		t_ustr.email_addr[0]=0;
		line[0]=0; line2[0]=0;
		continue;
	    }
       }
      else {
            sprintf(mess,"%-*s w/email  %s",NAME_LEN+count_color(t_ustr.say_name,0),t_ustr.say_name,t_ustr.email_addr);
            fputs(mess,fp);
            fputs("\n",fp);
            t_ustr.email_addr[0]=0;
            line[0]=0; line2[0]=0;
            num++;
           } 
     } /* end of type else if */    
    else if (type==2) {
       if (search) {
       strcpy(line2,t_ustr.homepage);
       strtolower(line2);
       strcpy(line,inpstr);
       strtolower(line);
       if (strstr(line2,line))
        {
            sprintf(mess,"%-*s w/page  %s",NAME_LEN+count_color(t_ustr.say_name,0),t_ustr.say_name,t_ustr.homepage);
            fputs(mess,fp);
            fputs("\n",fp);
            t_ustr.homepage[0]=0;
            line[0]=0; line2[0]=0;
            num++;
        } 
       else {
		t_ustr.homepage[0]=0;
		line[0]=0; line2[0]=0;
		continue;
	    }
       }
      else {
            sprintf(mess,"%-*s w/page  %s",NAME_LEN+count_color(t_ustr.say_name,0),t_ustr.say_name,t_ustr.homepage);
            fputs(mess,fp);
            fputs("\n",fp);
            t_ustr.homepage[0]=0;
            line[0]=0; line2[0]=0;
            num++;
           } 
     } /* end of type else if */    
    else if (type==3) {
       if (search) {
       strcpy(line2,t_ustr.desc);
       strtolower(line2);
       strcpy(line,inpstr);
       strtolower(line);
       if (strstr(line2,line))
        {
            sprintf(mess,"%-*s w/desc  %s",NAME_LEN+count_color(t_ustr.say_name,0),t_ustr.say_name,t_ustr.desc);
            fputs(mess,fp);
            fputs("\n",fp);
            t_ustr.desc[0]=0;
            line[0]=0; line2[0]=0;
            num++;
        } 
       else { 
		t_ustr.desc[0]=0;
		line[0]=0; line2[0]=0;
		continue;
	    }
       }
      else {
            sprintf(mess,"%-*s w/desc  %s",NAME_LEN+count_color(t_ustr.say_name,0),t_ustr.say_name,t_ustr.desc);
            fputs(mess,fp);
            fputs("\n",fp);
            t_ustr.desc[0]=0;
            line[0]=0; line2[0]=0;
            num++;
           } 
     } /* end of type else if */    
    else if ((type==4) || (type==5)) {
       strcpy(line2,small_buff);
       strtolower(line2);
       strcpy(line,inpstr);
       strtolower(line);
      if (strstr(line2,line)) {
         sprintf(filename2,"%s/%s",filerid,small_buff);
         if (!(fp2=fopen(filename2,"r"))) {
            write_str(user,"Cant open file for reading!");
            continue;
            }
         fgets(timebuf,13,fp2);
         FCLOSE(fp2);
         timenum=atoi(timebuf);
         tm_then=((time_t) timenum);
         sprintf(mess,"%-35s %s ago",small_buff,converttime((long)((tm-tm_then)/60)));
         fputs(mess,fp);
         fputs("\n",fp);
         timebuf[0]=0;
         timenum=0;  
         len=0;
         num++;
         fputs("COMMENT: ",fp);
         sprintf(filename2,"%s/%s.c",filerid,small_buff);
         fp2=fopen(filename2,"r");   
         fgets(comment,ARR_SIZE,fp2);
         FCLOSE(fp2);
         fputs(comment,fp);
         fputs("\n\n",fp);
         comment[0]=0;
        }
       small_buff[0]=0;
       line[0]=0; line2[0]=0;
       len=0;
     } /* end of type else if */
    else if (type==6) {
      if (search==1) {
       /* we have to try and convert the creation time which is a */
       /* string to a time format so we can retrieve unix time    */
       /* to do calculations                                      */
	strcpy(line,t_ustr.creation);
	remove_first(line); /* day of the week */
	sscanf(line,"%s ",month);
	remove_first(line);
	sscanf(line,"%s ",day);
	remove_first(line);
	sscanf(line,"%s ",datetime);
	remove_first(line);
	strcpy(year,line);
	sprintf(line,"%s-%s-%s %s",day,month,year,datetime);
	memset(&tmBuf, 0, sizeof(struct tm));
	if (my_strptime(line, "%d-%b-%Y %H:%M:%S", &tmBuf) == NULL) {
	write_str(user,"Can't convert time! Aborting!");
	write_log(ERRLOG,YESTIME,"Can't convert time creation \"%s\" line \"%s\" for \"%s\"\n",t_ustr.creation,line,t_ustr.name);
	diff=0;
	}
	else {
	 tm_then = mktime(&tmBuf);
	 time(&tm);
	 diff=(long)(tm-tm_then);
	}
       }
else if (search==2) diff = t_ustr.numcoms;
else if (search==3) diff = (long)t_ustr.times_on;
else if (search==4) diff = t_ustr.totl;
else if (search==5) diff = t_ustr.aver;
else if (search==6) {
if (t_ustr.hang_wins==0) diff=0;
else diff = (long)(((float)((float)t_ustr.hang_wins/(float)(t_ustr.hang_wins+t_ustr.hang_losses))) * 100);
}
else if (search==7) {
if (t_ustr.ttt_kills==0) diff=0;
else diff = (long)(((float)((float)t_ustr.ttt_kills/(float)(t_ustr.ttt_kills+t_ustr.ttt_killed))) * 100);
}

	for (i=0;i<20;++i) {
        sscanf(list[i],"%ld ",&value);
	  if (diff >= value) {
		if (i < MAXPOS-1) {
        	for (j=MAXPOS-1;j>(i-1);--j) {
        	strcpy(list[j],list[j-1]);
        	}
		}

		sprintf(list[i],"%ld %s",diff,t_ustr.say_name);
		break;
	  } /* end of if greater than or equal to */
	} /* end of for */
		if (search==1) t_ustr.creation[0]=0;
		else if (search==2) t_ustr.numcoms=0;
		else if (search==3) t_ustr.times_on=0;
		else if (search==4) t_ustr.totl=0;
		else if (search==5) t_ustr.aver=0;
		else if (search==6) { t_ustr.hang_wins=0; t_ustr.hang_losses=0; }
		else if (search==7) { t_ustr.ttt_kills=0; t_ustr.ttt_killed=0; }
		line[0]=0;
		continue;
     } /* end of type else if */    
   }       /* End of while */
 
if (type==0)
 t_ustr.super=0;
else if (type==1) {
 t_ustr.email_addr[0]=0;
 line[0]=0; line2[0]=0;
 }
else if (type==2) {
 t_ustr.homepage[0]=0;
 line[0]=0; line2[0]=0;
 }
else if (type==3) {
 t_ustr.desc[0]=0;
 line[0]=0; line2[0]=0;
 }
else if (type==6) {
 t_ustr.creation[0]=0;
 line[0]=0; line2[0]=0;
 switch(search) {
 case 1: fputs("20 oldest users\n",fp); break;
 case 2: fputs("Top 20 command users\n",fp); break;
 case 3: fputs("Top 20 login users\n",fp); break;
 case 4: fputs("Top 20 cumulative time users\n",fp); break;
 case 5: fputs("Top 20 average login users\n",fp); break;
 case 6: fputs("Top 20 hangman winners (by %)\n",fp); break;
 case 7: fputs("Top 20 tic-tac-toe winners (by %)\n",fp); break;
 default: break;
 }
 for (i=0;i<20;++i) {
  sscanf(list[i],"%ld ",&value);
  if (!value) continue;
  remove_first(list[i]);
 switch(search) {
 case 1: sprintf(mess,"%-*s %s old",NAME_LEN+count_color(list[i],0),list[i],converttime((long)(value/60))); break;
 case 2: sprintf(mess,"%-*s %-8ld commands",NAME_LEN+count_color(list[i],0),list[i],value); break;
 case 3: sprintf(mess,"%-*s %-5ld logins",NAME_LEN+count_color(list[i],0),list[i],value); break;
 case 4: sprintf(mess,"%-*s %s cumul. time",NAME_LEN+count_color(list[i],0),list[i],converttime(value)); break;
 case 5: sprintf(mess,"%-*s %s average time",NAME_LEN+count_color(list[i],0),list[i],converttime(value)); break;
 case 6: sprintf(mess,"%-*s %-3ld%%",NAME_LEN+count_color(list[i],0),list[i],value); break;
 case 7: sprintf(mess,"%-*s %-3ld%%",NAME_LEN+count_color(list[i],0),list[i],value); break;
 default: break;
 }
 fputs(mess,fp);
 fputs("\n",fp);
 num++;
 } /* end of for */
}

 if ((type==4) || (type==5))
  sprintf(mess,"Displayed %d banned site%s",num,num == 1 ? "" : "s");
 else {
  fputs("\n",fp);
  sprintf(mess,"Displayed %d user%s",num,num == 1 ? "" : "s");
  }
 fputs(mess,fp);
 fputs("\n",fp);

 fclose(fp);
 (void) closedir(dirp);

 if (!cat(filename,user,0)) {
     write_str(user,BAD_FILEIO);
     write_log(ERRLOG,YESTIME,"Couldn't cat file \"%s\" in clist! %s\n",filename,get_error());
     }

 return;
}

/*-----------------------------------------------------------------*/
/*  Vote command - ALWAYS, ALWAYS do a .vote -c to start a vote    */
/*   For votes, do NOT create any files but the "votefile"         */
/*-----------------------------------------------------------------*/
void vote(int user, char *inpstr)
{
int a,b,c;
char line[ARR_SIZE];
char filename[FILE_NAME_LEN];
FILE *fp;
FILE *fp2;

/* First check to see if file exist, no matter what */
   sprintf(filename,"%s/%s",LIBDIR,"votetallies");
   if (!(fp=fopen(filename,"r"))) {
   fp=fopen(filename,"w");
   fputs("0\n0\n0\n",fp);
   }
   FCLOSE(fp);
if (!strlen(inpstr)) {
   sprintf(filename,"%s/%s",LIBDIR,"votefile");
   if (!cat(filename,user,0)) {
      write_str(user,"There is no current topic that we are voting on.");
      return;
      }
   write_str(user," ");
if (ustr[user].tempsuper >= VOTE_LEVEL) {
   write_str(user,"Current tally");
   sprintf(filename,"%s/%s",LIBDIR,"votetallies");
   if (!(fp2=fopen(filename,"r")))
     {     
      write_str(user,"Noone has voted yet.");
      return;
     }
     fscanf(fp2,"%s\n",line);
   sprintf(mess,"Choice 1 - %s votes",line);
   write_str(user,mess);
   line[0]=0;
     fscanf(fp2,"%s\n",line);
   sprintf(mess,"Choice 2 - %s votes",line);
   write_str(user,mess);
   line[0]=0;
     fscanf(fp2,"%s\n",line);
   sprintf(mess,"Choice 3 - %s votes",line);
   write_str(user,mess);
   line[0]=0;
   write_str(user," ");
   FCLOSE(fp2);
   return;
   }  /* end of tally if */
  else { return; }
 }  /* end of main if */
else if ( (!strcmp(inpstr,"1")) || (!strcmp(inpstr,"2"))
           || (!strcmp(inpstr,"3")) ) {
   sprintf(filename,"%s/%s",LIBDIR,"voteusers");
   if (!(fp=fopen(filename,"r"))) {
     goto ADD;
     }

   while (!feof(fp)) {
     fscanf(fp,"%s\n",line);
     if (!strcmp(line,ustr[user].name)) {
        write_str(user,"You already voted!");
        FCLOSE(fp); 
        return;
        }
     line[0]=0;
    }  /* end of while*/
   FCLOSE(fp);

  ADD:
  sprintf(filename,"%s/%s",LIBDIR,"votetallies");
   if (!(fp2=fopen(filename,"r")))
     {     
      write_str(user,"Cannot read tallies!");
      return;
     }
     fscanf(fp2,"%s\n",line);
     a=atoi(line);
     line[0]=0;
     fscanf(fp2,"%s\n",line);
     b=atoi(line);
     line[0]=0;
     fscanf(fp2,"%s\n",line);
     c=atoi(line);
     line[0]=0;
   FCLOSE(fp2);
   if (!(fp=fopen(filename,"w")))
     {     
      write_str(user,"Cannot write tallies!");
      a=0; b=0; c=0;
      return;
     }
  if (!strcmp(inpstr,"1")) a++;
  else if (!strcmp(inpstr,"2")) b++;
  else c++;

  sprintf(line,"%d\n",a);
  fputs(line,fp);
  line[0]=0;
  sprintf(line,"%d\n",b);
  fputs(line,fp);
  line[0]=0;
  sprintf(line,"%d\n",c);
  fputs(line,fp);
  line[0]=0;
  FCLOSE(fp);

   sprintf(filename,"%s/%s",LIBDIR,"voteusers");
   if (!(fp2=fopen(filename,"a")))
     {
      write_str(user,"Cannot append to user's vote file!");
      return;
     }
   fputs(ustr[user].name,fp2);
   fputs("\n",fp2);
   FCLOSE(fp2);

write_str(user,"*CHIK* *CHIK* Thanx for your vote!");
write_log(SYSTEMLOG,YESTIME,"VOTE: by %s\n",ustr[user].say_name);
return;
}

else if ( (!strcmp(inpstr,"-c")) && (ustr[user].tempsuper >= VOTE_LEVEL) ) {
   sprintf(filename,"%s/%s",LIBDIR,"voteusers");
   remove(filename);
   sprintf(filename,"%s/%s",LIBDIR,"votefile");
   remove(filename);
   write_str(user,"Vote topic deleted..");
   sprintf(filename,"%s/%s",LIBDIR,"votetallies");
   if (!(fp2=fopen(filename,"w"))) {
      write_str(user,"Reset user's file but cant erase and reset tallies");
      return;
      }
   fputs("0\n0\n0\n",fp2);
   FCLOSE(fp2);
   write_str(user,"Users and tallies erased..Files reset.");
   write_log(SYSTEMLOG,YESTIME,"VOTE: RESET by %s\n",ustr[user].say_name);
   return;
   }

else if ( (!strcmp(inpstr,"-d")) && (ustr[user].tempsuper >= VOTE_LEVEL) ) {
     inpstr[0]=0;
     enter_votedesc(user,inpstr);
     return;
     }

else {
  write_str(user,"Choice not understood. Pick 1, 2, or 3.");
  return;
  }
}

/* Enter vote file */
void enter_votedesc(int user, char *inpstr)
{
char *c;
int ret_val;
int redo = 0;
char filename2[FILE_NAME_LEN];
FILE *fp;

/* get memory */
STARTVOTED:

if (!ustr[user].vote_enter) {
        if (!(ustr[user].vote_start=(char *)malloc(82*VOTE_LINES))) {
        write_str(user,BAD_MALLOC);
        write_log(ERRLOG,YESTIME,"MALLOC: Can't malloc memory in enter_votedesc! %s\n",get_error());
        return;
        }
    ustr[user].vote_enter=1;
    ustr[user].vote_end=ustr[user].vote_start;
    if (!redo) strcpy(ustr[user].mutter,ustr[user].flags);
    user_ignore(user,"all");
    write_str(user,"");
    write_str(user,"** Entering next vote topic, finish with a '.' on a line by itself **");
    sprintf(mess,"** Max lines you can write is %d",VOTE_LINES);
    write_str(user,mess);
    write_str(user,"");
    write_str_nr(user,"1: ");
	 telnet_write_eor(user);
    noprompt=1;
    return;
    }
inpstr[80]=0;  c=inpstr;

/* check for dot terminator */
ret_val=0;

if (ustr[user].vote_enter > VOTE_LINES) {
   if (*c=='s' && *(c+1)==0) {
     ret_val=write_vote(user);
        if (ret_val) {
	  write_str(user,"");
	  write_str(user,"Vote Topic stored");
          write_str(user,"*** Don't forget to clear the votes with .vote -c ***");
          }
        else {
	write_str(user,"");
	write_str(user,"Vote Topic not stored");
	}
        free(ustr[user].vote_start);  ustr[user].vote_enter=0;
        ustr[user].vote_end='\0';
        noprompt=0;
        strcpy(ustr[user].flags,ustr[user].mutter);
        ustr[user].mutter[0]=0;
        return;
     }
   else if (*c=='v' && *(c+1)==0) {
write_str(user,"+-----------------------------------------------------------------------------+");
c='\0';
strcpy(filename2,get_temp_file());
fp=fopen(filename2,"w");
for (c=ustr[user].vote_start;c<ustr[user].vote_end;++c) { 
    putc(*c,fp);
    }
    fclose(fp);
    cat(filename2,user,0);
    remove(filename2);
c='\0';
write_str(user,"+-----------------------------------------------------------------------------+");
            write_str_nr(user,PROFILE_PROMPT);
		 telnet_write_eor(user);
            noprompt=1;  return;
        }
   else if (*c=='r' && *(c+1)==0) {
        free(ustr[user].vote_start); ustr[user].vote_enter=0;
        ustr[user].vote_end='\0';
        redo=1;
        goto STARTVOTED;
        }             
   else if (*c=='a' && *(c+1)==0) {
        free(ustr[user].vote_start); ustr[user].vote_enter=0;
        ustr[user].vote_end='\0';
	write_str(user,"");
        write_str(user,"Vote Topic not stored");
        noprompt=0;
        strcpy(ustr[user].flags,ustr[user].mutter);
        ustr[user].mutter[0]=0;
        return;
        }             
   else {
    write_str_nr(user,PROFILE_PROMPT);
	 telnet_write_eor(user);
    return;
   } 
  }

if (*c=='.' && *(c+1)==0) {
        if (ustr[user].vote_enter!=1)   {
            ustr[user].vote_enter= VOTE_LINES + 1;
            write_str_nr(user,PROFILE_PROMPT);
		 telnet_write_eor(user);
            noprompt=1;  return; 
            }
        else {
	write_str(user,"");
	write_str(user,"Vote Topic not stored");
	}
        free(ustr[user].vote_start);  ustr[user].vote_enter=0;
        noprompt=0;
        strcpy(ustr[user].flags,ustr[user].mutter);
        ustr[user].mutter[0]=0;
        return;
        }

/* write string to memory */
while(*c) *ustr[user].vote_end++=*c++;
*ustr[user].vote_end++='\n';

/* end of lines */
if (ustr[user].vote_enter==VOTE_LINES) {
            ustr[user].vote_enter= VOTE_LINES + 1;
            write_str_nr(user,PROFILE_PROMPT);
		 telnet_write_eor(user);
            noprompt=1;  return;
        }
sprintf(mess,"%d: ",++ustr[user].vote_enter);
write_str_nr(user,mess);
telnet_write_eor(user);
}
     
/*--------------------------------------------*/
/* Force a user to change his or her settings */
/* or to execute a command                    */
/*--------------------------------------------*/
void force_user(int user, char *inpstr)
{
int u,com_num_two,value=0;
int online=1;
char temp[EMAIL_LENGTH+1];
char other_user[ARR_SIZE],command[256];
char filename[FILE_NAME_LEN];

command[0]=0;

if (!strlen(inpstr)) {
   write_str(user,"Who do you want to force?");
   write_str(user,".force <user> <option> <setting>");
   return;
   }

sscanf(inpstr,"%s ",other_user);
strtolower(other_user);

if ((u=get_user_num(other_user,user))== -1)
  {
    if (!read_user(other_user)) {
    write_str(user,NO_USER_STR);
    return;
    }
    online=0;
  }

if (online) {
if (u == user)
  {
   write_str(user,"Why go through all that trouble when you can do it yourself!");
   return;
  }
}

if (online) {
 if ( (!strcmp(ustr[u].name,ROOT_ID)) ||
      (!strcmp(ustr[u].name,BOT_ID) && strcmp(ustr[user].name,BOTS_ROOTID)) ||
      ((ustr[user].tempsuper <= ustr[u].super) && (strcmp(ustr[user].name,ROOT_ID)))
    ) {
    write_str(user,"Yeah, right!");
    return;
    }
 }
else {
 if ((!strcmp(other_user,ROOT_ID)) || 
    (!strcmp(other_user,BOT_ID) && strcmp(ustr[user].name,BOTS_ROOTID)) ||
    ((ustr[user].tempsuper <= t_ustr.super) && (strcmp(ustr[user].name,ROOT_ID)))
    ) {
    write_str(user,"Yeah, right!");
    return;
    }
}


remove_first(inpstr);
sscanf(inpstr,"%s ",command);
strtolower(command);
if (strlen(command) > 8) {
   write_str(user,"Setting name too long");
   write_str(user,"Valid options are:");
   write_str(user,"  abbrs (toggle)   autoread (toggle)   autofwd  (toggle)");  
   write_str(user,"  car   (toggle)   color    (on|off)   cols     (16-256)");  
   write_str(user,"  desc  ( mess )   email    ( mess )   entermsg ( mess )");  
   write_str(user,"  fail  ( mess )   gender   ( mess )   hi       (toggle)");
   write_str(user,"  quote (toggle)   rows     (5-256)    homepage ( mess )");
   write_str(user,"  space (toggle)   succ     ( mess )   exitmsg  ( mess )");
   write_str(user,"  passhid (toggle) pbreak   (toggle)   beeps    (toggle)");
   write_str(user,"  icq   ( mess )   profdel");
   write_str(user,"");
   return;
   }

remove_first(inpstr);
if (!strcmp(command,"abbrs")) goto ABBR;
else if (!strcmp(command,"autoread")) goto AUTOR;
else if (!strcmp(command,"autofwd")) goto AUTOF;
else if (!strcmp(command,"car") || !strcmp(command,"carriage")) goto CAR;
else if (!strcmp(command,"color")) goto COLOR;
else if (!strcmp(command,"cols") || !strcmp(command,"width")) goto COLS;
else if (!strcmp(command,"desc")) goto DESC;
else if (!strcmp(command,"email")) goto EMAIL;
else if (!strcmp(command,"entermsg")) goto ENTER;
else if (!strcmp(command,"exitmsg")) goto EXITM;
else if (!strcmp(command,"fail")) goto FAIL;
else if (!strcmp(command,"gender")) goto GENDER;
else if (!strcmp(command,"hi")) goto HILI;
else if (!strcmp(command,"quote")) goto QUOTEP;
else if (!strcmp(command,"profdel")) goto PROFDEL;
else if (!strcmp(command,"rows") || !strcmp(command,"lines")) goto ROWS;
else if (!strcmp(command,"homepage")) goto HOMEP;
else if (!strcmp(command,"space")) goto SPACE;
else if (!strcmp(command,"succ")) goto SUCC;
else if (!strcmp(command,"com")) goto COMMS;
else if (!strcmp(command,"passhid")) goto PASS;
else if (!strcmp(command,"pbreak")) goto P_BREAK;
else if (!strcmp(command,"beeps")) goto BEEPSET;
else if (!strcmp(command,"icq")) goto ICQSET;
else {
      write_str(user,"Option unknown.");
      write_str(user,"Valid options are:");
   write_str(user,"  abbrs (toggle)   autoread (toggle)   autofwd  (toggle)");  
   write_str(user,"  car   (toggle)   color    (on|off)   cols     (16-256)");  
   write_str(user,"  desc  ( mess )   email    ( mess )   entermsg ( mess )");  
   write_str(user,"  fail  ( mess )   gender   ( mess )   hi       (toggle)");
   write_str(user,"  quote (toggle)   rows     (5-256)    homepage ( mess )");
   write_str(user,"  space (toggle)   succ     ( mess )   exitmsg  ( mess )");
   write_str(user,"  passhid (toggle) pbreak   (toggle)   beeps    (toggle)");
   write_str(user,"  icq   ( mess )   profdel");
   write_str(user,"");
   return;
     }

ABBR:
if (online) {
  if (ustr[u].abbrs)
    {
      write_str(user, "Abbreviations are now off for them.");
      ustr[u].abbrs = 0;
    }
   else
    {
      write_str(user, "They can now use abbreviations");
      ustr[u].abbrs = 1;
    }

  read_user(ustr[u].login_name);
  t_ustr.abbrs = ustr[u].abbrs;
  write_user(ustr[u].login_name);
  write_log(SYSTEMLOG,YESTIME,"FORCE: %s FORCED %s with abbrs_set\n",ustr[user].say_name,ustr[u].say_name);
}
else {
  if (t_ustr.abbrs)
    {
      write_str(user, "Abbreviations are now off for them.");
      t_ustr.abbrs = 0;
    }
   else
    {
      write_str(user, "They can now use abbreviations");
      t_ustr.abbrs = 1;
    }
write_user(other_user);
write_log(SYSTEMLOG,YESTIME,"FORCE: %s FORCED %s with abbrs_set\n",ustr[user].say_name,t_ustr.say_name);
}
goto END;

AUTOR:
if (online) {
  if (ustr[u].autor==3)
    {
      write_str(user, "Autoread now ^HYoff^ for them.");
      ustr[u].autor = 0;
    }
   else if (ustr[u].autor==2)
    {
      write_str(user, "Autoread now on ^HYfor logins and online^ for them.");
      ustr[u].autor = 3;
    }
   else if (ustr[u].autor==1)
    {
      write_str(user, "Autoread now on ^HYfor online only^ for them.");
      ustr[u].autor = 2;
    }
   else if (ustr[u].autor==0)
    {
      write_str(user, "Autoread now on ^HYfor logins only^ for them.");
      ustr[u].autor = 1;
    }

  copy_from_user(u);
  write_user(ustr[u].name);
  write_log(SYSTEMLOG,YESTIME,"FORCE: %s FORCED %s with Autoread_set\n",ustr[user].say_name,ustr[u].say_name);
}
else {
  if (t_ustr.autor==3)
    {
      write_str(user, "Autoread now ^HYoff^ for them.");
      t_ustr.autor = 0;
    }
   else if (t_ustr.autor==2)
    {
      write_str(user, "Autoread now on ^HYfor logins and online^ for them.");
      t_ustr.autor = 3;
    }
   else if (t_ustr.autor==1)
    {
      write_str(user, "Autoread now on ^HYfor online only^ for them.");
      t_ustr.autor = 2;
    }
   else if (t_ustr.autor==0)
    {
      write_str(user, "Autoread now on ^HYfor logins only^ for them.");
      t_ustr.autor = 1;
    }

write_user(other_user);
write_log(SYSTEMLOG,YESTIME,"FORCE: %s FORCED %s with Autoread_set\n",ustr[user].say_name,t_ustr.say_name);
}
goto END;

AUTOF:
if (online) {
  if (ustr[u].autof==2)
    {
      write_str(user, "Autofwd now ^HYoff^ for them.");
      ustr[u].autof = 0;
    }
   else if (ustr[u].autof==0)
    {
      write_str(user, "Autofwd now on ^HYall the time^ for them.");
      ustr[u].autof = 1;
    }
   else if (ustr[u].autof==1)
    {
      write_str(user, "Autofwd now on ^HYonly when not online^ for them.");
      ustr[u].autof = 2;
    }

  copy_from_user(u);
  write_user(ustr[u].name);
  write_log(SYSTEMLOG,YESTIME,"FORCE: %s FORCED %s with Autofwd_set\n",ustr[user].say_name,ustr[u].say_name);
}
else {
  if (t_ustr.autof==2)
    {
      write_str(user, "Autofwd now ^HYoff^ for them.");
      t_ustr.autof = 0;
    }
   else if (t_ustr.autof==0)
    {
      write_str(user, "Autofwd now on ^HYall the time^ for them.");
      t_ustr.autof = 1;
    }
   else if (t_ustr.autof==1)
    {
      write_str(user, "Autofwd now on ^HYonly when not online^ for them.");
      t_ustr.autof = 2;
    }

write_user(other_user);
write_log(SYSTEMLOG,YESTIME,"FORCE: %s FORCED %s with Autofwd_set\n",ustr[user].say_name,t_ustr.say_name);
}
goto END;

CAR:

if (online) {
  
if (!strlen(inpstr)) {
  if (!ustr[u].car_return) {
   write_str(user,"Set their carriage returns ON");
   ustr[u].car_return = 1;
   }
   else {
   write_str(user,"Set their carriage returns OFF");
   ustr[u].car_return = 0;
   }
 }
else {
 if (!strcmp(inpstr,"1")) {
   write_str(user,"Set their carriage returns ON");
   ustr[u].car_return = 1;
   }
 else if (!strcmp(inpstr,"0")) {
   write_str(user,"Set their carriage returns OFF");
   ustr[u].car_return = 0;
  }
 else {
   write_str(user,"Set their carriage returns ON");
   ustr[u].car_return = 1;
  }   
 } /* end of else */
  copy_from_user(u);
  write_user(ustr[u].login_name);
  write_log(SYSTEMLOG,YESTIME,"FORCE: %s FORCED %s with Carriages_set\n",ustr[user].say_name,ustr[u].say_name);
}
else {
if (!strlen(inpstr)) {
  if (!t_ustr.car_return) {
   write_str(user,"Set their carriage returns ON");
   t_ustr.car_return = 1;
   }
   else {
   write_str(user,"Set their carriage returns OFF");
   t_ustr.car_return = 0;
   }
 }
else {
 if (!strcmp(inpstr,"1")) {
   write_str(user,"Set their carriage returns ON");
   t_ustr.car_return = 1;
   }
 else if (!strcmp(inpstr,"0")) {
   write_str(user,"Set their carriage returns OFF");
   t_ustr.car_return = 0;
  }
 else {
   write_str(user,"Set their carriage returns ON");
   t_ustr.car_return = 1;
  }   
 } /* end of else */

write_user(other_user);
write_log(SYSTEMLOG,YESTIME,"FORCE: %s FORCED %s with Carriages_set\n",ustr[user].say_name,t_ustr.say_name);
}
goto END;

COLOR:
if (online) {
if (!strcmp(inpstr,"on") || !strcmp(inpstr,"ON")) {
   write_str(user,"Color is now   On for them.");
   ustr[u].color=1;
   }

if (!strcmp(inpstr,"off") || !strcmp(inpstr,"OFF")) {
   write_str(user,"Color is now   Off for them.");
   ustr[u].color=0;
   }

  copy_from_user(u);
  write_user(ustr[u].name);
  write_log(SYSTEMLOG,YESTIME,"FORCE: %s FORCED %s with Color_set\n",ustr[user].say_name,ustr[u].say_name);
}
else {
if (!strcmp(inpstr,"on") || !strcmp(inpstr,"ON")) {
   write_str(user,"Color is now   On for them.");
   t_ustr.color=1;
   }

if (!strcmp(inpstr,"off") || !strcmp(inpstr,"OFF")) {
   write_str(user,"Color is now   Off for them.");
   t_ustr.color=0;
   }

write_user(other_user);
write_log(SYSTEMLOG,YESTIME,"FORCE: %s FORCED %s with Color_set\n",ustr[user].say_name,t_ustr.say_name);
}
goto END;

COLS:
  value=5;

  sscanf(inpstr,"%d", &value);

  if (value < 16 || value > 256)
    {
      write_str(user,"cols set to 256 (valid range is 16 to 256)");
      value = 256;
    }

  sprintf(mess,"Set their terminal cols to: %d",value);
  write_str(user,mess);

if (online) {
  ustr[u].cols = value;
  copy_from_user(u);
  write_user(ustr[u].name);
  write_log(SYSTEMLOG,YESTIME,"FORCE: %s FORCED %s with Cols_set\n",ustr[user].say_name,ustr[u].say_name);
}
else {
  t_ustr.cols     = value;
  write_user(other_user);
  write_log(SYSTEMLOG,YESTIME,"FORCE: %s FORCED %s with Cols_set\n",ustr[user].say_name,t_ustr.say_name);
}
goto END;

DESC:
if (online) {
if (!strlen(inpstr))
  {
   sprintf(mess,"Their description is : %s",ustr[u].desc);
   write_str(user,mess);
   return;
  }

if (strlen(inpstr) > DESC_LEN-1)
  {
    write_str(user,"Description too long");
    return;
  }

strcpy(ustr[u].desc,inpstr);
copy_from_user(u);
write_user(ustr[u].login_name);
sprintf(mess,"Their new desc: %s",ustr[u].desc);
write_str(user,mess);
write_log(SYSTEMLOG,YESTIME,"FORCE: %s FORCED %s with Desc_set\n",ustr[user].say_name,ustr[u].say_name);
}
else {
if (!strlen(inpstr))
  {
   sprintf(mess,"Their description is : %s",t_ustr.desc);
   write_str(user,mess);
   return;
  }

if (strlen(inpstr) > DESC_LEN-1)
  {
    write_str(user,"Description too long");
    return;
  }

strcpy(t_ustr.desc,inpstr);
sprintf(mess,"Their new desc: %s",t_ustr.desc);
write_str(user,mess);
write_user(other_user);
write_log(SYSTEMLOG,YESTIME,"FORCE: %s FORCED %s with Desc_set\n",ustr[user].say_name,t_ustr.say_name);
}
goto END;

EMAIL:
if (online) {
  /* Check for illegal characters in email addy */
  if (strpbrk(inpstr,";/[]\\") ) {
     write_str(user,"Illegal email address");
     return;
     }

  if (strstr(inpstr,"^")) {
      write_str(user,"Email cant have color or hilite codes in it.");
      return;
      }
  if (strlen(inpstr)>EMAIL_LENGTH)
    {
      write_str(user,"Email address truncated");
      inpstr[EMAIL_LENGTH-1]=0;
    }
  if ((!strcmp(inpstr,"-c")) || (!strcmp(inpstr,"clear"))) {
      strcpy(inpstr,DEF_EMAIL);
      write_str(user,"Email address cleared and reset.");
      goto SKIP;
      }       

 strcpy(temp,inpstr);
 strtolower(temp);

  if (strstr(temp,"whitehouse.gov"))
      {
       write_str(user,"Email address not valid.");
       return;
      }
  else if (!strlen(inpstr)) {
      write_str(user,"Must specify an address or -c|clear");
      return;
      }
  else if (!strstr(inpstr,".") || !strstr(inpstr,"@")) {
       write_str(user,"Email address not valid.");
       return;
      }

  sprintf(mess,"Set their email address to: %s",inpstr);
  write_str(user,mess);

  SKIP:
  strcpy(ustr[u].email_addr,inpstr);
  copy_from_user(u);
  write_user(ustr[u].name);
  write_log(SYSTEMLOG,YESTIME,"FORCE: %s FORCED %s with Email_set\n",ustr[user].say_name,ustr[u].say_name);
}
else {
  /* Check for illegal characters in email addy */
  if (strpbrk(inpstr,";/[]\\") ) {
     write_str(user,"Illegal email address");
     return;
     }

  if (strstr(inpstr,"^")) {
      write_str(user,"Email cant have color or hilite codes in it.");
      return;
      }
  if (strlen(inpstr)>EMAIL_LENGTH)
    {
      write_str(user,"Email address truncated");
      inpstr[EMAIL_LENGTH-1]=0;
    }
  if ((!strcmp(inpstr,"-c")) || (!strcmp(inpstr,"clear"))) {
      strcpy(inpstr,DEF_EMAIL);
      write_str(user,"Email address cleared and reset.");
      goto SKIP2;
      }       

 strcpy(temp,inpstr);
 strtolower(temp);

  if (strstr(temp,"whitehouse.gov"))
      {
       write_str(user,"Email address not valid.");
       return;
      }
  else if (!strlen(inpstr)) {
      write_str(user,"Must specify an address or -c|clear");
      return;
      }
  else if (!strstr(inpstr,".") || !strstr(inpstr,"@")) {
       write_str(user,"Email address not valid.");
       return;
      }

  sprintf(mess,"Set their email address to: %s",inpstr);
  write_str(user,mess);

  SKIP2:
  strcpy(t_ustr.email_addr,inpstr);
  write_user(other_user);
  write_log(SYSTEMLOG,YESTIME,"FORCE: %s FORCED %s with Email_set\n",ustr[user].say_name,t_ustr.say_name);
}
goto END;

ENTER:
if (online) {
if (!strlen(inpstr)) {
   sprintf(mess,"Their entermessage is: %s",ustr[u].entermsg);
   write_str(user,mess);
   return;
   }
if (!strcmp(inpstr,"clear") || !strcmp(inpstr,"none") ||
    !strcmp(inpstr,"-c")) {
    strcpy(ustr[u].entermsg,DEF_ENTER);
    copy_from_user(u);
    write_user(ustr[u].name);
    write_str(user,"Their entermsg now set to default.");
    write_log(SYSTEMLOG,YESTIME,"FORCE: %s FORCED %s with EnterMess_set\n",ustr[user].say_name,ustr[u].say_name);
    goto END;
    }
if (strlen(inpstr) > MAX_ENTERM) {
   write_str(user,"Message too long.");
   return;
   }

strcpy(ustr[u].entermsg,inpstr);
copy_from_user(u);
write_user(ustr[u].name);
sprintf(mess,"Their new entermsg: %s",ustr[u].entermsg);
write_str(user,mess);
write_log(SYSTEMLOG,YESTIME,"FORCE: %s FORCED %s with EnterMess_set\n",ustr[user].say_name,ustr[u].say_name);
}
else {
if (!strlen(inpstr)) {
   sprintf(mess,"Their entermessage is: %s",t_ustr.entermsg);
   write_str(user,mess);
   return;
   }
if (!strcmp(inpstr,"clear") || !strcmp(inpstr,"none") ||
    !strcmp(inpstr,"-c")) {
    strcpy(t_ustr.entermsg,DEF_ENTER);
    write_str(user,"Their entermsg now set to default.");
    write_user(other_user);
    write_log(SYSTEMLOG,YESTIME,"FORCE: %s FORCED %s with EnterMess_set\n",ustr[user].say_name,t_ustr.say_name);
    goto END;
    }
if (strlen(inpstr) > MAX_ENTERM) {
   write_str(user,"Message too long.");
   return;
   }

strcpy(t_ustr.entermsg,inpstr);
sprintf(mess,"Their new entermsg: %s",t_ustr.entermsg);
write_str(user,mess);
write_user(other_user);
write_log(SYSTEMLOG,YESTIME,"FORCE: %s FORCED %s with EnterMess_set\n",ustr[user].say_name,t_ustr.say_name);
}
goto END;

EXITM:
if (online) {
if (!strlen(inpstr)) {
   sprintf(mess,"Their exitmessage is: %s",ustr[u].exitmsg);
   write_str(user,mess);
   return;
   }
if (!strcmp(inpstr,"clear") || !strcmp(inpstr,"none") ||
    !strcmp(inpstr,"-c")) {
    strcpy(ustr[u].exitmsg,DEF_EXIT);
    copy_from_user(u);
    write_user(ustr[u].name);
    write_str(user,"Their exitmsg now set to default.");
    write_log(SYSTEMLOG,YESTIME,"FORCE: %s FORCED %s with ExitMess_set\n",ustr[user].say_name,ustr[u].say_name);
    goto END;
    }
if (strlen(inpstr) > MAX_EXITM) {
   write_str(user,"Message too long.");
   return;
   }

strcpy(ustr[u].exitmsg,inpstr);
copy_from_user(u);
write_user(ustr[u].name);
sprintf(mess,"Their new exitmsg: %s",ustr[u].exitmsg);
write_str(user,mess);
write_log(SYSTEMLOG,YESTIME,"FORCE: %s FORCED %s with ExitMess_set\n",ustr[user].say_name,ustr[u].say_name);
}
else {
if (!strlen(inpstr)) {
   sprintf(mess,"Their exitmessage is: %s",t_ustr.exitmsg);
   write_str(user,mess);
   return;
   }
if (!strcmp(inpstr,"clear") || !strcmp(inpstr,"none") ||
    !strcmp(inpstr,"-c")) {
    strcpy(t_ustr.exitmsg,DEF_EXIT);
    write_str(user,"Their exitmsg now set to default.");
    write_user(other_user);
    write_log(SYSTEMLOG,YESTIME,"FORCE: %s FORCED %s with ExitMess_set\n",ustr[user].say_name,t_ustr.say_name);
    goto END;
    }
if (strlen(inpstr) > MAX_EXITM) {
   write_str(user,"Message too long.");
   return;
   }

strcpy(t_ustr.exitmsg,inpstr);
sprintf(mess,"Their new exitmsg: %s",t_ustr.exitmsg);
write_str(user,mess);
write_user(other_user);
write_log(SYSTEMLOG,YESTIME,"FORCE: %s FORCED %s with ExitMess_set\n",ustr[user].say_name,t_ustr.say_name);
}
goto END;

FAIL:
if (online) {
if (!strlen(inpstr)) {
   sprintf(mess,"Their fail is: %s",ustr[u].fail);
   write_str(user,mess);
   return;
   }
if (!strcmp(inpstr,"clear") || !strcmp(inpstr,"none") ||
    !strcmp(inpstr,"-c")) {
    strcpy(ustr[u].fail,"");
    copy_from_user(u);
    write_user(ustr[u].name);
    write_str(user,"Fail message cleared.");
    write_log(SYSTEMLOG,YESTIME,"FORCE: %s FORCED %s with Fail_set\n",ustr[user].say_name,ustr[u].say_name);
    return;
    }

if (strlen(inpstr) > MAX_ENTERM) {
   write_str(user,"Message too long.");
   return;
   }

strcpy(ustr[u].fail,inpstr);
copy_from_user(u);
write_user(ustr[u].name);
sprintf(mess,"Their new fail: %s",ustr[u].fail);
write_str(user,mess);
write_log(SYSTEMLOG,YESTIME,"FORCE: %s FORCED %s with Fail_set\n",ustr[user].say_name,ustr[u].say_name);
}
else {
if (!strlen(inpstr)) {
   sprintf(mess,"Their fail is: %s",t_ustr.fail);
   write_str(user,mess);
   return;
   }
if (!strcmp(inpstr,"clear") || !strcmp(inpstr,"none") ||
    !strcmp(inpstr,"-c")) {
    strcpy(t_ustr.fail,"");
    write_str(user,"Fail message cleared.");
    write_user(other_user);
    write_log(SYSTEMLOG,YESTIME,"FORCE: %s FORCED %s with Fail_set\n",ustr[user].say_name,t_ustr.say_name);
    return;
    }

if (strlen(inpstr) > MAX_ENTERM) {
   write_str(user,"Message too long.");
   return;
   }

strcpy(t_ustr.fail,inpstr);
sprintf(mess,"Their new fail: %s",t_ustr.fail);
write_str(user,mess);
write_user(other_user);
write_log(SYSTEMLOG,YESTIME,"FORCE: %s FORCED %s with Fail_set\n",ustr[user].say_name,t_ustr.say_name);
}
goto END;

GENDER:
if (online) {
  if (strlen(inpstr)>29)
    {
      write_str(user,"Gender truncated");
      inpstr[29]=0;
    }

  sprintf(mess,"Set their gender to: %s",inpstr);
  write_str(user,mess);

  strcpy(ustr[u].sex,inpstr);
  copy_from_user(u);
  write_user(ustr[u].name);
  write_log(SYSTEMLOG,YESTIME,"FORCE: %s FORCED %s with Gender_set\n",ustr[user].say_name,ustr[u].say_name);
}
else {
  if (strlen(inpstr)>29)
    {
      write_str(user,"Gender truncated");
      inpstr[29]=0;
    }

  sprintf(mess,"Set their gender to: %s",inpstr);
  write_str(user,mess);

  strcpy(t_ustr.sex,inpstr);
write_user(other_user);
write_log(SYSTEMLOG,YESTIME,"FORCE: %s FORCED %s with Gender_set\n",ustr[user].say_name,t_ustr.say_name);
}
goto END;

HILI:
if (online) {
  if (ustr[u].hilite==2)
    {
      write_str(user, "High_lighting now off for them.");
      ustr[u].hilite = 0;
    }
   else if (ustr[u].hilite==1)
    {
      write_str(user, "High_lighting now on for them for everything except private communication which will be normal with color.");
      ustr[u].hilite = 2;
    }
   else
    {
      write_str(user, "High_lighting now on for them for everyhting.");
      ustr[u].hilite = 1;
    }

  copy_from_user(u);
  write_user(ustr[u].name);
  write_log(SYSTEMLOG,YESTIME,"FORCE: %s FORCED %s with Hi_set\n",ustr[user].say_name,ustr[u].say_name);
}
else {
  if (t_ustr.hilite==2)
    {
      write_str(user, "High_lighting now off for them.");
      t_ustr.hilite = 0;
    }
   else if (t_ustr.hilite==1)
    {
      write_str(user, "High_lighting now on for them for everything except private communication which will be normal with color.");
      t_ustr.hilite = 2;
    }
   else
    {
      write_str(user, "High_lighting now on for them for everything.");
      t_ustr.hilite = 1;
    }

write_user(other_user);
write_log(SYSTEMLOG,YESTIME,"FORCE: %s FORCED %s with Hi_set\n",ustr[user].say_name,t_ustr.say_name);
}
goto END;

PASS:
if (online) {
  if (ustr[u].passhid)
    {
      write_str(user, "Password WILL be echoed during logins for them.");
      ustr[u].passhid = 0;
    }
   else
    {
      write_str(user, "Password will NOT be echoed during logins for them.");
      ustr[u].passhid = 1;
    }

  copy_from_user(u);
  write_user(ustr[u].name);
  write_log(SYSTEMLOG,YESTIME,"FORCE: %s FORCED %s with Passhid_set\n",ustr[user].say_name,ustr[u].say_name);
}
else {
  if (t_ustr.passhid)
    {
      write_str(user, "Password WILL be echoed during logins for them.");
      t_ustr.passhid = 0;
    }
   else
    {
      write_str(user, "Password will NOT be echoed during logins for them.");
      t_ustr.passhid = 1;
    }

write_user(other_user);
write_log(SYSTEMLOG,YESTIME,"FORCE: %s FORCED %s with Passhid_set\n",ustr[user].say_name,t_ustr.say_name);
}
goto END;

P_BREAK:
if (online) {
  if (ustr[u].pbreak)
    {
      write_str(user, "Who listing will be continuous for them.");
      ustr[u].pbreak = 0;
    }
   else
    {
      write_str(user, "Who listing will be paged for them.");
      ustr[u].pbreak = 1;
    }

  copy_from_user(u);
  write_user(ustr[u].name);
  write_log(SYSTEMLOG,YESTIME,"FORCE: %s FORCED %s with Pbreak_set\n",ustr[user].say_name,ustr[u].say_name);
}
else {
  if (t_ustr.pbreak)
    {
      write_str(user, "Who listing will be continuous for them.");
      t_ustr.pbreak = 0;
    }
   else
    {
      write_str(user, "Who listing will be paged for them.");
      t_ustr.pbreak = 1;
    }

write_user(other_user);
write_log(SYSTEMLOG,YESTIME,"FORCE: %s FORCED %s with Pbreak_set\n",ustr[user].say_name,t_ustr.say_name);
}
goto END;

BEEPSET:
if (online) {
  if (ustr[u].beeps)
    {
      write_str(user, "They now will ^NOT^ get beeps on priv. comms.");
      ustr[u].beeps = 0;
    }
   else
    {
      write_str(user, "They now ^WILL^ get beeps on priv. comms.");
      ustr[u].beeps = 1;
    }

  copy_from_user(u);
  write_user(ustr[u].name);
  write_log(SYSTEMLOG,YESTIME,"FORCE: %s FORCED %s with beep_set\n",ustr[user].say_name,ustr[u].say_name);
}
else {
  if (t_ustr.beeps)
    {
      write_str(user, "They now will ^NOT^ get beeps on priv. comms.");
      t_ustr.beeps = 0;
    }
   else
    {
      write_str(user, "They now ^WILL^ get beeps on priv. comms.");
      t_ustr.beeps = 1;
    }

write_user(other_user);
write_log(SYSTEMLOG,YESTIME,"FORCE: %s FORCED %s with beep_set\n",ustr[user].say_name,t_ustr.say_name);
}
goto END;

QUOTEP:
if (online) {
  if (ustr[u].quote)
    {
      write_str(user, "Quote feature now  off for them.");
      ustr[u].quote = 0;
    }
   else
    {
      write_str(user,"Quote feature now  on for them.");
      ustr[u].quote = 1;
    }

  copy_from_user(u);
  write_user(ustr[u].name);
  write_log(SYSTEMLOG,YESTIME,"FORCE: %s FORCED %s with Quote_set\n",ustr[user].say_name,ustr[u].say_name);
}
else {
  if (t_ustr.quote)
    {
      write_str(user, "Quote feature now  off for them.");
      t_ustr.quote = 0;
    }
   else
    {
      write_str(user,"Quote feature now  on for them.");
      t_ustr.quote = 1;
    }

write_user(other_user);
write_log(SYSTEMLOG,YESTIME,"FORCE: %s FORCED %s with Quote_set\n",ustr[user].say_name,t_ustr.say_name);
}
goto END;

ROWS:
  value=5;

  sscanf(inpstr,"%d", &value);

  if (value < 5 || value > 256)
    {
      write_str(user,"rows set to 25 (valid range is 5 to 256)");
      value = 25;
    }

  sprintf(mess,"Set their terminal rows to: %d",value);
  write_str(user,mess);

if (online) {
  ustr[u].rows = value;
  copy_from_user(u);
  write_user(ustr[u].name);
  write_log(SYSTEMLOG,YESTIME,"FORCE: %s FORCED %s with Rows_set\n",ustr[user].say_name,ustr[u].say_name);
}
else {
  t_ustr.rows     = value;
  write_user(other_user);
  write_log(SYSTEMLOG,YESTIME,"FORCE: %s FORCED %s with Rows_set\n",ustr[user].say_name,t_ustr.say_name);
}
goto END;

HOMEP:
if (online) {
  if (!strlen(inpstr)) {
	sprintf(mess,"Their homepage is: %s",ustr[u].homepage);
	write_str(user, mess);
	return;
	}

  if (strstr(inpstr,"^")) {
      write_str(user,"Homepage cant have color or hilite codes in it.");
      return;
      }

  if (strlen(inpstr) > HOME_LEN)
     {
      write_str(user,"Home page address truncated");
      inpstr[HOME_LEN-1]=0;
     }

  sprintf(mess,"Set page to: %s",inpstr);
  write_str(user,mess);

  strcpy(ustr[u].homepage,inpstr);
  copy_from_user(u);
  write_user(ustr[u].name);
  write_log(SYSTEMLOG,YESTIME,"FORCE: %s FORCED %s with Homepage_set\n",ustr[user].say_name,ustr[u].say_name);
}
else {
  if (!strlen(inpstr)) {
	sprintf(mess,"Their homepage is: %s",t_ustr.homepage);
	write_str(user, mess);
	return;
	}

  if (strstr(inpstr,"^")) {
      write_str(user,"Homepage cant have color or hilite codes in it.");
      return;
      }

  if (strlen(inpstr) > HOME_LEN)
     {
      write_str(user,"Home page address truncated");
      inpstr[HOME_LEN-1]=0;
     }

  sprintf(mess,"Set page to: %s",inpstr);
  write_str(user,mess);

  strcpy(t_ustr.homepage,inpstr);
  write_user(other_user);
  write_log(SYSTEMLOG,YESTIME,"FORCE: %s FORCED %s with Homepage_set\n",ustr[user].say_name,t_ustr.say_name);
}
goto END;

ICQSET:
if (online) {
  if (!strlen(inpstr)) {
	sprintf(mess,"Their ICQ # is: %s",ustr[u].icq);
	write_str(user, mess);
	return;
	}

  if (strstr(inpstr,"^")) {
      write_str(user,"ICQs cant have color or hilite codes in them.");
      return;
      }

  if (strlen(inpstr) > 20)
     {
      write_str(user,"ICQ number truncated");
      inpstr[20-1]=0;
     }

  sprintf(mess,"Set ICQ # to: %s",inpstr);
  write_str(user,mess);

  strcpy(ustr[u].icq,inpstr);
  copy_from_user(u);
  write_user(ustr[u].name);
  write_log(SYSTEMLOG,YESTIME,"FORCE: %s FORCED %s with ICQ_set\n",ustr[user].say_name,ustr[u].say_name);
}
else {
  if (!strlen(inpstr)) {
	sprintf(mess,"Their ICQ # is: %s",t_ustr.icq);
	write_str(user, mess);
	return;
	}

  if (strstr(inpstr,"^")) {
      write_str(user,"ICQs cant have color or hilite codes in them.");
      return;
      }

  if (strlen(inpstr) > 20)
     {
      write_str(user,"ICQ number truncated");
      inpstr[20-1]=0;
     }

  sprintf(mess,"Set ICQ # to: %s",inpstr);
  write_str(user,mess);

  strcpy(t_ustr.icq,inpstr);
  write_user(other_user);
  write_log(SYSTEMLOG,YESTIME,"FORCE: %s FORCED %s with ICQ_set\n",ustr[user].say_name,t_ustr.say_name);
}
goto END;

SPACE:
if (online) {
  if (ustr[u].white_space)
    {
      write_str(user, "White space removal is now off for them.");
      ustr[u].white_space = 0;
    }
   else
    {
      write_str(user, "White space removal is now on for them.");
      ustr[u].white_space = 1;
    }

  copy_from_user(u);
  write_user(ustr[u].name);
  write_log(SYSTEMLOG,YESTIME,"FORCE: %s FORCED %s with Space_set\n",ustr[user].say_name,ustr[u].say_name);
}
else {
  if (t_ustr.white_space)
    {
      write_str(user, "White space removal is now off for them.");
      t_ustr.white_space = 0;
    }
   else
    {
      write_str(user, "White space removal is now on for them.");
      t_ustr.white_space = 1;
    }

write_user(other_user);
write_log(SYSTEMLOG,YESTIME,"FORCE: %s FORCED %s with Space_set\n",ustr[user].say_name,t_ustr.say_name);
}
goto END;

SUCC:
if (online) {
if (!strlen(inpstr)) {
   sprintf(mess,"Their success is: %s",ustr[u].succ);
   write_str(user,mess);
   return;
   }
if (!strcmp(inpstr,"clear") || !strcmp(inpstr,"none") ||
    !strcmp(inpstr,"-c")) {
    strcpy(ustr[u].succ,"");
    copy_from_user(u);
    write_user(ustr[u].name);
    write_str(user,"Success message cleared.");
    write_log(SYSTEMLOG,YESTIME,"FORCE: %s FORCED %s with Succ_set\n",ustr[user].say_name,ustr[u].say_name);
    return;
    }
if (strlen(inpstr) > MAX_ENTERM) {
   write_str(user,"Message too long.");
   return;
   }

strcpy(ustr[u].succ,inpstr);
copy_from_user(u);
write_user(ustr[u].name);
sprintf(mess,"Their new success: %s",ustr[u].succ);
write_str(user,mess);
write_log(SYSTEMLOG,YESTIME,"FORCE: %s FORCED %s with Succ_set\n",ustr[user].say_name,ustr[u].say_name);
}
else {
if (!strlen(inpstr)) {
   sprintf(mess,"Their success is: %s",t_ustr.succ);
   write_str(user,mess);
   return;
   }
if (!strcmp(inpstr,"clear") || !strcmp(inpstr,"none") ||
    !strcmp(inpstr,"-c")) {
    strcpy(t_ustr.succ,"");
    write_str(user,"Success message cleared.");
    write_user(other_user);
    write_log(SYSTEMLOG,YESTIME,"FORCE: %s FORCED %s with Succ_set\n",ustr[user].say_name,t_ustr.say_name);
    return;
    }
if (strlen(inpstr) > MAX_ENTERM) {
   write_str(user,"Message too long.");
   return;
   }

strcpy(t_ustr.succ,inpstr);
sprintf(mess,"Their new success: %s",t_ustr.succ);
write_str(user,mess);
write_user(other_user);
write_log(SYSTEMLOG,YESTIME,"FORCE: %s FORCED %s with Succ_set\n",ustr[user].say_name,t_ustr.say_name);
}
goto END;

PROFDEL:
if (online) {
  sprintf(filename,"%s/%s",PRO_DIR,ustr[u].name);
  remove(filename);
  write_log(SYSTEMLOG,YESTIME,"FORCE: %s FORCED %s with Profile_Delete\n",ustr[user].say_name,ustr[u].say_name);
}
else {
  sprintf(filename,"%s/%s",PRO_DIR,t_ustr.name);
  remove(filename);
  write_log(SYSTEMLOG,YESTIME,"FORCE: %s FORCED %s with Profile_Delete\n",ustr[user].say_name,t_ustr.say_name);
}
  write_str(user,"User profile deleted.");
  goto END;


COMMS:
	if (!online) {
	write_str(user,"You cant force a command on someone that isn't here!");
	return;
	}
	       com_num_two=get_com_num(u,inpstr);

                if ((com_num_two == -1) &&
                    (inpstr[0] == '.' || !strcmp(ustr[u].name,BOT_ID)))
		  {
		   write_str(user,SYNTAX_ERROR);
		   return;
		  }
		  
		if (com_num_two != -1) 
		  {
		   last_user=u;
		   write_log(SYSTEMLOG,YESTIME,"FORCE: %s FORCED %s with COM %s\n",ustr[user].say_name,ustr[u].say_name,inpstr);
                   if ((!strcmp(ustr[u].name,BOT_ID) || !strcmp(ustr[u].name,ROOT_ID)) && inpstr[0]=='_')
			exec_bot_com(com_num_two,u,inpstr);
                   else
			exec_com(com_num_two,u,inpstr);
			
		   last_user= -1;
                   write_str(user,"Forced.");
		  }
goto END;

END:
online=0;
}

/*** Read the system log, search for string if specified ***/
void readlog(int user, char *inpstr)
{
int occured=0,mode=0,i=0,found=0;
char word[ARR_SIZE],line[ARR_SIZE],line2[ARR_SIZE];
char filename[FILE_NAME_LEN],filenamer[FILE_NAME_LEN];
FILE *fp;
FILE *pp;

if (ustr[user].log_stage) {
 /* we're coming back */
 if (!strlen(ustr[user].temp_buffer)) mode=1;
 else if (ustr[user].temp_buffer[0]=='-') mode=2;
 else mode=3;
 goto STARTL;
}
else {
 /* we're just starting */
 ustr[user].temp_buffer[0]=0;
 inpstr[80]=0;
 }

if (!strlen(inpstr)) 
  {
   sprintf(filenamer,logfacil[SYSTEMLOG].file,LOGDIR);
   if (!cat(filenamer,user,0)) {
      write_str(user,BAD_FILEIO);
      return;
      }
   return;
  }

sscanf(inpstr,"%s ",word);
strtolower(word);

if (!strcmp(word,"all")) {
remove_first(inpstr);
 if (!strlen(inpstr)) { ustr[user].temp_buffer[0]=0; mode=1; }
 else {
 sscanf(inpstr,"%s ",word);
 strtolower(word);
 if (word[0]=='-') {
    strcpy(ustr[user].temp_buffer,word);
    midcpy(word,word,1,3);
    mode=2;
 }
 else {
    strcpy(ustr[user].temp_buffer,word);
    mode=3;
    }
 } /* end of strlen else */

STARTL:
for (i=ustr[user].log_stage;logfacil[i].loglevel!=-1;++i) {
   sprintf(filenamer,logfacil[i].file,LOGDIR);
   if (!check_for_file(filenamer)) { ustr[user].log_stage++; continue; }
   sprintf(mess," ::%s::",logfacil[i].name);
   write_str(user,mess);
 if (mode==1) {
   if (!cat(filenamer,user,0)) { ustr[user].log_stage++; continue; }
   ustr[user].log_stage++;
   if (ustr[user].file_posn==0) continue;
   return;
 }
 else if (mode==2) {
   midcpy(ustr[user].temp_buffer,word,1,3);
   strcpy(filename,get_temp_file());
   sprintf(mess,"tail -%s %s",word,filenamer);
 if (!(pp=popen(mess,"r"))) {
	write_str(user,"Can't open pipe to get the log!");
        ustr[user].log_stage=0;
        ustr[user].temp_buffer[0]=0;
	return;
	}
 if (!(fp=fopen(filename,"w"))) {
	pclose(pp);
	write_str(user,"Can't open temp file for writing!");
        ustr[user].log_stage=0;
        ustr[user].temp_buffer[0]=0;
	return;
	}
while (fgets(line,256,pp) != NULL) {
	fputs(line,fp);
      } /* end of while */
fclose(fp);
pclose(pp);

   if (!cat(filename,user,0)) { ustr[user].log_stage++; continue; }
   ustr[user].log_stage++;
   if (ustr[user].file_posn==0) continue;
   return;
 }
 else if (mode==3) {
 occured=0;
	if (!(fp=fopen(filenamer,"r"))) { 
           write_str(user,BAD_FILEIO);
           ustr[user].log_stage=0;
           ustr[user].temp_buffer[0]=0;
           return;
           }
	fgets(line,256,fp);
	while(!feof(fp)) {
		strcpy(line2,line);
	        strtolower(line);
		if (instr2(0,line,ustr[user].temp_buffer,0)== -1) goto NEXT;
                   line2[strlen(line2)-1]=0;
		   write_str(user,line2);	
		   ++occured;
		NEXT:
		fgets(line,256,fp);
		}
	FCLOSE(fp);
  if (!occured) write_str(user,"No occurences found");
 }
} /* end of for */
ustr[user].log_stage=0;
return;
} /* END OF IF ALL */
else if (!strcmp(word,"-l")) {
write_str(user,"Availble logs to search");
write_str(user,"------------------------");

for (i=0;logfacil[i].loglevel!=-1;++i) {
   write_str(user,logfacil[i].name);
}

write_str(user,"");
return;
} /* END OF IF -l */
else {
sscanf(inpstr,"%s ",word);

for (i=0;logfacil[i].loglevel!=-1;++i) {
   if (!strcmp(logfacil[i].name,word)) { found=1; break; }
}
if (!found) {
  write_str(user,"There is no log by that name!");
  return;
  }
else found=0;
 
 sprintf(filenamer,logfacil[i].file,LOGDIR);
 remove_first(inpstr);
 sprintf(mess," ::%s::",logfacil[i].name);
 write_str(user,mess);

if (!strlen(inpstr)) {
   /* cat entire file */
   if (!cat(filenamer,user,0))
     write_str(user,BAD_FILEIO);
   return;
}
else {
   sscanf(inpstr,"%s ",word);
   strtolower(word);
  if (word[0]=='-') {
   midcpy(word,word,1,3);
   strcpy(filename,get_temp_file());
   sprintf(mess,"tail -%s %s",word,filenamer);
 if (!(pp=popen(mess,"r"))) {
	write_str(user,"Can't open pipe to get the log!");
	return;
	}
 if (!(fp=fopen(filename,"w"))) {
	pclose(pp);
	write_str(user,"Can't open temp file for writing!");
	return;
	}
while (fgets(line,256,pp) != NULL) {
	fputs(line,fp);
      } /* end of while */
fclose(fp);
pclose(pp);

   if (!cat(filename,user,0))
     write_str(user,BAD_FILEIO);
   return;


 } /* end of tail if */
 else {
	if (!(fp=fopen(filenamer,"r"))) { 
           write_str(user,BAD_FILEIO);
           return;
           }
	fgets(line,256,fp);
	while(!feof(fp)) {
		strcpy(line2,line);
	        strtolower(line);
		if (instr2(0,line,word,0)== -1) goto NEXT2;
                   line2[strlen(line2)-1]=0;
		   write_str(user,line2);	
		   ++occured;
		NEXT2:
		fgets(line,256,fp);
		}
	FCLOSE(fp);
  if (!occured) write_str(user,"No occurences found");
 } /* end of search else */
} /* end of strlen else */
} /* END OF MAIN ELSE */

}

/** Send a user to his or her home room */
void home_user(int user)
{
int area=ustr[user].area;
int new_area;
int found=0;

if (ustr[user].anchor) {
  write_str(user,ANCHORED_DOWN);
  return;
  }

      if ((ustr[user].tempsuper==0) && (!strcmp(ustr[user].desc,DEF_DESC))
          && (area==new_room)) {
          write_str(user,"You cant leave this room until you set a description with .desc");
          return;
         }

   found = FALSE;
   for (new_area=0; new_area < NUM_AREAS; ++new_area)
    {
     if ( !strcmp(astr[new_area].name, ustr[user].home_room) )
       {
         found = TRUE;
         break;
       }
    }
   
   if (!found) {
      write_str(user,"That room no longer exists.");
      return;
     }  

if (!strcmp(astr[area].name,ARREST_ROOM) &&
     strcmp(ustr[user].name,ROOT_ID)) {
   write_str(user,"You cant go home from this room!");
   return;
   }

/*----------------------------------------------*/
/* check to see if the user is in that room     */
/*----------------------------------------------*/
   
if (ustr[user].area == new_area)
  {
    write_str(user,"You are in that room now!");
    return;
  }

/*-----------------------------------------------------------*/
/* if the room is private abort home...inform user           */
/*-----------------------------------------------------------*/

if (astr[new_area].private && ustr[user].invite != new_area )
  {
   write_str(user,"Sorry - that room is currently private");
   return;
  }

  sprintf(mess,"%s goes home.",ustr[user].say_name);
      
/* send output to old room & to conv file */
if (!ustr[user].vis)
        strcpy(mess,INVIS_MOVES);

writeall_str(mess, 1, user, 0, user, NORM, NONE, 0);

   if (area == ustr[bot].area) {
    sprintf(mess,"+++++ left:%s", strip_color(ustr[user].say_name));
    write_bot(mess);
    }

/*-----------------------------------------------------------*/
/* return room to public     (if needed)                     */
/*-----------------------------------------------------------*/
      
if (astr[area].private && (find_num_in_area(area) <= PRINUM))
  {
   strcpy(mess, NOW_PUBLIC);
   writeall_str(mess, 1, user, 0, user, NORM, NONE, 0);
   cbuff(user);
   astr[area].private=0;
  }

/* record movement */ 
sprintf(mess,"%s %s",ustr[user].say_name,ustr[user].entermsg);

/* send output to new room */
if (!ustr[user].vis) 
	strcpy(mess,INVIS_MOVES);

ustr[user].area = new_area;

writeall_str(mess, 1, user, 0, user, NORM, NONE, 0);

   if (new_area == ustr[bot].area) {
    sprintf(mess,"+++++ came in:%s", strip_color(ustr[user].say_name));
    write_bot(mess);
    }

look(user,"");
}


/* MEMORY SUB FUNCTIONS FOR check_mem() */
long temp_mem_get(void)
{
int i=0;
int j=0;
int macro=0;
int abbrev1=0;
int con=0;
long tot_mem=0;
ConvPtr tmpConv;
MacroPtr tmpMacros = t_ustr.Macros;

     for (i=0;i<NUM_MACROS;++i) {
	 macro += strlen(tmpMacros->name[i]);
	 macro += strlen(tmpMacros->body[i]);
	 }
     for (i=0;i<NUM_ABBRS;++i) {
	 abbrev1 += strlen(t_ustr.custAbbrs[i].abbr);
	 abbrev1 += strlen(t_ustr.custAbbrs[i].com);
	 }
	tmpConv = t_ustr.conv;
if (tmpConv != NULL) {
     for (j=0;j<NUM_LINES;++j) {
	/* CYGNUS2 - do */
	 con += strlen(tmpConv->conv[j]);
	 }
}

tot_mem=
	strlen(t_ustr.name)
	+strlen(t_ustr.password)
	+strlen(t_ustr.desc)
	+strlen(t_ustr.email_addr)
	+strlen(t_ustr.sex)
	+strlen(t_ustr.site)
	+strlen(t_ustr.init_date)
	+strlen(t_ustr.init_site)
	+strlen(t_ustr.init_netname)
	+strlen(t_ustr.last_date)
	+strlen(t_ustr.last_site)
	+strlen(t_ustr.last_name)
	+strlen(t_ustr.say_name)
	+strlen(t_ustr.entermsg)
	+strlen(t_ustr.exitmsg)
	+strlen(t_ustr.succ)
	+strlen(t_ustr.fail)
	+strlen(t_ustr.homepage)
	+strlen(t_ustr.creation)
	+strlen(t_ustr.security)
	+strlen(t_ustr.login_name)
	+strlen(t_ustr.login_pass)
	+strlen(t_ustr.phone_user)
	+strlen(t_ustr.mutter)
	+strlen(t_ustr.page_file)
	+2 /* pro_enter */
	+2 /* roomd_enter */
	+2 /* vote_enter */
	+2 /* locked */
	+2 /* suspended */
	+2 /* area */
	+2 /* shout */
	+2 /* color */
	+2 /* clrmail */ 
	+2 /* sock */
	+2 /* monitor */
	+2 /* time */ 
	+2 /* vis */
	+2 /* super */
	+2 /* invite */
	+2 /* last_input */
	+2 /* warning_given */
	+2 /* logging_in */
	+2 /* attleft */
	+2 /* file_posn */
	+strlen(t_ustr.net_name)
	+macro /* all macros, since a list */
	+abbrev1 /* all abbreviations, since a list */
	+con   /* all conv buffers, since a list */
	+2 /* cat_mode */
	+2 /* rows */
	+2 /* cols */
	+2 /* car_return */
	+2 /* abbrs */
	+2 /* white_space */
	+2 /* line_count */
	+2 /* number_lines */
	+2 /* times_on */
	+2 /* afk */
	+2 /* lockafk */
	+2 /* upper */
	+2 /* lower */
	+2 /* aver */
	+4 /* totl */
	+2 /* autor */
	+2 /* autof */
	+2 /* automsgs */
	+2 /* gagcomm */
	+2 /* semail */
	+2 /* quote */
	+2 /* hilite */
	+2 /* new_mail */
	+4 /* numcoms */
	+2 /* mail_num */
	+2 /* numbering */
	+strlen(t_ustr.flags)
	+strlen(t_ustr.real_id)
	+1 /* attach_port */
	+2 /* char_buffer_size */
	+strlen(t_ustr.char_buffer)
	+2 /* friend_num */
	+2 /* revokes_num */
	+2 /* gag_num */
	+2 /* nerf_shots */
	+2 /* nerf_energy */
	+2 /* nerf_kills */
	+2 /* nerf_killed */
	+2 /* ttt_kills */
	+2 /* ttt_killed */
	+2 /* ttt_opponent */
	+2 /* ttt_playing */
	+strlen(t_ustr.icq)
	+strlen(t_ustr.miscstr1)
	+strlen(t_ustr.miscstr2)
	+strlen(t_ustr.miscstr3)
	+strlen(t_ustr.miscstr4)
	+2 /* pause_login */
	+2 /* miscnum2 */
	+2 /* miscnum3 */
	+2 /* miscnum4 */
	+2 /* miscnum5 */
	+2 /* tempnum1 */
	+2 /* term_type */
	+strlen(t_ustr.prev_term_type)
	+2 /* hang_wins */
	+2 /* hang_losses */
	+2 /* hang_stage */
	+strlen(t_ustr.hang_word)
	+strlen(t_ustr.hang_word_show)
	+strlen(t_ustr.hang_guess)
	+2 /* passhid */
	+2 /* pbreak */
	+2 /* beeps */
	+4 /* rawtime */
	+2 /* muz_time */
	+2 /* xco_time */
	+2 /* gag_time */
	+2 /* frog */
	+2 /* frog_time */
	+2 /* anchor */
	+2 /* anchor_time */
	+2 /* promote */
	+2 /* tempsuper */
	+strlen(t_ustr.home_room)
	+strlen(t_ustr.webpic)
	+strlen(t_ustr.afkmsg)
	+t_ustr.alloced_size;

       if (t_ustr.pro_enter)
         tot_mem += (82*PRO_LINES);
       else if (t_ustr.roomd_enter)
         tot_mem += (82*ROOM_DESC_LINES);
       else if (t_ustr.vote_enter)
         tot_mem += (82*VOTE_LINES);

	macro=0;
	con=0;
	i=0;
	j=0;

  return tot_mem;
}

long user_mem_get(int mode)
{
int u=0,i=0,j=0;
int macro=0,abbrev1=0,con=0;
int friends=0,gagged=0,revokes=0;
long userm=0;
long tot_mem=0;
ConvPtr tmpConv;
MacroPtr tmpMacros;

for (u=0;u<MAX_USERS;++u) {
     tmpMacros = ustr[u].Macros;
if (tmpMacros != NULL) {
     for (i=0;i<NUM_MACROS;++i) {
	 macro += MY_SIZE(tmpMacros->name[i],mode);
	 macro += MY_SIZE(tmpMacros->body[i],mode);
	 }
}
     for (i=0;i<NUM_ABBRS;++i) {
	 abbrev1 += MY_SIZE(ustr[u].custAbbrs[i].abbr,mode);
	 abbrev1 += MY_SIZE(ustr[u].custAbbrs[i].com,mode);
	 }
     for (i=0;i<MAX_ALERT;++i) friends += MY_SIZE(ustr[u].friends[i],mode);
     for (i=0;i<MAX_GAG;++i) gagged += MY_SIZE(ustr[u].gagged[i],mode);
     for (i=0;i<MAX_GRAVOKES;++i) revokes += MY_SIZE(ustr[u].revokes[i],mode);

	tmpConv = ustr[u].conv;
if (tmpConv != NULL) {
     for (j=0;j<NUM_LINES;++j) {
	/* CYGNUS2 */
	 con += MY_SIZE(tmpConv->conv[j],mode);
	 }
}

     userm=   
	MY_SIZE(ustr[u].name,mode)
	+MY_SIZE(ustr[u].password,mode)
	+MY_SIZE(ustr[u].desc,mode)
	+MY_SIZE(ustr[u].email_addr,mode)
	+MY_SIZE(ustr[u].sex,mode)
	+MY_SIZE(ustr[u].site,mode)
	+MY_SIZE(ustr[u].init_date,mode)
	+MY_SIZE(ustr[u].init_site,mode)
	+MY_SIZE(ustr[u].init_netname,mode)
	+MY_SIZE(ustr[u].last_date,mode)
	+MY_SIZE(ustr[u].last_site,mode)
	+MY_SIZE(ustr[u].last_name,mode)
	+MY_SIZE(ustr[u].say_name,mode)
	+MY_SIZE(ustr[u].entermsg,mode)
	+MY_SIZE(ustr[u].exitmsg,mode)
	+MY_SIZE(ustr[u].succ,mode)
	+MY_SIZE(ustr[u].fail,mode)
	+MY_SIZE(ustr[u].homepage,mode)
	+MY_SIZE(ustr[u].creation,mode)
	+MY_SIZE(ustr[u].security,mode)
	+MY_SIZE(ustr[u].login_name,mode)
	+MY_SIZE(ustr[u].login_pass,mode)
	+MY_SIZE(ustr[u].phone_user,mode)
	+MY_SIZE(ustr[u].mutter,mode)
	+MY_SIZE(ustr[u].page_file,mode)
	+2  /* pro_enter */
	+2  /* roomd_enter */
	+2  /* vote_enter */
	+2 /* locked */
	+2 /* suspended */
	+2 /* area */
	+2 /* shout */
	+2 /* color */
	+2 /* clrmail */ 
	+2 /* sock */
	+2 /* monitor */
	+2 /* time */ 
	+2 /* vis */
	+2 /* super */
	+2 /* invite */
	+2 /* last_input */
	+2 /* warning_given */
	+2 /* logging_in */
	+2 /* attleft */
	+2 /* file_posn */
	+MY_SIZE(ustr[u].net_name,mode)
	+macro
	+con
	+2 /* cat_mode */
	+2 /* rows */
	+2 /* cols */
	+2 /* car_return */
	+2 /* abbrs */
	+2 /* white_space */
	+2 /* line_count */
	+2 /* number_lines */
	+2 /* times_on */
	+2 /* afk */
	+2 /* lockafk */
	+2 /* upper */
	+2 /* lower */
	+2 /* aver */
	+4 /* totl */
	+2 /* autor */
	+2 /* autof */
	+2 /* automsgs */
	+2 /* gagcomm */
	+2 /* semail */
	+2 /* quote */
	+2 /* hilite */
	+2 /* new_mail */
	+4 /* numcoms */
	+2 /* mail_num */
	+2 /* numbering */
	+MY_SIZE(ustr[u].flags,mode)
	+MY_SIZE(ustr[u].real_id,mode)
	+1 /* attach_port */
	+2 /* char_buffer_size */
	+MY_SIZE(ustr[u].char_buffer,mode)
	+2 /* friend_num */
	+friends
	+2 /* gag_num */
	+gagged
	+2 /* revokes_num */
	+revokes
	+2 /* nerf_shots */
	+2 /* nerf_energy */
	+2 /* nerf_kills */
	+2 /* nerf_killed */
	+2 /* passhid */
	+2 /* pbreak */
	+2 /* beeps */
	+2 /* mailwarn */
	+4 /* rawtime */
	+2 /* muz_time */
	+2 /* xco_time */
	+2 /* gag_time */
	+2 /* frog */
	+2 /* frog_time */
	+2 /* promote */
	+MY_SIZE(ustr[u].home_room,mode)
	+MY_SIZE(ustr[u].afkmsg,mode)
	+MY_SIZE(ustr[u].last_name,mode)
	+abbrev1 /* all abbreviations, since a list */
	+MY_SIZE(ustr[u].say_name,mode)
	+2 /* help */
	+2 /* who */
	+2 /* ttt_kills */
	+2 /* ttt_killed */
	+2 /* ttt_board */
	+2 /* ttt_opponent */
	+2 /* ttt_playing */
	+2 /* hang_wins */
	+2 /* hang_losses */
	+2 /* hang_stage */
	+MY_SIZE(ustr[u].hang_word,mode)
	+MY_SIZE(ustr[u].hang_word_show,mode)
	+MY_SIZE(ustr[u].hang_guess,mode)
	+2 /* anchor */
	+2 /* anchor_time */
	+MY_SIZE(ustr[u].webpic,mode)
	+2 /* t_ent */
	+2 /* t_num */
	+MY_SIZE(ustr[u].t_name,mode)
	+MY_SIZE(ustr[u].t_host,mode)
	+MY_SIZE(ustr[u].t_ip,mode)
	+MY_SIZE(ustr[u].t_port,mode)
	+2 /* rwho */
	+2 /* needs_hostname */
	+2 /* tempsuper */
	+2 /* promptseq */
	+MY_SIZE(ustr[u].icq,mode)
	+MY_SIZE(ustr[u].miscstr1,mode)
	+MY_SIZE(ustr[u].miscstr2,mode)
	+MY_SIZE(ustr[u].miscstr3,mode)
	+MY_SIZE(ustr[u].miscstr4,mode)
	+2 /* pause_login */
	+2 /* miscnum2 */
	+2 /* miscnum3 */
	+2 /* miscnum4 */
	+2 /* miscnum5 */
	+2 /* write_offset */
	+ustr[u].alloced_size
	+2 /* log_stage */
	+MY_SIZE(ustr[u].temp_buffer,mode)
	+2 /* tempnum1 */
	+2 /* term_type */
	+MY_SIZE(ustr[u].prev_term_type,mode);

       if (ustr[u].pro_enter)
         userm += (82*PRO_LINES);
       else if (ustr[u].roomd_enter)
         userm += (82*ROOM_DESC_LINES);
       else if (ustr[u].vote_enter)
         userm += (82*VOTE_LINES);

       tot_mem += userm;
/* write_log(SYSTEMLOG,YESTIME,"Total Usage as of %d is %ld\n",u,tot_mem); */
       userm=0;
       macro=0;
       abbrev1=0;
       friends=0;
       gagged=0;
       revokes=0;
       con=0;
    }
 return tot_mem;
}

long area_mem_get(void)
{
int a=0;
int aream=0;
long tot_mem=0;

for (a=0;a<MAX_AREAS;++a) {
     aream=   
	strlen(astr[a].name)
	+strlen(astr[a].move)
	+strlen(astr[a].topic)
	+2  /* private   */
	+2  /* hidden    */
	+2  /* secure    */
	+2  /* mess_num  */
/* CYGNUS1 */
	+sizeof(astr[a].conv)  /* conv */
        +2; /* atmos     */

     tot_mem += aream;
     aream=0;
   }
 return tot_mem;
}

long conv_mem_get(int mode)
{
int i=0;
int j=0;
int conv_buf=0;
long tot_mem=0;
ConvPtr tmpConv;

     for (i=0;i<MAX_AREAS;++i) {
		tmpConv = astr[i].conv;
		if (tmpConv == NULL) continue;
          for (j=0;j<NUM_LINES;++j) {
             conv_buf += MY_SIZE(tmpConv->conv[j],mode);
             }

         tot_mem += conv_buf;
         conv_buf=0;
         j=0;
	}

conv_buf=0;
return tot_mem;
}

long wiz_mem_get(void)
{
int i=0;
long tot_mem=0;

     for (i=0;i<NUM_LINES;++i) {
             tot_mem += strlen(bt_conv[i]);
	 }

return tot_mem;
}

long shout_mem_get(void)
{
int i=0;
long tot_mem=0;

     for (i=0;i<NUM_LINES;++i) {
             tot_mem += strlen(sh_conv[i]);
	 }

return tot_mem;
}


/** Check on memory usage and allocations ***/
void check_mem(int user)
{
long tot1=0;
long tot2=0;
long mfree=0;
float per;

write_str(user,"+-----------------------------------------------------------------------+");
write_str(user,"|                          Memory Usage (in bytes)                      |");
write_str(user,"+-----------------------------------------------------------------------+");
write_str(user,"|                                                                       |");
write_str(user,"|  Type                Allocated      Used      Free     Capacity       |");

 per=((float)user_mem_get(0)/(float)user_mem_get(1))*100;
 mfree=user_mem_get(1)-user_mem_get(0);
 if (mfree < 0) mfree=0;
sprintf(mess,"|  User Data           %-6ld         %-6ld    %-6ld      %5.1f%%      |",
(long)user_mem_get(1),user_mem_get(0),mfree,per);
write_str(user,mess);
 per=0;
 mfree=0;
/* CYGNUS1 */
 per=((float)conv_mem_get(0)/(float)conv_mem_get(1))*100;
 mfree=conv_mem_get(1)-conv_mem_get(0);
 if (mfree < 0) mfree=0;
/* CYGNUS1 */
sprintf(mess,"|  Area Convo Bufs     %-6ld         %-6ld    %-6ld      %5.1f%%      |",
(long)conv_mem_get(1),conv_mem_get(0),mfree,per);
write_str(user,mess);
 per=0;
 mfree=0;
 per=((float)temp_mem_get()/(float)sizeof(t_ustr))*100;
 mfree=sizeof(t_ustr)-temp_mem_get();
 if (mfree < 0) mfree=0;
sprintf(mess,"|  Temp User Data      %-6ld         %-6ld    %-6ld      %5.1f%%      |",
(long)sizeof(t_ustr),temp_mem_get(),mfree,per);
write_str(user,mess);
 per=0;
 mfree=0;
 mfree=sizeof(astr)-area_mem_get();
 if (mfree < 0) mfree=0;
 per=((float)area_mem_get()/(float)sizeof(astr))*100;
sprintf(mess,"|  Area Data           %-6ld         %-6ld    %-6ld      %5.1f%%      |", 
(long)sizeof(astr),area_mem_get(),mfree,per);
write_str(user,mess);
 per=0;
 mfree=0;
 per=((float)wiz_mem_get()/(float)sizeof(bt_conv))*100;
 mfree=sizeof(bt_conv)-wiz_mem_get();
 if (mfree < 0) mfree=0;
sprintf(mess,"|  Wiz Convo Bufs      %-6ld         %-6ld    %-6ld      %5.1f%%      |",
(long)sizeof(bt_conv),wiz_mem_get(),mfree,per);
write_str(user,mess);
 per=0;
 mfree=0;
 per=((float)shout_mem_get()/(float)sizeof(sh_conv))*100;
 mfree=sizeof(sh_conv)-shout_mem_get();
 if (mfree < 0) mfree=0;
sprintf(mess,"|  Shout Convo Bufs    %-6ld         %-6ld    %-6ld      %5.1f%%      |",
(long)sizeof(sh_conv),shout_mem_get(),mfree,per);
write_str(user,mess);
 per=0;

 tot1=user_mem_get(1)+sizeof(astr)+sizeof(t_ustr)+conv_mem_get(1)+sizeof(bt_conv)+sizeof(sh_conv);
 tot2=user_mem_get(0)+area_mem_get()+temp_mem_get()+conv_mem_get(0)+wiz_mem_get()+shout_mem_get();

write_str(user,"|                                                                       |");
 per=((float)tot2/(float)tot1)*100;
 mfree=0;
 mfree=tot1-tot2;
 if (mfree < 0) mfree=0;

sprintf(mess,"|   TOTALS             %-6lu         %-6lu    %-6lu      %5.1f%%      |", tot1,tot2,mfree,per);
 per=0;
 mfree=0;
write_str(user,mess);
write_str(user,"+-----------------------------------------------------------------------+");

 tot1=0;
 tot2=0;
 per=0;
 mfree=0;

}


/***        Add an atmosphere to the list        ***/
/* V 1.2 adds numerical sorting of probabilities   */
void add_atmos(int user, char *inpstr)
{
int i=1, warn=0, lim=0, fprob, probint;
char filename[FILE_NAME_LEN], filename2[FILE_NAME_LEN];
char temp[ATMOS_LEN+11];
char temp2[ARR_SIZE];
FILE *fp, *fp2;

sprintf(filename, "%s/%s.atmos",datadir, astr[ustr[user].area].name);

sscanf(inpstr,"%s ",temp2);
if (!temp2[0]) {
	write_str(user, "Usage: .addatmos <chance to occur (1-99)> <atmosphere>");
	return;
	}
strcpy(temp2, strip_color(temp2));
probint=atoi(temp2);
if ((probint<1) || (probint>99)) {
	write_str(user, "The chance factor must be between 1 and 99");
	return;
	}
remove_first(inpstr);
if (!inpstr[0]) {
	write_str(user, "Usage: .addatmos <chance to occur (1-99)> <atmosphere>");
	return;
	}
/* inpstr now points to atmosphere string - need to truncate it if too long */
if (strlen(inpstr)>ATMOS_LEN) inpstr[ATMOS_LEN]=0;

if ((fp=fopen(filename, "r"))) {
	/* count atmospheres */
	fgets(temp, 80, fp);
	while(!feof(fp)) {
		fgets(temp, ATMOS_LEN+10, fp);
		i++;
		fgets(temp, 80, fp);
		}
	fclose(fp);
	if (i>MAX_ATMOS) {
                sprintf(mess,"You already have the maximum of %d atmospheres - you must delete one first",MAX_ATMOS);
                write_str(user,mess);
		return;
		}
	}

if ((fp=fopen(filename, "r"))) {
  strcpy(filename2, get_temp_file());
  if (!(fp2=fopen(filename2, "w"))) {
        write_str(user,BAD_FILEIO);
	write_log(ERRLOG,YESTIME,"Couldn't open tempfile(w) in add_atmos! %s\n",get_error());
	return;
	}
  
        /* get prob */
        temp[0]=0;
	fgets(temp, 80, fp);
        fprob=atoi(temp);
        if (probint<=fprob) {
          if (probint==fprob) warn=1;
          fprintf(fp2, "%d\n%s\n", probint, inpstr);
          lim=0;
          goto END;
          }

	while(!feof(fp)) {
                if (!lim) {
                temp[0]=0;
		fgets(temp, ATMOS_LEN+10, fp);
                fprintf(fp2, "%d\n%s", fprob, temp);
                lim=1;
                }
                else {
                /* get probability */
                temp[0]=0;
		fgets(temp, 80, fp);
                lim=0;
	        fprob=atoi(temp);
                if (probint<=fprob) {
                  if (probint==fprob) warn=1;
                  lim=0;
                  fprintf(fp2, "%d\n%s\n", probint, inpstr);
                  goto END;
                  }
                } /* end of else */
            } /* end of while */
        
        /* its greater than everything else, so just append it */
        fprintf(fp2, "%d\n%s\n", probint, inpstr);     
        goto THEEND;

END:
        /* write out probability */
        fprintf(fp2, "%d\n", fprob);

	while(!feof(fp)) {
               if (!lim) {
                temp[0]=0;
		fgets(temp, ATMOS_LEN+10, fp);
                fputs(temp, fp2);
                lim=1;
                }
               else {
                temp[0]=0;
		fgets(temp, 80, fp);
                fputs(temp, fp2);
                lim=0;
                }
           } /* end of while */

THEEND:
       fclose(fp);
       fclose(fp2);
       if (rename(filename2, filename)==-1) {
        write_str(user,BAD_FILEIO);
	write_log(ERRLOG,YESTIME,"Couldn't rename file \"%s\" to \"%s\" in add_atmos! %s\n",filename2,filename,get_error());
	return;
	}
      } /* end of file open if */
else {
  /* No atmosphere file exists..create one with new atmosphere */
  if (!(fp=fopen(filename, "a"))) {
        write_str(user,BAD_FILEIO);
	write_log(ERRLOG,YESTIME,"Couldn't open file(a) \"%s\" in add_atmos! %s\n",filename,get_error());
	return;
	}

  fprintf(fp, "%d\n%s\n", probint, inpstr);
  fclose(fp);
 } /* end of else */

if (i==1) strcpy(mess, "You now have 1 atmosphere");
else sprintf(mess, "You now have %d atmospheres", i);
write_str(user, mess);

if (warn)
 write_str(user,"WARNING: Two distinct atmospheres have the same probability number. Doing this disregards the latter since a random number will match them equally.");

write_log(SYSTEMLOG,YESTIME,"ATMOS: added to area %s by %s\n",astr[ustr[user].area].name,ustr[user].say_name);
}


/*** Remove an atmosphere from the list ***/
void del_atmos(int user, char *inpstr)
{
char filename[FILE_NAME_LEN], tempfile[FILE_NAME_LEN];
char atmos[ATMOS_LEN+11], probch[81];
FILE *fp, *dp;
int atm_num, i=1, wrote=0;

sprintf(filename, "%s/%s.atmos",datadir, astr[ustr[user].area].name);
strcpy(tempfile,get_temp_file());

if (!inpstr[0]) {
	write_str(user, "Usage: .delatmos <atmosphere number>");
	write_str(user, "       .delatmos all");
	return;
	}

if (!strcmp(inpstr,"all")) {
   remove(filename);
   write_str(user,"All atmospheres have been deleted for this room.");
   return;
   }

atm_num=atoi(inpstr);
if ((atm_num<1) || (atm_num>MAX_ATMOS)) {
        sprintf(mess,"Atmosphere number must be between 1 and %d",MAX_ATMOS);
	write_str(user,mess);
	return;
	}

if (!(fp=fopen(filename,"r"))) {
	write_str(user, "You have no atmospheres at the moment");
	return;
	}

if (!(dp=fopen(tempfile, "w"))) {
        write_str(user,BAD_FILEIO);
	write_log(ERRLOG,YESTIME,"Couldn't open tempfile(w) in del_atmos! %s\n",get_error());
	fclose(dp);  return;
	}

fgets(probch, 80, fp);
while(!feof(fp)) {
	fgets(atmos, ATMOS_LEN+10, fp);
	if (i!=atm_num) {
		fputs(probch, dp);
		fputs(atmos, dp);
                wrote=1;
		}
	i++;
	fgets(probch, 80, fp);
	}

fclose(fp);
fclose(dp);

if (wrote) {
  /* Make the temp file the new atmospheres file for the user */
  if (rename(tempfile, filename)==-1) {
        write_str(user,BAD_FILEIO);
	write_log(ERRLOG,YESTIME,"Couldn't rename file \"%s\" to \"%s\" in del_atmos! %s\n",tempfile,filename,get_error());
	return;
	}
  }
else {
  remove(filename);
  remove(tempfile);
  }

if (atm_num>=i) {
sprintf(mess, "There is no atmosphere %d", atm_num);
write_str(user, mess);
}
else {
sprintf(mess, "Atmosphere %d deleted", atm_num);
write_str(user, mess);
write_log(SYSTEMLOG,YESTIME,"ATMOS: deleted #%d from area %s by %s\n",atm_num,astr[ustr[user].area].name,ustr[user].say_name);
}
}



/*** List the current atmospheres ***/
void list_atmos(int user)
{
char filename[80], atmos[ATMOS_LEN+11], probch[81];
FILE *fp;
int probint, i=1;

sprintf(filename, "%s/%s.atmos",datadir, astr[ustr[user].area].name);
if (!(fp=fopen(filename,"r"))) {
	write_str(user, "This room has no atmospheres at the moment");
	return;
	}

/* probint is num. between 0 - 100 (hopefully) */
fgets(probch, 80, fp);
write_str(user, "--------------------------------------------------");
write_str(user, "Room atmospheres are:     (a @ denotes a new line)");
write_str(user, "--------------------------------------------------");
while(!feof(fp)) {
	probint=atoi(probch);
	fgets(atmos, ATMOS_LEN+10, fp);
	atmos[strlen(atmos)-1]=0;
	sprintf(mess, "(%d) %2d%% %s", i, probint, atmos);
	write_str(user, mess);
	i++;
	fgets(probch, 80, fp);
	}
write_str(user, "--------------------------------------------------");
fclose(fp);
}


/*** Let a user delete his or her account ***/
void suicide_user(int user, char *inpstr)
{
char nuke_name[NAME_LEN+1];
int new_pos = 0;

/* Demotion check. If user was demoted to a 0 and they were */
/* promoted at the beginning, dont let them suicide         */
/* Doing so would mean they could just make a new character */
/* with the same name and auto-promote themselves           */
/*  Thanx to Jazzin for pointing this out.                  */
if (ustr[user].super==0 && ustr[user].promote==1) {
    write_str(user,"Nope. Sorry, we cant let you do that.");
    return;
   }

if (!strlen(inpstr)) {
    write_str(user,"To delete your account, you must enter your password after the command.");
    return;
    }

strtolower(inpstr);
st_crypt(inpstr);

if (strcmp(ustr[user].password,inpstr)) {
   write_str(user,NO_MATCH_SUIC);
   return;
   }

strcpy(nuke_name,ustr[user].name);
strtolower(nuke_name);
write_str(user,"");
write_str(user,"Quitting you and deleting this account..");
write_str(user,"");

sprintf(mess,"%s SUICIDE: By user %s",STAFF_PREFIX,strip_color(ustr[user].say_name));
writeall_str(mess, WIZ_ONLY, user, 0, user, BOLD, WIZT, 0);

strncpy(bt_conv[bt_count],mess,MAX_LINE_LEN);
new_pos = ( ++bt_count ) % NUM_LINES;
bt_count = new_pos;

write_log(SYSTEMLOG,YESTIME,"SUICIDE: By user %s\n",ustr[user].say_name);

user_quit(user,1);
 
remove_exem_data(nuke_name);
remove_user(nuke_name);

}


/** Gagcomm a user, takes away their capability **/
/** for all private communication commands      **/
void gag_comm(int user, char *inpstr, int type)
{
char other_user[ARR_SIZE];
int u,inlen;
unsigned int i;

if (!strlen(inpstr)) 
  {
   write_str(user,"Users GCommed & logged on     Time left"); 
   write_str(user,"-------------------------     ---------"); 
   for (u=0; u<MAX_USERS; ++u) 
    {
     if (ustr[u].gagcomm == 1 && ustr[u].area > -1) 
       {
        if (ustr[u].gag_time == 0)
           sprintf(mess,"%-*s %s",NAME_LEN+count_color(ustr[u].say_name,0),ustr[u].say_name,"Perm");
        else
           sprintf(mess,"%-*s %s",NAME_LEN+count_color(ustr[u].say_name,0),ustr[u].say_name,converttime((long)ustr[u].gag_time));
        write_str(user, mess);
       }
    }
   write_str(user,"(end of list)");
   return;
  }

sscanf(inpstr,"%s ",other_user);
strtolower(other_user);

if ((u=get_user_num(other_user,user))== -1) 
  {
   not_signed_on(user,other_user);
   return;
  }
if (u == user)
  {   
   write_str(user,"You are definitly wierd! Trying to gagcomm yourself, geesh."); 
   return;
  }

 if ((!strcmp(ustr[u].name,ROOT_ID)) || (!strcmp(ustr[u].name,BOT_ID)
      && strcmp(ustr[user].name,ROOT_ID))) {
    write_str(user,"Yeah, right!");
    return;
    }
    
if ((ustr[user].tempsuper <= ustr[u].super) && strcmp(ustr[user].name,ROOT_ID)) 
  {
   write_str(user,"That would not be wise...");
   sprintf(mess,GCOMM_CANT,ustr[user].say_name);
   write_str(u,mess);
   return;
  }

if (type >= 1) goto ARREST;

if (ustr[u].gagcomm == 0) {
  remove_first(inpstr);
if (strlen(inpstr) && strcmp(inpstr,"0")) {
   if (strlen(inpstr) > 5) {
      write_str(user,"Minutes cant exceed 5 digits.");
      return;
      }
   inlen=strlen(inpstr);
   for (i=0;i<inlen;++i) {
     if (!isdigit((int)inpstr[i])) {
        write_str(user,"Numbers only!");
        return;
        }
     }
    i=0;
    i=atoi(inpstr);
    if ( i > 32767) {
       write_str(user,"Minutes cant exceed 32767.");
       i=0;
       return;
      }
  i=0;
  ustr[u].gag_time=atoi(inpstr);
  ustr[u].gagcomm = 1;
    write_str(u,GCOMMON_MESS);
    sprintf(mess,"GCOM: ON for %s by %s for %s\n",ustr[u].say_name,
ustr[user].say_name, converttime((long)ustr[u].gag_time));
}
 else {
   ustr[u].gagcomm = 1;
   ustr[u].gag_time= 0;
   write_str(u,GCOMMON_MESS);
   sprintf(mess,"GCOM: ON for %s by %s\n",ustr[u].say_name, ustr[user].say_name);
  }
 btell(user, mess);
 write_log(SYSTEMLOG,YESTIME,mess);
 return;
} /* end of if gagcommed */

else {
    ustr[u].gagcomm = 0;
    ustr[u].gag_time = 0;
    write_str(u,GCOMMOFF_MESS);
    sprintf(mess,"GCOM: OFF for %s by %s",ustr[u].say_name, ustr[user].say_name);
    btell(user, mess);
    write_log(SYSTEMLOG,YESTIME,mess);
    return;
  } 


ARREST:
if (type==1)
 ustr[u].gagcomm=1;
else
 ustr[u].gagcomm=0;
ustr[u].gag_time=0;

if (type==1) {
write_str(u,GCOMMON_MESS);
sprintf(mess,"GCOM: ON for %s by %s\n",ustr[u].say_name, ustr[user].say_name);
}
else {
write_str(u,GCOMMOFF_MESS);
sprintf(mess,"GCOM: OFF for %s by %s\n",ustr[u].say_name, ustr[user].say_name);
}
btell(user, mess);
write_log(SYSTEMLOG,YESTIME,mess);

}

/*-----------------------------------------------*/
/* frog a user                                   */
/*-----------------------------------------------*/
void frog_user(int user, char *inpstr)
{
char other_user[ARR_SIZE];
int u,inlen;
unsigned int i;

if (!strlen(inpstr)) 
  {
   write_str(user,"Users Frogged & logged on     Time left"); 
   write_str(user,"-------------------------     ---------"); 
   for (u=0;u<MAX_USERS;++u) 
    {
     if (ustr[u].frog  && ustr[u].area > -1) 
       {
        if (ustr[u].frog_time == 0)
           sprintf(mess,"%-*s %s",NAME_LEN+count_color(ustr[u].say_name,0),ustr[u].say_name,"Perm");
        else
           sprintf(mess,"%-*s %s",NAME_LEN+count_color(ustr[u].say_name,0),ustr[u].say_name,converttime((long)ustr[u].frog_time));
        write_str(user, mess);
       };
    }
   write_str(user,"(end of list)");
   return;
  }

sscanf(inpstr,"%s ",other_user);
strtolower(other_user);

if ((u=get_user_num(other_user,user))== -1) 
  {
   not_signed_on(user,other_user);
   return;
  }
  
if (u == user)
  {   
   write_str(user,"You are definitly wierd! Trying to frog yourself, geesh."); 
   return;
  }

 if ((!strcmp(ustr[u].name,ROOT_ID)) || (!strcmp(ustr[u].name,BOT_ID)
      && strcmp(ustr[user].name,ROOT_ID))) {
    write_str(user,"Yeah, right!");
    return;
    }

if ((ustr[user].tempsuper <= ustr[u].super) && strcmp(ustr[user].name,ROOT_ID)) 
  {
   write_str(user,"That would not be wise...");
   sprintf(mess,FROG_CANT,ustr[user].say_name);
   write_str(u,mess);
   return;
  }

if (ustr[u].frog == 0) {
remove_first(inpstr);
if (strlen(inpstr) && strcmp(inpstr,"0")) {
   if (strlen(inpstr) > 5) {
      write_str(user,"Minutes cant exceed 5 digits.");
      return;
      }
   inlen=strlen(inpstr);
   for (i=0;i<inlen;++i) {
     if (!isdigit((int)inpstr[i])) {
        write_str(user,"Numbers only!");
        return;
        }
     }
    i=0;
    i=atoi(inpstr);
    if ( i > 32767) {
       write_str(user,"Minutes cant exceed 32767.");
       i=0;
       return;
      }
  i=0;
  ustr[u].frog_time=atoi(inpstr);
  ustr[u].frog = 1;
    write_str(u,FROGON_MESS);
    sprintf(mess,"FROG: ON for %s by %s for %s\n",ustr[u].say_name,
ustr[user].say_name, converttime((long)ustr[u].frog_time));
}
else {
    ustr[u].frog = 1;
    ustr[u].frog_time=0;
    write_str(u,FROGON_MESS);
    sprintf(mess,"FROG: ON for %s by %s\n",ustr[u].say_name, ustr[user].say_name);
  }
}     /* end of if frogged */

else {
    ustr[u].frog = 0;
    ustr[u].frog_time=0;
    write_str(u,FROGOFF_MESS);
    sprintf(mess,"FROG: OFF for %s by %s\n",ustr[u].say_name, ustr[user].say_name);
  } 

btell(user, mess);
write_log(SYSTEMLOG,YESTIME,mess);

write_str(user,"Ok");
}

/* Set talkers auto-nuke flag on or off */
void auto_nuke(int user)
{
char line[132];

  if (autonuke)
    {
      write_str(user,"Auto-nuke DISABLED");
      autonuke=0;  
      sprintf(line,"Auto-nuke DISABLED by %s\n",strip_color(ustr[user].say_name));
    }
   else
    {
      write_str(user,"Auto-nuke ENABLED");
      autonuke=1;  
      sprintf(line,"Auto-nuke ENABLED by %s\n",strip_color(ustr[user].say_name));
    }
    
 btell(user,line);
 write_log(SYSTEMLOG,YESTIME,line);
}

/* Jazzin - makes .autopromote .autonuke and .autoexpire into one command */
void auto_com(int user, char *inpstr)
{
char option[ARR_SIZE];
char onoff[2][4];

if (!strlen(inpstr))
  {
   write_str(user,"To use .auto you have to specify which auto-setting you want.");
   write_str(user," .auto nuke      Will turn on/off the auto-nuke flag");
   write_str(user," .auto expire    Will turn on/off the auto-expire flag");
   write_str(user," .auto promote   Will turn on/off the auto-promotion flag");
   write_str(user," .auto settings  Lets you see what they are set at currently");
   write_str(user,"");
   return;
  }

sscanf(inpstr,"%s",option);
strtolower(option);
strcpy(onoff[0],"OFF");
strcpy(onoff[1],"ON");
   if (!strcmp(option,"nuke")) 
     {
      auto_nuke(user);
      return;
     }
    else if (!strcmp(option,"expire"))
     {
      auto_expr(user);
      return;
     }
   else if (!strcmp(option,"promote"))
     {
      auto_prom(user);
      return;
     }
   else if (!strcmp(option,"settings"))
     {
      write_str(user,"----------------------------------------------------");
      sprintf(mess,"Auto-promote : %s",onoff[autopromote]);
      write_str(user,mess);
      sprintf(mess,"Auto-nuke    : %s",onoff[autonuke]);
      write_str(user,mess);
      if (autoexpire==0)
      write_str(user,"Auto-expire  : Expiring DISABLED, Warnings DISABLED");
      else if (autoexpire==1)
      write_str(user,"Auto-expire  : Expiring DISABLED, Warnings ENABLED");
      else if (autoexpire==2)
      write_str(user,"Auto-expire  : Expiring ENABLED, Warnings ENABLED");
      else if (autoexpire==3)
      write_str(user,"Auto-expire  : Expiring ENABLED, Warnings DISABLED");
      write_str(user,"----------------------------------------------------");
      return;
    } 
  else
    {
   write_str(user,"Option Unkown.");
   write_str(user,"To use .auto you have to specify which auto-setting you want.");
   write_str(user," .auto nuke     Will turn on/off the auto-nuke flag");
   write_str(user," .auto expire   Will turn on/off the auto-expire flag");
   write_str(user," .auto promote  Will turn on/off the auto-promotion flag");
   write_str(user," .auto settings  Lets you see what they are set at currently");
   write_str(user,"");
   }

}

/* Set talkers auto-promote flag on or off */
void auto_prom(int user)
{
char line[132];

  if (autopromote==1)
    {
      write_str(user,"Auto-promote DISABLED");
      autopromote=0;  
      sprintf(line,"Auto-promote DISABLED by %s\n",strip_color(ustr[user].say_name));
    }
   else
    {
      write_str(user,"Auto-promote ENABLED");
      autopromote=1;
      sprintf(line,"Auto-promote ENABLED by %s\n",strip_color(ustr[user].say_name));
    }

 btell(user,line);
 write_log(SYSTEMLOG,YESTIME,line);
}

/* Set talkers auto-promote flag on or off */
void auto_expr(int user)
{
char line[132];

  if (autoexpire==0)
    {
      write_str(user,"Auto-expiring DISABLED, warnings ENABLED");
      autoexpire=1;
      sprintf(line,"Auto-expiring DISABLED, warnings ENABLED  by %s\n",strip_color(ustr[user].say_name));
    }
  else if (autoexpire==1)
    {
      write_str(user,"Auto-expiring ENABLED, warnings ENABLED");
      autoexpire=2;
      sprintf(line,"Auto-expiring ENABLED, warnings ENABLED by %s\n",strip_color(ustr[user].say_name));
    }
  else if (autoexpire==2)
    {
      write_str(user,"Auto-expiring ENABLED, warnings DISABLED");
      autoexpire=3;
      sprintf(line,"Auto-expiring ENABLED, warnings DISABLED by %s\n",strip_color(ustr[user].say_name));
    }
  else if (autoexpire==3)
    {
      write_str(user,"Auto-expiring DISABLED, warnings DISABLED");
      autoexpire=0;
      sprintf(line,"Auto-expiring DISABLED, warnings DISABLED by %s\n",strip_color(ustr[user].say_name));
    }
    
 btell(user,line);
 write_log(SYSTEMLOG,YESTIME,line);
}


/** Eight ball command..I promised smoothie i would do this **/
void eight_ball(int user, char *inpstr)
{
int i;

if (ustr[user].area==INIT_ROOM) {
  write_str(user,"The oracle does not respond well to public places");
  write_str(user,"Try somewhere less public.");
  return;
  }

if (!strlen(inpstr)) {
  write_str(user,"Your thoughts have floated away. Try thinking harder!");
  return;
  }

/* User is writing gibberish */
/* They might still be writing gibberish after this check */
/* but it's the best I can do                             */
if (!strstr(inpstr," ") || !strstr(inpstr,"?")) {
  write_str(user,"Your thoughts are still unclear to me.");
  write_str(user,"Please put your thoughts into the form of a quesiton.");
  return;
  }

i = rand() % NUM_BALL_LINES;

inpstr[0]=toupper((int)inpstr[0]);
sprintf(mess,"%s asks the all knowing Guru of %s: %s",ustr[user].say_name,SYSTEM_NAME,inpstr);
write_str(user,mess);
writeall_str(mess,1,user,0,user,NORM,SAY_TYPE,0);

sprintf(mess,"The Guru looks up from his deep trance and says: %s",
              ball_text[i]);
write_str(user,mess);
write_str(user,"");
writeall_str(mess,1,user,0,user,NORM,SAY_TYPE,0);
strcpy(mess,"");
writeall_str(mess,1,user,0,user,NORM,SAY_TYPE,0);

}

/* Add warning log for a user */
void warning(int user, char *inpstr, int mode)
{
char timestr[30];
char other_user2[ARR_SIZE];
char z_mess[ARR_SIZE+45];
char filename[FILE_NAME_LEN];
FILE *fp;
time_t tm;

if (ustr[user].tempsuper >= WIZ_LEVEL) {

  if (!strlen(inpstr)) {
   write_str(user,"Whose warning log do you want to search?");
   return;
   }

sscanf(inpstr,"%s ",other_user2);
other_user2[80]=0;

/* plug security hole */
if (check_fname(other_user2,user)) 
  {
   write_str(user,"Illegal name.");
   return;
  }

 } /* end of it*/
else {
 strcpy(other_user2,ustr[user].name);
 inpstr[0]=0;
 }


strtolower(other_user2);
remove_first(inpstr);

if (!strlen(inpstr)) {
   sprintf(filename,"%s/%s",WLOGDIR,other_user2);
   if (!check_for_file(filename)) {
     write_str(user,"No log on that user!");
     return;
     }
   write_str(user,"^HG***^ ^HYWarning log^ ^HG***^");
   cat(filename,user,0);
  }
else {
   if (!check_for_user(other_user2)) {
     write_str(user,NO_USER_STR);
     return;
     }

   sprintf(filename,"%s/%s",WLOGDIR,other_user2);

   if (!(fp=fopen(filename,"a"))) {
     write_str(user,BAD_FILEIO);
     write_log(ERRLOG,YESTIME,"Couldn't open file(a) \"%s\" in warning! %s\n",filename,get_error());
     return;
    }

   time(&tm);
   strcpy(timestr,ctime(&tm));
   midcpy(timestr,timestr,4,15);

   if (strlen(inpstr) > REASON_LEN) {
      inpstr[REASON_LEN]=0;
      write_str(user,"Reason too long..truncated.");
     }

   z_mess[0]=0;
   sprintf(z_mess,"(%s) From %s: %s\n",timestr,ustr[user].say_name,inpstr);
   fputs(z_mess,fp);
   fclose(fp);
   if (mode==1) {
   sprintf(mess,"logged a warning about %s",other_user2);
   btell(user,mess);
   write_log(SYSTEMLOG,YESTIME,"UWARNING: %s %s\n",ustr[user].say_name,mess);
   }
 }
}


/*** invite someone into private room ***/
void invite_user(int user, char *inpstr)
{
int u,area=ustr[user].area;
char other_user[ARR_SIZE];

if (!astr[area].private) {
	write_str(user,"The area is public anyway");  return;
	}
if (!strlen(inpstr)) {
	write_str(user,"Invite who?");  return;
	}
sscanf(inpstr,"%s ",other_user);
strtolower(other_user);

/* see if other user exists */
if ((u=get_user_num(other_user,user))== -1) {
	not_signed_on(user,other_user);
        return;
       }

if (!strcmp(other_user,ustr[user].name)) {
	write_str(user,"You cannot invite yourself!");  return;
	}
if (ustr[u].area==ustr[user].area) {
	sprintf(mess,"%s is already in the room!",ustr[u].say_name);
	write_str(user,mess);
	return;
	}

if (ustr[u].pro_enter || ustr[u].vote_enter || ustr[u].roomd_enter) {
    write_str(user,IS_ENTERING);
    return;
    }

write_str(user,"Ok");
sprintf(mess,"%s has invited you to the %s",ustr[user].say_name,astr[area].name);
if (!ustr[user].vis) 
	sprintf(mess,"%s has invited you to the %s",INVIS_ACTION_LABEL,astr[area].name);
write_str(u,mess);
ustr[u].invite=area;
}


/*-------------------------------------------*/
/* save message to room message board file */
/*-------------------------------------------*/
void write_board(int user, char *inpstr, int mode)
{
FILE *fp;
char stm[20],filename[FILE_NAME_LEN],name[SAYNAME_LEN];
time_t tm;

/* process wiz notes */
if (mode == 1)
  {
   if (!strlen(inpstr))
     {
      read_board(user,1,"");
      return;
     } 
   if ((strlen(inpstr) < 3) && (strlen(inpstr) > 0)) {
      if (isdigit((int)inpstr[0])) {
        read_board(user,1,inpstr);
        return;
        }
    }
   sprintf(t_mess,"%s/wizmess",MESSDIR);
  }
else if (mode == 2)
  {
   if (!strlen(inpstr))
     {
      if (ustr[user].tempsuper < GRIPE_LEVEL) {
        write_str(user,"^You cant read the gripe board.^");
        return;
        }
      else {
        read_board(user,2,"");
        return;
        }
     }
   if ((strlen(inpstr) < 3) && (strlen(inpstr) > 0)) {
      if (ustr[user].tempsuper < GRIPE_LEVEL) {
        write_str(user,"^You cant read the gripe board.^");
        return;
        }
      if (isdigit((int)inpstr[0])) {
        read_board(user,2,inpstr);
        return;
        }
    }
   sprintf(t_mess,"%s/gripes",MESSDIR);
  }
else if (mode == 3) {
   if (!strlen(inpstr))
     {
      read_board(user,3,"");
      return;
     } 
   if ((strlen(inpstr) < 3) && (strlen(inpstr) > 0)) {
      if (isdigit((int)inpstr[0])) {
        read_board(user,3,inpstr);
        return;
        }
    }
   sprintf(t_mess,"%s/suggs",MESSDIR);
  }
 else
  {
   sprintf(t_mess,"%s/board%d",MESSDIR,ustr[user].area);
  }
  
if (!strlen(inpstr)) 
  {
   write_str(user,"You forgot the message"); return;
  }

if (ustr[user].frog) {
   write_str(user,"Frogs cant write, silly.");
   return;
   }

/* open board file */
strncpy(filename,t_mess,FILE_NAME_LEN);

if (!(fp=fopen(filename,"a"))) 
  {
   write_str(user,BAD_FILEIO);
   write_log(ERRLOG,YESTIME,"Couldn't open file(a) \"%s\" in write_board! %s\n",filename,get_error());
   return;
  }

/* write message - alter nums. in midcpy to suit */
time(&tm);
midcpy(ctime(&tm),stm,4,15);
strcpy(name,ustr[user].say_name);


if (mode == 2)
 sprintf(mess,"(%s) %s gripes: %s\n",stm,name,inpstr);
else if (mode == 3) {
 if (ANON_SUGGEST)
  sprintf(mess,"Someone suggests: %s\n",inpstr);
 else
  sprintf(mess,"%s suggests: %s\n",name,inpstr);
 }
else 
 sprintf(mess,"(%s) From %s: %s\n",stm,name,inpstr);
fputs(mess,fp);
FCLOSE(fp);

/* send output */
if (mode == 2)
 write_str(user,"Your gripe is duely noted.");
else if (mode == 3)
 write_str(user,"Suggestion written. Thank you.");
else 
 write_str(user,USER_WRITE);

if (mode == 0)
  {
   sprintf(mess,OTHER_WRITE,ustr[user].say_name);

   if (!ustr[user].vis) 
	sprintf(mess,INVIS_WRITE);

   writeall_str(mess, 1, user, 0, user, NORM, MESSAGE, 0);
   astr[ustr[user].area].mess_num++;
  }
 else if ((mode == 2) || (mode == 3))
  {
  }
 else
  {
   strcpy(mess,"added a wiz note.");
   btell(user,mess);
  }
}


/*** read the message board  ***/
void read_board(int user, int mode, char *inpstr)
{
char filename[FILE_NAME_LEN];
char filename2[FILE_NAME_LEN];
char name_area[ARR_SIZE];
char junk[1001];
int number = 0;
int area, found, new_area,a=0;
int num_lines=0;
int lines=0;
int b=0;
FILE *fp;
FILE *tfp;

/* send output to user */
area = ustr[user].area;

if (mode==1)
 {
  sprintf(mess, "*** The wizards note board (important info only, non-wipeable) ***");
  sprintf(filename,"%s/wizmess",MESSDIR);
  goto WIZ;
 }
else if (mode==2)
 {
  sprintf(mess, "*** User gripes ***");
  sprintf(filename,"%s/gripes",MESSDIR);
  goto WIZ;
 }
else if (mode==3)
 {
  sprintf(mess,"^HG***^^HYReading suggestion board^^HG***^");
  sprintf(filename,"%s/suggs",MESSDIR);
  goto WIZ;
 }
else
 {
  if (!strlen(inpstr)) {
   if (astr[area].hidden && ((ustr[user].tempsuper < GRIPE_LEVEL)
	|| (ustr[user].security[area]!='Y')))
    {
     write_str(user,"^Secured message board, read not allowed.^");
     return;
    }
  
  if (astr[area].hidden)
    sprintf(mess,"** A secured message board **");
   else
    sprintf(mess,"** The %s message board **",astr[area].name);
 }

if (strlen(inpstr)) {
   if (isalpha((int)inpstr[0])) {
       if (strlen(inpstr) > 20) {
              write_str(user,"Room name is too long");  return;
              }
       name_area[0]=0;
       sscanf(inpstr,"%s ",name_area);
       remove_first(inpstr);        
        }   
   else {  name_area[0]=0; }
  }
  else {
        name_area[0]=0;
       }

if ((strlen(inpstr) < 3) && (strlen(inpstr) > 0)) {
      num_lines=atoi(inpstr);
      a=1;
      }

   if (strlen(name_area))               
     {
      found = FALSE;
      for (new_area=0; new_area < NUM_AREAS; ++new_area)
       { 
        if (! instr2(0, astr[new_area].name, name_area, 0) )         
          { 
            found = TRUE;
            area = new_area;
	   if (astr[new_area].hidden && ((ustr[user].tempsuper < GRIPE_LEVEL)
		|| (ustr[user].security[new_area]!='Y')))
             {
              write_str(user,"^Secured message board, read not allowed.^");
              return;
             }
           if (astr[new_area].hidden)
            sprintf(mess,"** A secured message board **");
           else
            sprintf(mess,"***Reading message board in %s***",astr[new_area].name);
            break;
          }
       }
 
      if (!found)
        {
         write_str(user, NO_ROOM);
         return;
        }
     }

  /* If number given but no area name */
  else {
           if (astr[area].hidden)
            sprintf(mess,"** A secured message board **");
           else
            sprintf(mess,"***Reading message board in %s***",astr[area].name);
       }
  
   sprintf(filename,"%s/board%d",MESSDIR,area);

 }  /* End of main else */


WIZ:
write_str(user," ");
write_str(user,mess);
write_str(user," ");

if (ustr[user].tempsuper >= LINENUM_LEVEL) number = 1;

if ((strlen(inpstr) < 3) && (strlen(inpstr) > 0)) {
      num_lines=atoi(inpstr);
      a=1;
      }

if (a==1) {
       if (num_lines < 1) {
           if (!cat(filename,user,number))
               write_str(user,"^There are no messages on the board.^");
               goto DONE;
               }
lines = file_count_lines(filename);
if (num_lines <= lines) {
    ustr[user].numbering = (lines - num_lines) +1;
    }
 else {
      num_lines=lines;
      }
num_lines = lines - num_lines;

if (!(fp=fopen(filename,"r"))) {
      write_str(user,"^There are no messages on the board.^");
      num_lines=0;  a=0;  lines=0;
      ustr[user].numbering = 0;
      return;
      }

strcpy(filename2,get_temp_file());
tfp=fopen(filename2,"w");

while (!feof(fp)) {
          fgets(junk,1000,fp);
          b++;
          if (b <= num_lines) {
              junk[0]=0;
              continue;
              }
          else {
              fputs(junk,tfp);
              junk[0]=0;
              }
         }
FCLOSE(fp);
FCLOSE(tfp);
num_lines=0;  lines=0;
  if (!cat(filename2,user,number))
      write_str(user,"^There are no messages on the board.^");
 
DONE:
b=0;
}

if (a==0) {
ustr[user].numbering=0;
if (!cat(filename, user, number)) 
   write_str(user,"^There are no messages on the board.^");
  }

name_area[0]=0;
/* ustr[user].numbering = 0; */
}


/*** wipe board (erase file) ***/
void wipe_board(int user, char *inpstr, int wizard)
{
char filename[FILE_NAME_LEN];
FILE *bfp;
int lower=-1;
int upper=-1;
int mode=0;

if (wizard==0)
  {
   sprintf(t_mess,"%s/board%d",MESSDIR,ustr[user].area);
  }
 else if (wizard==2)
  {
   write_str(user,"***Gripe Wipe***");
   sprintf(t_mess,"%s/gripes",MESSDIR);
  }
 else
  {
   write_str(user,"***Wizard Note Wipe***");
   sprintf(t_mess,"%s/wizmess",MESSDIR);
  }
  
strncpy(filename,t_mess,FILE_NAME_LEN);

/*---------------------------------------------*/
/* check if there is any mail                  */
/*---------------------------------------------*/

if (!(bfp=fopen(filename,"r"))) 
  {
   write_str(user,"^There are no messages to wipe off the board.^"); 
   return;
  }
FCLOSE(bfp);

/*---------------------------------------------*/
/* get the delete parameters                   */
/*---------------------------------------------*/

get_bounds_to_delete(inpstr, &lower, &upper, &mode);
 
if (upper == -1 && lower == -1)
  {
   write_str(user,"No messages wiped.  Specification of what to ");
   write_str(user,"wipe did not make sense.  Type: .help wipe ");
   write_str(user,"for detailed instructions on use. ");
   return;
  }
    
   switch(mode)
    {
     case 0: return;
             break;
        
     case 1: 
            sprintf(mess,"Wiped all messages.");
            upper = -1;
            lower = -1;
            break;
        
     case 2: 
            sprintf(mess,"Wiped line %d.", lower);
            
            break;
        
     case 3: 
            sprintf(mess,"Wiped from line %d to the end.",lower);
            break;
        
     case 4: 
            sprintf(mess,"Wiped from begining of board to line %d.",upper);
            break;
        
     case 5: 
	   if (lower == -1)
            sprintf(mess,"Wiped all except line %d.",upper);
	   else
            sprintf(mess,"Wiped all except lines %d to %d.",upper, lower);

            break;

     case 6: 
            sprintf(mess,"Wiped from line %d to %d.", lower, upper);
            break;
        
     default: return;
              break;
    }


remove_lines_from_file(user, 
                       filename, 
                       lower, 
                       upper);

write_str(user,mess);
if (wizard == 0)
  {
   astr[ustr[user].area].mess_num = file_count_lines(filename);
   if (!astr[ustr[user].area].mess_num)  remove(filename);
  }
 else
  {
    if (!file_count_lines(filename)) remove(filename);
  }
}

/*** sets room topic ***/
void set_topic(int user, char *inpstr)
{

if (!strlen(inpstr)) 
  {
   if (!strlen(astr[ustr[user].area].topic)) 
     {
      write_str(user,"There is no current topic here");  
      return;
     }
    else 
     {
      sprintf(mess,"Current topic is : %s",astr[ustr[user].area].topic);
      write_str(user,mess);  
      return;
     }
  }
	
if (strlen(inpstr)>TOPIC_LEN) 
  {
   write_str(user,"Topic description is too long");  
   return;
  }

if (ustr[user].frog) strcpy(inpstr,FROG_TALK);

strcpy(astr[ustr[user].area].topic,inpstr);

/* send output to users */
sprintf(mess,"Topic set to %s",inpstr);
write_str(user,mess);

if (!ustr[user].vis)
  sprintf(mess,"%s set the topic to %s",INVIS_ACTION_LABEL,inpstr);
 else
  sprintf(mess,"%s has set the topic to %s",ustr[user].say_name,inpstr);

writeall_str(mess, 1, user, 0, user, NORM, TOPIC, 0);

write_log(SYSTEMLOG,YESTIME,"TOPIC in %s changed by %s to \"%s\"\n",astr[ustr[user].area].name,ustr[user].say_name,inpstr);
}


/*** force annoying user to quit prog ***/
void kill_user(int user, char *inpstr)
{
char name[ARR_SIZE];
char line[SAYNAME_LEN*2];
int a;

int victim;

if (!strlen(inpstr))
  {
   write_str(user,"Kill who?");  
   return;
  }
	
sscanf(inpstr,"%s ",name);
strtolower(name);

remove_first(inpstr);

if (strlen(inpstr) > KILL_LEN) {
   sprintf(mess,"Kill message too long. Max is %d characters",KILL_LEN);
   write_str(user,mess);
   return;
   }

if ((victim=get_user_num_exact(name,user)) == -1)
  {
   not_signed_on(user,name);  
   return;
  }

/* cant kill master user */
if (ustr[user].tempsuper<=ustr[victim].super) 
   {
    write_str(user,"That wouldn't be wise....");
    sprintf(mess,"%s actually tried to blast you, HAH!",ustr[user].say_name);
    write_str(victim,mess);
    return;
   }

sprintf(line,"KILL %s performed by %s",strip_color(ustr[victim].say_name),ustr[user].say_name);
btell(user,line);
write_log(SYSTEMLOG,YESTIME,"%s\n",line);

if (!strlen(inpstr)) {
  a = rand() % NUM_KILL_MESSAGES;
  sprintf(mess, kill_text[a], ustr[victim].say_name);
  writeall_str(mess, 0, victim, 0, user, NORM, KILL, 0);
 }
else {
  strcpy(mess, inpstr);
  writeall_str(mess, 0, victim, 0, user, NORM, KILL, 0);
 }

write_str(victim,"");
write_str(victim,DEF_KILL_MESS);

/* kill user */
user_quit(victim,1);
}

/*** shutdown talk server ***/
void shutdown_d(int user, char *inpstr)
{
int u,inlen,said_reboot=0;
int j=0;
int fd;
unsigned int i;
char option[ARR_SIZE];

#if defined(WINDOWS)
PROCESS_INFORMATION p_info;
STARTUPINFO s_info;
char *args=strdup(thisprog); strcat(args," "); strcat(args,datadir);
#else
char *args[]={ PROGNAME,datadir,NULL };
#endif

if ((shutd == -1) && !strlen(inpstr)) goto NORMAL;
else if (strlen(inpstr) > 0) {
       sscanf(inpstr,"%s ",option);
         if (!strcmp(option,"cancel") || !strcmp(option,"-c")) {
          if (down_time==0) {
                  write_str(user,"Auto shutdown/reboot is not on in the first place!");
                  return;
                  }
           else {
                   down_time=0;
		if (treboot==1) {
                   write_str(user,"Auto soft-reboot cancelled!");
		   treboot=0;
		   }
		else if (treboot==2) {
                   write_str(user,"Auto hard-reboot cancelled!");
		   treboot=0;
		   }
		else {
                   write_str(user,"Auto shutdown cancelled!");
		  }
                   return;
                 }
           } /* end of cancel if */
         if (!strcmp(option,"-r")) {            
            remove_first(inpstr);
              if (strlen(inpstr) > 0) {
               option[0]=0;
               sscanf(inpstr,"%s",option);
               said_reboot=1;
               }
              else { 
               treboot=1;
               goto NORMAL;
               }
         } /* if soft reboot */
         else if (!strcmp(option,"-h")) {            
            remove_first(inpstr);
              if (strlen(inpstr) > 0) {
               option[0]=0;
               sscanf(inpstr,"%s",option);
               said_reboot=2;
               }
              else { 
               treboot=2;
               goto NORMAL;
               }
         } /* if hard reboot */
         if ((!strcmp(option,"0")) || (strlen(option) > 5)) {
            write_str(user,"Specify only numbers between 1 and 32766");
            return;
            }

         inlen=strlen(option);
         for (i=0;i<inlen;++i) {
           if (!isdigit((int)option[i])) 
              {
              write_str(user,"Specify only numbers between 1 and 32766");
              return;
              }
         }
           i=0;
          if (down_time > 0) {
	    if (said_reboot)
             write_str(user,"Auto-reboot is already enabled, cancel it first before you change the delay.");
	    else
             write_str(user,"Auto-shutdown is already enabled, cancel it first before you change the delay.");
             return;
             }
          i=atoi(option);

          if (i > 32766) { 
             write_str(user,"Minutes cant exceed 32766.");
             i=0;
             return;
             }
         i=0;
         down_time=atoi(option);
	if (said_reboot) {
         sprintf(t_mess,"System will automatically %sreboot in  %i minutes.",said_reboot==1?"soft-":"hard-",down_time);
         treboot=said_reboot;   /* actually tell the talker that reboot is on */
         }
	else {
         sprintf(t_mess,"System will be automatically shutdown in  %i  minutes.",down_time);
         treboot=0;
         }
         broadcast(user,t_mess);
         down_time=down_time+1;   /* Make sure down_time is at least 1 */
         return;                  /* to keep away from conflicts */
}

else goto START_DOWN;

NORMAL:
write_str_nr(user,"\nAre you sure about this (y/n)? ");
telnet_write_eor(user);
   shutd=user; 
   noprompt=1;
   return;

START_DOWN:
if (treboot==1) (void)setsignal(SIGALRM,(handler_t)SIG_IGN);
(void)setsignal(SIGILL,(handler_t)SIG_IGN);
(void)setsignal(SIGINT,(handler_t)SIG_IGN);
(void)setsignal(SIGABRT,(handler_t)SIG_IGN);
(void)setsignal(SIGFPE,(handler_t)SIG_IGN);
(void)setsignal(SIGSEGV,(handler_t)SIG_IGN);
(void)setsignal(SIGTERM,(handler_t)SIG_IGN);

#if !defined(WINDOWS)
(void)setsignal(SIGTRAP,(handler_t)SIG_IGN);
#if !defined(__CYGWIN32__)
(void)setsignal(SIGIOT,(handler_t)SIG_IGN);
#endif
(void)setsignal(SIGBUS,(handler_t)SIG_IGN);
(void)setsignal(SIGTSTP,(handler_t)SIG_IGN);
(void)setsignal(SIGCONT,(handler_t)SIG_IGN);
(void)setsignal(SIGHUP,(handler_t)SIG_IGN);
(void)setsignal(SIGQUIT,(handler_t)SIG_IGN);
#if !defined(__CYGWIN32__)
(void)setsignal(SIGURG,(handler_t)SIG_IGN);
#endif
(void)setsignal(SIGPIPE,(handler_t)SIG_IGN);
(void)setsignal(SIGTTIN,(handler_t)SIG_IGN);
(void)setsignal(SIGTTOU,(handler_t)SIG_IGN);
#endif

if (!treboot) do_tracking(0, NULL);

requeue_smtp(-1);

if (treboot) {
#if defined(WINDOWS)
  GetStartupInfo(&s_info);
#endif
}

if (treboot!=1) write_str(user,"Quitting users...");
else {
if (SHOW_SREBOOT==1) {
sprintf(mess,"%s *** System soft-rebooting, Please stand by.. ***",STAFF_PREFIX);
writeall_str(mess, WIZ_ONLY, -1, 0, -1, BOLD, WIZT, 0);
}
}

for (u=0;u<MAX_USERS;++u) 
  {
   if (ustr[u].sock==-1) continue;
   if (u == user) continue;
  if (treboot==1) {
   if (SHOW_SREBOOT==2) {
	write_str(u, "");
	write_str(u," ^*** System soft-rebooting, Please stand by.. ***^");
   }
   /* write_bot("+++++ REBOOT"); */
   }
  else if (treboot==2) {
   write_str(u, "");
   write_str(u," ^*** System hard-rebooting. It will be back momentarily ***^");
   write_bot("+++++ REBOOT");
   }
  else {
   write_str(u, "");
   write_str(u," ^*** System shutting down ***^");
   write_bot("+++++ SHUTDOWN");
   }

/*   write_str(u, ""); */
   if (treboot!=1) user_quit(u,1);
   else user_hot_quit(u);
   }
   
sysud(0,user);
check_mess(2);

if (treboot!=1) write_str(user,"Now quitting you...");
if (treboot==1) {
 if (SHOW_SREBOOT==2) write_str(user," ^*** System in soft-reboot. Please stand by. ***^");
}
else if (treboot==2)
 write_str(user," ^*** System will be back momentarily. ***^");
else
 write_str(user," ^*** System is now off. ***^");

strtolower(ustr[user].name);

if (treboot!=1) user_quit(user,1);
else user_hot_quit(user);

/* close listening sockets */
if (treboot!=1) {
for (j=0;j<4;++j) {
 SHUTDOWN(listen_sock[j], 2);
 while (CLOSE(listen_sock[j]) == -1 && errno == EINTR)
	; /* empty while */
 /* FD_CLR(listen_sock[j],&readmask); */
 }
kill_resolver_clipon();
}
else if (resolve_names==2 || resolve_names==3) send_resolver_request(-1,NULL,NULL);

#if defined(WINDOWS)
/* Shutdown winsock + timer thread before exit */
WSACleanup();
TerminateThread(hThread,0);
#endif

if (treboot) {
	/* If someone has changed the binary or the config filename while */
        /* this prog has been running this won't work */
        sleep(2);
        fd = open( "/dev/null", O_RDONLY );
        if ((fd != 0) && (fd != -1)) {
            dup2( fd, 0 );
            CLOSE(fd);
        }
        fd = open( "/dev/null", O_WRONLY );
        if ((fd != 1) && (fd != -1)) {
            dup2( fd, 1 );
            CLOSE(fd);
        }
        fd = open( "/dev/tty", O_WRONLY );
        if ((fd != 2) && (fd != -1)) {
            dup2( fd, 2 );
            CLOSE(fd);
        }

#if defined(WINDOWS)
  CreateProcess(thisprog,args,NULL,NULL,0,
                DETACHED_PROCESS | CREATE_NEW_PROCESS_GROUP | NORMAL_PRIORITY_CLASS,
                NULL, NULL, &s_info, &p_info);
#else
  execvp(PROGNAME,args);
#endif

	/* If we get this far it hasn't worked */
        write_log(BOOTLOG,YESTIME,"BOOT: %sREBOOT Failed! %s\n",treboot==1?"SOFT-":"HARD-",get_error());
	CLOSE(0);
	CLOSE(1);
	CLOSE(2);
	remove(REBOOTFILE);
        exit(12);
       }
exit(0); 
}

void shutdown_auto()
{
int u;
int j=0;
int fd;

#if defined(WINDOWS)
PROCESS_INFORMATION p_info;
STARTUPINFO s_info;
char *args=strdup(thisprog); strcat(args," "); strcat(args,datadir);
#else
char *args[]={ PROGNAME,datadir,NULL };
#endif

if (treboot==1) (void)setsignal(SIGALRM,(handler_t)SIG_IGN);
(void)setsignal(SIGILL,(handler_t)SIG_IGN);
(void)setsignal(SIGINT,(handler_t)SIG_IGN);
(void)setsignal(SIGABRT,(handler_t)SIG_IGN);
(void)setsignal(SIGFPE,(handler_t)SIG_IGN);
(void)setsignal(SIGSEGV,(handler_t)SIG_IGN);
(void)setsignal(SIGTERM,(handler_t)SIG_IGN);

#if !defined(WINDOWS)
(void)setsignal(SIGTRAP,(handler_t)SIG_IGN);
#if !defined(__CYGWIN32__)
(void)setsignal(SIGIOT,(handler_t)SIG_IGN);
#endif
(void)setsignal(SIGBUS,(handler_t)SIG_IGN);
(void)setsignal(SIGTSTP,(handler_t)SIG_IGN);
(void)setsignal(SIGCONT,(handler_t)SIG_IGN);
(void)setsignal(SIGHUP,(handler_t)SIG_IGN);
(void)setsignal(SIGQUIT,(handler_t)SIG_IGN);
#if !defined(__CYGWIN32__)
(void)setsignal(SIGURG,(handler_t)SIG_IGN);
#endif
(void)setsignal(SIGPIPE,(handler_t)SIG_IGN);
(void)setsignal(SIGTTIN,(handler_t)SIG_IGN);
(void)setsignal(SIGTTOU,(handler_t)SIG_IGN);
#endif

if (!treboot) do_tracking(0, NULL);

requeue_smtp(-1);

if (treboot) {
#if defined(WINDOWS)
  GetStartupInfo(&s_info);
#endif
}

if (!num_of_users) goto CONT;

if (treboot==1 && SHOW_SREBOOT==1) {
sprintf(mess,"%s *** System soft-rebooting. Please stand by.. ***",STAFF_PREFIX);
writeall_str(mess, WIZ_ONLY, -1, 0, -1, BOLD, WIZT, 0);
}

for (u=0;u<MAX_USERS;++u) 
  {
   if (ustr[u].area == -1 && !ustr[u].logging_in) continue;
 if (user_wants_message(u,BEEPS)) {
  if (treboot==1) {
  if (SHOW_SREBOOT==2) {
	write_str(u, "");
	write_str(u," \07^*** System soft-rebooting. Please stand by.. ***^\07");
  }
  }
  else if (treboot==2) {
   write_str(u, "");
   write_str(u," \07^*** System hard-rebooting. It will be back momentarily ***^\07");
  }
  else {
   write_str(u, "");
   write_str(u," \07^*** System shutting down ***^\07");
   }
  }
 else {
  if (treboot==1) {
   if (SHOW_SREBOOT==2) {
	write_str(u, "");
	write_str(u," ^*** System soft-rebooting ***^");
   }
   }
  else if (treboot==1) {
   write_str(u, "");
   write_str(u," ^*** System hard-rebooting. It will be back momentarily ***^");
   }
  else {
   write_str(u, "");
   write_str(u," ^*** System shutting down ***^");
   }
  }
/*   write_str(u, ""); */
   if (treboot!=1) user_quit(u,1);
   else user_hot_quit(u);
   }

CONT:
check_mess(2);

if (treboot) {
 write_log(BOOTLOG,YESTIME,"AUTO-%sREBOOT in progress\n",treboot==1?"SOFT-":"HARD-");
 if (treboot!=1) write_bot("+++++ REBOOT");
 }
else {
 write_log(BOOTLOG,YESTIME,"AUTO-SHUTDOWN executed\n");
 write_bot("+++++ SHUTDOWN");
 }

/* close listening sockets */
if (treboot!=1) {
for (j=0;j<4;++j) {
 SHUTDOWN(listen_sock[j], 2);
 while (CLOSE(listen_sock[j]) == -1 && errno == EINTR)
	; /* empty while */
 /* FD_CLR(listen_sock[j],&readmask); */
 }
kill_resolver_clipon();
}
else if (resolve_names==2 || resolve_names==3) send_resolver_request(-1,NULL,NULL);

sysud(0,-1);

#if defined(WINDOWS)
/* Shutdown winsock + timer thread before exit */
WSACleanup();
TerminateThread(hThread,0);
#endif

if (treboot) {
	/* If someone has changed the binary or the config filename while */
        /* this prog has been running this won't work */
        /* sprintf(args,"%s %s\0",PROGNAME,datadir); */
        sleep(2);
        fd = open( "/dev/null", O_RDONLY );
        if ((fd != 0) && (fd != -1)) {
            dup2( fd, 0 );
            CLOSE(fd);
        }
        fd = open( "/dev/null", O_WRONLY );
        if ((fd != 1) && (fd != -1)) {
            dup2( fd, 1 );
            CLOSE(fd);
        }
        fd = open( "/dev/tty", O_WRONLY );
        if ((fd != 2) && (fd != -1)) {
            dup2( fd, 2 );
            CLOSE(fd);
        }

#if defined(WINDOWS)
  CreateProcess(thisprog,args,NULL,NULL,0,
                DETACHED_PROCESS | CREATE_NEW_PROCESS_GROUP | NORMAL_PRIORITY_CLASS,
                NULL, NULL, &s_info, &p_info);
#else
  execvp(PROGNAME,args);
#endif

	/* If we get this far it hasn't worked */
	write_log(BOOTLOG,YESTIME,"BOOT: AUTO-%sREBOOT Failed! %s\n",treboot==1?"SOFT-":"HARD-",get_error());
	CLOSE(0);
	CLOSE(1);
	CLOSE(2);
	remove(REBOOTFILE);
        exit(12);
       }

exit(0); 
}

char *log_error(int error) {
static char message2[256];

switch(error) {
   case 1: strcpy(message2,"a failure to accept a new who socket"); break;
   case 2: strcpy(message2,"a failure to set the who socket non-blocking"); break;
   case 3: strcpy(message2,"a failure to accept a new www socket"); break;
   case 4: strcpy(message2,"a failure to set the www socket non-blocking"); break;
   case 5: strcpy(message2,"a failure to accept a new user socket"); break;
   case 6: strcpy(message2,"a failure to accept a new wiz socket"); break;
   case 7: strcpy(message2,"a failure to set the user or wiz socket non-blocking"); break;
   case 8: strcpy(message2,"a loop panic. Talker broke out of main loop!"); break;
   case 9: strcpy(message2,"an error during select()"); break;
   case 10: strcpy(message2,"job kill"); break;
   case 11: strcpy(message2,"a segmentation fault! - SIGSEGV"); break;
#if !defined(WINDOWS)
   case 12: strcpy(message2,"a bus error! - SIGBUS"); break;
#endif
   case 13: strcpy(message2,"the command line shutdown script"); break;
   case 14: strcpy(message2,"a failure to write to the system logs"); break;
   case 15: strcpy(message2,"execution of an illegal instruction"); break;
   default: strcpy(message2,"an unknown failure"); break;
  }

write_log(ERRLOG,YESTIME,"CAUGHT sig or err, case %d (%s)\n",error,message2);

return message2;
}

void shutdown_error(char *message)
{
int u;
int j=0;
int fd;
char filename1[FILE_NAME_LEN];
char filename2[FILE_NAME_LEN];
char filebuf[ARR_SIZE];
FILE *fp;

#if defined(WINDOWS)
PROCESS_INFORMATION p_info;
STARTUPINFO s_info;
char *args=strdup(thisprog); strcat(args," "); strcat(args,datadir);
#else
char *args[]={ PROGNAME,datadir,NULL };
#endif

if (treboot==1) (void)setsignal(SIGALRM,(handler_t)SIG_IGN);
(void)setsignal(SIGILL,(handler_t)SIG_IGN);
(void)setsignal(SIGINT,(handler_t)SIG_IGN);
(void)setsignal(SIGABRT,(handler_t)SIG_IGN);
(void)setsignal(SIGFPE,(handler_t)SIG_IGN);
(void)setsignal(SIGSEGV,(handler_t)SIG_IGN);
(void)setsignal(SIGTERM,(handler_t)SIG_IGN);

#if !defined(WINDOWS)
(void)setsignal(SIGTRAP,(handler_t)SIG_IGN);
#if !defined(__CYGWIN32__)
(void)setsignal(SIGIOT,(handler_t)SIG_IGN);
#endif
(void)setsignal(SIGBUS,(handler_t)SIG_IGN);
(void)setsignal(SIGTSTP,(handler_t)SIG_IGN);
(void)setsignal(SIGCONT,(handler_t)SIG_IGN);
(void)setsignal(SIGHUP,(handler_t)SIG_IGN);
(void)setsignal(SIGQUIT,(handler_t)SIG_IGN);
#if !defined(__CYGWIN32__)
(void)setsignal(SIGURG,(handler_t)SIG_IGN);
#endif
(void)setsignal(SIGPIPE,(handler_t)SIG_IGN);
(void)setsignal(SIGTTIN,(handler_t)SIG_IGN);
(void)setsignal(SIGTTOU,(handler_t)SIG_IGN);
#endif

/* If we're going down, move the lastcommand to lastcommand.CRASH so we dont lose it */
if (strcmp(message,"the command line shutdown script") && strcmp(message,"job kill")) {
	sprintf(filename1,"%s/%s",LOGDIR,LASTFILE);
	sprintf(filename2,"%s.CRASH",filename1);
if (!check_for_file(filename2))
        rename(filename1,filename2);
else {
fp=fopen(filename1,"r");
fgets(filebuf,9000,fp);
fclose(fp);
remove(filename1);
fp=fopen(filename2,"a");
fputs(filebuf,fp);
fclose(fp);
}
}

if (!treboot) do_tracking(0, message);

requeue_smtp(-1);

if (treboot) {
#if defined(WINDOWS)
  GetStartupInfo(&s_info);
#endif
}

if (!num_of_users) goto CONT2;

if (treboot==1 && SHOW_SREBOOT==1) {
sprintf(mess,"%s *** System soft-rebooting due to %s. Please stand by.. ***",STAFF_PREFIX,message);
writeall_str(mess, WIZ_ONLY, -1, 0, -1, BOLD, WIZT, 0);
}

for (u=0;u<MAX_USERS;++u) 
  {
   if (ustr[u].area == -1 && !ustr[u].logging_in) continue;
   if (treboot==1) {
    if (SHOW_SREBOOT==2) {
	write_str(u, "");
	sprintf(mess," ^*** System soft-rebooting due to %s. Please stand by.. ***^",message);
	write_str(u,mess);
    }
   }
   else if (treboot==2) {
	write_str(u, "");
    sprintf(mess," ^*** System hard-rebooting due to %s ***^",message);
    write_str(u,mess);
   }
   else {
	write_str(u, "");
    sprintf(mess," ^*** System shutting down due to %s ***^",message);
    write_str(u,mess);
   }

   if (treboot!=1) user_quit(u,1);
   else user_hot_quit(u);
  }

CONT2:
check_mess(2);

if (treboot) {
 if (treboot!=1) write_bot("+++++ REBOOT");
 write_log(BOOTLOG,YESTIME,"%sREBOOTING pid %u due to %s\n",treboot==1?"SOFT-":"HARD-",(unsigned int)getpid(),message);
 }
else {
 write_bot("+++++ SHUTDOWN");
 write_log(BOOTLOG,YESTIME,"SHUTDOWN pid %u due to %s\n",(unsigned int)getpid(),message);
 }


/* close listening sockets */
if (treboot!=1) {
for (j=0;j<4;++j) {
 SHUTDOWN(listen_sock[j], 2);
 while (CLOSE(listen_sock[j]) == -1 && errno == EINTR)
	; /* empty while */
 FD_CLR(listen_sock[j],&readmask);
 }
kill_resolver_clipon();
}
else if (resolve_names==2 || resolve_names==3) send_resolver_request(-1,NULL,NULL);

sysud(0,-1);

#if defined(WINDOWS)
/* Shutdown winsock + timer thread before exit */
WSACleanup();
TerminateThread(hThread,0);
#endif

if (treboot) {
	/* If someone has changed the binary or the config filename while */
        /* this prog has been running this won't work */
        /* sprintf(args,"%s %s\0",PROGNAME,datadir); */
        sleep(2);
        fd = open( "/dev/null", O_RDONLY );
        if ((fd != 0) && (fd != -1)) {
            dup2( fd, 0 );
            CLOSE(fd);
        }
        fd = open( "/dev/null", O_WRONLY );
        if ((fd != 1) && (fd != -1)) {
            dup2( fd, 1 );
            CLOSE(fd);
        }
        fd = open( "/dev/tty", O_WRONLY );
        if ((fd != 2) && (fd != -1)) {
            dup2( fd, 2 );
            CLOSE(fd);
        }

#if defined(WINDOWS)
  CreateProcess(thisprog,args,NULL,NULL,0,
                DETACHED_PROCESS | CREATE_NEW_PROCESS_GROUP | NORMAL_PRIORITY_CLASS,
                NULL, NULL, &s_info, &p_info);
#else
  execvp(PROGNAME,args);
#endif

	/* If we get this far it hasn't worked */
        write_log(BOOTLOG,YESTIME,"BOOT: %sREBOOT Failed! %s\n",treboot==1?"SOFT-":"HARD-",get_error());
	CLOSE(0);
	CLOSE(1);
	CLOSE(2);
	remove(REBOOTFILE);
        exit(12);
       }

exit(0); 
}


/*** search for specific word in the message files ***/
void search_boards(int user, char *inpstr)
{
int b,occured=0;
char word[ARR_SIZE],filename[FILE_NAME_LEN];
char line[ARR_SIZE],line2[ARR_SIZE];
FILE *fp;

if (!strlen(inpstr)) 
  {
    write_str(user,"Search for what?");  
    return;
  }
  
sscanf(inpstr,"%s ",word);
strtolower(word);

if (!strcmp(word,"email")) {
  remove_first(inpstr);
  if (!strlen(inpstr)) 
    {
      write_str(user,"Search for what?");  
      return;
    }
  sprintf(t_mess,"%s/%s",MAILDIR,ustr[user].name);
  strncpy(filename,t_mess,FILE_NAME_LEN);
  if (!(fp=fopen(filename,"r"))) {
    write_str(user,BAD_FILEIO);
    write_log(ERRLOG,YESTIME,"Couldn't open file(r) \"%s\" in search_boards! %s\n",filename,get_error());
    return;
    }

	strtolower(inpstr);
	fgets(line,300,fp);
	while(!feof(fp)) {
		strcpy(line2,line);
		strtolower(line2);
		if (instr2(0,line2,inpstr,0)== -1) goto NEXT2;
		write_str(user,line);	
		++occured;
		NEXT2:
		fgets(line,300,fp);
		}
	FCLOSE(fp);
  if (!occured) write_str(user,"No occurences found");
  else {
        write_str(user," ");
	sprintf(mess,"%d occurence%s found",occured,occured == 1 ? "" : "s");
	write_str(user,mess);
       }
  return;
 }
/* look through boards */
for (b=0;b<NUM_AREAS;++b) {
	sprintf(t_mess,"%s/board%d",MESSDIR,b);
	strncpy(filename,t_mess,FILE_NAME_LEN);

	if (!(fp=fopen(filename,"r"))) continue;
	fgets(line,300,fp);
	while(!feof(fp)) {
		strcpy(line2,line);
		strtolower(line2);
		if (instr2(0,line2,word,0)== -1) goto NEXT;
		sprintf(mess,"%s : %s",astr[b].name,line);
		mess[0]=toupper((int)mess[0]);
		if (!astr[b].hidden || (ustr[user].tempsuper>=GRIPE_LEVEL))
		  {
		   write_str(user,mess);	
		   ++occured;
		  }
		NEXT:
		fgets(line,300,fp);
		}
	FCLOSE(fp);
	}
if (!occured) write_str(user,"No occurences found");
else {
        write_str(user," ");
	sprintf(mess,"%d occurence%s found",occured,occured == 1 ? "" : "s");
	write_str(user,mess);
	}
}



/*** review last lines of conversation in room ***/
void review(int user)
{
int area=ustr[user].area;

write_str(user,"^----^ Start of review conversation buffer ^----^");
/* CYGNUS1 */
write_conv_buffer(user, astr[area].conv);
write_str(user,"^----^ End of review conversation buffer ^----^");    
}


/*** help function ***/
void help(int user, char *inpstr)
{
int c,nl=0,d;
char command[20];
char z_mess[20];
char result[FILE_NAME_LEN];
char filename[FILE_NAME_LEN];
FILE *fp;

/* help for one command */
if (strlen(inpstr)) {

   if (strlen(inpstr) < 2) {
      write_str(user,"Pattern not unique enough.");
      return;
      }

	if (strstr(inpstr,"/")) {
           write_str(user,"String has illegal character in it.");
	   write_log(WARNLOG,YESTIME,"User %s attempted to .help %s",ustr[user].say_name,inpstr);
	   return;
	   }

        if (inpstr[0]=='.') midcpy(inpstr,inpstr,1,ARR_SIZE);

	sprintf(result,"%s",get_help_file(inpstr,user));
	if (!strcmp(result,"failed")) {
	    write_str(user,"Sorry - there is no help on that command at the moment");
	    return;
	    }
	else if (!strcmp(result,"failed2"))
	    return;

        sprintf(filename,"%s/%s",HELPDIR,result);
        c=0;
        for (c=0; sys[c].jump_vector != -1 ;++c) {
	  command[0]=0;
	  midcpy(sys[c].command,command,1,ARR_SIZE);

          if (!strcmp(command,result)) {
            if (strlen(sys[c].command) >= 6)
             sprintf(mess,"Command:  %s\t\t\t\t\t\tLevel: %d",sys[c].command,sys[c].su_com);
            else
             sprintf(mess,"Command:  %s\t\t\t\t\t\t\tLevel: %d",sys[c].command,sys[c].su_com);

            write_str(user,mess);
            write_str(user,"");
            break;
            }
          }

	cat(filename,user,0);
	return;
	}

   strcpy(filename,get_temp_file());
   if (!(fp=fopen(filename,"w"))) {
     write_str(user,"Can't create file for paged help listing!");
     return;
     } 

/***** Ncohafmuta format help *****/
if (ustr[user].help==0) {
fputs("Remember - all commands start with a '.' and can be abbreviated\n",fp);
 sprintf(mess,"       ------- Currently these are the commands for Level  ^%i^ -------\n",ustr[user].tempsuper);
fputs(mess,fp);

nl = -1;
c = 0;
mess[0]=0;

   for (c=0; sys[c].jump_vector != -1 ;++c)
      {
	if (sys[c].type==COMM && sys[c].su_com<=ustr[user].tempsuper) {
	   if (nl == -1) {
	       fputs("   ^COMMS:^ ",fp);
	       nl=0;
	       }
           else if (nl == 0) {
               fputs("          ",fp);
               }
	 sprintf(t_mess,"%s %s",sys[c].command,sys[c].cabbr);
         midcpy(t_mess,t_mess,1,20);
	 if (nl==4) 
          sprintf(z_mess,"%s",t_mess);
	 else
          sprintf(z_mess,"%-12s",t_mess);
        strcat(mess,z_mess);
	++nl;
	if (nl==5)
	  {
	    fputs(mess,fp);
	    fputs("\n",fp);
            mess[0]=0;
	    nl= 0;
	  }
	 }   /* end of COMM if */
       }    /* end of  command for */
if (nl) {
 fputs(mess,fp);
 fputs("\n",fp);
}

nl = -1;
c = 0;
mess[0]=0;

   for (c=0; sys[c].jump_vector != -1 ;++c)
      {
	if (sys[c].type==INFO && sys[c].su_com<=ustr[user].tempsuper) {
	   if (nl == -1) {
	       fputs("    ^INFO:^ ",fp);
	       nl=0;
	       }
           else if (nl == 0) {
               fputs("          ",fp);
               }
	 sprintf(t_mess,"%s %s",sys[c].command,sys[c].cabbr);
         midcpy(t_mess,t_mess,1,20);
	 if (nl==4) 
          sprintf(z_mess,"%s",t_mess);
	 else
          sprintf(z_mess,"%-12s",t_mess);
        strcat(mess,z_mess);
	++nl;
	if (nl==5)
	  {
	    fputs(mess,fp);
	    fputs("\n",fp);
            mess[0]=0;
	    nl= 0;
	  }
	 }   /* end of INFO if */
       }    /* end of  command for */
if (nl) {
 fputs(mess,fp);
 fputs("\n",fp);
}

nl = -1;
c = 0;
mess[0]=0;

   for (c=0; sys[c].jump_vector != -1 ;++c)
      {
	if (sys[c].type==MAIL && sys[c].su_com<=ustr[user].tempsuper) {
	   if (nl == -1) {
	       fputs("    ^MAIL:^ ",fp);
	       nl=0;
	       }
           else if (nl == 0) {
               fputs("          ",fp);
               }
	 sprintf(t_mess,"%s %s",sys[c].command,sys[c].cabbr);
         midcpy(t_mess,t_mess,1,20);
	 if (nl==4) 
          sprintf(z_mess,"%s",t_mess);
	 else
          sprintf(z_mess,"%-12s",t_mess);
        strcat(mess,z_mess);
	++nl;
	if (nl==5)
	  {
	    fputs(mess,fp);
	    fputs("\n",fp);
            mess[0]=0;
	    nl= 0;
	  }
	 }   /* end of MAIL if */
       }    /* end of  command for */
if (nl) {
 fputs(mess,fp);
 fputs("\n",fp);
}

nl = -1;
c = 0;
mess[0]=0;

   for (c=0; sys[c].jump_vector != -1 ;++c)
      {
	if (sys[c].type==MESG && sys[c].su_com<=ustr[user].tempsuper) {
	   if (nl == -1) {
	       fputs("^MESSAGIN:^ ",fp);
	       nl=0;
	       }
           else if (nl == 0) {
               fputs("          ",fp);
               }
	 sprintf(t_mess,"%s %s",sys[c].command,sys[c].cabbr);
         midcpy(t_mess,t_mess,1,20);
	 if (nl==4) 
          sprintf(z_mess,"%s",t_mess);
	 else
          sprintf(z_mess,"%-12s",t_mess);
        strcat(mess,z_mess);
	++nl;
	if (nl==5)
	  {
	    fputs(mess,fp);
	    fputs("\n",fp);
            mess[0]=0;
	    nl= 0;
	  }
	 }   /* end of MESG if */
       }    /* end of  command for */
if (nl) {
 fputs(mess,fp);
 fputs("\n",fp);
}

nl = -1;
c = 0;
mess[0]=0;

   for (c=0; sys[c].jump_vector != -1 ;++c)
      {
	if (sys[c].type==MISC && sys[c].su_com<=ustr[user].tempsuper) {
	   if (nl == -1) {
	       fputs("    ^MISC:^ ",fp);
	       nl=0;
	       }
           else if (nl == 0) {
               fputs("          ",fp);
               }
	 sprintf(t_mess,"%s %s",sys[c].command,sys[c].cabbr);
         midcpy(t_mess,t_mess,1,20);
	 if (nl==4) 
          sprintf(z_mess,"%s",t_mess);
	 else
          sprintf(z_mess,"%-12s",t_mess);
        strcat(mess,z_mess);
	++nl;
	if (nl==5)
	  {
	    fputs(mess,fp);
	    fputs("\n",fp);
            mess[0]=0;
	    nl= 0;
	  }
	 }   /* end of MISC if */
       }    /* end of  command for */
if (nl) {
 fputs(mess,fp);
 fputs("\n",fp);
}

nl = -1;
c = 0;
mess[0]=0;

   for (c=0; sys[c].jump_vector != -1 ;++c)
      {
	if (sys[c].type==MOOV && sys[c].su_com<=ustr[user].tempsuper) {
	   if (nl == -1) {
	       fputs("^MOVEMENT:^ ",fp);
	       nl=0;
	       }
           else if (nl == 0) {
               fputs("          ",fp);
               }
	 sprintf(t_mess,"%s %s",sys[c].command,sys[c].cabbr);
         midcpy(t_mess,t_mess,1,20);
	 if (nl==4) 
          sprintf(z_mess,"%s",t_mess);
	 else
          sprintf(z_mess,"%-12s",t_mess);
        strcat(mess,z_mess);
	++nl;
	if (nl==5)
	  {
	    fputs(mess,fp);
	    fputs("\n",fp);
            mess[0]=0;
	    nl= 0;
	  }
	 }   /* end of MOOV if */
       }    /* end of  command for */
if (nl) {
 fputs(mess,fp);
 fputs("\n",fp);
}

nl = -1;
c = 0;
mess[0]=0;

   for (c=0; sys[c].jump_vector != -1 ;++c)
      {
	if (sys[c].type==BANS && sys[c].su_com<=ustr[user].tempsuper) {
	   if (nl == -1) {
	       fputs(" ^BANNING:^ ",fp);
	       nl=0;
	       }
           else if (nl == 0) {
               fputs("          ",fp);
               }
	 sprintf(t_mess,"%s %s",sys[c].command,sys[c].cabbr);
         midcpy(t_mess,t_mess,1,20);
	 if (nl==4) 
          sprintf(z_mess,"%s",t_mess);
	 else
          sprintf(z_mess,"%-12s",t_mess);
        strcat(mess,z_mess);
	++nl;
	if (nl==5)
	  {
	    fputs(mess,fp);
	    fputs("\n",fp);
            mess[0]=0;
	    nl= 0;
	  }
	 }   /* end of BANS if */
       }    /* end of  command for */
if (nl) {
 fputs(mess,fp);
 fputs("\n",fp);
}

nl = -1;
c = 0;
mess[0]=0;

   for (c=0; sys[c].jump_vector != -1 ;++c)
      {
	if (sys[c].type==SETS && sys[c].su_com<=ustr[user].tempsuper) {
	   if (nl == -1) {
	       fputs("^SETTINGS:^ ",fp);
	       nl=0;
	       }
           else if (nl == 0) {
               fputs("          ",fp);
               }
	 sprintf(t_mess,"%s %s",sys[c].command,sys[c].cabbr);
         midcpy(t_mess,t_mess,1,20);
	 if (nl==4) 
          sprintf(z_mess,"%s",t_mess);
	 else
          sprintf(z_mess,"%-12s",t_mess);
        strcat(mess,z_mess);
	++nl;
	if (nl==5)
	  {
	    fputs(mess,fp);
	    fputs("\n",fp);
            mess[0]=0;
	    nl= 0;
	  }
	 }   /* end of SETS if */
       }    /* end of  command for */
if (nl) {
 fputs(mess,fp);
 fputs("\n",fp);
}

fputs("\n",fp);
fputs("For further help type  .help <command>\n",fp);

fclose(fp);
cat(filename,user,0);
}

/***** Iforms format help *****/
else if (ustr[user].help==1) {
fputs("Remember - all commands start with a '.' and can be abbreviated\n",fp);
 sprintf(mess,"       ------- Currently these are the commands for Level  ^%i^ -------\n",ustr[user].tempsuper);
fputs(mess,fp);

nl = -1;
    
for (d=0;d<ustr[user].tempsuper+1;d++)
  {
    
    if (nl!= -1)
      {
       fputs(" \n",fp);
      }

    sprintf(mess,"%c)",ranks[d].abbrev);
    fputs(mess,fp);
    nl=0;
   
    for (c=0; sys[c].su_com != -1 ;++c)
      {
        if (sys[c].type==NONE) continue;
        sprintf(mess,"%-11.11s",sys[c].command);
        mess[0]=' ';
        if (d!=sys[c].su_com) continue;
        if (nl== -1)
          {fputs("  ",fp);
           nl=0;
          }
        fputs(mess,fp);
        ++nl;
        if (nl==6)
          {
            fputs(" \n",fp);
            nl= -1;
          }
       }
   }
if (nl) fputs(" \n",fp);
fputs(" \n",fp);
fputs("For further help type  .help <command>\n",fp);
fclose(fp);
cat(filename,user,0);
}

/***** Nuts 3 format help *****/
else if (ustr[user].help==2) {
 sprintf(mess,"*** Commands available for user level ^%s^ ***\n",ranks[ustr[user].tempsuper].sname);
fputs(mess,fp);

nl = -1;
    
for (d=0;d<ustr[user].tempsuper+1;d++)
  {
    
    if (nl!= -1)
      {
       fputs(" \n",fp);
      }

    sprintf(mess,"^HC(%s)^",ranks[d].sname);
    fputs(mess,fp);
    fputs("\n",fp);
    nl=0;
    c=0;

    for (c=0; sys[c].jump_vector != -1 ;++c)
      {
        if (sys[c].type==NONE) continue;
        if (ustr[user].tempsuper < sys[c].su_com) continue;
        sprintf(mess,"%-11s ",sys[c].command);
        mess[0]=' ';
        if (d!=sys[c].su_com) continue;
        if (nl == -1) nl=0;
        fputs(mess,fp);
        ++nl;
        if (nl==6) {
          fputs("\n",fp);
          nl=-1;
          }
        }

} /* end of level for */

if (nl) fputs(" \n",fp);
fputs(" \n",fp);
fputs("For further help type  .help <command>\n",fp);

fclose(fp);
cat(filename,user,0);
}

/***** Nuts 2 format help *****/
else if (ustr[user].help==3) {
 sprintf(mess,"*** Commands available for user level ^%s^ ***\n",ranks[ustr[user].tempsuper].sname);
fputs(mess,fp);

    for (c=0; sys[c].jump_vector != -1 ;++c)
      {
        if (sys[c].type==NONE) continue;
        if (ustr[user].tempsuper < sys[c].su_com) continue;
        sprintf(mess,"%-11s ",sys[c].command);
        mess[0]=' ';
        fputs(mess,fp);
        ++nl;
        if (nl==6) {  fputs("\n",fp);  nl=0; }
      }

if (nl) fputs(" \n",fp);
fputs(" \n",fp);
fputs("For further help type  .help <command>\n",fp);

fclose(fp);
cat(filename,user,0);
}

}


/*** broadcast message to everyone without the "X shouts:" bit ***/
void broadcast(int user, char *inpstr)
{
if (!strlen(inpstr)) {
	write_str(user,"Broadcast what?");  return;
	}

if (ustr[user].frog) strcpy(inpstr,FROG_TALK);

sprintf(mess,"*** [ %s ] ***",inpstr);
writeall_str(mess, 0, user, 1, user, BOLD, BCAST, 0);
write_log(SYSTEMLOG,YESTIME,"BCAST: by %s\n",ustr[user].say_name);
}



/*** give system status ***/
void system_status(int user)
{
int per_day;
int new_per_day;
int days;
int hours;
long minutes;
int ms;
float total_time;
float dec_hours;
char stm[30];
char onoff[2][4];
char yesno[2][4];
char new[3][4];

strcpy(onoff[0],"OFF");
strcpy(onoff[1],"ON ");
strcpy(yesno[0],"NO ");
strcpy(yesno[1],"YES");
strcpy(new[0],"NO ");
strcpy(new[1],"VFY");
strcpy(new[2],"YES");
write_str(user,"+------------------------SYSTEM STATUS----------------------+");
write_str(user,"| Overall:                                                  |");

strcpy(stm,ctime(&start_time));
stm[strlen(stm)-1]=0;  /* get rid of nl */

minutes=(time(0)-start_time)/60;
days=(int)minutes/1440;
ms=(int)minutes%1440;
hours=ms/60;
dec_hours= (float)hours/24;

 if (days==0) {
     per_day=system_stats.logins_since_start;
     new_per_day=system_stats.new_since_start;
     }
 else {
     total_time=days+dec_hours;
     per_day=system_stats.logins_since_start / total_time;
     new_per_day=system_stats.new_since_start / total_time;
     }

sprintf(mess,  "|        System started: %24.24s           |",stm);
write_str(user,mess);
sprintf(mess,  "|                        %29.29s ago  |",converttime((long)((time(0)-start_time)/60)));
write_str(user,mess);
write_str(user,"|                                                           |");
write_str(user,"|        Atmos    Sys Open    New users     Max Users       |");
sprintf(mess,  "|         %s        %s         %s           %3.3d          |",
             onoff[atmos_on], yesno[sys_access], new[allow_new], MAX_USERS);
write_str(user,mess);

if (atmos_on)
  {
   write_str(user,"|                                                           |");
   write_str(user,"| Atmos: Cycle Time       Factor           Count   Last     |");
   sprintf(mess,  "|        %4d             %4d            %4d    %3d       |",
                                    ATMOS_RESET, ATMOS_FACTOR, ATMOS_COUNTDOWN, ATMOS_LAST);
   write_str(user,mess);
  }

/*
write_str(user,"|                                                           |");
*/     
write_str(user,"|        New Total        New Today        New Per Day      |");
sprintf(mess,  "|           %3.3ld              %3.3ld               %3.3d          |",
                                            system_stats.new_since_start,
system_stats.new_users_today, new_per_day);

write_str(user,mess);
/*
write_str(user,"|                                                           |");
*/
write_str(user,"|        Logins Total     Logins Today     Avg Per Day      |");
sprintf(mess,  "|         %10.10ld         %6.6ld            %3.3d          |",
system_stats.logins_since_start, system_stats.logins_today, per_day);
write_str(user,mess);
/*
write_str(user,"|                                                           |");
*/
write_str(user,"|        New Quota        Message Life     Total expired    |");
sprintf(mess,  "|           %3.3ld              %3.3d days          %3.3ld          |",
system_stats.quota, MESS_LIFE, system_stats.tot_expired);
write_str(user,mess);
write_str(user,"|        Cache Hits       Cache Misses                      |");
sprintf(mess,  "|           %4.4d             %4.4d                           |",
system_stats.cache_hits,system_stats.cache_misses);
write_str(user,mess);
write_str(user,"+-----------------------------------------------------------+");
write_str(user,"^");

per_day=0;
new_per_day=0;
total_time=0;
dec_hours=0;
}


/*** move user somewhere else ***/
void move(int user, char *inpstr)
{
char other_user[ARR_SIZE],area_name[260], tempstr[SAYNAME_LEN+1];
int area,user2,online=1;

/* check user */
if (!strlen(inpstr)) 
  {
   user2 = user;
   area = INIT_ROOM;
   write_str(user,"^HB*** Warp to main room ***^");  
   goto FOUND;
  }
  
sscanf(inpstr,"%s %s",other_user,area_name);

/* plug security hole */
if (check_fname(other_user,user)) 
  {
   write_str(user,"Illegal name.");
   return;
  }

strtolower(other_user);

user2 = get_user_num(other_user,user);
if (user2 == -1) 
  {
   if (!check_for_user(other_user)) {
      write_str(user,NO_USER_STR);
      return;
      }
   read_user(other_user);
   online=0;
  }
else { }

if (online==1) {
/* see if user is moving himself */
if (user==user2) 
  {
   write_str(user,"What do you want to do that for?");
   return;
  }

 if ((!strcmp(ustr[user2].name,ROOT_ID)) || (!strcmp(ustr[user2].name,BOT_ID)
      && strcmp(ustr[user].name,ROOT_ID))) {
    write_str(user,"Yeah, right!");
    return;
    }
  
/* see if user to be moved is superior */
if (ustr[user].tempsuper < ustr[user2].super) 
  {
   write_str(user,"Hmm... inadvisable");
   sprintf(mess,"%s thought about moving you",ustr[user].say_name);
   write_str(user2,mess);
   return;
  }
	
/* check area */
remove_first(inpstr);
if (!strlen(inpstr)) 
  {
   area = INIT_ROOM;
   sprintf(mess,"%s moved to %s",ustr[user2].say_name, astr[area].name);
   write_str(user,mess);  
   goto FOUND;
  }

if (!strcmp(inpstr,HEAD_ROOM)) {
   write_str(user,"Authorized personnel only..sorry");
   return;
   }

  
for (area=0;area<NUM_AREAS;++area) 
     if (! instr2(0, astr[area].name, area_name, 0) ) goto FOUND;
/* CYGNUS */
/*	if (!strcmp(astr[area].name,area_name)) goto FOUND; */
		
write_str(user, NO_ROOM);
return;

FOUND:
if (area==ustr[user2].area) 
  {
   sprintf(mess,"%s is already in that room!",ustr[user2].say_name);
   write_str(user,mess);
   return;
  }

/*-----------------------------------------------------------*/
/* if the room is private abort move...inform user           */
/*-----------------------------------------------------------*/

if (astr[area].private && ustr[user2].invite != area ) 
  {
   write_str(user,"Sorry - that room is currently private");
   return;
  }

/** send output **/
write_str(user2,MOVE_TOUSER);

if (!ustr[user2].vis) 
  strcpy(tempstr,INVIS_ACTION_LABEL);
else
  strcpy(tempstr,ustr[user2].say_name);

/** to old area */
sprintf(mess,MOVE_TOREST,tempstr);
writeall_str(mess, 1, user2, 0, user, NORM, MOVE, 0);

   if (ustr[user2].area == ustr[bot].area) {
    sprintf(mess,"+++++ left:%s", strip_color(ustr[user2].say_name));
    write_bot(mess);
    }

if ((find_num_in_area(ustr[user2].area) <= PRINUM) && astr[ustr[user2].area].private)
  {
    strcpy(mess, NOW_PUBLIC);
    writeall_str(mess, 1, user2, 0, user, NORM, NONE, 0);
    astr[ustr[user2].area].private=0;
  }
  
ustr[user2].area=area;
look(user2,"");

/* to new area */
sprintf(mess,MOVE_TONEW,tempstr,"s");
writeall_str(mess, 1, user2, 0, user, NORM, MOVE, 0);

   if (ustr[user2].area == ustr[bot].area) {
    sprintf(mess,"+++++ came in:%s", strip_color(ustr[user2].say_name));
    write_bot(mess);
    }
}
else {

 if ((!strcmp(t_ustr.name,ROOT_ID)) || (!strcmp(t_ustr.name,BOT_ID)
      && strcmp(ustr[user].name,ROOT_ID))) {
    write_str(user,"Yeah, right!");
    return;
    }
  
/* see if user to be moved is superior */
if (ustr[user].tempsuper < t_ustr.super) 
  {
   write_str(user,"Hmm... inadvisable");
   return;
  }
	
/* check area */
remove_first(inpstr);
if (!strlen(inpstr)) 
  {
   area = INIT_ROOM;
   goto NEW_FOUND;
  }

if (!strcmp(inpstr,HEAD_ROOM)) {
   write_str(user,"Authorized personnel only..sorry");
   return;
   }
  
for (area=0;area<NUM_AREAS;++area) 
     if (! instr2(0, astr[area].name, area_name, 0) ) goto NEW_FOUND;
/* CYGNUS */
/*	if (!strcmp(astr[area].name,area_name)) goto NEW_FOUND; */
		
write_str(user, NO_ROOM);
return;

NEW_FOUND:
if (area==t_ustr.area) 
  {
   sprintf(mess,"%s is already in that room!",t_ustr.say_name);
   write_str(user,mess);
   return;
  }

t_ustr.area=area;
sprintf(mess,"%s moved to %s",t_ustr.say_name, astr[t_ustr.area].name);
write_str(user,mess);  
write_user(other_user);
write_log(SYSTEMLOG,YESTIME,"%s moved %s (offline) to %s\n",ustr[user].name,t_ustr.name, astr[t_ustr.area].name);
}

write_str(user,"Ok");
}



/*** set system access to allow or disallow further logins ***/
void system_access(int user, char *inpstr, int co)
{
char line[SAYNAME_LEN+200];

if (!strlen(inpstr)) {
   sprintf(line,"Main port is     %s",opcl[sys_access]);
   write_str(user,line);
   sprintf(line,"Wizard port is   %s",opcl[wiz_access]);
   write_str(user,line);
   sprintf(line,"Who port is      %s",opcl[who_access]);
   write_str(user,line);
   sprintf(line,"WWW port is      %s",opcl[www_access]);
   write_str(user,line);
   return;
   }

strtolower(inpstr);

if (!co) {
  if(!strcmp(inpstr,"all")) {
        sprintf(line,"ALL PORTS CLOSED BY %s",ustr[user].say_name);
        btell(user,line);
        write_log(SYSTEMLOG,YESTIME,"%s\n",line);
	strcpy(line,"*** System is now closed to all further logins ***");
	writeall_str(line, 0, user, 1, user, BOLD, NONE, 0);
	sys_access=0;

	if (WIZ_OFFSET!=0)
        wiz_access=0;

	if (WHO_OFFSET!=0)
        who_access=0;

	if (WWW_OFFSET!=0)
        www_access=0;

	return;
       }
  if(!strcmp(inpstr,"main")) {
        sprintf(line,"MAIN PORT CLOSED BY %s",ustr[user].say_name);
        btell(user,line);
        write_log(SYSTEMLOG,YESTIME,"%s\n",line);
	sys_access=0;
	return;
       }
  if(!strcmp(inpstr,"wiz")) {
	if (WIZ_OFFSET==0) {
	write_str(user,"The wizard port has been disabled in the code.");
	return;
	}
        sprintf(line,"WIZ PORT CLOSED BY %s",ustr[user].say_name);
        btell(user,line);
        write_log(SYSTEMLOG,YESTIME,"%s\n",line);
	wiz_access=0;
	return;
       }
  if(!strcmp(inpstr,"who")) {
	if (WHO_OFFSET==0) {
	write_str(user,"The who port has been disabled in the code.");
	return;
	}
        sprintf(line,"WHO PORT CLOSED BY %s",ustr[user].say_name);
        btell(user,line);
        write_log(SYSTEMLOG,YESTIME,"%s\n",line);
	who_access=0;
	return;
       }
  if(!strcmp(inpstr,"www")) {
	if (WWW_OFFSET==0) {
	write_str(user,"The web port has been disabled in the code.");
	return;
	}
        sprintf(line,"WWW PORT CLOSED BY %s",ustr[user].say_name);
        btell(user,line);
        write_log(SYSTEMLOG,YESTIME,"%s\n",line);
	www_access=0;
	return;
       }
  else {
        write_str(user,"Unknown option.");
        return;
       }
  }  /* end of co if */
else {	
  if(!strcmp(inpstr,"all")) {
        sprintf(line,"ALL PORTS OPENED BY %s",ustr[user].say_name);
        btell(user,line);
        write_log(SYSTEMLOG,YESTIME,"%s\n",line);
	strcpy(line,"*** System is now open to all further logins ***");
	writeall_str(line, 0, user, 1, user, BOLD, NONE, 0);
	sys_access=1;

	if (WIZ_OFFSET!=0)
        wiz_access=1;

	if (WHO_OFFSET!=0)
        who_access=1;

	if (WWW_OFFSET!=0)
        www_access=1;

	return;
       }
  if(!strcmp(inpstr,"main")) {
        sprintf(line,"MAIN PORT OPENED BY %s",ustr[user].say_name);
        btell(user,line);
        write_log(SYSTEMLOG,YESTIME,"%s\n",line);
	sys_access=1;
	return;
       }
  if(!strcmp(inpstr,"wiz")) {
	if (WIZ_OFFSET==0) {
	write_str(user,"The wizard port has been disabled in the code.");
	return;
	}
        sprintf(line,"WIZ PORT OPENED BY %s",ustr[user].say_name);
        btell(user,line);
        write_log(SYSTEMLOG,YESTIME,"%s\n",line);
	wiz_access=1;
	return;
       }
  if(!strcmp(inpstr,"who")) {
	if (WHO_OFFSET==0) {
	write_str(user,"The who port has been disabled in the code.");
	return;
	}
        sprintf(line,"WHO PORT OPENED BY %s",ustr[user].say_name);
        btell(user,line);
        write_log(SYSTEMLOG,YESTIME,"%s\n",line);
	who_access=1;
	return;
       }
  if(!strcmp(inpstr,"www")) {
	if (WWW_OFFSET==0) {
	write_str(user,"The web port has been disabled in the code.");
	return;
	}
        sprintf(line,"WWW PORT OPENED BY %s",ustr[user].say_name);
        btell(user,line);
        write_log(SYSTEMLOG,YESTIME,"%s\n",line);
	www_access=1;
	return;
       }
  else {
        write_str(user,"Unknown option.");
        return;
       }
 }

}


/*** print out greeting in large letters ***/
void greet(int user, char *inpstr)
{
char pbuff[256];
int slen,lc,c,i,j,found=0;

if (ustr[user].frog) return;

slen = strlen(inpstr);
if (!slen) 
  {
   write_str(user,"Greet whom?"); 
   return;
  }
  
if (slen>10) slen=10;

/* check for special characters in string */
for (i=0;i<slen;++i) {
	if (!isalpha((int)inpstr[i])) { found=1; break; }
}
if (found==1) {
write_str(user,"Message cannot have special characters in it.");
return;
}
i=0;

strcpy(mess,"");
write_str(user,mess);
writeall_str(mess, 1, user, 0, user, NORM, GREET, 0);

for (i=0; i<5; ++i) 
  {
   pbuff[0] = '\0';
   for (c=0; c<slen; ++c) 
     {
      lc = tolower((int)inpstr[c]) - 'a';
      if (lc >= 0 && lc < 27) 
        {
         for (j=0;j<5;++j)
           {
            if(biglet[lc][i][j]) 
              strcat(pbuff,"#"); 
             else 
              strcat(pbuff," ");
           }
         strcat(pbuff,"  ");
        }
      }
   sprintf(mess,"%s",pbuff);
   write_str(user,mess);
   writeall_str(mess, 1, user, 0, user, NORM, GREET, 0);
  }
strcpy(mess,"");
write_str(user,mess);
writeall_str(mess, 1, user, 0, user, NORM, GREET, 0);
}


/** Place a user under arrest, move him to brig **/
/** or unarrest user                            **/
void arrest(int user, char *inpstr, int mode)
{
char other_user[ARR_SIZE],area_name[260];
int area,user2;

/* check for user to move */
if (!strlen(inpstr)) 
  {
   if (mode==0)
    write_str(user,"Arrest whom?"); 
   else if (mode==1)
    write_str(user,"UNarrest whom?");
   return;
  }
 
sscanf(inpstr,"%s",other_user);
if ((user2=get_user_num(other_user,user))== -1) 
  {
    not_signed_on(user,other_user); 
    return;
  }

/* User cannot arrest himself */
if (user==user2) 
  {
   write_str(user,"What do you want to arrest/unarrest yourself for?");
   return;
  }

 if ((!strcmp(ustr[user2].name,ROOT_ID)) || (!strcmp(ustr[user2].name,BOT_ID)
      && strcmp(ustr[user].name,ROOT_ID))) {
    write_str(user,"Yeah, right!");
    return;
    }

/* See if arrest target user is an SU */
if ((ustr[user].tempsuper <= ustr[user2].super) && strcmp(ustr[user].name,ROOT_ID)) 
  {
   write_str(user,"That would not be wise");
   if (mode==0)
   sprintf(mess,"%s thought of placing you under arrest.",ustr[user].say_name);
   else if (mode==1)
   sprintf(mess,"%s thought of unarresting you.",ustr[user].say_name);
   write_str(user2,mess);
   return;
  }

if (mode==0) {
/* Define target area */
sprintf(area_name,"%s",ARREST_ROOM);
for (area=0; area<NUM_AREAS; ++area)
  {
   if (!strcmp(astr[area].name,area_name)) goto FOUND;
  }
  
write_str(user,"Unexpected Error: Arrest_Area Not Found");
return;

FOUND:
if (area==ustr[user2].area) 
  {
   sprintf(mess,"%s is already under arrest!",ustr[user2].say_name);
   write_str(user,mess);
   return;
  }

/** Send output **/
write_str(user2,ARREST_TOUSER);

/* to old area */
sprintf(mess,ARREST_TOREST,ustr[user2].say_name);
writeall_str(mess, 1, user2, 0, user, NORM, NONE, 0);

   if (ustr[user2].area == ustr[bot].area) {
    sprintf(mess,"+++++ left:%s", strip_color(ustr[user2].say_name));
    write_bot(mess);
    }

muzzle(user,ustr[user2].name,1);
gag_comm(user,ustr[user2].name,1);

if ((find_num_in_area(ustr[user2].area)<=PRINUM) && astr[ustr[user2].area].private)
  {
   strcpy(mess, NOW_PUBLIC);
   writeall_str(mess, 1, user2, 0, user, NORM, NONE, 0);
   astr[ustr[user2].area].private=0;
  }

ustr[user2].area=area;
look(user2,"");

/* to brig area */
sprintf(mess,ARREST_TOJAIL,ustr[user2].say_name);
writeall_str(mess, 1, user2, 0, user, NORM, NONE, 0);
sprintf(mess, "ARREST: %s by %s",ustr[user2].say_name,ustr[user].say_name);
btell(user,mess);
write_log(SYSTEMLOG,YESTIME,"%s\n",mess);
sprintf(mess,"%s Arrested by %s",ustr[user2].name,ustr[user].say_name);
warning(user,mess,0);
}
else if (mode==1) {
  unmuzzle(user,ustr[user2].name);
  gag_comm(user,ustr[user2].name,2);
  ustr[user2].area = INIT_ROOM;
  look(user2,"");
  sprintf(mess, "UNARREST: %s by %s",ustr[user2].say_name,ustr[user].say_name);
  btell(user,mess);
  write_log(SYSTEMLOG,YESTIME,"%s\n",mess);
  sprintf(mess,"%s UNArrested by %s",ustr[user2].name,ustr[user].say_name);
  warning(user,mess,0);
 }

}


/** Clear the conversation buffer(s) **/
void cbuff2(int user, char *inpstr)
{
int area=ustr[user].area;

if (!strlen(inpstr)) {
/* CYGNUS1 */
 if (astr[area].conv) init_conv_buffer(astr[area].conv);
 write_str(user,"Conversation buffer cleared!");
 }

else if (!strcmp(inpstr,"temp") && (ustr[user].tempsuper >= CBUFF_LEVEL) ) {
 remove_junk(0);
 write_str(user,"All temp files deleted.");
 return;
 }
else if (!strcmp(inpstr,"rooms") && (ustr[user].tempsuper >= CBUFF_LEVEL) ) {
 area=0;
 for (area=0;area<NUM_AREAS;++area) {
/* CYGNUS1 */
    if (astr[area].conv) init_conv_buffer(astr[area].conv);
  }
 write_str(user,"All room conversation buffers cleared!");
 }

else if (!strcmp(inpstr,"tells")) {
 ctellbuff(user);
 write_str(user,"Tell buffer cleared!");
 }

else if (!strcmp(inpstr,"shouts") && (ustr[user].tempsuper >= CBUFF_LEVEL) ) {
 cshbuff();
 write_str(user,"Shout buffers cleared!");
 }

else if (!strcmp(inpstr,"wiz") && (ustr[user].tempsuper >= CBUFF_LEVEL) ) {
 cbtbuff();
 write_str(user,"Wiztell buffers cleared!");
 }

else if (!strcmp(inpstr,"all") && (ustr[user].tempsuper >= CBUFF_LEVEL) ) {
 area=0;
 for (area=0;area<NUM_AREAS;++area) {
/* CYGNUS1 */
    if (astr[area].conv) init_conv_buffer(astr[area].conv);
  }
 cshbuff();
 cbtbuff();
 write_str(user,"All buffers cleared!");
 }

else {
 if (ustr[user].tempsuper >= CBUFF_LEVEL)
  write_str(user,"That option doesn't exist.");
 else
  write_str(user,NOT_WORTHY);
 return;
 }

}

/** Clear the conversation buffer in the user's room  NON-COMMAND **/
void cbuff(int user)
{
int area=ustr[user].area;

/* CYGNUS1 */
 if (astr[area].conv) init_conv_buffer(astr[area].conv);
 write_str(user,"Conversation buffer cleared!");

}


/** Clear the wiz conversation buffer  **/
void cbtbuff()
{
int i;

for (i=0;i<NUM_LINES;++i) 
  bt_conv[i][0]=0;
  
bt_count = 0;
  
}

/** Clear the shout conversation buffer  **/
void cshbuff()
{
int i;

for (i=0;i<NUM_LINES;++i) 
  sh_conv[i][0]=0;
  
sh_count = 0;
  
}

/** Clear the users tell conversation buffer  **/
void ctellbuff(int user)
{

/* CYGNUS2 */
init_conv_buffer(ustr[user].conv);

}


/*** Display user macros ***/
void macros(int user, char *inpstr)
{
int m=0;
int num=0;
int found=0;
char mname[ARR_SIZE];
char chunk[3];
MacroPtr tmpMacros = ustr[user].Macros;

if (!tmpMacros) {
write_str(user,"Your macro buffers haven't been allocated! This should never happen!");
write_log(ERRLOG,YESTIME,"MALLOC: %s tried to access macros, but their buffer wasn't alloced!\n",ustr[user].say_name);
return;
}

if (!strlen(inpstr)) {
write_str(user,"Your current macros:");
for (m=0;m<NUM_MACROS;++m) {
   if (strlen(tmpMacros->body[m])) {
    sprintf(mess,"%-11s ^=^ %s",tmpMacros->name[m],tmpMacros->body[m]);
    write_str(user,mess);
    num++;
   }
  }
if (num) {
  sprintf(mess,"You have ^HG%d^ of ^HG%d^ defined",num,NUM_MACROS);
  write_str(user,mess);
  return;
  }
else {
  sprintf(mess,"You have no macros defined. You can define up to ^HG%d^",NUM_MACROS);
  write_str(user,mess);
  return;
  }
 } /* end of if not strlen */

sscanf(inpstr,"%s ",mname);
if (!strcmp(mname,"-c") || !strcmp(mname,"clear")) {
   init_macro_buffer(ustr[user].Macros);
   copy_from_user(user);
   write_user(ustr[user].name);
   write_str(user,"Macros cleared.");
   return;
  } /* end of if */
else if (!strcmp(mname,"-d") || !strcmp(mname,"del")) {
   remove_first(inpstr);
   if (!strlen(inpstr)) {
     write_str(user,"You must specify a macro to delete.");
     return;
     }
   for (m=0;m<NUM_MACROS;++m) {
      if (!strcmp(tmpMacros->name[m],inpstr)) {
        tmpMacros->name[m][0]=0;
        tmpMacros->body[m][0]=0;
        copy_from_user(user);
        write_user(ustr[user].name);
        write_str(user,"Macro deleted.");
        return;
        }
    }  /* end of for */
   write_str(user,"No such macro defined.");
   return;
  } /* end of if */
else {
  if (strlen(mname)>11) {
    write_str(user,MACRO_NLONG);
    return;
    }
   /* Check to see if we alreayd have a macro named this */
   for (m=0;m<NUM_MACROS;++m) {
      if (!strcmp(tmpMacros->name[m],mname)) {
        write_str(user,"You have a macro named that! Delete it first if you wish to redefine it.");
        return;
        }
    }  /* end of for */
  remove_first(inpstr);
  if (strlen(inpstr) > MACRO_LEN) {
    write_str(user,MACRO_LONG);
    return;
    }
  for (m=0;m<NUM_MACROS;++m) {
    if (!strlen(tmpMacros->body[m])) {
      num=m;
      found=1;
      break;
      }
    } /* end of for */
  if (!found) {
    sprintf(mess,"You have all ^HG%d^ macros defined. Delete one/some first.",NUM_MACROS);
    write_str(user,mess);
    return;
   }

  /* Do nerf macro check */
  midcpy(inpstr,chunk,0,1);
  if (!NERF_MACRO) {
    if (!strcmp(chunk,".n")) {
     midcpy(inpstr,chunk,1,2);
     if (!strcmp(chunk,"ne")) {
       write_str(user,CANT_MACRO);
       inpstr[0]=0;
       return;
       }
     midcpy(inpstr,chunk,2,2);
     if (!strcmp(chunk," ")) {
       write_str(user,CANT_MACRO);
       inpstr[0]=0;
       return;
      }
     }
    }       

  strcpy(tmpMacros->name[num], strip_color(mname));
  strcpy(tmpMacros->body[num], inpstr);
  copy_from_user(user);
  write_user(ustr[user].name);
  write_str(user,"Macro set.");
 } /* end of main else */
}


/*** Read Mail ***/
void read_mail(int user, char *inpstr)
{
int b=0;
int a=0;
int lines=0;
int num_lines=0;
int buf_lines=0;
long filesize = 0;
char junk[1001];
char filename[FILE_NAME_LEN];
char filename2[FILE_NAME_LEN];
struct stat fileinfo;
FILE *fp;
FILE *tfp;

if (strlen(inpstr)) {
     if (!strcmp(inpstr,"-s")) {
      sprintf(t_mess,"%s/%s",MAILDIR,ustr[user].name);
      strncpy(filename,t_mess,FILE_NAME_LEN);

      /* Get filename size */
      if (stat(filename, &fileinfo) == -1) {
       if (check_for_file(filename)) {
       write_log(WARNLOG,YESTIME,"Couldn't read inbox size for \"%s\" in read_mail! %s\n",filename,get_error());
        }
       }
      else filesize = fileinfo.st_size;

      sprintf(mess,"Your inbox is using %ld out of the max %d bytes.",filesize,MAX_MAILSIZE);
      write_str(user,mess);

     filesize = 0;

      sprintf(t_mess,"%s/%s.sent",MAILDIR,ustr[user].name);
      strncpy(filename,t_mess,FILE_NAME_LEN);

      /* Get filename size */
      if (stat(filename, &fileinfo) == -1) {
       if (check_for_file(filename)) {
       write_log(WARNLOG,YESTIME,"Couldn't read sent-box size for \"%s\" in read_mail! %s\n",filename,get_error());
       }
      }
      else filesize = fileinfo.st_size;

      sprintf(mess,"Your sent mailbox is using %ld out of the max %d bytes.",filesize,MAX_SMAILSIZE);
      write_str(user,mess);
      filesize = 0;
      return;
     }

     if ((strlen(inpstr) < 3) && (strlen(inpstr) > 0) && 
        (!isalpha((int)inpstr[0]))) 
         {
         num_lines=atoi(inpstr);
         buf_lines=num_lines;
         a=1;
         }
     else {
         write_str(user,"Number invalid.");
         return;
         }
    }

/* Send output to user */
sprintf(t_mess,"%s/%s",MAILDIR,ustr[user].name);
strncpy(filename,t_mess,FILE_NAME_LEN);

sprintf(mess,"\n^HR** Your Private Mail Console **^");
write_str(user,mess);

if (a==1) {
   if (num_lines < 1) {
       if (!cat(filename,user,1))
           {
             write_str(user,"You don't have any mail waiting");
             if (ustr[user].new_mail) {
                 write_str(user,"^ ** You should have new mail but your mailfile was deleted. See the admin. **^");
                 }
            }
        goto DONE;
       }
    lines = file_count_lines(filename);
    if (num_lines <= lines) {
       ustr[user].numbering = (lines - num_lines) +1;
       }
     else {
          num_lines=lines;
          }
    num_lines = lines - num_lines;

    if (!(fp=fopen(filename,"r"))) {
        write_str(user,"You don't have any mail waiting");
        num_lines=0;  a=0;  lines=0;
        ustr[user].numbering = 0;
        return;
        }
strcpy(filename2,get_temp_file());
tfp=fopen(filename2,"w");

while (!feof(fp)) {
         fgets(junk,1000,fp);
         b++;
         if (b <= num_lines)  {
             junk[0]=0;
             continue;
            }
          else {
             fputs(junk,tfp);
             junk[0]=0;
             }
       }
FCLOSE(fp);
FCLOSE(tfp);
num_lines=0;  lines=0;
if (!cat(filename2,user,1))
    write_str(user,"You don't have any mail waiting");

DONE:
b=0;
}

if (a==0) {
ustr[user].numbering = 0;
if (!cat(filename,user,1))
  {
   write_str(user,"You don't have any mail waiting");
   if (ustr[user].new_mail)
     {
       write_str(user,"");
       write_str(user,"^ ** You should have new mail but your mailfile was deleted. See the admin. **^");
     }
  }
}

if (a==1) {
   if ((buf_lines >= ustr[user].mail_num) || (buf_lines < 1))
    ustr[user].new_mail = FALSE;
   
   if ((buf_lines >=1) && (buf_lines <= ustr[user].mail_num))
    ustr[user].mail_num-=buf_lines;
   else
    ustr[user].mail_num=0;
 }
else {
ustr[user].new_mail = FALSE;
ustr[user].mail_num = 0;
}

/* ustr[user].numbering= 0; */

copy_from_user(user);
write_user(ustr[user].name);

}

 
/*-----------------------------------------------------------*/
/* Send mail routing                                         */
/* mode == 0 is a non-interactive mail                       */
/*-----------------------------------------------------------*/
void send_mail(int user, char *inpstr, int mode)
{
int u=-1,ret=0,done=0,bad=0,i=0;
int multi=0,level=0,count=0,current=0;
int point=0,point2=0,lastspace=0,lastcomma=0,gotchar=0;
int newread=0;
long filesize=0;
char stm[20],mess2[ARR_SIZE+25],filename[FILE_NAME_LEN],name[SAYNAME_LEN+1];
char other_user[ARR_SIZE];
char message[ARR_SIZE];
char filerid[FILE_NAME_LEN];
char multilist[MAX_MULTIS][ARR_SIZE];
char multiliststr[ARR_SIZE];
struct stat fileinfo;
time_t tm;
FILE *fp;
DIR  *dirp=NULL;
struct dirent *dp;


for (i=0;i<MAX_MULTIS;++i) multilist[i][0]=0;
multiliststr[0]=0;

/*-------------------------------------------------------*/
/* check for any input                                   */
/*-------------------------------------------------------*/

if (!strlen(inpstr)) 
  {
   if (!mode) write_str(user,"Who do you want to mail?"); 
   return;
  }

/* Check if user is gagcommed */
if (ustr[user].gagcomm) {
   if (!mode) write_str(user,NO_COMM);
   return;
   }

if (ustr[user].frog) {
   if (!mode) write_str(user,"Frogs cant write, silly.");
   return;
}

/*-------------------------------------------------------*/
/* get the other user name                               */
/*-------------------------------------------------------*/

sscanf(inpstr,"%s ",other_user);
/*
other_user[NAME_LEN+1]=0;
CHECK_NAME(other_user);
strtolower(other_user);
remove_first(inpstr);
*/

if (!strcmp(other_user,"-f")) {
        other_user[0]=0;
        for (i=0;i<MAX_ALERT;++i) {
         if (strlen(ustr[user].friends[i])) {
          strcpy(multilist[count],ustr[user].friends[i]);
          count++;
          if (count==MAX_MULTIS) break;
          }
        }
        if (!count) {
                write_str(user,"You dont have any friends!");
                return;
        }
        i=0;
	multi=5;
        remove_first(inpstr);
} /* -f */
else if (!strcmp(other_user,"-a")) {
 multi=2;
 level=MAX_LEVEL;
 remove_first(inpstr);

/* write_it(ustr[user].sock,"in -a all\n"); */

 sprintf(t_mess,"%s",USERDIR);
 strncpy(filerid,t_mess,FILE_NAME_LEN);

 dirp=opendir((char *)filerid);

 if (dirp == NULL) {
        sprintf(t_mess,"SEND_EMAIL: Can't open users directory \"%s\" in send_email! %s\n",filerid,get_error());
        write_log(ERRLOG,YESTIME,t_mess);
        write_str(user,t_mess);
        return;
 }
/* write_it(ustr[user].sock,"opened user dir\n"); */
} /* -a */
else if (!strcmp(other_user,"from") || !strcmp(other_user,"to")) {
	remove_first(inpstr);
        sscanf(inpstr,"%s",message);
        remove_first(inpstr);
        for (i=0;i<strlen(message);++i) {
                if (!isdigit((int)message[i])) {
			write_str(user,"Invalid level passed to from/to");
			return;
		}
        }
        if (!strcmp(other_user,"from")) multi=1;
        else if (!strcmp(other_user,"to")) multi=2;
        level=atoi(message);
        if (level > MAX_LEVEL) level=MAX_LEVEL;
        message[0]=0;

 sprintf(t_mess,"%s",USERDIR);
 strncpy(filerid,t_mess,FILE_NAME_LEN);

 dirp=opendir((char *)filerid);

 if (dirp == NULL) {
        sprintf(t_mess,"SEND_EMAIL: Can't open users directory \"%s\" in send_email! %s\n",filerid,get_error());
        write_log(ERRLOG,YESTIME,t_mess);
        write_str(user,t_mess);
        return;
 }
} /* from <level>, to <level> */
else {
        for (i=0;i<strlen(other_user);++i) {
                if (!isdigit((int)other_user[i])) {
                /* probably a user that doesn't exist */
		goto NORMTELL;
                }
        }

 multi=3;
 level=atoi(other_user);
 remove_first(inpstr);
 if (level > MAX_LEVEL) level=MAX_LEVEL;

 sprintf(t_mess,"%s",USERDIR);
 strncpy(filerid,t_mess,FILE_NAME_LEN);

 dirp=opendir((char *)filerid);

 if (dirp == NULL) {
        sprintf(t_mess,"SEND_EMAIL: Can't open users directory \"%s\" in send_email! %s\n",filerid,get_error());
        write_log(ERRLOG,YESTIME,t_mess);
        write_str(user,t_mess);
        return;
 }

NORMTELL:
 if (!multi) {
 /* check for multi-mail, specific users  */
  other_user[0]=0;
  for (i=0;i<strlen(inpstr);++i) {
        if (inpstr[i]==' ') {
                if (lastspace && !gotchar) { point++; point2++; continue; }
                if (!gotchar) { point++; point2++; }
                lastspace=1;
                continue;
          } /* end of if space */
        else if (inpstr[i]==',') {
                if (!gotchar) {
                        lastcomma=1;
                        point++;
                        point2++;
                        continue;
                }
                else {
                if (count <= MAX_MULTIS-1) {
                midcpy(inpstr,multilist[count],point,point2-1);
/*
sprintf(mess,"multia %d is: \"%s\"\n",count,multilist[count]);
write_it(ustr[user].sock,mess);
*/
                count++;
                }
                point=i+1;
                point2=point;
                gotchar=0;
                lastcomma=1;
                continue;
                }

        } /* end of if comma */
        if ((inpstr[i-1]==' ') && (gotchar)) {
                if (count <= MAX_MULTIS-1) {
                midcpy(inpstr,multilist[count],point,point2-1);
/*
sprintf(mess,"multib %d is: \"%s\"\n",count,multilist[count]);
write_it(ustr[user].sock,mess);
*/
                count++;
                }
                break;
        }
        gotchar=1;
        lastcomma=0;
        lastspace=0;
        point2++;
  } /* end of for */
  midcpy(inpstr,multiliststr,i,ARR_SIZE);

/*
sprintf(mess,"multiliststr1 is: \"%s\"\n",multiliststr);
write_it(ustr[user].sock,mess);
sprintf(mess,"inpstr1 is: \"%s\"\n",inpstr);
write_it(ustr[user].sock,mess);
*/

  if (!strlen(multiliststr)) {
        /* no message string, copy last user */
        midcpy(inpstr,multilist[count],point,point2);
/*
sprintf(mess,"multiliststr2 is: \"%s\"\n",multilist[count]);
write_it(ustr[user].sock,mess);
*/
        count++;
        strcpy(inpstr,"");
        }
  else {
        strcpy(inpstr,multiliststr);
/*
sprintf(mess,"inpstr2 is: \"%s\"\n",inpstr);
write_it(ustr[user].sock,mess);
*/
        multiliststr[0]=0;
     }
  if (count > 1) multi=4;
  multiliststr[0]=0;
 } /* !multi */
} /* else just one level, single, or multi user */

i=0;
point=0;
point2=0;
gotchar=0;

if (!strlen(inpstr)) {
   if (!mode) write_str(user,"You have not specified a message");
   return;
}

if (multi && multi<4) {
 if ((ustr[user].tempsuper < MMAIL_LEVEL) || (ustr[user].tempsuper < level)) {
  /* only staff members can mass-mail */
  /* and staff members can only mass-mail up to their level */
  write_str(user,"You don't have that much power!");
  (void) closedir(dirp);
  return;
 }
}

/* write_it(ustr[user].sock,"after users check\n"); */

/* Create prefix to describe multiple recipients */
if (multi) {
 if (multi==1) sprintf(message,"To ALL USERS LEVEL %d AND UP: ",level);
 else if (multi==2) {
  if (level==MAX_LEVEL)
   strcpy(message,"To ALL USERS: ");
  else
   sprintf(message,"To ALL USERS UP TO LEVEL %d: ",level);
 }
 else if (multi==3) sprintf(message,"To ALL USERS OF LEVEL %d: ",level);
 else if (multi==4) {
  for (i=0;i<count;++i) {
   if (i) strcat(multiliststr,",");
   strcat(multiliststr,multilist[i]);
  }
  sprintf(message,"To %s: ",multiliststr);
 }
 else if (multi==5) strcpy(message,"To ALL FRIENDS: ");
}
else message[0]=0;

/* cat on actual message */
strcat(message,inpstr);

/* multi:
   0: single user
   1: from <level> to MAX_LEVEL
   2: from lowest level to <level>
   3: just <level>
   4: specific multiple users
   5: friends
   all is multi=2 with <level>=MAX_LEVEL
*/

/* START OF LOOP */

while (!done) {
/* write_it(ustr[user].sock,"in while\n"); */

if (!multi || multi==4 || multi==5) {
    /* friends or specified */
    if (current >= count) { done=1; continue; }
    strcpy(other_user,multilist[current]);
    current++;
} /* !multi || multi==4 || multi==5 */
else if (multi<4) {
	if ((dp = readdir(dirp)) == NULL) {
		done=1;
		continue;
	}
	else {
/*
write_it(ustr[user].sock,"d_name is\n");
write_it(ustr[user].sock,dp->d_name);
write_it(ustr[user].sock,"\n");
*/
		if (dp->d_name[0]=='.') continue;
		sprintf(other_user,"%s",dp->d_name);
		count++;
	}
} /* multi<4 */

/*
write_it(ustr[user].sock,"other user is\n");
write_it(ustr[user].sock,other_user);
write_it(ustr[user].sock,"\n");
*/

/* plug security hole */
if (check_fname(other_user,user))
  {
   if (!mode) write_str(user,"Illegal name.");
   bad++;
   if (multi) continue;
   else break;
  }

strtolower(other_user);

ret=check_gag2(user,other_user);
if (!ret) {
 bad++;
 if (multi) continue;
 else break;
}
else if (ret==2) {
  write_str(user,NO_USER_STR);
  /* write_str(user,BAD_FILEIO); */
  write_log(ERRLOG,YESTIME,"Couldn't open file(r) for \"%s\" in check_gag2! %s\n",other_user,get_error());
  bad++;
  if (multi) continue;
  else break;
}

if (!read_user(other_user)) {
 write_str(user,BAD_FILEIO);
 write_log(ERRLOG,YESTIME,"Couldn't open user file for \"%s\" in check_gag2! %s\n",other_user,get_error());
 bad++;
 if (multi) continue;
 else break;
}

/* write_it(ustr[user].sock,"checking levels\n"); */

/* check level-based mailing */
if ( (multi==1 && (t_ustr.super < level)) ||
     (multi==2 && (t_ustr.super > level)) ||
     (multi==3 && (t_ustr.super != level)) ) {
	count--; continue;
}

/*
write_it(ustr[user].sock,"GOOD user is\n");
write_it(ustr[user].sock,other_user);
write_it(ustr[user].sock,"\n");
*/

/*--------------------------------------------------*/
/* prepare message to be sent                       */
/*--------------------------------------------------*/
time(&tm);
midcpy(ctime(&tm),stm,4,15);
strcpy(name,ustr[user].say_name);

sprintf(mess,"(%s) From %s: %s\n",stm,name,message);

sprintf(t_mess,"%s/%s",MAILDIR,other_user);
strncpy(filename,t_mess,FILE_NAME_LEN);

/* Get filename size */
if (stat(filename, &fileinfo) == -1) {
    if (check_for_file(filename)) {
    write_log(WARNLOG,YESTIME,"Couldn't read inbox size for \"%s\" in read_mail! %s\n",filename,get_error());
    }
}
else filesize = fileinfo.st_size;

/* write_it(ustr[user].sock,"checking filesize\n"); */

/* THIS IS TO PREVENT EXCESSIVE MAIL SPAMMING */
/* If recepients mailsize is at or over size limit, tell sender the */
/* mail send failed, mail recepient from the talker that mailfile   */
/* is over limit, and notify user, if online.                       */

if (filesize >= MAX_MAILSIZE) {

 if (mode==0 && !multi)
   write_str(user,"Recipient's mailfile is at or over the size limit. Send failed.");

 if (t_ustr.mail_warn == 0) {
   sprintf(t_mess,MAILFILE_NOTIFY,filesize-MAX_MAILSIZE);
   sprintf(mess,"(%s) From THE TALKER: %s\n",stm,t_mess);
   t_mess[0]=0;
   if (!(fp=fopen(filename,"a")))
     {
	if (mode==0 && !multi) {
         sprintf(mess,"%s : message cannot be written\n", syserror);
         write_str(user,mess);
	}
      bad++;
      if (multi) continue;
      else break;
     }
   fputs(mess,fp);
   FCLOSE(fp);
   t_ustr.mail_warn = 1;
   t_ustr.new_mail = TRUE;
   t_ustr.mail_num++;

 /*-------------------------------------------------------*/
 /* write users to inform them of transaction             */
 /*-------------------------------------------------------*/

 if ((u=get_user_num(other_user,user)) != -1) 
  {
   strcpy(t_mess,MAILFROM_TALKER);
   write_str(u,t_mess);
   ustr[u].new_mail = TRUE;
   ustr[u].mail_warn = 1;
   if (ustr[u].autor > 1) {
      if (ustr[u].mail_num > 0) ustr[u].new_mail = TRUE;
      else {
       ustr[u].new_mail = FALSE;
       t_ustr.new_mail = FALSE;
       }
     t_ustr.mail_num--;
     newread=1;
   } /* end if autor > 1 */
   else ustr[u].mail_num++;

   if ((ustr[u].autof==1) && (ustr[u].automsgs < MAX_AUTOFORS)) {
       /* autofwd - USER ONLINE */
       if ((strlen(ustr[u].email_addr) < 8) || !strcmp(ustr[u].email_addr,DEF_EMAIL)) { 
	   /* bad email address */
           write_str(u,"Your set email address is not a valid address..aborting autofwd.");
           copy_from_user(u);
       }
       else {
	/* Send external email */
	if (send_ext_mail(-2, u, 0, NULL, mess, DATA_IS_MSG, NULL)==-1) {
		write_log(ERRLOG,YESTIME,"Couldn't send external email in send_mail()\n");
	}
	else {
		write_str(u,MAIL_AUTOFWD);
		ustr[u].automsgs++;
	}
       } /* good email address */
     } /* end of if autof */
  } /* end of if user online if */
 else if ((t_ustr.autof > 0) && (t_ustr.automsgs < MAX_AUTOFORS)) {
       /* autofwd - USER NOT ONLINE */
       if ((strlen(t_ustr.email_addr) < 8) || !strcmp(t_ustr.email_addr,DEF_EMAIL)) {
	 /* bad email address */
       }
       else {
	/* Send external email */
	if (send_ext_mail(-2, -1, 0, NULL, mess, DATA_IS_MSG, NULL)==-1) {
		write_log(ERRLOG,YESTIME,"Couldn't send external email in send_mail()\n");
	}
	else {
		t_ustr.automsgs++;
	}
       } /* good email address */

   } /* end of if autof */
 } /* end of if mail warn is 0 */
 else { }

 write_user(other_user);

 /* If recepient is online and has autoread in dual mode, */
 /* read their new message                                */

 if (u && newread) read_mail(u,"1");

 bad++;
 if (multi) continue;
 else break;
} /* end of if over filesize */

/* End of mailfile size check */

filesize = 0;

/*---------------------------------------------------*/
/* write message to recipient's mailfile             */
/*---------------------------------------------------*/

if (!(fp=fopen(filename,"a"))) {
 if (mode==0) {
  sprintf(mess,"Mail message cannot be written for user %s! %s", other_user, get_error());
  write_str(user,mess);
 }
 bad++;
 if (multi) continue;
 else break;
}
fputs(mess,fp);
FCLOSE(fp);

/*--------------------------------------------------*/
/* set a new mail flag for that other user          */
/*--------------------------------------------------*/

t_ustr.new_mail = TRUE;
t_ustr.mail_num++;

/*-------------------------------------------------------*/
/* write users to inform them of transaction             */
/*-------------------------------------------------------*/

/* Inform recipient of new mail */
 if ((u=get_user_num_exact(other_user,user)) != -1) 
  {
   sprintf(t_mess,MAILFROM_USER,ustr[user].say_name);
   write_str(u,t_mess);
   ustr[u].new_mail = TRUE;
   if (ustr[u].autor > 1) {
      if (ustr[u].mail_num > 0) ustr[u].new_mail = TRUE;
      else {
       ustr[u].new_mail = FALSE;
       t_ustr.new_mail = FALSE;
       }
     t_ustr.mail_num--;
     newread=1;
   } /* if autor > 1 */
   else ustr[u].mail_num++;

   if ((ustr[u].autof==1) && (ustr[u].automsgs < MAX_AUTOFORS)) {
       /* autofwd - USER ONLINE */
       if ((strlen(ustr[u].email_addr) < 8) || !strcmp(ustr[u].email_addr,DEF_EMAIL)) { 
	/* bad email address */
           write_str(u,"Your set email address is not a valid address..aborting autofwd.");
           copy_from_user(u);
       }
       else {
	/* Send external email */
	if (send_ext_mail(user, u, 1, NULL, mess, DATA_IS_MSG, NULL)==-1) {
		write_log(ERRLOG,YESTIME,"Couldn't send external email in send_mail()\n");
	}
	else {
		write_str(u,MAIL_AUTOFWD);
		ustr[u].automsgs++;
	}
       } /* good email address */

     } /* end of if autof */
    /* MAYBE DO copy_from_user(u); HERE? */
  } /* end of if user online if */
 else if ((t_ustr.autof > 0) && (t_ustr.automsgs < MAX_AUTOFORS)) {
       /* autofwd - USER NOT ONLINE */
       if ((strlen(t_ustr.email_addr) < 8) || !strcmp(t_ustr.email_addr,DEF_EMAIL)) {
	/* bad email address */
       }
       else {
	/* Send external email */
	if (send_ext_mail(user, -1, 1, NULL, mess, DATA_IS_MSG, NULL)==-1) {
		write_log(ERRLOG,YESTIME,"Couldn't send external email in send_mail()\n");
	}
	else {
		t_ustr.automsgs++;
	}
       } /* good email address */
 } /* else if offline autofwd */

write_user(other_user);

/* If recepient is online and has autoread in dual mode, */
/* read their new message                                */

if (u && newread) read_mail(u,"1");

if (!multi) done=1; /* email to only one user, stop */
} /* while !done */

/* END OF LOOP */

if (multi && multi<4) (void) closedir(dirp);

if (!count) {
 write_str(user,"Your mail did not find any recipient(s)");
 return;
}

if (bad) {
 sprintf(t_mess,"Your mail failed to reach %d of %d intended recipient(s) because of an error.",bad,count);
 write_str(user,t_mess);
}

if (bad < count) {
 /*--------------------------------------------------------*/
 /* write sent mail message                                */
 /*--------------------------------------------------------*/

 if (multi)
  sprintf(mess2,"(%s) %s\n",stm,message);
 else
  sprintf(mess2,"(%s) To %s: %s\n",stm,other_user,message);

 sprintf(t_mess,"%s/%s.sent",MAILDIR,ustr[user].name);
 strncpy(filename,t_mess,FILE_NAME_LEN);

 /* Get filename size */
 if (stat(filename, &fileinfo) == -1) {
    if (check_for_file(filename)) {
     write_log(WARNLOG,YESTIME,"Couldn't read sent-box size for \"%s\" in read_mail! %s\n",filename,get_error());
    }
 }
 else filesize = fileinfo.st_size;

 if (filesize >= MAX_SMAILSIZE) {
	if (mode==0 && !multi) {
    	 sprintf(t_mess,MAILFILE2_NOTIFY,filesize-MAX_SMAILSIZE);
    	 write_str(user,t_mess);
	}
    t_mess[0]=0;
 }
 else {
  if (!(fp=fopen(filename,"a"))) {
	if (mode==0) {
  	 sprintf(mess2,"Mail message cannot be written to your sent-mail box! %s", get_error());
   	 write_str(user,mess2);
	}
  }
  else {
	fputs(mess2,fp);
	FCLOSE(fp);
  }
 }

 /* Notify user of any success */
 if (!multi) {
  sprintf(t_mess,MAIL_TO,other_user);
 }
 else if (multi==1) {
  sprintf(t_mess,"^HY==^ You have mailed all users from level %d to %d a message ^HY==^",level,MAX_LEVEL);
 }
 else if (multi==2) {
  if (level==MAX_LEVEL)
   strcpy(t_mess,"^HY==^ You have mailed all users a message ^HY==^");
  else
   sprintf(t_mess,"^HY==^ You have mailed all users up to level %d a message ^HY==^",level);
 }
 else if (multi==3) {
  sprintf(t_mess,"^HY==^ You have mailed all users of level %d a message ^HY==^",level);
 }
 else if (multi==4) {
  sprintf(t_mess,"^HY==^ You have mailed %s a message ^HY==^",multiliststr);
 }
 else if (multi==5) {
  strcpy(t_mess,"^HY==^ You have mailed all your friends a message ^HY==^");
 }

 write_str(user,t_mess);

} /* bad < count */

return;
}


/*** Clear Mail ***/
void clear_mail(int user, char *inpstr)
{
char filename[FILE_NAME_LEN];
FILE *bfp;
int lower=-1;
int upper=-1;
int mode=0;

/*---------------------------------------------*/
/* check if there is any mail                  */
/*---------------------------------------------*/

sprintf(t_mess,"%s/%s",MAILDIR,ustr[user].name);
strncpy(filename,t_mess,FILE_NAME_LEN);

if (!(bfp=fopen(filename,"r"))) 
  {
   write_str(user,"You have no mail."); 
   return;
  }
FCLOSE(bfp);

/* remove the mail file */
if (ustr[user].clrmail== -1) 
  {
   /*---------------------------------------------*/
   /* get the delete parameters                   */
   /*---------------------------------------------*/

   get_bounds_to_delete(inpstr, &lower, &upper, &mode);
 
   if (upper == -1 && lower == -1)
     {
      write_str(user,"No mail deleted.  Specification of what to ");
      write_str(user,"delete did not make sense.  Type: .help cmail ");
      write_str(user,"for detailed instructions on use. ");
      return;
     }
    
   switch(mode)
    {
     case 0: return;
             break;
        
     case 1: 
            sprintf(mess,"Cmail: Delete all mail messages? ");
            upper = -1;
            lower = -1;
            break;
        
     case 2: 
            sprintf(mess,"Cmail: Delete line %d? ", lower);
            
            break;
        
     case 3: 
            sprintf(mess,"Cmail: Delete from line %d to the end?",lower);
            break;
        
     case 4: 
            sprintf(mess,"Cmail: Delete from begining to line %d?",upper);
            break;
        
     case 5: 
           if (lower == -1)
            sprintf(mess,"Cmail: Delete all except line %d?",upper);
           else
            sprintf(mess,"Cmail: Delete all except lines %d to %d?",upper, lower);

            break;

     case 6: 
            sprintf(mess,"Cmail: Delete from line %d to %d.", lower, upper);
            break;
        
     default: return;
              break;
    }

   ustr[user].lower = lower;
   ustr[user].upper = upper;

   ustr[user].clrmail=user; 
   noprompt=1;
   write_str(user,mess);
   write_str_nr(user,"Do you wish to do this? (y/n) ");
	 telnet_write_eor(user);
   return;
  }
  
remove_lines_from_file(user, 
                       filename, 
                       ustr[user].lower, 
                       ustr[user].upper);

sprintf(mess,"You deleted specified mail messages.");
write_str(user,mess);
ustr[user].mail_warn = 0;

if (!file_count_lines(filename))  remove(filename);

}


/*------------------------------------------------------------------------*/
/* promote a user                                                         */
/*------------------------------------------------------------------------*/
void promote(int user, char *inpstr)
{
char other_user[ARR_SIZE];
int u, new_level, a=0;

if (!strlen(inpstr)) 
  {
   write_str(user,"Promote who?"); 
   return;
  }

sscanf(inpstr,"%s ",other_user);

/* plug security hole */
if (check_fname(other_user,user)) 
  {
   write_str(user,"Illegal name.");
   return;
  }
 

strtolower(other_user);

if (!strcmp(ustr[user].name,other_user)) {
   write_str(user,"You cannot promote yourself!");
   return;
   }

   if (!read_user(other_user))
     {
      write_str(user,NO_USER_STR);
      return;
     }

remove_first(inpstr);  

new_level = t_ustr.super;

 if (new_level == MAX_LEVEL)
   {
    write_str(user,"That person is already the highest level in the system.");
    return;
   }

if (ustr[user].tempsuper <= new_level)
  {
    sprintf(mess,"Cant promote %s: That is beyond your authority",t_ustr.say_name);
    write_str(user,mess);
    return;
  }

sscanf(inpstr,"%d", &new_level);
  
if (new_level < 0)
  {
   new_level = 0;
  }
  
if ( ((new_level == MAX_LEVEL-1) && (t_ustr.super == MAX_LEVEL-1)) || 
    (new_level == MAX_LEVEL) )
  {
   write_str(user,"That person is being set to the highest level in the system.");
   new_level = MAX_LEVEL;
  }
  
if (new_level == t_ustr.super) new_level++;
  
if (new_level == ustr[user].tempsuper && PROMOTE_TO_SAME == FALSE && strcmp(ustr[user].name,ROOT_ID))
    {
      sprintf(mess,"Cant promote %s: That is beyond your authority",t_ustr.say_name);
      write_str(user,mess);
      return;
    }

if (new_level > ustr[user].tempsuper && PROMOTE_TO_ABOVE == FALSE)
    {
      sprintf(mess,"Cant promote %s: That is beyond your authority",t_ustr.say_name);
      write_str(user,mess);
      return;
    }

/* Add new level */
t_ustr.super   = new_level;

/* Tell talker that user is promoted     */
/* so they cant auto-promote themselves */
t_ustr.promote = 1;

/* Give new wizzes access to hidden rooms */
for (a=0;a<NUM_AREAS;++a)
  {
    if (astr[a].hidden)
      {
       if (t_ustr.super >= WIZ_LEVEL) {
       t_ustr.security[a]='Y';
       if((u=get_user_num_exact(other_user,user))>-1) {
        ustr[u].security[a]='Y';
        sprintf(mess,"You have been cleared to enter room %s",astr[a].name);
        write_str(u,mess);
        } /* end of if user on */
       } /* end of if level */
      }
  }
a=0;

write_user(other_user); 

write_log(SYSTEMLOG,YESTIME,"PROMOTION to level %d by %s for %s\n",
            t_ustr.super, ustr[user].say_name, t_ustr.say_name);

if((u=get_user_num_exact(other_user,user))>-1) 
  {
   ustr[u].super=new_level;
   sprintf(mess,PROMOTE_MESS,ustr[user].say_name,ranks[ustr[u].super].lname);
   write_str(u,mess);
  }
else
  {
   sprintf(mess,PROMOTE_MESS,ustr[user].say_name,ranks[t_ustr.super].lname);
   sprintf(t_mess,"%s %s",other_user,mess);
   send_mail(user,t_mess,1);
  }

 sprintf(mess,UPROMOTE_MESS,t_ustr.say_name,ranks[t_ustr.super].lname);
 write_str(user,mess);

sprintf(mess,"has PROMOTED %s to %s",other_user,ranks[t_ustr.super].lname);
btell(user,mess);

/* update staff list file */
if (t_ustr.super >= WIZ_LEVEL) do_stafflist();
}

/* Check which part of auto-promotion user is in and change accordingly */
void check_promote(int user, int mode)
{
 int num=0;

 /* User is already promoted */
 if (ustr[user].promote==1) return;

 num = ustr[user].promote + mode;

 /* user is trying to use same command twice to promote themselves */
 if ((num==12) || (num==14) || (num==18)) return;

 if (ustr[user].super==0) {
  if ((num==6) || (num==7) || (num==9)) {
    ustr[user].promote=num;
    strcpy(mess,"Finished step 1 of 3 for AUTO-PROMOTION");
    write_str(user,mess);
    copy_from_user(user);
    write_user(ustr[user].name);
    return;
    }
  else if ((num==13) || (num==15) || (num==16)) {
    ustr[user].promote=num;
    strcpy(mess,"Finished step 2 of 3 for AUTO-PROMOTION");
    write_str(user,mess);
    copy_from_user(user);
    write_user(ustr[user].name);
    return;
    }
  else if (num==22) {
    ustr[user].promote=1;
    ustr[user].super++;
    copy_from_user(user);
    write_user(ustr[user].name);

    /* Inform user of their promotion */
    sprintf(mess,APROMOTE_MESS,ranks[ustr[user].super].lname);
    write_str(user,mess);

    /* Inform wizards of their promotion */
    sprintf(mess,"%s %s has been AUTO-PROMOTED",STAFF_PREFIX,strip_color(ustr[user].say_name));
    writeall_str(mess, WIZ_ONLY, user, 0, user, BOLD, WIZT, 0);

    /* Inform system log */
    write_log(SYSTEMLOG,YESTIME,"AUTO-PROMOTION by THE TALKER for %s\n",ustr[user].say_name);
    return;
    }
 } /* end of if super */
}

/* Demote a user */
void demote(int user, char *inpstr)
{
char other_user[ARR_SIZE];
int u, a=0;
char z_mess[ARR_SIZE];
 
if (!strlen(inpstr)) 
  {
   write_str(user,"Demote who?"); 
   return;
  }

sscanf(inpstr,"%s ",other_user);

/* plug security hole */
if (check_fname(other_user,user)) 
  {
   write_str(user,"Illegal name.");
   return;
  }
 
strtolower(other_user);

if (!read_user(other_user))
  {
   write_str(user,NO_USER_STR);
   return;
  }

if (t_ustr.super == 0)
  {
    sprintf(z_mess,"Cant demote %s: Bottom ranked already",t_ustr.say_name);
    write_str(user,z_mess);
    return;
  }

if (!DEMOTE_SAME) {  
if ((t_ustr.super >= ustr[user].tempsuper) &&
    (strcmp(ustr[user].name,ROOT_ID)) )
  {
    sprintf(z_mess,"Cant demote %s, they hold rank over you or are same rank.",t_ustr.say_name);
    write_str(user,z_mess);
    return;
  }
 }
else {
if ((t_ustr.super > ustr[user].tempsuper) &&
    (strcmp(ustr[user].name,ROOT_ID)) )
  {
    sprintf(z_mess,"Cant demote %s, they hold rank over you or are same rank.",t_ustr.say_name);
    write_str(user,z_mess);
    return;
  }
 }

t_ustr.super--;
  /* if user being demoted below the monitor privledge level, take */
  /* their monitor privledges away */
if (t_ustr.super == MONITOR_LEVEL-1) t_ustr.monitor=0;

/* If were a wiz level or higher and not now take away */
/* hidden room privledges                              */
for (a=0;a<NUM_AREAS;++a)
  {
    if (astr[a].hidden)
      {
       if (t_ustr.super < WIZ_LEVEL) {
       t_ustr.security[a]='N';
       if((u=get_user_num_exact(other_user,user))>-1) {
        ustr[u].security[a]='N';
        sprintf(mess,"You have been removed from special access for room %s",astr[a].name);
        write_str(u,mess);
        } /* end of if user on */
       } /* end of if level */
      }
  }
a=0;

write_user(other_user);
write_log(SYSTEMLOG,YESTIME,"DEMOTION to level %d by %s for %s\n",
              t_ustr.super, ustr[user].say_name, t_ustr.say_name);

if ((u=get_user_num_exact(other_user,user))>-1) 
  {
    ustr[u].super--;
    if (ustr[u].super == MONITOR_LEVEL-1) ustr[u].monitor=0;
    sprintf(z_mess,DEMOTE_MESS,ustr[user].say_name,ranks[ustr[u].super].lname);
    write_str(u,z_mess);
   }
else
  {
   sprintf(z_mess,DEMOTE_MESS,ustr[user].say_name,ranks[t_ustr.super].lname);
   sprintf(t_mess,"%s %s",other_user,z_mess);
   send_mail(user,t_mess,1);
  }

sprintf(z_mess,UDEMOTE_MESS,t_ustr.say_name,ranks[t_ustr.super].lname);
write_str(user,z_mess);

sprintf(z_mess,"has DEMOTED %s to %s",other_user,ranks[t_ustr.super].lname);
btell(user,z_mess);

/* update staff list file */
/* if their old level was a wiz or higher */
if ((t_ustr.super+1) >= WIZ_LEVEL) do_stafflist();
} 


/** Muzzle a user, takes away his .shout capability **/
void muzzle(int user, char *inpstr, int type)
{
char other_user[ARR_SIZE];
int u,inlen;
unsigned int i;

if (!strlen(inpstr)) 
  {
   write_str(user,"Users Muzzled & logged on     Time left"); 
   write_str(user,"-------------------------     ---------"); 
   for (u=0; u<MAX_USERS; ++u) 
    {
     if (ustr[u].shout == 0 && ustr[u].area > -1) 
       {
        if (ustr[u].muz_time == 0)
           sprintf(mess,"%-*s %s",NAME_LEN+count_color(ustr[u].say_name,0),ustr[u].say_name,"Perm");
        else
           sprintf(mess,"%-*s %s",NAME_LEN+count_color(ustr[u].say_name,0),ustr[u].say_name,converttime((long)ustr[u].muz_time));
        write_str(user, mess);
       }
    }
   write_str(user,"(end of list)");
   return;
  }

sscanf(inpstr,"%s ",other_user);
strtolower(other_user);

if ((u=get_user_num(other_user,user))== -1) 
  {
   not_signed_on(user,other_user);
   return;
  }
if (u == user)
  {   
   write_str(user,"You are definitly wierd! Trying to muzzle yourself, geesh."); 
   return;
  }

if (!ustr[u].shout) {
   write_str(user,"They are already muzzled!");
   return;
   }

 if ((!strcmp(ustr[u].name,ROOT_ID)) || (!strcmp(ustr[u].name,BOT_ID)
      && strcmp(ustr[user].name,ROOT_ID))) {
    write_str(user,"Yeah, right!");
    return;
    }
    
if ((ustr[user].tempsuper <= ustr[u].super) && strcmp(ustr[user].name,ROOT_ID)) 
  {
   write_str(user,"That would not be wise...");
   sprintf(mess,MUZZLE_CANT,ustr[user].say_name);
   write_str(u,mess);
   return;
  }

if (type==1)
  goto ARREST;
else
  remove_first(inpstr);

if (strlen(inpstr) && strcmp(inpstr,"0")) {
   if (strlen(inpstr) > 5) {
      write_str(user,"Minutes cant exceed 5 digits.");
      return;
      }
   inlen=strlen(inpstr);
   for (i=0;i<inlen;++i) {
     if (!isdigit((int)inpstr[i])) {
        write_str(user,"Numbers only!");
        return;
        }
     }
    i=0;
    i=atoi(inpstr);
    if ( i > 32767) {
       write_str(user,"Minutes cant exceed 32767.");
       i=0;
       return;
      }
  i=0;
  ustr[u].muz_time=atoi(inpstr);
}
else { ustr[u].muz_time=0; }

ARREST:
ustr[u].shout=0;

sprintf(mess,"%s cant shout anymore",ustr[u].say_name);
writeall_str(mess, 1, u, 1, user, NORM, NONE, 0);
write_str(u,MUZZLEON_MESS);

if (ustr[u].muz_time > 0) {
 sprintf(mess,"MUZZLE: ON for %s by %s for %s",ustr[u].say_name,
         ustr[user].say_name, converttime((long)ustr[u].muz_time));
 }
else {
 sprintf(mess,"MUZZLE: ON for %s by %s",ustr[u].say_name, ustr[user].say_name);
 }
btell(user, mess);
write_log(SYSTEMLOG,YESTIME,"%s\n",mess);
}


/** Unmuzzle a muzzled user, so they can shout again **/
void unmuzzle(int user, char *inpstr)
{
char other_user[ARR_SIZE];
int u;
 
if (!strlen(inpstr)) 
  {
   write_str(user,"Users Currently Unmuzzled and logged on"); 
   write_str(user,"---------------------------------------"); 
   for (u=0;u<MAX_USERS;++u) 
    {
     if (ustr[u].shout && ustr[u].area > -1) 
       {
        write_str(user,ustr[u].say_name);
       };
    }
   write_str(user,"(end of list)");
   return;
  }

sscanf(inpstr,"%s ",other_user);
strtolower(other_user);
 
if ((u=get_user_num(other_user,user))== -1) 
  {
   not_signed_on(user,other_user);
   return;
  }
 
if ((ustr[user].tempsuper <= ustr[u].super) && strcmp(ustr[user].name,ROOT_ID)) 
  {
   write_str(user,"Why do you want to do that?");
   return;
  }

if (ustr[u].shout)
  {
   sprintf(mess,"%s is not muzzled",ustr[u].say_name);
   write_str(user,mess);
   return;
  }
 
if (u == user && ustr[u].super < SENIOR_LEVEL)
  {
   write_str(user,"Silly user, think it would be that simple.");
   return;
  }
 
ustr[u].shout=1;
ustr[u].muz_time=0;
sprintf(mess,"%s can shout again",ustr[u].say_name);
writeall_str(mess, 1, u, 1, user, NORM, NONE, 0);
write_str(u,MUZZLEOFF_MESS);

sprintf(mess,"MUZZLE: OFF for %s by %s",ustr[u].say_name, ustr[user].say_name);
btell(user, mess);
write_log(SYSTEMLOG,YESTIME,"%s\n",mess);
}


/*** Bring a user to you ***/
void bring(int user, char *inpstr)
{
int point=0,count=0,i=0,lastspace=0,lastcomma=0,gotchar=0;
int point2=0,multi=0;
int multilistnums[MAX_MULTIS];
char multilist[MAX_MULTIS][ARR_SIZE];
char multiliststr[ARR_SIZE];
int user2=-1,area=ustr[user].area;
char other_user[ARR_SIZE];
char tempstr[SAYNAME_LEN+200];

for (i=0;i<MAX_MULTIS;++i) { multilist[i][0]=0; multilistnums[i]=-1; }
multiliststr[0]=0;
i=0;

if (!strlen(inpstr)) {
   write_str(user,"Bring who?");
   return;
   }

if (!strcmp(astr[area].name,HEAD_ROOM)) {
   write_str(user,"Noone can be .brung into this room  Hehe.");
   return;
   }

sscanf(inpstr,"%s ",other_user);
if (!strcmp(other_user,"-f")) {
        other_user[0]=0; 
        for (i=0;i<MAX_ALERT;++i) {
         if (strlen(ustr[user].friends[i])) {
          strcpy(multilist[count],ustr[user].friends[i]);
          count++;
	  if (count==MAX_MULTIS) break;
          }
        }
        if (!count) {
                write_str(user,"You dont have any friends!");
                return;
        }
        i=0;
        remove_first(inpstr);  
  }
else {
other_user[0]=0;
          
for (i=0;i<strlen(inpstr);++i) {
        if (inpstr[i]==' ') {
                if (lastspace && !gotchar) { point++; point2++; continue; }
                if (!gotchar) { point++; point2++; }
                lastspace=1;
                continue;
          } /* end of if space */
        else if (inpstr[i]==',') {
                if (!gotchar) {
                        lastcomma=1;
                        point++;
                        point2++;
                        continue;
                }
                else {
                if (count <= MAX_MULTIS-1) {
                midcpy(inpstr,multilist[count],point,point2-1);
                count++;
                }
                point=i+1;
                point2=point;
                gotchar=0;
                lastcomma=1;
                continue;
                }
                        
        } /* end of if comma */
        if ((inpstr[i-1]==' ') && (gotchar)) {
                if (count <= MAX_MULTIS-1) {
                midcpy(inpstr,multilist[count],point,point2-1);
                count++;
                }
                break;
        }
        gotchar=1;
        lastcomma=0;
        lastspace=0;
        point2++;
} /* end of for */      
midcpy(inpstr,multiliststr,i,ARR_SIZE);
if (!strlen(multiliststr)) {
        /* no message string, copy last user */
        midcpy(inpstr,multilist[count],point,point2);
        count++; 
        strcpy(inpstr,"");
        }
else {
        strcpy(inpstr,multiliststr);
        multiliststr[0]=0;
     }
} /* end of friend else */

i=0;
point=0; 
point2=0;
gotchar=0;

if (count>1) multi=1;

/* go into loop and check users */
for (i=0;i<count;++i) {

strcpy(other_user,multilist[i]);

/* plug security hole */
if (check_fname(other_user,user))
  { 
   if (!multi) {
   write_str(user,"Illegal name.");
   return;
   }
   else continue;
  }

strtolower(other_user);


if ((user2=get_user_num(other_user,user))== -1) {
   not_signed_on(user,other_user);
   if (!multi) return;
   else continue;
   }

if (user==user2) {
   write_str(user,"You cant bring yourself!");
   if (!multi) return;
   else continue;
   }

 if ((!strcmp(ustr[user2].name,ROOT_ID)) || (!strcmp(ustr[user2].name,BOT_ID)
      && strcmp(ustr[user].name,ROOT_ID))) {
    write_str(user,"Yeah, right!");
    if (!multi) return;
    else continue;
    }

/* Cant bring a master user */
if (ustr[user2].super > ustr[user].tempsuper) {
   write_str(user,"Hmm... inadvisable");
   sprintf(mess,"%s thought about bringing you to the %s",ustr[user].say_name,astr[ustr[user].area].name);
   write_str(user2,mess);
   if (!multi) return;
   else continue;
   }

if (area==ustr[user2].area) {
   sprintf(mess,"%s is already in this room!",ustr[user2].say_name);
   write_str(user,mess);
   if (!multi) return;
   else continue;
   }

/* check if this user is already in the list */
/* we're gonna reuse some ints here          */
for (point2=0;point2<MAX_MULTIS;++point2) {
        if (multilistnums[point2]==user2) { gotchar=1; break; }
   }
point2=0;
if (gotchar) {
  gotchar=0;
  continue;
  }

/* it's ok to send the tell to this user, add them to the multistr */
/* add this user to the list for our next loop */
multilistnums[point]=user2;
point++;
} /* end of user for */
i=0;
    
/* no multilistnums, must be all bad users */
if (!point) { 
        return;
  }

/* loop to compose the messages and print to the users */
for (i=0;i<point;++i) {

user2=multilistnums[i];

count=0;
point2=0;
multiliststr[0]=0;

/** send output **/
write_str(user2,MOVE_TOUSER);

/** to old area **/
if (!ustr[user2].vis) 
  strcpy(tempstr,INVIS_ACTION_LABEL);
else
  strcpy(tempstr,ustr[user2].say_name);

sprintf(mess,MOVE_TOREST,tempstr);
writeall_str(mess, 1, user2, 0, user, NORM, MOVE, 0);

   if (ustr[user2].area == ustr[bot].area) {
    sprintf(mess,"+++++ left:%s", strip_color(ustr[user2].say_name));
    write_bot(mess);
    }

if ((find_num_in_area(ustr[user2].area)<=PRINUM) && astr[ustr[user2].area].private)
   {
   strcpy(mess, NOW_PUBLIC);
   writeall_str(mess, 1, user2, 0, user, NORM, NONE, 0);
   astr[ustr[user2].area].private=0;
   }
ustr[user2].area=area;
look(user2,"");

   if (ustr[user2].area == ustr[bot].area) {
    sprintf(mess,"+++++ came in:%s", strip_color(ustr[user2].say_name));
    write_bot(mess);
    }

} /* end of message compisition for loop */

/* make multi string to send to this user */
if (multi) {
point2=0;
multiliststr[0]=0;
for (point2=0;point2<point;++point2) {
if (point2>0)
 strcat(multiliststr,",");
/* add their name to the output string */
if (!ustr[multilistnums[point2]].vis)
 strcat(multiliststr,INVIS_ACTION_LABEL);
else
 strcat(multiliststr,ustr[multilistnums[point2]].say_name);
}
} /* end of if multi */
else strcpy(multiliststr,tempstr);

/* To new area */
sprintf(mess,MOVE_TONEW,multiliststr,multi == 1 ? "" : "s");
if (!multi)
 writeall_str(mess, 1, user2, 0, user, NORM, MOVE, 0);
else
 writeall_str(mess, 1, -1, 0, user, NORM, MOVE, 0);

write_str(user,"Ok");
}

/* This commands allows a user to hide their entry on the who list */
/* and make themselves a shadow in the current room, masking their */
/* name                                                            */
void hide(int user, char *inpstr)
{
char name[ARR_SIZE];
int victim,userlevel;
char str2[ARR_SIZE];

name[0]=0;

if (!strlen(inpstr)) 
  {
   victim=user;
   userlevel=ustr[victim].tempsuper;
  }
 else
  {
   sscanf(inpstr,"%s",name);
   strtolower(name);
   if ((victim=get_user_num(name,user))== -1) 
     {
      not_signed_on(user,name);
      return;
     }
   userlevel=ustr[victim].super;
  }
  
if (userlevel < MIN_HIDE_LEVEL)
  {
    write_str(user,"Cannot use hide on that person");
    ustr[victim].vis=1;
    return;
  }
  
if ((ustr[victim].monitor==1) || (ustr[victim].monitor==3)) 
  {
   strcpy(str2,"<");
   strcat(str2,ustr[user].say_name);
   strcat(str2,"> ");
  }
 else
  { str2[0]=0; }


if ( (userlevel >= ustr[user].tempsuper) && 
     (strcmp(ustr[user].name,ustr[victim].name)) ) 
  {
   write_str(user,"That would not be wise...");
   if (!ustr[victim].vis)
      sprintf(mess,"%s wanted to make you visible.",  ustr[user].say_name);
     else
      sprintf(mess,"%s wanted to make you invisible.",  ustr[user].say_name);

   write_str(victim, mess);
   return;
  }
  
if (!ustr[victim].vis) 
  {
   sprintf(mess,COME_VIS,ustr[victim].say_name);
   writeall_str(mess, 1, victim, 0, user, NORM, MOVE, 0);
	if (strlen(name)) write_str(user,mess);
   sprintf(mess,UCOME_VIS,str2);
   write_str(victim,mess);
   ustr[victim].vis=1;
  }
 else
  {
   sprintf(mess,GO_INVIS,ustr[victim].say_name);
   writeall_str(mess, 1, victim, 0, user, NORM, MOVE, 0);
	if (strlen(name)) write_str(user,mess);
   sprintf(mess,UGO_INVIS,str2);
   write_str(victim,mess);
   ustr[victim].vis=0;
  }

}

/* Display of list of levels. Also display the fight odds */
/* and number of commands associated with each.           */
void display_ranks(int user)
{
  char z_mess[80];
  int i=0;
  int c=0;
  int count=0;
  int numcmds=0;

write_str(user,"");
sprintf(z_mess,"Your rank is ^%d^ (%s)",ustr[user].tempsuper,ranks[ustr[user].tempsuper].lname);
write_str(user,z_mess);  
write_str(user,"------------------------------------------------------------------");
write_str(user,"lvl  rank                  odds     cmds this level    cmds total");
write_str(user,"------------------------------------------------------------------");

for(i=0;i<MAX_LEVEL+1;i++)
  {
    for (c=0; sys[c].su_com != -1; ++c) {
     if ((sys[c].type != NONE) && (sys[c].su_com==i)) numcmds++;
     }
    count += numcmds;
    if (ustr[user].tempsuper==i)
	    sprintf(z_mess,"^HG%c    %-20.20s (%-5.5d)          %-3d              %-3d^",ranks[i].abbrev,ranks[i].lname,ranks[i].odds,numcmds,count);
    else
	    sprintf(z_mess,"%c    %-20.20s (%-5.5d)          %-3d              %-3d",ranks[i].abbrev,ranks[i].lname,ranks[i].odds,numcmds,count);
    write_str(user,z_mess);
    c=0;
    numcmds=0;
  }
write_str(user,"");
sprintf(z_mess,"There are %d commands in the system.",count);
write_str(user,z_mess);
write_str(user,"");
}


/*----------------------------------------------------------*/
/* no-op code for disabled commands                         */
/*----------------------------------------------------------*/
void command_disabled(int user)
{
write_str(user,"Sorry: That command is temporarily disabled");
}


/*----------------------------------------------------------*/
/* print out all users, or those with letter matches        */
/*----------------------------------------------------------*/
void print_users(int user, char *inpstr)
{
char filename[FILE_NAME_LEN];

sprintf(t_mess,"%s",USERDIR);
strncpy(filename,t_mess,FILE_NAME_LEN);

if (!strcmp(inpstr," "))
   print_dir(user, filename, NULL);
  else
   print_dir(user, filename, inpstr);
return;  
}

    
    
/*----------------------------------------------------------*/
/* create a file in the restrict library for an ip site     */
/* that has been banned.                                    */
/* to make it easier, large sites can be banned by level    */
/* a b c or d.                                              */
/* where host address is a.b.c.d                            */
/* or where ip address is d.c.b.a                           */
/*----------------------------------------------------------*/
void restrict(int user, char *inpstr, int type)
{
int num;
int len=0;
int timenum;
char small_buff[64];
char filename[FILE_NAME_LEN];
char filename2[FILE_NAME_LEN];
char filename3[FILE_NAME_LEN];
char text_mess[35];
char timestr[23];
char timebuf[23];
char site_name[65];
char chunk[10];
char reason[ARR_SIZE];
char comment[ARR_SIZE];
time_t tm_then;
struct dirent *dp;
FILE *fp;
FILE *fp2;
DIR *dirp;

if (!strcmp(inpstr,"list") || !strlen(inpstr))
  {
   if (type==ANY)
     { 
      sprintf(t_mess,"%s",RESTRICT_DIR);
     }
   else
     {
      sprintf(t_mess,"%s",RESTRICT_NEW_DIR);
     }

   strncpy(filename,t_mess,FILE_NAME_LEN);

 num=0;
 dirp=opendir((char *)filename);
  
 if (dirp == NULL)
   {write_str(user,"Directory information not found.");
    write_log(ERRLOG,YESTIME,"Directory information not found for directory \"%s\" in restrict %s\n",
    filename,get_error());
    return;
   }

   write_str(user,"Site/Cluster/Domain                 Ban Started"); 
   write_str(user,"-------------------------           -----------"); 

   strcpy(filename3,get_temp_file());
   if (!(fp2=fopen(filename3,"w"))) {
	write_str(user,BAD_FILEIO);
        write_log(ERRLOG,YESTIME,"Couldn't open tempfile(w) in restrict! %s\n",get_error());
     (void) closedir(dirp);
     return;
     }
   
 while ((dp = readdir(dirp)) != NULL) 
   { 
    sprintf(small_buff,"%s",dp->d_name);
    len=strlen(small_buff);
       if ((small_buff[0]=='.') || 
           ( (small_buff[len-2]=='.') && 
             ((small_buff[len-1]=='c') ||
              (small_buff[len-1]=='r')) ) ) {
         small_buff[0]=0;
         len=0;
         continue;
        }
      else
        {
         sprintf(filename2,"%s/%s",filename,small_buff);
         if (!(fp=fopen(filename2,"r"))) {
            write_str(user,"Cant open file for reading!");
            continue;
            }
         fgets(timebuf,13,fp);
         FCLOSE(fp);
         timenum=atoi(timebuf);
         tm_then=((time_t) timenum);
         sprintf(mess,"%-35s %s ago",small_buff,converttime((long)((time(0)-tm_then)/60)));
         fputs(mess,fp2);
         fputs("\n",fp2);
         timebuf[0]=0;
         timenum=0;
         len=0;
         num++;
         fputs("COMMENT: ",fp2);
         sprintf(filename2,"%s/%s.c",filename,small_buff);
         fp=fopen(filename2,"r");
         fgets(comment,ARR_SIZE,fp);
         FCLOSE(fp);
         fputs(comment,fp2);
         fputs("\n\n",fp2);
         comment[0]=0;
        }
      small_buff[0]=0;
      len=0;
  }
 sprintf(mess,"Displayed %d banned site%s",num,num == 1 ? "" : "s");
 fputs(mess,fp2);
 fputs("\n",fp2);

 fclose(fp2);
 (void) closedir(dirp);

 if (!cat(filename3,user,0)) {
     write_str(user,BAD_FILEIO);   
     write_log(ERRLOG,YESTIME,"Couldn't cat file \"%s\" in restrict! %s\n",filename3,get_error());
     }

 return;
 }

if (inpstr[0]=='.' ||
    inpstr[0]=='*' ||
    inpstr[0]=='/' ||
    inpstr[0]=='+' ||
    inpstr[0]=='-' ||
    inpstr[0]=='?' )
  {
   write_str(user,"Invalid site name.");
   return;
  }  
   
 sscanf(inpstr,"%s ",site_name);
 remove_first(inpstr);


/* open board file */
if (type==ANY)
  {
   sprintf(t_mess,"%s/%s",RESTRICT_DIR,site_name);
   strcpy(text_mess,"RESTRICTED from access.");
  }
else
  {
   sprintf(t_mess,"%s/%s",RESTRICT_NEW_DIR,site_name);
   strcpy(text_mess,"NEW USER restricted.");
  }
strncpy(filename,t_mess,FILE_NAME_LEN);


if (!strlen(inpstr)) {
 if ((fp=fopen(filename,"r"))) {
    write_str(user,"That site is already restricted.");
    fclose(fp);
    return;
    }
 FCLOSE(fp);
if (type==ANY)
  {
   sprintf(t_mess,"%s/%s.r",RESTRICT_DIR,site_name);
   strncpy(filename2,t_mess,FILE_NAME_LEN);
   fp=fopen(filename2,"w");
   sprintf(mess,BANNED_MESS,SYSTEM_EMAIL);   
   fputs(mess,fp);
   fclose(fp);
   sprintf(t_mess,"%s/%s.c",RESTRICT_DIR,site_name);
   strncpy(filename2,t_mess,FILE_NAME_LEN);
   fp=fopen(filename2,"w");
   fputs(BANNED_COMMENT,fp);
   fclose(fp);
  }
else
  {
   sprintf(t_mess,"%s/%s.r",RESTRICT_NEW_DIR,site_name);
   strncpy(filename2,t_mess,FILE_NAME_LEN);
   fp=fopen(filename2,"w");
   fputs(BANNED_NEW_MESS,fp);
   fclose(fp);
   sprintf(t_mess,"%s/%s.c",RESTRICT_NEW_DIR,site_name);
   strncpy(filename2,t_mess,FILE_NAME_LEN);
   fp=fopen(filename2,"w");
   fputs(BANNED_COMMENT,fp);
   fclose(fp);
  }
 }

else {
 midcpy(inpstr,chunk,0,1);

 if (strlen(inpstr) > 2) {
   if (!strcmp(chunk,"-c")) {
      midcpy(inpstr,comment,3,ARR_SIZE);
      if (!strlen(comment)) {
       write_str(user,"If you specify option, must give a message also.");
       chunk[0]=0;
       return;
       }
      if (type==ANY) {
      sprintf(t_mess,"%s/%s.c",RESTRICT_DIR,site_name);
      strncpy(filename2,t_mess,FILE_NAME_LEN);
      fp=fopen(filename2,"w");
      fputs(comment,fp);
      fclose(fp);
      }
      else {
      sprintf(t_mess,"%s/%s.c",RESTRICT_NEW_DIR,site_name);
      strncpy(filename2,t_mess,FILE_NAME_LEN);
      fp=fopen(filename2,"w");
      fputs(comment,fp);
      fclose(fp);
      }      
     }  /* end of comment if */
   else if (!strcmp(chunk,"-r")) {
      midcpy(inpstr,reason,3,ARR_SIZE);
      if (!strlen(reason)) {
       write_str(user,"If you specify option, must give a message also.");
       chunk[0]=0;
       return;
       }
      if (strlen(reason) > REASON_LEN) {
         write_str(user,"Reason too long.");
         reason[0]=0;
         chunk[0]=0;
         return;
         }
      if (type==ANY) {
      sprintf(t_mess,"%s/%s.r",RESTRICT_DIR,site_name);
      strncpy(filename2,t_mess,FILE_NAME_LEN);
      fp=fopen(filename2,"w");
      fputs(reason,fp);
      fclose(fp);
      }
      else {
      sprintf(t_mess,"%s/%s.r",RESTRICT_NEW_DIR,site_name);
      strncpy(filename2,t_mess,FILE_NAME_LEN);
      fp=fopen(filename2,"w");
      fputs(reason,fp);
      fclose(fp);
      }      
     }  /* end of reason if */
   else {
      write_str(user,"Option not understood..Aborting.");
      chunk[0]=0;
      return;
      }
   } /* end of sub-if */

 else {
   write_str(user,"Option not understood..Aborting.");
   chunk[0]=0;
   return;
   }
  /* inpstr exists but less than one or not equal to -c or -r */
} /* end of main else */

/* Write time banned to site file if file doesn't exist */
/* if file exists, leave alone */

if ((fp2=fopen(filename,"r"))) {
    fclose(fp2);
    sprintf(mess,"Comment/Reason added to site %s by %s",site_name,ustr[user].say_name);
    btell(user,mess);
    write_log(BANLOG,YESTIME,"%s\n",mess);
   }  /* end of if */

else {
 if (!(fp2=fopen(filename,"w"))) {
         write_str(user,BAD_FILEIO);
         write_log(ERRLOG,YESTIME,"Couldn't open file(w) \"%s\" in restrict! %s\n",filename,get_error()); 
         return;
         }
 sprintf(timestr,"%ld\n",(unsigned long)time(0));
 fputs(timestr,fp2);
 fclose(fp2);
  /* Now since we're creating a new ban, check which - option we       */
  /* specified..if we didn't give one, the default reason and comment  */
  /* are already written to the files from above. Write now the reason  */
  /* file if -c was specified or write the comment file if -r was given */
   if (!strcmp(chunk,"-c")) {
     sprintf(filename,"%s.r",filename);
     fp2=fopen(filename,"w");
     if (type==ANY) {
   	sprintf(mess,BANNED_MESS,SYSTEM_EMAIL);   
   	fputs(mess,fp2);
	}
     else
      fputs(BANNED_NEW_MESS,fp2);
     fclose(fp2);
     }
   else if (!strcmp(chunk,"-r")) {
     sprintf(filename,"%s.c",filename);
     fp2=fopen(filename,"w");
     fputs(BANNED_COMMENT,fp2);
     fclose(fp2);
     }

 sprintf(mess,"Site %s is now %s.",site_name, text_mess);
 write_str(user,mess);
 sprintf(mess,"%s: site %s by %s", text_mess, site_name, ustr[user].say_name);
 btell(user,mess);
 write_log(BANLOG,YESTIME,"%s\n",mess);
 }  /* end of else */
}


/* Unban a site */
void unrestrict(int user, char *inpstr, int type)
{
char filename[FILE_NAME_LEN];

if (!strlen(inpstr)) {
        write_str(user,"You forgot the address"); return;
        }
 
/* check site name for stupid shit     */
/* like "." and ".." and "/etc/passwd" */
if (inpstr[0]=='.' ||
    inpstr[0]=='*' ||
    inpstr[0]=='/' ||
    inpstr[0]=='+' ||
    inpstr[0]=='-' ||
    inpstr[0]=='?' )
  {
   write_str(user,"Invalid address.");
   return;
  }  

/* Remove time restrict file */
 if (type==ANY)
   {
    sprintf(t_mess,"%s/%s",RESTRICT_DIR,inpstr);
   }
 else
   {
    sprintf(t_mess,"%s/%s",RESTRICT_NEW_DIR,inpstr);
   }
strncpy(filename,t_mess,FILE_NAME_LEN);

remove(filename);

/* Remove comment file */
 if (type==ANY)
   {
    sprintf(t_mess,"%s/%s.c",RESTRICT_DIR,inpstr);
   }
 else
   {
    sprintf(t_mess,"%s/%s.c",RESTRICT_NEW_DIR,inpstr);
   }
strncpy(filename,t_mess,FILE_NAME_LEN);

remove(filename);

/* Remove reason file */
 if (type==ANY)
   {
    sprintf(t_mess,"%s/%s.r",RESTRICT_DIR,inpstr);
   }
 else
   {
    sprintf(t_mess,"%s/%s.r",RESTRICT_NEW_DIR,inpstr);
   }
strncpy(filename,t_mess,FILE_NAME_LEN);

remove(filename);

sprintf(mess,"Site %s is ALLOWED ACCESS again.",inpstr);
write_str(user,mess);

if (type==ANY)
 sprintf(mess,"UNRESTRICT: site %s by %s",inpstr,ustr[user].say_name);
else
 sprintf(mess,"UNBANNEW: site %s by %s",inpstr,ustr[user].say_name);

btell(user,mess);
write_log(BANLOG,YESTIME,"%s\n",mess);
}


/*** START OF BOT COMMANDS ***/

void bot_whoinfo(int user, char *inpstr)
{
int u,v;

if (strlen(inpstr)) strtolower(inpstr);

for (v=0;v<NUM_AREAS;++v) 
     {
       for (u=0;u<MAX_USERS;++u) {
		if ((ustr[u].area!= -1) && (ustr[u].area == v) && (!ustr[u].logging_in))
	        {
		  if (!strlen(inpstr) || ( strlen(inpstr) && !strcmp(ustr[u].name,inpstr) ) )
			{
			sprintf(mess,"+++++ command: who: %s %s %d",ustr[u].say_name,astr[ustr[u].area].name,ustr[u].vis);
			write_bot(mess);
			}
		} /* end of in area if */
	  } /* end of user for */
     } /* end of area for */
}


/*-----------------------------------------------------------*/
/* atmospherics code                                         */
/*-----------------------------------------------------------*/
/* until future changes, this is how the files must be:      */
/*                                                           */
/*  example:                                                 */
/*          10                                               */
/*          line of text                                     */
/*          20                                               */
/*          line of text                                     */
/*          30                                               */
/*          line of text                                     */
/*          40                                               */
/*          line of text                                     */
/*                                                           */
/*  To have a multi line message, use '@' in the line        */
/*-----------------------------------------------------------*/
/*** atmospheric function (uses area directory) ***/
void atmospherics()
{
FILE *fp;
char filename[FILE_NAME_LEN],probch[10],line[512];
int probint,area;
int rnd;

ATMOS_COUNTDOWN = ATMOS_COUNTDOWN - ((rand() % ATMOS_FACTOR) +1);

if ( ATMOS_COUNTDOWN > 0) return;

ATMOS_COUNTDOWN = ATMOS_RESET;

for (area=0; area<NUM_AREAS; ++area) 
  {
   if (!find_num_in_area(area)) continue;

   if (astr[area].atmos==0) continue;
	
   sprintf(t_mess, "%s/%s.atmos",datadir, astr[area].name);
   strncpy(filename,t_mess,FILE_NAME_LEN);

   if (!(fp=fopen(filename,"r"))) continue;

   rnd=rand() % 100;
   ATMOS_LAST = rnd;
	
   fgets(probch,6,fp);
   while(!feof(fp)) 
    {
     probint=atoi(probch);
     strcpy(line,"");
     line[0]=0;
     fgets(line,511,fp);
		
     if (rnd<probint) 
       { 
        write_area(area,line);  
        break;
       } 

      strcpy(probch,"");
      probch[0]=0;
      fgets(probch,6,fp);
     }
	  
   FCLOSE(fp);
  }
}


/*** write to areas - if area= -1 write to all areas ***/
void write_area(int area, char *inpstr)
{
int u;
int i=0;
int j=0;
char buff[ARR_SIZE];
char buff2[10];

for (u=0;u<MAX_USERS;++u) 
  {
    strcpy(buff,"");
    buff[0]=0;

    if (!user_wants_message(u,ATMOS)) continue;
    if (ustr[u].area==-1)             continue;
     
    if (ustr[u].area==area || area== -1)  
      { 
        j = strlen(inpstr);

        for (i=0;i<j;i++)
          {
            if (inpstr[i]=='@') {
              strcat(buff,"\n");
              if (ustr[u].car_return) { strcat(buff,"\r"); }
             }
            else {
             strcpy(buff2,"");
             buff2[0]=0;
             midcpy(inpstr,buff2,i,i);
             strcat(buff,buff2);
            }
          } /* end of for */

       write_str(u,buff);
      }
   }
}


/* Write login activity graph to file */
void write_meter(int mode)
{
int i=0;
int j=0;
char timebuf[100];
char datestr[80];
char daystr[11];
char filename[FILE_NAME_LEN];
time_t tm;
time_t day_ago = 60 * 60 * 24;
FILE *fp;

time(&tm);

if (mode==1) tm -= day_ago;
strcpy(datestr,ctime(&tm));
midcpy(datestr,daystr,0,9);

sprintf(mess,"User login activity for %s\n",daystr);

/* Open the file to write to it */
sprintf(filename,"%s",ACTVYFILE);
if (!(fp=fopen(filename,"a"))) {
      write_log(ERRLOG,YESTIME,"Couldn't open file(a) \"%s\" in write_meter! %s\n",filename,get_error());
      return;
  }

fputs(mess,fp);
time(&tm);
strcpy(datestr,ctime(&start_time));
datestr[strlen(datestr)-1]=0;
sprintf(mess,"System Booted: %s\n",datestr);
fputs(mess,fp);
sprintf(mess,"Total logins for this day: %ld\n\n",system_stats.logins_today);
fputs(mess,fp);
fputs("LOGS\n",fp);

timebuf[0]=0;

/* If the hour's login numbers fall into the range, put an asterix */
/* in the buffer, else put a space                                 */

strcpy(timebuf,"100+  | ");

for (i=0;i<24;++i) {
   if (logstat[i].logins > 100)
    strcat(timebuf,"*  ");
   else
    strcat(timebuf,"   ");
 if (i==23) {
   j=strlen(timebuf);
   timebuf[j-2]=0;
  }
}

fputs(timebuf,fp);
fputs("\n",fp);

i=0;
j=0;

strcpy(timebuf,"91-100| ");

for (i=0;i<24;++i) {
   if ((logstat[i].logins > 90) && (logstat[i].logins <= 100))
    strcat(timebuf,"*  ");
   else
    strcat(timebuf,"   ");
 if (i==23) {
   j=strlen(timebuf);
   timebuf[j-2]=0;
  }
}

fputs(timebuf,fp);
fputs("\n",fp);
i=0;
j=0;

strcpy(timebuf,"81-90 | ");

for (i=0;i<24;++i) {
   if ((logstat[i].logins > 80) && (logstat[i].logins <= 90))
    strcat(timebuf,"*  ");
   else
    strcat(timebuf,"   ");
 if (i==23) {
   j=strlen(timebuf);
   timebuf[j-2]=0;
  }
}

fputs(timebuf,fp);
fputs("\n",fp);
i=0;
j=0;

strcpy(timebuf,"71-80 | ");

for (i=0;i<24;++i) {
   if ((logstat[i].logins > 70) && (logstat[i].logins <= 80))
    strcat(timebuf,"*  ");
   else
    strcat(timebuf,"   ");
 if (i==23) {
   j=strlen(timebuf);
   timebuf[j-2]=0;
  }
}

fputs(timebuf,fp);
fputs("\n",fp);
i=0;
j=0;

strcpy(timebuf,"61-70 | ");

for (i=0;i<24;++i) {
   if ((logstat[i].logins > 60) && (logstat[i].logins <= 70))
    strcat(timebuf,"*  ");
   else
    strcat(timebuf,"   ");
 if (i==23) {
   j=strlen(timebuf);
   timebuf[j-2]=0;
  }
}

fputs(timebuf,fp);
fputs("\n",fp);
i=0;
j=0;

strcpy(timebuf,"51-60 | ");

for (i=0;i<24;++i) {
   if ((logstat[i].logins > 50) && (logstat[i].logins <= 60))
    strcat(timebuf,"*  ");
   else
    strcat(timebuf,"   ");
 if (i==23) {
   j=strlen(timebuf);
   timebuf[j-2]=0;
  }
}

fputs(timebuf,fp);
fputs("\n",fp);
i=0;
j=0;

strcpy(timebuf,"41-50 | ");

for (i=0;i<24;++i) {
   if ((logstat[i].logins > 40) && (logstat[i].logins <= 50))
    strcat(timebuf,"*  ");
   else
    strcat(timebuf,"   ");
 if (i==23) {
   j=strlen(timebuf);
   timebuf[j-2]=0;
  }
}

fputs(timebuf,fp);
fputs("\n",fp);
i=0;
j=0;

strcpy(timebuf,"31-40 | ");

for (i=0;i<24;++i) {
   if ((logstat[i].logins > 30) && (logstat[i].logins <= 40))
    strcat(timebuf,"*  ");
   else
    strcat(timebuf,"   ");
 if (i==23) {
   j=strlen(timebuf);
   timebuf[j-2]=0;
  }
}

fputs(timebuf,fp);
fputs("\n",fp);
i=0;
j=0;

strcpy(timebuf,"21-30 | ");

for (i=0;i<24;++i) {
   if ((logstat[i].logins > 20) && (logstat[i].logins <= 30))
    strcat(timebuf,"*  ");
   else
    strcat(timebuf,"   ");
 if (i==23) {
   j=strlen(timebuf);
   timebuf[j-2]=0;
  }
}

fputs(timebuf,fp);
fputs("\n",fp);
i=0;
j=0;

strcpy(timebuf,"11-20 | ");

for (i=0;i<24;++i) {
   if ((logstat[i].logins > 10) && (logstat[i].logins <= 20))
    strcat(timebuf,"*  ");
   else
    strcat(timebuf,"   ");
 if (i==23) {
   j=strlen(timebuf);
   timebuf[j-2]=0;
  }
}

fputs(timebuf,fp);
fputs("\n",fp);
i=0;
j=0;

strcpy(timebuf,"0-10  | ");

for (i=0;i<24;++i) {
   if ((logstat[i].logins > 0) && (logstat[i].logins <= 10))
    strcat(timebuf,"*  ");
   else
    strcat(timebuf,"   ");
 if (i==23) {
   j=strlen(timebuf);
   timebuf[j-2]=0;
  }
}

fputs(timebuf,fp);
fputs("\n",fp);
i=0;
j=0;

strcpy(timebuf,"      +-|--|--|--|--|--|--|--|--|--|--|--|--|--|--|--|--|--|--|--|--|--|--|--|");
fputs(timebuf,fp);
fputs("\n",fp);
timebuf[0]=0;
strcpy(timebuf,"      12a  1  2  3  4  5  6  7  8  9 10 11 12p 1  2  3  4  5  6  7  8  9 10 11");
fputs(timebuf,fp);
fputs("\n\n\n",fp);

fclose(fp);
strcpy(timebuf,"");
timebuf[0]=0;
i=0;
j=0;

return;
}


/*----------------------------------------------------------------------*/
/* initialize a new user                                                */
/*----------------------------------------------------------------------*/
void init_user(int user)
{
int i;
time_t tm;

time(&tm);
   strcpy(ustr[user].name,       ustr[user].login_name);
   strcpy(ustr[user].say_name,   ustr[user].name);

ustr[user].say_name[0]=toupper((int)ustr[user].say_name[0]);

   strcpy(ustr[user].email_addr, DEF_EMAIL);
   strcpy(ustr[user].desc,       DEF_DESC);
   strcpy(ustr[user].sex,        DEF_GENDER);
   strcpy(ustr[user].init_date,  ctime(&tm));
   ustr[user].init_date[24]=0;
   strcpy(ustr[user].last_date,  ctime(&tm));
   ustr[user].last_date[24]=0;
   strcpy(ustr[user].creation,   ctime(&tm));
   ustr[user].creation[24]=0;
   strcpy(ustr[user].init_site,    ustr[user].site);
   strcpy(ustr[user].last_site,    ustr[user].site);
   strcpy(ustr[user].last_name,    ustr[user].net_name);
   strcpy(ustr[user].init_netname, ustr[user].net_name);
   strcpy(ustr[user].succ,       DEF_SUCC);
   strcpy(ustr[user].fail,       DEF_FAIL);
   strcpy(ustr[user].entermsg,   DEF_ENTER);
   strcpy(ustr[user].exitmsg,    DEF_EXIT);
   strcpy(ustr[user].homepage,   DEF_URL);
   strcpy(ustr[user].webpic,     DEF_PICURL);
   strcpy(ustr[user].home_room, astr[new_room].name);
   ustr[user].rawtime   = tm;
   ustr[user].afkmsg[0] = 0;
   ustr[user].promote   = 0;

   for(i=0;i<MAX_AREAS;i++)
     {
      ustr[user].security[i]='N';
     }

ustr[user].super=            0;
ustr[user].area=             new_room;
ustr[user].shout=            1;
ustr[user].vis=              1;
ustr[user].locked=           0;
ustr[user].suspended=        0;
ustr[user].monitor=          0;
ustr[user].rows=             24;
ustr[user].cols=             256;
ustr[user].car_return=       1;
ustr[user].abbrs =           1;
ustr[user].times_on =        0;
ustr[user].white_space =     1;
ustr[user].aver =            0;
ustr[user].totl =            0;
ustr[user].autor =           0;
ustr[user].autof =           0;
ustr[user].automsgs =        0;
ustr[user].gagcomm =         0;
ustr[user].semail =          0;
ustr[user].quote =           1;
ustr[user].hilite =          1;
ustr[user].new_mail =        0;
ustr[user].color =           COLOR_DEFAULT;
ustr[user].numcoms =         0;
ustr[user].mail_num =        0;
ustr[user].numbering =       0;
ustr[user].ttt_kills =       0;
ustr[user].ttt_killed =      0;
ustr[user].ttt_board =       0;
ustr[user].ttt_opponent =   -3;
ustr[user].ttt_playing =     0;
ustr[user].hang_stage =     -1;   
ustr[user].hang_word[0] =   '\0';
ustr[user].hang_word_show[0]='\0';
ustr[user].hang_guess[0] =  '\0';
ustr[user].hang_wins =       0;
ustr[user].hang_losses =     0;
strcpy(ustr[user].icq, DEF_ICQ);
strcpy(ustr[user].miscstr1, "NA");
strcpy(ustr[user].miscstr2, "NA");
strcpy(ustr[user].miscstr3, "NA");
strcpy(ustr[user].miscstr4, "NA");
ustr[user].pause_login =     1;
ustr[user].miscnum2 =        0;
ustr[user].miscnum3 =        0;
ustr[user].miscnum4 =        0;
ustr[user].miscnum5 =        0;
ustr[user].tempnum1 =        0;

if (ustr[user].Macros) init_macro_buffer(ustr[user].Macros);

   for (i=0; i<MAX_ALERT; i++)
     {
      ustr[user].friends[i][0]=0;
     }
   i=0;
   for (i=0; i<MAX_GAG; i++)
     {
      ustr[user].gagged[i][0]=0;
     }
   i=0;
   for (i=0; i<MAX_GRAVOKES; i++)
     {
      ustr[user].revokes[i][0]=0;
     }
   i=0;
/* CYGNUS2 */
	if (ustr[user].conv) init_conv_buffer(ustr[user].conv);
/*
   for (i=0; i<NUM_LINES; i++)
     {
      ustr[user].conv[i][0]=0;
     }
*/

initabbrs(user);
listen_all(user);

/* ROOT_ID's first login */
if (!strcmp(ustr[user].login_name,ROOT_ID)) 
  {
    ustr[user].super = MAX_LEVEL;
    ustr[user].promote = 1;

    for(i=0;i<MAX_AREAS;i++)
     {
      ustr[user].security[i]='Y';
     }

  }

}

/*----------------------------------------------------------------------*/
/* copy the user structures from the temp buffer to an actual user      */
/*----------------------------------------------------------------------*/
void copy_to_user(int user)
{
int i=0;
MacroPtr tmpUserMacros = ustr[user].Macros;
MacroPtr tmpMacros = t_ustr.Macros;

strcpy(ustr[user].name,t_ustr.name);
strcpy(ustr[user].say_name,t_ustr.say_name);
strcpy(ustr[user].password,t_ustr.password);

ustr[user].super=t_ustr.super;

strcpy(ustr[user].email_addr,t_ustr.email_addr);
strcpy(ustr[user].desc,t_ustr.desc);
strcpy(ustr[user].sex,t_ustr.sex);
strcpy(ustr[user].init_date,t_ustr.init_date);
strcpy(ustr[user].last_date,t_ustr.last_date);
strcpy(ustr[user].init_site,t_ustr.init_site);
strcpy(ustr[user].last_site,t_ustr.last_site);
strcpy(ustr[user].last_name,t_ustr.last_name);
strcpy(ustr[user].init_netname,t_ustr.init_netname);

while (strlen(t_ustr.custAbbrs[i].com) > 1) {   
  strcpy(ustr[user].custAbbrs[i].abbr,t_ustr.custAbbrs[i].abbr);
  strcpy(ustr[user].custAbbrs[i].com,t_ustr.custAbbrs[i].com);
  i++;
 }
i=0;
for (i=0;i<NUM_MACROS;i++) {
 if (strlen(tmpMacros->name[i])) {
  strcpy(tmpUserMacros->name[i],tmpMacros->name[i]);
  strcpy(tmpUserMacros->body[i],tmpMacros->body[i]);
  }
 else {
  tmpUserMacros->name[i][0]=0;
  tmpUserMacros->body[i][0]=0;
 }
}
i=0;
for (i=0; i<MAX_ALERT; i++) {
 if (strlen(t_ustr.friends[i]))
  strcpy(ustr[user].friends[i],t_ustr.friends[i]);
 else
  ustr[user].friends[i][0]=0;
 }
i=0;
for (i=0; i<MAX_GAG; i++) {
 if (strlen(t_ustr.gagged[i]))
  strcpy(ustr[user].gagged[i],t_ustr.gagged[i]);
 else
  ustr[user].gagged[i][0]=0;
 }

ustr[user].area           =t_ustr.area;
ustr[user].shout          =t_ustr.shout;
ustr[user].vis            =t_ustr.vis;
ustr[user].locked         =t_ustr.locked;
ustr[user].suspended      =t_ustr.suspended;

strcpy(ustr[user].entermsg,t_ustr.entermsg);
strcpy(ustr[user].exitmsg,t_ustr.exitmsg);
strcpy(ustr[user].home_room,t_ustr.home_room);
strcpy(ustr[user].fail,t_ustr.fail);
strcpy(ustr[user].succ,t_ustr.succ);
strcpy(ustr[user].homepage,t_ustr.homepage);
strcpy(ustr[user].creation,t_ustr.creation);

strcpy(ustr[user].security,t_ustr.security);
strcpy(ustr[user].flags, t_ustr.flags);
strcpy(ustr[user].webpic, t_ustr.webpic);

ustr[user].numcoms        =t_ustr.numcoms;
ustr[user].totl           =t_ustr.totl;
ustr[user].rawtime        =t_ustr.rawtime;

ustr[user].monitor        =t_ustr.monitor;
ustr[user].rows           =t_ustr.rows;
ustr[user].cols           =t_ustr.cols;
ustr[user].car_return     =t_ustr.car_return;
ustr[user].abbrs          =t_ustr.abbrs;
ustr[user].times_on       =t_ustr.times_on;
ustr[user].white_space    =t_ustr.white_space;
ustr[user].aver           =t_ustr.aver;
ustr[user].autor          =t_ustr.autor;
ustr[user].autof          =t_ustr.autof;
ustr[user].automsgs       =t_ustr.automsgs;
ustr[user].gagcomm        =t_ustr.gagcomm;
ustr[user].semail         =t_ustr.semail;
ustr[user].quote          =t_ustr.quote;
ustr[user].hilite         =t_ustr.hilite;
ustr[user].new_mail       =t_ustr.new_mail;
ustr[user].color          =t_ustr.color;
ustr[user].passhid        =t_ustr.passhid;

ustr[user].pbreak         =t_ustr.pbreak;
ustr[user].beeps          =t_ustr.beeps;
ustr[user].mail_warn      =t_ustr.mail_warn;
ustr[user].mail_num       =t_ustr.mail_num;
ustr[user].friend_num     =t_ustr.friend_num;
ustr[user].revokes_num    =t_ustr.revokes_num;
ustr[user].gag_num        =t_ustr.gag_num;
ustr[user].nerf_kills     =t_ustr.nerf_kills;
ustr[user].nerf_killed    =t_ustr.nerf_killed;
ustr[user].muz_time       =t_ustr.muz_time;
ustr[user].xco_time       =t_ustr.xco_time;
ustr[user].gag_time       =t_ustr.gag_time;
ustr[user].frog           =t_ustr.frog;
ustr[user].frog_time      =t_ustr.frog_time;
ustr[user].anchor         =t_ustr.anchor;
ustr[user].anchor_time    =t_ustr.anchor_time;
ustr[user].promote        =t_ustr.promote;
ustr[user].help           =t_ustr.help;
ustr[user].who            =t_ustr.who;
ustr[user].ttt_kills      =t_ustr.ttt_kills;
ustr[user].ttt_killed     =t_ustr.ttt_killed;
ustr[user].hang_wins      =t_ustr.hang_wins;
ustr[user].hang_losses    =t_ustr.hang_losses;
ustr[user].pause_login    =t_ustr.pause_login;
ustr[user].miscnum2       =t_ustr.miscnum2;
ustr[user].miscnum3       =t_ustr.miscnum3;
ustr[user].miscnum4       =t_ustr.miscnum4;
ustr[user].miscnum5       =t_ustr.miscnum5;
ustr[user].tempnum1       =t_ustr.tempnum1;
ustr[user].term_type      =t_ustr.term_type;
strcpy(ustr[user].icq,t_ustr.icq);
strcpy(ustr[user].miscstr1,t_ustr.miscstr1);
strcpy(ustr[user].miscstr2,t_ustr.miscstr2);
strcpy(ustr[user].miscstr3,t_ustr.miscstr3);
strcpy(ustr[user].miscstr4,t_ustr.miscstr4);
strcpy(ustr[user].prev_term_type,t_ustr.prev_term_type);

i=0;
for (i=0; i<MAX_GRAVOKES; i++) {
 if (strlen(t_ustr.revokes[i]))
  strcpy(ustr[user].revokes[i],t_ustr.revokes[i]);
 else
  ustr[user].revokes[i][0]=0;
 }

return;
}

/*----------------------------------------------------------------------*/
/* copy structures from an online user to a temp buffer                 */
/*----------------------------------------------------------------------*/
void copy_from_user(int user)
{
int i=0;
MacroPtr tmpUserMacros = ustr[user].Macros;
MacroPtr tmpMacros = t_ustr.Macros;

strcpy(t_ustr.name,ustr[user].name);
strcpy(t_ustr.say_name,ustr[user].say_name);
strcpy(t_ustr.password,ustr[user].password);

t_ustr.super = ustr[user].super;

strcpy(t_ustr.email_addr, ustr[user].email_addr);
strcpy(t_ustr.desc,       ustr[user].desc);
strcpy(t_ustr.sex,        ustr[user].sex);
strcpy(t_ustr.init_date,  ustr[user].init_date);
strcpy(t_ustr.last_date,  ustr[user].last_date);
strcpy(t_ustr.init_site,  ustr[user].init_site);
strcpy(t_ustr.last_site,  ustr[user].last_site);
strcpy(t_ustr.last_name,  ustr[user].last_name);
strcpy(t_ustr.init_netname,  ustr[user].init_netname);

while (strlen(ustr[user].custAbbrs[i].com) > 1) {
  strcpy(t_ustr.custAbbrs[i].abbr,ustr[user].custAbbrs[i].abbr);
  strcpy(t_ustr.custAbbrs[i].com, ustr[user].custAbbrs[i].com);
  i++;
 }
i=0;
for (i=0;i<NUM_MACROS;i++) {
 if (tmpUserMacros && strlen(tmpUserMacros->name[i])) {
  strcpy(tmpMacros->name[i],tmpUserMacros->name[i]);
  strcpy(tmpMacros->body[i],tmpUserMacros->body[i]);
 }
 else {
  tmpMacros->name[i][0]=0;
  tmpMacros->body[i][0]=0;
 }
}
i=0;
for (i=0; i<MAX_ALERT; i++) {
 if (strlen(ustr[user].friends[i]))
  strcpy(t_ustr.friends[i],ustr[user].friends[i]);
 else
  t_ustr.friends[i][0]=0;
}
i=0;
for (i=0; i<MAX_GAG; i++) {
 if (strlen(ustr[user].gagged[i]))
  strcpy(t_ustr.gagged[i],ustr[user].gagged[i]);
 else
  t_ustr.gagged[i][0]=0;
}

t_ustr.area            =ustr[user].area;
t_ustr.shout           =ustr[user].shout;
t_ustr.vis             =ustr[user].vis;
t_ustr.locked          =ustr[user].locked;
t_ustr.suspended       =ustr[user].suspended;

strcpy(t_ustr.entermsg, ustr[user].entermsg);
strcpy(t_ustr.exitmsg, ustr[user].exitmsg);
strcpy(t_ustr.home_room, ustr[user].home_room);
strcpy(t_ustr.fail, ustr[user].fail);
strcpy(t_ustr.succ, ustr[user].succ);
strcpy(t_ustr.homepage, ustr[user].homepage);
strcpy(t_ustr.creation, ustr[user].creation);
strcpy(t_ustr.security, ustr[user].security);
strcpy(t_ustr.flags, ustr[user].flags);
strcpy(t_ustr.webpic, ustr[user].webpic);

t_ustr.numcoms         =ustr[user].numcoms;
t_ustr.totl            =ustr[user].totl;
t_ustr.rawtime         =ustr[user].rawtime;

t_ustr.monitor         =ustr[user].monitor;
t_ustr.rows            =ustr[user].rows;
t_ustr.cols            =ustr[user].cols;
t_ustr.car_return      =ustr[user].car_return;
t_ustr.abbrs           =ustr[user].abbrs;
t_ustr.times_on        =ustr[user].times_on;
t_ustr.white_space     =ustr[user].white_space;
t_ustr.aver            =ustr[user].aver;
t_ustr.autor           =ustr[user].autor;
t_ustr.autof           =ustr[user].autof;
t_ustr.automsgs        =ustr[user].automsgs;
t_ustr.gagcomm         =ustr[user].gagcomm;
t_ustr.semail          =ustr[user].semail;
t_ustr.quote           =ustr[user].quote;
t_ustr.hilite          =ustr[user].hilite;
t_ustr.new_mail        =ustr[user].new_mail;
t_ustr.color           =ustr[user].color;
t_ustr.passhid         =ustr[user].passhid;

t_ustr.pbreak          =ustr[user].pbreak;
t_ustr.beeps           =ustr[user].beeps;
t_ustr.mail_warn       =ustr[user].mail_warn;
t_ustr.mail_num        =ustr[user].mail_num;
t_ustr.friend_num      =ustr[user].friend_num;
t_ustr.revokes_num     =ustr[user].revokes_num;
t_ustr.gag_num         =ustr[user].gag_num;
t_ustr.nerf_kills      =ustr[user].nerf_kills;
t_ustr.nerf_killed     =ustr[user].nerf_killed;
t_ustr.muz_time        =ustr[user].muz_time;
t_ustr.xco_time        =ustr[user].xco_time;
t_ustr.gag_time        =ustr[user].gag_time;
t_ustr.frog            =ustr[user].frog;
t_ustr.frog_time       =ustr[user].frog_time;
t_ustr.anchor          =ustr[user].anchor;
t_ustr.anchor_time     =ustr[user].anchor_time;
t_ustr.promote         =ustr[user].promote;
t_ustr.help            =ustr[user].help;
t_ustr.who             =ustr[user].who;
t_ustr.ttt_kills       =ustr[user].ttt_kills;
t_ustr.ttt_killed      =ustr[user].ttt_killed;
t_ustr.hang_wins       =ustr[user].hang_wins;
t_ustr.hang_losses     =ustr[user].hang_losses;
t_ustr.pause_login     =ustr[user].pause_login;
t_ustr.miscnum2        =ustr[user].miscnum2;
t_ustr.miscnum3        =ustr[user].miscnum3;
t_ustr.miscnum4        =ustr[user].miscnum4;
t_ustr.miscnum5        =ustr[user].miscnum5;
t_ustr.tempnum1        =ustr[user].tempnum1;
t_ustr.term_type       =ustr[user].term_type;
strcpy(t_ustr.icq,ustr[user].icq);
strcpy(t_ustr.miscstr1,ustr[user].miscstr1);
strcpy(t_ustr.miscstr2,ustr[user].miscstr2);
strcpy(t_ustr.miscstr3,ustr[user].miscstr3);
strcpy(t_ustr.miscstr4,ustr[user].miscstr4);
strcpy(t_ustr.prev_term_type,ustr[user].prev_term_type);

i=0;
for (i=0; i<MAX_GRAVOKES; i++) {
 if (strlen(ustr[user].revokes[i]))
  strcpy(t_ustr.revokes[i],ustr[user].revokes[i]);
 else
  t_ustr.revokes[i][0]=0;
}

return;
}


/*----------------------------------------------------------------------*/
/* read the users data file into the temp buffer                        */
/*----------------------------------------------------------------------*/
int read_user(char *name)
{
int i=0;
int l=0;
int num=0;
char buff1[ARR_SIZE];
char filename[FILE_NAME_LEN];
char z_mess[FILE_NAME_LEN+NAME_LEN+1];
FILE *f;                 /* user file*/
MacroPtr tmpMacros = t_ustr.Macros;
struct stat fileinfo;

buff1[0]=0;

if (!strlen(name)) return 0;

sprintf(z_mess,"%s/%s",USERDIR,name);
strncpy(filename,z_mess,FILE_NAME_LEN);

f = fopen (filename, "r"); /* open for output */
if (f == NULL)
  {
    return 0;
  }

stat(filename, &fileinfo);

if (fileinfo.st_size == 0) {
 fclose(f);
 remove(filename);
 write_log(WARNLOG,YESTIME,"Found 0 length user file for %s in read_user..removing.\n",name);
 return -1;
 }

/*--------------------------------------------------------*/
/* values added after initial release must be initialized */
/*--------------------------------------------------------*/
t_ustr.numcoms       = 0;
t_ustr.totl          = 0;
t_ustr.rawtime       = 0;

t_ustr.monitor       = 0;
t_ustr.rows          = 24;
t_ustr.cols          = 256;
t_ustr.car_return    = 1;
t_ustr.abbrs         = 1;
t_ustr.white_space   = 1;
t_ustr.times_on      = 1;
t_ustr.aver          = 0;
t_ustr.autor         = 0;
t_ustr.autof         = 0;
t_ustr.automsgs      = 0;
t_ustr.gagcomm       = 0;
t_ustr.semail        = 0;
t_ustr.quote         = 1;
t_ustr.hilite        = 0;
t_ustr.new_mail      = 0;
t_ustr.color         = COLOR_DEFAULT;
t_ustr.passhid       = 0;

t_ustr.pbreak        = 0;
t_ustr.beeps         = 0;
t_ustr.mail_warn     = 0;
t_ustr.mail_num      = 0;
t_ustr.friend_num    = 0;
t_ustr.revokes_num   = 0;
t_ustr.gag_num       = 0;
t_ustr.nerf_kills    = 0;
t_ustr.nerf_killed   = 0;
t_ustr.muz_time      = 0;
t_ustr.xco_time      = 0;
t_ustr.gag_time      = 0;
t_ustr.frog          = 0;
t_ustr.frog_time     = 0;
t_ustr.anchor        = 0;
t_ustr.anchor_time   = 0;
t_ustr.promote       = 0;
t_ustr.ttt_kills     = 0;
t_ustr.ttt_killed    = 0;
t_ustr.hang_wins     = 0;
t_ustr.hang_losses   = 0;
t_ustr.pause_login   = 0;
t_ustr.miscnum2      = 0;
t_ustr.miscnum3      = 0;
t_ustr.miscnum4      = 0;
t_ustr.miscnum5      = 0;
listen_all(-1);

/* first line is either version number or users name */
rbuf(buff1,NAME_LEN);
if (strstr(buff1,".ver")) {
	/* this is a standardized file format */
	/* now check for version difference */
	if (!strcmp(buff1,UDATA_VERSION)) {
	  /* no difference, continue on reading */
	  rbuf(t_ustr.name,NAME_LEN);            /* users name */
	}
	else {
	  /* version difference, lets try and convert */
	  if (!convert_file(f,filename,0)) { return 0; }
	  /* reopen file after the conversion and continue on */
	  /* convert_file() closes the file		      */
	  f = fopen (filename, "r"); /* open for output */
		if (f == NULL)
		  {
		    return 0;
		  }

	  rbuf(buff1,NAME_LEN); /* VERSION */
	  rbuf(t_ustr.name,NAME_LEN); /* users name */
	} /* end of else */
  }
else {
	/* old data file format */
	/* we need to convert to new format */
	if (!convert_file(f,filename,1)) { return 0; }
	/* reopen file after the conversion and continue on */
	/* convert_file() closes the file		    */
	f = fopen (filename, "r"); /* open for output */
		if (f == NULL)
		  {
		    return 0;
		  }
	rbuf(buff1,NAME_LEN);		       /* VERSION */
	rbuf(t_ustr.name,NAME_LEN);            /* users name */
  }

rbuf(t_ustr.say_name,SAYNAME_LEN);        /* users properly capitalized name */
rbuf(t_ustr.password,-1);              /* users encrypted password */
rval(t_ustr.super);                    /* users level or rank */
rbuf(t_ustr.email_addr,EMAIL_LENGTH);  /* users email address */
rbuf(t_ustr.desc,DESC_LEN);            /* users description */
rbuf(t_ustr.sex,32);                   /* users gender */
rbuf(t_ustr.init_date,25);             /* users original login time */
rbuf(t_ustr.last_date,25);             /* users last login time */
rbuf(t_ustr.init_site,21);             /* users original site */
rbuf(t_ustr.last_site,21);             /* users last site */
rbuf(t_ustr.last_name,64);             /* users last hostname */
rbuf(t_ustr.init_netname,64);          /* users original hostname */

/* Clear first ..Abbrs.. line */
rbuf(buff1,-1);
strcpy(buff1,"");

for (;;) {
 rbuf(buff1,20);
 if (!strcmp(buff1,"..End abbrs..")) break;
  if (i) {
    strcpy(t_ustr.custAbbrs[l].com,buff1);
    i=0; l++; continue;
   }
  else {
    strcpy(t_ustr.custAbbrs[l].abbr,buff1);
    i=1;
   }
 }
 
i=0;
l=0;
    
/* Clear first ..Macros.. line */
rbuf(buff1,-1);
strcpy(buff1,"");
    
for (;;) {
 rbuf(buff1,MACRO_LEN);
 if (!strcmp(buff1,"..End macros..")) break;
 if (i) {
  strcpy(tmpMacros->body[num],buff1);
  l=strlen(tmpMacros->body[num]);
  tmpMacros->body[num][l]=0;
  i=0; num++; continue;
  }
 else {
  strcpy(tmpMacros->name[num],buff1);
  l=strlen(tmpMacros->name[num]);
  tmpMacros->name[num][l]=0;
  i=1;
  }
 }  

/* blank the unused slots */
for (i=num;i<NUM_MACROS;++i) {
 tmpMacros->name[i][0]=0;
 tmpMacros->body[i][0]=0;
 }

i=0;
num=0;

/* Clear first ..Friends.. line */
rbuf(buff1,-1);
strcpy(buff1,"");
    
for (;;) {
 rbuf(buff1,NAME_LEN);
 if (!strcmp(buff1,"..End friends..")) break;
 else {
  strcpy(t_ustr.friends[i],buff1);
  l=strlen(t_ustr.friends[i]);
  t_ustr.friends[i][l]=0;
  i++;
  }
 }  

for (l=i;l<MAX_ALERT;++l) t_ustr.friends[l][0]=0;

i=0;

/* Clear first ..Gagged.. line */
rbuf(buff1,-1);
strcpy(buff1,"");
    
for (;;) {
 rbuf(buff1,NAME_LEN);
 if (!strcmp(buff1,"..End gagged..")) break;
 else {
  strcpy(t_ustr.gagged[i],buff1);
  l=strlen(t_ustr.gagged[i]);
  t_ustr.gagged[i][l]=0;
  i++;
  }
 }  

for (l=i;l<MAX_GAG;++l) t_ustr.gagged[l][0]=0;

/*----------------------------------------*/
/*  users last area in and will login to  */
/*  users muzzled or not                  */
/*  users visible or not                  */
/*  users locked or not                   */
/*  users xcommed or not                  */
/*----------------------------------------*/
fscanf(f, "%d %d %d %d %d\n", &t_ustr.area, &t_ustr.shout,
         &t_ustr.vis, &t_ustr.locked, &t_ustr.suspended);

rbuf(t_ustr.entermsg,MAX_ENTERM);    /* users room enter message */
rbuf(t_ustr.exitmsg,MAX_EXITM);      /* users room exit message */
rbuf(t_ustr.home_room,NAME_LEN);     /* users home room */
rbuf(t_ustr.fail,MAX_ENTERM);        /* users fail message */
rbuf(t_ustr.succ,MAX_ENTERM);        /* users success message */
rbuf(t_ustr.homepage,HOME_LEN);      /* users homepage */

rbuf(t_ustr.creation,25);    /* users creation date */
rbuf(t_ustr.security,MAX_AREAS);    /* users room permissions */

i=0;
if (MAX_AREAS > strlen(t_ustr.security)) {
 for (i=0;i<(MAX_AREAS-strlen(t_ustr.security));++i)
  strcat(t_ustr.security,"N");
 }
i=0;

rbuf(t_ustr.flags,NUM_IGN_FLAGS+2);       /* users listening and ignoring flags */
rlong(t_ustr.numcoms);    /* users number of commands done */
rlong(t_ustr.totl);       /* users total minutes online */
rtime(t_ustr.rawtime);    /* users last login in time_t format */

/* Read rest of values, too many to document here */
fscanf(f, "%d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d\n",
         &t_ustr.monitor, &t_ustr.rows, &t_ustr.cols, &t_ustr.car_return,
         &t_ustr.abbrs, &t_ustr.times_on, &t_ustr.white_space, &t_ustr.aver,
         &t_ustr.autor, &t_ustr.autof, &t_ustr.automsgs,
         &t_ustr.gagcomm, &t_ustr.semail, &t_ustr.quote, &t_ustr.hilite,
         &t_ustr.new_mail, &t_ustr.color, &t_ustr.passhid);

fscanf(f, "%d %d %d %d %d %d %d %d %d %d %d %d %d %d\n",
         &t_ustr.pbreak, &t_ustr.mail_num, &t_ustr.friend_num, &t_ustr.gag_num,
         &t_ustr.nerf_kills, &t_ustr.nerf_killed, &t_ustr.muz_time,
         &t_ustr.xco_time, &t_ustr.gag_time, &t_ustr.frog, &t_ustr.frog_time,
         &t_ustr.promote, &t_ustr.beeps, &t_ustr.mail_warn);

fscanf(f,"%d %d %d %d %d %d\n", &t_ustr.help, &t_ustr.who, &t_ustr.anchor,
       &t_ustr.anchor_time, &t_ustr.ttt_kills, &t_ustr.ttt_killed);

rbuf(t_ustr.webpic,HOME_LEN);       /* users url for picture */

if (feof(f)) goto END;
rval(t_ustr.revokes_num);	    /* number of revoked commands */

i=0;

/* Clear first ..Revokes.. line */
rbuf(buff1,-1);
strcpy(buff1,"");
    
for (;;) {
 rbuf(buff1,NAME_LEN);
 if (!strcmp(buff1,"..End revokes..")) break;
 else {
	if (!strcmp(buff1,"-1") || (strlen(buff1)<3)) buff1[0]=0;
  strcpy(t_ustr.revokes[i],buff1);
  l=strlen(t_ustr.revokes[i]);
  t_ustr.revokes[i][l]=0;
  i++;
  }
 } 

for (l=i;l<MAX_GRAVOKES;++l) t_ustr.revokes[l][0]=0;

i=0;

rval(t_ustr.hang_wins);	    /* number of hangman wins */
rval(t_ustr.hang_losses);    /* number of hangman losses */

rbuf(t_ustr.icq,20);		/* icq number */
rbuf(t_ustr.miscstr1,10);	/* miscstr1 */
rbuf(t_ustr.miscstr2,10);	/* miscstr2 */
rbuf(t_ustr.miscstr3,10);	/* miscstr3 */
rbuf(t_ustr.miscstr4,10);	/* miscstr4 */
fscanf(f,"%d %d %d %d %d\n", &t_ustr.pause_login, &t_ustr.miscnum2,
       &t_ustr.miscnum3, &t_ustr.miscnum4, &t_ustr.miscnum5);

rbuf(buff1,-1);	/* ENDVER STRING */

/* add your own structures to read in here */


/* STOP adding your own structures to read in here */

END:
/*---------------------------------------------------------------------*/
/* check for possible bad values in the users config                   */
/*---------------------------------------------------------------------*/

/* longs */
if (t_ustr.numcoms > 10000000 || t_ustr.numcoms < 0)   t_ustr.numcoms = 1;
if (t_ustr.totl > 1439999    || t_ustr.totl < 0)        t_ustr.totl = 0;

/* ints, first set */
if (t_ustr.super > MAX_LEVEL || t_ustr.super < 0)      t_ustr.super = 0;
if (t_ustr.area > MAX_AREAS || t_ustr.area < 0)        t_ustr.area = 0;
if (t_ustr.shout > 1        || t_ustr.shout < 0)       t_ustr.shout = 1;
if (t_ustr.vis > 1          || t_ustr.vis < 0)         t_ustr.vis = 1;
if (t_ustr.locked > 1       || t_ustr.locked < 0)      t_ustr.locked = 0;
if (t_ustr.suspended > 1    || t_ustr.suspended < 0)   t_ustr.suspended = 0;

/* ints, second set */
if (t_ustr.monitor > 3      || t_ustr.monitor < 0)     t_ustr.monitor = 0;
if (t_ustr.rows > 256       || t_ustr.rows < 0)        t_ustr.rows = 24;
if (t_ustr.cols > 256       || t_ustr.cols < 0)        t_ustr.cols = 256;
if (t_ustr.car_return > 1   || t_ustr.car_return < 0)  t_ustr.car_return = 1;
if (t_ustr.abbrs > 1        || t_ustr.abbrs < 0)       t_ustr.abbrs = 0;
if (t_ustr.times_on > 32767 || t_ustr.times_on < 0)    t_ustr.times_on = 0;
if (t_ustr.white_space > 1  || t_ustr.white_space < 0) t_ustr.white_space = 0;
if (t_ustr.aver > 16000     || t_ustr.aver < 0)        t_ustr.aver = 16000;
if (t_ustr.autor > 3        || t_ustr.autor < 0)       t_ustr.autor = 0;
if (t_ustr.autof > 2        || t_ustr.autof < 0)       t_ustr.autof = 0;
if (t_ustr.automsgs > MAX_AUTOFORS        || t_ustr.automsgs < 0) t_ustr.automsgs = 0;
if (t_ustr.gagcomm > 1      || t_ustr.gagcomm < 0)     t_ustr.gagcomm = 0;
if (t_ustr.semail > 1       || t_ustr.semail < 0)      t_ustr.semail = 0;
if (t_ustr.quote > 1        || t_ustr.quote < 0)       t_ustr.quote = 0;
if (t_ustr.hilite > 2       || t_ustr.hilite < 0)      t_ustr.hilite = 0;
if (t_ustr.color > 1        || t_ustr.color < 0)       t_ustr.color = COLOR_DEFAULT;
if (t_ustr.passhid > 1      || t_ustr.passhid < 0)     t_ustr.passhid =0;

/* ints, third set */
if (t_ustr.pbreak > 1       || t_ustr.pbreak < 0)      t_ustr.pbreak =0;
if (t_ustr.beeps > 1        || t_ustr.beeps < 0)      t_ustr.beeps =0;
if (t_ustr.mail_warn > 1        || t_ustr.mail_warn < 0) t_ustr.mail_warn =0;
if (t_ustr.mail_num > 200   || t_ustr.mail_num < 0)    t_ustr.mail_num = 0;
if (t_ustr.friend_num > MAX_ALERT || t_ustr.friend_num < 0)  t_ustr.friend_num=0;
if (t_ustr.gag_num > MAX_GAG   || t_ustr.gag_num < 0)    t_ustr.gag_num =0;
if (t_ustr.revokes_num > MAX_GRAVOKES   || t_ustr.revokes_num < 0)	t_ustr.revokes_num =0;
if (t_ustr.nerf_kills > 32767     || t_ustr.nerf_kills < 0)       t_ustr.nerf_kills =0;
if (t_ustr.nerf_killed > 32767     || t_ustr.nerf_killed < 0)       t_ustr.nerf_killed =0;
if (t_ustr.muz_time > 32767     || t_ustr.muz_time < 0)       t_ustr.muz_time =0;
if (t_ustr.xco_time > 32767      || t_ustr.xco_time < 0)       t_ustr.xco_time =0;
if (t_ustr.gag_time > 32767      || t_ustr.gag_time < 0)  t_ustr.gag_time =0;
if (t_ustr.frog > 1      || t_ustr.frog < 0)       t_ustr.frog =0;
if (t_ustr.frog_time > 32767      || t_ustr.frog_time < 0)  t_ustr.frog_time =0;
if (t_ustr.anchor > 1      || t_ustr.anchor < 0)       t_ustr.anchor =0;
if (t_ustr.anchor_time > 32767      || t_ustr.anchor_time < 0) t_ustr.anchor_time =0;
if (t_ustr.promote > 22      || t_ustr.promote < 0)       t_ustr.promote=1;
if (t_ustr.ttt_kills > 32767     || t_ustr.ttt_kills < 0)       t_ustr.ttt_kills =0;
if (t_ustr.ttt_killed > 32767     || t_ustr.ttt_killed < 0)       t_ustr.ttt_killed =0;
if (t_ustr.hang_wins > 32767     || t_ustr.hang_wins < 0)       t_ustr.hang_wins =0;
if (t_ustr.hang_losses > 32767     || t_ustr.hang_losses < 0)       t_ustr.hang_losses =0;
if (t_ustr.pause_login > 32767     || t_ustr.pause_login < 0)	t_ustr.pause_login=0;
if (t_ustr.miscnum2 > 32767     || t_ustr.miscnum2 < 0)	t_ustr.miscnum2=0;
if (t_ustr.miscnum3 > 32767     || t_ustr.miscnum3 < 0)	t_ustr.miscnum3=0;
if (t_ustr.miscnum4 > 32767     || t_ustr.miscnum4 < 0)	t_ustr.miscnum4=0;
if (t_ustr.miscnum5 > 32767     || t_ustr.miscnum5 < 0)	t_ustr.miscnum5=0;

FCLOSE(f);
return 1;

}



/*----------------------------------------------------------------------*/
/* read the users data file to the online users strcuture               */
/*----------------------------------------------------------------------*/
int read_to_user(char *name, int user)
{
int i=0;
int l=0;
int num=0;
char buff1[ARR_SIZE];
char filename[FILE_NAME_LEN];
FILE *f;                 /* user file */
MacroPtr tmpMacros = ustr[user].Macros;
struct stat fileinfo;

buff1[0]=0;

if (!strlen(name)) return 0;

sprintf(t_mess,"%s/%s",USERDIR,name);
strncpy(filename,t_mess,FILE_NAME_LEN);

f = fopen (filename, "r"); /* open for output */
if (f == NULL)
  {
    return 0;
  }

stat(filename, &fileinfo);

if (fileinfo.st_size == 0) {
 fclose(f);
 remove(filename);
 write_log(WARNLOG,YESTIME,"Found 0 length user file for %s in read_to_user..removing.\n",name);
 return -1;
 }


/*--------------------------------------------------------*/
/* values added after initial release must be initialized */
/*--------------------------------------------------------*/
ustr[user].numcoms       = 0;
ustr[user].totl          = 0;
ustr[user].rawtime       = 0;

ustr[user].monitor       = 0;
ustr[user].rows          = 24;
ustr[user].cols          = 256;
ustr[user].car_return    = 1;
ustr[user].abbrs         = 1;
ustr[user].white_space   = 1;
ustr[user].times_on      = 1;
ustr[user].aver          = 0;
ustr[user].autor         = 0;
ustr[user].autof         = 0;
ustr[user].automsgs      = 0;
ustr[user].gagcomm       = 0;
ustr[user].semail        = 0;
ustr[user].quote         = 1;
ustr[user].hilite        = 0;
ustr[user].new_mail      = 0;
ustr[user].color         = COLOR_DEFAULT;
ustr[user].passhid       = 0;

ustr[user].pbreak        = 0;
ustr[user].beeps         = 0;
ustr[user].mail_warn     = 0;
ustr[user].mail_num      = 0;
ustr[user].friend_num    = 0;
ustr[user].revokes_num   = 0;
ustr[user].gag_num       = 0;
ustr[user].nerf_kills    = 0;
ustr[user].nerf_killed   = 0;
ustr[user].muz_time      = 0;
ustr[user].xco_time      = 0;
ustr[user].gag_time      = 0;
ustr[user].frog          = 0;
ustr[user].frog_time     = 0;
ustr[user].anchor        = 0;
ustr[user].anchor_time   = 0;
ustr[user].promote       = 0;
ustr[user].help          = 0;
ustr[user].who           = 0;
ustr[user].ttt_kills     = 0;
ustr[user].ttt_killed    = 0;
ustr[user].hang_wins     = 0;
ustr[user].hang_losses   = 0;
ustr[user].pause_login   = 0;
ustr[user].miscnum2      = 0;
ustr[user].miscnum3      = 0;
ustr[user].miscnum4      = 0;
ustr[user].miscnum5      = 0;
listen_all(-1);

/* first line is either version number or users name */
rbuf(buff1,NAME_LEN);
if (strstr(buff1,".ver")) {
	/* this is a standardized file format */
	/* now check for version difference */
	if (!strcmp(buff1,UDATA_VERSION)) {
	  /* no difference, continue on reading */
	  rbuf(ustr[user].name,NAME_LEN);            /* users name */
	}
	else {
	  /* version difference, lets try and convert */
	  if (!convert_file(f,filename,0)) { return 0; }
	  /* reopen file after the conversion and continue on */
	  /* convert_file() closes the file		      */
	  f = fopen (filename, "r"); /* open for output */
		if (f == NULL)
		  {
		    return 0;
		  }

	  rbuf(buff1,NAME_LEN); /* VERSION */
	  rbuf(ustr[user].name,NAME_LEN); /* users name */
	} /* end of else */
  }
else {
	/* old data file format */
	/* we need to convert to new format */
	if (!convert_file(f,filename,1)) { return 0; }
	/* reopen file after the conversion and continue on */
	/* convert_file() closes the file		    */
	f = fopen (filename, "r"); /* open for output */
		if (f == NULL)
		  {
		    return 0;
		  }
	rbuf(buff1,NAME_LEN);		       /* VERSION */
	rbuf(ustr[user].name,NAME_LEN);            /* users name */
  }

rbuf(ustr[user].say_name,SAYNAME_LEN);       /* users properly capitalized name */
rbuf(ustr[user].password,-1);             /* users encrypted password */
rval(ustr[user].super);                   /* users level or rank */
rbuf(ustr[user].email_addr,EMAIL_LENGTH); /* users email address */
rbuf(ustr[user].desc,DESC_LEN);           /* users description */
rbuf(ustr[user].sex,32);                  /* users gender */
rbuf(ustr[user].init_date,25);            /* users original login time */
rbuf(ustr[user].last_date,25);            /* users last login time */
rbuf(ustr[user].init_site,21);            /* users original site */
rbuf(ustr[user].last_site,21);            /* users last site */
rbuf(ustr[user].last_name,64);            /* users last hostname */
rbuf(ustr[user].init_netname,64);         /* users original hostname */

/* Clear first ..Abbrs.. line */
rbuf(buff1,-1);
strcpy(buff1,"");

for (;;) {
 rbuf(buff1,20);
 if (!strcmp(buff1,"..End abbrs..")) break;
  if (i) {
    strcpy(ustr[user].custAbbrs[l].com,buff1);
    i=0; l++; continue;
   }
  else {
    strcpy(ustr[user].custAbbrs[l].abbr,buff1);
    i=1;
   }
 }

i=0;
l=0;
  
/* Clear first ..Macros.. line */
rbuf(buff1,-1);
strcpy(buff1,"");
  
for (;;) {
 rbuf(buff1,MACRO_LEN);
 if (!strcmp(buff1,"..End macros..")) break;
 if (i) {
  strcpy(tmpMacros->body[num],buff1);
  l=strlen(tmpMacros->body[num]);
  tmpMacros->body[num][l]=0;
  i=0; num++; continue;
  }
 else {
  strcpy(tmpMacros->name[num],buff1);
  l=strlen(tmpMacros->name[num]);
  tmpMacros->name[num][l]=0;
  i=1;
  }
 }

/* blank the unused slots */
for (i=num;i<NUM_MACROS;++i) {
 tmpMacros->name[i][0]=0;
 tmpMacros->body[i][0]=0;
 }

i=0;
num=0;

/* Clear first ..Friends.. line */
rbuf(buff1,-1);
strcpy(buff1,"");
    
for (;;) {
 rbuf(buff1,NAME_LEN);
 if (!strcmp(buff1,"..End friends..")) break;
 else {
  strcpy(ustr[user].friends[i],buff1);
  l=strlen(ustr[user].friends[i]);
  ustr[user].friends[i][l]=0;
  i++;
  }
 }  

for (l=i;l<MAX_ALERT;++l) ustr[user].friends[l][0]=0;

i=0;

/* Clear first ..Gagged.. line */
rbuf(buff1,-1);
strcpy(buff1,"");
    
for (;;) {
 rbuf(buff1,NAME_LEN);
 if (!strcmp(buff1,"..End gagged..")) break;
 else {
  strcpy(ustr[user].gagged[i],buff1);
  l=strlen(ustr[user].gagged[i]);
  ustr[user].gagged[i][l]=0;
  i++;
  }
 }  

for (l=i;l<MAX_GAG;++l) ustr[user].gagged[l][0]=0;

/*----------------------------------------*/
/*  users last area in and will login to  */
/*  users muzzled or not                  */
/*  users visible or not                  */
/*  users locked or not                   */
/*  users xcommed or not                  */
/*----------------------------------------*/
fscanf(f, "%d %d %d %d %d\n", &ustr[user].area, &ustr[user].shout,
         &ustr[user].vis, &ustr[user].locked, &ustr[user].suspended);

rbuf(ustr[user].entermsg,MAX_ENTERM);   /* users room enter message */
rbuf(ustr[user].exitmsg,MAX_EXITM);     /* users room exit message */
rbuf(ustr[user].home_room,NAME_LEN);    /* users home room */
rbuf(ustr[user].fail,MAX_ENTERM);       /* users fail message */
rbuf(ustr[user].succ,MAX_ENTERM);       /* users success message */
rbuf(ustr[user].homepage,HOME_LEN);     /* users homepage */
rbuf(ustr[user].creation,25);           /* users creation date */
rbuf(ustr[user].security,MAX_AREAS);    /* users room permissions */

i=0;
if (MAX_AREAS > strlen(ustr[user].security)) {
 for (i=0;i<(MAX_AREAS-strlen(ustr[user].security));++i)
  strcat(ustr[user].security,"N");
 }
i=0;

rbuf(ustr[user].flags,NUM_IGN_FLAGS+2); /* users listening and ignoring flags */
rlong(ustr[user].numcoms);              /* users number of commands done */
rlong(ustr[user].totl);                 /* users total minutes online */
rtime(ustr[user].rawtime);              /* users last login in time_t format */

/* Read rest of values, too many to document here */
fscanf(f, "%d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d\n",
         &ustr[user].monitor, &ustr[user].rows, &ustr[user].cols,
         &ustr[user].car_return, &ustr[user].abbrs, &ustr[user].times_on,
         &ustr[user].white_space, &ustr[user].aver, &ustr[user].autor,
         &ustr[user].autof, &ustr[user].automsgs, &ustr[user].gagcomm,
         &ustr[user].semail, &ustr[user].quote, &ustr[user].hilite,
         &ustr[user].new_mail, &ustr[user].color, &ustr[user].passhid);

fscanf(f, "%d %d %d %d %d %d %d %d %d %d %d %d %d %d\n",
         &ustr[user].pbreak, &ustr[user].mail_num, &ustr[user].friend_num,
         &ustr[user].gag_num, &ustr[user].nerf_kills, &ustr[user].nerf_killed,
         &ustr[user].muz_time, &ustr[user].xco_time, &ustr[user].gag_time,
         &ustr[user].frog, &ustr[user].frog_time, &ustr[user].promote,
         &ustr[user].beeps, &ustr[user].mail_warn);

fscanf(f,"%d %d %d %d %d %d\n",&ustr[user].help,&ustr[user].who,
&ustr[user].anchor,&ustr[user].anchor_time,&ustr[user].ttt_kills,&ustr[user].ttt_killed);

rbuf(ustr[user].webpic,HOME_LEN);       /* users url for picture */

if (feof(f)) goto END;
rval(ustr[user].revokes_num);		/* number of revoked commands */

i = 0;

/* Clear first ..Revokes.. line */
rbuf(buff1,-1);
strcpy(buff1,"");

for (;;) {
 rbuf(buff1,NAME_LEN);
 if (!strcmp(buff1,"..End revokes..")) break;
 else {
	if (!strcmp(buff1,"-1") || (strlen(buff1)<3)) buff1[0]=0;
  strcpy(ustr[user].revokes[i],buff1);
  l=strlen(ustr[user].revokes[i]);
  ustr[user].revokes[i][l]=0;
  i++;
  }
 }

for (l=i;l<MAX_GRAVOKES;++l) ustr[user].revokes[l][0]=0;

i=0;

rval(ustr[user].hang_wins);		/* number of hangman wins */
rval(ustr[user].hang_losses);		/* number of hangman losses */

rbuf(ustr[user].icq,20);            /* icq number */
rbuf(ustr[user].miscstr1,10);       /* miscstr1 */
rbuf(ustr[user].miscstr2,10);       /* miscstr2 */
rbuf(ustr[user].miscstr3,10);       /* miscstr3 */
rbuf(ustr[user].miscstr4,10);       /* miscstr4 */
fscanf(f,"%d %d %d %d %d\n", &ustr[user].pause_login, &ustr[user].miscnum2,       
       &ustr[user].miscnum3, &ustr[user].miscnum4, &ustr[user].miscnum5);

rbuf(buff1,-1);	/* ENDVER STRING */

/* add your own structures to read in here */


/* STOP adding your own structures to read in here */

END:
/*---------------------------------------------------------------------*/
/* check for possible bad values in the users config                   */
/*---------------------------------------------------------------------*/

/* longs */
if (ustr[user].numcoms > 10000000 || ustr[user].numcoms < 0) ustr[user].numcoms = 1;
if (ustr[user].totl > 1439999    || ustr[user].totl < 0) ustr[user].totl = 0;

/* ints, first set */
if (ustr[user].super > MAX_LEVEL || ustr[user].super < 0)      ustr[user].super = 0;
if (ustr[user].area > MAX_AREAS || ustr[user].area < 0)        ustr[user].area = 0;
if (ustr[user].shout > 1        || ustr[user].shout < 0)       ustr[user].shout = 0;
if (ustr[user].vis > 1          || ustr[user].vis < 0)         ustr[user].vis = 0;
if (ustr[user].locked > 1       || ustr[user].locked < 0)      ustr[user].locked = 0;
if (ustr[user].suspended > 1    || ustr[user].suspended < 0)   ustr[user].suspended = 0;

/* ints, second set */
if (ustr[user].monitor > 3      || ustr[user].monitor < 0)     ustr[user].monitor = 0;
if (ustr[user].rows > 256       || ustr[user].rows < 0)        ustr[user].rows = 24;
if (ustr[user].cols > 256       || ustr[user].cols < 0)        ustr[user].cols = 256;
if (ustr[user].car_return > 1   || ustr[user].car_return < 0)  ustr[user].car_return = 1;
if (ustr[user].abbrs > 1        || ustr[user].abbrs < 0)       ustr[user].abbrs = 0;
if (ustr[user].times_on > 32767 || ustr[user].times_on < 0)    ustr[user].times_on = 0;
if (ustr[user].white_space > 1  || ustr[user].white_space < 0) ustr[user].white_space = 0;
if (ustr[user].aver > 16000     || ustr[user].aver < 0)        ustr[user].aver = 16000;
if (ustr[user].autor > 3        || ustr[user].autor < 0)       ustr[user].autor = 0;
if (ustr[user].autof > 2        || ustr[user].autof < 0)       ustr[user].autof = 0;
if (ustr[user].automsgs > MAX_AUTOFORS  || ustr[user].automsgs < 0) ustr[user].automsgs = 0;
if (ustr[user].gagcomm > 1      || ustr[user].gagcomm < 0)     ustr[user].gagcomm = 0;
if (ustr[user].semail > 1       || ustr[user].semail < 0)      ustr[user].semail = 0;
if (ustr[user].quote > 1        || ustr[user].quote < 0)       ustr[user].quote = 0;
if (ustr[user].hilite > 2       || ustr[user].hilite < 0)      ustr[user].hilite = 0;
if (ustr[user].color > 1        || ustr[user].color < 0)       ustr[user].color = COLOR_DEFAULT;
if (ustr[user].passhid > 1      || ustr[user].passhid < 0)     ustr[user].passhid = 0; 

/* ints, third set */
if (ustr[user].pbreak > 1       || ustr[user].pbreak < 0)      ustr[user].pbreak = 0; 
if (ustr[user].beeps > 1        || ustr[user].beeps < 0)       ustr[user].beeps = 0; 
if (ustr[user].mail_warn > 1    || ustr[user].mail_warn < 0) ustr[user].mail_warn = 0;
if (ustr[user].mail_num > 200   || ustr[user].mail_num < 0)    ustr[user].mail_num = 0;
if (ustr[user].friend_num > MAX_ALERT || ustr[user].friend_num < 0 ) ustr[user].friend_num=0;
if (ustr[user].gag_num > MAX_GAG || ustr[user].gag_num < 0) ustr[user].gag_num =0;
if (ustr[user].revokes_num > MAX_GRAVOKES || ustr[user].revokes_num < 0)	ustr[user].revokes_num =0;
if (ustr[user].nerf_kills > 32767 || ustr[user].nerf_kills < 0)  ustr[user].nerf_kills =0;
if (ustr[user].nerf_killed > 32767 || ustr[user].nerf_killed < 0)  ustr[user].nerf_killed =0;
if (ustr[user].muz_time > 32767 || ustr[user].muz_time < 0)  ustr[user].muz_time =0;
if (ustr[user].xco_time > 32767 || ustr[user].xco_time < 0)   ustr[user].xco_time =0;
if (ustr[user].gag_time > 32767 || ustr[user].gag_time < 0)   ustr[user].gag_time =0;
if (ustr[user].frog > 1 || ustr[user].frog < 0)    ustr[user].frog =0;
if (ustr[user].frog_time > 32767 || ustr[user].frog_time < 0)  ustr[user].frog_time =0;
if (ustr[user].anchor > 1 || ustr[user].anchor < 0)    ustr[user].anchor =0;
if (ustr[user].anchor_time > 32767 || ustr[user].anchor_time < 0) ustr[user].anchor_time =0;
if (ustr[user].promote > 22 || ustr[user].promote < 0)  ustr[user].promote=1;
if (ustr[user].help > 3 || ustr[user].help < 0)  ustr[user].help=0;
if (ustr[user].who > 4 || ustr[user].who < 0)  ustr[user].who=0;
if (ustr[user].ttt_kills > 32767 || ustr[user].ttt_kills < 0)  ustr[user].ttt_kills =0;
if (ustr[user].ttt_killed > 32767 || ustr[user].ttt_killed < 0)  ustr[user].ttt_killed =0;
if (ustr[user].hang_wins > 32767 || ustr[user].hang_wins < 0)  ustr[user].hang_wins =0;
if (ustr[user].hang_losses > 32767 || ustr[user].hang_losses < 0)  ustr[user].hang_losses =0;
if (ustr[user].pause_login  > 32767 || ustr[user].pause_login < 0)  ustr[user].pause_login=0;
if (ustr[user].miscnum2  > 32767 || ustr[user].miscnum2 < 0) ustr[user].miscnum2=0;
if (ustr[user].miscnum3  > 32767 || ustr[user].miscnum3 < 0) ustr[user].miscnum3=0;
if (ustr[user].miscnum4  > 32767 || ustr[user].miscnum4 < 0) ustr[user].miscnum4=0;
if (ustr[user].miscnum5  > 32767 || ustr[user].miscnum5 < 0) ustr[user].miscnum5=0;

FCLOSE(f);
return 1;
}

/*----------------------------------------------------------------------*/
/* write a user temp buffer to a users data file                        */
/*----------------------------------------------------------------------*/
void write_user(char *name)
{
int i=0;
FILE *f;                 /* user file*/
char filename[FILE_NAME_LEN];
char buff1[25];
MacroPtr tmpMacros = t_ustr.Macros;

buff1[0]=0;

sprintf(t_mess,"%s/%s",USERDIR,name);
strncpy(filename,t_mess,FILE_NAME_LEN);

/* open for output */
if (!(f=fopen(filename, "w"))) {
   write_log(ERRLOG,YESTIME,"Couldn't open file(w) \"%s\" in write_user! %s\n",filename,get_error());
   return;
  }

wbuf(UDATA_VERSION);	  /* FILE VERSION */
wbuf(t_ustr.name);        /* users name */
wbuf(t_ustr.say_name);    /* users properly capitalized name */
wbuf(t_ustr.password);    /* users excrypted password */
wval(t_ustr.super);       /* users level or rank */
wbuf(t_ustr.email_addr);  /* users email address */
wbuf(t_ustr.desc);        /* users description */
wbuf(t_ustr.sex);         /* users gender */
wbuf(t_ustr.init_date);   /* users original login time */
wbuf(t_ustr.last_date);   /* users last login time */
wbuf(t_ustr.init_site);   /* users original site */
wbuf(t_ustr.last_site);   /* users last site */
wbuf(t_ustr.last_name);   /* users last hostname */
wbuf(t_ustr.init_netname);   /* users original hostname */

/*
  while (strlen(t_ustr.custAbbrs[i].com) > 1) {  
    wbuf(t_ustr.custAbbrs[i].abbr);
    wbuf(t_ustr.custAbbrs[i].com);
	write_log(DEBUGLOG,YESTIME,"Abbr %d: %s\n",i,t_ustr.custAbbrs[i].abbr);
	write_log(DEBUGLOG,YESTIME,"Com  %d: %s\n",i,t_ustr.custAbbrs[i].com);
    i++;
   }
*/

   wbuf("..Abbrs..");
   for (i=0;i<NUM_ABBRS;++i) {
    wbuf(t_ustr.custAbbrs[i].abbr);
    wbuf(t_ustr.custAbbrs[i].com);
   }
   wbuf("..End abbrs..");
i=0;
   wbuf("..Macros..");
   for (i=0;i<NUM_MACROS;i++) {
    wbuf(tmpMacros->name[i]);
    wbuf(tmpMacros->body[i]);
    }
   wbuf("..End macros..");
i=0;
   wbuf("..Friends..");
   for (i=0;i<MAX_ALERT;i++) wbuf(t_ustr.friends[i]);
   wbuf("..End friends..");
i=0;
   wbuf("..Gagged..");
   for (i=0;i<MAX_GAG;i++) wbuf(t_ustr.gagged[i]);
   wbuf("..End gagged..");
i=0;

/*----------------------------------------*/
/*  users last area in and will login to  */
/*  users muzzled or not                  */
/*  users visible or not                  */
/*  users locked or not                   */
/*  users xcommed or not                  */
/*----------------------------------------*/
fprintf(f, "%d %d %d %d %d\n", t_ustr.area, t_ustr.shout,
           t_ustr.vis, t_ustr.locked, t_ustr.suspended);

wbuf(t_ustr.entermsg);    /* users room enter message */
wbuf(t_ustr.exitmsg);     /* users room exit message */
wbuf(t_ustr.home_room);   /* users home room */
wbuf(t_ustr.fail);        /* users fail message */
wbuf(t_ustr.succ);        /* users success message */
wbuf(t_ustr.homepage);    /* users homepage */
wbuf(t_ustr.creation);    /* users creation date */
wbuf(t_ustr.security);    /* users room permissions */
wbuf(t_ustr.flags);       /* users listening and ignoring flags */
wlong(t_ustr.numcoms);    /* users number of commands done */
wlong(t_ustr.totl);       /* users total minutes online */
wtime(t_ustr.rawtime);    /* users last login in time_t format */

/* Read rest of values, too many to document here */
fprintf(f, "%d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d\n",
          t_ustr.monitor, t_ustr.rows, t_ustr.cols, t_ustr.car_return,
          t_ustr.abbrs, t_ustr.times_on, t_ustr.white_space, t_ustr.aver,
          t_ustr.autor, t_ustr.autof, t_ustr.automsgs,
          t_ustr.gagcomm, t_ustr.semail, t_ustr.quote, t_ustr.hilite,
          t_ustr.new_mail, t_ustr.color, t_ustr.passhid);

fprintf(f, "%d %d %d %d %d %d %d %d %d %d %d %d %d %d\n",
          t_ustr.pbreak, t_ustr.mail_num, t_ustr.friend_num, t_ustr.gag_num,
          t_ustr.nerf_kills, t_ustr.nerf_killed, t_ustr.muz_time,
          t_ustr.xco_time, t_ustr.gag_time, t_ustr.frog, t_ustr.frog_time,
          t_ustr.promote, t_ustr.beeps, t_ustr.mail_warn);

fprintf(f,"%d %d %d %d %d %d\n",t_ustr.help, t_ustr.who, t_ustr.anchor,
          t_ustr.anchor_time, t_ustr.ttt_kills, t_ustr.ttt_killed);

wbuf(t_ustr.webpic);       /* users url for picture */

wval(t_ustr.revokes_num);	/* number of revoked commands */

i=0;
   wbuf("..Revokes..");
   for (i=0;i<MAX_GRAVOKES;i++) wbuf(t_ustr.revokes[i]);
   wbuf("..End revokes..");

wval(t_ustr.hang_wins);		/* number of hangman wins */
wval(t_ustr.hang_losses);	/* number of hangman losses */

wbuf(t_ustr.icq);            /* icq number */
wbuf(t_ustr.miscstr1);       /* miscstr1 */
wbuf(t_ustr.miscstr2);       /* miscstr2 */
wbuf(t_ustr.miscstr3);       /* miscstr3 */
wbuf(t_ustr.miscstr4);       /* miscstr4 */
fprintf(f,"%d %d %d %d %d\n", t_ustr.pause_login, t_ustr.miscnum2,       
       t_ustr.miscnum3, t_ustr.miscnum4, t_ustr.miscnum5);      

/* tack on marker */
sprintf(buff1,"--ENDVER %s",UDATA_VERSION);
wbuf(buff1);

/* add your own structures to write out, here */


/* STOP adding your own structures to write out, here */

FCLOSE(f);
return;

}


/*-------------------------------------------------------------------------*/
/* check to see if the user exists                                         */
/*-------------------------------------------------------------------------*/
int check_for_user(char *name)
{
char filename[ARR_SIZE];
FILE * bfp;

sprintf(filename,"%s/%s",USERDIR,name);
bfp = fopen(filename, "r");

if (bfp) 
  {
    FCLOSE(bfp);     
    return 1;
  }
return 0;
}

/*-------------------------------------------------------------------------*/
/* check to see if the username is already in the process of creatio       */
/*-------------------------------------------------------------------------*/
int check_for_creation(char *name)
{
int u=0;

for (u=0;u<MAX_USERS;++u) {
        if (!strcmp(name,ustr[u].login_name) && 
	    ustr[u].logging_in && ustr[u].logging_in!=3) {
	/* We found a user logging in with the same name	 */
	/* and not at the username prompt, so they get		 */
	/* precedence for creation over the user we are checking */
        return u;
        }
}

return -1;
}


/*----------------------------------------*/
/* check to see if the file exists        */
/*----------------------------------------*/
int check_for_file(char *name)
{
char filename[FILE_NAME_LEN];
FILE * bfp;

sprintf(filename,"%s",name);
bfp = fopen(filename, "r");

if (bfp) 
  {
    FCLOSE(bfp);     
    return 1;
  }
return 0;
}

/*-------------------------------------------------------------------------*/
/* remove a user                                                           */
/*-------------------------------------------------------------------------*/
void remove_user(char *name)
{
char filename[FILE_NAME_LEN];

         /* Remove user's data file */
        sprintf(t_mess,"%s/%s",USERDIR,name);
        strncpy(filename,t_mess,FILE_NAME_LEN);
        remove(filename);

         /* Remove user's INBOX mail file */
        sprintf(t_mess,"%s/%s", MAILDIR, name);
        strncpy(filename, t_mess, FILE_NAME_LEN);
        remove(filename);

         /* Remove user's SENT mail file */
        sprintf(t_mess,"%s/%s.sent", MAILDIR, name);
        strncpy(filename, t_mess, FILE_NAME_LEN);
        remove(filename);

         /* Remove user's EMAIL mail file */
        sprintf(t_mess,"%s/%s.email", MAILDIR, name);
        strncpy(filename, t_mess, FILE_NAME_LEN);
        remove(filename);

         /* Remove user's PROFILE file */
        sprintf(t_mess,"%s/%s",PRO_DIR, name);
        strncpy(filename, t_mess, FILE_NAME_LEN);
        remove(filename);

} 


/*-----------------------------------------------------*/
/* Put a string with return to a file                  */
/*-----------------------------------------------------*/
void putbuf(FILE *f, char *buf2)
{
fputs(buf2,f);
fputs("\n",f);
}


/*--------------------------------------------------------------------*/
/* a simple attempt to encrypt a password                             */
/*--------------------------------------------------------------------*/

void st_crypt(char str[])
{
int i = 0;
char last = ' ';

while(str[i])
  {str[i]= (( (str[i] - 32) + lock[(i%num_locks)] + (last - 32) ) % 94) + 32;
   last = str[i];
   i++;
   }
}



/*-----------------------------------------------------------------------*/
/* the preview command                                                   */
/*-----------------------------------------------------------------------*/
void preview(int user, char *inpstr)
{
int num=0;
char filename[FILE_NAME_LEN];
char filename2[FILE_NAME_LEN];
char buffer[ARR_SIZE];
char line[257];
FILE *fp;
FILE *pp;

if (!strlen(inpstr)) 
  {
   strcpy(inpstr,"names"); 
   }

inpstr[80]=0;
        
/* plug security hole */
if (check_fname(inpstr,user)) 
  {
   write_str(user,"Illegal name.");
   return;
  }

sscanf(inpstr,"%s ",buffer);
remove_first(inpstr);
/* open board file */
sprintf(t_mess,"%s/%s",PICTURE_DIR,buffer);
strncpy(filename,t_mess,FILE_NAME_LEN);

if (!check_for_file(filename)) {
    write_str(user,"Picture does not exist.");
    return;
    }

 if (!strlen(inpstr)) {
    cat(filename,user,0);
  }
 else {
  for (num=0;num<strlen(inpstr);num++) {
  if (!isdigit((int)inpstr[num])) {
     write_str(user,"Lines from bottom must be a number.");
     return;
     }
  }
  num=0;
  num=atoi(inpstr);
  strcpy(filename2,get_temp_file());

  sprintf(mess,"tail -%d %s",num,filename);

 if (!(pp=popen(mess,"r"))) {
	write_str(user,"Can't open pipe to get those picture lines!");
	return;
	}
 if (!(fp=fopen(filename2,"w"))) {
	write_str(user,"Can't open temp file for writing!");
	return;
	}
while (fgets(line,256,pp) != NULL) {
	fputs(line,fp);
      } /* end of while */
fclose(fp);
pclose(pp);

  cat(filename2,user,0);
  return;
 }

}

/*-----------------------------------------------------------------------*/
/* the picture command                                                   */
/*-----------------------------------------------------------------------*/
void picture(int user, char *inpstr)
{
int num=0;
char filename[FILE_NAME_LEN];
char filename2[FILE_NAME_LEN];
char buffer[ARR_SIZE];
char name[SAYNAME_LEN+1], z_mess[SAYNAME_LEN+20];
char line[257];
FILE *fp;
FILE *pp;

if (ustr[user].gagcomm) {
   write_str(user,NO_COMM);
   return;
   }

if (!strlen(inpstr)) 
  {
   strcpy(inpstr,"names");
   preview(user, inpstr);
   return;
  }
        
inpstr[80]=0;

/* plug security hole */
if (check_fname(inpstr,user)) 
  {
   write_str(user,"Illegal name.");
   return;
  }

sscanf(inpstr,"%s ",buffer);
if (strlen(buffer) < 2) return;

remove_first(inpstr);  
sprintf(t_mess,"%s/%s",PICTURE_DIR,buffer);
strncpy(filename,t_mess,FILE_NAME_LEN);

if (!check_for_file(filename)) 
  {
   write_str(user,"Picture does not exist.");
   return;
  }

 if (!strlen(inpstr)) {
  strcpy(name,ustr[user].say_name);
  sprintf(z_mess,"(%s) shows:",name);
  writeall_str(z_mess, 1, user, 0, user, NORM, PICTURE, 0);
  catall(user,filename);
  }
 else {
  for (num=0;num<strlen(inpstr);num++) {
  if (!isdigit((int)inpstr[num])) {
     write_str(user,"Lines from bottom must be a number.");
     return;
     }
  }
  num=0;
  num=atoi(inpstr);
  strcpy(filename2,get_temp_file());

  sprintf(mess,"tail -%d %s",num,filename);

 if (!(pp=popen(mess,"r"))) {
	write_str(user,"Can't open pipe to get those picture lines!");
	return;
	}
 if (!(fp=fopen(filename2,"w"))) {
	write_str(user,"Can't open temp file for writing!");
	return;
	}
while (fgets(line,256,pp) != NULL) {
	fputs(line,fp);
      } /* end of while */
fclose(fp);
pclose(pp);

  strcpy(name,ustr[user].say_name);
  sprintf(z_mess,"(%s) shows:",name);
  writeall_str(z_mess, 1, user, 0, user, NORM, PICTURE, 0);
  catall(user,filename2);
  return;
 }

}



/*-------------------------------------------------------------*/
/* change a password                                           */
/*-------------------------------------------------------------*/
void password(int user, char *pword)
{

 if (pword[0]<32 || strlen(pword)< 3) 
    {
     write_str(user,SYS_PASSWD_INVA);  
     return;
    }
        
  if (strlen(pword)>NAME_LEN-1) 
    {
     write_str(user,SYS_PASSWD_LONG);  
     return;
    }

  if (strstr(pword,"^")) {
     write_str(user,"Password cant have color or hilite codes in it.");
     return;
     }

  if (strstr(pword," ")) {
     write_str(user,"Password cant have spaces in it.");
     write_str(user,"Syntax: .password <new_password>");
     return;
     }

  /*-------------------------------------------------------------*/
  /* convert name & passwd to lowercase and encrypt the password */
  /*-------------------------------------------------------------*/
  
  strtolower(pword);
  
  if (strcmp(ustr[user].login_name, pword) ==0)
    {
        write_str(user,"\nPassword cannot be the login name. \nPassword not changed."); 
        return;  
    }
   
  st_crypt(pword);                                   
  strcpy(ustr[user].password,pword);   
  copy_from_user(user);
  write_user(ustr[user].name);
  write_str(user,"Password is now changed.");
} 

/*-------------------------------------------------------------*/
/* toggle-monitoring                                           */
/*-------------------------------------------------------------*/
void tog_monitor(int user)
{

 if (ustr[user].monitor==3)
   {
    ustr[user].monitor=0;
    write_str(user," *** Monitoring is now totally off for you ***");
   }
 else if (ustr[user].monitor==2)
   {
    ustr[user].monitor=3;
    write_str(user," *** Monitoring is now on for users and incoming logins ***");
   }
 else if (ustr[user].monitor==1)
   {
    ustr[user].monitor=2;
    write_str(user," *** Monitoring is now on for incoming logins ***");
   }
 else if (ustr[user].monitor==0)
   {
    ustr[user].monitor=1;
    write_str(user," *** Monitoring is now on for users ***");
   }
 write_str(user," *** .monitor again for more options ***");

copy_from_user(user);
write_user(ustr[user].name);
} 

/*-----------------------------------------------------------*/
/* check file name for hack                                  */
/*-----------------------------------------------------------*/
int check_fname(char *inpstr, int user)
{
if (strpbrk(inpstr,".$/+*[]\\") )
  { 
   write_log(WARNLOG,YESTIME,"ILLEGAL: User %s tried to input file \"%s\"\n",ustr[user].say_name,inpstr);
   return(1);
  }
return 0;
}

/*--------------------------------------------------------------*/
/* display file to all in a room                                */
/*--------------------------------------------------------------*/
void catall(int user, char *filename)
{
FILE *fp;

if (!(fp=fopen(filename,"r"))) 
  {
   return;
  }
                                
/* jump to reading posn in file */
  fseek(fp,0,0);

/* loop until end of file or end of page reached */
strcpy(mess," ");
while(!feof(fp)) 
   {
     writeall_str(mess, 1, user, 1, user, NORM, PICTURE, 0);
     fgets(mess,sizeof(mess)-1,fp);
     mess[strlen(mess)-1]=0;
   }
   
FCLOSE(fp);
return;
}


/*------------------------------------------------*/
/* follow someone to a room                       */
/*------------------------------------------------*/
void follow(int user, char *inpstr)
{
int u;
char other_user[ARR_SIZE];

if (!strlen(inpstr)) {
   write_str(user,"Follow who?");
   return;
   }

sscanf(inpstr,"%s",other_user);
if ((u=get_user_num(other_user,user))== -1) {
   not_signed_on(user,other_user);
   return;
   }

if (!check_gag(user,u,0)) return;

strcpy(inpstr,astr[ustr[u].area].name);
go(user,inpstr,2);
}


/*--------------------------------------------------------------------*/
/* this command basically lists out a specified directory to the user */
/* the directory is specified in the inpstr                           */
/*--------------------------------------------------------------------*/
void print_ban_dir(int user, char *inpstr, char *s_search)
{
int num,timenum;
char buffer[132];
char small_buff[64];
char timebuf[23];
time_t tm_then;
struct dirent *dp;
FILE *fp2;
DIR  *dirp;
 
 strcpy(buffer,"    ");
 num=0;
 dirp=opendir((char *)inpstr);
  
 if (dirp == NULL)
   {
    write_str(user,"Directory information not found.");
    write_log(ERRLOG,YESTIME,"Directory information not found for directory \"%s\" in print_ban_dir %s\n",
    inpstr,get_error());
    return;
   }

   write_str(user,"Site/Cluster/Domain                 Ban Started"); 
   write_str(user,"-------------------------           -----------"); 
   
 while ((dp = readdir(dirp)) != NULL) 
   { 
    sprintf(small_buff,"%s",dp->d_name);
       if (small_buff[0]!='.')
        {
          write_str(user,mess);
         if (!(fp2=fopen(small_buff,"r"))) {
            write_str(user,"Cant open file for reading!");
            continue;
            }
         fgets(timebuf,13,fp2);
         FCLOSE(fp2);
	 timenum=atoi(timebuf);
         tm_then=((time_t) timenum);
         sprintf(mess,"%-35s %s ago",small_buff,converttime((long)((time(0)-tm_then)/60)));
         write_str(user,mess);
         timebuf[0]=0;
         num++;
        }
      
  }
 write_str(user,"");
 sprintf(mess,"Displayed %d banned site%s",num,num == 1 ? "" : "s");
 write_str(user,mess);

 (void) closedir(dirp);
}


/*--------------------------------------------------------------------*/
/* this command basically lists out a specified directory to the user */
/* the directory is specified in the inpstr                           */
/*--------------------------------------------------------------------*/
void print_dir(int user, char *inpstr, char *s_search)
{
int num,mode=0;
char buffer[132];
char small_buff[64];
struct dirent *dp;
DIR  *dirp;
 
 if (!strcmp(s_search,"-n")) { mode=1; }
 else { mode=0; }

 strcpy(buffer,"    ");
 num=0;
 dirp=opendir((char *)inpstr);
  
 if (dirp == NULL)
   {
    write_str(user,"Directory information not found.");
    write_log(ERRLOG,YESTIME,"Directory information not found for directory \"%s\" in print_dir %s\n",
    inpstr,get_error());
    return;
   }

 if (mode) write_str(user,"Counting..");
   
 while ((dp = readdir(dirp)) != NULL) 
   { 
    sprintf(small_buff,"%-18s ",dp->d_name);
    if (s_search && !mode)
      { if (strstr(small_buff,s_search))
        {
         if (small_buff[0]!='.')
          { write_str_nr(user,small_buff);
            num++;
           if (num%4==0) write_str(user,"");
          }
          
        }
      }
     else
      {
       if (small_buff[0]!='.')
        { if (!mode) write_str_nr(user,small_buff);
         num++;
         if ((num%4==0) && (!mode)) write_str(user,"");
        }
      }
      
  }
 if (!mode)
  write_str(user,"");

 sprintf(mess,"%s %d item%s",mode == 1 ? "Counted" : "Displayed",num,num == 1 ? "" : "s");
 write_str(user,mess);

 (void) closedir(dirp);
}


/*------------------------------------------------------------*/
/* this command shows detailed statistics and info on a user  */
/*------------------------------------------------------------*/
void usr_stat(int user, char *inpstr, int mode)
{
char temp2[7];
char temp3[7];
char other_user[ARR_SIZE];
char sw[4][4];
char yesno[2][4];
char noyes[2][4];
char filename[FILE_NAME_LEN];
int i,min=0,ret=0;
int signed_on = FALSE;
time_t tm_then;

strcpy(sw[0],"off");
strcpy(sw[1],"on ");
strcpy(sw[2],"on ");
strcpy(sw[3],"on ");
strcpy(yesno[0],"no ");
strcpy(yesno[1],"yes");
strcpy(noyes[0],"yes");
strcpy(noyes[1],"no ");

if (strlen(inpstr)) 
  {
   
   sscanf(inpstr,"%s ",other_user);
   strtolower(other_user);
   CHECK_NAME(other_user);


   if ((i = get_user_num(other_user,user)) != -1)
     if (strcmp(ustr[i].name, other_user) == 0) 
     {
      signed_on = TRUE;
     }
  }
 else
  {
    strcpy(other_user, ustr[user].name);
    signed_on = TRUE;
    i=user;
  }

ret=check_gag2(user,other_user);
if (!ret) return;
else if (ret==2) {
 write_str(user,NO_USER_STR);
 other_user[0]=0;
 inpstr[0]=0;
 return;
 }


if (signed_on) min=(int)((time(0)-ustr[i].time)/60);

if (!read_user(other_user))
  {
   write_str(user,NO_USER_STR);
   other_user[0]=0;
   inpstr[0]=0;
   return;
  }

if (strlen(inpstr) && (signed_on == TRUE))
write_str(user,"+---------------^*** NOTE: That user is currently logged on. ***^---------------+");
else
write_str(user,"+-----------------------------------------------------------------------------+");

if (signed_on == TRUE) {
sprintf(mess,"| Id:  %-*s %s", NAME_LEN+count_color(ustr[i].say_name,0), ustr[i].say_name, ustr[i].desc );
write_str(user,mess);

if (!mode) {
if (ustr[i].new_mail)
  { 
     sprintf(mess,"|                                      Has %3.3d new mail message%s.",ustr[i].mail_num,ustr[i].mail_num == 1 ? "" : "s");
     write_str(user,mess);
     write_str(user,"|");
  }
else
 write_str(user,"|");
} /* end of no mode */

if (mode) {
sprintf(mess,"| Homepage:  %s",ustr[i].homepage);
write_str(user,mess);

if (ustr[i].semail && ustr[user].tempsuper < EMAIL_LEVEL && (i!=user))
strcpy(mess,"| Email:      -hidden-");
else if (ustr[i].semail && ((ustr[user].tempsuper >= EMAIL_LEVEL) || (i==user)))
sprintf(mess,"| Email:     %s  ^-hidden-^",ustr[i].email_addr);
else
sprintf(mess,"| Email:     %s",ustr[i].email_addr);
write_str(user,mess);

sprintf(mess,"| Gender:    %-32s  ICQ: %s",ustr[i].sex,ustr[i].icq);
write_str(user,mess);
} /* end of mode */

if (!mode) {
sprintf(mess,"| Created:    %-25s  commands total: %-8.8li",
                           ustr[i].creation, ustr[i].numcoms);
write_str(user,mess);

if (astr[ustr[i].area].hidden)
sprintf(mess,"| Last room:  %-16.16s           logins to date: %-5.5d",
                           "",ustr[i].times_on);
else
sprintf(mess,"| Last room:  %-16.16s           logins to date: %-5.5d",
                           astr[ustr[i].area].name, 
                           ustr[i].times_on);
write_str(user,mess);

sprintf(mess,"|                                        ave time/login: %-5.5d mins",
             ustr[i].aver);
write_str(user,mess);
} /* end of no mode */

if (mode) {
sprintf(mess,"| Has been signed on for:  %s", converttime((long)min));
write_str(user,mess);
} /* end of mode */

if (!mode) {
sprintf(mess,"| Cumulative time:         %s", 
               converttime(ustr[i].totl+(long)min)); write_str(user,mess);
sprintf(mess,"| From site:  %-15.15s %s",
              ustr[i].last_site,ustr[i].last_name);
if (ustr[user].tempsuper >= WIZ_LEVEL)      write_str(user,mess);
sprintf(mess,"| Made from:  %-15.15s %s",
              ustr[i].init_site,ustr[i].init_netname);
if (ustr[user].tempsuper >= WIZ_LEVEL)      write_str(user,mess);

sprintf(mess,"| Rank:       %d  (^%s^)",
              ustr[i].super,ranks[ustr[i].super].lname);
if ((ustr[user].tempsuper >= WIZ_LEVEL) || (i==user)) write_str(user,mess); 

write_str(user,"|");

sprintf(mess,"| Set: Rows   %-3d  Cols    %-3d  Carriages %s  Abbrs     %s  Hilite  %s", 
             ustr[i].rows, ustr[i].cols,sw[ustr[i].car_return], sw[ustr[i].abbrs],sw[ustr[i].hilite]);
write_str(user,mess);

sprintf(mess,"|      Space  %s  Visible %s  PrivBeeps %s  Igtells   %s  Passhid %s",
             sw[ustr[i].white_space],yesno[ustr[i].vis],sw[ustr[i].beeps],
noyes[user_wants_message(i,TELLS)],sw[ustr[i].passhid]);
write_str(user,mess);
sprintf(mess,"|      Color  %s  AutoFwd %s  AutoRead  %s",
             sw[ustr[i].color],sw[ustr[i].autof],sw[ustr[i].autor]); 
write_str(user,mess);
sprintf(mess,"|      Gagged %s  Xcommed %s  Anchored  %s  Muzzled   %s  Frogged %s",
   yesno[ustr[i].gagcomm],yesno[ustr[i].suspended],yesno[ustr[i].anchor],
   noyes[ustr[i].shout],yesno[ustr[i].frog]);
write_str(user,mess);
write_str(user,"|");
sprintf(mess,"|      TTT Wins       %3d  TTT Losses  %3d  Hangman Wins  %3d",
   ustr[i].ttt_kills,ustr[i].ttt_killed,ustr[i].hang_wins);
write_str(user,mess);
sprintf(mess,"|      Hangman Losses %3d  Nerf shots   %2d  Nerf energy    %2d",
   ustr[i].hang_losses,ustr[i].nerf_shots,ustr[i].nerf_energy);
write_str(user,mess);
if (ustr[i].nerf_kills==1) strcpy(temp2, "person");
else strcpy(temp2, "people");
if (ustr[i].nerf_killed==1) strcpy(temp3, "time");
else strcpy(temp3, "times");
sprintf(mess,"|      Has nerfed %3d %s and has been nerfed %3d %-5s",  
  ustr[i].nerf_kills, temp2, ustr[i].nerf_killed, temp3);
write_str(user,mess);
 if (ustr[i].nerf_kills > (ustr[i].nerf_killed + 5))
write_str(user,"|      This nerfer will eat your nerf-gun for breakfast!");
 else if (ustr[i].nerf_kills > ustr[i].nerf_killed)
write_str(user,"|      This nerfer is pretty good.");
 else if (ustr[i].nerf_kills < (ustr[i].nerf_killed - 5))
write_str(user,"|      This nerfer really sucks wind!");
 else if (ustr[i].nerf_kills < ustr[i].nerf_killed)
write_str(user,"|      This nerfer is mediocre..so-so");
 else { }
} /* end of no mode */
}

else {
sprintf(mess,"| Id:  %-*s %s", NAME_LEN+count_color(t_ustr.say_name,0), t_ustr.say_name, t_ustr.desc );
write_str(user,mess);

if (!mode) {
if (t_ustr.new_mail)
  { 
     sprintf(mess,"|                                      Has %3.3d new mail message%s.",t_ustr.mail_num,t_ustr.mail_num == 1 ? "" : "s");
   write_str(user,mess);
   write_str(user,"|");
  }
else
 write_str(user,"|");
} /* end of no mode */

if (mode) {
sprintf(mess,"| Homepage:  %s",t_ustr.homepage);
write_str(user,mess);

if (t_ustr.semail && ustr[user].tempsuper < EMAIL_LEVEL)
strcpy(mess,"| Email:      -hidden-");
else if (t_ustr.semail && ustr[user].tempsuper >= EMAIL_LEVEL)
sprintf(mess,"| Email:     %s  ^-hidden-^",t_ustr.email_addr);
else
sprintf(mess,"| Email:     %s",t_ustr.email_addr);
write_str(user,mess);

sprintf(mess,"| Gender:    %-32s  ICQ: %s",t_ustr.sex,t_ustr.icq);
write_str(user,mess);
} /* end of mode */

if (!mode) {
sprintf(mess,"| Created:    %-25s  commands total: %-8.8li",
                           t_ustr.creation,t_ustr.numcoms);
write_str(user,mess);

if (astr[t_ustr.area].hidden)
sprintf(mess,"| Last room:  %-16.16s           logins to date: %-5.5d",
                           "",t_ustr.times_on);
else
sprintf(mess,"| Last room:  %-16.16s           logins to date: %-5.5d",
                           astr[t_ustr.area].name, 
                           t_ustr.times_on);
write_str(user,mess);

sprintf(mess,"|                                        ave time/login: %-5.5d mins",
             t_ustr.aver);
write_str(user,mess);
} /* end of no mode */

if (mode) {
tm_then=((time_t) t_ustr.rawtime);
sprintf(mess,"| Last login: %-16.16s that was %s ago",
                           t_ustr.last_date, 
                           converttime((long)((time(0)-tm_then)/60)));
write_str(user,mess);
} /* end of mode */

if (!mode) {
sprintf(mess,"| Cumulative time:         %s",
              converttime(t_ustr.totl));
write_str(user,mess);
sprintf(mess,"| From site:  %-15.15s %s",
              t_ustr.last_site,t_ustr.last_name);
if (ustr[user].tempsuper >= WIZ_LEVEL)      write_str(user,mess);
sprintf(mess,"| Made from:  %-15.15s %s",
              t_ustr.init_site,t_ustr.init_netname);
if (ustr[user].tempsuper >= WIZ_LEVEL)      write_str(user,mess);

sprintf(mess,"| Rank:       %d  (^%s^)",
              t_ustr.super,ranks[t_ustr.super].lname);
if (ustr[user].tempsuper >= WIZ_LEVEL)      write_str(user,mess); 

write_str(user,"|");

sprintf(mess,"| Set: Rows   %-3d  Cols    %-3d  Carriages %s  Abbrs     %s  Hilite  %s", 
             t_ustr.rows, t_ustr.cols,sw[t_ustr.car_return],sw[t_ustr.abbrs],sw[t_ustr.hilite]);
write_str(user,mess);

sprintf(mess,"|      Space  %s  Visible %s  PrivBeeps %s  Igtells   %s  Passhid %s",
             sw[t_ustr.white_space],yesno[t_ustr.vis],sw[t_ustr.beeps],
noyes[user_wants_message(-1,TELLS)],sw[t_ustr.passhid]);
write_str(user,mess);
sprintf(mess,"|      Color  %s  AutoFwd %s  AutoRead  %s",
             sw[t_ustr.color],sw[t_ustr.autof],sw[t_ustr.autor]); 
write_str(user,mess);
sprintf(mess,"|      Gagged %s  Xcommed %s  Anchored  %s  Muzzled   %s  Frogged %s",
   yesno[t_ustr.gagcomm],yesno[t_ustr.suspended],yesno[t_ustr.anchor],
   noyes[t_ustr.shout],yesno[t_ustr.frog]);
write_str(user,mess);
write_str(user,"|");
sprintf(mess,"|      TTT Wins       %3d  TTT Losses  %3d  Hangman Wins  %3d",
   t_ustr.ttt_kills,t_ustr.ttt_killed,t_ustr.hang_wins);
write_str(user,mess);
sprintf(mess,"|      Hangman Losses %3d  Nerf shots   %2d  Nerf energy    %2d",
   t_ustr.hang_losses,t_ustr.nerf_shots,t_ustr.nerf_energy);
write_str(user,mess);
if (t_ustr.nerf_kills==1) strcpy(temp2, "person");  
else strcpy(temp2, "people");
if (t_ustr.nerf_killed==1) strcpy(temp3, "time");
else strcpy(temp3, "times");
sprintf(mess,"|      Has nerfed %3d %s and has been nerfed %3d %-5s",
  t_ustr.nerf_kills, temp2, t_ustr.nerf_killed, temp3);
write_str(user,mess);
 if (t_ustr.nerf_kills > (t_ustr.nerf_killed + 5))
write_str(user,"|      This nerfer will eat your nerf-gun for breakfast!");
 else if (t_ustr.nerf_kills > t_ustr.nerf_killed)
write_str(user,"|      This nerfer is pretty good.");
 else if (t_ustr.nerf_kills < (t_ustr.nerf_killed - 5))
write_str(user,"|      This nerfer really sucks wind!");
 else if (t_ustr.nerf_kills < t_ustr.nerf_killed)
write_str(user,"|      This nerfer is mediocre..so-so");
 else { }
} /* end of no mode */
}

if (mode) {
write_str(user,"+-----------------------------------------------------------------------------+");
sprintf(filename,"%s/%s",PRO_DIR,other_user);
if (!cat(filename,user,0))
  write_str(user,"No profile. Sorry :)");
write_str(user,"+-----------------------------------------------------------------------------+");
} /* end of mode */
else {
write_str(user,"+-----------------------------------------------------------------------------+");
} /* end of no mode */

   other_user[0]=0;
   inpstr[0]=0;

}


/*------------------------------------------------------------------*/
/* the nuke command....remove user from user dir                    */
/*------------------------------------------------------------------*/
void nuke(int user, char *inpstr, int mode)
{
int u,a=0;
char other_user[ARR_SIZE];
char other_name[SAYNAME_LEN+1];
char z_mess[ARR_SIZE];
 
if (!strlen(inpstr)) 
  {
   write_str(user,"Nuke who?"); 
   return;
  }
 
sscanf(inpstr,"%s ",other_user);
strtolower(other_user);
CHECK_NAME(other_user);

if ((u=get_user_num_exact(other_user,user)) == -1) {
  if (!read_user(other_user))
  {
   write_str(user,NO_USER_STR);
   return;
  }
 }


if (u==-1) {
 if ((t_ustr.super >= ustr[user].tempsuper) && strcmp(ustr[user].name,ROOT_ID))
  {
    strcpy(z_mess,"You cannot nuke a user of same or higher rank.");
    write_str(user,z_mess);
    return;
  }
 }
else {
 if ((ustr[u].super >= ustr[user].tempsuper) && strcmp(ustr[user].name,ROOT_ID))
  {
    strcpy(z_mess,"You cannot nuke a user of same or higher rank.");
    write_str(user,z_mess);
    return;
  }
 }

/* kill user */    
if (u!=-1) {
  a = rand() % NUM_KILL_MESSAGES;

  strcpy(other_name,ustr[u].say_name);
  sprintf(mess, kill_text[a], other_name);
  writeall_str(mess, 1, u, 1, user, NORM, KILL, 0);

  write_str(u,DEF_KILL_MESS);
  user_quit(u,1);
 }

remove_exem_data(other_user);
remove_user(other_user);

if (u==-1) {
 /* normal manual nuke */
 if (!mode) {
  sprintf(z_mess,"NUKE: %s by %s",t_ustr.say_name,ustr[user].say_name);   
  btell(user,z_mess);
  write_log(SYSTEMLOG,YESTIME,"%s\n",z_mess);
  }
 else {
 /* manual user expire nuke */
  write_log(SYSTEMLOG,YESTIME,"MANUAL-EXPIRE-NUKE: %s by %s\n",t_ustr.say_name,ustr[user].say_name);   
 }
 }
else {
 sprintf(z_mess,"KILL-NUKE: %s by %s",other_name,ustr[user].say_name);   
 btell(user,z_mess);
 write_log(SYSTEMLOG,YESTIME,"%s\n",z_mess);
 }

}


/*--------------------------------*/
/* clear screen                   */
/*--------------------------------*/
void cls(int user)
{
int   i         = ustr[user].rows;
char  addem[3];

strcpy(addem,"\n\r");
mess[0] = 0;

if (!ustr[user].car_return) addem[1]=0;
if (i > 75) i = 75;

for(; i--;)
 { strcat(mess,addem); }
 
strcat(mess, "OK");
strcat(mess, addem);

write_str(user,mess);
}


/*----------------------------------------------------------*/
/* determine the result of a random event between two users */
/*----------------------------------------------------------*/
int determ_rand(int u1, int u2)
{
int v1, v2, v3, result;
float f_fact;

v1 = get_odds_value(u1);
v2 = get_odds_value(u2);

if (v1 == v2)  /* truely amazing, a real tie */
  { return(TIE); }
  
if (v1 > v2)
  { 
    result = 1;
    f_fact = (float)((float)v2/(float)v1);
  }
 else
  { 
    result = 2;
    f_fact = (float)((float)v1/(float)v2);
  }
  
v3 = (int) (f_fact * 100.0);

if (v3 > CLOSE_NUMBER)
  { if (  rand() % 2 )
      return(TIE);
     else
      return(BOTH_LOSE);
  }
  
return(result);
}

/*----------------------------------------------------------*/
/* issue a fight challenge to another user                  */
/*----------------------------------------------------------*/
void issue_chal(int user, int user2)
{
  fight.first_user  = user;
  fight.second_user = user2;
  fight.issued      = 1;
  fight.time        = time(0);
  
  sprintf(mess, chal_text[ rand() % num_chal_text ],
                ustr[user].say_name,
                ustr[user2].say_name);
              
  writeall_str(mess, 1, user, 0, user, NORM, FIGHT, 0);
  write_str(user,mess);
  
  write_str(user2,"");
  write_str(user2,"");
  write_str(user2,CHAL_LINE);
  write_str(user2,"");
  write_str(user2,CHAL_LINE2);
  write_str(user,CHAL_ISSUED);
}

/*----------------------------------------------------------*/
/* accept a fight challenge                                 */
/*----------------------------------------------------------*/
void accept_chal(int user)
{
int x;
int a,b;

a=fight.first_user;
b=fight.second_user;

x = determ_rand(a, b);

if (x == TIE)
  {
   sprintf(mess, tie1_text[ rand() % num_tie1_text ],
                 ustr[a].say_name,
                 ustr[b].say_name);
              
   writeall_str(mess, 1, user, 0, user, NORM, FIGHT, 0);
   write_str(user,mess);
   return;
  }
  
if (x == BOTH_LOSE)
  {
   sprintf(mess, tie2_text[ rand() % num_tie2_text ],
                 ustr[a].say_name,
                 ustr[b].say_name);
              
   writeall_str(mess,1,user,0,user,NORM,FIGHT,0);
   write_str(user,mess);
   user_quit(a,1);
   user_quit(b,1);
   return;
  }
  
if (x == 1)
  {
   sprintf(mess, wins1_text[ rand() % num_wins1_text ],
                 ustr[a].say_name,
                 ustr[b].say_name);
              
   writeall_str(mess,1,user,0,user,NORM,FIGHT,0);
   write_str(user,mess);
   user_quit(b,1);
   return;
  }
 
if (x == 2)
  {
   sprintf(mess, wins2_text[ rand() % num_wins2_text ],
                 ustr[b].say_name,
                 ustr[a].say_name);
              
   writeall_str(mess,1,user,0,user,NORM,FIGHT,0);
   write_str(user,mess);
   user_quit(a,1);
   return;
  }
}

/*----------------------------------------------------------*/
/* reset the fight                                          */
/*----------------------------------------------------------*/
void reset_chal(int user, char *inpstr)
{
  fight.first_user = -1;
  fight.second_user = -1;
  fight.issued = 0;
  fight.time = 0;
}

/*----------------------------------------------------------*/
/* the fight command                                        */
/*----------------------------------------------------------*/
void fight_another(int user, char *inpstr)
{
char other_user[ARR_SIZE];
int user2=-1;
int mode;

if (!strlen(inpstr)) 
  {
   write_str(user,"Fight status:");
   if (fight.issued)
     { 
      sprintf(mess,"Aggressor:   %s",ustr[fight.first_user].say_name);
      write_str(user,mess);
      sprintf(mess,"Defender:    %s",ustr[fight.second_user].say_name);
      write_str(user,mess);
     }
    else
     write_str(user,"   No current fight is challenged.");
   return;
  }
  
mode = 0;
if (!strcmp(inpstr,"reset"))   
  {
    reset_chal(user, inpstr);
    return;
  }
  
if (!strcmp(inpstr,"1"))   mode = 2;
if (!strcmp(inpstr,"yes")) mode = 2;
if (!strcmp(inpstr,"0"))   mode = 1;
if (!strcmp(inpstr,"no"))  mode = 1;

if (!mode)
  {
    sscanf(inpstr, "%s", other_user);
    strtolower(other_user);

    if ((user2 = get_user_num(other_user, user)) == -1 ) {
       if (!check_for_user(other_user))
         write_str(user,NO_USER_STR);
       else
         not_signed_on(user,other_user);
       return;
       }    
    if (ustr[user2].afk)
     {
    if (ustr[user2].afk == 1) {
      if (!strlen(ustr[user2].afkmsg))
       sprintf(t_mess,"- %s is Away From Keyboard -",ustr[user2].say_name);
      else
       sprintf(t_mess,"- %s %-45s -(A F K)",ustr[user2].say_name,ustr[user2].afkmsg);
      }
     else {
      if (!strlen(ustr[user2].afkmsg))
      sprintf(t_mess,"- %s is blanked AFK (is not seeing this) -",ustr[user2].say_name);
      else
      sprintf(t_mess,"- %s %-45s -(B A F K)",ustr[user2].say_name,ustr[user2].afkmsg);
      }

       write_str(user,t_mess);
       return;
     }

    mode = 3;
  }
  

if (fight.issued && mode == 3)
  {
    write_str(user, "Sorry, you must wait until the others are done.");
    return;
  }
  
if (!fight.issued && (mode == 1 || mode == 2) )
  {
    write_str(user, "You are not being challenged to a fight at this time.");
    return;
  }
  
if ((mode == 1 || mode == 2) && fight.second_user != user)
  {
    write_str(user, "You are not the challenged user...type .fight to see");
    return;
  }
  
if (mode == 3)
  {
   if (user == user2)
     {
       write_str(user,"You need help! (Fighting yourself...tsk tsk tsk)");
       return;
     }
     
/*----------------------------------------------------*/
/* check for standard fight room                      */
/*----------------------------------------------------*/

   if (FIGHT_ROOM != -1) 
     {
       if (ustr[user].area == FIGHT_ROOM)
         {
           sprintf(t_mess,"Noone can fight in %s.",astr[FIGHT_ROOM].name);
           write_str(user,t_mess);
           return;
         }
         
       if (ustr[user2].area != ustr[user].area)
         {
           sprintf(t_mess,"%s is not here to fight you.",ustr[user2].say_name);
           write_str(user,t_mess);
           
           sprintf(t_mess,"%s wanted to fight you, but you must be in the same room to do that.",
                           ustr[user].say_name);
           write_str(user2,t_mess);
           return;
         }
     }
     
   issue_chal(user, user2);
   return;
  }

if (mode == 1)
  {
   sprintf(mess, wimp_text[ rand() % num_wimp_text ], ustr[user].say_name);
   writeall_str(mess,1,user,0,user,NORM,FIGHT,0);
   write_str(user,mess);
   reset_chal(user, inpstr);
   return;
  }
  
if (mode == 2)
  {
   accept_chal(user);
   reset_chal(user, inpstr);
   return;
  }
  
}

/* DNS resolver */
void resolve_names_set(int user, char *inpstr)
{
int count=0;
char line[256];
char myjunk1[256];
char myjunk2[256];
char filename[256];
FILE *fp;
FILE *wfp;

  if (!strcmp(inpstr,"-c") || !strcmp(inpstr,"clearcache")) {
        remove(SITECACHE_FILE);
        write_str(user,"DNS cache cleared");
  }
  else if (!strcmp(inpstr,"-l") || !strcmp(inpstr,"listcache")) {
        if (!(fp=fopen(SITECACHE_FILE,"r"))) {
                write_str(user,BAD_FILEIO);
                write_log(ERRLOG,YESTIME,"Couldn't open file(r) \"%s\" in resolve_names_set! %s\n",SITECACHE_FILE,get_error());
                return;
        }
        strcpy(filename,get_temp_file());
        if (!(wfp=fopen(filename,"w"))) {  
                write_str(user,BAD_FILEIO);
                fclose(fp);
                return;
        }
   
        fprintf(wfp,"Current DNS cache (system TTL is %d)\n",MAX_DNS_TTL);
        fputs("==========================================\n",wfp);
        fprintf(wfp,"%-15s %-54s TTL\n","IP","HOSTNAME");
        while (fgets(line,256,fp)!=NULL) {
                line[strlen(line)-1]=0;
                sscanf(line,"%s",myjunk1);
                remove_first(line);
                sscanf(line,"%s",myjunk2); 
                remove_first(line);
                midcpy(line,line,0,53);
                fprintf(wfp,"%-15s %-54s %ld%s\n",myjunk1,line,(unsigned long)(time(0)-atol(myjunk2)),(time(0)-atol(myjunk2))>=MAX_DNS_TTL?"*":"");
                count++;
        } /* while */
        fclose(fp);
        fprintf(wfp,"%d entries found (limit before recycling: %d)\n",count,SITECACHE_SIZE);
        fclose(wfp);
        cat(filename,user,0);
  } /* else if */
  else {
  if (resolve_names==0) {
   resolve_names = 1;
   btell(user," Site name resolver now configured for cache and gethostbyaddr");
   }
  else if (resolve_names==1) {
   resolve_names = 2;
   btell(user," Site name resolver now configured for cache and resolver clip-on");
   init_resolver_clipon();
   }
  else if (resolve_names==2) {
   resolve_names = 3;
   btell(user," Site name resolver now configured for just resolver clip-on");
   }
  else if (resolve_names==3) {
   resolve_names = 4;
   btell(user," Site name resolver now configured for just gethostbyaddr");
   kill_resolver_clipon();
   }
  else if (resolve_names==4) {
   resolve_names = 0;
   btell(user," Site name resolver now turned off");
   }
  } /* main else */
}

/* Atmospheric control */
void toggle_atmos(int user, char *inpstr)
{
char line[SAYNAME_LEN+200];
int a=0;
int found=0;

if (!strlen(inpstr)) {
   write_str(user,"Specify a room name or \"all\" for all rooms.");
   return;
   }

if (!strcmp(inpstr,"all")) {
  if (atmos_on)
    {
      write_str(user,"Atmospherics OFF for all rooms");
      atmos_on=0;
      for (a=0;a<MAX_AREAS;++a) {
          astr[a].atmos=0;
          }  
      sprintf(line,"ATMOS disabled by %s for all rooms\n",ustr[user].say_name);
    }
   else
    {
      write_str(user,"Atmospherics ON for all rooms");
      atmos_on=1;  
      for (a=0;a<MAX_AREAS;++a) {
          astr[a].atmos=1;
          }  
      sprintf(line,"ATMOS enabled by %s for all rooms\n",ustr[user].say_name);
    }
  }

 else {

  if (strlen(inpstr) > NAME_LEN) 
     {
      write_str(user,"Room name length too long.");
      return;
     }

   /*--------------------*/
   /* see if area exists */
   /*--------------------*/

   found = FALSE;
   for (a=0; a < NUM_AREAS; ++a)
    { 
     if (!strcmp(astr[a].name, inpstr) )
       { 
         found = TRUE;
         break;
       }
    }
 
   if (!found)
     {
      write_str(user, NO_ROOM);
      return;
     }

  if (astr[a].atmos) {
   astr[a].atmos=0;
   sprintf(line,"ATMOS: disabled by %s for %s",ustr[user].say_name,astr[a].name);
   }
  else {
   astr[a].atmos=1;
   atmos_on=1;
   sprintf(line,"ATMOS: enabled by %s for %s",ustr[user].say_name,astr[a].name);
   }

 }  /* end of else */

 btell(user,line);
 write_log(SYSTEMLOG,YESTIME,"%s\n",line);
}	

/* New user creation control */		 
void toggle_allow(int user, char *inpstr)
{
char line[SAYNAME_LEN+200];

  if (allow_new > 1)
    {
      write_str(user,"New users DISALLOWED totally");
      allow_new=0;
      sprintf(line,"ALLOW: NEW users disallowed totally by %s",ustr[user].say_name);
    }
   else if (allow_new==1)
    {
      if (check_for_file(VERIFILE)) remove(VERIFILE);

      write_str(user,"New users ALLOWED freely");
      allow_new=2;
      sprintf(line,"ALLOW: NEW users allowed freely by %s",ustr[user].say_name);
    }
   else
    {
      write_str(user,"New users ALLOWED w/email verify");
      allow_new=1;
      sprintf(line,"ALLOW: NEW users allowed v/email verify by %s",ustr[user].say_name);
    }
    
 btell(user,line);
 write_log(SYSTEMLOG,YESTIME,"%s\n",line);
}


/*---------------------------------------------*/
/* get upper/lower bounds                      */
/*---------------------------------------------*/
void get_bounds_to_delete(char *str, int *lower, int *upper, int *mode)
{
char token1[20];
char token2[20];
char token3[20];

int  val_1 = -1;
int  val_2 = -1;

lower[0] = -1;
upper[0] = -1;
mode[0]  = 0;

if (strlen(str)) 
  {
    sscanf(str,"%s ",t_mess);
    remove_first(str);     
    /* token1 can be "from", "to", "except", "all", or a number */
    strncpy(token1, t_mess, 19);
    sscanf(token1, "%d", &val_2);
    if (strlen(str))
      {
	/* from x */
	/* to x */
	/* except x */
       sscanf(str,"%s ",t_mess);
       sscanf(t_mess, "%d", &val_1);
       remove_first(str);
       if (!strcmp(token1,"from") && strlen(str)) {
	t_mess[0]=0;
	sscanf(str,"%s ",t_mess);
	strncpy(token2, t_mess, 19);
	if (!strcmp(token2,"to")) {
	 /* from x to y */
	 remove_first(str);
	 if (strlen(str)) {
	  t_mess[0]=0;
	  sscanf(str,"%s ",t_mess);
	  strncpy(token3, t_mess, 19);
	  sscanf(token3, "%d", &val_2);
	 } /* strlen(str) */
	} /* is "to" */
       } /* is from and strlen(str) */
       else if (!strcmp(token1,"except") && strlen(str)) {
	t_mess[0]=0;
	sscanf(str,"%s ",t_mess);
	strncpy(token2, t_mess, 19);
	if (!strcmp(token2,"to")) {
	 /* except x to y */
	 remove_first(str);
	 if (strlen(str)) {
	  t_mess[0]=0;
	  sscanf(str,"%s ",t_mess);
	  strncpy(token3, t_mess, 19);
	  sscanf(token3, "%d", &val_2);
	 } /* strlen(str) */
	} /* is "to" */
       } /* is except and strlen(str) */
     }
    else
     {
      t_mess[0] = 0;
      val_1 = -1;
     }
    
    /*------------------*/
    /* delete all lines */
    /*------------------*/
    if (strcmp(token1,"all") == 0)
      {
       lower[0] = 0;
       upper[0] = 0;
       mode[0] = 1;
       return;
      }
      
    /*------------------*/
    /* delete to end    */
    /*------------------*/
    if (strcmp(token1,"from") == 0)
      {
       if (val_1 > 0)
        {
	 /* from x */
         if (val_2 > 0)
         {
	  /* from x to y */
          lower[0] = val_1;
          upper[0] = val_2;
	  if (val_1 == val_2) {
           mode[0] = 2;
	  }
	  else if (val_1 > val_2) {
	   lower[0] = -1;
	   upper[0] = -1;
	  }
	  else {
           mode[0] = 6;
	  }
	  return;
         }
          lower[0] = val_1;
          upper[0] = 0;
          mode[0] = 3;
        } /* val_1 > 0 */
       return;
      }

    /*----------------------------*/
    /* delete all EXCEPT these    */
    /*----------------------------*/
    if (strcmp(token1,"except") == 0)
      {
       if (val_1 > 0)
        {
	 /* except x */
         if (val_2 > 0)
         {
	  /* except x to y */
          lower[0] = val_2;
          upper[0] = val_1;
	  if (val_1 == val_2) {
	   lower[0] = -1;
	  }
	  else if (val_1 > val_2) {
	   lower[0] = -1;
	   upper[0] = -1;
	  }
          mode[0] = 5;
	  return;
         }
          lower[0] = -1;
          upper[0] = val_1;
          mode[0] = 5;
        } /* val_1 > 0 */
       return;
      }
      
    /*-------------------------*/
    /* delete from begining    */
    /*-------------------------*/
    if (strcmp(token1,"to") == 0)
      {
       if (val_1 > 0)
        {
         lower[0] = 0;
         upper[0] = val_1;
         mode[0]  = 4;
        }
       return;
      }

    /*-------------------------*/
    /* delete single line      */
    /*-------------------------*/
    if (val_2 > 0)
      {
       lower[0] = val_2;
       upper[0] = val_2;
       mode[0]  = 2;
       return;
      }
  }
 else
  {
   /* delete the first line (default) */
   lower[0] = 1;
   upper[0] = 1;
   mode[0]  = 2; 
  }
  
}

/*--------------------------------------------------------------*/
/* create the away from keyboard command                        */
/*--------------------------------------------------------------*/
void set_afk(int user, char *inpstr)
{
int i;
char nbuffer[ARR_SIZE];

if (!AFK_NERF) {
 if (!strcmp(astr[ustr[user].area].name,NERF_ROOM)) {
     write_str(user,"That command is not allowed in this room.");
     return;
     }
 }

i = rand() % NUM_IDLE_LINES;

if ((inpstr[0]=='-') && (inpstr[1]=='l') ) {
  sscanf(inpstr,"%s ",nbuffer);
  }
else {
  strcpy(nbuffer,inpstr);
  }

if (strlen(nbuffer) > DESC_LEN) {
   write_str(user,"Message too long...not set afk");
   return;
   }
if ((strlen(nbuffer) > 1) && (strcmp(nbuffer,"-l"))) 
 {
  strcpy(ustr[user].afkmsg,nbuffer);
  if (!ustr[user].vis)
   sprintf(mess,"- %s %-41s -(A F K)",INVIS_ACTION_LABEL,ustr[user].afkmsg);
  else
   sprintf(mess,"- %s %-41s -(A F K)",ustr[user].say_name,ustr[user].afkmsg);
 }
else if ((strlen(nbuffer) > 1) && (!strcmp(nbuffer,"-l")) ) {
 remove_first(inpstr);
 if (strlen(inpstr) > DESC_LEN) {
    write_str(user,"Message too long..not set afk");
    return;
    }
 else if (!strlen(inpstr)) {
    if (!ustr[user].vis)
     sprintf(mess, idle_text[i], INVIS_ACTION_LABEL, "(A F K)");
    else
     sprintf(mess, idle_text[i], ustr[user].say_name, "(A F K)");
    strcpy(ustr[user].afkmsg,"");
    }
 else {
    strcpy(ustr[user].afkmsg,inpstr);
    if (!ustr[user].vis)
     sprintf(mess,"- %s %-41s -(A F K)",INVIS_ACTION_LABEL,ustr[user].afkmsg);
    else
     sprintf(mess,"- %s %-41s -(A F K)",ustr[user].say_name,ustr[user].afkmsg);
    }
 writeall_str(mess,1,user,0,user,NORM,AFK_TYPE,0);
 write_str(user,mess);
 write_str(user,"");
 write_str(user,"THIS TERMINAL IS NOW LOCKED.");
 write_str_nr(user,"Enter your account password to return: ");
 telnet_echo_off(user);
 telnet_write_eor(user);
 ustr[user].afk = 1;
 ustr[user].lockafk=1;
 noprompt=1;
 return;
 }
else {
 if (!ustr[user].vis)
  sprintf(mess, idle_text[i], INVIS_ACTION_LABEL, "(A F K)");
 else
  sprintf(mess, idle_text[i], ustr[user].say_name, "(A F K)");
 strcpy(ustr[user].afkmsg,"");
 }
writeall_str(mess,1,user,0,user,NORM,AFK_TYPE,0);
write_str(user,mess);

ustr[user].afk = 1;
}

/*--------------------------------------------------------------*/
/* create the boss - away from keyboard command                 */
/*--------------------------------------------------------------*/
void set_bafk(int user, char *inpstr)
{
int i;
char obuffer[ARR_SIZE];

if (!AFK_NERF) {
 if (!strcmp(astr[ustr[user].area].name,NERF_ROOM)) {
     write_str(user,"That command is not allowed in this room.");
     return;
     }
 }

i = rand() % NUM_IDLE_LINES;

if ((inpstr[0]=='-') && (inpstr[1]=='l') ) {
  sscanf(inpstr,"%s ",obuffer);
  }
else {
  strcpy(obuffer,inpstr);
  }

if (strlen(obuffer) > DESC_LEN) {
   write_str(user,"Message too long...not set afk");
   return;
   }
if ((strlen(obuffer) > 1) && (strcmp(obuffer,"-l")) ) 
 {
  strcpy(ustr[user].afkmsg,obuffer);
  if (!ustr[user].vis)
   sprintf(mess,"- %s %-41s -(B A F K)",INVIS_ACTION_LABEL,ustr[user].afkmsg);
  else
   sprintf(mess,"- %s %-41s -(B A F K)",ustr[user].say_name,ustr[user].afkmsg);
 }
else if ((strlen(obuffer) > 1) && (!strcmp(obuffer,"-l")) ) {
 remove_first(inpstr);
 if (strlen(inpstr) > DESC_LEN) {
    write_str(user,"Message too long..not set afk");
    return;
    }
 else if (!strlen(inpstr)) {
    if (!ustr[user].vis)
     sprintf(mess, idle_text[i], INVIS_ACTION_LABEL, "(B A F K)");
    else
     sprintf(mess, idle_text[i], ustr[user].say_name, "(B A F K)");
    strcpy(ustr[user].afkmsg,"");
    }
 else {
    strcpy(ustr[user].afkmsg,inpstr);
    if (!ustr[user].vis)
     sprintf(mess,"- %s %-41s -(B A F K)",INVIS_ACTION_LABEL,ustr[user].afkmsg);
    else
     sprintf(mess,"- %s %-41s -(B A F K)",ustr[user].say_name,ustr[user].afkmsg);
    }
 writeall_str(mess,1,user,0,user,NORM,AFK_TYPE,0);
 write_str(user,mess);
 cls(user);
 write_str(user,"");
 write_str(user,"THIS TERMINAL IS NOW LOCKED.");
 write_str_nr(user,"Enter your account password to return: ");
 telnet_echo_off(user);
 telnet_write_eor(user);
 ustr[user].afk = 2; 
 ustr[user].lockafk=1;
 noprompt=1;
 return;
 }
else {
 if (!ustr[user].vis)
  sprintf(mess, idle_text[i], INVIS_ACTION_LABEL, "(B A F K)");
 else
  sprintf(mess, idle_text[i], ustr[user].say_name, "(B A F K)");
 strcpy(ustr[user].afkmsg,"");
 }
writeall_str(mess,1,user,0,user,NORM,AFK_TYPE,0);
write_str(user,mess);
cls(user);
ustr[user].afk = 2;

}

/* Give time info for major timezones or whatever zone user */
/* asks for that we can show                                */
void systime(int user, char *inpstr)
{
char buf1[100];
struct tm *clocker;
time_t tm_now;

if (!strlen(inpstr)) {
write_str(user,"+-----------------------------------------------------------------------------+");

do_timeset(TZONE);

time(&tm_now);
clocker=localtime(&tm_now);
strftime(buf1,sizeof(buf1),"%a, %b %d %I:%M:%S %p %Y ",clocker);
sprintf(mess,"System time is (%s)%-*s : %s",TZONE,(int)(23-strlen(TZONE)),"",buf1);
write_str(user,mess);
write_str(user," ");

do_timeset("US/Eastern");

time(&tm_now);
clocker=localtime(&tm_now);
strftime(buf1,sizeof(buf1),"%a, %b %d %I:%M:%S %p %Y ",clocker);
sprintf(mess,"US Eastern Time                          : %s",buf1);
write_str(user,mess);

do_timeset("US/Central");

time(&tm_now);
clocker=localtime(&tm_now);
strftime(buf1,sizeof(buf1),"%a, %b %d %I:%M:%S %p %Y ",clocker);
sprintf(mess,"US Central Time                          : %s",buf1);
write_str(user,mess);

do_timeset("US/Mountain");

time(&tm_now);
clocker=localtime(&tm_now);
strftime(buf1,sizeof(buf1),"%a, %b %d %I:%M:%S %p %Y ",clocker);
sprintf(mess,"US Mountain Time                         : %s",buf1);
write_str(user,mess);

do_timeset("US/Pacific");

time(&tm_now);
clocker=localtime(&tm_now);
strftime(buf1,sizeof(buf1),"%a, %b %d %I:%M:%S %p %Y ",clocker);
sprintf(mess,"US Pacific Time                          : %s",buf1);
write_str(user,mess);

do_timeset("Brazil/East");

time(&tm_now);
clocker=localtime(&tm_now);
strftime(buf1,sizeof(buf1),"%a, %b %d %I:%M:%S %p %Y ",clocker);
sprintf(mess,"East Brasilia Time                       : %s",buf1);
write_str(user,mess);

do_timeset("Greenwich");

time(&tm_now);
clocker=localtime(&tm_now);
strftime(buf1,sizeof(buf1),"%a, %b %d %I:%M:%S %p %Y ",clocker);
sprintf(mess,"Greenwich Mean Time                      : %s",buf1);
write_str(user,mess);

do_timeset("CET");

time(&tm_now);
clocker=localtime(&tm_now);
strftime(buf1,sizeof(buf1),"%a, %b %d %I:%M:%S %p %Y ",clocker);
sprintf(mess,"Central European Time                    : %s",buf1);
write_str(user,mess);

do_timeset("Australia/NSW");

time(&tm_now);
clocker=localtime(&tm_now);
strftime(buf1,sizeof(buf1),"%a, %b %d %I:%M:%S %p %Y ",clocker);
sprintf(mess,"Australian NSW Time                      : %s",buf1);
write_str(user,mess);

do_timeset("NZ");

time(&tm_now);
clocker=localtime(&tm_now);
strftime(buf1,sizeof(buf1),"%a, %b %d %I:%M:%S %p %Y ",clocker);
sprintf(mess,"New Zealand Time                         : %s",buf1);
write_str(user,mess);
write_str(user,"+-----------------------------------------------------------------------------+");
}
else if (!strcmp(inpstr,"-l")) {
 write_str(user,"japan    ausnorth    aussouth    auswest    hawaii");
 write_str(user,"hongkong singapore   israel      cuba       mexico");
 write_str(user,"egypt    chile       eeurope     weurope    meurope");
 write_str(user,"zulu");
}
else {
 strtolower(inpstr);
 if (!strcmp(inpstr,"japan"))
  do_timeset("Japan");
 else if (!strcmp(inpstr,"ausnorth"))
  do_timeset("Australia/North");
 else if (!strcmp(inpstr,"aussouth"))
  do_timeset("Australia/South");
 else if (!strcmp(inpstr,"auswest"))
  do_timeset("Australia/West");
 else if (!strcmp(inpstr,"hawaii"))
  do_timeset("US/Hawaii");
 else if (!strcmp(inpstr,"hongkong"))
  do_timeset("Hongkong");
 else if (!strcmp(inpstr,"singapore"))
  do_timeset("Singapore");
 else if (!strcmp(inpstr,"israel"))
  do_timeset("Israel");
 else if (!strcmp(inpstr,"cuba"))
  do_timeset("Cuba");
 else if (!strcmp(inpstr,"mexico"))
  do_timeset("Mexico/General");
 else if (!strcmp(inpstr,"egypt"))
  do_timeset("Egypt");
 else if (!strcmp(inpstr,"chile"))
  do_timeset("Chile/Continental");
 else if (!strcmp(inpstr,"eeurope"))
  do_timeset("EET");
 else if (!strcmp(inpstr,"weurope"))
  do_timeset("WET");
 else if (!strcmp(inpstr,"meurope"))
  do_timeset("MET");
 else if (!strcmp(inpstr,"zulu"))
  do_timeset("Zulu");
 else {
   write_str(user,"Zone doesn't exist. \".time -l\" for list");
   return;
  }

time(&tm_now);
clocker=localtime(&tm_now);
strftime(buf1,sizeof(buf1),"%a, %b %d %I:%M:%S %p %Y ",clocker);
sprintf(mess,"%s Time%-*s: %s",inpstr,(int)(36-strlen(inpstr)),"",buf1);
mess[0]=toupper((int)mess[0]);
write_str(user,"+-----------------------------------------------------------------------------+");
write_str(user,mess);
write_str(user,"+-----------------------------------------------------------------------------+");
} /* end of strlen else */

/* Reset time back to local time */
do_timeset(TZONE);

}


/*--------------------------------------------------------------*/
/* meter command                                                */
/*--------------------------------------------------------------*/
void meter(int user, char *inpstr)
{
int i=0;
int j=0;
char datestr[80];
char daystr[10];
char timebuf[80];
char graph[26];
time_t tm;

if (!strlen(inpstr)) {
sprintf(mess,"+-------------------------------------------+");
write_str(user, mess);

sprintf(mess,"|         Activity meter                    |");
write_str(user, mess);

sprintf(mess,"+-------------------------------------------+");
write_str(user, mess);

sprintf(mess,"| Commands this period: %4d                |",commands);
write_str(user, mess);

sprintf(mess,"|                                           |");
write_str(user, mess);

fill_bar(says, commands, graph);
sprintf(mess,"| says  %s              |", graph);
write_str(user, mess);

fill_bar(tells, commands, graph);
sprintf(mess,"| tells %s              |", graph);
write_str(user, mess);

sprintf(mess,"|                                           |");
write_str(user, mess);

sprintf(mess,"| Commands per minute ave: %4d             |",commands_running);
write_str(user, mess);

sprintf(mess,"| Tells per minute ave:    %4d             |",tells_running);
write_str(user, mess);

sprintf(mess,"| Says per minute ave:     %4d             |",says_running);
write_str(user, mess);

sprintf(mess,"+-------------------------------------------+");
write_str(user, mess);
}

/* Do user activity graph */
else if (!strcmp(inpstr,"-l")) {

time(&tm);

strcpy(datestr,ctime(&tm));
midcpy(datestr,daystr,0,2);

time(&tm);
strcpy(datestr,ctime(&start_time));

if (!strcmp(daystr,"Sun"))
 strcpy(daystr,"Sunday");
else if (!strcmp(daystr,"Mon"))
 strcpy(daystr,"Monday");
else if (!strcmp(daystr,"Tue"))
 strcpy(daystr,"Tuesday");
else if (!strcmp(daystr,"Wed"))
 strcpy(daystr,"Wednesday");
else if (!strcmp(daystr,"Thu"))
 strcpy(daystr,"Thursday");
else if (!strcmp(daystr,"Fri"))
 strcpy(daystr,"Friday");
else if (!strcmp(daystr,"Sat"))
 strcpy(daystr,"Saturday");

write_str(user,"");
sprintf(mess,"User login activity for ^%s^",daystr);
write_str(user,mess);
datestr[strlen(datestr)-1]=0;
sprintf(mess,"System Booted: %s",datestr);
write_str(user,mess);
write_str(user,"");
write_str(user,"LOGS");

timebuf[0]=0;

/* If the hour's login numbers fall into the range, put aa asterix */
/* in the buffer, else put a space                                 */

strcpy(timebuf,"100+  | ");

for (i=0;i<24;++i) {
   if (logstat[i].logins > 100)
    strcat(timebuf,"*  ");
   else
    strcat(timebuf,"   ");
 if (i==23) {
   j=strlen(timebuf);
   timebuf[j-2]=0;
  }
}

write_str(user,timebuf);
i=0;
j=0;

strcpy(timebuf,"91-100| ");

for (i=0;i<24;++i) {
   if ((logstat[i].logins > 90) && (logstat[i].logins <= 100))
    strcat(timebuf,"*  ");
   else
    strcat(timebuf,"   ");
 if (i==23) {
   j=strlen(timebuf);
   timebuf[j-2]=0;
  }
}

write_str(user,timebuf);
i=0;
j=0;

strcpy(timebuf,"81-90 | ");

for (i=0;i<24;++i) {
   if ((logstat[i].logins > 80) && (logstat[i].logins <= 90))
    strcat(timebuf,"*  ");
   else
    strcat(timebuf,"   ");
 if (i==23) {
   j=strlen(timebuf);
   timebuf[j-2]=0;
  }
}

write_str(user,timebuf);
i=0;
j=0;

strcpy(timebuf,"71-80 | ");

for (i=0;i<24;++i) {
   if ((logstat[i].logins > 70) && (logstat[i].logins <= 80))
    strcat(timebuf,"*  ");
   else
    strcat(timebuf,"   ");
 if (i==23) {
   j=strlen(timebuf);
   timebuf[j-2]=0;
  }
}

write_str(user,timebuf);
i=0;
j=0;

strcpy(timebuf,"61-70 | ");

for (i=0;i<24;++i) {
   if ((logstat[i].logins > 60) && (logstat[i].logins <= 70))
    strcat(timebuf,"*  ");
   else
    strcat(timebuf,"   ");
 if (i==23) {
   j=strlen(timebuf);
   timebuf[j-2]=0;
  }
}

write_str(user,timebuf);
i=0;
j=0;

strcpy(timebuf,"51-60 | ");

for (i=0;i<24;++i) {
   if ((logstat[i].logins > 50) && (logstat[i].logins <= 60))
    strcat(timebuf,"*  ");
   else
    strcat(timebuf,"   ");
 if (i==23) {
   j=strlen(timebuf);
   timebuf[j-2]=0;
  }
}

write_str(user,timebuf);
i=0;
j=0;

strcpy(timebuf,"41-50 | ");

for (i=0;i<24;++i) {
   if ((logstat[i].logins > 40) && (logstat[i].logins <= 50))
    strcat(timebuf,"*  ");
   else
    strcat(timebuf,"   ");
 if (i==23) {
   j=strlen(timebuf);
   timebuf[j-2]=0;
  }
}

write_str(user,timebuf);
i=0;
j=0;

strcpy(timebuf,"31-40 | ");

for (i=0;i<24;++i) {
   if ((logstat[i].logins > 30) && (logstat[i].logins <= 40))
    strcat(timebuf,"*  ");
   else
    strcat(timebuf,"   ");
 if (i==23) {
   j=strlen(timebuf);
   timebuf[j-2]=0;
  }
}

write_str(user,timebuf);
i=0;
j=0;

strcpy(timebuf,"21-30 | ");

for (i=0;i<24;++i) {
   if ((logstat[i].logins > 20) && (logstat[i].logins <= 30))
    strcat(timebuf,"*  ");
   else
    strcat(timebuf,"   ");
 if (i==23) {
   j=strlen(timebuf);
   timebuf[j-2]=0;
  }
}

write_str(user,timebuf);
i=0;
j=0;

strcpy(timebuf,"11-20 | ");

for (i=0;i<24;++i) {
   if ((logstat[i].logins > 10) && (logstat[i].logins <= 20))
    strcat(timebuf,"*  ");
   else
    strcat(timebuf,"   ");
 if (i==23) {
   j=strlen(timebuf);
   timebuf[j-2]=0;
  }
}

write_str(user,timebuf);
i=0;
j=0;

strcpy(timebuf,"0-10  | ");

for (i=0;i<24;++i) {
   if ((logstat[i].logins > 0) && (logstat[i].logins <= 10))
    strcat(timebuf,"*  ");
   else
    strcat(timebuf,"   ");
 if (i==23) {
   j=strlen(timebuf);
   timebuf[j-2]=0;
  }
}

write_str(user,timebuf);
i=0;
j=0;

write_str(user,"      +-|--|--|--|--|--|--|--|--|--|--|--|--|--|--|--|--|--|--|--|--|--|--|--|");
write_str(user,"      12a  1  2  3  4  5  6  7  8  9 10 11 12p 1  2  3  4  5  6  7  8  9 10 11");

} /* end of else if */

else if (!strcmp(inpstr,"-b")) {
backup_logs(user);
write_str(user,"Backup done!");
}
else {
 write_str(user,"Invalid option.");
 write_str(user,"Usage: .meter [-l]");
 }

}

void fill_bar(int val1, int val2, char *str)
{
 int i, j;

  strcpy(str,"|--------------------|");

  val1 = val1 * 100;
  val2 = val2;

  if (val2 == 0) return;
  i = (val1 / val2) / 5;
  
  if (i > 20) i = 20;
  
  if (i == 0) return;

  for(j = 0; j<i; j++)
   { 
     str[1+j] = '#';
   }
}

/*------------------------------------------------*/
/* set new user quota                             */
/*------------------------------------------------*/
void set_quota(int user, char *inpstr)
{

  int value = 0;
  
  sscanf(inpstr,"%d", &value);
  
  if (value < 0)
    {
      value = 0;
    }
  else if (value > 999)
    {
      write_str(user,"WARNING: Valid quota range is 0 to 999");
      value = 999;
    }
    
  sprintf(mess,"The new quota is: %d",value);
  write_str(user,mess);
  write_log(SYSTEMLOG,YESTIME,"QUOTA: %s changed the new user quota from %ld to %d\n",ustr[user].say_name,system_stats.quota,value);

  system_stats.logins_today++;    
  system_stats.logins_since_start++;
  system_stats.new_since_start++;
  system_stats.new_users_today    = 0;
  system_stats.quota              = value;
}

/*------------------------------------------------*/
/* IMPORTANT: make sure there are at least 10     */
/* extra bytes in the string to append on         */
/*------------------------------------------------*/
void add_hilight(char *str)
{
char buff[ARR_SIZE];

strcat(str,"^");
strcpy(buff,str);
strcpy(str,"^ ");
strcat(str,buff);

}


/*-----------------------------------------------*/
/* x communicate a user                          */
/*-----------------------------------------------*/
void xcomm(int user, char *inpstr)
{
char other_user[ARR_SIZE];
int u,inlen;
unsigned int i;

if (!strlen(inpstr)) 
  {
   write_str(user,"Users XCOMMed & logged on     Time left"); 
   write_str(user,"-------------------------     ---------"); 
   for (u=0;u<MAX_USERS;++u) 
    {
     if (ustr[u].suspended  && ustr[u].area > -1) 
       {
        if (ustr[u].xco_time == 0)
           sprintf(mess,"%-*s %s",NAME_LEN+count_color(ustr[u].say_name,0),ustr[u].say_name,"Perm");
        else
           sprintf(mess,"%-*s %s",NAME_LEN+count_color(ustr[u].say_name,0),ustr[u].say_name,converttime((long)ustr[u].xco_time));
        write_str(user, mess);
       };
    }
   write_str(user,"(end of list)");
   return;
  }

sscanf(inpstr,"%s ",other_user);
strtolower(other_user);

if ((u=get_user_num(other_user,user))== -1) 
  {
   not_signed_on(user,other_user);
   return;
  }
  
if (u == user)
  {   
   write_str(user,"You are definitly wierd! Trying to xcom yourself, geesh."); 
   return;
  }

 if ((!strcmp(ustr[u].name,ROOT_ID)) || (!strcmp(ustr[u].name,BOT_ID)
      && strcmp(ustr[user].name,ROOT_ID))) {
    write_str(user,"Yeah, right!");
    return;
    }

if (ustr[user].tempsuper <= ustr[u].super) 
  {
   write_str(user,"That would not be wise..");   
   sprintf(mess,XCOMM_CANT,ustr[user].say_name);
   write_str(u,mess);
   return;
  }

if (ustr[u].suspended == 0) {
remove_first(inpstr);
if (strlen(inpstr) && strcmp(inpstr,"0")) {
   if (strlen(inpstr) > 5) {
      write_str(user,"Minutes can't exceed 5 digits.");
      return;
      }
   inlen=strlen(inpstr);
   for (i=0;i<inlen;++i) {
     if (!isdigit((int)inpstr[i])) {
        write_str(user,"Numbers only!");
        return;
        }
     }
    i=0;
    i=atoi(inpstr);
    if ( i > 32767) {
       write_str(user,"Minutes can't exceed 32767.");
       i=0;
       return;
      }
  i=0;
  ustr[u].xco_time=atoi(inpstr);
  ustr[u].suspended = 1;
    write_str(u,XCOMMON_MESS);
    sprintf(mess,"XCOM: ON for %s by %s for %s",ustr[u].say_name,
ustr[user].say_name, converttime((long)ustr[u].xco_time));
}
else {
    ustr[u].suspended = 1;
    ustr[u].xco_time=0;
    write_str(u,XCOMMON_MESS);
    sprintf(mess,"XCOM: ON for %s by %s",ustr[u].say_name, ustr[user].say_name);
  }
}     /* end of if suspended */

else {
    ustr[u].suspended = 0;
    ustr[u].xco_time=0;
    write_str(u,XCOMMOFF_MESS);
    sprintf(mess,"XCOM: OFF for %s by %s",ustr[u].say_name, ustr[user].say_name);
  } 
btell(user, mess);
write_log(SYSTEMLOG,YESTIME,"%s\n",mess);
write_str(user,"Ok");
}


/*--------------------------------------------------------*/
/* logic to detemine if message is to be ignored          */
/*--------------------------------------------------------*/
int user_wants_message(int user, int type)
{
 if (user==-1) {
 if (t_ustr.flags[type] == '1')
  return 0;
 else
  return 1;
 }
 else {
 if (ustr[user].flags[type] == '1')
  return 0;
 else
  return 1;
 }
}


/*----------------------*/
/* listen to all flags  */
/*----------------------*/
void listen_all(int user)
{
int i=0;

if (user==-1)
 strcpy(t_ustr.flags,"");
else
 strcpy(ustr[user].flags,"");

/* all 0s */
 for (i=0;i<NUM_IGN_FLAGS;++i) {
  if (user==-1)
   strcat(t_ustr.flags,"0");
  else
   strcat(ustr[user].flags,"0");
 }

}


/*----------------------*/
/* ignore all flags     */
/*----------------------*/
void ignore_all(int user)
{
int i=0;

if (user==-1)
 strcpy(t_ustr.flags,"");
else
 strcpy(ustr[user].flags,"");

/* all 1s */
 for (i=0;i<NUM_IGN_FLAGS;++i) {
  if (user==-1)
   strcat(t_ustr.flags,"1");
  else
   strcat(ustr[user].flags,"1");
 }

}


/*----------------------------------------------------------*/
/* set listening flags                                      */
/*----------------------------------------------------------*/
void user_listen(int user, char *inpstr)
{
int u;

if (!strlen(inpstr)) 
  {
   write_str(user,"^HG+------------------------------------+^"); 
   write_str(user,"^HG|^ ^HYYou can hear:^                      ^HG|^"); 
   write_str(user,"^HG+------------------------------------+^");
   for (u=1;u<NUM_IGN_FLAGS;++u) 
    {
     if (ustr[user].flags[u] != '1') 
       {
        write_str(user,flag_names[u].text);
       };
    }
   write_str(user,"(end of list)");
   return;
  }
  
if (!instr2(0,"all",inpstr,0) )
 {
  listen_all(user);
  write_str(user,"You will now hear all messages.");
  return;
 } 

u = get_flag_num(inpstr);
if (u > -1)
  {
   ustr[user].flags[u] = '0';
  
   sprintf(mess,"You are now listening to %s",flag_names[u].text);
   write_str(user,mess);
  }
 else
  {
   write_str(user,"That message type not known");
  }
		 
}

/*----------------------------------------------------------*/
/* set ignoring flags                                       */
/*----------------------------------------------------------*/
void user_ignore(int user, char *inpstr)
{
int u;

if (!strlen(inpstr)) 
  {
   write_str(user,"^HG+------------------------------------+^"); 
   write_str(user,"^HG|^ ^HYYou are ignoring:^                  ^HG|^"); 
   write_str(user,"^HG+------------------------------------+^");
   for (u=1;u<NUM_IGN_FLAGS;++u) 
    {
     if (ustr[user].flags[u] == '1') 
       {
        write_str(user,flag_names[u].text);
       };
    }
   write_str(user,"(end of list)");
   return;
  }
  
if (!instr2(0,"all",inpstr,0) )
 {
  ignore_all(user);
  write_str(user,"You are now ignoring all messages.");
  return;
 } 

u = get_flag_num(inpstr);
if (u > -1)
  {
   ustr[user].flags[u] = '1';
  
   sprintf(mess,"You are now ignoring %s.", flag_names[u].text);
   write_str(user,mess);
  }
 else
  {
   write_str(user,"That message type not known");
  }
}

char *itoa(int num) {
static char num_ascii[6];

num_ascii[0]=0;
sprintf(num_ascii,"%d",num);
return num_ascii;
}


void write_rebootdb(int user) {
int i=0;
FILE *f;
ConvPtr tmpConv;

if (!check_for_file(REBOOTFILE)) {
f=fopen(REBOOTFILE,"w");
wval(listen_sock[0]);
if (WIZ_OFFSET != 0)
 wval(listen_sock[1]);
else
 wval(-1);
if (WHO_OFFSET != 0)
 wval(listen_sock[2]);
else
 wval(-1);
if (WWW_OFFSET != 0)
 wval(listen_sock[3]);
else
 wval(-1);

if (resolve_names==2 || resolve_names==3) {
wval(FROM_CLIENT_READ);
wval(FROM_CLIENT_WRITE);
wval(resolver_clipon_pid);
}
else {
wval(-1);
wval(-1);
wval(-1);
}

wval(num_of_users);
write_log(BOOTLOG,YESTIME,"REBOOT: Writing %d users to reboot log\n",num_of_users);
}
else f=fopen(REBOOTFILE,"a");

write_log(BOOTLOG,YESTIME,"REBOOT: Writing %s to reboot log\n",ustr[user].name);

wval(user);
wbuf(ustr[user].name);
wval(ustr[user].sock);
wval(ustr[user].time);
wbuf(ustr[user].real_id);
wval(ustr[user].nerf_shots);
wval(ustr[user].nerf_energy);
wval(ustr[user].warning_given);
wval(ustr[user].ttt_board);
wval(ustr[user].ttt_playing);
wval(ustr[user].ttt_opponent);
wval(ustr[user].hang_stage);
wbuf(ustr[user].hang_word);
wbuf(ustr[user].hang_word_show);
wbuf(ustr[user].hang_guess);
wbuf(ustr[user].site);
wbuf(ustr[user].net_name);
wbuf(ustr[user].afkmsg);
wval(ustr[user].promptseq);
wval(ustr[user].t_ent);
wval(ustr[user].t_num);
wbuf(ustr[user].t_name);
wbuf(ustr[user].t_host);
wbuf(ustr[user].t_ip);
wbuf(ustr[user].t_port);
wbuf(ustr[user].phone_user);
wval(ustr[user].tempsuper);
wval(ustr[user].rwho);
wchar(ustr[user].attach_port);
wval(ustr[user].last_input);
wval(ustr[user].invite);
wval(ustr[user].term_type);
wbuf(ustr[user].page_file);

wval(NUM_LINES);
/* CYGNUS2 */
tmpConv = ustr[user].conv;
for (i=0;i<NUM_LINES;++i) wbuf(tmpConv->conv[i]);

/*
wval(ustr[user].alloced_size);
wval(ustr[user].write_offset);
wbuf(ustr[user].output_data);
*/

fclose(f);
}

void read_rebootdb(void) {
int i=0,j=0,user2=0,num_lines=0;
char buff1[ARR_SIZE];
FILE *f;

if (!(f=fopen(REBOOTFILE,"r"))) {
write_log(ERRLOG,YESTIME,"Can't read REBOOT db!\n");
return;
}

buff1[0]=0;

rval(listen_sock[0]);
rval(listen_sock[1]);
rval(listen_sock[2]);
rval(listen_sock[3]);
rval(FROM_CLIENT_READ);
rval(FROM_CLIENT_WRITE);
rval(resolver_clipon_pid);
rval(num_of_users);
write_log(BOOTLOG,YESTIME,"REBOOT: Reading %d users from reboot log\n",num_of_users);
for (i=0;i<num_of_users;++i) {
rval(user2);
rbuf(ustr[user2].name,NAME_LEN);
rval(ustr[user2].sock);
rval(ustr[user2].time);
rbuf(ustr[user2].real_id,50);
rval(ustr[user2].nerf_shots);
rval(ustr[user2].nerf_energy);
rval(ustr[user2].warning_given);
rval(ustr[user2].ttt_board);
rval(ustr[user2].ttt_playing);
rval(ustr[user2].ttt_opponent);
rval(ustr[user2].hang_stage);
rbuf(ustr[user2].hang_word,TOPIC_LEN);
rbuf(ustr[user2].hang_word_show,TOPIC_LEN);
rbuf(ustr[user2].hang_guess,TOPIC_LEN);
rbuf(ustr[user2].site,21);
rbuf(ustr[user2].net_name,64);
rbuf(ustr[user2].afkmsg,46);
rval(ustr[user2].promptseq);
rval(ustr[user2].t_ent);
rval(ustr[user2].t_num);
rbuf(ustr[user2].t_name,24);
rbuf(ustr[user2].t_host,31);
rbuf(ustr[user2].t_ip,16);
rbuf(ustr[user2].t_port,6);
rbuf(ustr[user2].phone_user,NAME_LEN);
rval(ustr[user2].tempsuper);
rval(ustr[user2].rwho);
rchar(ustr[user2].attach_port);
rval(ustr[user2].last_input);
rval(ustr[user2].invite);
rval(ustr[user2].term_type);
rbuf(ustr[user2].page_file,80);

rval(num_lines);

	/* CYGNUS2 */
	if (ustr[user2].conv == NULL) {
		ustr[user2].conv = (ConvPtr) malloc (sizeof (ConvBuffer));
		if (ustr[user2].conv == NULL)       /* malloc failed */
		{
		write_log(ERRLOG,YESTIME,"MALLOC: Failed for conv buffer in read_rebootdb() %s\n",get_error());
		continue;
		}
		init_conv_buffer(ustr[user2].conv);
	} /* end of if conv null */
	/* CYGNUS3 */
	if (ustr[user2].Macros == NULL) {
		ustr[user2].Macros = (MacroPtr) malloc (sizeof (MacroBuffer));
		if (ustr[user2].Macros == NULL)       /* malloc failed */
		{
		write_log(ERRLOG,YESTIME,"MALLOC: Failed for Macros in read_rebootdb() %s\n",get_error());
		continue;
		}
		init_macro_buffer(ustr[user2].Macros);
	} /* end of if Macros null */

j=0;
for (j=0;j<num_lines;++j) {
rbuf(buff1,-1);
/* CYGNUS2 */
if (j <= NUM_LINES) addto_conv_buffer(ustr[user2].conv, buff1);
/* if (j <= NUM_LINES) strcpy(ustr[user2].conv[j],buff1); */
buff1[0]=0;
}

/*
rval(ustr[user2].alloced_size);
rval(ustr[user2].write_offset);
ustr[user2].output_data = (char *)malloc(ustr[user2].alloced_size+1);
rbuf(buff1,-1);
strcpy(ustr[user2].output_data,buff1);
*/

/* set socket to non-blocking */
/*
#if defined(WINDOWS)
ioctlsocket(ustr[user2].sock, FIONBIO, &arg);
#else
fcntl(ustr[user2].sock, F_SETFL, NBLOCK_CMD);
#endif
*/
MY_FCNTL(ustr[user2].sock, MY_F_SETFL, NBLOCK_CMD);

add_user2(user2,0);

read_to_user(ustr[user2].name,user2);
strcpy(ustr[user2].login_name,ustr[user2].name);
if (!strcmp(ustr[user2].name,BOT_ID)) bot=user2;
write_log(BOOTLOG,YESTIME,"REBOOT: Got user sock: %d name: %s slot: %d\n",ustr[user2].sock,ustr[user2].name,user2);
} /* end of main for */

fclose(f);
remove(REBOOTFILE);
write_log(BOOTLOG,YESTIME,"REBOOT: Read %d users\n",i);

}

void backup_stuff(int user, char *inpstr) {
int timegiven=0;
char option[81];

if (!strlen(inpstr)) {
 if (dobackups) {
 dobackups=0;
 write_str(user,"Log archiving at midnight turned OFF");
 return;
 }
 else {
 dobackups=1;
 write_str(user,"Log archiving at midnight turned ON");
 return;
 }
}
else {
inpstr[80]=0;
sscanf(inpstr,"%s ",option);
  if (!strcmp(option,"-t")) {
	remove_first(inpstr);
	if (!strlen(inpstr))
	 trim_backups(user,TRIM_BACKUPS);
	else {
	 inpstr[3]=0;
	 timegiven=atoi(inpstr);
	 if (timegiven > 0) trim_backups(user,timegiven);
	 else {
	 /* bad number */
	 write_str(user,"Invalid number of days");
	 return;
	 }
	}
  } /* end of if -t */
  else if (!strcmp(option,"-b")) {
  if (backup_logs(user) != -1) write_str(user,"All logs have been archived");
  } /* end of if -b option */
 else {
 /* bad option */
 write_str(user,"That option doesn't exist!");
 return;
 }
} /* end of strlen else */

}

int backup_logs(int user) {
int i=0;
char filerid[FILE_NAME_LEN];
char filename[FILE_NAME_LEN*2];
char filename2[FILE_NAME_LEN*2];
char timestr[30];
FILE *fp;
struct tm *clocker;
time_t tm_now;

time(&tm_now);
clocker=localtime(&tm_now);
strftime(timestr,sizeof(timestr),"%m%d%Y.%H:%M:%S",clocker);
sprintf(filerid,"%s/%s",LOGDIR,timestr);
#if defined(HAVE_MKDIR_ARGS)
if (mkdir(filerid,0700) == -1) {
#else
if (mkdir(filerid) == -1) {
#endif
  if (user!=-1) write_str(user,BAD_FILEIO);
  write_log(ERRLOG,YESTIME,"BACKUP: Couldn't create directory \"%s\" in backup_logs! %s\n",filerid,get_error());
  return -1;
}

/* create a TIMESTAMP file containing the current unix time */
/* for trimming backups later */
sprintf(filename,"%s/TIMESTAMP",filerid);
if (!(fp=fopen(filename,"w"))) {
  if (user!=-1) write_str(user,BAD_FILEIO);
  write_log(ERRLOG,YESTIME,"BACKUP: Couldn't open file(w) \"%s\" in backup_logs! %s\n",filename,get_error());
  rmdir(filerid);
  return -1;
  }
sprintf(timestr,"%ld\n",(unsigned long)tm_now);
fputs(timestr,fp);
fclose(fp);

/* move all logs to new directory by renaming them */
for (i=0;logfacil[i].file!=NULL;++i) {
sprintf(filename,logfacil[i].file,LOGDIR);
sprintf(filename2,logfacil[i].file,filerid);
rename(filename,filename2);
}
/* now do lastcommand and lastlogs file */
sprintf(filename,"%s/%s",LOGDIR,LASTFILE);
sprintf(filename2,"%s/%s",filerid,LASTFILE);
rename(filename,filename2);
sprintf(filename,"%s/%s.CRASH",LOGDIR,LASTFILE);
sprintf(filename2,"%s/%s.CRASH",filerid,LASTFILE);
rename(filename,filename2);
sprintf(filename,"%s/%s",LOGDIR,LASTLOGS);
sprintf(filename2,"%s/%s",filerid,LASTLOGS);
rename(filename,filename2);

write_log(SYSTEMLOG,YESTIME,"BACKUP: All log files backed up by %s to directory \"%s\"\n",user==-1?"the TALKER":ustr[user].say_name,filerid);
return 1;
}

void trim_backups(int user, int days) {
int i=0,count=0;
char filename[FILE_NAME_LEN];
char filerid[FILE_NAME_LEN];
char filerid2[FILE_NAME_LEN];
unsigned long diff;
long tm_then;
char small_buffer[64];
time_t tm_now;
struct dirent *dp;
DIR *dirp;
FILE *fp;

if (TRIM_BACKUPS <= 0) {
if (user!=-1) write_str(user,"The system is set to not allow log trimming.");
return;
}

 time(&tm_now);
 sprintf(t_mess,"%s",LOGDIR);
 strncpy(filerid,t_mess,FILE_NAME_LEN);
 dirp=opendir((char *)filerid);
 if (dirp == NULL)
   {
	if (user!=-1) write_str(user,BAD_FILEIO);
        write_log(ERRLOG,YESTIME,"TBACKUP: Can't open directory \"%s\" for trim_backups! %s\n",filerid,get_error());
	return;
   }

 while ((dp = readdir(dirp)) != NULL)
   {
    sprintf(small_buffer,"%s",dp->d_name);
        if ((small_buffer[0] == '.') || !strstr(small_buffer,":"))
         continue;
	/* we have a log sub-directory */
	/* check the timestamp against how long we want to keep */
	sprintf(filerid2,"%s/%s",LOGDIR,small_buffer);
	sprintf(filename,"%s/TIMESTAMP",filerid2);
	if (!(fp=fopen(filename,"r"))) {
	 if (user!=-1) write_str(user,BAD_FILEIO);
         write_log(ERRLOG,YESTIME,"TBACKUP: Couldn't open file(r) \"%s\" in trim_backups! %s\n",filename,get_error());
	 continue;
	}
	 fscanf(fp,"%ld\n",&tm_then);
         FCLOSE(fp);
	 diff = tm_now - tm_then; /* seconds since now */
	 if (diff >= (days*86400)) {
	 /* longer than the time we want to keep */
		for (i=0;logfacil[i].file!=NULL;++i) {
		sprintf(filename,logfacil[i].file,filerid2);
		remove(filename);
		}
		i=0;
		sprintf(filename,"%s/%s",filerid2,LASTFILE);
		remove(filename);
		sprintf(filename,"%s/%s.CRASH",filerid2,LASTFILE);
		remove(filename);
		sprintf(filename,"%s/%s",filerid2,LASTLOGS);
		remove(filename);
		sprintf(filename,"%s/TIMESTAMP",filerid2);
		remove(filename);
		/* now that all files are removed	*/
		/* remove the empty directory		*/
		rmdir(filerid2);
		count=1;
		write_log(SYSTEMLOG,YESTIME,"TBACKUP: In trimming to %d days by %s, removed \"%s\"\n",days,user==-1?"the TALKER":ustr[user].say_name,filerid2);
	 } /* end of if */
	 diff=0;
	 tm_then=0;
   } /* end of directory while */
   (void)closedir(dirp);

if (user != -1) {
if (count) write_str(user,"Logs trimmed.");
else write_str(user,"No logs needed to be trimmed.");
}

}

void do_tracking(int mode, char *downmess) {
char thisip[21];
unsigned long thisaddr;
struct hostent *hp;


if (DO_TRACKING != 1) return;

if (mode==1) {
#if defined(HAVE_GETHOSTBYNAME)
 hp = gethostbyname(thishost);
 if (hp) {
  memcpy(&thisaddr, hp->h_addr, 4);
  thisaddr=ntohl(thisaddr);
  sprintf(thisip,"%ld.%ld.%ld.%ld", (thisaddr >> 24) & 0xff, (thisaddr >> 16) & 0xff,
         (thisaddr >> 8) & 0xff, thisaddr & 0xff);
 } /* hp */
 else strcpy(thisip,"NA");
#else
 strcpy(thisip,"NA");
#endif
} /* mode */

/* construct message */
if (mode==1) {
/* we are coming up */
sprintf(mess,
"SYSTEM_NAME: %s\nPID: %u\nSTATUS: UP\nROOT_ID: %s\nVERSION: %s\nHOSTNAME: %s\nIP: %s\nMAIN_PORT: %d\nWIZ_PORT: %d\nWHO_PORT: %d\nWWW_PORT: %d\nSYSTEM_EMAIL: %s\nNUM_USERS: %ld\nNEWUSER_STATUS: %d\nTHEME: %s\nEIGHTTPLUS: %d\nOS: %s\nEND:\n",
SYSTEM_NAME,(unsigned int)getpid(),ROOT_ID,VERSION,thishost,thisip,PORT,
PORT+WIZ_OFFSET,PORT+WHO_OFFSET,PORT+WWW_OFFSET,SYSTEM_EMAIL,
system_stats.tot_users,allow_new,TTHEME,EIGHTTPLUS,thisos);
} /* end of mode 1 */
else if (mode==0) {
/* we are going down */
	if (downmess) {
	sprintf(mess,"SYSTEM_NAME: %s\nPID: %u\nSTATUS: DOWN %s\nEND:\n",
	SYSTEM_NAME,(unsigned int)getpid(),downmess);
	}
	else {
	sprintf(mess,"SYSTEM_NAME: %s\nPID: %u\nSTATUS: DOWN Normal shutdown\nEND:\n",
	SYSTEM_NAME,(unsigned int)getpid());
	}
} /* end of mode 0 */

/* send external email */
if (send_ext_mail(-2, -2, 3, "TRACKING", mess, DATA_IS_MSG, "tinfo@asteroid-b612.org")==-1) {
	write_log(ERRLOG,YESTIME,"TRACKING: Couldn't send external email in do_tracking()\n");
	return;
}

}

void do_timeset(char *zonetime) {
char wcd1[FILE_NAME_LEN+10];
static char timemess[FILE_NAME_LEN+100];

/* SET TALKER TIME TO THE TIMEZONE WE WANT */
/* Get the current working directory so we can reference */
/* to the absolute path of the TZ info                   */
getcwd(wcd1,FILE_NAME_LEN);
strcat(wcd1,"/tzinfo");

if (!strcmp(zonetime,"localtime")) {
#if defined(FREEBSD_SYS) || defined(NETBSD_SYS)
putenv("TZ=:/etc/localtime");
#else
putenv("TZ=localtime");
#endif
}
else {
sprintf(timemess,"TZ=:%s/%s",wcd1,zonetime);
putenv(timemess);
}
#if defined(HAVE_TZSET) && !defined(__CYGWIN32__)
tzset();
#endif

}


void old_func(int user, char *inpstr, int mode) {

if (mode==1) {
/* .desc */
write_str(user,"NOTE! \".desc\" WILL BE DEPRECATED SOON! USE \".set desc\" INSTEAD!");
write_str(user,"");
set_desc(user, inpstr);
}
else if (mode==2) {
/* .entpro */
write_str(user,"NOTE! \".entpro\" WILL BE DEPRECATED SOON! USE \".set profile\" INSTEAD!");
write_str(user,"");
set_profile(user, inpstr);
}
else if (mode==3) {
/* .fail */
write_str(user,"NOTE! \".fail\" WILL BE DEPRECATED SOON! USE \".set fail\" INSTEAD!");
write_str(user,"");
set_fail(user, inpstr);
}
else if (mode==4) {
/* .succ */
write_str(user,"NOTE! \".succ\" WILL BE DEPRECATED SOON! USE \".set succ\" INSTEAD!");
write_str(user,"");
set_succ(user, inpstr);
}
else if (mode==5) {
/* .entermsg */
write_str(user,"NOTE! \".entermsg\" WILL BE DEPRECATED SOON! USE \".set entermsg\" INSTEAD!");
write_str(user,"");
set_entermsg(user, inpstr);
}
else if (mode==6) {
/* .exitmsg */
write_str(user,"NOTE! \".exitmsg\" WILL BE DEPRECATED SOON! USE \".set exitmsg\" INSTEAD!");
write_str(user,"");
set_exitmsg(user, inpstr);
}


}
