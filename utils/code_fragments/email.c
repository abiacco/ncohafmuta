#define DATA_IS_MSG 0
#define DATA_IS_FILE 1

/*
   mailsize error to recipient
    ONLINE  send_ext_email(-2, u, 0, NULL, mess, DATA_IS_MSG, NULL); 
    OFFLINE send_ext_email(-2, -1, 0, NULL, mess, DATA_IS_MSG, NULL); 
   autofwd
    ONLINE  send_ext_email(user, u, 1, NULL, mess, DATA_IS_MSG, NULL); 
    OFFLINE send_ext_email(user, -1, 1, NULL, mess, DATA_IS_MSG, NULL); 
   mailfile
	    send_ext_email(-2, -2, 2, NULL, filename2, DATA_IS_FILE, mail_addr);
   nukewarn
	    send_ext_email(-2, -1, 3, NULL, NUKEWARN, DATA_IS_FILE, NULL);
   do_tracking
	    send_ext_email(-2, -2, 3, "TRACKING", mess, DATA_IS_MSG, "tinfo");
   everify
	    send_ext_email(-2, user, 4, NULL, filename, DATA_IS_FILE, emailadd);
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
char line[512];
FILE *tfp;


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
  fputs(replline,wfp);
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

}


void send_mail(int user, char *inpstr, int mode)
{
int u=-1,ret=0,multi=0,level=0,done=0,bad=0,i=0;
int newread=0,sendmail=0,nosubject=0;
long filesize=0;
char stm[20],mess2[ARR_SIZE+25],filename[FILE_NAME_LEN],name[SAYNAME_LEN+1];
char other_user[ARR_SIZE];
char message[ARR_SIZE];
char filerid[FILE_NAME_LEN];
struct stat fileinfo;
time_t tm;
FILE *fp;
FILE *wfp=NULL;
DIR  *dirp=NULL;
struct dirent *dp;


/* Check if user is gagcommed */
if (ustr[user].gagcomm) {
   write_str(user,NO_COMM);
   return;
   }

/*-------------------------------------------------------*/
/* check for any input                                   */
/*-------------------------------------------------------*/

if (!strlen(inpstr)) 
  {
   write_str(user,"Who do you want to mail?"); 
   return;
  }

/*-------------------------------------------------------*/
/* get the other user name                               */
/*-------------------------------------------------------*/

sscanf(inpstr,"%s ",other_user);
other_user[NAME_LEN+1]=0;
CHECK_NAME(other_user);
strtolower(other_user);
remove_first(inpstr);

/*-------------------------------------------------------*/
/* check to see if a message was supplied                */
/*-------------------------------------------------------*/

if (!strlen(inpstr)) {
   write_str(user,"You have not specified a message"); 
   return;
}

if (ustr[user].frog) {
   if (mode==0)
    write_str(user,"Frogs cant write, silly.");
   return;
}

if (!check_for_user(other_user)) {
write_it(ustr[user].sock,"checking for multi\n");

if (!strcmp(other_user,"from") || !strcmp(other_user,"to")) {
	sscanf(inpstr,"%s",message);
	remove_first(inpstr);
	for (i=0;i<strlen(message);++i) {
		if (!isdigit((int)message[i])) return;
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
else if (!strcmp(other_user,"-a")) {
 multi=2;
 level=MAX_LEVEL;

write_it(ustr[user].sock,"in -a all\n");

 sprintf(t_mess,"%s",USERDIR);
 strncpy(filerid,t_mess,FILE_NAME_LEN);

 dirp=opendir((char *)filerid);

 if (dirp == NULL) {
	sprintf(t_mess,"SEND_EMAIL: Can't open users directory \"%s\" in send_email! %s\n",filerid,get_error());
	write_log(ERRLOG,YESTIME,t_mess);
	write_str(user,t_mess);
	return;
 }
write_it(ustr[user].sock,"opened user dir\n");
} /* -a */
else {
        for (i=0;i<strlen(other_user);++i) {
                if (!isdigit((int)other_user[i])) {
		/* probably a user that doesn't exist */
		write_str(user,NO_USER_STR);
		return;
		}
        }

 multi=3;
 level=atoi(other_user);
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
} /* else just one level */

if (multi && multi<4) {
 if ((ustr[user].tempsuper < WIZ_LEVEL) || (ustr[user].tempsuper < level)) {
  /* only staff members can mass-mail */
  /* and staff members can only mass-mail up to their level */
  write_str(user,"You don't have that much power!");
  (void) closedir(dirp);
  return;
 }
}

} /* bad check_for_user */
write_it(ustr[user].sock,"after check_for_user\n");

/* multi:
   1: from <level> to MAX_LEVEL
   2: from lowest level to <level>
   3: just <level>
   4: (future) just multiple users
   all is multi=2 with <level>=MAX_LEVEL
*/

while (!done) {
write_it(ustr[user].sock,"in while\n");

if (multi && multi<4) {
	if ((dp = readdir(dirp)) == NULL) {
		done=1;
		continue;
	}
	else {
write_it(ustr[user].sock,"d_name is\n");
write_it(ustr[user].sock,dp->d_name);
write_it(ustr[user].sock,"\n");
		if (dp->d_name[0]=='.') continue;
		sprintf(other_user,"%s",dp->d_name);
	}
} /* multi>0 && multi<4 */

write_it(ustr[user].sock,"other user is\n");
write_it(ustr[user].sock,other_user);
write_it(ustr[user].sock,"\n");

ret=check_gag2(user,other_user);
if (!ret) { bad=1; break; }
else if (ret==2) {
 write_str(user,BAD_FILEIO);
 write_log(ERRLOG,YESTIME,"Couldn't open file(r) for \"%s\" in check_gag2! %s\n",other_user); 
 bad=1; break;
 }

if (!read_user(other_user)) {
 write_str(user,BAD_FILEIO);
 write_log(ERRLOG,YESTIME,"Couldn't open user file for \"%s\" in check_gag2! %s\n",other_user); 
 bad=1; break;
 }

write_it(ustr[user].sock,"checking levels\n");

/* check level-based mailing */
if (multi==1 && (t_ustr.super < level)) continue;
else if (multi==2 && (t_ustr.super > level)) continue;
else if (multi==3 && (t_ustr.super != level)) continue;

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
 else if (multi==4) sprintf(message,"To you,%s: ",other_user);
}
else message[0]=0;

write_it(ustr[user].sock,"GOOD user is\n");
write_it(ustr[user].sock,other_user);
write_it(ustr[user].sock,"\n");

/*--------------------------------------------------*/
/* prepare message to be sent                       */
/*--------------------------------------------------*/
time(&tm);
midcpy(ctime(&tm),stm,4,15);
strcpy(name,ustr[user].say_name);


strcat(message,inpstr);
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

write_it(ustr[user].sock,"checking filesize\n");

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
      bad=1; break;
     }
   fputs(mess,fp);
   FCLOSE(fp);
   t_ustr.mail_warn = 1;
   t_ustr.new_mail = TRUE;
   t_ustr.mail_num++;

/*-------------------------------------------------------*/
/* write users to inform them of transaction             */
/*-------------------------------------------------------*/

if ((u=get_user_num(other_user,user))!= -1) 
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
   } /* end if autor */
   else ustr[u].mail_num++;

   if ((ustr[u].autof==1) && (ustr[u].automsgs < MAX_AUTOFORS)) {
       if ((strlen(ustr[u].email_addr) < 8) ||
           !strcmp(ustr[u].email_addr,DEF_EMAIL)) { 
           write_str(u,"Your set email address is not a valid address..aborting autofwd.");
           copy_from_user(u);
           write_user(ustr[u].name);
           bad=1; break;
           }

/*---------------------------------------------------*/
/* write email message                               */
/*---------------------------------------------------*/

if (mailgateway_port) {
        if (!(wfp=get_mailqueue_file())) {
	   if (mode==0 && !multi) {
            sprintf(mess,"%s : autofwd message cannot be written\n", syserror);
            write_str(user,mess);
	   }
           write_log(ERRLOG,YESTIME,"Couldn't open new queue file in send_mail(1)! %s\n",get_error());
           bad=1; break;
        }
       fprintf(wfp,"%s\n",SYSTEM_EMAIL);
       fprintf(wfp,"%s\n",ustr[u].email_addr);
}
else if (strstr(MAILPROG,"sendmail")) {
  sprintf(t_mess,"%s",MAILPROG);
  sendmail=1;
  }
else {
  sprintf(t_mess,"%s %s",MAILPROG,ustr[u].email_addr);
  if (strstr(MAILPROG,"-s"))
	nosubject=0;
  else
	nosubject=1;
  }  
strncpy(filename,t_mess,FILE_NAME_LEN);

if (!mailgateway_port) {
if (!(wfp=popen(filename,"w"))) 
  {
	if (mode==0 && !multi) {
   	sprintf(mess,"%s : autofwd message cannot be written\n", syserror);
   	write_str(user,mess);
	write_log(ERRLOG,YESTIME,"Couldn't open popen(w) \"%s\" in send_mail(1)! %s\n",filename,get_error());
	}
   bad=1; break;
  }
}

if (sendmail || mailgateway_port) {
fprintf(wfp,"From: %s <%s>\n",SYSTEM_NAME,SYSTEM_EMAIL);
fprintf(wfp,"To: %s <%s>\n",strip_color(ustr[u].say_name),ustr[u].email_addr);
fprintf(wfp,"Subject: Mailfile error on %s\n\n",SYSTEM_NAME);
}
else if (nosubject) {
nosubject=0;
fprintf(wfp,"Mailfile error on %s\n",SYSTEM_NAME);
}

strcpy(mess, strip_color(mess));
fputs(mess,wfp);
fputs(EXT_MAIL1,wfp);
sprintf(mess,EXT_MAIL2,SYSTEM_NAME);
fputs(mess,wfp);

fputs(EXT_MAIL4,wfp);
fputs(EXT_MAIL5,wfp);
   fputs(EXT_MAIL8,wfp);

fputs(EXT_MAIL7,wfp);
fputs(".\n",wfp);

if (mailgateway_port) fclose(wfp);
else pclose(wfp);

/*
      sprintf(mess,"%s -s \'Mailfile error on %s\' %s < %s/%s.email 2> /dev/null",MAILPROG,SYSTEM_NAME,ustr[u].email_addr,MAILDIR,ustr[u].say_name); 
      system(mess);
      remove(filename);
*/
      write_str(u,MAIL_AUTOFWD);
      ustr[u].automsgs++;
     } /* end of if autof */
  } /* end of if user online if */
 else if ((t_ustr.autof > 0) && (t_ustr.automsgs < MAX_AUTOFORS)) {
  /* USER NOT ONLINE */
     if ((strlen(t_ustr.email_addr) < 8) ||
         !strcmp(t_ustr.email_addr,DEF_EMAIL)) {
		write_user(t_ustr.name);
		bad=1; break;
	}

/*---------------------------------------------------*/
/* write email message                               */
/*---------------------------------------------------*/

if (mailgateway_port) {
        if (!(wfp=get_mailqueue_file())) {
	   if (mode==0 && !multi) {
            sprintf(mess,"%s : autofwd message cannot be written\n", syserror);
            write_str(user,mess);
	   }
           write_log(ERRLOG,YESTIME,"Couldn't open new queue file in send_mail(2)! %s\n",get_error());
           bad=1; break;
        }
       fprintf(wfp,"%s\n",SYSTEM_EMAIL);
       fprintf(wfp,"%s\n",t_ustr.email_addr);
}
else if (strstr(MAILPROG,"sendmail")) {
  sprintf(t_mess,"%s",MAILPROG);
  sendmail=1;
  }
else {
  sprintf(t_mess,"%s %s",MAILPROG,t_ustr.email_addr);
  if (strstr(MAILPROG,"-s"))
	nosubject=0;
  else
	nosubject=1;
  }  
strncpy(filename,t_mess,FILE_NAME_LEN);

if (!mailgateway_port) {
if (!(wfp=popen(filename,"w"))) 
  {
	if (mode==0 && !multi) {
 	sprintf(mess,"%s : autofwd message cannot be written\n", syserror);
   	write_str(user,mess);
	write_log(ERRLOG,YESTIME,"Couldn't open popen(w) \"%s\" in send_mail(2)! %s\n",filename,get_error());
	}
   bad=1; break;
  }
}

if (sendmail || mailgateway_port) {
fprintf(wfp,"From: %s <%s>\n",SYSTEM_NAME,SYSTEM_EMAIL);
fprintf(wfp,"To: %s <%s>\n",strip_color(t_ustr.say_name),t_ustr.email_addr);
fprintf(wfp,"Subject: Mailfile error on %s\n\n",SYSTEM_NAME);
}
else if (nosubject) {
nosubject=0;
fprintf(wfp,"Mailfile error on %s\n",SYSTEM_NAME);
}

strcpy(mess, strip_color(mess));
fputs(mess,wfp);
fputs(EXT_MAIL1,wfp);
sprintf(mess,EXT_MAIL2,SYSTEM_NAME);
fputs(mess,wfp);

fputs(EXT_MAIL4,wfp);
fputs(EXT_MAIL5,wfp);
   fputs(EXT_MAIL8,wfp);

fputs(EXT_MAIL7,wfp);
fputs(".\n",wfp);

if (mailgateway_port) fclose(wfp);
else pclose(wfp);

/*
   sprintf(mess,"%s -s \'Mailfile error on %s\' %s < %s/%s.email 2> /dev/null",MAILPROG,SYSTEM_NAME,t_ustr.email_addr,MAILDIR,t_ustr.say_name); 
   system(mess);
   remove(filename);
*/
   t_ustr.automsgs++;
   } /* end of if autof */
 } /* end of if mail warn is 0 */
 else { }

 write_user(other_user);

 /* If recepient is online and has autoread in dual mode, */
 /* read their new message                                */

 if (u && newread) read_mail(u,"1");

 bad=1; break;
 } /* end of if over filesize */

/* End of mailfile size check */

filesize = 0;

/*---------------------------------------------------*/
/* write mail message                                */
/*---------------------------------------------------*/

if (!(fp=fopen(filename,"a"))) 
  {
	if (mode==0 && !multi) {
   	sprintf(mess,"%s : message cannot be written\n", syserror);
  	write_str(user,mess);
	}
   bad=1; break;
  }
fputs(mess,fp);
FCLOSE(fp);

/*--------------------------------------------------*/
/* set a new mail flag for that other user          */
/*--------------------------------------------------*/

t_ustr.new_mail = TRUE;
t_ustr.mail_num++;

/*--------------------------------------------------------*/
/* write sent mail message                                */
/*--------------------------------------------------------*/

/*   TAKEN OUT AFTER MULTIS PUT IN. NOTIFICATION NOW AT FUNCTION END
sprintf(mess2,"(%s) To %s: %s\n",stm,other_user,message);

sprintf(t_mess,"%s/%s.sent",MAILDIR,ustr[user].name);
strncpy(filename,t_mess,FILE_NAME_LEN);

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
 if (!(fp=fopen(filename,"a"))) 
  {
	if (mode==0 && !multi) {
  	sprintf(mess2,"%s : sent mail message cannot be written\n", syserror);
   	write_str(user,mess2);
	}
   goto NEXT;
  }
 fputs(mess2,fp);
 FCLOSE(fp);
}

NEXT:

filesize = 0;
*/

/*-------------------------------------------------------*/
/* write users to inform them of transaction             */
/*-------------------------------------------------------*/

/*   TAKEN OUT AFTER MULTIS PUT IN. NOTIFICATION NOW AT FUNCTION END
if (mode==0 && !multi) {
  sprintf(t_mess,MAIL_TO,other_user);
  write_str(user,t_mess);
  }
*/

/* Inform recipient of new mail */
if ((u=get_user_num_exact(other_user,user))!= -1) 
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
   }
   else ustr[u].mail_num++;

   if ((ustr[u].autof==1) && (ustr[u].automsgs < MAX_AUTOFORS)) {
       if ((strlen(ustr[u].email_addr) < 8) ||
           !strcmp(ustr[u].email_addr,DEF_EMAIL)) { 
           write_str(u,"Your set email address is not a valid address..aborting autofwd.");
           copy_from_user(u);
           write_user(ustr[u].name);
           bad=1; break;
           }

/*---------------------------------------------------*/
/* send external autofwd email message               */
/*---------------------------------------------------*/

if (mailgateway_port) {
        if (!(wfp=get_mailqueue_file())) {
	   if (mode==0 && !multi) {
            sprintf(mess,"%s : autofwd message cannot be written\n", syserror);
            write_str(user,mess);
	   }
           write_log(ERRLOG,YESTIME,"Couldn't open new queue file in send_mail(3)! %s\n",get_error());
           bad=1; break;
        }
       if (!ustr[user].semail && strstr(ustr[user].email_addr,"@"))
        fprintf(wfp,"%s\n",ustr[user].email_addr);
       else
        fprintf(wfp,"%s\n",SYSTEM_EMAIL);

       fprintf(wfp,"%s\n",ustr[u].email_addr);
}
else if (strstr(MAILPROG,"sendmail")) {
  sprintf(t_mess,"%s",MAILPROG);
  sendmail=1;
  }
else {
  sprintf(t_mess,"%s %s",MAILPROG,ustr[u].email_addr);
  if (strstr(MAILPROG,"-s"))
	nosubject=0;
  else
	nosubject=1;
  }  
strncpy(filename,t_mess,FILE_NAME_LEN);

if (!mailgateway_port) {
if (!(wfp=popen(filename,"w"))) 
  {
	if (mode==0 && !multi) {
   	sprintf(mess,"%s : autofwd message cannot be written\n", syserror);
   	write_str(user,mess);
	write_log(ERRLOG,YESTIME,"Couldn't open popen(w) \"%s\" in send_mail(3)! %s\n",filename,get_error());
	}
   goto NEXT3;
  }
}

if (sendmail || mailgateway_port) {
if (!ustr[user].semail && strstr(ustr[user].email_addr,"@"))
fprintf(wfp,"From: %s on %s <%s>\n",strip_color(ustr[user].say_name),SYSTEM_NAME,ustr[user].email_addr);
else
fprintf(wfp,"From: %s <%s>\n",SYSTEM_NAME,SYSTEM_EMAIL);

fprintf(wfp,"To: %s <%s>\n",strip_color(ustr[u].say_name),ustr[u].email_addr);
fprintf(wfp,"Subject: New smail from %s on %s\n\n",strip_color(ustr[user].say_name),SYSTEM_NAME);
}
else if (nosubject) {
nosubject=0;
fprintf(wfp,"New smail from %s on %s\n",strip_color(ustr[user].say_name),SYSTEM_NAME);
}

strcpy(mess, strip_color(mess));
fputs(mess,wfp);
fputs(EXT_MAIL1,wfp);
sprintf(mess,EXT_MAIL2,SYSTEM_NAME);
fputs(mess,wfp);
if (ustr[user].semail)
 fputs(EXT_MAIL3,wfp);

fputs(EXT_MAIL4,wfp);
fputs(EXT_MAIL5,wfp);
if (sendmail || mailgateway_port) {
if (!ustr[user].semail && strstr(ustr[user].email_addr,"@")) {
   fputs(EXT_MAIL8,wfp);
   }
else {
   fputs(EXT_MAIL9,wfp);
   }
}
else {
if (!ustr[user].semail && strstr(ustr[user].email_addr,"@")) {
   sprintf(mess,EXT_MAIL10,ustr[user].email_addr);
   fputs(mess,wfp);
   }
else {
   fputs(EXT_MAIL9,wfp);
   }
}

fputs(EXT_MAIL7,wfp);
fputs(".\n",wfp);

if (mailgateway_port) fclose(wfp);
else pclose(wfp);

/*
      sprintf(mess,"%s %s < %s/%s.email 2> /dev/null",MAILPROG,ustr[u].email_addr,MAILDIR,ustr[u].name); 
      system(mess);
      remove(filename);
*/
      write_str(u,MAIL_AUTOFWD);
      ustr[u].automsgs++;
     } /* end of if autof */
  } /* end of if user online if */
 else if ((t_ustr.autof > 0) && (t_ustr.automsgs < MAX_AUTOFORS)) {
  /* USER NOT ONLINE */
     if ((strlen(t_ustr.email_addr) < 8) ||
         !strcmp(t_ustr.email_addr,DEF_EMAIL)) {
		write_user(t_ustr.name);
		bad=1; break;
	}

/*---------------------------------------------------*/
/* send external autofwd email message               */
/*---------------------------------------------------*/

if (mailgateway_port) {
        if (!(wfp=get_mailqueue_file())) {
	   if (mode==0 && !multi) {
            sprintf(mess,"%s : autofwd message cannot be written\n", syserror);
            write_str(user,mess);
	   }
           write_log(ERRLOG,YESTIME,"Couldn't open new queue file in send_mail(4)! %s\n",get_error());
           bad=1; break;
        }
       if (!ustr[user].semail && strstr(ustr[user].email_addr,"@"))
        fprintf(wfp,"%s\n",ustr[user].email_addr);
       else
        fprintf(wfp,"%s\n",SYSTEM_EMAIL);

       fprintf(wfp,"%s\n",t_ustr.email_addr);
}
else if (strstr(MAILPROG,"sendmail")) {
  sprintf(t_mess,"%s",MAILPROG);
  sendmail=1;
  }
else {
  sprintf(t_mess,"%s %s",MAILPROG,t_ustr.email_addr);
  if (strstr(MAILPROG,"-s"))
	nosubject=0;
  else
	nosubject=1;
  }  
strncpy(filename,t_mess,FILE_NAME_LEN);

if (!mailgateway_port) {
if (!(wfp=popen(filename,"w"))) 
  {
	if (mode==0 && !multi) {
   	sprintf(mess,"%s : autofwd message cannot be written\n", syserror);
   	write_str(user,mess);
	write_log(ERRLOG,YESTIME,"Couldn't open popen(w) \"%s\" in send_mail(4)! %s\n",filename,get_error());
	}
   goto NEXT3;
  }
}

if (sendmail || mailgateway_port) {
if (!ustr[user].semail && strstr(ustr[user].email_addr,"@"))
fprintf(wfp,"From: %s on %s <%s>\n",strip_color(ustr[user].say_name),SYSTEM_NAME,ustr[user].email_addr);
else
fprintf(wfp,"From: %s <%s>\n",SYSTEM_NAME,SYSTEM_EMAIL);

fprintf(wfp,"To: %s <%s>\n",strip_color(t_ustr.say_name),t_ustr.email_addr);
fprintf(wfp,"Subject: New smail from %s on %s\n\n",strip_color(ustr[user].say_name),SYSTEM_NAME);
}
else if (nosubject) {
nosubject=0;
fprintf(wfp,"New smail from %s on %s\n",strip_color(ustr[user].say_name),SYSTEM_NAME);
}

strcpy(mess, strip_color(mess));
fputs(mess,wfp);
fputs(EXT_MAIL1,wfp);
sprintf(mess,EXT_MAIL2,SYSTEM_NAME);
fputs(mess,wfp);
if (ustr[user].semail)
 fputs(EXT_MAIL3,wfp);

fputs(EXT_MAIL4,wfp);
fputs(EXT_MAIL5,wfp);
if (sendmail || mailgateway_port) {
if (!ustr[user].semail && strstr(ustr[user].email_addr,"@")) {
   fputs(EXT_MAIL8,wfp);
   }
else {
   fputs(EXT_MAIL9,wfp);
   }
}
else {
if (!ustr[user].semail && strstr(ustr[user].email_addr,"@")) {
   sprintf(mess,EXT_MAIL10,ustr[user].email_addr);
   fputs(mess,wfp);
   }
else {
   fputs(EXT_MAIL9,wfp);
   }
}

fputs(EXT_MAIL7,wfp);
fputs(".\n",wfp);

if (mailgateway_port) fclose(wfp);
else pclose(wfp);

/*
   sprintf(mess,"%s %s < %s/%s.email 2> /dev/null",MAILPROG,t_ustr.email_addr,MAILDIR,t_ustr.name); 
   system(mess);
   remove(filename);
*/
   t_ustr.automsgs++;
   }

NEXT3:
write_user(other_user);

/* If recepient is online and has autoread in dual mode, */
/* read their new message                                */

if (u && newread) read_mail(u,"1");

if (!multi) done=1; /* email to only one user, stop */
} /* while !done */

if (multi && multi<4) (void) closedir(dirp);

if (bad) {
write_str(user,"Your mail did not reach the intended recipients because of an error.");
return;
}
else {
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
 if (!(fp=fopen(filename,"a"))) 
  {
	if (mode==0 && !multi) {
  	sprintf(mess2,"%s : sent mail message cannot be written\n", syserror);
   	write_str(user,mess2);
	}
  }
 else {
	fputs(mess2,fp);
	FCLOSE(fp);
 }
}


 /* Notify user of success */
 if (!multi) {
  sprintf(t_mess,MAIL_TO,other_user);
 }
 else if (multi==1) {
  sprintf(t_mess,"== You have mailed all users from level %d to %d a message ==",level,MAX_LEVEL);
 }
 else if (multi==2) {
  if (level==MAX_LEVEL)
   strcpy(t_mess,"== You have mailed all users a message ==");
  else
   sprintf(t_mess,"== You have mailed all users up to level %d a message ==",level);
 }
 else if (multi==3) {
  sprintf(t_mess,"== You have mailed all users of level %d a message ==",level);
 }
 else if (multi==4) {
  sprintf(t_mess,"== You have mailed %s a message ==",other_user);
 }

 write_str(user,t_mess);
}

return;
}
