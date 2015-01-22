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


/*----------------------------------------------*/
/* Check to see if a site or hostname is banned */
/*----------------------------------------------*/
int check_restriction(int user, int type, int type2)
{
int i=0;
char small_buff[128];
char mess_buf[321];  
char filerid[FILE_NAME_LEN];
char filename[FILE_NAME_LEN];
struct dirent *dp;
FILE *fp;
DIR  *dirp;

if (type2==THEIR_HOST) {
 if (!strcmp(ustr[user].net_name,SYS_RES_OFF) ||
     !strcmp(ustr[user].net_name,SYS_LOOK_FAILED) ||
     !strcmp(ustr[user].net_name,SYS_LOOK_PENDING)) return 0;
}

if (type==ANY) {
 sprintf(t_mess,"%s",RESTRICT_DIR);
 }
else {
 sprintf(t_mess,"%s",RESTRICT_NEW_DIR);  
 }

 strncpy(filerid,t_mess,FILE_NAME_LEN);

 dirp=opendir((char *)filerid);

 if (dirp == NULL)
   {
    write_str(user,"Directory information not found.");
    write_log(ERRLOG,YESTIME,
    "Directory information not found for directory \"%s\" in check_restriction %s\n",
    filerid,get_error());
    return 0;
   }  
 
 while ((dp = readdir(dirp)) != NULL)
   {
    sprintf(small_buff,"%s",dp->d_name);
    if (small_buff[0]=='.') continue;
      i=strlen(small_buff);
      if (isdigit((int)small_buff[i-1]) && ((type2==THEIR_IP) || (type2==ANY)))
       {
        if (!strcmp(small_buff,ustr[user].site)) {
           sprintf(filename,"%s/%s.r",filerid,small_buff);
           if (!(fp=fopen(filename,"r"))) {
              write_str(user,"Cant open restrict file.");
              return 1;  
              }
           mess_buf[0]=0;
           fgets(mess_buf,REASON_LEN+1,fp);
           write_str(user,mess_buf); 
           write_str(user," ");
           FCLOSE(fp);
           (void) closedir(dirp);
           return 1;
           }
        else if (check_site(ustr[user].site,small_buff,1)) {
           sprintf(filename,"%s/%s.r",filerid,small_buff);
           if (!(fp=fopen(filename,"r"))) {
              write_str(user,"Cant open restrict file.");
              return 1;
              }
           mess_buf[0]=0;
           fgets(mess_buf,REASON_LEN+1,fp);
           write_str(user,mess_buf);
           write_str(user," ");
           FCLOSE(fp);
           (void) closedir(dirp);
           return 1;
           }
        else continue;
       }
      else if (!isdigit((int)small_buff[i-1]) && ((type2==THEIR_HOST) || (type2==ANY)))
       {
        if (!strcmp(small_buff,ustr[user].net_name)) {   
           sprintf(filename,"%s/%s.r",filerid,small_buff);
           if (!(fp=fopen(filename,"r"))) {
              write_str(user,"Cant open restrict file.");
              return 1;
              }
           mess_buf[0]=0;
           fgets(mess_buf,REASON_LEN+1,fp);
           write_str(user,mess_buf);
           write_str(user," ");
           FCLOSE(fp);
           (void) closedir(dirp);
           return 1;
           }
        else if (check_site(ustr[user].net_name,small_buff,0)) {
           sprintf(filename,"%s/%s.r",filerid,small_buff);
           if (!(fp=fopen(filename,"r"))) {
              write_str(user,"Cant open restrict file.");
              return 1;
              }
           mess_buf[0]=0;
           fgets(mess_buf,REASON_LEN+1,fp);
           write_str(user,mess_buf);
           write_str(user," ");
           FCLOSE(fp);
           (void) closedir(dirp);
           return 1;
           }
        else continue;
       }
      else { continue; }
   }       /* End of while */

 (void) closedir(dirp);
 return 0;

}


/*-----------------------------------------*/
/* Check restrictions for who and www port */
/*-----------------------------------------*/
int check_misc_restrict(int sock2, char *site, char *namesite)
{
int i=0;
char small_buff[128];
char mess_buf[321];   
char filerid[FILE_NAME_LEN];
char filename[FILE_NAME_LEN];
struct dirent *dp;
FILE *fp;     
DIR  *dirp;
 
 sprintf(t_mess,"%s",RESTRICT_DIR);
 strncpy(filerid,t_mess,FILE_NAME_LEN);
           
 dirp=opendir((char *)filerid);

 if (dirp == NULL)
   {
    return 0;
   }

 while ((dp = readdir(dirp)) != NULL)
   {
    sprintf(small_buff,"%s",dp->d_name);
    if (small_buff[0]=='.') continue;
      i=strlen(small_buff);
      if (isdigit((int)small_buff[i-1]))
       {
        if (!strcmp(small_buff,site)) {
           sprintf(filename,"%s/%s.r",filerid,small_buff);
           if (!(fp=fopen(filename,"r"))) {
              return 1;
              }
           mess_buf[0]=0;
           fgets(mess_buf,REASON_LEN+1,fp);
           write_it(sock2,mess_buf);
           write_it(sock2,"\n\r");   
           FCLOSE(fp);
           (void) closedir(dirp);
           return 1;
           }
        else if (check_site(site,small_buff,1)) {
           sprintf(filename,"%s/%s.r",filerid,small_buff);
           if (!(fp=fopen(filename,"r"))) {
              return 1;
              }
           mess_buf[0]=0;
           fgets(mess_buf,REASON_LEN+1,fp);
           write_it(sock2,mess_buf);
           write_it(sock2,"\n\r");
           FCLOSE(fp);
           (void) closedir(dirp);    
           return 1;  
           }
        else continue;
       }
      else if (!isdigit((int)small_buff[i-1]))   
       {
        if (!strcmp(small_buff,namesite)) {
           sprintf(filename,"%s/%s.r",filerid,small_buff);
           if (!(fp=fopen(filename,"r"))) {
              return 1;  
              }
           mess_buf[0]=0;
           fgets(mess_buf,REASON_LEN+1,fp);
           write_it(sock2,mess_buf);
           write_it(sock2,"\n\r");   
           FCLOSE(fp);
           (void) closedir(dirp);
           return 1;  
           }
        else if (check_site(namesite,small_buff,0)) {
           sprintf(filename,"%s/%s.r",filerid,small_buff);
           if (!(fp=fopen(filename,"r"))) {
              return 1;
              }
           mess_buf[0]=0;
           fgets(mess_buf,REASON_LEN+1,fp);
           write_it(sock2,mess_buf);
           write_it(sock2,"\n\r");
           FCLOSE(fp);
           (void) closedir(dirp);    
           return 1;  
           }
        else continue;
       }
      else { continue; }
   }       /* End of while */
           
 (void) closedir(dirp);
 return 0;
           
}


/*-----------------------------------------------------------------------*/
/* check if site ends are the same (search starts from the end of string */
/* for hostnames, beginning of string for ips)                           */
/*-----------------------------------------------------------------------*/
int check_site(char *str1, char *str2, int mode)
{
int i,j=strlen(str1); 
        
if (j<strlen(str2)) return 0;
   
if (!mode) {
/* hostname check */   
for (i=strlen(str2)-1;i>=0;i--) if (str1[--j]!=str2[i]) return 0;
return 1;  
}
else {     
/* ip check */
for (i=0;i<strlen(str2);++i) if (str1[i]!=str2[i]) return 0;
return 1;
}
}


/*------------------------------------------------*/
/* See if user's ip is in current connection list */
/* and has pattern of login hammering             */
/*------------------------------------------------*/
int check_connlist(int user) {
int pos=0,freeslot=0;

pos=in_connlist(user);

if (pos==-1) {
        /* not in list */
        freeslot=find_free_connslot();
        strcpy(connlist[freeslot].site,ustr[user].site);
        connlist[freeslot].connections=1;
        connlist[freeslot].starttime=time(0);
	write_log(BANLOG,YESTIME,"new conn\n");
        }
else {
        /* pos is where they are */
        if (connlist[pos].connections > MAX_CONNS_PER_MIN) {
        /* auto ban the site */
        auto_restrict(user);
        check_connlist_entries(pos);
	write_str(user,"----------------------------------------------------------------");
	write_str(user,"Notice:  You are attempting to use this computer system in a way");
	write_str(user,"         which is considered a crime under United States federal");
	write_str(user,"         access laws.  All attempts illegally accessing this site are ");
	write_str(user,"         logged.  Repeat violators of this offense will be ");
	write_str(user,"         prosecuted to the fullest extent of the law.");
	write_str(user,"----------------------------------------------------------------");
        return 1;
        }
        else {
	write_log(BANLOG,YESTIME,"incrementing conn\n");
	connlist[pos].connections++;
	}
}

return 0;
}


/*-----------------------------------------------*/
/* Auto-restrict a site that is hacking on login */
/*-----------------------------------------------*/
void auto_restrict(int user)
{
char timestr[30];
char filename[FILE_NAME_LEN];
FILE *fp;

sprintf(t_mess,"%s/%s", RESTRICT_DIR, ustr[user].site);
strncpy(filename, t_mess, FILE_NAME_LEN);

 if (!(fp=fopen(filename,"w"))) {
  return;
 }

sprintf(timestr,"%ld\n",(unsigned long)time(0));
fputs(timestr,fp);
FCLOSE(fp);

/* Add set reason to reason file */
sprintf(t_mess,"%s/%s.r", RESTRICT_DIR,ustr[user].site);
strncpy(filename, t_mess, FILE_NAME_LEN);

 if (!(fp=fopen(filename,"w"))) {
  return;
 }

fputs("Your site is denied access for attempted hacking or hammering\n",fp);
FCLOSE(fp);

/* Add set comment to comment file */
sprintf(t_mess,"%s/%s.c",RESTRICT_DIR,ustr[user].site);
strncpy(filename, t_mess, FILE_NAME_LEN);

 if (!(fp=fopen(filename,"w"))) {
  return;
 }

sprintf(mess,"Site denied access for attempted hacking or hammering, possibly user %s\n",ustr[user].login_name);
fputs(mess,fp);
FCLOSE(fp);

write_log(BANLOG,YESTIME,"AUTO-RESTRICT of site %s, possibly user %s\n",
		ustr[user].site,ustr[user].login_name);

sprintf(mess,"%s AUTO-RESTRICT of site %s, possibly user %s",STAFF_PREFIX,ustr[user].site,ustr[user].login_name);
writeall_str(mess, WIZ_ONLY, -1, 0, -1, BOLD, WIZT, 0);
}

/*-----------------------------------------------*/
/* Function to check name to see if it is banned */
/*-----------------------------------------------*/
int check_nban(char *str, char *sitename)
{
int found=0;
int i=0;
char tempname[NAME_LEN+1];
 
     for (i=0;i<NUM_NAMEBANS;++i) {
        strcpy(tempname,nbanned[i]);
        strtolower(tempname);
        if (SUB_BANNAME) {
          if (strstr(str,tempname)) {
               found=1;
               break;
              }
          }
        else {
          if (!strcmp(str,tempname)) {
               found=1;
               break;
              }
          }
       }
        
if (found==1) {
 write_log(BANLOG,YESTIME,
 "BANNAME: User from site %s tried to login with banned name %s\n",sitename,str);
 return 1;
}
else return 0; 
           
}


