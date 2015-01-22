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
extern char t_mess[ARR_SIZE+25];  /* functions use t_mess as a buffer    */
extern int says;		  /* says in defined time period         */
extern int bot;			  /* this will hold the bots user number */
extern struct command_struct sys[];
extern int tells;		  /* tells in defined time period        */
extern char bt_conv[NUM_LINES][MAX_LINE_LEN+1]; /* stores lines of conversation in wiztell buffer */
extern int bt_count;		  /* wiztell count in the buffer         */

char sh_conv[NUM_LINES][MAX_LINE_LEN+1]; /* store review shouts */
int sh_count;            /* shout count in the buffer */


/*----------------------------------*/
/* Normal speech                    */
/*----------------------------------*/
void say(int user, char *inpstr, int mode)
{
  int z=0,gravoked=0;
  int area = ustr[user].area;

if (!mode) {
 /* Check if command was revoked from user - UNDER CONSTRUCTION */
 for (z=0;z<MAX_GRAVOKES;++z) {
	if (!is_revoke(ustr[user].revokes[z])) continue;
	if (strip_com(ustr[user].revokes[z])==sys[get_com_num(user,".say")].jump_vector) { gravoked=1; break; }
   }
 if (gravoked==1) {
    write_str(user,NOT_WORTHY);
    gravoked=0; z=0;
    return;
    }
} /* end of !mode */
 
  if (!strlen(inpstr) && (bot==-5 || area!=ustr[bot].area))
    {
      write_str(user," [Default blank say action is a review]");
      review(user);
      return;
    }
    

  says++;
  if (!strcmp(inpstr,"quit") || !strcmp(inpstr,"q") || 
      !strcmp(inpstr,"/quit") || !strcmp(inpstr,"QUIT")) {
      write_str(user,"The command to leave is --> .quit");
      return;
      }
  if ((!strcmp(inpstr,"help") && (bot==-5 || area!=ustr[bot].area)) || 
      !strcmp(inpstr,"/help") ||
      !strcmp(inpstr,"HELP")) {
      write_str(user,HELP_HELP);
      return;
     }

if (ustr[user].frog) strcpy(inpstr,FROG_TALK);

		sprintf(mess,VIS_SAYS,ustr[user].say_name,get_reaction(inpstr),inpstr);
		write_str(user,mess);

		if (!ustr[user].vis)
		 sprintf(mess,INVIS_SAYS,INVIS_TALK_LABEL,get_reaction(inpstr),inpstr);

	writeall_str(mess,1,user,0,user,NORM,SAY_TYPE,0);

/*--------------------------------*/
/* store say to the review buffer */
/*--------------------------------*/
/* CYGNUS1 */
  addto_conv_buffer(astr[area].conv, strip_color(mess));

  /* write to bot */
  if (ustr[user].area == ustr[bot].area) {
  sprintf(mess,"+++++ comm_say:%s %s",strip_color(ustr[user].say_name),inpstr);
  write_bot(mess);
  }
}


/*** shout sends speech to all users regardless of area ***/
void shout(int user, char *inpstr)
{
int pos = sh_count%NUM_LINES;
int new_pos = 0;
int f; 

if (!ustr[user].shout) 
  {
   write_str(user,NO_SHOUT);
   return;
  }
  
if (!strlen(inpstr)) 
  {
   write_str(user,"Review shouts:"); 
    
    for (f=0;f<NUM_LINES;++f) 
      {
        if ( strlen( sh_conv[pos] ) )
         {
	  write_str(user,sh_conv[pos]);  
	 }
	new_pos = ( ++pos ) % NUM_LINES;
	pos = new_pos;
      }

    write_str(user,"<Done>");  
    return; 
  }
  
if (ustr[user].frog) {
   strcpy(inpstr,FROG_TALK);
   say(user,inpstr,0);
   return;
   }

sprintf(mess,USER_SHOUTS,ustr[user].say_name,inpstr);

if (!ustr[user].vis)
	sprintf(mess,INVIS_SHOUTS,INVIS_TALK_LABEL,inpstr);

/** Store the shout in the buffer **/
strncpy(sh_conv[sh_count],mess,MAX_LINE_LEN);
new_pos = ( ++sh_count ) % NUM_LINES;
sh_count = new_pos;
	
writeall_str(mess, 0, user, 0, user, NORM, SHOUT, 0);
sprintf(mess,YOU_SHOUT,inpstr);
write_str(user,mess);

  /* write to bot */
  sprintf(mess,"+++++ comm_shout:%s %s",strip_color(ustr[user].say_name),inpstr);
  write_bot(mess);

}


/*** tells another user something without anyone else hearing ***/
void tell_usr(int user, char *inpstr, int mode)
{
int point=0,count=0,i=0,lastspace=0,lastcomma=0,gotchar=0;
int point2=0,multi=0;
int multilistnums[MAX_MULTIS];
char multilist[MAX_MULTIS][ARR_SIZE];
char multiliststr[ARR_SIZE];
char other_user[ARR_SIZE];
int u=-1;
char prefix[SAYNAME_LEN+10];

for (i=0;i<MAX_MULTIS;++i) { multilist[i][0]=0; multilistnums[i]=-1; }
multiliststr[0]=0;
i=0;

if (!strlen(inpstr)) 
  {
    write_str(user,"Review tells:"); 
    write_conv_buffer(user, ustr[user].conv);
    write_str(user,"<Done>");  
    return;
  }

if (ustr[user].gagcomm) {
   write_str(user,NO_COMM);
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

tells++;

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

if ((u=get_user_num(other_user,user))== -1) 
  {
   if (!read_user(other_user)) {
      write_str(user,NO_USER_STR);
      if (!multi) return;
      else continue;
      }
   not_signed_on(user,other_user);
if (user_wants_message(user,FAILS)) {
   sprintf(mess,"%s",t_ustr.fail);
   write_str(user,mess);
   }
      if (!multi) return;
      else continue;
  }

if (!check_gag(user,u,0)) {
      if (!multi) return;
      else continue;
  }

if (ustr[u].pro_enter || ustr[u].vote_enter || ustr[u].roomd_enter) {
    write_str(user,IS_ENTERING);
      if (!multi) return;
      else continue;
    }
  
if (ustr[u].afk)
  {
    if (ustr[u].afk == 1) {
      if (!strlen(ustr[u].afkmsg))
       sprintf(t_mess,"- %s is Away From Keyboard -",ustr[u].say_name);
      else
       sprintf(t_mess,"- %s %-45s -(A F K)",ustr[u].say_name,ustr[u].afkmsg);
      }
     else {
      if (!strlen(ustr[u].afkmsg))
      sprintf(t_mess,"- %s is blanked AFK (is not seeing this) -",ustr[u].say_name);
      else
       sprintf(t_mess,"- %s %-45s -(B A F K)",ustr[u].say_name,ustr[u].afkmsg);
      }

    write_str(user,t_mess);
  }
  
if (!user_wants_message(u,TELLS) && ustr[user].tempsuper<WIZ_LEVEL) 
  {
   sprintf(mess,"%s is ignoring tells",ustr[u].say_name);
   write_str(user,mess);
   if (user_wants_message(user,FAILS)) write_str(user,ustr[u].fail);
      if (!multi) return;
      else continue;
  }

/* check if this user is already in the list */
/* we're gonna reuse some ints here          */
for (point2=0;point2<MAX_MULTIS;++point2) {
	if (multilistnums[point2]==u) { gotchar=1; break; }
   }
point2=0;
if (gotchar) {
  gotchar=0;
  continue;
  }

/* it's ok to send the tell to this user, add them to the multistr */
/* add this user to the list for our next loop */
multilistnums[point]=u;
point++;
} /* end of user for */  
i=0;

/* no multilistnums, must be all bad users */
if (!point) {
	return;
  }

/* loop to compose the messages and print to the users */
for (i=0;i<point;++i) {
u=multilistnums[i];

count=0;
point2=0;
multiliststr[0]=0;
/* make multi string to send to this user */
for (point2=0;point2<point;++point2) {
/* dont send recipients name to themselves */
if (u==multilistnums[point2]) continue;
else count++;
if (count>0)
 strcat(multiliststr,",");
/* add their name to the output string */
if (!ustr[multilistnums[point2]].vis)
 strcat(multiliststr,INVIS_ACTION_LABEL);
else
 strcat(multiliststr,ustr[multilistnums[point2]].say_name);
}

if ((ustr[u].monitor==1) || (ustr[u].monitor==3))
  {
    strcpy(prefix,"<");
    strcat(prefix,ustr[user].say_name);
    strcat(prefix,"> ");
  }
 else
  {
   prefix[0]=0;
  }

/* write to user being told */
if (!strlen(inpstr) && (mode==0)) {
if (ustr[user].vis) 
  {
    sprintf(mess,"You sense that %s is looking for you in the %s",
ustr[user].say_name,astr[ustr[user].area].name);
  }
 else
  {
  if ((ustr[u].monitor==1) || (ustr[u].monitor==3))
    {
      sprintf(prefix,"? %s",ustr[user].say_name);
    }
   else
    {
     strcpy(prefix,INVIS_TALK_LABEL);
    }
   sprintf(mess,"You sense that %s is looking for you.",prefix);
  }

}
else {
if (ustr[user].frog) strcpy(inpstr,FROG_TALK);

if (ustr[user].vis) 
  {
   if (!multi)
    sprintf(mess,VIS_TELLS,ustr[user].say_name,inpstr);
   else
    sprintf(mess,VIS_TELLS_M,ustr[user].say_name,multiliststr,inpstr);
  }
 else
  {
  if ((ustr[u].monitor==1) || (ustr[u].monitor==3))
    {
      sprintf(prefix,"? %s",ustr[user].say_name);
      if (!multi)
       sprintf(mess,VIS_TELLS,prefix,inpstr);
      else
       sprintf(mess,VIS_TELLS_M,prefix,multiliststr,inpstr);
    }
   else
    {
     if (!multi)
      sprintf(mess,INVIS_TELLS,INVIS_TALK_LABEL,inpstr);
     else
      sprintf(mess,INVIS_TELLS_M,INVIS_TALK_LABEL,multiliststr,inpstr);
    }
  }
}

/* if a .beep (mode 1) and .set beeps are on add an extra beep */
/* if mode 0, a normal tell, if beep listening add beep        */
if (ustr[u].beeps) {
 if (mode==1) { }
 else {
 if (user_wants_message(u,BEEPS))
  strcat(mess,"\07");
 }
}

/* if a .beep (mode 1) add beep listening add the beep message */
/* else non beep message                                       */
if (mode==1) {
  if (user_wants_message(u,BEEPS)) {
   strcat(mess," \07 *Beep*\07");
   strcat(inpstr,"  *Beep*");
   }
  else {
   strcat(mess,"  *Beep*");
   strcat(inpstr,"  *Beep*");
   }
 }

/*-----------------------------------*/
/* store the tell in the rev buffer  */
/*-----------------------------------*/
/* moved because of multi tells */
/* CYGNUS2 */
addto_conv_buffer(ustr[u].conv, strip_color(mess));

if (ustr[u].hilite==2)
 write_str(u,mess);
else {
 strcpy(mess, strip_color(mess));
 write_hilite(u,mess);
 }

  /* write to bot */
  if (u==bot) {
  sprintf(mess,"+++++ comm_tell:%s %s",strip_color(ustr[user].say_name),inpstr);
  write_bot(mess);
  }

} /* end of message compisition for loop */

if (multi) {
point2=0;
multiliststr[0]=0;
/* make multi string to send to this user */
for (point2=0;point2<point;++point2) {
/* dont send recipients name to themselves */
if (point2>0)
 strcat(multiliststr,",");
/* add their name to the output string */
if (!ustr[multilistnums[point2]].vis)
 strcat(multiliststr,INVIS_ACTION_LABEL);
else
 strcat(multiliststr,ustr[multilistnums[point2]].say_name);
}
} /* end of if multi */

/* write to teller */
if (strlen(inpstr)) {
 if (!multi)
  sprintf(mess,VIS_FROMTELLS,ustr[u].say_name,inpstr);
 else
  sprintf(mess,VIS_FROMTELLS,multiliststr,inpstr);
}
else {
 if (!multi)
  sprintf(mess,"Telepathic message sent to %s.",ustr[u].say_name);
 else
  sprintf(mess,"Multi-Telepathic message sent to %s.",multiliststr);
}

write_str(user,mess);
if (!multi) {
if (user_wants_message(user,SUCCS) && strlen(inpstr) && strlen(ustr[u].succ))
write_str(user,ustr[u].succ);
}

/* CYGNUS2 */
addto_conv_buffer(ustr[user].conv, strip_color(mess));

i=0;
for (i=0;i<MAX_MULTIS;++i) { multilist[i][0]=0; }
multiliststr[0]=0;

return;
}

/* friend tell */
void frtell(int user, char *inpstr)
{
char str[ARR_SIZE+4];

strcpy(str,"-f ");
strcat(str,inpstr);
tell_usr(user,str,0);
}

/* friend emote */
void femote(int user, char *inpstr)
{
char str[ARR_SIZE+4];

strcpy(str,"-f ");
strcat(str,inpstr);
semote(user,str);
}

/*** beep a user ***/
void beep_u(int user, char *inpstr)
{
if (!strlen(inpstr)) 
  {
    write_str(user,"Beep who?");  
    return;
  }

tell_usr(user,inpstr,1);
}



/** shout and emote together **/
void shemote(int user, char *inpstr)
{
int pos = sh_count%NUM_LINES;
int new_pos = 0;
int f; 
char comstr[ARR_SIZE];

if (!ustr[user].shout) 
  {
   write_str(user,NO_SHOUT);
   return;
  }
  
if (!strlen(inpstr)) 
  {
   write_str(user,"Review shouts:"); 
    
    for (f=0;f<NUM_LINES;++f) 
      {
        if ( strlen( sh_conv[pos] ) )
         {
	  write_str(user,sh_conv[pos]);  
	 }
	new_pos = ( ++pos ) % NUM_LINES;
	pos = new_pos;
      }

    write_str(user,"<Done>");  
    return; 
  }


if (ustr[user].frog) {
   strcpy(inpstr,FROG_EMOTE);
   emote(user,inpstr);
   return;
   }

if (ustr[user].vis) {
   comstr[0]=inpstr[0];
   comstr[1]=0;
   if (comstr[0] == '\'') {
        inpstr[0] = ' ';
        while(inpstr[0] == ' ') inpstr++;
        sprintf(mess,VIS_SHEMOTE_P,ustr[user].say_name,inpstr);
        }
    else
      sprintf(mess,VIS_SHEMOTE,ustr[user].say_name,inpstr);
    }
  else {
   comstr[0]=inpstr[0];
   comstr[1]=0;
   if (comstr[0] == '\'') {
        inpstr[0] = ' ';
        while(inpstr[0] == ' ') inpstr++;
        sprintf(mess,INVIS_SHEMOTE_P,INVIS_ACTION_LABEL,inpstr);
       }
    else
       sprintf(mess,INVIS_SHEMOTE,INVIS_ACTION_LABEL,inpstr);
    }

/** Store the shemote in the buffer **/
strncpy(sh_conv[sh_count],mess,MAX_LINE_LEN);
new_pos = ( ++sh_count ) % NUM_LINES;
sh_count = new_pos;

writeall_str(mess, 0, user, 0, user, NORM, SHOUT, 0);
write_str(user,mess);

  /* write to bot */
  sprintf(mess,"+++++ comm_shemote:%s %s",strip_color(ustr[user].say_name),inpstr);
  write_bot(mess);

}


/*** shout sends speech to all users regardless of area ***/
void shout_think(int user, char *inpstr)
{
int pos = sh_count%NUM_LINES;
int new_pos = 0;
int f; 

if (!ustr[user].shout) 
  {
   write_str(user,NO_SHOUT);
   return;
  }
  
if (!strlen(inpstr)) 
  {
   write_str(user,"Review shouts:"); 
    
    for (f=0;f<NUM_LINES;++f) 
      {
        if ( strlen( sh_conv[pos] ) )
         {
	  write_str(user,sh_conv[pos]);  
	 }
	new_pos = ( ++pos ) % NUM_LINES;
	pos = new_pos;
      }

    write_str(user,"<Done>");  
    return;
  }

if (ustr[user].frog) {
   strcpy(inpstr,"I'm a frog, I'm a frog!");
   think(user,inpstr);
   return;
   }

sprintf(mess,USER_SHTHINKS,ustr[user].say_name,inpstr);

if (!ustr[user].vis)
	sprintf(mess,INVIS_SHTHINKS,INVIS_ACTION_LABEL,inpstr);

/** Store the shout in the buffer **/
strncpy(sh_conv[sh_count],mess,MAX_LINE_LEN);
new_pos = ( ++sh_count ) % NUM_LINES;
sh_count = new_pos;
	
writeall_str(mess, 0, user, 0, user, NORM, SHOUT, 0);
sprintf(mess,YOU_SHTHINKS,inpstr);
write_str(user,mess);

}


/** same as .think but remotely **/
void sthink(int user, char *inpstr)
{
int point=0,count=0,i=0,lastspace=0,lastcomma=0,gotchar=0;
int point2=0,multi=0;
int multilistnums[MAX_MULTIS];
char multilist[MAX_MULTIS][ARR_SIZE];
char multiliststr[ARR_SIZE];
char other_user[ARR_SIZE];
int u=-1;
char prefix[SAYNAME_LEN+10];

for (i=0;i<MAX_MULTIS;++i) { multilist[i][0]=0; multilistnums[i]=-1; }
multiliststr[0]=0;
i=0;

if (ustr[user].gagcomm) {
   write_str(user,NO_COMM);
   return;
   }

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

i=0;
point=0;
point2=0;
gotchar=0;
        
if (!strlen(inpstr)) 
  {
   write_str(user,"Who are you thinking about?");  
	for (i=0;i<MAX_MULTIS;++i) { multilist[i][0]=0; } 
	multiliststr[0]=0;
   return;
  }

tells++; 

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

if ((u=get_user_num(other_user,user))== -1) 
  {
   if (!read_user(other_user)) {
      write_str(user,NO_USER_STR);
      if (!multi) return;
      else continue;
      }
   not_signed_on(user,t_ustr.say_name);
if (user_wants_message(user,FAILS)) {
   sprintf(mess,"%s",t_ustr.fail);
   write_str(user,mess);
   }
      if (!multi) return;
      else continue;
  }

if (!check_gag(user,u,0)) {
      if (!multi) return;
      else continue;
   }

if (ustr[u].pro_enter || ustr[u].vote_enter || ustr[u].roomd_enter) {
    write_str(user,IS_ENTERING);
      if (!multi) return;
      else continue;
    }

if (ustr[u].afk)
  {
    if (ustr[u].afk == 1) {
      if (!strlen(ustr[u].afkmsg))
       sprintf(t_mess,"- %s is Away From Keyboard -",ustr[u].say_name);
      else
       sprintf(t_mess,"- %s %-45s -(A F K)",ustr[u].say_name,ustr[u].afkmsg);
      }
     else {
      if (!strlen(ustr[u].afkmsg))
      sprintf(t_mess,"- %s is blanked AFK (is not seeing this) -",ustr[u].say_name);
      else
      sprintf(t_mess,"- %s %-45s -(B A F K)",ustr[u].say_name,ustr[u].afkmsg);
      }

    write_str(user,t_mess);
  }

if (!user_wants_message(u,TELLS) && ustr[user].tempsuper<WIZ_LEVEL) 
  {
   sprintf(mess,"%s is ignoring tells, semotes, and sthinks",ustr[u].say_name);
   write_str(user,mess);
   if (user_wants_message(user,FAILS)) write_str(user,ustr[u].fail);
      if (!multi) return;
      else continue;
  }
  
/* check if this user is already in the list */
/* we're gonna reuse some ints here          */
for (point2=0;point2<MAX_MULTIS;++point2) {
        if (multilistnums[point2]==u) { gotchar=1; break; }
   }
point2=0;
if (gotchar) {
  gotchar=0;
  continue;
  }   
    
/* it's ok to send the tell to this user, add them to the multistr */
/* add this user to the list for our next loop */
multilistnums[point]=u;
point++;
} /* end of user for */
i=0;

/* no multilistnums, must be all bad users */
if (!point) {
        return;
  } 

/* loop to compose the messages and print to the users */
for (i=0;i<point;++i) {
u=multilistnums[i];

count=0;
point2=0;
multiliststr[0]=0;
/* make multi string to send to this user */
for (point2=0;point2<point;++point2) {
/* dont send recipients name to themselves */
if (u==multilistnums[point2]) continue;
else count++;
if (count>0)
 strcat(multiliststr,",");
/* add their name to the output string */
if (!ustr[multilistnums[point2]].vis)
 strcat(multiliststr,INVIS_ACTION_LABEL);
else
 strcat(multiliststr,ustr[multilistnums[point2]].say_name);
}

if ((ustr[u].monitor==1) || (ustr[u].monitor==3))
  {
    strcpy(prefix,"<");
    strcat(prefix,ustr[user].say_name);
    strcat(prefix,"> ");
  }
 else
  {
   prefix[0]=0;
  }

if (ustr[user].frog) strcpy(inpstr,"I'm a frog, I'm a frog!");

/* write to user being told */
if (ustr[user].vis) {
  if (!multi)
   sprintf(mess,"--> %s thinks . o O ( %s )", ustr[user].say_name, inpstr);
  else
   sprintf(mess,"--> %s thinks (To you%s) . o O ( %s )", ustr[user].say_name,multiliststr,inpstr);
  }
else {
  if (!multi)
   sprintf(mess,"%s--> %s thinks . o O ( %s )",prefix,INVIS_ACTION_LABEL,inpstr); 
  else
   sprintf(mess,"%s--> %s thinks (To you%s) . o O ( %s )",prefix,INVIS_ACTION_LABEL,multiliststr,inpstr); 
  }

if (ustr[u].beeps) {
 if (user_wants_message(u,BEEPS))
  strcat(mess,"\07");
 }

/*-----------------------------------*/
/* store the sthink in the rev buffer*/
/*-----------------------------------*/
/* moved because of multi tells */
/* CYGNUS2 */
addto_conv_buffer(ustr[u].conv, strip_color(mess));

if (ustr[u].hilite==2)
 write_str(u,mess);
else {
 strcpy(mess, strip_color(mess));
 write_hilite(u,mess);
 }

} /* end of message compisition for loop */

if (multi) {
point2=0;
multiliststr[0]=0;
/* make multi string to send to this user */
for (point2=0;point2<point;++point2) {
/* dont send recipients name to themselves */
if (point2>0)
 strcat(multiliststr,",");
/* add their name to the output string */
if (!ustr[multilistnums[point2]].vis)
 strcat(multiliststr,INVIS_ACTION_LABEL);
else
 strcat(multiliststr,ustr[multilistnums[point2]].say_name);
}
} /* end of if multi */

/* write to teller */
if (!multi)
 sprintf(mess,"You thought to %s: %s",ustr[u].say_name,inpstr);
else
 sprintf(mess,"You thought to %s: %s",multiliststr,inpstr);

write_str(user,mess);
if (!multi) {
if (user_wants_message(user,SUCCS) && strlen(ustr[u].succ))
 write_str(user,ustr[u].succ);
}

/* CYGNUS2 */
addto_conv_buffer(ustr[user].conv, strip_color(mess));

i=0;
for (i=0;i<MAX_MULTIS;++i) { multilist[i][0]=0; }
multiliststr[0]=0;   

}


/** phone a user for a tell link **/
void call(int user, char *inpstr)
{
int u;
char other_user[ARR_SIZE];

if (!strlen(inpstr)) {
   write_str(user,"Who do you want to call?");
   return;
   }

if (!strcmp(inpstr,"clear") || !strcmp(inpstr,"Clear")) {
    ustr[user].phone_user[0]=0;
    write_str(user,"Your tell link has been disconnected.");
    return;
    }
if (strlen(inpstr) > NAME_LEN) {
   write_str(user,"Name too long.");
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

if ((u=get_user_num(other_user,user))== -1) 
  {
   if (!read_user(other_user)) {
      write_str(user,NO_USER_STR);
      return;
      }
   not_signed_on(user,t_ustr.say_name);
if (user_wants_message(user,FAILS)) {
   sprintf(mess,"%s",t_ustr.fail);
   write_str(user,mess);
   }
   return;
  }

if (!check_gag(user,u,0)) return;

if (ustr[u].afk)
  {
    if (ustr[u].afk == 1) {
      if (!strlen(ustr[u].afkmsg))
       sprintf(t_mess,"- %s is Away From Keyboard -",ustr[u].say_name);
      else
       sprintf(t_mess,"- %s %-45s -(A F K)",ustr[u].say_name,ustr[u].afkmsg);
      }
     else {
      if (!strlen(ustr[u].afkmsg))
      sprintf(t_mess,"- %s is blanked AFK (is not seeing this) -",ustr[u].say_name);
      else
      sprintf(t_mess,"- %s %-45s -(B A F K)",ustr[u].say_name,ustr[u].afkmsg);
      }

    write_str(user,t_mess);
    write_str(user,"Tell link was not established.");
    return;
  }

if (!user_wants_message(u,TELLS)) 
  {
   sprintf(mess,"%s is ignoring tells",ustr[u].say_name);
   write_str(user,mess);
   write_str(user,"Tell link not established.");
   return;
  }
  

strcpy(ustr[user].phone_user,strip_color(ustr[u].say_name));
sprintf(mess,"Tell link to %s established.\nUse .cr <mess> to reply to that user.",ustr[u].say_name);

write_str(user,mess);
write_str(user,"^HY*WARNING*^ If you are invis, the user you are telling will see your name");
sprintf(mess,"          instead of *%s*",INVIS_ACTION_LABEL);
write_str(user,mess);

}


/** reply to your phone partner **/
void creply(int user, char *inpstr)
{
int u;
char lowername[NAME_LEN+1];
char comstr[ARR_SIZE];

if (ustr[user].gagcomm) {
   write_str(user,NO_COMM);
   return;
   }

if (!strlen(inpstr)) {
   write_str(user,"What do you want to say?");
   return;
   }

if (strlen(ustr[user].phone_user) < 3) {
   write_str(user,"You don't have a tell link to anyone. Use  .call <user>  first."); 
   return;
   }

/* Copy phoned users name to temp arrays so we can display their name */
/* capitalized right and do checks at the same time                   */
strcpy(lowername,ustr[user].phone_user);
strtolower(lowername);

if ((u=get_user_num(lowername,user))== -1)
  {
   not_signed_on(user,ustr[user].phone_user);
   return;
  }

if (!check_gag(user,u,0)) return;

if (ustr[u].pro_enter || ustr[u].vote_enter || ustr[u].roomd_enter) {
    write_str(user,IS_ENTERING);
    return;
    }

if (ustr[u].afk)
  {
    if (ustr[u].afk == 1) {
      if (!strlen(ustr[u].afkmsg))
       sprintf(t_mess,"- %s is Away From Keyboard -",ustr[u].say_name);
      else
       sprintf(t_mess,"- %s %-45s -(A F K)",ustr[u].say_name,ustr[u].afkmsg);
      }
     else {
      if (!strlen(ustr[u].afkmsg))
      sprintf(t_mess,"- %s is blanked AFK (is not seeing this) -",ustr[u].say_name);
      else
      sprintf(t_mess,"- %s %-45s -(B A F K)",ustr[u].say_name,ustr[u].afkmsg);
      }

    write_str(user,t_mess);
  }

if (!user_wants_message(u,TELLS)) 
  {
   sprintf(mess,"%s is ignoring tells",ustr[u].say_name);
   write_str(user,mess);
   return;
  }
  
tells++;

if (ustr[user].frog) strcpy(inpstr,FROG_TALK);

/* write to user being told */
comstr[0]=inpstr[0];
comstr[1]=0;
if (comstr[0] == ustr[user].custAbbrs[get_emote(user)].abbr[0]) 
  {
  inpstr[0] = ' ';
  while(inpstr[0] == ' ') inpstr++;
  sprintf(mess,VIS_SEMOTES,ustr[user].say_name,inpstr);
  } 
else if (comstr[0] == '\'') {
  inpstr[0] = ' ';
  while(inpstr[0] == ' ') inpstr++;
  sprintf(mess,VIS_SEMOTES_P,ustr[user].say_name,inpstr);
  }
 else {   
   sprintf(mess,VIS_TELLS,ustr[user].say_name,inpstr);
   }

/* CYGNUS2 */
addto_conv_buffer(ustr[u].conv, strip_color(mess));

if (ustr[u].beeps) {
 if (user_wants_message(u,BEEPS))
  strcat(mess,"\07");
 }

if (ustr[u].hilite==2)
 write_str(u,mess);
else {
 strcpy(mess, strip_color(mess));
 write_hilite(u,mess);
 }

/* write to teller */
if (comstr[0] == '\'')
sprintf(mess,VISFROMSEMOTE_P,ustr[u].say_name,ustr[user].say_name,inpstr);
else if (comstr[0] == ustr[user].custAbbrs[get_emote(user)].abbr[0])
sprintf(mess,VISFROMSEMOTE,ustr[u].say_name,ustr[user].say_name,inpstr);
else
sprintf(mess,VIS_FROMLINK,ustr[u].say_name,inpstr);

write_str(user,mess);

/** store reply in the tell buffer **/
/* CYGNUS2 */
addto_conv_buffer(ustr[user].conv, strip_color(mess));

return;
}


/*** Directed say to a person in the same room ***/
void say_to_user(int user, char *inpstr)
{
int u;
int area = ustr[user].area;
char other_user[ARR_SIZE];
char comstr[ARR_SIZE];

if (!strlen(inpstr)) {
   write_str(user,"You must specify a user and a message.");
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

if ((u=get_user_num(other_user,user))== -1)
  {
   if (check_for_user(other_user) == 1) {
      not_signed_on(user,other_user);
      return;
     }
   else {
      write_str(user,NO_USER_STR);
      return;
     }
  }

if (!check_gag(user,u,0)) return;

if (strcmp(astr[area].name,astr[ustr[u].area].name)) {
   write_str(user,"User is not in this room");
   return;
   }

remove_first(inpstr);

if (!strlen(inpstr)) {
     write_str(user, "What do you want to say to them?");
     return;
     }

if (ustr[u].afk) {
   write_str(user,"User is AFK, wait until they come back.");
   return;
   }

if (ustr[user].frog) strcpy(inpstr,FROG_TALK);

        comstr[0]=inpstr[0];
        comstr[1]=0;

if (comstr[0] == ustr[user].custAbbrs[get_emote(user)].abbr[0])
  {
        inpstr[0] = ' ';
        while(inpstr[0] == ' ') inpstr++;

        comstr[0]=inpstr[0];
        comstr[1]=0;

     if (comstr[0] == '\'') {
        /* inpstr[0] = ' '; */
        while(inpstr[0] == ' ') inpstr++;
	if (!ustr[u].vis)
        sprintf(mess,VIS_DIREMOTE,ustr[user].say_name,inpstr,INVIS_ACTION_LABEL);
	else
        sprintf(mess,VIS_DIREMOTE,ustr[user].say_name,inpstr,ustr[u].say_name);
        }
    else {
     strcpy(comstr," ");
     strcat(comstr,inpstr);
     strcpy(inpstr,comstr);
     if (!ustr[u].vis)
     sprintf(mess,VIS_DIREMOTE,ustr[user].say_name,inpstr,INVIS_ACTION_LABEL);
     else
     sprintf(mess,VIS_DIREMOTE,ustr[user].say_name,inpstr,ustr[u].say_name);
     }
    
    write_str(user,mess);
   if (!ustr[user].vis) {
     if (comstr[0] == '\'') {
	/*
        inpstr[0] = ' ';
        while(inpstr[0] == ' ') inpstr++;
	*/
	if (!ustr[u].vis)
        sprintf(mess,INVIS_DIREMOTE,INVIS_ACTION_LABEL,inpstr,INVIS_ACTION_LABEL);
	else
        sprintf(mess,INVIS_DIREMOTE,INVIS_ACTION_LABEL,inpstr,ustr[u].say_name);
        }
      else {
	if (!ustr[u].vis)
        sprintf(mess,INVIS_DIREMOTE,INVIS_ACTION_LABEL,inpstr,INVIS_ACTION_LABEL);
	else
        sprintf(mess,INVIS_DIREMOTE,INVIS_ACTION_LABEL,inpstr,ustr[u].say_name);
	}
    } /* end of !vis */
  } /* end of if emote */
else {
  if (!ustr[u].vis)
  sprintf(mess,VIS_DIRECTS,ustr[user].say_name,get_reaction(inpstr),INVIS_ACTION_LABEL,inpstr);
  else
  sprintf(mess,VIS_DIRECTS,ustr[user].say_name,get_reaction(inpstr),ustr[u].say_name,inpstr);
  write_str(user,mess);

  if (!ustr[user].vis) {
    if (!ustr[u].vis)
    sprintf(mess,INVIS_DIRECTS,INVIS_TALK_LABEL,get_reaction(inpstr),INVIS_ACTION_LABEL,inpstr);
    else
    sprintf(mess,INVIS_DIRECTS,INVIS_TALK_LABEL,get_reaction(inpstr),ustr[u].say_name,inpstr);
    }
}

  writeall_str(mess,1,user,0,user,NORM,SAY_TYPE,0);

says++;

/*--------------------------------*/
/* store say to the review buffer */
/*--------------------------------*/
  addto_conv_buffer(astr[area].conv, strip_color(mess));

}


/*** emote func used for expressing emotional or visual stuff ***/
void semote(int user, char *inpstr)
{
int point=0,count=0,i=0,lastspace=0,lastcomma=0,gotchar=0;
int point2=0,multi=0;
int multilistnums[MAX_MULTIS];
char multilist[MAX_MULTIS][ARR_SIZE];
char multiliststr[ARR_SIZE];
int u=-1;
char prefix[SAYNAME_LEN+10];
char other_user[ARR_SIZE];
char comstr[ARR_SIZE];
char *other_input='\0';

for (i=0;i<MAX_MULTIS;++i) { multilist[i][0]=0; multilistnums[i]=-1; }
multiliststr[0]=0;
i=0;

if (ustr[user].gagcomm) {
   write_str(user,NO_COMM);
   return;
   }

if (!strlen(inpstr)) 
  {
   write_str(user,"Secret emote who what?");  
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

if (!strlen(inpstr)) {
	write_str(user,"What do you want to secret emote to them?");
	return;
   }

i=0;
point=0;
point2=0;
gotchar=0;

if ((strstr(inpstr,"ARRIVING")) || (strstr(inpstr,"LEAVING")) ) {
   write_str(user,"You cant semote that.");
   for (i=0;i<MAX_MULTIS;++i) { multilist[i][0]=0; multilistnums[i]=-1; }
   multiliststr[0]=0;
   return;
   }

tells++;
        
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

if ((u=get_user_num(other_user,user))== -1) 
  {
   if (!read_user(other_user)) {
      write_str(user, NO_USER_STR);
      if (!multi) return;
      else continue;
      }
   not_signed_on(user,t_ustr.say_name);
if (user_wants_message(user,FAILS)) {
   sprintf(mess,"%s",t_ustr.fail);
   write_str(user,mess);
   }
	if (!multi) return;
	else continue;
  }

if (!check_gag(user,u,0)) {
	if (!multi) return;
	else continue;
  }

if (ustr[u].pro_enter || ustr[u].vote_enter || ustr[u].roomd_enter) {
    write_str(user,IS_ENTERING);
    if (!multi) return;
    else continue;
    }

if (ustr[u].afk)
  {
    if (ustr[u].afk == 1) {
      if (!strlen(ustr[u].afkmsg))
       sprintf(t_mess,"- %s is Away From Keyboard -",ustr[u].say_name);
      else
       sprintf(t_mess,"- %s %-45s -(A F K)",ustr[u].say_name,ustr[u].afkmsg);
      }
     else {
      if (!strlen(ustr[u].afkmsg))
      sprintf(t_mess,"- %s is blanked AFK (is not seeing this) -",ustr[u].say_name);
      else
      sprintf(t_mess,"- %s %-45s -(B A F K)",ustr[u].say_name,ustr[u].afkmsg);
      }

    write_str(user,t_mess);
  }

if (!user_wants_message(u,TELLS) && ustr[user].super<WIZ_LEVEL) 
  {
   sprintf(mess,"%s is ignoring tells and secret emotes",ustr[u].say_name);
   write_str(user,mess);
   if (user_wants_message(user,FAILS)) write_str(user,ustr[u].fail);
   if (!multi) return;
   else continue;
  }

/* check if this user is already in the list */
/* we're gonna reuse some ints here          */
for (point2=0;point2<MAX_MULTIS;++point2) {
        if (multilistnums[point2]==u) { gotchar=1; break; }
   }
point2=0;
if (gotchar) {
  gotchar=0;
  continue;
  }

/* it's ok to send the tell to this user, add them to the multistr */
/* add this user to the list for our next loop */
multilistnums[point]=u;
point++;
} /* end of user for */
i=0;
  
/* no multilistnums, must be all bad users */
if (!point) {
        return;
  }

/* loop to compose the messages and print to the users */
for (i=0;i<point;++i) {

u=multilistnums[i];

count=0;
point2=0;
multiliststr[0]=0;
/* make multi string to send to this user */
for (point2=0;point2<point;++point2) {
/* dont send recipients name to themselves */
if (u==multilistnums[point2]) continue;
else count++;
if (count>0)
 strcat(multiliststr,",");
/* add their name to the output string */
if (!ustr[multilistnums[point2]].vis)
 strcat(multiliststr,INVIS_ACTION_LABEL);
else
 strcat(multiliststr,ustr[multilistnums[point2]].say_name);
}

other_input='\0';
other_input=inpstr;

if ((ustr[u].monitor==1) || (ustr[u].monitor==3))
  {
    strcpy(prefix,"<");
    strcat(prefix,ustr[user].say_name);
    strcat(prefix,"> ");
  }
 else
  {
   prefix[0]=0;
  }


/* write to user being told */
if (!strlen(inpstr)) {
if (ustr[user].vis) {
	sprintf(mess,"You sense that %s is looking for you in the %s",
ustr[user].say_name, astr[ustr[user].area].name);
       }
else {
  if ((ustr[u].monitor==1) || (ustr[u].monitor==3))
    {
      sprintf(prefix,"? %s",ustr[user].say_name);
    }
   else
    {
     strcpy(prefix,INVIS_TALK_LABEL);
    }
   sprintf(mess,"You sense that %s is looking for you.",prefix);
 }
}
else {
if (ustr[user].frog) strcpy(other_input,FROG_SEMOTE);

if (ustr[user].vis) {
   comstr[0]=other_input[0];
   comstr[1]=0;
   if (comstr[0] == '\'') {
        while(other_input[0] == '\'') { other_input++; }

	if (!multi)
   	 sprintf(mess,VIS_SEMOTES_P,ustr[user].say_name, other_input);
	else
   	 sprintf(mess,VIS_SEMOTE_MP,multiliststr,ustr[user].say_name,other_input);
       }
   else {
	if (!multi)
	 sprintf(mess,VIS_SEMOTES,ustr[user].say_name, other_input);
	else
	 sprintf(mess,VIS_SEMOTES_M,multiliststr,ustr[user].say_name,other_input);
    }
   }
 else {
   comstr[0]=other_input[0];
   comstr[1]=0;
   if (comstr[0] == '\'') {
      while(other_input[0] == '\'') other_input++;
      if (!multi)
       sprintf(mess,INVIS_SEMOTES_P,prefix,INVIS_ACTION_LABEL,other_input);
      else
       sprintf(mess,INVIS_SEMOTE_MP,prefix,multiliststr,INVIS_ACTION_LABEL,other_input);
     }
   else {
      if (!multi)
       sprintf(mess,INVIS_SEMOTES,prefix,INVIS_ACTION_LABEL,other_input);
      else
       sprintf(mess,INVIS_SEMOTES_M,prefix,multiliststr,INVIS_ACTION_LABEL,other_input);
   }
 }
}

if (ustr[u].beeps) {
 if (user_wants_message(u,BEEPS))
  strcat(mess,"\07");
 }

/*-----------------------------------*/
/* store the semote in the rev buffer*/
/*-----------------------------------*/
/* moved because of multi tells */
/* CYGNUS2 */
addto_conv_buffer(ustr[u].conv, strip_color(mess));

if (ustr[u].hilite==2)
 write_str(u,mess);
else {
 strcpy(mess, strip_color(mess));
 write_hilite(u,mess);
 }

  /* write to bot */
  if (u==bot) {
  sprintf(mess,"+++++ comm_semote:%s %s",strip_color(ustr[user].say_name),other_input);
  write_bot(mess);
  }

} /* end of message compisition for loop */

if (multi) {
point2=0;
multiliststr[0]=0; 
/* make multi string to send to this user */
for (point2=0;point2<point;++point2) {
/* dont send recipients name to themselves */
if (point2>0)
 strcat(multiliststr,",");
/* add their name to the output string */  
if (!ustr[multilistnums[point2]].vis)
 strcat(multiliststr,INVIS_ACTION_LABEL);
else
 strcat(multiliststr,ustr[multilistnums[point2]].say_name);
}
} /* end of if multi */

/* write to teller */
if (strlen(inpstr)) {
   if (inpstr[0] == '\'') {
      while(inpstr[0] == '\'') inpstr++;

    if (!ustr[user].vis) {
	if (!multi)
    sprintf(mess,VISFROMSEMOTE_P,ustr[u].say_name,INVIS_ACTION_LABEL,inpstr);
	else
    sprintf(mess,VISFROMSEMOTE_P,multiliststr,INVIS_ACTION_LABEL,inpstr);
    }
    else {
	if (!multi)
    sprintf(mess,VISFROMSEMOTE_P,ustr[u].say_name,ustr[user].say_name,inpstr);
	else
    sprintf(mess,VISFROMSEMOTE_P,multiliststr,ustr[user].say_name,inpstr);
    }
    }
   else {
    if (!ustr[user].vis) {
	if (!multi)
    sprintf(mess,VISFROMSEMOTE,ustr[u].say_name,INVIS_ACTION_LABEL,inpstr);
	else
    sprintf(mess,VISFROMSEMOTE,multiliststr,INVIS_ACTION_LABEL,inpstr);
    }
    else {
	if (!multi)
    sprintf(mess,VISFROMSEMOTE,ustr[u].say_name,ustr[user].say_name,inpstr);
	else
    sprintf(mess,VISFROMSEMOTE,multiliststr,ustr[user].say_name,inpstr);
    }
   }
 }
else {
	if (!multi)
 sprintf(mess,"Telepathic message sent to %s.",ustr[u].say_name);
	else
 sprintf(mess,"Multi-Telepathic message sent to %s.",ustr[u].say_name);
 }
write_str(user,mess);

if (!multi) {
if (user_wants_message(user,SUCCS) && strlen(inpstr) && strlen(ustr[u].succ))
write_str(user,ustr[u].succ);
}

/* CYGNUS2 */
addto_conv_buffer(ustr[user].conv, strip_color(mess));

i=0;
for (i=0;i<MAX_MULTIS;++i) { multilist[i][0]=0; }
multiliststr[0]=0;

}


/** think function for EW-too heads **/
void think(int user, char *inpstr)
{

sprintf(t_mess,"thinks . o O ( %s )",inpstr);
strcat(t_mess,"\0");
emote(user,t_mess);

}


/*** sos - request help from all logged in wizards ***/
void sos(int user, char *inpstr)
{
int new_pos = 0;

if (!ustr[user].shout)
  {
   write_str(user,NO_SHOUT_SOS);
   return;
  }

if (!strlen(inpstr))
  {
   write_str(user,"What do you want to ask all the wizards?");
   return;
  }

if (ustr[user].frog) strcpy(inpstr,FROG_TALK);

sprintf(mess,"<SOS> from %s: %s",ustr[user].say_name,inpstr);
strcpy(mess, strip_color(mess));
write_hilite(user,mess);
writeall_str(mess, WIZ_ONLY, user, 0, user, BOLD, NONE, 0);
write_log(SYSTEMLOG,YESTIME,"SOS: by %s, \"%s\"\n",ustr[user].say_name,mess);

/*---------------------------*/
/* store the sos in the buff */
/*---------------------------*/

strncpy(bt_conv[bt_count],mess,MAX_LINE_LEN);
new_pos = ( ++bt_count ) % NUM_LINES;
bt_count = new_pos;
}


/*--------------------------------------------------------*/
/* Function used for expressing emotional or visual stuff */
/*--------------------------------------------------------*/
void emote(int user, char *inpstr)
{ 
int area;
char comstr[ARR_SIZE];
        
if (!strlen(inpstr))
  {
   write_str(user,"Emote what?");
   return;
  }
 
if (ustr[user].frog == 1) inpstr = FROG_EMOTE;

if ((strstr(inpstr,"ARRIVING")) || (strstr(inpstr,"LEAVING")) ) {
   write_str(user,"You cant emote that.");
   return;
   }

if (!ustr[user].vis) {
   comstr[0]=inpstr[0];
   comstr[1]=0;
   if (comstr[0] == '\'') {
        inpstr[0] = ' ';
        while(inpstr[0] == ' ') inpstr++;
        sprintf(mess,"%s\'%s",INVIS_ACTION_LABEL,inpstr);
        }
    else
       sprintf(mess,"%s %s",INVIS_ACTION_LABEL,inpstr);
 }
 else {
   comstr[0]=inpstr[0];
   comstr[1]=0;
   if (comstr[0] == '\'') {
        inpstr[0] = ' ';
        while(inpstr[0] == ' ') inpstr++;
        sprintf(mess,"%s\'%s",ustr[user].say_name,inpstr);
        }
    else
       sprintf(mess,"%s %s",ustr[user].say_name,inpstr);
 }
         
/* write output */
write_str(user,mess);
writeall_str(mess, 1, user, 0, user, NORM, SAY_TYPE, 0);
 
/*-----------------------------------*/
/* store the emote in the rev buffer */
/*-----------------------------------*/
        
area = ustr[user].area;
  addto_conv_buffer(astr[area].conv, strip_color(mess));  
         
  /* write to bot */
  if (ustr[user].area == ustr[bot].area) {
  sprintf(mess,"+++++ comm_emote:%s %s",strip_color(ustr[user].say_name),inpstr);
  write_bot(mess);
  }

}


/*--------------------------------------------------------*/
/* Say a message to everyone in room but specified user   */
/*--------------------------------------------------------*/
void mutter(int user, char *inpstr)
{  
int u;
char other_user[ARR_SIZE];
char comstr[ARR_SIZE];
  
if (!strlen(inpstr)) {
   write_str(user,"You must specify a user and a message.");
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
   write_str(user,"Mutter to everyone but yourself? Get real.");
   return;
   }

if ((u=get_user_num(other_user,user))== -1)
  {
   if (check_for_user(other_user) == 1) {
      not_signed_on(user,other_user);
      return;
     }
   else {
      write_str(user,NO_USER_STR);
      return;
     }
  }
   
if (strcmp(ustr[u].name,other_user)) {
   write_str(user,NO_USER_STR);
   return;
   }
   
if (strcmp(astr[ustr[user].area].name,astr[ustr[u].area].name)) {
   write_str(user,"User is not in this room");
   return;   
   }  
   
strcpy(ustr[user].mutter,ustr[u].say_name);
remove_first(inpstr);
      
if (ustr[user].frog) strcpy(inpstr,FROG_TALK);
   
        comstr[0]=inpstr[0];
        comstr[1]=0;
   
if (comstr[0] == ustr[user].custAbbrs[get_emote(user)].abbr[0])
  {
        comstr[0]=inpstr[1];
        comstr[1]=0;
   
     if (comstr[0] == '\'') {
        inpstr[0] = ' ';
        while(inpstr[0] == ' ') inpstr++;  
        sprintf(mess,VIS_MUTEMOTE,ustr[user].say_name,inpstr,ustr[user].mutter);
        }
    else {
     inpstr[0] = ' ';
     sprintf(mess,VIS_MUTEMOTE,ustr[user].say_name,inpstr,ustr[user].mutter);
     }
   
    write_str(user,mess);
   if (!ustr[user].vis) {
     if (comstr[0] == '\'') {
        inpstr[0] = ' ';
        while(inpstr[0] == ' ') inpstr++;
        sprintf(mess,INVIS_MUTEMOTE,INVIS_ACTION_LABEL,inpstr,ustr[user].mutter);
        }
      else
        inpstr[0] = ' ';
        sprintf(mess,INVIS_MUTEMOTE,INVIS_ACTION_LABEL,inpstr,ustr[user].mutter);
    } /* end of !vis */
  } /* end of if emote */
else {
    sprintf(mess,VIS_MUTTERS,ustr[user].say_name,inpstr,ustr[user].mutter);
    write_str(user,mess);
   if (!ustr[user].vis)  
    sprintf(mess,INVIS_MUTTERS,INVIS_TALK_LABEL,inpstr,ustr[user].mutter);
  }
       
  writeall_str(mess,1,user,0,user,NORM,SAY_TYPE,0);
ustr[user].mutter[0]=0;
}


/*----------------------------------------------*/
/* Echo function writes straight text to screen */
/*----------------------------------------------*/
void my_echo(int user, char *inpstr)
{
char fword[ARR_SIZE];
char *err="Sorry - you cant echo that";
int u=0; 
int area;

if (ustr[user].gagcomm) {  
   write_str(user,NO_COMM);
   return;
   }
   
if (!strlen(inpstr))
  {
   write_str(user,"Echo what?");
   return;
  }

if (ustr[user].frog) strcpy(inpstr,FROG_ECHO);

/* get first word & check it for illegal words */
sscanf(inpstr,"%s",fword);
if (
    instr2(0,fword,"SYSTEM",0) != -1||
    instr2(0,fword,"***",0) != -1||
    instr2(0,fword,"myster",0) != -1 ||
    instr2(0,fword,"Someone",0) != -1||
    instr2(0,fword,"someone",0) != -1||
    instr2(0,fword,"[",0) != -1||
    instr2(0,fword,"-->",0) != -1)
    {
     write_str(user,err);
     return;
    }

if (strstr(fword,"^")) {  
    write_str(user,"System does not allow the hiliting of echos.");
    return;
   }
    
/* check for user names */
strtolower(fword);
    
for (u=0;u<MAX_USERS;++u) {
        if ((instr2(0,fword,ustr[u].name,0)!= -1) ||
            (instr2(0,fword,BOTS_ROOTID,0)!= -1)) {
                write_str(user,err);  return;
                }
        }

/* even check if user is not online */
if (check_for_user(fword)) {
  write_str(user,err);
  return;
  } 

/* write message */
strcpy(mess,inpstr);
mess[0]=toupper((int)mess[0]);
write_str(user,mess);
writeall_str(mess, 1, user, 0, user, NORM, ECHOM, 0);
                
/*-----------------------------------*/
/* store the echo in the rev buffer  */
/*-----------------------------------*/

area = ustr[user].area;
  addto_conv_buffer(astr[area].conv, strip_color(mess));
  
}


/*----------------------------------------------------------------------*/
/* Picture tell                                                         */
/*----------------------------------------------------------------------*/
void ptell(int user, char *inpstr)
{
int u;
char x_name[256],filename[FILE_NAME_LEN],temp[SAYNAME_LEN+80];
char other_user[ARR_SIZE];
 
if (ustr[user].gagcomm) {  
   write_str(user,NO_COMM);
   return;
   }
   
if (!strlen(inpstr)) {  
   strcpy(inpstr, "names");
   preview(user, inpstr);
   return;
   }
 
strcpy(filename,"*not found*");
sscanf(inpstr,"%s ",other_user);

/* plug security hole */
if (check_fname(other_user,user))
  {
   write_str(user,"Illegal name.");
   return;
  }

strtolower(other_user);
   
   if (!check_for_user(other_user)) {
     write_str(user,NO_USER_STR);
     return;
     }

if ((u=get_user_num(other_user,user))== -1)
  {
   not_signed_on(user,other_user);
   return;
   }
   
if (!check_gag(user,u,0)) return;

if (ustr[u].pro_enter || ustr[u].vote_enter || ustr[u].roomd_enter) {
    write_str(user,IS_ENTERING);
    return;
    }
     
if (ustr[u].afk)
  {
    if (ustr[u].afk == 1) {
      if (!strlen(ustr[u].afkmsg))
       sprintf(t_mess,"- %s is Away From Keyboard -",ustr[u].say_name);
      else
       sprintf(t_mess,"- %s %-45s -(A F K)",ustr[u].say_name,ustr[u].afkmsg);
      }
     else {
      if (!strlen(ustr[u].afkmsg))
      sprintf(t_mess,"- %s is blanked AFK (is not seeing this) -",ustr[u].say_name);
      else
      sprintf(t_mess,"- %s %-45s -(B A F K)",ustr[u].say_name,ustr[u].afkmsg);
      }
     
    write_str(user,t_mess);
  }
    
remove_first(inpstr);
inpstr[80]=0;
sscanf(inpstr,"%s",filename);

if (!strlen(filename))
  {
   write_str(user,"Show what picture?");
   return;
  }
      
/* plug security hole */
if (check_fname(filename,user))
  {
   sprintf(mess,"Illegal file name: %s",filename);
   write_str(user,mess);
   return;
  }

sprintf(t_mess,"%s/%s",PICTURE_DIR,filename);
strncpy(x_name,t_mess,256);
   
if (!check_for_file(x_name))
  {
   write_str(user,"Picture does not exist.");
   return;
  }

if (!user_wants_message(u,PICTURE)) {
  sprintf(temp,"%s is ignoring pictures. Not sent.",ustr[u].say_name);
  write_str(user,temp); 
  }
else {
  sprintf(temp,"Picture request sent to %s",ustr[u].say_name);
  write_str(user,temp);

  sprintf(temp,"Private picture sent by: %s ",ustr[user].say_name);
  write_str(u,temp);
   
  cat(x_name,u,0);

  }
}


/*-------------------*/
/* Staff/Wizard tell */
/*-------------------*/
void btell(int user, char *inpstr)
{
char line[ARR_SIZE];
char comstr[ARR_SIZE];
int pos = bt_count%NUM_LINES;
int new_pos = 0;
int f;
 
if (user==-1)
  {
   return;
  }
  
if (!strlen(inpstr))
  {
    write_str(user,"Review wiztells:");
  
    for (f=0;f<NUM_LINES;++f)
      {
        if ( strlen(bt_conv[pos]) )
         {
          write_str(user,bt_conv[pos]);
         }
	new_pos = ( ++pos ) % NUM_LINES;
	pos = new_pos;
      }

    write_str(user,"<Done>");
    return;
  }

if (ustr[user].frog) strcpy(inpstr,FROG_TALK);
  
comstr[0]=inpstr[0];
comstr[1]=0;
        
if (comstr[0] == ustr[user].custAbbrs[get_emote(user)].abbr[0])
  {
  inpstr[0] = ' ';
  while(inpstr[0] == ' ') inpstr++;
  sprintf(line,"%s %s %s",STAFF_PREFIX,ustr[user].say_name,inpstr);
  }
else if (comstr[0] == '\'') {
  inpstr[0] = ' ';
  while(inpstr[0] == ' ') inpstr++;
  sprintf(line,"%s %s\'%s",STAFF_PREFIX,ustr[user].say_name,inpstr);
  }
 else
  sprintf(line,"%s %s: %s",STAFF_PREFIX,ustr[user].say_name,inpstr);

strcpy(line, strip_color(line));
write_hilite(user,line);
   
writeall_str(line, WIZ_ONLY, user, 0, user, BOLD, WIZT, 0);
  
/*-------------------------------------*/
/* store the wiztell in the rev buffer */
/*-------------------------------------*/
  
strncpy(bt_conv[bt_count],line,MAX_LINE_LEN);
new_pos = ( ++bt_count ) % NUM_LINES;
bt_count = new_pos;   
}



