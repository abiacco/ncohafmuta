#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/time.h>

void midcpy(char *strf, char *strt, int fr, int to);

int main(int argc, char **argv)
{
int i=0,num_days=0;
char mess[513];
char small_buffer[256];
char directory[256];
char days[3];
unsigned long diff=0;
struct stat fileinfo;
struct dirent *dp;
time_t tm;
time_t filetime;
DIR *dirp;

if (argc < 3) {
	printf("Missing required argument(s)!\n");
	printf("Usage: %s <backup directory> <# of days to keep>\n",argv[0]);
	exit(0);
	}

strncpy(directory,argv[1],255);
strncpy(days,argv[2],2);

for (i=0;i<strlen(days);++i) {
	if (!isdigit((int)days[i])) {
	printf("# of days is not a number! Exiting..\n");
	exit(0);
	}
}
i=0;
num_days=atoi(days);

dirp=opendir((char *)directory);

if (dirp == NULL) {
	printf("Can't open directory \"%s\" Exiting..\n",directory);
	exit(0);
	}

while ((dp = readdir(dirp)) != NULL) {
	  sprintf(small_buffer,"%s",dp->d_name);
        if (!strcmp(small_buffer,".") || !strcmp(small_buffer,"..")
	    || strstr(small_buffer,"restored"))
         continue;

	diff=0;
	filetime=0;
	sprintf(mess,"%s/%s",directory,small_buffer);
	stat(mess, &fileinfo);
	filetime = fileinfo.st_mtime;
	time(&tm);
	diff = tm - filetime;
	/* +200 to take in account variation in exact time files */
	/* were written */
	if ((diff > 0) && (diff > (86400 * num_days + 200))) {
	/* remove file */
	remove(mess);
	printf("Removed %s for being approx. %d days old\n",mess,(int)diff/86400);
	}

 } /* end of while */
closedir(dirp);
return 1;
}


void midcpy(char *strf, char *strt, int fr, int to)
{
int f;
for (f=fr;f<=to;++f)
  {
   if (!strf[f])
     {
      strt[f-fr]='\0';
      return;
     }
   strt[f-fr]=strf[f];
  }
strt[f-fr]='\0';
}

