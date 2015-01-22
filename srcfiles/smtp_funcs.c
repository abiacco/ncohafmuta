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

extern char mess[ARR_SIZE+25];
extern char t_mess[ARR_SIZE+25];
extern char thishost[101];     /* FQDN were running on */
extern char mailgateway_ip[16];
extern unsigned int mailgateway_port;
extern int PORT;


/* Make a connection to the SMTP server */
int do_smtp_connect(void) {
        struct  sockaddr_in     raddr;
        struct  hostent         *hp;
        int                     fd;
	int			i=0;
        int                     size=sizeof(struct sockaddr_in);
        char                    *p;

/* Zero out memory for address */
memset((char *)&raddr, 0, size);

        p = mailgateway_ip;
        while(*p != '\0' && (*p == '.' || isdigit((int)*p)))
                p++;

        /* not all digits or dots */
        if(*p != '\0') {
                if((hp = gethostbyname(mailgateway_ip)) == (struct hostent *)0) {
                        write_log(ERRLOG,YESTIME,"SMTP: Unknown hostname %s, can't make smtp connection\n",mailgateway_ip);
                        return -1;
                }

                (void)bcopy(hp->h_addr,(char *)&raddr.sin_addr,hp->h_length);
        }
        else {
                unsigned long   f;

                if((f = inet_addr(mailgateway_ip)) == -1L) {
                        write_log(ERRLOG,YESTIME,"SMTP: Unknown ip address %s, can't make smtp connection\n",mailgateway_ip);
                        return -1;
                        }
                (void)bcopy((char *)&f,(char *)&raddr.sin_addr,sizeof(f));
        }

        raddr.sin_port = htons((unsigned short)mailgateway_port);
        raddr.sin_family = AF_INET;

        if ((fd = socket(AF_INET,SOCK_STREAM,0)) == INVALID_SOCKET) {
                write_log(ERRLOG,YESTIME,"SMTP: Socket creation failed for %s %d! %s\n",mailgateway_ip,mailgateway_port,get_error());
#if !defined(WINDOWS)
                reset_alarm();
#endif
                return -1;
        }

        if (MY_FCNTL(fd,MY_F_SETFL,NBLOCK_CMD)==SOCKET_ERROR) {
                write_log(ERRLOG,YESTIME,"BLOCK: ERROR setting smtp socket to non-blocking %s\n",get_error());
		SHUTDOWN(fd, 2);
		CLOSE(fd);
                return -1;
        }

	if ( (i = find_free_slot('5') ) == -1 ) {
		write_log(ERRLOG,YESTIME,"SMTP: Can't find free slot for connection!\n");
		SHUTDOWN(fd, 2);
		CLOSE(fd);
		return -1;
	}

miscconn[i].sock=fd;
miscconn[i].type=2;
miscconn[i].stage=0;
miscconn[i].port=mailgateway_port;
miscconn[i].time=time(0);

              if (log_misc_connect(i,raddr.sin_addr.s_addr,4) == -1) {
		 write_log(ERRLOG,YESTIME,"SMTP: Can't write connection to log!\n");
                 free_sock(i,'5');
                 return -1;  
                }

         
/* we may not even get any error except for EINPROGRESS since we set the socket non-blocking */
/* get_input will be the function that tells us when this stuff happens */
        if ((connect(fd, (struct sockaddr *)&raddr, sizeof(raddr)) == SOCKET_ERROR) &&
                (errno != EINPROGRESS)) {
           if (errno == ECONNREFUSED)
                write_log(ERRLOG,YESTIME,"SMTP: Connection refused to server!  Server may be down.");
           else if (errno == ETIMEDOUT)
                write_log(ERRLOG,YESTIME,"SMTP: Connection timed out to server!  Server or internet route may be down.");
           else if (errno == ENETUNREACH)
                write_log(ERRLOG,YESTIME,"SMTP: Network remote server is on is unreachable!");
           else
                write_log(ERRLOG,YESTIME,"SMTP: Unknown problem. Try later.");
         
                /* Uncomment if you want connection errors logged
                write_log(ERRLOG,YESTIME,"SMTP: Connection failed to %s %d %s\n",mailgateway_ip,mailgateway_port,get_error());
                */
                
                free_sock(i,'5');    
		return -1;
        }

        write_log(SYSTEMLOG,YESTIME,"SMTP: sck#%d:slt#%d:Connection to %s %d in progress..\n",
	miscconn[i].sock,i,mailgateway_ip,mailgateway_port);
	return 1;
}



/* Put one or all queue files being processed back in the hold queue */
void requeue_smtp(int user) {
int i=0;
char filename[FILE_NAME_LEN];
char filename2[FILE_NAME_LEN];
char small_buff[64];
DIR *dirp;
struct dirent *dp;

if (user!=-1) {
 if (strlen(miscconn[user].queuename)) {
 snprintf(filename,sizeof(filename),"%s/%s",MAILDIR_SMTP_ACTIVE,miscconn[user].queuename);
 snprintf(filename2,sizeof(filename2),"%s/%s",MAILDIR_SMTP_QUEUE,miscconn[user].queuename);
 rename(filename,filename2);
#if defined(SMTP_DEBUG)
 write_log(DEBUGLOG,YESTIME,"SMTP: Re-queued queue file %s (single)\n",miscconn[user].queuename);
#endif
 }
}
else {

dirp=opendir((char *)MAILDIR_SMTP_ACTIVE);
  
if (dirp == NULL) {
        write_log(ERRLOG,YESTIME,"Failed to open directory \"%s\" for reading in requeue_smtp %s\n",MAILDIR_SMTP_ACTIVE);
        return;
} 

 while ((dp = readdir(dirp)) != NULL)
   {    
    sprintf(small_buff,"%s",dp->d_name);
       if (small_buff[0]=='.') continue;
        /* if we got here, we found a queue file */
	i=find_queue_slot(small_buff);
	if (i!=-1) {
	 FCLOSE(miscconn[i].fd);
	 free_sock(i,'5');
	}
	snprintf(filename,sizeof(filename),"%s/%s",MAILDIR_SMTP_ACTIVE,small_buff);
	snprintf(filename2,sizeof(filename2),"%s/%s",MAILDIR_SMTP_QUEUE,small_buff);
	rename(filename,filename2);
#if defined(SMTP_DEBUG)
 write_log(DEBUGLOG,YESTIME,"SMTP: Re-queued queue file %s (all)\n",small_buff);
#endif
   }
   (void)closedir(dirp);

} /* else */

}


/* Move a queue file from the hold queue to the active queue for processing */
int queuetoactive_smtp(int user) {
int count=0;
char filename[FILE_NAME_LEN];
char filename2[FILE_NAME_LEN];
char small_buff[64];
DIR *dirp;
struct dirent *dp;

dirp=opendir((char *)MAILDIR_SMTP_QUEUE);

if (dirp == NULL) {
	write_log(ERRLOG,YESTIME,"Failed to open directory \"%s\" for reading in queuetoactive_smtp %s\n",MAILDIR_SMTP_QUEUE);
	return -1;
}

 while ((dp = readdir(dirp)) != NULL)
   {
    sprintf(small_buff,"%s",dp->d_name);
       if (small_buff[0]=='.') continue;
	/* if we got here, we found a queue file */
	count=1;
	break;
   }
   (void)closedir(dirp);

if (!count) {
	return -1;
}

midcpy(small_buff,miscconn[user].queuename,0,sizeof(miscconn[user].queuename)-1);
snprintf(filename,sizeof(filename),"%s/%s",MAILDIR_SMTP_QUEUE,miscconn[user].queuename);
snprintf(filename2,sizeof(filename2),"%s/%s",MAILDIR_SMTP_ACTIVE,miscconn[user].queuename);
if (rename(filename,filename2) == -1) return -1;
#if defined(SMTP_DEBUG)
 write_log(DEBUGLOG,YESTIME,"SMTP: Moved queue file %s to active queue\n",miscconn[user].queuename);
#endif

return 1;
}


/* Write out SMTP commands or data to the SMTP server */
int write_smtp_data(int user, int type) {
unsigned int cnt=0;

if (type != 4) {

 if (type==0) {
  sprintf(mess,"HELO %s\r\n",thishost);
 } /* if type 0 */
 else if (type==1) {
  fgets(t_mess,256,miscconn[user].fd);
  t_mess[strlen(t_mess)-1]=0;
  sprintf(mess,"MAIL FROM: <%s>\r\n",t_mess);
 } /* if type 1 */
 else if (type==2) {
  fgets(t_mess,256,miscconn[user].fd);
  t_mess[strlen(t_mess)-1]=0;
  sprintf(mess,"RCPT TO: <%s>\r\n",t_mess);
 } /* if type 2 */
 else if (type==3) {
  sprintf(mess,"DATA\r\n");
 } /* if type 3 */

 cnt = S_WRITE(miscconn[user].sock,mess,strlen(mess));

 if (cnt < strlen(mess)) {                                         
  switch(type) {
	case 0:
  	write_log(ERRLOG,YESTIME,"SMTP: Outgoing:sck#%d:slt#%d:NA:bad HELO write %s\n",
  	miscconn[user].sock,user,cnt==-1?get_error():"(partial)");
	break;
	case 1:
  	write_log(ERRLOG,YESTIME,"SMTP: Outgoing:sck#%d:slt#%d:%s:bad MAIL FROM write %s\n",
  	miscconn[user].sock,user,miscconn[user].queuename,cnt==-1?get_error():"(partial)");
	FCLOSE(miscconn[user].fd);
	miscconn[user].fd=NULL;
	break;
	case 2:
  	write_log(ERRLOG,YESTIME,"SMTP: Outgoing:sck#%d:slt#%d:%s:bad RCPT TO write %s\n",
  	miscconn[user].sock,user,miscconn[user].queuename,cnt==-1?get_error():"(partial)");
	FCLOSE(miscconn[user].fd);
	miscconn[user].fd=NULL;
	break;
	case 3:
  	write_log(ERRLOG,YESTIME,"SMTP: Outgoing:sck#%d:slt#%d:%s:bad DATA write %s\n",
  	miscconn[user].sock,user,miscconn[user].queuename,cnt==-1?get_error():"(partial)");
	FCLOSE(miscconn[user].fd);
	miscconn[user].fd=NULL;
	break;
	default: break;
  } /* switch */
  return -1;
 } /* bad cnt */
} /* type not 4/BODY */
else {

 while (fgets(mess,256,miscconn[user].fd) != NULL) {

  mess[strlen(mess)-1]=0;
  strcat(mess,"\r\n");

  cnt = S_WRITE(miscconn[user].sock,mess,strlen(mess));

  if (cnt < strlen(mess)) {                                         
  	write_log(ERRLOG,YESTIME,"SMTP: Outgoing:sck#%d:slt#%d:%s:bad BODY write %s\n",
  	miscconn[user].sock,user,miscconn[user].queuename,cnt==-1?get_error():"(partial)");
	FCLOSE(miscconn[user].fd);
	miscconn[user].fd=NULL;
	return -1;
  } /* bad cnt */

 } /* while fp */
 FCLOSE(miscconn[user].fd);
 miscconn[user].fd=NULL;

} /* BODY write */

return 1;
}


/* Find the queue slot for a particular queue file/name */
int find_queue_slot(char *inpstr) {
int u=0;

for (u=0;u<MAX_MISC_CONNECTS;++u) {
	if (!strcmp(miscconn[u].queuename,inpstr)) return u;
}

return -1;
}


/* Get an unused name for a new SMTP queue file */
FILE *get_mailqueue_file(void) {
time_t tm;
int subtime=0;
char filename[FILE_NAME_LEN];
static FILE *tfp;
  
        tm=time(0);
                
        sprintf(filename,"%s/%ld.%d",MAILDIR_SMTP_QUEUE,(long)tm,subtime);
        while ((tfp=fopen(filename,"r")) && subtime != 100) {
                fclose(tfp);
                subtime++;
                sprintf(filename,"%s/%ld.%d",MAILDIR_SMTP_QUEUE,(long)tm,subtime);
        }
        if (subtime==100) {
                write_log(ERRLOG,YESTIME,"Could not open a new queue file in fmail()\n");
                return NULL;
        }
        else {
#if defined(SMTP_DEBUG)
 write_log(DEBUGLOG,YESTIME,"SMTP: Found unused name %s for new queue file\n",filename);
#endif
         if (!(tfp=fopen(filename,"w"))) {
                write_log(ERRLOG,YESTIME,"Could not open the new queue file %s in fmail() %s\n",filename,get_error());
                return NULL;
         }
         else return tfp; 
        }
        return NULL;
}


/*
   mailsize error to recipient
    ONLINE  send_ext_mail(-2, u, 0, NULL, mess, DATA_IS_MSG, NULL); 
    OFFLINE send_ext_mail(-2, -1, 0, NULL, mess, DATA_IS_MSG, NULL); 
   autofwd
    ONLINE  send_ext_mail(user, u, 1, NULL, mess, DATA_IS_MSG, NULL); 
    OFFLINE send_ext_mail(user, -1, 1, NULL, mess, DATA_IS_MSG, NULL); 
   mailfile
	    send_ext_mail(-2, -2, 2, NULL, filename2, DATA_IS_FILE, mail_addr);
   nukewarn
	    send_ext_mail(-2, -1, 3, NULL, NUKEWARN, DATA_IS_FILE, NULL);
   do_tracking
	    send_ext_mail(-2, -2, 3, "TRACKING", mess, DATA_IS_MSG, "tinfo");
   everify
	    send_ext_mail(-2, user, 4, NULL, filename, DATA_IS_FILE, emailadd);
*/
/* user   >= 0 : user did		*/
/* user   == -1: temp struct		*/
/* user   == -2: talker			*/
/* target >= 0 : user			*/
/* target == -1: temp struct		*/
/* mode   == 0 : mailsize error		*/
/* mode   == 1 : autofwd		*/
/* mode   == 2 : mailfile		*/
/* mailmess : message to send or file to send if mode == 2	*/
/* mailtype : is data sending a text message or a whole file?	*/
/* recipient: email address to send to if specified		*/
int send_ext_mail(int user, int target, int mode, char *subject, char *mailmess, int mailtype, char *recipient) {
int sendmail=0,nosubject=0;
char fromaddr[100];
char fromline[512];
char toaddr[100];
char toline[512];
char subjline[512];
char replyline[256];
char filename[256];
char line[512];
FILE *tfp=NULL;
FILE *wfp=NULL;


/* CREATE RECIPIENT ENVELOPE */
if (recipient != NULL) {
 sprintf(toline,"To: %s",recipient);
 strncpy(toaddr,recipient,sizeof(toaddr));
}
else {
 if (target >= 0) {
  /* recipient is a user */
  sprintf(toline,"To: %s <%s>",strip_color(ustr[target].say_name),ustr[target].email_addr);
  strncpy(toaddr,ustr[target].email_addr,sizeof(toaddr));
 }
 else if (target == -1) {
  sprintf(toline,"To: %s <%s>",strip_color(t_ustr.say_name),t_ustr.email_addr);
  strncpy(toaddr,t_ustr.email_addr,sizeof(toaddr));
 }
}

/* CREATE SENDER ENVELOPE */
if (user >= 0) {
 /* sender is a user */
 if (!ustr[user].semail && strstr(ustr[user].email_addr,"@")) {
  sprintf(fromline,"From: %s on %s <%s>",strip_color(ustr[user].say_name),SYSTEM_NAME,ustr[user].email_addr);
  strcpy(fromaddr,ustr[user].email_addr);
  strcpy(replyline,EXT_MAIL8);
 }
 else {
  sprintf(fromline,"From: %s <%s>",SYSTEM_NAME,SYSTEM_EMAIL);
  strcpy(fromaddr,SYSTEM_EMAIL);
  strcpy(replyline,EXT_MAIL9);
 }
}
else if (user == -1) {
 /* sender is a temp structure */
 if (!t_ustr.semail && strstr(t_ustr.email_addr,"@")) {
  sprintf(fromline,"From: %s on %s <%s>",strip_color(t_ustr.say_name),SYSTEM_NAME,t_ustr.email_addr);
  strcpy(fromaddr,t_ustr.email_addr);
  strcpy(replyline,EXT_MAIL8);
 }
 else {
  sprintf(fromline,"From: %s <%s>",SYSTEM_NAME,SYSTEM_EMAIL);
  strcpy(fromaddr,SYSTEM_EMAIL);
  strcpy(replyline,EXT_MAIL9);
 }
}
else if (user == -2) {
 /* sender is the talker */
 sprintf(fromline,"From: %s <%s>",SYSTEM_NAME,SYSTEM_EMAIL);
 strcpy(fromaddr,SYSTEM_EMAIL);
 strcpy(replyline,EXT_MAIL9);
}

/* CREATE SUBJECT HEADER */
if (!mode) {
 if (subject==NULL) sprintf(subjline,"Mailfile error on %s",SYSTEM_NAME);
 else sprintf(subjline,"%s",subject);
} /* !mode */
else if (mode==1) {
 if (subject==NULL) sprintf(subjline,"New smail from %s on %s",strip_color(ustr[user].say_name),SYSTEM_NAME);
 else sprintf(subjline,"%s",subject);
} /* mode == 1 */
else if (mode==2) {
 if (subject==NULL) sprintf(subjline,"Your mailfile from %s",SYSTEM_NAME);
 else sprintf(subjline,"%s",subject);
} /* mode == 2 */
else if (mode==3) {
 if (subject==NULL) sprintf(subjline,"Your account on %s",SYSTEM_NAME);
 else sprintf(subjline,"%s",subject);
} /* mode == 3 */
else if (mode==4) {
 if (subject==NULL) sprintf(subjline,"%s new account info",SYSTEM_NAME);
 else sprintf(subjline,"%s",subject);
} /* mode == 4 */

/* OPEN FILE IF WE HAVE TO */

if (mailtype==DATA_IS_FILE) {
 if (!(tfp=fopen(mailmess,"r"))) {
  write_log(ERRLOG,YESTIME,"Couldn't open mailfile(r) \"%s\" in send_ext_email! %s\n",mailmess,get_error());
  return -1;
 }
}

/* SEND MESSAGE ENVELOPE */

if (mailgateway_port) {
        if (!(wfp=get_mailqueue_file())) {
           write_log(ERRLOG,YESTIME,"Couldn't open new queue file in send_ext_mail(1)! %s\n",get_error());
	   return -1;
        }
       fprintf(wfp,"%s\n",fromaddr);
       fprintf(wfp,"%s\n",toaddr);
}
else if (strstr(MAILPROG,"sendmail")) {
  sprintf(t_mess,"%s",MAILPROG);
  sendmail=1;
  }
else {
  sprintf(t_mess,"%s %s",MAILPROG,toaddr);
  if (strstr(MAILPROG,"-s"))
	nosubject=0;
  else
	nosubject=1;
  }  
strncpy(filename,t_mess,FILE_NAME_LEN);

if (!mailgateway_port) {
if (!(wfp=popen(filename,"w"))) 
  {
	write_log(ERRLOG,YESTIME,"Couldn't open popen(w) \"%s\" in send_ext_mail(1)! %s\n",filename,get_error());
	return -1;
  }
}

/* SEND MESSAGE HEADERS */

if (sendmail || mailgateway_port) {
fprintf(wfp,"%s\n",fromline);
fprintf(wfp,"%s\n",toline);
fprintf(wfp,"Subject: %s\n\n",subjline);
}
else if (nosubject) {
nosubject=0;
fprintf(wfp,"%s\n",subjline);
}

/* SEND MESSAGE DATA */

if (mailtype==DATA_IS_MSG) {
 strcpy(mailmess, strip_color(mailmess));
 fputs(mailmess,wfp);
} /* mailtype == DATA_IS_MSG */
else if (mailtype==DATA_IS_FILE) {
fgets(line,512,tfp);
strcpy(line,check_var(line,SYS_VAR,SYSTEM_NAME));
if (target>=0) strcpy(line,check_var(line,USER_VAR,ustr[target].say_name));
else if (target==-1) strcpy(line,check_var(line,USER_VAR,t_ustr.say_name));
strcpy(line,check_var(line,HOST_VAR,thishost));
strcpy(line,check_var(line,MAINPORT_VAR,itoa(PORT)));
strcpy(line,check_var(line,"%var1%",itoa(TIME_TO_GO)));
while (!feof(tfp)) {
   fputs(strip_color(line),wfp);
   fgets(line,512,tfp);
   strcpy(line,check_var(line,SYS_VAR,SYSTEM_NAME));
   if (target>=0) strcpy(line,check_var(line,USER_VAR,ustr[target].say_name));
   else if (target==-1) strcpy(line,check_var(line,USER_VAR,t_ustr.say_name));
   strcpy(line,check_var(line,HOST_VAR,thishost));
   strcpy(line,check_var(line,MAINPORT_VAR,itoa(PORT)));
   strcpy(line,check_var(line,"%var1%",itoa(TIME_TO_GO)));
  } /* end of while */
fclose(tfp);
fputs("\n",wfp);
} /* mailtype == DATA_IS_FILE */

fputs(EXT_MAIL1,wfp);

/* SEND MESSAGE FOOTER */

if (!mode) {
 sprintf(mess,EXT_MAIL2,SYSTEM_NAME);
 fputs(mess,wfp);
 fputs(EXT_MAIL4,wfp);
 fputs(EXT_MAIL5,wfp);
 fputs(replyline,wfp);
} /* !mode */
else if (mode==1) {
 sprintf(mess,EXT_MAIL2,SYSTEM_NAME);
 fputs(mess,wfp);
 fputs(EXT_MAIL4,wfp);
 fputs(EXT_MAIL5,wfp);
 if (sendmail || mailgateway_port)
  fputs(replyline,wfp);
 else {
  sprintf(mess,EXT_MAIL10,ustr[user].email_addr);
  fputs(mess,wfp);
 }
} /* mode == 1 */
else if (mode==4) {
fputs("\n",wfp);
sprintf(mess," Name/Login name: %s\n",ustr[target].login_name);
fputs(mess,wfp);
sprintf(mess," Password       : %s\n",ustr[target].login_pass);
fputs(mess,wfp);
fputs("\n\n",wfp);

strncpy(mailmess,AGREEFILE,FILE_NAME_LEN);

 if (!(tfp=fopen(mailmess,"r"))) {
  write_log(ERRLOG,YESTIME,"Couldn't open file(r) \"%s\" in send_ext_email! %s\n",mailmess,get_error());
 }
 else {
  fgets(line,512,tfp);

  while (!feof(tfp)) {
     fputs(line,wfp);
     fgets(line,512,tfp);
    } /* end of while */
  fclose(tfp);
 } /* end of else */
} /* mode == 4 */

fputs(EXT_MAIL7,wfp);
fputs(".\n",wfp);

if (mailgateway_port) fclose(wfp);
else pclose(wfp);

return 1;
}

