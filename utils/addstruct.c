/* Utility to add structures to the end of user data files */
/* on a directory-wide level                               */

/*-------------------------------------------------------------*/
/* includes used for this code                                 */
/*-------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>  
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h> 
#include <sys/time.h>      /* for linux users */

#include <arpa/telnet.h>
#include <netinet/in.h>

#include <fcntl.h>
#include <sys/file.h>
#include <netdb.h>
#include <dirent.h>
#include <signal.h>
#include <time.h>
#include <errno.h>
#include <sys/wait.h>
#include <sys/un.h>            /* linux users      */
#include <string.h>


#undef feof  /* otherwise feof func. wont compile */
#define LOOP_FOREVER while(1)

#define TRUE 1
#define FALSE 0
#define USERDIR "../users"
#define FILE_NAME_LEN  256

char mess[9725];
char t_mess[9725];

main()
{
int num=0;
int i=0;
int mode=0;
long number;
char yesno[9000];
char buffer[9000];
char command[9000];
char timestr[9000];
char small_buff[64];
char filename[FILE_NAME_LEN];
char filerid[FILE_NAME_LEN];
time_t tm;
struct dirent *dp;
DIR  *dirp;
FILE *fp;

puts("Addstruct 1.0");
puts(" ");
puts("This utility is used to add a bogus structure, integer or array, to");
puts("the end of a users data file, so the talker can read the file");
puts("without any problems, after the programmers adds new saveable strcutures");
puts("to the ustr[user]. profile");

puts("int   = number");
puts("char  = string");
puts("timen = numeric time used for calculations");
puts("times = time string used jsut for displaying");
puts(" ");
START:
puts("Which type would you like to add? (int,char,timen,times)");
gets(command);
if (!strlen(command)) exit(0);

if (!strcmp(command,"int")) {
   INT:
   puts("What number would you like the structure to start at? (normally 0)");
   gets(buffer);
   if (!strlen(buffer)) {
     puts("You must enter a number!");
     buffer[0]=0;
     goto INT;
     }
   for (i=0;i<strlen(buffer);++i) {
      if (!isdigit(buffer[i])) {
        puts("That's not a numeric string");
        buffer[0]=0;
        i=0;
        goto INT;
        }
      }
    i=0;
    number=atoi(buffer);
    mode=1;
  }
else if (!strcmp(command,"char")) {
   CHAR:
   puts("What would you like the defaulting string to say?");
   gets(buffer);
   if (!strlen(buffer)) {
     puts("You must enter a string!");
     buffer[0]=0;
     goto CHAR;
     }
  mode=2;
  }
else if (!strcmp(command,"timen")) {
  time(&tm);
  number = tm;
  mode=3;
  }
else if (!strcmp(command,"times")) {
  time(&tm);
  strcpy(timestr,ctime(&tm));
  timestr[24]=0;
  mode=4;
  }
else {
  puts("Option does not exist!");
  puts(" ");
  command[0]=0;
  strcpy(command,"");
  goto START;
  }

 sprintf(t_mess,"%s",USERDIR);
 strncpy(filerid,t_mess,FILE_NAME_LEN);

YES:
puts("Would you like to backup your user files first? (y/n) ");
gets(yesno);
if (!strlen(yesno)) goto YES;

if (!strcmp(yesno,"y") || !strcmp(yesno,"Y"))
 {
  puts(" ");
  puts("Will first backup user data files to  users.tar.gz");
  sprintf(mess,"tar cvf /tmp/users.tar %s/*",USERDIR);
  system(mess);
  strcpy(mess,"gzip -9 /tmp/users.tar");
  system(mess);
  strcpy(mess,"mv /tmp/users.tar.gz .");
  system(mess);
  puts("Modifying user files..");
  sleep(2);
 }
else if (!strcmp(yesno,"n") || !strcmp(yesno,"N")) {
  puts("Modifying user files..");
  sleep(1);
 }
else {
  puts("Invalid answer.");
  yesno[0]=0;
  goto YES;
 }

 /* Reset new structures to 0 */ 
 num=0;

 dirp=opendir((char *)filerid);
  
 if (dirp == NULL)
   { printf("Directory information not found.\n");
    return;
   }
   
 while ((dp = readdir(dirp)) != NULL) 
   { 
    sprintf(small_buff,"%s",dp->d_name);
    if (small_buff[0]=='.') {
      small_buff[0]=0;
      continue;
      }

sprintf(filename,"%s/%s",USERDIR,small_buff);

 /* Open file for appending */
 fp=fopen(filename,"a");

 if ((mode==1) || (mode==3)) {
   fprintf(fp,"%d\n",number);
   fclose(fp);
  }
 else if (mode==2) {
   fprintf(fp,"%s\n",buffer);
   fclose(fp);
  }
 else if (mode==4) {
   fprintf(fp,"%s\n",timestr);
   fclose(fp);
  }

 num++;

    /* Set structures back to 0 just to be sure */
    printf("%d\n",num);
    small_buff[0]=0;
   }       /* End of while */
 
  printf("\n");
  sprintf(mess,"Modified %d users",num);
  printf("%s\n",mess);

  (void) closedir(dirp);

}
