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

extern char *syserror;
extern char mess[ARR_SIZE+25];    /* functions use mess to send output   */
extern char t_mess[ARR_SIZE+25];  /* functions use t_mess as a buffer    */
extern char thishost[101];      /* FQDN were running on                   */
extern int PORT;                  /* main login port for incoming   */
extern unsigned int mailgateway_port;


/* Generate a random password */
char *generate_password(void)
{
int i=0;
static char pass[12];

pass[0]=0;

i = rand() % 32767;
sprintf(pass,"pin%d",i);

return pass;
}

/* Delete a user that was in tracking but now is a new user */
void delete_verify(int user)
{
char filename[FILE_NAME_LEN];
char filename2[FILE_NAME_LEN];
char junk[12];
char name2[NAME_LEN+1];
long timenum;
FILE *fp;
FILE *fp2;

strcpy(filename,VERIFILE);
if (!(fp=fopen(filename,"r"))) return;

strcpy(filename2,get_temp_file());

if (!(fp2=fopen(filename2,"w"))) {
  fclose(fp);
  write_log(ERRLOG,YESTIME,"EMAILVER: Couldn't open tempfile(w) in delete_verify! %s\n",get_error());
  return;
  }

while (!feof(fp)) {
 fscanf(fp,"%s %s %ld\n",name2,junk,&timenum);
 if (strcmp(name2,ustr[user].login_name))
  {
   fprintf(fp2,"%s %s %ld\n",name2,junk,timenum);
  }
 } /* end of while */

fclose(fp);
fclose(fp2);

remove(filename);
if (!file_count_lines(filename2))
 remove(filename2);
else
 rename(filename2,filename);

}

/* Check to see if user is an email verifying user */
int check_verify(int user, int mode)
{
int found=0;
char filename[FILE_NAME_LEN];
char filename2[FILE_NAME_LEN];
char name2[NAME_LEN+1];
char junk[12];
unsigned long diff=0;
long timenum;
FILE *fp;
FILE *fp2;

strcpy(filename,VERIFILE);
if (!(fp=fopen(filename,"r"))) return 0;

if (mode==0) {
while (!feof(fp)) {
 fscanf(fp,"%s %s %ld\n",name2,ustr[user].login_pass,&timenum);
 if (!strcmp(name2,ustr[user].login_name)) {
  found=1;
  break;
  } /* end of if */
 } /* end of while */

fclose(fp);

 if (found==1) { return 1; }
 else {
  ustr[user].login_pass[0]=0;
  return 0;
  }
} /* end of mode if */
else if (mode==1) {

strcpy(filename2,get_temp_file());

if (!(fp2=fopen(filename2,"w"))) {
  fclose(fp);
  write_log(ERRLOG,YESTIME,"Couldn't open tempfile(w) in check_verify! %s\n",get_error());
  return 0;
  }

while (!feof(fp)) {
 fscanf(fp,"%s %s %ld\n",name2,junk,&timenum);
 diff = (unsigned long)(time(0) - timenum);
 if (diff >= (3600*24)) { continue; }
 else { fprintf(fp2,"%s %s %ld\n",name2,junk,(unsigned long)timenum); }
 } /* end of while */

fclose(fp);
fclose(fp2);
remove(filename);

if (!file_count_lines(filename2))
 remove(filename2);
else
 rename(filename2,filename);

 return 1;
} /* end of else if */

return 0;
}

/* Write user email verification info to tracking file */
int write_verifile(int user)
{
char filename[FILE_NAME_LEN];
FILE *fp;

strcpy(filename,VERIFILE);
if (!(fp=fopen(filename,"a"))) {
  write_log(ERRLOG,YESTIME,"EMAILVER: Couldn't open file(a) \"%s\" in write_verifile! %s\n",filename,get_error());
  return -1;
  }

sprintf(mess,"%s %s %ld\n",ustr[user].login_name,ustr[user].login_pass,(unsigned long)time(0));
fputs(mess,fp);
fclose(fp);
return 1;

}

/* E-Mail the user their username and the random passsord to emailadd */
int mail_verify(int user, char *emailadd)
{
char filename[FILE_NAME_LEN];

strncpy(filename,VERIEMAIL,FILE_NAME_LEN);

/* send external email */
if (send_ext_mail(-2, user, 4, NULL, filename, DATA_IS_FILE, emailadd)==-1) {
	write_log(ERRLOG,YESTIME,"EMAILVER: Couldn't send external email in mail_verify()\n");
	return -1;
}

/* Write to log */
write_log(VEMAILLOG,YESTIME,"SENT VERIFY %s:%s:%s:%s\n",ustr[user].login_name,
             ustr[user].site,ustr[user].net_name,emailadd);

return 1;
}

