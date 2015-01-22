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

/* EXTERNS */
extern char mess[ARR_SIZE+25];    /* functions use mess to send output   */
extern char t_mess[ARR_SIZE+25];  /* functions use t_mess as a buffer    */
extern int NUM_AREAS;          /* number of areas defined in config file */
extern char datadir[255];       /* config directory                       */
extern char area_nochange[MAX_AREAS];

/* FUNCTIONS */

/*** look decribes the surrounding scene **/
void look(int user, char *inpstr)
{
int f, i, u, area=0, new_area, found=0;
int occupied=0;
int spys=0;
char text[8];
char filename[FILE_NAME_LEN];

i = rand() % NUM_COLORS;

if (!strlen(inpstr))
 area=ustr[user].area;
else {
  if (strlen(inpstr) > 20) {
      write_str(user,"Room name is too long");  return;
      }
  for (new_area=0; new_area < NUM_AREAS; ++new_area)
       {
        if (! instr2(0, astr[new_area].name, inpstr, 0) )
          {
            found = TRUE;
            area = new_area;
           if (astr[new_area].hidden && ((ustr[user].tempsuper < GRIPE_LEVEL)
		|| (ustr[user].security[new_area]!='Y')))
             {
              write_str(user,"^Secured room, peeking not allowed.^");
              return;
             }
            break;
          }
       } /* end of for */
 if (!found)
        {  
         write_str(user, NO_ROOM);
         return;
        }
 } /* end of else */

write_str(user,"^HG+-------------------------------------------+^");

if (found)
 strcpy(text,"Looking");
else
 strcpy(text,"You are");

if (!strcmp(astr[area].name,HEAD_ROOM))
    sprintf(mess,"| %s in %-20.20s           |",text,HEAD_ROOM);
else if (!strcmp(astr[area].name,"hideaway"))
    sprintf(mess,"| %s in hideaway                       |",text);
else if (!strcmp(astr[area].name,"sky_palace"))
    sprintf(mess,"^HG| %s in Cygnus's sky palace         |^",text);
else if (astr[area].hidden) {
    sprintf(mess,"^HG|^ %s in the secure room %2.2d             ^HG|^",text,area);
    }
else
    sprintf(mess,"^HG|^ %s in the ^HY%-20.20s       ^^HG|^",text,astr[area].name);
write_str(user,mess);
write_str(user,"^HG+-------------------------------------------+^");

/* open and read room description file */
if (user_wants_message(user,ROOMD)) {
  sprintf(t_mess,"%s/%s",datadir,astr[area].name);
  strncpy(filename,t_mess,FILE_NAME_LEN);
  cat(filename,user,0);
}

/* show exits from room */
write_str(user,"");
if (found)
 write_str_nr(user,"Exits from there are : ");
else
 write_str_nr(user,"You can go to the : ");

     write_str_nr(user,color_text[i]);
for (f = 0; f < strlen( astr[area].move ); ++f) 
  {
   if (!astr[astr[area].move[f]-'A'].hidden)
      {
       write_str_nr(user,astr[ astr[area].move[f]-'A' ].name);
       write_str_nr(user,"  ");
      }
  }
       write_str_nr(user,"^");
	
write_str(user,"");
for (u=0; u<MAX_USERS; ++u) 
  {
   if (ustr[u].area != area || u == user) 
     continue;

   if ((!occupied) && (!spys)) {     
    if (found)
      write_str(user,"^HYLook who's there!:^");
    else
      write_str(user,"^HYLook who's here!:^");
     }

     if (!strlen(ustr[u].afkmsg))
      sprintf(mess,"      %s %s",ustr[u].say_name,ustr[u].desc);
     else
      sprintf(mess,"      %s %s",ustr[u].say_name,ustr[u].afkmsg);

   if (ustr[u].afk) { 
      strcat(mess,"  ^HR(afk)^");
     }
   else if (ustr[u].pro_enter) { 
      strcat(mess,"  ^HR(profile)^");
     }     
   else if (ustr[u].vote_enter) { 
      strcat(mess,"  ^HR(vote topic)^");
     }
   else if (ustr[u].roomd_enter) { 
      strcat(mess,"  ^HR(room desc)^");
     }
   if (!ustr[u].vis) {
      strcat(mess,"  ^HR(invis)^");
      if (ustr[user].tempsuper >= MIN_HIDE_LEVEL) {
        write_str(user,mess);
        occupied++;
        }
      else spys++;
     } 
   else {
     write_str(user,mess);
     occupied++;
    }
  } /* end of for user loop */

/* There are either no users at all or invis users */
if (!occupied) {
 write_str(user," ");
  if (spys) {
    sprintf(t_mess,VIS_IN_HERE,spys == 1 ? "is" : "are",spys,spys == 1 ? "" : "s");
    write_str(user,t_mess);
    }
  else {
  if (found)
   write_str(user,"There is no one there");
  else
   write_str(user,"There is no one here");
  }
 write_str(user," ");
  }
else {
 if (spys) {
  sprintf(t_mess,VIS_IN_HERE,spys == 1 ? "is" : "are",spys,spys == 1 ? "" : "s");
  write_str(user,t_mess);
  }
 write_str(user," "); /* users in the room */
 }

f=0;
strcpy(t_mess,"The room is set to ");

for (f = 0; f < strlen(area_nochange); ++f)
  {
   if (area_nochange[f] == area+65)
     {
       strcpy(t_mess,"The room is ^HClocked^ to ");
       break;
      }
  }

write_str_nr(user,t_mess);
if ( astr[area].private ) { 
   write_str_nr(user,"^HRprivate^");
  } 
else { 
   write_str_nr(user,"^HYpublic^");
  }

  sprintf(mess," and there %s ^HM%d^ message%s",astr[area].mess_num == 1 ? "is" : "are",astr[area].mess_num
              ,astr[area].mess_num == 1 ? "" : "s");

  write_str(user,mess);

if (user_wants_message(u,TOPIC)) {
 if (!strlen(astr[area].topic)) {
    if (found)
      write_str(user,"There is ^LRno^ current topic there");
    else
      write_str(user,"There is ^LRno^ current topic here");
    }
 else {
   sprintf(mess,"The current topic is : %s",astr[area].topic);
   write_str(user,mess);
   }
 } /* end of if user wants topic */

}


/*** gives current status of rooms */
void rooms(int user, char *inpstr)
{
int area;
int totl_hide;
int showhide=0;
int i,j=1;
char pripub[2][8];
char cbe[3];
char atm[3];
char filename[FILE_NAME_LEN];
FILE *fp;

if (!strlen(inpstr)) showhide=0;
else if (!strcmp(inpstr,"-h")) {
  if (ustr[user].tempsuper < ROOMVIS_LEVEL) {
    write_str(user,"You don't have that much power");
    return;
    }
  showhide=1;
  }
else {
  write_str(user,"Invalid option.");
  write_str(user,"Usage: .rooms [-h]");
  return;
  }

strcpy(pripub[0],"public");
strcpy(pripub[1],"private");
strcpy(cbe," ");
strcpy(atm," ");

totl_hide = 0;

 strcpy(filename,get_temp_file());
 if (!(fp=fopen(filename,"w"))) {
     write_str(user,BAD_FILEIO);
     write_log(ERRLOG,YESTIME,"Couldn't open tempfile(w) in rooms! %s\n",get_error());
     return;
     }

fputs("------------------------------------------------------------------------------\n",fp);
fputs("At   Occupied Rooms:          Usrs  Msgs  Topic\n",fp);
fputs("------------------------------------------------------------------------------\n",fp);
for (area=0;area<NUM_AREAS;++area) 
  {
   i = find_num_in_area(area);
   
   if (strchr(area_nochange, (char) area+65)== NULL)
     cbe[0]=' ';
    else 
     cbe[0]='*';

   if (astr[area].atmos)
     atm[0]='A';
    else
     atm[0]=' ';

   sprintf(mess,"%s%s %-*s : %-7s : %2d : %3d  ",  cbe,  atm,
                                                  ROOM_LEN,astr[area].name,
                                                  pripub[astr[area].private],
                                                  i,astr[area].mess_num);
                                              
   if (!strlen(astr[area].topic)) 
     strcat(mess,"<no topic>\n");
   else {
     strcat(mess,astr[area].topic);
     strcat(mess,"\n");
     }
     
   mess[0]=toupper((int)mess[0]);

if (!showhide) {   
   if (!astr[area].hidden )
     {
      if ( i ) fputs(mess,fp);
     }
    else
     totl_hide++;
   }
else {
   if (astr[area].hidden )
     {
      if ( i ) fputs(mess,fp);
      totl_hide++;
     }
   }
  } /* end of for */
  
fputs("\n",fp);
fputs("------------------------------------------------------------------------------\n",fp);
fputs("Other Rooms: (with number of messages)\n",fp);
fputs("------------------------------------------------------------------------------\n",fp);
  
for (area=0;area<NUM_AREAS;++area) 
  {
   i = find_num_in_area(area);  
    
   if (strchr(area_nochange, (char) area+65) == NULL) {
      if (astr[area].private)
       cbe[0]='P';
      else
       cbe[0]=' ';
    }
    else {
     cbe[0]='*';
     }
   
   if (astr[area].atmos)
    atm[0]='A';
   else
    atm[0]=' ';

   sprintf(mess,"%*s(%s%s%3d) ",ROOM_LEN,astr[area].name,cbe,atm,astr[area].mess_num);
   mess[0]=toupper((int)mess[0]);

 if (!showhide) {   
   if (!astr[area].hidden)
     {
      if (!i)
        {
         fputs(mess,fp);
	 if (!(j++%3) )
	   {j = 1;
	    fputs("\n",fp);
	   }
	}
     }
   }
 else {
   if (astr[area].hidden)
     {
      if (!i)
        {
         fputs(mess,fp);
	 if (!(j++%3) )
	   {j = 1;
	    fputs("\n",fp);
	   }
	}
     }
  }
 } /* end of for */
fputs(" \n \n",fp);

if (!showhide)
 sprintf(mess,"Currently there %s %d room%s\n",(NUM_AREAS-totl_hide)==1?"is":"are",
NUM_AREAS-totl_hide,(NUM_AREAS-totl_hide)==1?"":"s");
else
 sprintf(mess,"Currently there %s %d hidden room%s\n",totl_hide==1?"is":"are",
totl_hide,totl_hide==1?"":"s");

fputs(mess,fp);
fclose(fp);

 if (!cat(filename,user,0)) {
     write_str(user,BAD_FILEIO);
     write_log(ERRLOG,YESTIME,"Couldn't cat file \"%s\" in rooms! %s\n",filename,get_error());
     }

return;
}


/*----------------------------------------------------------------------*/
/* Add or remove security clearance for a user for a room               */
/*----------------------------------------------------------------------*/
void permission_u(int user, char *inpstr)
{
int u,a,b,j=1;
char permit;
char permission[ARR_SIZE];
char other_user[ARR_SIZE];
char room[ARR_SIZE];
char room_check[ARR_SIZE];
char filename[FILE_NAME_LEN];
FILE *fp;

if (!strlen(inpstr)) {
   write_str(user,"Usage: .permission <user> <add|sub> <room>"); 
   write_str(user,"Leave everything after <user> blank for permission list"); 
   return;
   }
sscanf(inpstr,"%s ",other_user);

if (!strlen(other_user)) {
   write_str(user,"Whose security clearance are we talking about?"); 
   return;
   }

strtolower(other_user);
CHECK_NAME(other_user);

remove_first(inpstr);

/* List room permissions */
if (!strlen(inpstr)) {

    if (!read_user(other_user))
     {
      write_str(user,NO_USER_STR);
      return;
     }

 strcpy(filename,get_temp_file());
 if (!(fp=fopen(filename,"w"))) {
     write_str(user,BAD_FILEIO);
     write_log(ERRLOG,YESTIME,"Couldn't open tempfile(w) in permission_u! %s\n",get_error());
     return;
     }

    fputs("---------------------------------------------------------\n",fp);
    fprintf(fp," Room permissions for %s\n",t_ustr.say_name);
    fputs("---------------------------------------------------------\n",fp);
    fputs("^HGSPECIAL ACCESS ROOMS:^\n",fp);

    for (a=0;a<NUM_AREAS;++a) 
      {
	if (t_ustr.security[a]=='Y' && strlen(astr[a].name))
	  {
	   sprintf(mess,"%*s ",ROOM_LEN,astr[a].name);
	   fputs(mess,fp);
	  }
	else continue;
	if (!(j++%3) )
           {
            j = 1;
            fputs("\n",fp);
	   }
      } /* end of for */
if (j!=1 && j<=3) fputs("\n",fp);
j=1;
a=0;

    fputs("^HYALLOWED ROOMS:^\n",fp);

    for (a=0;a<NUM_AREAS;++a) 
      {
	if (t_ustr.security[a]=='N' && !astr[a].hidden && strlen(astr[a].name))
	  {
	   sprintf(mess,"%*s ",ROOM_LEN,astr[a].name);
	   fputs(mess,fp);
	  }
	else continue;
	if (!(j++%3) )
           {
            j = 1;
            fputs("\n",fp);
	   }
      } /* end of for */
if (j!=1 && j<=3) fputs("\n",fp);
j=1;
a=0;

    fputs("^HRDENIED ROOMS:^\n",fp);
    for (a=0;a<NUM_AREAS;++a) 
      {
	if (t_ustr.security[a]=='X' && strlen(astr[a].name))
	  {
	   sprintf(mess,"%*s ",ROOM_LEN,astr[a].name);
	   fputs(mess,fp);
	  }
	else continue;
	if (!(j++%3) )
           {
            j = 1;
            fputs("\n",fp);
	   }
      } /* end of for */
if (j!=1 && j<=3) fputs("\n",fp);
   fputs("\n",fp);

fclose(fp);

 if (!cat(filename,user,0)) {
     write_str(user,BAD_FILEIO);   
     write_log(ERRLOG,YESTIME,"Couldn't cat file \"%s\" in permission_u! %s\n",filename,get_error());
     }

   a=0; j=1;
   return;
} /* end of permission list */
else {
sscanf(inpstr,"%s ",permission);
strtolower(permission);

if (!strcmp(permission,"add"))
  {
   permit = 'Y';
  }
else if (!strcmp(permission,"sub"))
  {
   permit = 'N';
  }
else
  {
   write_str(user,"Permission option does not exist.");
   return;
  }
} /* end of strlen else */

remove_first(inpstr);

if (!strlen(inpstr)) {
   write_str(user,"What room's security clearance do you want to change for that user?"); 
   return;
   }
 
sscanf(inpstr,"%s ",room);
strtolower(room);

if (!read_user(other_user))
  {
   write_str(user,NO_USER_STR);
   return;
  }

b= -1;

/* read in descriptions and joinings */
for (a=0;a<NUM_AREAS;++a) 
  {
    strncpy(room_check,astr[a].name,ROOM_LEN);
    strtolower(room_check);
    if (!strcmp(room,room_check))
      {
	if (permit=='Y') {
		if (t_ustr.security[a]=='X') {
			t_ustr.security[a]='N';
			write_str(user,"Security clearance changed from ^HRno^ access to ^HYnormal^ access");
		}
		else if (t_ustr.security[a]=='N') {
			t_ustr.security[a]='Y';
			write_str(user,"Security clearance changed from ^HYnormal^ access to ^HGspecial^ access");
		}
	} /* end of if permit */
	else if (permit=='N') {
		if (t_ustr.security[a]=='N') {
			t_ustr.security[a]='X';
			write_str(user,"Security clearance changed from ^HYnormal^ access to ^HRno^ access");
		}
		else if (t_ustr.security[a]=='Y') {
			t_ustr.security[a]='N';
			write_str(user,"Security clearance changed from ^HGspecial^ access to ^HYnormal^ access");
		}
	} /* end of else if permit */
       b=a;
       a=NUM_AREAS;
      }
  }
  
/* Do room check here, so it goes no further if room doesn't exist */
if (b == -1) {
   write_str(user,NO_ROOM);
   return;
   }

write_user(other_user);

if ((u=get_user_num(other_user,user))>-1) 
  {
	if (permit=='Y') {
		if (ustr[u].security[b]=='X') {
			ustr[u].security[b]='N';
			sprintf(mess,"%s has changed your security clearance from ^HRno^ access to ^HYnormal^ access for room %s",ustr[user].say_name,astr[b].name);
			write_str(u,mess);
		}
		else if (ustr[u].security[b]=='N') {
			ustr[u].security[b]='Y';
			sprintf(mess,"%s has changed your security clearance from ^HYnormal^ access to ^HGspecial^ access for room %s",ustr[user].say_name,astr[b].name);
			write_str(u,mess);
		}
	} /* end of if permit */
	else if (permit=='N') {
		if (ustr[u].security[b]=='N') {
			ustr[u].security[b]='X';
			sprintf(mess,"%s has changed your security clearance from ^HYnormal^ access to ^HRno^ access for room %s",ustr[user].say_name,astr[b].name);
			write_str(u,mess);
		}
		else if (ustr[u].security[b]=='Y') {
			ustr[u].security[b]='N';
			sprintf(mess,"%s has changed your security clearance from ^HGspecial^ access to ^HYnormal^ access for room %s",ustr[user].say_name,astr[b].name);
			write_str(u,mess);
		}
	} /* end of else if permit */
  } /* end of if user online */

}


/*** room_access sets room to private or public ***/
void room_access(int user, int priv)
{
int f,area=ustr[user].area;
char *noset="This rooms access cannot be set";
char pripub[2][15];
int spys = 0,u;

strcpy(pripub[0],"^HYpublic^");
strcpy(pripub[1],"^HRprivate^");

for (f = 0; f < strlen(area_nochange); ++f) 
  {
   if (area_nochange[f] == area+65) 
     {
       write_str(user,noset);  
       return;
      }
  }

/* see if access already set to user request */
if (priv==astr[area].private) 
  {
   sprintf(mess,"The room is already %s!",pripub[priv]);
   write_str(user,mess);  
   return;
  }

/* set to public */
if (!priv) 
  {
   write_str(user,"Room now set to ^HYpublic^");
   sprintf(mess,"%s has set the room to ^HYpublic^",ustr[user].say_name);
   
   if (!ustr[user].vis) 
     sprintf(mess,"%s has set the room to ^HYpublic^",INVIS_ACTION_LABEL);
     
   writeall_str(mess, 1, user, 0, user, NORM, NONE, 0);
	
   cbuff(user); 
   astr[area].private=0;
   return;
  }

/* need at least PRINUM people to set room to private unless u r superuser */
if ((find_num_in_area(area) < PRINUM) && ustr[user].tempsuper < PRIV_ROOM_RANK)
  {
   sprintf(mess,"You need at least %d people in the room",PRINUM);
   write_str(user,mess);
   return;
  };
  
write_str(user,"Room now set to ^HRprivate^");

for (u=0; u<MAX_USERS; ++u) 
 {
   if (ustr[u].area == area && !ustr[u].vis) spys++;
 }
   
sprintf(mess,"%s has set the room to ^HRprivate^",ustr[user].say_name);

if (!ustr[user].vis)
   sprintf(mess,"%s has set the room to ^HRprivate^",INVIS_ACTION_LABEL);

writeall_str(mess, 1, user, 0, user, NORM, NONE, 0);

if (spys)
  {
    sprintf(mess,VIS_IN_HERE,spys == 1 ? "is" : "are",spys,spys == 1 ? "" : "s");
    writeall_str(mess, 1, user, 0, user, BOLD, NONE, 0);
    write_str(user,mess);
  }
   
astr[area].private=1;
}

