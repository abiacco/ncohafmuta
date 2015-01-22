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

extern char mess[ARR_SIZE+25];    /* functions use mess to send output   */
extern int num_of_users;     /* total number of users online           */
extern int allow_new;          /* can new users be created?              */
extern int bot;           /* this will hold the bots user number   */
extern int autopromote; /* allowing users to promote themselves? */


/*------------------------------------------------------------------*/
/* This function is called after the MOTD is shown.  It allows      */
/* the connector to login to the system as an old user or create a  */
/* new user.                                                        */
/* logging_in = 1   User at password confirmation for new user	    */
/* logging_in = 2   User at first password prompt		    */
/* logging_in = 3   User at username prompt			    */
/* logging_in = 5   User at email verification prompt (enter addy)  */
/* logging_in = 10  User at password prompt, back from emailver	    */
/* logging_in = 11  User at info press-return prompt (normal)	    */
/* logging_in = 12  User at info press-return prompt (new user)	    */
/*------------------------------------------------------------------*/
void my_login(int user, char *inpstr)
{
  char         name[ARR_SIZE];
  char         passwd[ARR_SIZE];
  char         email[ARR_SIZE];
  char         lowemail[71];
  char         email_pass[NAME_LEN];
  char         tempname[NAME_LEN+1];
  char         filename[FILE_NAME_LEN];
  int          f=0;
  int          su;
  time_t       tm;
  time_t       tm_then;

  passwd[0]=0;  
  email_pass[0]=0;
  lowemail[0]=0;

  /*----------------------------------------------------------*/
  /* if this is the second time the password has been entered */
  /*----------------------------------------------------------*/
  
  if (ustr[user].logging_in==1)  
    goto CHECK_PASS;

  /*------------------------------------------------------------*/
  /* If we're getting an email address for account verification */
  /*------------------------------------------------------------*/
  if (ustr[user].logging_in==5) {
    email[0]=0;
    sscanf(inpstr,"%s",email);
    email[70]=0;
    strcpy(lowemail,email);
    strtolower(lowemail);

    if (!strcmp(lowemail,"quit")) {
      ustr[user].logging_in=3;
      return;
     }
    if ((!strstr(lowemail,"@") && !strstr(lowemail,".")) || strpbrk(lowemail,";/[]\\") ||
        strstr(lowemail,"whitehouse.gov")) {
      write_str(user,"Invalid email address.");
      write_str_nr(user,EMAIL_VERIFY);
	telnet_write_eor(user);
      return;
      }
    /* Generate password */
    strcpy(ustr[user].login_pass,generate_password());
    strcpy(ustr[user].say_name,ustr[user].login_name);

    /* Email user with password */
    if (mail_verify(user,email) == -1) {
      write_str(user,"Mail message could not be sent. Try later.");
      user_quit(user,1);
      return;
      }

    /* Encrypt password writing time, user, and password to VERIFILE */
    st_crypt(ustr[user].login_pass);
    write_verifile(user);

    /* Tell them their username and a password was mailed to them */
    write_str(user,"");
    sprintf(mess,"Your username and password has been emailed to %s",email);
    write_str(user,mess);
    write_str(user,"");
    write_str(user,"Come back when you have received it to activate your account");
    write_str(user,"Thank you for stopping by!");

    user_quit(user,1);
    return;
    }

  /*------------------------------------------------------------*/
  /* If we're getting a password for account email verification */
  /*------------------------------------------------------------*/
  if (ustr[user].logging_in==10) {

     telnet_echo_on(user);
     sscanf(inpstr,"%s",passwd);
     strtolower(passwd);
     st_crypt(passwd);                                    
      
     if (strcmp(ustr[user].login_pass,passwd)) 
       {
         write_str(user,NON_VERIFY);
         ustr[user].login_pass[0]=0;
         user_quit(user,1);
         return;
       }

    delete_verify(user);
    goto EMAIL2_PASS;
   }

  /* If user is coming from a login info prompt */
if (ustr[user].pause_login==1) {
  if (ustr[user].logging_in==11) {
     add_user(user);
     add_user2(user,0);
     return;
    }

  if (ustr[user].logging_in==12) {
     add_user(user);
     add_user2(user,1);
     return;
    }
}

  /*-------------------------------------*/
  /* get login name                      */
  /*-------------------------------------*/

  if (ustr[user].logging_in==3) 
    {
      name[0]=0;
      sscanf(inpstr,"%s",name);
	strtolower(name);
	reset_user_struct(user,0);
        if (!strcmp(name,"quit")) {
                write_str(user,IF_QUIT_LOGIN);
                user_quit(user,1);  return;
                }      
        if (!strcmp(name,"help")) {
		/* write out login help file to user */
	        sprintf(filename,"%s",LOGINFILE); 
		cat(filename,user,0);

		telnet_echo_on(user);
                write_str_nr(user,SYS_LOGIN);
		telnet_write_eor(user);
                return;
	}
        if (!strcmp(name,"who")) {
             if (LONGLOGIN_WHO==2) {
                t_who(user,"",0);
                }
             else if (LONGLOGIN_WHO==1){
                write_str(user,"");
                newwho(user);
                write_str(user,"");
               }
	     else {
                write_str(user,"");
                write_str(user,"Who-ing from the login prompt is disabled.");
                write_str(user,"");
	       }
		telnet_echo_on(user);
                write_str_nr(user,SYS_LOGIN);
		telnet_write_eor(user);
                return;
                }      
      if (name[0]<32 || !strlen(name)) 
        {
	 write_str_nr(user,SYS_LOGIN);
	 telnet_write_eor(user);
	 return;
	}
	
      if (strlen(name)<3) 
        {
        write_str(user,SYS_NAME_SHORT);
        write_str(user," ");
        write_str(user," Note: Your telnet client might not support line mode of operation.");
        write_str(user,"       If this is true, you might not be able to use this service.");
        write_str(user," ");
        attempts(user);
        return;
	}
	
     if (strlen(name)>NAME_LEN-1) 
       {
	write_str(user,SYS_NAME_LONG);
	attempts(user);  
	return;
       }
	
	/* see if only letters in login */
     for (f=0; f<strlen(name); ++f) 
       {
         if (!isalpha((int)name[f]) || name[f]<'A' || name[f] >'z') 
           {
	     write_str(user,ONLY_LET);
	     attempts(user);  
	     return;
	   }
       }

     /* Check to see if name is not allowed */
     strcpy(tempname,name);
     if (check_nban(tempname,ustr[user].site) == 1) {
       write_str(user,IS_BANNED);
       attempts(user);
       return;
       }


     strcpy(ustr[user].login_name,tempname);

     /* Check to see if someone already has this name		*/
     /* We don't want to check if they're a new user, but	*/
     /* more specifically if they're a new user with a name	*/
     /* someone else is trying to create at the same time	*/
     if (!check_for_user(ustr[user].login_name)) {
	f=check_for_creation(ustr[user].login_name);
	if (f != -1) {
		write_str(user,NO_CREATION);
		write_log(SYSTEMLOG,YESTIME,"CREATE: User from %s:%s tried to create user %s\n",ustr[user].site,ustr[user].net_name,ustr[user].login_name);
		write_log(SYSTEMLOG,YESTIME,"CREATE: but already in process of creation by %s:%s\n",ustr[f].site,ustr[f].net_name);
		attempts(user);
		return;
		}
     }

     /* Check to see if this user is returning with a password   */
     /* we emailed him. If so, check their site against new-bans */
     /* If they pass ask them for the password                   */
   if (allow_new==1) {
     if (check_verify(user,0) == 1) {
	     if (check_restriction(user, NEW, ANY) == 1)
		{
		write_log(BANLOG,YESTIME,"MAIN: Creation attempt by %s, back from emailver, BANNEWed site %s:%s:sck#%d:slt#%d\n",ustr[user].login_name,ustr[user].site,ustr[user].net_name,ustr[user].sock,user);
		attempts(user);
		return;
		}
        sprintf(filename,"%s",VERINSTRUCT2);
        /* Print out the welcome back and password instructions file */
        /* to the user */
     cat(filename,user,0);
     
     write_str_nr(user,PASEMAIL_VERIFY); 
	telnet_write_eor(user);
     ustr[user].logging_in=10;
     return;
     } /* end of check_verify */

        /* First check if new users are banned from this site */
        if (check_restriction(user, NEW, ANY) == 1)
         {
	  write_log(BANLOG,YESTIME,"MAIN: Creation attempt by %s, BANNEWed site %s:%s:sck#%d:slt#%d\n",ustr[user].login_name,ustr[user].site,ustr[user].net_name,ustr[user].sock,user);
          attempts(user);
          return;
         }
       /* See if user already exists */
       if (!check_for_user(ustr[user].login_name)) {
        sprintf(filename,"%s",VERIINSTRUCT); 
        /* Print out the email verification instructions file */
        /* to the user */
        cat(filename,user,0);
     
        write_str_nr(user,EMAIL_VERIFY); 
	telnet_write_eor(user);
        ustr[user].logging_in=5;
        return;
        } /* end of check_for_user */
   } /* end of allow_new */

     ustr[user].logging_in=2;
     strtolower(ustr[user].login_name);
 
   /* See if we need to hide the password */    
   t_ustr.passhid = 0;
   read_user(ustr[user].login_name);
   ustr[user].passhid = t_ustr.passhid;  

     write_str_nr(user,SYS_PASSWD_PROMPT);
	telnet_echo_off(user);  
	telnet_write_eor(user);
     return;
   }

  /*-------------------------------------*/
  /* get first password                  */
  /*-------------------------------------*/

  if (ustr[user].logging_in==2) 
    {
      passwd[0]=0;
      telnet_echo_on(user);
      sscanf(inpstr,"%s",passwd);
      
      if (passwd[0]<32 || !strlen(passwd) || (strlen(passwd) < 3))
        {
	if (ustr[user].promptseq==0 && ustr[user].passhid==1) write_str(user,"");
          write_str(user,SYS_PASSWD_INVA);
	  write_str_nr(user,SYS_PASSWD_PROMPT);
	  telnet_echo_off(user);
	  telnet_write_eor(user);
	  return;
        }
        
      if (strlen(passwd)>NAME_LEN-1) 
        {
	if (ustr[user].promptseq==0 && ustr[user].passhid==1) write_str(user,"");
	  write_str(user,SYS_PASSWD_LONG);
	  write_str_nr(user,SYS_PASSWD_PROMPT);  
          telnet_echo_off(user);
	  telnet_write_eor(user);
	  return;
	}
     }

  /*-------------------------------------------------------------*/
  /* convert name & passwd to lowercase and encrypt the password */
  /*-------------------------------------------------------------*/
  
  strtolower(ustr[user].login_name);
  strtolower(passwd);
   
  if (strcmp(ustr[user].login_name, passwd) ==0)
    {
	if (ustr[user].promptseq==0 && ustr[user].passhid==1) write_str(user,"");
       write_str(user,PASS_NO_NAME1);         
       write_str(user,PASS_NO_NAME2);         
       attempts(user);                                        
       return;                                                
    }
    
  st_crypt(passwd);                                  
  strcpy(ustr[user].login_pass,passwd);              


  /*-------------------------------------------------------------------------*/
  /* check for user and login info                                           */
  /*-------------------------------------------------------------------------*/

  if (read_to_user(ustr[user].login_name,user) )
    {   
                                                               
     /*---------------------------------------------*/         
     /* The file exists, so the user has an account */        
     /*---------------------------------------------*/        
     su = t_ustr.super;                                       

     if ( strcmp(ustr[user].login_pass,ustr[user].password) )        
       {
	if (ustr[user].promptseq==0 && ustr[user].passhid==1) write_str(user,"");
        write_str(user,PASS_NOT_RIGHT);
        write_log(LOGINLOG,YESTIME,"wrong passwd from %s %s:%s:sck#%d:slt#%d\n", ustr[user].say_name, ustr[user].site, ustr[user].net_name, ustr[user].sock, user);
        attempts(user);
        return;
       }
      else
       {
     telnet_echo_on(user);
     write_str(user,"");

/* For ROOT_ID */
if (!strcmp(ustr[user].login_name,ROOT_ID))
  {
    ustr[user].super = MAX_LEVEL;
    ustr[user].promote = 1;

    for(f=0;f<MAX_AREAS;f++)
     {
      ustr[user].security[f]='Y';   
     }
  }

if (ustr[user].super < MIN_HIDE_LEVEL)
  {
    ustr[user].vis=1;
  }

write_str(user,"+---------------------------------------------------------------------------+");
if (ustr[user].vis)
  {write_str_nr(user,"Status [^HYVisible,^ ");}
  else
  {write_str_nr(user,"Status [^HRInVisible,^ ");}
  
if (ustr[user].shout)
  {write_str_nr(user,"^HYUnmuzzled,^ ");}
  else
  {write_str_nr(user,"^HRMuzzled,^ ");}

if (ustr[user].suspended)
  {write_str_nr(user,"^HRXcommed,^ ");}
  else
  {write_str_nr(user,"^HYUNxcommed,^ ");}

if (ustr[user].frog)
  {write_str_nr(user,"^HRFrogged,^ ");}
  else
  {write_str_nr(user,"^HYUNfrogged,^ ");}

if (ustr[user].anchor)
  {write_str_nr(user,"^HRAnchored,^ ");}
  else
  {write_str_nr(user,"^HYUNanchored,^ ");}
 
if (ustr[user].gagcomm)
  {write_str(user,"^HRGagcommed^]");}
  else
  {write_str(user,"^HYUNgagcommed^]");}

if (how_many_users(ustr[user].name) <= 1) {
	if (astr[ustr[user].area].private)
  	{
   	ustr[user].area=INIT_ROOM;
   	write_str(user,IS_PRIVATE);
  	}
}
write_str(user,"");
sprintf(mess,"Welcome to ^%s^ %s %s",SYSTEM_NAME,ranks[ustr[user].super].lname,ustr[user].say_name);
write_str(user,mess);
write_str(user,"");

write_str(user," Last login from..");
sprintf(mess,"  ^%s^ (^HG%s^)",ustr[user].last_name,ustr[user].last_site);
write_str(user,mess);
tm_then=((time_t) ustr[user].rawtime);
time(&tm);
sprintf(mess,"  %s ago.",converttime((long)((tm-tm_then)/60)));
write_str(user,mess);
write_str(user,"");
write_str(user," This login from..");
sprintf(mess,"  ^%s^ (^HG%s^)",ustr[user].net_name,ustr[user].site);
write_str(user,mess);
write_str(user,"");
quotes(user);
write_str(user,"+---------------------------------------------------------------------------+");   
        strcpy(ustr[user].last_date, ctime(&tm));
         ustr[user].last_date[24]=0;
        strcpy(ustr[user].last_site, ustr[user].site);
        strcpy(ustr[user].last_name, ustr[user].net_name);
        ustr[user].rawtime = tm;
        ustr[user].logging_in=11;
	if (how_many_users(ustr[user].name) > 1) {
         write_str(user,ALREADY_ON);
	 if (!quit_multiples(user)) {
		user_quit(user,1);
		return;
		}
	 }
	if (ustr[user].pause_login==1) {
         write_str_nr(user,"--- Press <ENTER> to complete login ---");
	 telnet_write_eor(user);
	}
	else {
	 add_user(user);
	 add_user2(user,0);
	 }
        return;
       }
    }
   else
    {
     /*---------------------------------------------*/
     /* The file does not exists, so the user has   */      
     /* no previous account                         */      
     /*---------------------------------------------*/      
     if (check_restriction(user, NEW, ANY) == 1)
       {
	write_log(BANLOG,YESTIME,"MAIN: Creation attempt by %s, BANNEWed site %s:%s:sck#%d:slt#%d\n",ustr[user].login_name,ustr[user].site,ustr[user].net_name,ustr[user].sock,user);
        attempts(user);
        return;
       }

     if (!allow_new)
        {
          write_str(user,NO_NEW_LOGIN);
  	  attempts(user);
	  return;
         }
     
     if (system_stats.quota > 0 && system_stats.new_users_today >= system_stats.quota)
       {
         write_str(user,"=====================================================");
         write_str(user,"We are currently using a maximum quota for new users.");
         write_str(user,"The limit for today has been reached.");
         write_str(user,"This will be reset at midnight.");
         write_str(user,"=====================================================");
         attempts(user);                                   
	 return;                                             
       }                                                   

         
     write_str(user,NEW_USER_MESS); 

     write_str(user,"");
     write_str(user,"");

     sprintf(filename,"%s",AGREEFILE); 
      /* Print out the terms of agreement file to the user */
     cat(filename,user,0);

     write_str(user,"");
     write_str(user,"");
     
     write_str_nr(user,PASS_VERIFY); 
     telnet_write_eor(user);
     telnet_echo_off(user);
     strcpy(ustr[user].login_pass,passwd);                   
     ustr[user].logging_in=1;                              
     return;                                                
    }                                                         
                                                        
                                                             
  /*------------------------------------------------------------------------------*/
  /* For new users, double check the password to make sure they entered it right  */
  /* and save the new account if allowed                                          */
  /*------------------------------------------------------------------------------*/

  CHECK_PASS:
     telnet_echo_on(user);
     sscanf(inpstr,"%s",passwd);
     strtolower(passwd);
     st_crypt(passwd);                                    
      
     if (strcmp(ustr[user].login_pass,passwd)) 
       {
         write_str(user,NON_VERIFY);
         ustr[user].login_pass[0]=0;
         attempts(user);  
         return;
       }

     EMAIL2_PASS:
     write_str(user," ");
     telnet_echo_on(user);   
     write_str(user,NEW_CREATE);   

     write_log(SYSTEMLOG,YESTIME,"CREATE: NEW USER created - %s\n", ustr[user].login_name);
     strcpy(ustr[user].password,passwd);   
     init_user(user);
     copy_from_user(user);                        
     write_user(ustr[user].login_name);            
     /* update staff list file */
     /* this will probably only get hit on ROOT_ID creation */
     /* since no NEW user is given a level of WIZ_LEVEL     */
     if (ustr[user].super >= WIZ_LEVEL) do_stafflist();
     ustr[user].logging_in=12;
     if (ustr[user].pause_login==1) {
      write_str_nr(user,"--- Press <ENTER> to complete login ---");
      telnet_write_eor(user);
     }
     else {
      add_user(user);
      add_user2(user,1);
      }
     return;
}


/*-----------------------------------------------------*/
/*   check to see if user has had max login attempts   */
/*-----------------------------------------------------*/
void attempts(int user)
{
if (!--ustr[user].attleft) {
	write_str(user,ATTEMPT_MESS);
	user_quit(user,1); 
	return;
	}

reset_user_struct(user,0);
ustr[user].logging_in=3;
telnet_echo_on(user);
write_str_nr(user,SYS_LOGIN);
telnet_write_eor(user);
}


/* Check if user is already online..if so, terminate old user */
int quit_multiples(int user)
{
int u;
int reload1=0;
ConvPtr tmpConv = NULL;

	for (u=0;u<MAX_USERS;++u) {
	  if (!strcmp(ustr[u].name,ustr[user].name) && 
	      ustr[u].area!= -1 &&
	      u != user ) 
	    {
	      write_str(u,TERMIN);
              reload1=1;
              /* User was hung, but we're gonna save the following into their */
	      /* new login: tell buffer and .call user */

/* CYGNUS2 */
		tmpConv = ustr[u].conv;
		ustr[u].conv = ustr[user].conv;
		ustr[user].conv = tmpConv;

		strcpy(ustr[user].phone_user,ustr[u].phone_user);
		user_quit(u,2); 
	    } /* end of if */
	} /* end of for */

/* reset user structure */
if (reload1) {
	if (!read_to_user(ustr[user].login_name,user)) return 0;
	/* this is a cheesy way to do this, but it works	*/
	/* the mutter string here is used to tell add_user()	*/
	/* to replace room with RE-LOGIN in the user's online	*/
	/* announcement						*/
	strcpy(ustr[user].mutter,"RE-LOGIN");
	}
return 1;
}


/*-----------------------------------------------------------*/
/* Initialize a user for the system or set up data for a     */
/* new user if he can get on.                                */
/*-----------------------------------------------------------*/
void add_user(int user)
{
char room[32];
time_t tm;
#if defined(WINDOWS)
unsigned long arg = 1;
#endif

 if (ustr[user].attach_port == '2' && ustr[user].super < WIZ_LEVEL)
   {
     write_str(user,NO_WIZ_ENTRY);
     user_quit(user,1);
     return;
   }

 /* do we really need to clear these? */ 
 /* doesn't reset_user_struct take care of this in the beginning? */
 ustr[user].locked=           0;
 
 ustr[user].clrmail=          -1;
 ustr[user].time=             time(0);
 ustr[user].invite=           -1;
 ustr[user].last_input=       time(0);
 ustr[user].logging_in=       0;
 ustr[user].file_posn=        0;
 ustr[user].pro_enter=        0;
 ustr[user].t_ent=            0;
 ustr[user].t_num=            0;
 ustr[user].t_name[0]=        0;
 ustr[user].t_host[0]=        0;
 ustr[user].t_ip[0]=          0;
 ustr[user].t_port[0]=        0;
 ustr[user].roomd_enter=      0;
 ustr[user].vote_enter=       0;
 ustr[user].warning_given=    0;
 ustr[user].needs_hostname=   0;
 
 ustr[user].cat_mode =        0;
 ustr[user].numbering =       0;

 if (strcmp(ustr[user].mutter,"RE-LOGIN")) ustr[user].phone_user[0] =   0;
 ustr[user].real_id[0] =      0;
 ustr[user].afkmsg[0] =       0;
 ustr[user].rwho=             1;
 ustr[user].tempsuper=        0;

if (strcmp(ustr[user].mutter,"RE-LOGIN")) {
/* CYGNUS2 */
	if (ustr[user].conv) init_conv_buffer(ustr[user].conv);
}

num_of_users++;

if (!strcmp(ustr[user].name,BOT_ID)) bot=user;

/*
#if defined(WINDOWS)
ioctlsocket(ustr[user].sock, FIONBIO, &arg);
#else
fcntl(ustr[user].sock, F_SETFL, NBLOCK_CMD);
#endif
*/
MY_FCNTL(ustr[user].sock, MY_F_SETFL, NBLOCK_CMD); /* set socket to non-blocking */

/* send room details to user */
time(&tm);
strcpy(ustr[user].last_date,  ctime(&tm));
 ustr[user].last_date[24]=0;
strcpy(ustr[user].last_site, ustr[user].site);
strcpy(ustr[user].last_name, ustr[user].net_name);
ustr[user].rawtime = tm;

copy_from_user(user);                          
write_user(ustr[user].login_name);        

look(user,""); 
check_alert(user,0);

/*----------------------------------*/
/* If user is new, tell all wizzes  */
/*----------------------------------*/
if (ustr[user].numcoms==0) {

sprintf(mess,NEW_USER_TO_WIZ,strip_color(ustr[user].say_name),ustr[user].net_name,ustr[user].site);
   writeall_str(mess, WIZ_ONLY, user, 0, user, BOLD, NONE, 0);
   write_str(user,NEW_HELP);

   if (autopromote == 1)
    write_str(user,NEW_HELP2);

   mess[0]=0;
   }

check_mail(user);

/* If user logging in again from a hung connection, make the spot where the */
/* room name usually goes, say "RE-LOGIN" instead                           */
if (strcmp(ustr[user].mutter,"RE-LOGIN")) {
if (astr[ustr[user].area].hidden)
    sprintf(room," ? ");
else
    sprintf(room,"%s",astr[ustr[user].area].name);
}
else {
 strcpy(room,"RE-LOGIN");
 }
 ustr[user].mutter[0] =       0;

/* send message to other users and to file */
if (ustr[user].super >= WIZ_LEVEL) 
  {
   sprintf(mess, ANNOUNCEMENT_HI, ustr[user].say_name, ustr[user].desc,room);
   writeall_str(mess, 0, user, 0, user, NORM, LOGIO, 0);
  }
else 
  {
   sprintf(mess, ANNOUNCEMENT_LO, ustr[user].say_name, ustr[user].desc,room);
   writeall_str(mess, 0, user, 0, user, NORM, LOGIO, 0);
  }

/* stick signon in file */
syssign(user,1);
if (user==bot) write_bot("+++++ WELCOME");
}

/* Misc add user stuff */
void add_user2(int user, int mode)
{

if (mode==0) {
     system_stats.logins_today++;
     system_stats.logins_since_start++;
     ustr[user].logging_in=0;
     }
else if (mode==1) {
     system_stats.logins_today++;    
     system_stats.logins_since_start++;
     system_stats.new_since_start++;
     system_stats.new_users_today++;
     ustr[user].logging_in=0;
     }

}

