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

extern char *our_delimiters;
extern char mess[ARR_SIZE+25];	/* functions use mess to send output	  */
extern int noprompt;		/* talker waiting for user input?	  */
extern int NUM_AREAS;		/* number of areas defined in config file */
extern int autopromote;		/* allowing users to promote themselves?  */
extern int new_room;		/* room new users log into		  */


/*------------------------------------------------*/
/* set user stuff                                 */
/*------------------------------------------------*/
void set(int user, char *inpstr)
{
  char onoff[3][4];
  char yesno[2][4];
  char whotype[4][7];
  char helptype[4][7];
  char command[ARR_SIZE];

command[0]=0;
  sscanf(inpstr,"%s ",command);
  remove_first(inpstr);  /* get rid of commmand word */
  strtolower(command);

  if (!strcmp("email",command))
    { set_email(user, inpstr); }
  else if (!strcmp("gender",command))
    { set_sex(user, inpstr); }
  else if (!strcmp("homepage",command))
    { set_homepage(user, inpstr); }
  else if (!strcmp("picurl",command))
    { set_webpic(user, inpstr); }
  else if (!strcmp("rows",command) || !strcmp("lines",command))
    { set_rows(user, inpstr); }
  else if (!strcmp("cols",command) || !strcmp("width",command))
    { set_cols(user, inpstr); }
  else if (!strcmp("car",command) || !strcmp("carriage",command) )
    { set_car_ret(user, inpstr); }
  else if (!strcmp("abbrs",command))
    { set_abbrs(user, inpstr); }
  else if (!strcmp("space",command))
    { set_white_space(user, inpstr); }
  else if (!strcmp("hi",command))
    { set_hilite(user, inpstr); }
  else if (!strcmp("hidden",command))
    { set_hidden(user, inpstr); }
  else if (!strcmp("pbreak",command))
    { set_pbreak(user, inpstr); }
  else if (!strcmp("recap",command))
    { set_recap(user, inpstr); }
  else if (!strcmp("home",command))
    { set_home(user, inpstr); }
  else if (!strcmp("atmos",command))
    { set_atmos(user, inpstr); }
  else if (!strcmp("beeps",command))
    { set_beep(user, inpstr); }
  else if (!strcmp("help",command))
    { set_help(user); }
  else if (!strcmp("who",command))
    { set_who(user); }
  else if (!strcmp("color",command))
    { set_color(user, inpstr); }
  else if (!strcmp("visemail",command))
    { set_visemail(user); }
  else if (!strcmp("icq",command))
    { set_icq(user, inpstr); }
  else if (!strcmp("pause",command))
    { set_pause(user); }
  else if (!strcmp("autoread",command))
    { set_autoread(user); }
  else if (!strcmp("autofwd",command))
    { set_autofwd(user); }
  else if (!strcmp("desc",command))
    { set_desc(user, inpstr); }
  else if (!strcmp("fail",command))
    { set_fail(user, inpstr); }
  else if (!strcmp("succ",command))
    { set_succ(user, inpstr); }
  else if (!strcmp("profile",command))
    { set_profile(user, inpstr); }
  else if (!strcmp("entermsg",command))
    { set_entermsg(user, inpstr); }
  else if (!strcmp("exitmsg",command))
    { set_exitmsg(user, inpstr); }
  else if (!strcmp("show",command)) {
   strcpy(onoff[0],"OFF");
   strcpy(onoff[1],"ON ");
   strcpy(onoff[2],"ON ");
   strcpy(yesno[0],"NO ");
   strcpy(yesno[1],"YES");
   strcpy(whotype[0],"OURS  ");
   strcpy(whotype[1],"NUTS  ");
   strcpy(whotype[2],"IFORMS");
   strcpy(whotype[3],"NEW   ");
   strcpy(helptype[0],"OURS  ");
   strcpy(helptype[1],"IFORMS");
   strcpy(helptype[2],"NUTS3 ");
   strcpy(helptype[3],"NUTS2 ");
   write_str(user,"----------------------------------------------------");
   write_str(user," Your .set settings:");
   write_str(user,"");
   sprintf(mess,"Name    : %s",ustr[user].say_name);
   write_str(user,mess);
   sprintf(mess,"Desc    : %s",ustr[user].desc);
   write_str(user,mess);
   sprintf(mess,"Email   : %s",ustr[user].email_addr);
   write_str(user,mess);
   sprintf(mess,"Homepage: %s",ustr[user].homepage);
   write_str(user,mess);
   sprintf(mess,"Pic URL : %s",ustr[user].webpic);
   write_str(user,mess);
   sprintf(mess,"Gender  : %s",ustr[user].sex);
   write_str(user,mess);
   sprintf(mess,"Home    : %s",ustr[user].home_room);
   write_str(user,mess);
   sprintf(mess,"ICQ #   : %s",ustr[user].icq);
   write_str(user,mess);
   sprintf(mess,"Terminal: %s (%d)",terms[ustr[user].term_type].name,ustr[user].term_type);
   write_str(user,mess);
   write_str(user,"");
   sprintf(mess,"Rows    : %-3d   Columns : %-3d   Who_Style   : %s",
    ustr[user].rows,ustr[user].cols,whotype[ustr[user].who]);
   write_str(user,mess);
   sprintf(mess,"Abbrs   : %s   Whtspace: %s   Help_Style  : %s",
    onoff[ustr[user].abbrs],onoff[ustr[user].white_space],helptype[ustr[user].help]);
   write_str(user,mess);
   sprintf(mess,"Cariages: %s   Hilites : %s   Email_Hidden: %s",
    onoff[ustr[user].car_return],onoff[ustr[user].hilite],yesno[ustr[user].semail]);
   write_str(user,mess);
   sprintf(mess,"Passhid : %s   Pbreak  : %s   Pause_Login : %s",onoff[ustr[user].passhid],onoff[ustr[user].pbreak],onoff[ustr[user].pause_login]);
   write_str(user,mess);
   sprintf(mess,"PrivBeep: %s   Color   : %s",
    onoff[ustr[user].beeps],onoff[ustr[user].color]);
   write_str(user,mess);
   write_str(user,"");
   sprintf(mess,"Enter msg: %s",ustr[user].entermsg);
   write_str(user,mess);
   sprintf(mess,"Exit msg : %s",ustr[user].exitmsg);
   write_str(user,mess);
   sprintf(mess,"Succ     : %s",ustr[user].succ);
   write_str(user,mess);
   sprintf(mess,"Fail     : %s",ustr[user].fail);
   write_str(user,mess);
   write_str(user,"----------------------------------------------------");
   return;
  }
 else {
 write_str(user,"Valid options are: (help on these under ^.h set^)");
 write_str(user,"  cols               abbrs    (toggle)");
 write_str(user,"  desc               autofwd  (toggle)");
 write_str(user,"  email              autoread (toggle)");
 write_str(user,"  entermsg           beeps    (toggle)");
 write_str(user,"  exitmsg            car      (toggle)");
 write_str(user,"  fail               color    (toggle)");
 write_str(user,"  gender             help     (toggle)");
 write_str(user,"  home               hi       (toggle)");
 write_str(user,"  homepage           hidden   (toggle)");
 write_str(user,"  icq                pause    (toggle)");
 write_str(user,"  picurl             pbreak   (toggle)");
 write_str(user,"  profile            space    (toggle)");
 write_str(user,"  recap              visemail (toggle)");
 write_str(user,"  rows               who      (toggle)");
 write_str(user,"  show");
 write_str(user,"  succ");
 }
}


/*** set user description ***/
void set_desc(int user, char *inpstr)
{
int i=0,number_of_carets=0;

if (!strlen(inpstr)) 
  {
   sprintf(mess,"Your description is : %s",ustr[user].desc);
   write_str(user,mess);  
   return;
  }

if (!strcmp(inpstr,DEF_DESC) && !strcmp(ustr[user].desc,DEF_DESC)) {
   write_str(user,"No, you need to set a different one.");
   return;
   }

/* count the carets */
for(i=0;i<strlen(inpstr);i++)
{
  if (inpstr[i]=='^') number_of_carets++;
}

if (number_of_carets%2)
	strcat(inpstr,"^");
  
if (strlen(inpstr) > DESC_LEN-1) 
  {
    write_str(user,"Description too long");  
    return;
  }

strcpy(ustr[user].desc,inpstr);

if (autopromote == 1)
 check_promote(user,6);

if ((ustr[user].tempsuper==0) && (ustr[user].area==new_room))
 strcpy(ustr[user].home_room,astr[INIT_ROOM].name);

copy_from_user(user);
write_user(ustr[user].name);
sprintf(mess,"New desc: %s",ustr[user].desc);
write_str(user,mess);

}


/** set your fail message for when you are not online **/
void set_fail(int user, char *inpstr)
{

if (!strlen(inpstr)) { 
   sprintf(mess,"^HYYour fail is:^ %s",ustr[user].fail);
   write_str(user,mess);
   return; 
   }
if (!strcmp(inpstr,"clear") || !strcmp(inpstr,"none") || 
    !strcmp(inpstr,"-c")) {
    ustr[user].fail[0]=0;
    copy_from_user(user);
    write_user(ustr[user].name);
    write_str(user,"Fail message cleared.");
    return; 
    } 

if (strlen(inpstr) > MAX_ENTERM) {
   write_str(user,"Message too long.");
   return;
   }

if (ustr[user].frog) strcpy(inpstr,FROG_TALK);

strcpy(ustr[user].fail,inpstr);
copy_from_user(user);
write_user(ustr[user].name);
sprintf(mess,"^HYNew fail:^ %s",ustr[user].fail);
write_str(user,mess);

}

/** set succ message for tells, remotes, beeps **/
void set_succ(int user, char *inpstr)
{

if (!strlen(inpstr)) { 
   sprintf(mess,"^HYYour success is:^ %s",ustr[user].succ);
   write_str(user,mess);
   return; 
   }
if (!strcmp(inpstr,"clear") || !strcmp(inpstr,"none") || 
    !strcmp(inpstr,"-c")) {
    ustr[user].succ[0]=0;
    copy_from_user(user);
    write_user(ustr[user].name);
    write_str(user,"Success message cleared.");
    return; 
    } 
if (strlen(inpstr) > MAX_ENTERM) {
   write_str(user,"Message too long.");
   return;
   }

if (ustr[user].frog) strcpy(inpstr,FROG_TALK);

strcpy(ustr[user].succ,inpstr);
copy_from_user(user);
write_user(ustr[user].name);
sprintf(mess,"^HYNew success:^ %s",ustr[user].succ);
write_str(user,mess);

}


/*------------------------------------------------*/
/* set autoread                                   */
/*------------------------------------------------*/
void set_autoread(int user)
{

  if (ustr[user].autor==3)
    {
      write_str(user, "Autoread now ^HYoff^.");
      ustr[user].autor = 0;
    }
   else if (ustr[user].autor==2)
    {
      write_str(user, "Autoread now on ^HYfor logins and online^.");
      ustr[user].autor = 3;
    }
   else if (ustr[user].autor==1)
    {
      write_str(user, "Autoread now on ^HYfor online only^.");
      ustr[user].autor = 2;
    }
   else if (ustr[user].autor==0)
    {
      write_str(user, "Autoread now on ^HYfor logins only^.");
      ustr[user].autor = 1;
    }
  write_str(user,"* .set autoread again for more options *");  

  copy_from_user(user);
  write_user(ustr[user].name);
}

/** auto forward mail to email_addr **/
void set_autofwd(int user)
{

  if (ustr[user].autof==2)
    {
      write_str(user, "Autofwd now ^HYoff^.");
      ustr[user].autof = 0;
    }
   else if (ustr[user].autof==0)
    {
      write_str(user, "Autofwd now on ^HYall the time^.");
      ustr[user].autof = 1;
    }
   else if (ustr[user].autof==1)
    {
      write_str(user, "Autofwd now on ^HYonly when you're not online^.");
      ustr[user].autof = 2;
    }
  write_str(user,"* .set autofwd again for more options *");  

  copy_from_user(user);
  write_user(ustr[user].name);
}

/*------------------------------------------------*/
/* set email address                              */
/*------------------------------------------------*/
void set_email(int user, char *inpstr)
{
char temp[EMAIL_LENGTH+1];

  if (!strlen(inpstr)) {
	sprintf(mess,"Your email is: %s",ustr[user].email_addr);
	write_str(user, mess);
	return;
	}

  /* Check for illegal characters in email addy */
  if (strpbrk(inpstr,";/[]\\") ) {
     write_str(user,"Illegal email address");
     return;
     }

  if (strstr(inpstr,"^")) {
     write_str(user,"Email cannot have color or hilite codes in it.");
     return;
     }

  if (strlen(inpstr)>EMAIL_LENGTH)
    {
      write_str(user,"Email address truncated");
      inpstr[EMAIL_LENGTH-1]=0;
    }

 strcpy(temp,inpstr);
 strtolower(temp);

 if ((!strcmp(inpstr,"-c")) || (!strcmp(inpstr,"clear"))) {
     strcpy(inpstr,DEF_EMAIL);
     write_str(user,"Email address cleared and reset.");
     goto SKIP;
     }
 else if (strstr(temp,"whitehouse.gov"))
      {
       write_str(user,"Email address not valid.");
       return;
      }
  else if (!strstr(inpstr,".") || !strstr(inpstr,"@")) {
       write_str(user,"Email address not valid.");
       return;
      }

  sprintf(mess,"Set user email address to: %s",inpstr);
  write_str(user,mess);

SKIP:
  strcpy(ustr[user].email_addr,inpstr);

  copy_from_user(user);
  write_user(ustr[user].login_name);
}

/*------------------------------------------------*/
/* set homepage                                   */
/*------------------------------------------------*/
void set_homepage(int user, char *inpstr)
{

  if (!strlen(inpstr)) {
	sprintf(mess,"Your homepage is: %s",ustr[user].homepage);
	write_str(user, mess);
	return;
	}

  if (strstr(inpstr,"^")) {
     write_str(user,"Homepage cannot have color or hilite codes in it.");
     return;
     }

  if (strlen(inpstr) > HOME_LEN) 
     {
      write_str(user,"Home page address truncated");
      inpstr[HOME_LEN-1]=0;
     }

  strcpy(ustr[user].homepage,inpstr);

  copy_from_user(user);
  write_user(ustr[user].login_name);
  sprintf(mess,"Set page to: %s",inpstr);
  write_str(user,mess);

}

/*------------------------------------------------*/
/* set URL where users picture can be found       */
/*------------------------------------------------*/
void set_webpic(int user, char *inpstr)
{

  if (!strlen(inpstr)) {
	sprintf(mess,"Your picture URL is: %s",ustr[user].webpic);
	write_str(user, mess);
	return;
	}

  if (strstr(inpstr,"^")) {
     write_str(user,"Picture URL address cannot have color or hilite codes in it.");
     return;
     }

  if (strlen(inpstr) > HOME_LEN) 
     {
      write_str(user,"Picture URL address truncated");
      inpstr[HOME_LEN-1]=0;
     }

  strcpy(ustr[user].webpic,inpstr);

  copy_from_user(user);
  write_user(ustr[user].login_name);
  sprintf(mess,"Set picture URL to: %s",inpstr);
  write_str(user,mess);

}

/*------------------------------------------------*/
/* set gender                                     */
/*------------------------------------------------*/
void set_sex(int user, char *inpstr)
{

if (!strlen(inpstr))
  {
   sprintf(mess,"Your gender is : %s",ustr[user].sex);
   write_str(user,mess);
   return;
  }

  if (strlen(inpstr)>29)
    {
      write_str(user,"Gender truncated");
      inpstr[29]=0;
    }

  strcpy(ustr[user].sex,inpstr);

  if (autopromote == 1)
   check_promote(user,7);

  copy_from_user(user);
  write_user(ustr[user].name);
  sprintf(mess,"Set user gender to: %s",inpstr);
  write_str(user,mess);
}

/*------------------------------------------------*/
/* set rows                                       */
/*------------------------------------------------*/
void set_rows(int user, char *inpstr)
{

  int value=5;
  
  sscanf(inpstr,"%d", &value);
  
  if (value < 5 || value > 256)
    {
      write_str(user,"Rows set to 25 (valid range is 5 to 256)");
      value = 25;
    }
    
  ustr[user].rows = value;

  copy_from_user(user);
  write_user(ustr[user].login_name);
  sprintf(mess,"Set terminal rows to: %d",value);
  write_str(user,mess);
}

/*------------------------------------------------*/
/* set cols                                       */
/*------------------------------------------------*/
void set_cols(int user, char *inpstr)
{

  int value=5;
  
  sscanf(inpstr,"%d", &value);
  
  if (value < 16 || value > 256)
    {
      write_str(user,"Columns set to 256 (valid range is 16 to 256)");
      value = 256;
    }
    
  ustr[user].cols = value;

  copy_from_user(user);
  write_user(ustr[user].login_name);
  sprintf(mess,"Set terminal cols to: %d",value);
  write_str(user,mess);
}

/*------------------------------------------------*/
/* set car_return                                 */
/*------------------------------------------------*/
void set_car_ret(int user, char *inpstr)
{

if (!strlen(inpstr)) {
  if (!ustr[user].car_return) {
   write_str(user,"Set carriage returns ON");
   ustr[user].car_return = 1;
   }
   else {
   write_str(user,"Set carriage returns OFF");
   ustr[user].car_return = 0;
   }
 }
else {
 if (!strcmp(inpstr,"1")) {
   write_str(user,"Set carriage returns ON");
   ustr[user].car_return = 1;
   }
 else if (!strcmp(inpstr,"0")) {
   write_str(user,"Set carriage returns OFF");
   ustr[user].car_return = 0;
  }
 else {
   write_str(user,"Set carriage returns ON");
   ustr[user].car_return = 1;
  }   
 } /* end of else */

  copy_from_user(user);
  write_user(ustr[user].login_name);
}

/*------------------------------------------------*/
/* set atmos                                      */
/*------------------------------------------------*/
void set_atmos(int user, char *inpstr)
{

  int    value  = -1;
  int    factor = -1;


if (ustr[user].tempsuper < WIZ_LEVEL) {
 write_str(user,NOT_WORTHY);
 return;
}  

  sscanf(inpstr,"%d %d", &value, &factor);
  
  if (value < 0 || value > 1000)
    {
      value = 50;
    }
    
 if (factor < 0 || factor >1000)
    {
      factor = 5;
    }

  sprintf(mess,"Atmos frequency chance set to: %d %d",value, factor);
  write_str(user,mess);

  ATMOS_RESET     = value;
  ATMOS_FACTOR    = factor;
  ATMOS_COUNTDOWN = value;
}

/*------------------------------------------------*/
/* set abbrs                                      */
/*------------------------------------------------*/
void set_abbrs(int user, char *inpstr)
{


  if (ustr[user].abbrs)
    {
      write_str(user, "Abbreviations are now off for you.");
      ustr[user].abbrs = 0;
    }
   else
    {
      write_str(user, "You can now use abbreviations");
      ustr[user].abbrs = 1;
    }
    
  copy_from_user(user);
  write_user(ustr[user].login_name);
}

/*------------------------------------------------*/
/* set white space                                */
/*------------------------------------------------*/
void set_white_space(int user, char *inpstr)
{


  if (ustr[user].white_space)
    {
      write_str(user, "White space removal is now off.");
      ustr[user].white_space = 0;
    }
   else
    {
      write_str(user, "White space removal is now on.");
      ustr[user].white_space = 1;
    }
    
  copy_from_user(user);
  write_user(ustr[user].login_name);
}

/*------------------------------------------------*/
/* set hilights                                   */
/*------------------------------------------------*/
void set_hilite(int user, char *inpstr)
{

  if (ustr[user].hilite==2)
    {
      write_str(user, "High_lighting now off.");
      ustr[user].hilite = 0;
    }
  else if (ustr[user].hilite==1)
    {
      write_str(user, "High_lighting now on for everything except private communication which will be normal with color.");
      ustr[user].hilite = 2;
    }
  else
    {
      write_str(user, "High_lighting now on for everything.");
      ustr[user].hilite = 1;
    }
    
  copy_from_user(user);
  write_user(ustr[user].login_name);
}

/*------------------------------------------------*/
/* set password echo                              */
/*------------------------------------------------*/
void set_hidden(int user, char *inpstr)
{


  if (ustr[user].passhid)
    {
      write_str(user, "Password WILL be echoed during logins");
      ustr[user].passhid = 0;
    }
   else
    {
      write_str(user, "Password will NOT be echoed during logins.");
      ustr[user].passhid = 1;
    }
    
  copy_from_user(user);
  write_user(ustr[user].login_name);
}

/*------------------------------------------------*/
/* set login pause                                */
/*------------------------------------------------*/
void set_pause(int user)
{

  if (ustr[user].pause_login)
    {
      write_str(user, "You will NOT get a pause when your login");
      ustr[user].pause_login = 0;
    }
   else
    {
      write_str(user, "You WILL get a pause when your login");
      ustr[user].pause_login = 1;
    }
    
  copy_from_user(user);
  write_user(ustr[user].login_name);
}

/*------------------------------------------------*/
/* set break in who or not                        */
/*------------------------------------------------*/
void set_pbreak(int user, char *inpstr)
{


  if (ustr[user].pbreak)
    {
      write_str(user, "Who listing will be continuous.");
      ustr[user].pbreak = 0;
    }
   else
    {
      write_str(user, "Who listing will be paged.");
      ustr[user].pbreak = 1;
    }
    
  copy_from_user(user);
  write_user(ustr[user].login_name);
}


/*------------------------------------------------*/
/* set beeps on private communication             */
/*------------------------------------------------*/
void set_beep(int user, char *inpstr)
{

  if (ustr[user].beeps)
    {
      write_str(user, "You now will ^NOT^ get beeps on private communications");
      ustr[user].beeps = 0;
    }
   else
    {
      write_str(user, "You now ^WILL^ get beeps on private communications");
      ustr[user].beeps = 1;
    }
    
  copy_from_user(user);
  write_user(ustr[user].login_name);
}


/*------------------------------------------------*/
/* set your name capitalization                   */
/*------------------------------------------------*/
void set_recap(int user, char *inpstr)
{
int i=0,len=0,number_of_carets=0;
char sayname[SAYNAME_LEN+1];
char lowername[SAYNAME_LEN+1];
char colorname[SAYNAME_LEN+3];

strncpy(colorname,inpstr,SAYNAME_LEN-4);
strcpy(sayname,strip_color(colorname));
len = strlen(sayname);

if (!len) {
  write_str(user,GIVE_CAPNAME);
  return;
 }

if (len > NAME_LEN) {
  write_str(user,SYS_NAME_LONG);
  return;
 }

strcpy(lowername,sayname);
strtolower(lowername);

/* name = sayname */
/* tempname = lowername */

/* if the name itself does not match the original, reject */
if (strcmp(lowername, ustr[user].login_name)) {
  sprintf(mess,"New name (%s) is not the same as original name (%s)",
          sayname, ustr[user].login_name);
  write_str(user,mess);
  return;
  }

/* count the carets */
for(i=0;i<strlen(colorname);i++)
{
  if (colorname[i]=='^') number_of_carets++;
}

/* do not allow 3 successive carets */
for(i=0;i<strlen(colorname);i++)
{
  if ((colorname[i]=='^') && (colorname[i+1]=='^') && (colorname[i+2]=='^'))
  {
    write_str(user,"You have too many carets together.  Please redo");
    return;
  }
}

/* do not allow 1st and 2nd chars OR 2nd and 3rd chars to both be carets */
if ((colorname[0]=='^' && colorname[1]=='^') || (colorname[1]=='^' && colorname[2]=='^'))
{
  write_str(user,"Incorrect format. Please try again.");
  return;
}

/* if odd number of carets, cat on an ending caret */
if (number_of_carets%2)
	strcat(colorname,"^");

if (strstr(colorname,"^")) {
	strcat(colorname,our_delimiters);
	strcpy(ustr[user].say_name, our_delimiters);
	strcat(ustr[user].say_name, colorname);
}
else strcpy(ustr[user].say_name, colorname);
sprintf(mess, "Your name is recapped to: %s", ustr[user].say_name);
write_str(user,mess);

/* change exempt file */
/* first arguement is to check against names in file for user */
/* second is the new name we're changing to */
change_exem_data(ustr[user].name,strip_color(ustr[user].say_name));

copy_from_user(user);
write_user(ustr[user].login_name);
}

/*------------------------------------------------*/
/* set your home room                             */
/*------------------------------------------------*/
void set_home(int user, char *inpstr)
{
int found = 0;
int new_area;

  if (!strlen(inpstr)) {
     sprintf(mess,"Your home room is the: %s",ustr[user].home_room);
     write_str(user,mess);
     return;
     }

  if (strstr(inpstr,"^")) {
     write_str(user,"Room cannot have color or hilite codes in it.");
     return;
     }

  if (strlen(inpstr) > NAME_LEN) 
     {
      write_str(user,"Room name length too long.");
      return;
     }
         /* Cygnus */
   if ( (!strcmp(inpstr,HEAD_ROOM)) || (!strcmp(inpstr,ARREST_ROOM)) ||
        (!strcmp(inpstr,"sky_palace")) ) {
      write_str(user,"You cannot make that room your home.");
      return;
      }

   /*--------------------*/
   /* see if area exists */
   /*--------------------*/

   found = FALSE;
   for (new_area=0; new_area < NUM_AREAS; ++new_area)
    { 
     if (!strcmp(astr[new_area].name, inpstr) )
       { 
         found = TRUE;
         break;
       }
    }
 
   if (!found)
     {
      write_str(user,NO_ROOM);
      return;
     }
  
/*----------------------------------------------*/
/* check for secure room                        */
/*----------------------------------------------*/

if (ustr[user].security[new_area] == 'X')
  {
   write_str(user,"Your security clearance does not let you enter there");
   return;
  }


/*-------------------------------------------------------------------*/
/* see if new room has exits, if not and not wizard, no set possible */
/*-------------------------------------------------------------------*/
found = TRUE;
if ( (!strlen(astr[new_area].move)) || (!strcmp(astr[new_area].move,"*")) ) 
    {
     found = FALSE;
    }

/*--------------------------------------------------------------*/
/* anyone above a 3 can teleport to non-connected rooms         */
/*--------------------------------------------------------------*/

if (!found)
  {
    if ((ustr[user].tempsuper < TELEP_LEVEL) && (ustr[user].security[new_area] == 'N')) 
      {
       write_str(user,"You cannot make a non-connected room your home.");
       return;
      }
  }

  strcpy(ustr[user].home_room,astr[new_area].name);

  copy_from_user(user);
  write_user(ustr[user].login_name);
  sprintf(mess,"Set home room to: %s",astr[new_area].name);
  write_str(user,mess);
}

/** command to test and turn on/off color attribs. **/
void set_color(int user, char *inpstr)
{
int a=0;

if (!strlen(inpstr)) {
  if (ustr[user].color==0) {
   write_str(user,"Color is now   On");
   ustr[user].color=1;
   }
  else {
   write_str(user,"Color is now   Off");
   ustr[user].color=0;
   }
 }
else if (!strcmp(inpstr,"on") || !strcmp(inpstr,"ON")) {
   write_str(user,"Color is now   On");
   ustr[user].color=1;
   }
else if (!strcmp(inpstr,"off") || !strcmp(inpstr,"OFF")) {
   write_str(user,"Color is now   Off");
   ustr[user].color=0;
   }
else if (!strcmp(inpstr,"test") || !strcmp(inpstr,"TEST")) {
a=ustr[user].color;
if (a==0) ustr[user].color=1;
 write_str(user,"This test is to see whether your terminal is capable of");
 write_str(user,"displaying ANSI color. During the test, you should see the color"); 
 write_str(user,"displayed next to its corresponding name, if your terminal IS compatible.");
 write_str(user,"To use a color code in something, read .h coloruse");
 write_str(user,"  COLOR                EFFECT                  CODE");
 write_str(user,"  Low Green      ^LG      XXXXXXXXXXXXXXXXX ^       LG");
 write_str(user,"  Low Yellow     ^LY      XXXXXXXXXXXXXXXXX ^       LY");
 write_str(user,"  Low Red        ^LR      XXXXXXXXXXXXXXXXX ^       LR");
 write_str(user,"  Low Blue       ^LB      XXXXXXXXXXXXXXXXX ^       LB");
 write_str(user,"  Low Magenta    ^LM      XXXXXXXXXXXXXXXXX ^       LM");
 write_str(user,"  Low White      ^LW      XXXXXXXXXXXXXXXXX ^       LW");
 write_str(user,"  Low Cyan       ^LC      XXXXXXXXXXXXXXXXX ^       LC");
 write_str(user,"  High Green     ^HG      XXXXXXXXXXXXXXXXX ^       HG");
 write_str(user,"  High Yellow    ^HY      XXXXXXXXXXXXXXXXX ^       HY");
 write_str(user,"  High Red       ^HR      XXXXXXXXXXXXXXXXX ^       HR");
 write_str(user,"  High Blue      ^HB      XXXXXXXXXXXXXXXXX ^       HB");
 write_str(user,"  High Magenta   ^HM      XXXXXXXXXXXXXXXXX ^       HM");
 write_str(user,"  High White     ^HW      XXXXXXXXXXXXXXXXX ^       HW");
 write_str(user,"  High Cyan      ^HC      XXXXXXXXXXXXXXXXX ^       HC");
 write_str(user,"  Bold       ^          XXXXXXXXXXXXXXXXX ^       None");
 write_str(user,"  Blinking             ^BLXXXXXXXXXXXXXXXXX ^       BL");
 write_str(user,"  Underlined           ^ULXXXXXXXXXXXXXXXXX ^       UL");
 write_str(user,"  Reverse Video        ^RVXXXXXXXXXXXXXXXXX ^       RV");
if (a==0) ustr[user].color=0;
 write_str(user,"<Ok>"); 
 }
else {
   write_str(user,"Usage: .set color        -  Toggle your color switch on/off");
   write_str(user,"       .set color on     -  Turns colored attributes on");
   write_str(user,"       .set color off    -  Turns colored attributes off");
   write_str(user,"       .set color test   -  Tests your terminal for ANSI color");
 }

   read_user(ustr[user].login_name);
   t_ustr.color = ustr[user].color;
   write_user(ustr[user].login_name);
}


/** show email address - yes or no **/
void set_visemail(int user)
{

  if (ustr[user].semail)
    {
      write_str(user, "Email address now visible.");
      ustr[user].semail = 0;
    }
   else
    {
      write_str(user,"Email address now hidden.");
      ustr[user].semail = 1;
    }
    
  read_user(ustr[user].login_name);
  t_ustr.semail = ustr[user].semail;
  write_user(ustr[user].login_name);
}

/* Set .help style */
void set_help(int user)
{
 char type[4][7];

   strcpy(type[0],"OURS  ");
   strcpy(type[1],"IFORMS");
   strcpy(type[2],"NUTS3 ");
   strcpy(type[3],"NUTS2 ");

 if (ustr[user].help==0)
    ustr[user].help=1;
 else if (ustr[user].help==1)
    ustr[user].help=2;
 else if (ustr[user].help==2)
    ustr[user].help=3;
 else if (ustr[user].help==3)
    ustr[user].help=0;

    sprintf(mess,".help type now set to: ^HY%s^",type[ustr[user].help]);
    write_str(user,mess);

  read_user(ustr[user].login_name);
  t_ustr.help = ustr[user].help;
  write_user(ustr[user].login_name);
}

/* Set .who style */
void set_who(int user)
{
 char type[4][7];

   strcpy(type[0],"OURS  ");
   strcpy(type[1],"NUTS  ");
   strcpy(type[2],"IFORMS");
   strcpy(type[3],"NEW   ");

 if (ustr[user].who==0)
    ustr[user].who=1;
 else if (ustr[user].who==1)
    ustr[user].who=2;
 else if (ustr[user].who==2)
    ustr[user].who=3;
 else if (ustr[user].who==3)
    ustr[user].who=0;

    sprintf(mess,".who type now set to: ^HY%s^",type[ustr[user].who]);
    write_str(user,mess);

  read_user(ustr[user].login_name);
  t_ustr.who = ustr[user].who;
  write_user(ustr[user].login_name);
}

/*------------------------------------------------*/
/* set icq number                                 */
/*------------------------------------------------*/
void set_icq(int user, char *inpstr)
{

  if (!strlen(inpstr)) {
	sprintf(mess,"Your ICQ # is: %s",ustr[user].icq);
	write_str(user, mess);
	return;
	}

  if (strstr(inpstr,"^")) {
     write_str(user,"ICQs cannot have color or hilite codes in them.");
     return;
     }

  if (strlen(inpstr) > 20) 
     {
      write_str(user,"ICQ number truncated");
      inpstr[20-1]=0;
     }

  strcpy(ustr[user].icq,inpstr);

  copy_from_user(user);
  write_user(ustr[user].login_name);
  sprintf(mess,"Set ICQ # to: %s",inpstr);
  write_str(user,mess);

}


/** Enter profile ***/
void set_profile(int user, char *inpstr)
{
char *c;
int ret_val;
int redo=0;
int quickdone=0;
int i=0; /*******/
int op_mode=0; /******/
char option[ARR_SIZE]; /******/
char filename[FILE_NAME_LEN];
char filename2[FILE_NAME_LEN];
FILE *fp;

/* get memory */
STARTPRO:

if (!ustr[user].pro_enter) {
       option[0]=0;
       sscanf(inpstr,"%s ",option);
       if (!strcmp(option,"-c") || !strcmp(option,"clear")) {
       sprintf(filename,"%s/%s",PRO_DIR,ustr[user].name);
       remove(filename);
       write_str(user,"Profile deleted."); redo=0;
       return;
       }
        if (!(ustr[user].pro_start=(char *)malloc(82*PRO_LINES))) {
        write_str(user,BAD_MALLOC);
	write_log(ERRLOG,YESTIME,"MALLOC: Can't malloc memory in set_profile! %s\n",get_error());
        redo=0;
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
             write_str(user,"Profile lines start at 1. Not 0. Try again.");
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
            inedit_file(user,inpstr,1,op_mode);
            return;
           }
          else if (op_mode==5) {
            if (!strlen(inpstr)) {
              write_str(user,"You must have text or a -b after this option");
              return;
              }
            if (!strcmp(inpstr,"-b")) op_mode=6;
            inedit_file(user,inpstr,i,op_mode);
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
         write_str(user,"Profile lines start at 1. Not 0. Try again.");
         return;
         }
       if (i > PRO_LINES) {
         sprintf(mess,"The line number can't be higher than the max profile lines allowed, which is currently %d",PRO_LINES);
         write_str(user,mess);
         return;
         }
       remove_first(inpstr);
       if (!strcmp(inpstr,"-c")) op_mode=1;
       else if (!strcmp(inpstr,"-b")) op_mode=2;
       else op_mode=0;
       inedit_file(user,inpstr,i,op_mode);
       return;
      } /* end of inedit */         
    ustr[user].pro_enter=1;
    ustr[user].pro_end=ustr[user].pro_start;
    if (!redo) {
    sprintf(mess,"%s is entering a profile..",ustr[user].say_name);
    writeall_str(mess, 1, user, 0, user, NORM, NONE, 0);
    }
    if (!redo) {
	/* save all user listen-ignore flags so we can give */
	/* them back when we're done */
     strcpy(ustr[user].mutter,ustr[user].flags);
     user_ignore(user,"all");
     }
    write_str(user,"");
    write_str(user,"** Entering a profile, finish with a '.' on a line by itself **");
    sprintf(mess,"** Max lines you can write is %d",PRO_LINES);
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

if (ustr[user].pro_enter > PRO_LINES) {
QUICKDONE:
   if (*c=='s' && *(c+1)==0) {
     ret_val=write_pro(user);
        if (ret_val) {
	write_str(user,"");
	write_str(user,"Profile stored");
	}
        else {
	write_str(user,"");
	write_str(user,"Profile not stored");
	}
        free(ustr[user].pro_start);  ustr[user].pro_enter=0;
        ustr[user].pro_end='\0';
        noprompt=0;
        sprintf(mess,"%s finishes entering a profile",ustr[user].say_name);
        writeall_str(mess, 1, user, 0, user, NORM, NONE, 0);
	/* give them back their saved flags */
        strcpy(ustr[user].flags,ustr[user].mutter);
        ustr[user].mutter[0]=0;

        if (autopromote == 1)
         check_promote(user,9);

        redo=0;
        return;
     }
   else if (*c=='v' && *(c+1)==0) {
write_str(user,"+-----------------------------------------------------------------------------+");
c='\0';
strcpy(filename2,get_temp_file());
fp=fopen(filename2,"w");
for (c=ustr[user].pro_start;c<ustr[user].pro_end;++c) {
    putc(*c,fp);
    }
    fclose(fp);
    cat(filename2,user,0);
    remove(filename2);
c='\0';
write_str(user,"+-----------------------------------------------------------------------------+");
	if (quickdone==1) {
		quickdone=0;
		write_str(user,"");
		sprintf(mess,"%d: ",ustr[user].pro_enter);
		write_str_nr(user,mess);
		telnet_write_eor(user);
	}
	else {
            write_str_nr(user,PROFILE_PROMPT);
		 telnet_write_eor(user);
	}
            noprompt=1;  return;
        }
   else if (*c=='r' && *(c+1)==0) {
        free(ustr[user].pro_start); ustr[user].pro_enter=0;
        ustr[user].pro_end='\0';
        redo=1;
        goto STARTPRO;
        }             
   else if (*c=='a' && *(c+1)==0) {
        free(ustr[user].pro_start); ustr[user].pro_enter=0;
        ustr[user].pro_end='\0';
	write_str(user,"");
        write_str(user,"Profile not stored");
        noprompt=0;
        sprintf(mess,"%s finishes entering a profile",ustr[user].say_name);
        writeall_str(mess, 1, user, 0, user, NORM, NONE, 0);
	/* give them back their saved flags */
        strcpy(ustr[user].flags,ustr[user].mutter);
        ustr[user].mutter[0]=0;
        redo=0;
        return;
        }             
   else {
    write_str_nr(user,PROFILE_PROMPT);
	 telnet_write_eor(user);
    return;
   } 
  }

if (*c=='.' && *(c+1)==0) {
        if (ustr[user].pro_enter!=1)   {
            ustr[user].pro_enter= PRO_LINES + 1;
            write_str_nr(user,PROFILE_PROMPT);
		 telnet_write_eor(user);
            noprompt=1;  return;
            }
        else {
	write_str(user,"");
	write_str(user,"Profile not stored");
	}
        free(ustr[user].pro_start);  ustr[user].pro_enter=0;
        noprompt=0;
        sprintf(mess,"%s finishes entering a profile",ustr[user].say_name);
        writeall_str(mess, 1, user, 0, user, NORM, NONE, 0);
	/* give them back their saved flags */
        strcpy(ustr[user].flags,ustr[user].mutter);
        ustr[user].mutter[0]=0;
        redo=0;
        return;
        }
else if (*c=='.') {
if ( (*(c+1)=='s') || (*(c+1)=='r') || (*(c+1)=='a') ||
     (*(c+1)=='v') ) {
  *c=*(c+1);
  *(c+1)=0;
  quickdone=1;
  goto QUICKDONE;
  }
} /* end of else if */

/* write string to memory */
while(*c) *ustr[user].pro_end++=*c++;
*ustr[user].pro_end++='\n';

/* end of lines */
if (ustr[user].pro_enter==PRO_LINES) {
            ustr[user].pro_enter= PRO_LINES + 1;
            write_str_nr(user,PROFILE_PROMPT);
		 telnet_write_eor(user);
            noprompt=1;  return;
        }
sprintf(mess,"%d: ",++ustr[user].pro_enter);
write_str_nr(user,mess);
telnet_write_eor(user);
}


/** entermessage so instead of "walks in" if one chooses **/
void set_entermsg(int user, char *inpstr)
{

if (!strlen(inpstr)) { 
   sprintf(mess,"^HYYour entermessage is:^ %s",ustr[user].entermsg);
   write_str(user,mess);
   return; 
   }
if (!strcmp(inpstr,"clear") || !strcmp(inpstr,"none") ||
    !strcmp(inpstr,"-c")) {
    strcpy(ustr[user].entermsg,DEF_ENTER);
    copy_from_user(user);
    write_user(ustr[user].name);
    write_str(user,"Entermsg set to default.");
    return;
    }
if (strlen(inpstr) > MAX_ENTERM) {
   write_str(user,"Message too long.");
   return;
   }

strcpy(ustr[user].entermsg,inpstr);
copy_from_user(user);
write_user(ustr[user].name);
sprintf(mess,"^HYNew entermsg:^ %s",ustr[user].entermsg);
write_str(user,mess);

}

/** exitmessage so instead of "goes to the" if one chooses **/
void set_exitmsg(int user, char *inpstr)
{

if (!strlen(inpstr)) { 
   sprintf(mess,"^HYYour exitmessage is:^ %s",ustr[user].exitmsg);
   write_str(user,mess);
   return; 
   }
if (!strcmp(inpstr,"clear") || !strcmp(inpstr,"none") ||
    !strcmp(inpstr,"-c")) {
    strcpy(ustr[user].exitmsg,DEF_EXIT);
    copy_from_user(user);
    write_user(ustr[user].name);
    write_str(user,"Exitmsg set to default.");
    return;
    }
if (strlen(inpstr) > MAX_EXITM) {
   write_str(user,"Message too long.");
   return;
   }

strcpy(ustr[user].exitmsg,inpstr);
copy_from_user(user);
write_user(ustr[user].name);
sprintf(mess,"^HYNew exitmsg:^ %s",ustr[user].exitmsg);
write_str(user,mess);

}

