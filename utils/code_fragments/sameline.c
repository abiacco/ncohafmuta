/* Data file converter 1.0.26 to 1.1.0 */

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

/*------------------------------*/
/* START OF USER CONFIG SECTION */
/*------------------------------*/

/*-----------------------------------------------------------------*/
/* Change these 2 defines if you have increased or decreased your  */
/* max areas in you constants.h file from the default of 60        */
/*                                                                 */
/* Max areas defined in 1.0.26 version of code                     */

#define OLD_MAX_AREAS      60

/* Max areas defined in 1.1.0 version of code                      */

#define NEW_MAX_AREAS      60
/*                                                                 */
/*-----------------------------------------------------------------*/

/* Set these variables to whatever they're set at in the */
/* 1.0.26 constants.h                                    */
#define MAX_LINE_LEN   200
#define NAME_LEN       21
#define MACRO_LEN      70
#define DESC_LEN       41
#define EMAIL_LENGTH   80
#define HOME_LEN       66
#define NUM_LINES      15
#define NUM_ABBRS      16
#define MAX_GAG        15
#define MAX_ALERT      15
#define MAX_ENTERM     80
#define MAX_EXITM      45

/*----------------------------*/
/* END OF USER CONFIG SECTION */
/*----------------------------*/

#undef feof  /* otherwise feof func. wont compile */
#define LOOP_FOREVER while(1)
#define FCLOSE(file) if (file) fclose(file)

#define TRUE 1
#define FALSE 0
#define USERDIR "../users"
#define FILE_NAME_LEN  256
#define MAX_CHAR_BUFF  380
#define MAX_USERS      3000

char mess[9725];
char t_mess[9725];


main()
{
int num;
int i=0;
char small_buff[64];
char tmpbuf1[2000];
char filename[FILE_NAME_LEN];
char filename2[FILE_NAME_LEN];
char filerid[FILE_NAME_LEN];
struct dirent *dp;
DIR  *dirp;
FILE *fp;
FILE *fp2;

  puts(" ");
  puts("Will first backup user data files to  users.tar.gz");
  sprintf(mess,"tar cvf /tmp/users.tar %s/*",USERDIR);
  system(mess);
  strcpy(mess,"gzip -9 /tmp/users.tar");
  system(mess);
  strcpy(mess,"mv /tmp/users.tar.gz .");
  system(mess);
 
 sprintf(t_mess,"%s",USERDIR);
 strncpy(filerid,t_mess,FILE_NAME_LEN);

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

   puts("Opening file..");
   sprintf(filename2,"%s/%s",USERDIR,small_buff);
   if (!(fp=fopen(filename2,"r"))) {
     puts("Can't open original file\n"); exit(0);
     }
   sprintf(filename,"%s/%s.temp",USERDIR,small_buff);
   puts("Opening append file..");
   if (!(fp2=fopen(filename,"a"))) {
     puts("Can't open append file\n"); fclose(fp); exit(0);
     }
   puts("File opened.");

   tmpbuf1[0]=0;
   for (i=0;i<28;++i) {
     puts("Getting line..");
     fgets(tmpbuf1,1000,fp);
     puts("Got line..putting line to new file..");
     fputs(tmpbuf1,fp2);
     puts("Line written.");
     tmpbuf1[0]=0;
     }
   fgets(tmpbuf1,1000,fp);
   fclose(fp);
   printf("Line before mods: %s",tmpbuf1);
   tmpbuf1[strlen(tmpbuf1)-1]=0;
   strcat(tmpbuf1," 0");
   printf("Line after mods : %s\nEnd.\n",tmpbuf1);
   fputs(tmpbuf1,fp2);
   fputs("\n",fp2);
   fclose(fp2);
   num++;
   rename(filename,filename2);

/* Set structures back to 0 just to be sure */
    printf("%d\n",num);
    small_buff[0]=0;
   }       /* End of while */
 
 printf("\n");
 sprintf(mess,"Converted %d users",num);
 printf("%s\n",mess);

 (void) closedir(dirp);

}

