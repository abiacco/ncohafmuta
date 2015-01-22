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

extern struct command_struct sys[];


/* Reset all user auto-forward limits to nill               */
/* Also check abbreviations to see if any have been deleted */
/* added, or changed                                        */
void reset_userfors(int startup)
{
int c=0;
int i=0;
int b=1;
int a=0;
int j=0;
int fixed=0;
int matched=0;
int add=0;
int changed=0;
int ret=0;
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
	sprintf(z_mess,"USERFORS: Can't open directory \"%s\" for reset_userfors! %s\n",filerid,get_error());

        write_log(ERRLOG,YESTIME,z_mess);

	if (startup)
	 printf("%s",z_mess);
      return;
   }

 while ((dp = readdir(dirp)) != NULL) 
   { 

    sprintf(small_buffer,"%s",dp->d_name);
        if (small_buffer[0] == '.')
         continue;
        else {
         strtolower(small_buffer);
         ret=read_user(small_buffer);
         if (ret==0) {
          sprintf(z_mess,"USERFORS: Can't open userfile \"%s\" Permission problem probably\n",small_buffer);
          write_log(ERRLOG,YESTIME,z_mess);

          if (startup)
           printf("%s",z_mess);
          continue;
         }
         else if (ret==-1) {
          sprintf(z_mess,"USERFORS: Can't open userfile \"%s\" 0 length file! Removed. Continuing..\n",small_buffer);
          write_log(ERRLOG,YESTIME,z_mess);

          if (startup)
           printf("%s",z_mess);
          continue;
         }
 
         t_ustr.automsgs = 0;
	 /* Check to make sure granted/revoked commands still exist */
	 for (j=0;j<MAX_GRAVOKES;++j) {
		if (strlen(t_ustr.revokes[j])) {
		  for (i=0;sys[i].jump_vector != -1;++i) {
			if (strip_com(t_ustr.revokes[j]) == sys[i].jump_vector) {
				changed=1; break;
				}
		     }
		if (!changed) t_ustr.revokes[j][0]=0;
		changed=0; i=0;
		}
	    }
	 j=0;
	 i=0;
	 changed=0;

          /* Check if abbreviations have changed */
         for (j=0;j<NUM_ABBRS;++j) {
         if (strlen(t_ustr.custAbbrs[j].abbr) > 1) {
          t_ustr.custAbbrs[j].abbr[0]=0;
          t_ustr.custAbbrs[j].com[0]=0;
          }
         else if (t_ustr.custAbbrs[j].com[0]!='.') {
          t_ustr.custAbbrs[j].abbr[0]=0;
          t_ustr.custAbbrs[j].com[0]=0;
          }
         } /* end of for */

    /*-----------------------------------------------------------------*/
    /* PART 1 - Check if number of abbreviations defined has decreased */
    /*-----------------------------------------------------------------*/
    i = NUM_ABBRS;
    if (strlen(t_ustr.custAbbrs[i].com) > 1 ) {
       i=0;
        while (strlen(t_ustr.custAbbrs[i].com)) {
          for (c=0; sys[c].su_com != -1 ;++c) {
             if (!strcmp(t_ustr.custAbbrs[i].com,sys[c].command)) {
                if (!strlen(sys[c].cabbr)) {
while (strlen(t_ustr.custAbbrs[i+b].com) > 0) {
 strcpy(t_ustr.custAbbrs[i+a].com,t_ustr.custAbbrs[i+b].com);
 strcpy(t_ustr.custAbbrs[i+a].abbr,t_ustr.custAbbrs[i+b].abbr);
 a++;
 b++;
 }
 
/* Make sure last lingering copy is cleared out */
t_ustr.custAbbrs[i+a].com[0]=0;
t_ustr.custAbbrs[i+a].abbr[0]=0;
a=0;
b=1;
                  fixed=1;
                  changed=1;
                  break;
                  }
                else { fixed=0; break; }
               }
            } /* end of for */
          c=0;
          if (!fixed) i++;
         } /* end of while */
      } /* end of main if */

 c=0;
 i=0;
 fixed=0;

    /*---------------------------------------------------------*/
    /* PART 2 - Check if these were any abbreviation additions */
    /*---------------------------------------------------------*/
    for (i=0;i<NUM_ABBRS;++i) {
        if (strlen(t_ustr.custAbbrs[i].com) <= 1) add++;
       }
    /* Get number of current abbreviations */
    i = NUM_ABBRS - add;

    for (c=0; sys[c].su_com != -1 ;++c) {
       if (strlen(sys[c].cabbr) > 0) {
          for (a=0;a<i;++a) {
             if (!strcmp(sys[c].command,t_ustr.custAbbrs[a].com)) {
               matched=1;
               break;
               }
             } /* end of abbr for */
          if (!matched) {
            strcpy(t_ustr.custAbbrs[i].com,sys[c].command);
            strcpy(t_ustr.custAbbrs[i].abbr,sys[c].cabbr);
            changed=1;
            i++;
            }
          matched=0;
          a=0;
         } /* end of sys abbr if */
      } /* end of command for */

i=0;
b=1;
c=0;
a=0;
fixed=0;
matched=0;
add=0;

    /* Check if admin or coder changed the name of a command */
    for (i=0;i<NUM_ABBRS;++i) {
     for (c=0; sys[c].su_com != -1 ;++c) {
        if (!strcmp(t_ustr.custAbbrs[i].abbr,sys[c].cabbr) ) {
          if (strcmp(t_ustr.custAbbrs[i].com,sys[c].command)) {
             strcpy(t_ustr.custAbbrs[i].com,sys[c].command);
             changed=1;
            } /* end of sub strcmp if */
          } /* end of main strcmp if */
       } /* end of sys[] for */
      } /* end of num_abbrs for */

i=0;
a=0;
    /*---------------------------------------------------------*/
    /* PART 3 - Save data                                      */
    /*---------------------------------------------------------*/
        if ((a = get_user_num_exact(t_ustr.name,-1)) != -1) {
                write_log(DEBUGLOG,YESTIME,"USERFORS: Reset %s's automsgs to 0 (online)\n",ustr[a].name);
                ustr[a].automsgs = t_ustr.automsgs;
                while (strlen(t_ustr.custAbbrs[i].com) > 1) {
                        strcpy(ustr[a].custAbbrs[i].abbr,t_ustr.custAbbrs[i].abbr);
                        strcpy(ustr[a].custAbbrs[i].com,t_ustr.custAbbrs[i].com);
                        i++;
                }
                i=0;
                a=0;
        }

        write_user(small_buffer);
        } /* end of else */
   } /* while */

if (changed)
 write_log(SYSTEMLOG,YESTIME,"Re-evaluated user abbreviations\n");
 
 (void) closedir(dirp);

}


/*------------------------------------------------------*/
/* Compare NUM_ABBRS to abbr count in sys[] on startup  */
/*------------------------------------------------------*/
void abbrcount()
{
int c=0;
int count=0;

    for (c=0; sys[c].su_com != -1; ++c) {
      if (strlen(sys[c].cabbr) > 0) count++;
    }

    if (count != NUM_ABBRS) {
      write_log(ERRLOG,YESTIME,"BOOT: Abbreviation count in sys[] structure does not match that of NUM_ABBRS\n");
      printf("\nAbbreviation count in sys[] structure does not match that of NUM_ABBRS\n");
      printf("Aborting startup!\n");
#if defined(_WIN32) && !defined(__CYGWIN32__)
WSACleanup();
#endif
      exit(0);
      }
}


/*-----------------------------------------*/
/* Set up the default abbrevation keys     */
/*-----------------------------------------*/
void initabbrs(int user)
{
int c=0;
int i;
	
	for (i=0;i<NUM_ABBRS;++i)
	{
		ustr[user].custAbbrs[i].abbr[0] = 0;
		ustr[user].custAbbrs[i].com[0] = 0;
	}

        i=0;

        for (i=0;i<NUM_ABBRS;++i)
        {

         REDO:
          if (strlen(sys[c].cabbr) > 0) {
            strcpy(ustr[user].custAbbrs[i].com,sys[c].command);
            strcpy(ustr[user].custAbbrs[i].abbr,sys[c].cabbr);
            c++;
           }
          else {
            c++;
            goto REDO;
           }

        }	

}


/*-------------------------------------------------------------------*/
/* Copy abbreviation commands and marks under a blank field into the */
/* field so it isn't blank anymore                                   */
/*-------------------------------------------------------------------*/
void copy_abbrs(int user, int ref)
{
int a=0;
int b=1;

while (strlen(ustr[user].custAbbrs[ref+b].com) > 0) {
 strcpy(ustr[user].custAbbrs[ref+a].com,ustr[user].custAbbrs[ref+b].com);
 strcpy(ustr[user].custAbbrs[ref+a].abbr,ustr[user].custAbbrs[ref+b].abbr);
 a++;
 b++;
 }

/* Make sure last lingering copy is cleared out */
ustr[user].custAbbrs[ref+a].com[0]=0;
ustr[user].custAbbrs[ref+a].abbr[0]=0;
}

/*-----------------------------------------------------------------*/
/* Add new abbreviation fields to the end of the users abbr struct */
/*-----------------------------------------------------------------*/
void add_abbrs(int user, int ref, int num)
{

strcpy(ustr[user].custAbbrs[num].com,sys[ref].command);
strcpy(ustr[user].custAbbrs[num].com,sys[ref].cabbr);

}


/* see if user wants to quit from talker addition */
int is_quit(int user, char *str)
{

if (!strcmp(str,"q") || !strcmp(str,"Q")) {
    ustr[user].t_ent=0;
    ustr[user].t_num        = 0;
    ustr[user].t_name[0]    = 0;
    ustr[user].t_host[0]    = 0;
    ustr[user].t_ip[0]      = 0;
    ustr[user].t_port[0]    = 0;
    write_str(user,"Talker addition aborted!");
    return 1;
    }
return 0;
}


/* is this command revoked */
int is_revoke(char *str)
{
char junkmain[NAME_LEN];
char junk[2];

if (!strlen(str)) {
	/* no command in this slot */
	return 0;
	}

junk[0]=0;
junkmain[0]=0;
strcpy(junkmain,str);
remove_first(junkmain);
sscanf(junkmain,"%s ",junk);
if (!strcmp(junk,"-")) return 1;
else return 0;
}

/* is this command granted */
int is_grant(char *str)
{
char junkmain[NAME_LEN];
char junk[2];

if (!strlen(str)) {
	/* no command in this slot */
	return 0;
	}

junk[0]=0;
junkmain[0]=0;
strcpy(junkmain,str);
remove_first(junkmain);
sscanf(junkmain,"%s ",junk);
if (!strcmp(junk,"+")) return 1;
else return 0;
}


/*** get user number using name ***/
int how_many_users(char *name)
{
int u;
int num=0;

strtolower(name);

for (u=0;u<MAX_USERS;++u)
        if (!strcmp(ustr[u].name,name) && ustr[u].area != -1) num++;

return num;
}


/*** Finds number or users in given area ***/
int find_num_in_area(int area)
{
int u,num=0;
for (u=0;u<MAX_USERS;++u)
        if (ustr[u].area==area) ++num;
return num;
}


/* dynamically create the staff list file for .wlist     */
/* this will be called whenever a promote/demote is done */
void do_stafflist(void) {
int ret=0;
char filename[FILE_NAME_LEN];
char small_buffer[FILE_NAME_LEN];
char z_mess[ARR_SIZE];
char filerid[FILE_NAME_LEN];
char tempranks[MAX_LEVEL+1][1000];
int temprankscount[MAX_LEVEL+1];
struct dirent *dp;
FILE *fp;
DIR *dirp;
char *lineprefix = "             ";
char *linespacer = "  ";

for (ret=0;ret<MAX_LEVEL+1;++ret) {
 tempranks[ret][0]=0;
 temprankscount[ret]=0;
}

sprintf(z_mess,"%s",USERDIR);
strncpy(filerid,z_mess,FILE_NAME_LEN);

dirp=opendir((char *)filerid);

if (dirp == NULL) {
 write_log(ERRLOG,YESTIME,"Can't open directory \"%s\" for do_stafflist! %s\n",filerid,get_error());
 return;
}

sprintf(filename,"%s",WIZFILE);

if (!(fp=fopen(filename,"w"))) {
 (void) closedir(dirp);
 return;
}

while ((dp = readdir(dirp)) != NULL)
   {            
    sprintf(small_buffer,"%s",dp->d_name);
        if (small_buffer[0] == '.')
         continue;

         strtolower(small_buffer);
         ret=read_user(small_buffer);
         if (ret==0) {
          write_log(ERRLOG,YESTIME,"Can't open userfile \"%s\" in do_stafflist. Permission problem probably\n",small_buffer);
          continue;
         }
         else if (ret==-1) {
          write_log(ERRLOG,YESTIME,"Can't open userfile \"%s\" 0 length file! Removed. Continuing..\n",small_buffer);
          continue;
         }

	if (t_ustr.super >= WIZ_LEVEL) {

	 if (!temprankscount[t_ustr.super]) strcat(tempranks[t_ustr.super], lineprefix);
	 else if (temprankscount[t_ustr.super]==3) {
	  strcat(tempranks[t_ustr.super], lineprefix);
	  temprankscount[t_ustr.super]=0;
	 }
	 else strcat(tempranks[t_ustr.super], linespacer);
	 sprintf(z_mess,"%-*s",NAME_LEN+count_color(t_ustr.say_name,0),t_ustr.say_name);
	 strcat(tempranks[t_ustr.super],z_mess);
	 temprankscount[t_ustr.super]++;
	 if (temprankscount[t_ustr.super]==3) strcat(tempranks[t_ustr.super],"\n");

	} /* if staff */

   } /* while */

(void) closedir(dirp);

/* write out list to file */
fputs("\n",fp);

for (ret=0;ret<MAX_LEVEL+1;++ret) {
 if (!strlen(tempranks[ret])) continue;
 fprintf(fp,STAFF_FILE_HEADER,ranks[ret].sname);
 fputs("\n\n",fp);
 fputs(tempranks[ret],fp);
 fputs("\n",fp);
 if (temprankscount[ret] < 3) fputs("\n",fp);
}

FCLOSE(fp);

}
