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

extern int checked;
extern int dobackups;
extern int NUM_AREAS;
extern int MESS_LIFE;
extern char mess[ARR_SIZE+25];
extern char t_mess[ARR_SIZE+25];
extern char bt_conv[NUM_LINES][MAX_LINE_LEN+1]; /* stores lines of conversation in wiztell buffer */
extern int bt_count;           /* wiztell count in the buffer                   */
extern int down_time;
extern int treboot;
extern int says;
extern int says_running;
extern int tells;
extern int tells_running;
extern int commands;
extern int commands_running;
extern unsigned int mailgateway_port;

/*** check to see if messages are out of date ***/
void check_mess(int startup)
{  
int b,day,day2;
int normcount=0;
int wizcount=0;
char line[ARR_SIZE+31],datestr[30],timestr[7],daystr[11],daystr2[3];
char boardfile[FILE_NAME_LEN],tempfile[FILE_NAME_LEN];
time_t tm;
FILE *bfp,*tfp;

timestr[0]=0;

time(&tm);
strcpy(datestr,ctime(&tm));
midcpy(datestr,timestr,11,15);
midcpy(datestr,daystr,8,9);
day=atoi(daystr);   
    
/* see if its time to check (midnight) */
if (startup==1) goto SKIP;
else if (startup==2) goto WLOG;

if (checked)
  {
   checked = 0;
   return;
  }
else if ((!strcmp(timestr,"00:01") || !strcmp(timestr,"00:00")) && !checked)
  {
   checked = 1;
  }
else return; 

SKIP:
if (!startup) {  
        write_area(-1,"");
	write_area(-1,"SYSTEM: Midnight system check taking place, please wait...");
	write_log(SYSTEMLOG,YESTIME,"SYSTEM: Midnight system check in progress..\n"); 
	}
    
/* cycle through files */
            
strcpy(tempfile,get_temp_file());

for(b=0;b<NUM_AREAS+1;++b) {
        if (b == NUM_AREAS)
          sprintf(boardfile,"%s/wizmess",MESSDIR);
         else
          sprintf(boardfile,"%s/board%d",MESSDIR,b);

        if (!(bfp=fopen(boardfile,"r"))) continue;
        if (!(tfp=fopen(tempfile,"w"))) {
               if (startup==1) {
                write_log(ERRLOG,YESTIME,"Couldn't open tempfile(w) %s in check_mess! %s\n",tempfile,get_error());
                perror("\nSYSTEM: Cant open tempfile(w) in check_mess");
                FCLOSE(bfp);
#if defined(WINDOWS)
WSACleanup();
#endif
                exit(0);
                }
                else {
                write_log(ERRLOG,YESTIME,"Couldn't open tempfile(w) %s in check_mess! %s\n",tempfile,get_error());
                FCLOSE(bfp);
                checked = 0;
                return;
                }
           }
        
        /* go through board and write valid messages to temp file */
        fgets(line,ARR_SIZE+30,bfp);
        while(!feof(bfp)) {
                midcpy(line,daystr2,5,6);
                day2=atoi(daystr2);
                if (day2>day) day2 -= 30;  /* if mess from prev. month */
                if (day2>=day-MESS_LIFE)
                 fputs(line,tfp);
                else {
                 if (b == NUM_AREAS) wizcount++;
                 else normcount++;
                 
                 astr[b].mess_num--;
                 }
                fgets(line,1050,bfp);
                }   
        FCLOSE(bfp);
        FCLOSE(tfp);
        remove(boardfile);
        
        /* rename temp file back to board file */
        if (rename(tempfile,boardfile) == -1)
         astr[b].mess_num=0;
        
        remove(tempfile);

        /* If changed boards are left with no messages, remove them */
        if (!file_count_lines(boardfile)) remove(boardfile);
                
        } /* end of for */
                
  /* Reset user auto-forward limits to nill */
  if (startup==1)
    printf("Resetting auto-forward limits and checking user abbreviations..\n");
   reset_userfors(startup);
                 
  /* Remove all temp files from the junk directory */
  if (startup==1)
    printf("Removing temp files..\n");
   remove_junk(startup);
                 
  if (!startup) {   
  /* Auto expire users if set */
  auto_expire();
  /* Expire email verifications from today */
  check_verify(-1,1);
  /* We'll do daily backups here */
  if (dobackups) backup_logs(-1);
  /* We'll do cleaning of old backups here */
  trim_backups(-1,TRIM_BACKUPS);
  }     
        
WLOG:
  /* Append the day's user activity to the activity file */
  if ((startup==2) || (!startup)) {
    if (checked) write_meter(1);
    else write_meter(0);
   }
    
  /* Reset logins per hour numbers */
  b=0;           
  for (b=0;b<24;++b) {
     logstat[b].logins = 0;
     }
   
        system_stats.logins_today       = 0;
        system_stats.new_users_today    = 0;
  
if (!startup) { 
        write_area(-1,"");
        write_area(-1,"SYSTEM: Check completed. Carry on!");
        write_log(SYSTEMLOG,YESTIME,"SYSTEM: Midnight check completed.\n");
	write_log(SYSTEMLOG,YESTIME,"SYSTEM: %d old messages deleted from boards\n",normcount);
	write_log(SYSTEMLOG,YESTIME,"SYSTEM: %d old messages deleted from wiz board\n",wizcount);
   }    
if (startup==1) {
	write_log(SYSTEMLOG,YESTIME,"SYSTEM: %d old messages deleted from boards\n",normcount);
        write_log(SYSTEMLOG,YESTIME,"SYSTEM: %d old messages deleted from wiz board\n",wizcount);
   }

return;
} 


/* Count number of users in our user storage directory */
void check_total_users(int mode)
{
char small_buffer[FILE_NAME_LEN];
char filerid[FILE_NAME_LEN];
char z_mess[ARR_SIZE];
struct dirent *dp;
DIR *dirp;

 sprintf(z_mess,"%s",USERDIR);
 strncpy(filerid,z_mess,FILE_NAME_LEN);

 dirp=opendir((char *)filerid);

 if (dirp == NULL)
   {
    if (mode==1) {
      write_log(ERRLOG,YESTIME,"TOTUSERS: Can't open directory \"%s\" for check_total_users! %s\n",filerid,get_error());
      sprintf(z_mess,"\nSYSTEM: Can't open directory \"%s\" for check_total_users! %s",filerid,get_error());
      perror(z_mess);
#if defined(WINDOWS)
WSACleanup();
#endif
      exit(0); 
      }
    else {
      write_log(ERRLOG,YESTIME,"TOTUSERS: Can't open directory \"%s\" for check_total_users! %s\n",filerid,get_error());
      return;
     }
   }

system_stats.tot_users = 0;

 while ((dp = readdir(dirp)) != NULL)
   {

    sprintf(small_buffer,"%s",dp->d_name);
        if (small_buffer[0] == '.')
         continue;
        else
         system_stats.tot_users++;
    
   }
      
 (void) closedir(dirp);
}

/*--------------------------------------*/
/* Now we check who we have to alert    */
/* of our login.                        */
/*--------------------------------------*/
void check_alert(int user, int mode)
{
int u=0;
int i=0;
char check[50];

for (u=0;u<MAX_USERS;++u) {

          if (ustr[u].area!= -1 && u !=user) {
 
                        for (i=0; i<MAX_ALERT; ++i) {
                        strcpy(check,ustr[u].friends[i]);
                        strtolower(check);
                        if (!strcmp(ustr[user].name,check))
                           {
                            if (mode==0) {
                              if (!user_wants_message(u,BEEPS))
                               sprintf(mess,"^HR.ALERT!^ %s has logged on.",ustr[user].say_name);
                              else
                               sprintf(mess,"^HR.ALERT!^ %s has logged on.\07\07",ustr[user].say_name);
                              write_str(u,mess);
                           break;
                           } /* end of if mode 0 */
			else if ((mode==1) && (ustr[u].area!=ustr[user].area)) {
                              if (!user_wants_message(u,BEEPS))
				sprintf(mess,"^HM.ALERT!^ %s has come back from being ^HYAFK^",ustr[user].say_name);
                              else
				sprintf(mess,"^HM.ALERT!^ %s has come back from being ^HYAFK^\07\07",ustr[user].say_name);
                              write_str(u,mess);
                           break;
                           } /* end of if mode 1 */
                         } /* end of strcmp if */
                        } /* end of sub-for */
                  i=0; continue;  
              }    /* end of if */
   }   /* end of for */
 
}


/*------------------------*/
/* Check for gagged user  */
/*------------------------*/
int check_gag(int user, int user2, int mode)
{
int b=0;
char check[50];

      for (b=0; b<MAX_GAG; ++b) {
        strcpy(check,ustr[user2].gagged[b]);
        strtolower(check);
        if (!strcmp(ustr[user].name,check)) {
           if (!mode)
            write_str(user,IS_GAGGED);

           check[0]=0;
           return 0;
          }
        check[0]=0;
        }

return 1;  
}


/*----------------------------------------*/
/* Check for gagged user that isnt online */
/*----------------------------------------*/
int check_gag2(int user, char *name)
{
int b=0;
char check[50];

if (!read_user(name)) return 2;

      for (b=0; b<MAX_GAG; ++b) {
        strcpy(check,t_ustr.gagged[b]);
        strtolower(check);
        if (!strcmp(ustr[user].name,check)) {
           write_str(user,IS_GAGGED);
           check[0]=0;
           return 0;   
          }
        check[0]=0;
        }
         
return 1;
}


/*
 * Check for new user mail 
 */
void check_mail(int user) 
{
struct stat stbuf;
char filename[FILE_NAME_LEN], datestr[24];
    
sprintf(t_mess,"%s/%s",MAILDIR,ustr[user].name);
strncpy(filename,t_mess,FILE_NAME_LEN);
if (stat(filename, &stbuf) == -1)
  {
   return;             
  }

if (ustr[user].new_mail)
  {
   write_str(user," ");
      sprintf(t_mess,MAIL_NEW,ustr[user].mail_num,ustr[user].mail_num == 1 ? "" : "s");
   write_str(user,t_mess);
   write_str(user," ");
  }
   
strcpy(datestr,ctime(&stbuf.st_mtime));
   
sprintf(mess,MAIL_ACCESS,datestr);
write_str(user,mess);

if (ustr[user].new_mail &&
    ((ustr[user].autor==1) || (ustr[user].autor==3)) ) {
   sprintf(mess,"%i",ustr[user].mail_num);
   read_mail(user,mess);
   }

}


/*
 * Check for auto_shutdown and auto_reboot
 */
void check_shut(void)
{
int offset=0;
int c=0;

if (down_time > 1) offset=down_time-1;
        
if ((down_time >= 61) && (down_time < 1441)) {
for (c=1;c<24;++c) {
 if (down_time == (60*c+1)) {
   if (treboot)
    sprintf(mess,"SYSTEM:: Talker %sreboot in  %i  hou%s",treboot==1?"soft-":"hard-",c,c==1?"r":"rs");
   else
    sprintf(mess,"SYSTEM:: Talker auto-shutdown in  %i  hou%s",c,c==1?"r":"rs");

   write_area(-1,"");
   write_area(-1,mess);
   break;
   }   
 }  /* end of for */
c=0;
}  /* end of if */

else if ((down_time >= 1441) && (down_time <= 32767)) {
for (c=1;c<23;++c) {
 if (down_time == (1440*c+1)) {
   if (treboot)
    sprintf(mess,"SYSTEM:: Talker %sreboot in  %i  da%s",treboot==1?"soft-":"hard-",c,c==1?"y":"ys");
   else
    sprintf(mess,"SYSTEM:: Talker auto-shutdown in  %i  da%s",c,c==1?"y":"ys");

   write_area(-1,"");
   write_area(-1,mess);
   break;
   }
 }  /* end of for */
c=0;
}  /* end of if */

if (down_time==51 || down_time==41 || down_time==31 || down_time==21 || down_time==11 || 
    down_time==6 || down_time==2) {
if (treboot)
 sprintf(mess,"SYSTEM:: Talker %sreboot in  %i  minut%s",treboot==1?"soft-":"hard-",offset,offset==1?"e":"es");
else
 sprintf(mess,"SYSTEM:: Talker auto-shutdown in  %i  minut%s",offset,offset==1?"e":"es");

write_area(-1,"");
write_area(-1,mess);
}

if (down_time==1) shutdown_auto();
down_time = offset;
}


/*
 * See if any users are near or at idle limit or need flags reset
 */
void check_idle(void)
{
int min,user;
int new_pos = 0;

for (user=0; user<MAX_USERS; ++user) 
  {
   if (ustr[user].logging_in) 
     {
       min=(int)((time(0) - ustr[user].last_input)/60);
       if (min >= LOGIN_TIMEOUT)
         {
          if (user_wants_message(user,BEEPS))
           write_str(user,"\07");
          write_str(user,"Connection closed due to exceeeded login time limit");
          user_quit(user,1);
         }
     }
  }


for (user=0;user<MAX_USERS;++user) 
  {
    if (ustr[user].suspended && (ustr[user].xco_time==1)) 
     {
      ustr[user].suspended=0;
      ustr[user].xco_time=0;
      write_str(user,XCOMMOFF_MESS);
      sprintf(mess,"%s XCOM OFF: %s, by the talker.",STAFF_PREFIX,strip_color(ustr[user].say_name));
      writeall_str(mess, WIZ_ONLY, user, 0, user, BOLD, WIZT, 0);
      strncpy(bt_conv[bt_count],mess,MAX_LINE_LEN);
      new_pos = ( ++bt_count ) % NUM_LINES;
      bt_count = new_pos;
    }
   else if (ustr[user].suspended && (ustr[user].xco_time > 1)) {
      ustr[user].xco_time--;
      }

    if (ustr[user].frog && (ustr[user].frog_time==1)) 
     {
      ustr[user].frog=0;
      ustr[user].frog_time=0;
      write_str(user,FROGOFF_MESS);
      sprintf(mess,"%s FROG OFF: %s, by the talker.",STAFF_PREFIX,strip_color(ustr[user].say_name));
      writeall_str(mess, WIZ_ONLY, user, 0, user, BOLD, WIZT, 0);
      strncpy(bt_conv[bt_count],mess,MAX_LINE_LEN);
      new_pos = ( ++bt_count ) % NUM_LINES;
      bt_count = new_pos;
    }
   else if (ustr[user].frog && (ustr[user].frog_time > 1)) {
      ustr[user].frog_time--;
      }

    if (ustr[user].anchor && (ustr[user].anchor_time==1)) 
     {
      ustr[user].anchor=0;
      ustr[user].anchor_time=0;
      write_str(user,ANCHOROFF_MESS);
      sprintf(mess,"%s ANCHOR OFF: %s, by the talker.",STAFF_PREFIX,strip_color(ustr[user].say_name));
      writeall_str(mess, WIZ_ONLY, user, 0, user, BOLD, WIZT, 0);
      strncpy(bt_conv[bt_count],mess,MAX_LINE_LEN);
      new_pos = ( ++bt_count ) % NUM_LINES;
      bt_count = new_pos;
    }
   else if (ustr[user].anchor && (ustr[user].anchor_time > 1)) {
      ustr[user].anchor_time--;
      }

    if (ustr[user].gagcomm && (ustr[user].gag_time==1)) 
     {
      ustr[user].gagcomm=0;
      ustr[user].gag_time=0;
      write_str(user,GCOMMOFF_MESS);
      sprintf(mess,"%s GCOM OFF: %s, by the talker.",STAFF_PREFIX,strip_color(ustr[user].say_name));
      writeall_str(mess, WIZ_ONLY, user, 0, user, BOLD, WIZT, 0);
      strncpy(bt_conv[bt_count],mess,MAX_LINE_LEN);
      new_pos = ( ++bt_count ) % NUM_LINES;
      bt_count = new_pos;
    }
   else if (ustr[user].gagcomm && (ustr[user].gag_time > 1)) {
      ustr[user].gag_time--;
      }

   if (!ustr[user].shout && (ustr[user].muz_time==1))
     {
       ustr[user].shout=1;
       ustr[user].muz_time=0;
       sprintf(mess,"%s can shout again",ustr[user].say_name);
       writeall_str(mess, 1, user, 1, user, NORM, NONE, 0);
       write_str(user,MUZZLEOFF_MESS);
       sprintf(mess,"%s UNMUZZLE: %s, by the talker.",STAFF_PREFIX,strip_color(ustr[user].say_name));
       writeall_str(mess, WIZ_ONLY, user, 0, user, BOLD, WIZT, 0);
       strncpy(bt_conv[bt_count],mess,MAX_LINE_LEN);
       new_pos = ( ++bt_count ) % NUM_LINES;
       bt_count = new_pos;
     }
   else if (!ustr[user].shout && (ustr[user].muz_time > 1)) {
      ustr[user].muz_time--;
      }

   if (ustr[user].super >= IDLE_LEVEL) continue;
   
   if (!strcmp(ustr[user].name,BOT_ID)) continue;

   if ((ustr[user].area == -1 && !ustr[user].logging_in)) continue; 
   
   min=(int)((time(0) - ustr[user].last_input)/60);
   
   if ( ( min >= (IDLE_TIME - 2)) && !ustr[user].warning_given) 
     {
      if (user_wants_message(user,BEEPS))
       write_str(user,"\07*** Warning - input within 2 minutes or you will be disconnected ***");
      else
       write_str(user,"*** Warning - input within 2 minutes or you will be disconnected ***");
      ustr[user].warning_given=1;
      continue;
     }
     
   if (min >= IDLE_TIME ) 
    {
     write_str(user,IDLE_BYE_MESS);
     user_quit(user,1);
    }
  } /* end of user for */
   
says_running     = (says     + says_running)     / 2;
tells_running    = (tells    + tells_running)    / 2;
commands_running = (commands + commands_running) / 2;

tells = 0;
commands = 1;
says = 0;

if (fight.issued)
  {
    min = (int)((time(0)-fight.time)/60);
    if (min > IDLE_TIME)
      reset_chal(0," ");

  }
  
}	


/*
 * Zero out user input bytes for flood protection
 */
void check_flood_limits(void)
{
int user;

 if (FLOOD_INPUT_LIMIT > 0) {
	for (user=0; user<MAX_USERS; ++user) {
		if (ustr[user].bytes_read != 0)
			ustr[user].bytes_read = 0;
	} /* user for */
 } /* FLOOD_INPUT_LIMIT if */

 return;
}


/*
 * Check miscellaneous connections for needed actions
 */
void check_misc_connects(void) {
int user=0;

for (user=0;user<MAX_MISC_CONNECTS;++user) 
  {
   if (miscconn[user].type==1) {
    if ( miscconn[user].time && ((time(0) - miscconn[user].time) >= 45) ) {
    /* kill the remote who */
    if (ustr[miscconn[user].user].sock != -1) write_str(miscconn[user].user,"^HRRemote who finished. Connection timed out.^");
    free_sock(user,'5');
    }
    else if ( (miscconn[user].time) && (ustr[miscconn[user].user].sock != -1)) write_str(miscconn[user].user,"^HRRemote who still in progress..^");
   } /* type == 1 */
   else if (miscconn[user].type==2) {
    if ( miscconn[user].time && ((time(0) - miscconn[user].time) >= 45) ) {
    /* kill the SMTP connection */
	write_log(SYSTEMLOG,YESTIME,"SMTP: sck#%d:slt#%d: Closing connection due to timeout\n",
	miscconn[user].sock,user);
    if (miscconn[user].stage >= 2) {
	FCLOSE(miscconn[user].fd);
	requeue_smtp(user);
    }
    free_sock(user,'5');
    }
   } /* type == 2 */

  } /* for */

 return;
}


/* see if we have any queued mail, to see if we */
/* need to start up any SMTP connections        */
void check_smtp(void) {
int user=0;
int free_connections=0;
int need_connections=0;
char small_buff[64];
DIR *dirp;
struct dirent *dp;

if (!mailgateway_port) return;

 dirp=opendir((char *)MAILDIR_SMTP_QUEUE);
 
 if (dirp == NULL)
   {
    write_log(ERRLOG,YESTIME,"Cannot open directory %s for reading in check_smtp! %s\n",MAILDIR_SMTP_QUEUE,get_error());
    /* we'll try to make it */
    if (errno==ENOENT) {
#if defined(HAVE_MKDIR_ARGS)
	mkdir(MAILDIR_SMTP_QUEUE,0700);
#else
	mkdir(MAILDIR_SMTP_QUEUE);
#endif
	dirp=opendir((char *)MAILDIR_SMTP_QUEUE);
	if (dirp == NULL) return;
    }
    else return;
   }

 while ((dp = readdir(dirp)) != NULL)
   {   
    sprintf(small_buff,"%s",dp->d_name);
    if (small_buff[0]=='.') continue;
    need_connections++;
    if (need_connections==MAX_MISC_CONNECTS) break;
   }
   (void)closedir(dirp);

#if defined(SMTP_DEBUG)
 write_log(SYSTEMLOG,YESTIME,"check_smtp closed dir: need %d connections\n",need_connections);
#endif

 for (user=0;user<MAX_MISC_CONNECTS;++user) {
/*
 write_log(SYSTEMLOG,YESTIME,"%d: %d: %d: %d: %ld: %s: %s\n",
 user,miscconn[user].sock,miscconn[user].type,miscconn[user].stage,
 miscconn[user].time,miscconn[user].site,miscconn[user].queuename);
*/
 if (miscconn[user].sock==-1) free_connections++;
 }

#if defined(SMTP_DEBUG)
 write_log(SYSTEMLOG,YESTIME,"check_smtp check free: %d connections free for use\n",free_connections);
#endif

 while (free_connections && need_connections) {
#if defined(SMTP_DEBUG)
 write_log(SYSTEMLOG,YESTIME,"check_smtp doing SMTP connect: need %d free %d\n",need_connections,free_connections);
#endif
 if (do_smtp_connect() != -1) free_connections--;
 need_connections--;
 }

 return;
}


char *check_var(char *line, char *MACRO, char *Replacement) {
int index1;
int tempPointer;
char *pointer1;
char temparray[514];
char tempspace[514];
char linetemp[514];
        
        temparray[0]=0;
        tempspace[0]=0;
        linetemp[0]=0;
        
   while (1) {
        /* find string in line */
        pointer1 = (char *)(strstr(line, MACRO));
        /* if not found, exit */
        if (pointer1 == NULL) break;
        /* find at what position the result starts */
        index1 = pointer1 - line;
        /* copy the original line to a normal char */
        /* so midcpy doesn't mangle it all up      */
        strcpy(linetemp,line);
        /* copy up to the result to our output */
        midcpy(linetemp, temparray, 0, index1-1);
        /* append the replacement for it to our output */
        strcat(temparray, Replacement);
        /* ok, where's the rest of our string */ 
        tempPointer = index1+strlen(MACRO);
        /* copy the rest to a temp spot */
        midcpy(linetemp, tempspace, tempPointer, strlen(linetemp));
        /* cat the rest to our output */
        strcat(temparray, tempspace);
        /* make the original line equal to the output for our loop */
        
        line = temparray;
       }
        
        return line;
}

