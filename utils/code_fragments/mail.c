send_mail(user,inpstr)
int user;
char *inpstr;
{
int u,a=0;
int newread=0;
long filesize=0;
char stm[20],mess2[ARR_SIZE+25],filename[FILE_NAME_LEN],name[NAME_LEN];
char other_user[ARR_SIZE],check[50],filename2[FILE_NAME_LEN];
struct stat fileinfo;
time_t tm;
FILE *fp;
FILE *fp2;

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

if (!strlen(inpstr)) 
  {
   write_str(user,"You have not specified a message"); 
   return;
  }

if (ustr[user].frog) {
   write_str(user,"Frogs can't write, silly.");
   return;
   }
  
if (!read_user(other_user)) 
  {
   write_str(user,NO_USER_STR);
   return;
  }

/**  Check for gagged user **/
sprintf(filename2,"%s/%s",GAGDIR,other_user);
if (!(fp2=fopen(filename2,"r"))) goto DONEE;
while(!feof(fp2)) {
        fscanf(fp2,"%s\n",check);
        strtolower(check);
        if (!strcmp(ustr[user].name,check)) {
                a=1; break;
                }
        check[0]=0;
        }
fclose(fp2);

if (a==1) { 
        write_str(user,IS_GAGGED);
        a=0;
        filename2[0]=0;
        check[0]=0;
        return;
        }
DONEE:
filename2[0]=0;
check[0]=0;
a=0;
  
/*--------------------------------------------------*/
/* prepare message to be sent                       */
/*--------------------------------------------------*/
time(&tm);
midcpy(ctime(&tm),stm,4,15);
strcpy(name,ustr[user].say_name);


sprintf(mess,"(%s) From %s: %s\n",stm,name,inpstr);

sprintf(t_mess,"%s/%s",MAILDIR,other_user);
strncpy(filename,t_mess,FILE_NAME_LEN);

/* Get filename size */
if (stat(filename, &fileinfo) == -1) {
    if (check_for_file(filename)) {
    sprintf(mess2,"SYSTEM: Could not read mailfile size for user %s\n",other_user);
    print_to_syslog(mess2);
    }
   }
else filesize = fileinfo.st_size;

/* THIS IS TO PREVENT EXCESSIVE MAIL SPAMMING */
/* If recepients mailsize is at or over size limit, tell sender the */
/* mail send failed, mail recepient from the talker that mailfile   */
/* is over limit, and notify user, if online.                       */
if (filesize >= MAX_MAILSIZE) {
   write_str(user,"User's mailfile is at or over the size limit. Send failed.");
 if (t_ustr.mail_warn == 0) {
   sprintf(mess,"(%s) From THE TALKER: %s\n",stm,MAILFILE_NOTIFY);
   if (!(fp=fopen(filename,"a")))
     {
      sprintf(mess,"%s : message cannot be written\n", syserror);
      write_str(user,mess);
      return;
     }
   fputs(mess,fp);
   FCLOSE(fp);
   t_ustr.mail_warn = 1;
   t_ustr.new_mail = TRUE;
   t_ustr.mail_num++;
/*---------------------------------------------------*/
/* write email message                               */
/*---------------------------------------------------*/

sprintf(t_mess,"%s/%s.email",MAILDIR,other_user);
strncpy(filename,t_mess,FILE_NAME_LEN);

if (!(fp=fopen(filename,"w"))) 
  {
   sprintf(mess,"%s : message cannot be written\n", syserror);
   write_str(user,mess);
   return;
  }
fputs(mess,fp);
fputs(EXT_MAIL1,fp);
sprintf(mess,EXT_MAIL2,SYSTEM_NAME);
fputs(mess,fp);
fputs(EXT_MAIL3,fp);
fputs(EXT_MAIL7,fp);
FCLOSE(fp);

/*-------------------------------------------------------*/
/* write users to inform them of transaction             */
/*-------------------------------------------------------*/

if ((u=get_user_num(other_user,user))!= -1) 
  {
   strcpy(mess,MAILFROM_TALKER);
   write_str(u,mess);
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
   }
   else ustr[u].mail_num++;

   if ((ustr[u].autof==1) && (ustr[u].automsgs < MAX_AUTOFORS)) {
       if (strlen(ustr[u].email_addr) < 8) { 
           write_str(u,"Your set email address is too short to be a valid address..aborting autofwd.");
           return;
           }
      sprintf(mess,"%s -s \'Mailfile error on %s\' %s < %s/%s.email 2> /dev/null",MAILPROG,SYSTEM_NAME,ustr[u].email_addr,MAILDIR,ustr[u].say_name); 
      system(mess);
      print_to_syslog("AUTOFWD MESSAGE FOLLOWS...\n");
      strcat(mess,"\n");
      print_to_syslog(mess);
      write_str(u,MAIL_AUTOFWD);
      ustr[u].automsgs++;
     }
  } /* end of if user online if */
 else if ((t_ustr.autof > 0) && (t_ustr.automsgs < MAX_AUTOFORS)) {
     if (strlen(t_ustr.email_addr) < 8) { return; }
   sprintf(mess,"%s -s \'Mailfile error on %s\' %s < %s/%s.email 2> /dev/null",MAILPROG,SYSTEM_NAME,t_ustr.email_addr,MAILDIR,t_ustr.say_name); 
   system(mess);
   print_to_syslog("AUTOFWD MESSAGE FOLLOWS...\n");
   strcat(mess,"\n");
   print_to_syslog(mess);
   t_ustr.automsgs++;
   }
 }
 else { }

 write_user(other_user);
/* If recepient is online and has autoread in dual mode, */
/* read their new message                                */

if (u && newread) read_mail(u,"1");

  return;
 } /* end of if over filesize */

/* End of mailfile size check */

filesize = 0;

/*---------------------------------------------------*/
/* write mail message                                */
/*---------------------------------------------------*/

if (!(fp=fopen(filename,"a"))) 
  {
   sprintf(mess,"%s : message cannot be written\n", syserror);
   write_str(user,mess);
   return;
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

sprintf(mess2,"(%s) To %s: %s\n",stm,other_user,inpstr);

sprintf(t_mess,"%s/%s.sent",MAILDIR,ustr[user].name);
strncpy(filename,t_mess,FILE_NAME_LEN);

/* Get filename size */
if (stat(filename, &fileinfo) == -1) {
    if (check_for_file(filename)) {
    sprintf(t_mess,"SYSTEM: Could not read mailfile size for user %s\n",ustr[user].say_name);
    print_to_syslog(t_mess);
    }
   }
else filesize = fileinfo.st_size;

if (filesize >= MAX_MAILSIZE) {
    write_str(user,MAILFILE2_NOTIFY);
    }
else {
if (!(fp=fopen(filename,"a"))) 
  {
   sprintf(mess2,"%s : message cannot be written\n", syserror);
   write_str(user,mess2);
   return;
  }
fputs(mess2,fp);
FCLOSE(fp);
}

/* End of mailfile size check */

filesize = 0;

/*---------------------------------------------------*/
/* write email message                               */
/*---------------------------------------------------*/

sprintf(t_mess,"%s/%s.email",MAILDIR,other_user);
strncpy(filename,t_mess,FILE_NAME_LEN);

if (!(fp=fopen(filename,"w"))) 
  {
   sprintf(mess,"%s : message cannot be written\n", syserror);
   write_str(user,mess);
   return;
  }

strcpy(mess, strip_color(mess));
fputs(mess,fp);
fputs(EXT_MAIL1,fp);
sprintf(mess,EXT_MAIL2,SYSTEM_NAME);
fputs(mess,fp);
fputs(EXT_MAIL3,fp);
if (!ustr[user].semail) {
   fputs(EXT_MAIL4,fp);
   fputs(EXT_MAIL5,fp);
    sprintf(mess,EXT_MAIL6,ustr[user].email_addr);
   fputs(mess,fp);
   }
fputs(EXT_MAIL7,fp);
FCLOSE(fp);

/*-------------------------------------------------------*/
/* write users to inform them of transaction             */
/*-------------------------------------------------------*/

sprintf(mess,MAIL_TO,other_user);
write_str(user,mess);
if ((u=get_user_num_exact(other_user,user))!= -1) 
  {
   sprintf(mess,MAILFROM_USER,ustr[user].say_name);
   write_str(u,mess);
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
       if (strlen(ustr[u].email_addr) < 8) { 
           write_str(u,"Your set email address is too short to be a valid address..aborting autofwd.");
           return;
           }
      sprintf(mess,"%s -s \'New mail from %s on %s\' %s < %s/%s.email 2> /dev/null",MAILPROG,ustr[user].say_name,SYSTEM_NAME,ustr[u].email_addr,MAILDIR,ustr[u].name); 
      system(mess);
      print_to_syslog("AUTOFWD MESSAGE FOLLOWS...\n");
      print_to_syslog(mess);
      print_to_syslog("\n");
      write_str(u,MAIL_AUTOFWD);
      ustr[u].automsgs++;
     }
  } /* end of if user online if */
 else if ((t_ustr.autof > 0) && (t_ustr.automsgs < MAX_AUTOFORS)) {
     if (strlen(t_ustr.email_addr) < 8) { return; }
   sprintf(mess,"%s -s \'New mail from %s on %s\' %s < %s/%s.email 2> /dev/null",MAILPROG,ustr[user].say_name,SYSTEM_NAME,t_ustr.email_addr,MAILDIR,t_ustr.name); 
   system(mess);
   print_to_syslog("AUTOFWD MESSAGE FOLLOWS...\n");
   print_to_syslog(mess);
   print_to_syslog("\n");
   t_ustr.automsgs++;
   }
write_user(other_user);

/* If recepient is online and has autoread in dual mode, */
/* read their new message                                */

if (u && newread) read_mail(u,"1");

}
