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
extern int NUM_AREAS;          /* number of areas defined in config file */
extern char thishost[101];      /* FQDN were running on                   */
extern int noprompt;            /* talker waiting for user input?         */
extern int PORT;                  /* main login port for incoming   */


/*------------------------------------------*/
/* Remove all files from the junk directory */
/*------------------------------------------*/
void remove_junk(int startup)
{
char small_buff[64];
char filerid[FILE_NAME_LEN];
char filename[FILE_NAME_LEN];
struct dirent *dp;
DIR  *dirp;
 
 strcpy(t_mess,"junk");
 strncpy(filerid,t_mess,FILE_NAME_LEN);

 dirp=opendir((char *)filerid);

 if (dirp == NULL)
   {
    sprintf(t_mess,"RJUNK: Can't open directory \"%s\" for remove_junk! %s\n",filerid,get_error());
    write_log(ERRLOG,YESTIME,t_mess);

	if (startup)
	printf("%s",t_mess);
    return;
   }
   
 while ((dp = readdir(dirp)) != NULL) 
   { 
    sprintf(small_buff,"%s",dp->d_name);
       if (small_buff[0] != '.')
        {
         sprintf(filename,"%s/%s",filerid,small_buff);
         remove(filename);
        }
    filename[0]=0;
    small_buff[0]=0;
   }

 (void) closedir(dirp);
}


/* Convert userdata file from old version to new version */
/* mode 0 - convert already standardized file		 */
/* mode 1 - convert old non-standardized file		 */
int convert_file(FILE *f, char *filename, int mode)
{
char junk[20];
char buf[1001];
char tempfile[FILE_NAME_LEN];
FILE *rfp;
FILE *wfp;

fclose(f);
if (!(rfp=fopen(filename,"r"))) {
        write_log(ERRLOG,YESTIME,"CONVERT: Couldn't open userfile(r) \"%s\" in convert_file! %s\n",filename,get_error());
	return 0;
	}
strcpy(tempfile,get_temp_file());
if (!(wfp=fopen(tempfile,"w"))) {
        write_log(ERRLOG,YESTIME,"CONVERT: Couldn't open tempfile(w) \"%s\" to be used in convert_file! %s\n",tempfile,get_error());
	return 0;
	}

if (mode==0) {
	/* first line is version, discard */
	rbuf(junk,20);
	/* write current version to file */
	fputs(UDATA_VERSION,wfp);
	fputs("\n",wfp);

	while (fgets((char *)buf,1000,rfp) != NULL) {
	buf[strlen(buf)-1]=0; /* get rid of nl */

	if (!strncmp(buf,"--ENDVER",8)) {
		/* any new structs I have should get written here */
		remove_first(buf);
		if (!strcmp(buf,"121.ver")) {
		fputs("0\n",wfp); /* hangman wins */
		fputs("0\n",wfp); /* hangman losses */
		fputs("No idea\n",wfp); /* ICQ number */
		fputs("NA\n",wfp); /* miscstr1 */
		fputs("NA\n",wfp); /* miscstr2 */
		fputs("NA\n",wfp); /* miscstr3 */
		fputs("NA\n",wfp); /* miscstr4 */
		fputs("1 0 0 0 0\n",wfp); /* pause_login, miscnum2-5 */
		}
		/* tack on new marker for this version */
		sprintf(mess,"--ENDVER %s\n",UDATA_VERSION);
		fputs(mess,wfp);
		/* continue on looking for user-added structs */
	  } /* end of if ENDVER */
	else {
		fputs(buf,wfp);
		fputs("\n",wfp);
	  } /* end of else */

	} /* end of while */
	fclose(rfp);
	fclose(wfp);
	remove(filename);
	rename(tempfile,filename);
  } /* end of if mode 0 */
else if (mode==1) {
	fputs(UDATA_VERSION,wfp);
	fputs("\n",wfp);

	while (fgets((char *)buf,1000,rfp) != NULL) {
	buf[strlen(buf)-1]=0; /* get rid of nl */

	if (!strcmp(buf,"..End revokes..")) {
		fputs(buf,wfp);
		fputs("\n",wfp);
		/* any new structs I have should get written here */
		fputs("0\n",wfp); /* hangman wins */
		fputs("0\n",wfp); /* hangman losses */
		fputs("No idea\n",wfp); /* ICQ number */
		fputs("NA\n",wfp); /* miscstr1 */
		fputs("NA\n",wfp); /* miscstr2 */
		fputs("NA\n",wfp); /* miscstr3 */
		fputs("NA\n",wfp); /* miscstr4 */
		fputs("1 0 0 0 0\n",wfp); /* pause_login, miscnum2-5 */
		/* tack on marker */
		sprintf(mess,"--ENDVER %s\n",UDATA_VERSION);
		fputs(mess,wfp);
		/* continue on looking for user-added structs */
	  } /* end of if revoke */
	else {
		fputs(buf,wfp);
		fputs("\n",wfp);
	  } /* end of else */

	} /* end of while */
	fclose(rfp);
	fclose(wfp);
	remove(filename);
	rename(tempfile,filename);
  } /* end of else if mode if 1 */

return 1;
}


/*** count no. of messages (counts no. of newlines in message files) ***/
void messcount()
{
char filename[FILE_NAME_LEN];
int a;

for(a=0;a<NUM_AREAS;++a) {
	astr[a].mess_num=0;
	sprintf(t_mess,"%s/board%d",MESSDIR,a);
	strncpy(filename,t_mess,FILE_NAME_LEN);
	
        astr[a].mess_num = file_count_lines(filename);
	}
}


/** page a file out to a socket **/
int cat_to_sock(char *filename, int accept_sock)
{
int n;
int count=0;
char line[257];
FILE *fp;

if (!(fp=fopen(filename,"rb"))) 
  {
   write_log(ERRLOG,YESTIME,"Can't open binary file(rb) \"%s\" in cat_to_sock! %s\n",filename,get_error());
   return 0;
  }

line[0]=0;

/*
fread(line, 1, 257, fp);

while(!feof(fp)) {
	        S_WRITE(accept_sock,line,strlen(line));

  fread(line, 1, 256, fp);
*/

while ((n = fread(line, 1, sizeof(line), fp)) != 0) {
   count+=S_WRITE(accept_sock,line,n);
   } /* end of while */

fclose(fp);

return 1;
}


/** page a file out to a socket **/
int cat_to_www(char *filename, int user)
{
int n=0,istext=0;
char line[512];
FILE *fp;

if (strstr(filename,".htm") || strstr(filename,".txt")) istext=1;

if (!(fp=fopen(filename,"rb"))) 
  {
   write_log(ERRLOG,YESTIME,"Can't open binary file(rb) \"%s\" in cat_to_www! %s\n",filename,get_error());
   return 0;
  }

for (n=0;n<sizeof(line);++n) line[n]='\0';

/* for now we only do macro substitutions in HTML or TEXT files */
if (istext) {
 while (fgets(line, 256, fp) != NULL) {
  strcpy(line,check_var(line,SYS_VAR,SYSTEM_NAME));
  strcpy(line,check_var(line,HOST_VAR,thishost));
  strcpy(line,check_var(line,MAINPORT_VAR,itoa(PORT)));
  strcpy(line,check_var(line,WEBPORT_VAR,itoa(PORT+WWW_OFFSET)));
  n=strlen(line);
  write_str_www(user,line,n);
  for (n=0;n<sizeof(line);++n) line[n]='\0';
 } /* end of while */
} /* if */
else {
 while ((n = fread(line, 1, 256, fp)) != 0) {
  /* write_log(DEBUGLOG,YESTIME,"N IS %d STRLEN IS %d\n",n,strlen(line)); */
  write_str_www(user,line,n);
  for (n=0;n<sizeof(line);++n) line[n]='\0';
 } /* end of while */
} /* else */

fclose(fp);

return 1;
}


/*** page a file out to user ***/
int cat(char *filename, int user, int line_num)
{
int num_chars=0,lines=0,retval=1;
FILE *fp;
int max_lines = 25;
int line_pos = 0;
int file_lines = 0;
int i = 0;
char leader[17];


/* Count total lines in file */
if (!(file_lines = file_count_lines(filename))) {
   ustr[user].file_posn  = 0;  
   ustr[user].line_count = 0;
   ustr[user].numbering = 0;
   return 0;
  }

/* Used when presenting prompt at end of file..nah
if (ustr[user].line_count == file_lines) {
   ustr[user].number_lines = 0;
   ustr[user].file_posn    = 0;  
   ustr[user].line_count   = 0;
   ustr[user].numbering    = 0;
	noprompt=0;
	return 0;
}
*/

/* Open file */
if (!(fp=fopen(filename,"r"))) 
  {
   ustr[user].file_posn  = 0;  
   ustr[user].line_count = 0;
   ustr[user].numbering = 0;
   return 0;
  }

/* Do we want line numbering? */
if (line_num == 1)
  ustr[user].number_lines = 1;

  
/* Jump to reading posn in file */
if (line_num != -1) 
  {
    fseek(fp,ustr[user].file_posn,0);
    max_lines = ustr[user].rows;
    line_pos = ustr[user].line_count;
  }
 else
  {
    max_lines = 999;
    line_num = 0;
  }
  
if (max_lines < 5 || max_lines > 999)
   max_lines = 25;

/* loop until end of file or end of page reached */
mess[0]=0;
fgets(mess, sizeof(mess)-25, fp);
		   strcpy(mess,check_var(mess,SYS_VAR,SYSTEM_NAME));
		   strcpy(mess,check_var(mess,USER_VAR,ustr[user].say_name));
		   strcpy(mess,check_var(mess,HOST_VAR,thishost));
		   strcpy(mess,check_var(mess,MAINPORT_VAR,itoa(PORT)));
		   strcpy(mess,check_var(mess,WEBPORT_VAR,itoa(PORT+WWW_OFFSET)));

if (!ustr[user].cols) ustr[user].cols=80;

while(!feof(fp) && lines < max_lines) 
  {
   line_pos++;
  
   i = strlen(mess);
   lines      += i / ustr[user].cols + 1;
   num_chars  += i;
   
   if (ustr[user].number_lines) {
    if (ustr[user].numbering > 0)
     sprintf(leader,"%-3d ",ustr[user].numbering);
    else
     sprintf(leader,"%-3d ",line_pos);
    }
    else {
          leader[0]=0;
         }

   mess[i-1] = 0;      /* remove linefeed */
   sprintf(t_mess,"%s%s",leader, mess);
    
   write_str(user,t_mess);
   if (ustr[user].numbering > 0) ustr[user].numbering++;
   fgets(mess, sizeof(mess)-25, fp);
		   strcpy(mess,check_var(mess,SYS_VAR,SYSTEM_NAME));
		   strcpy(mess,check_var(mess,USER_VAR,ustr[user].say_name));
		   strcpy(mess,check_var(mess,HOST_VAR,thishost));
		   strcpy(mess,check_var(mess,MAINPORT_VAR,itoa(PORT)));
		   strcpy(mess,check_var(mess,WEBPORT_VAR,itoa(PORT)+WWW_OFFSET));
  }
  
if (user== -1) goto SKIP;

/* End of file? */
if (feof(fp)) 
  {
   ustr[user].number_lines = 0;
   ustr[user].file_posn    = 0;
   ustr[user].line_count   = 0;
   ustr[user].numbering    = 0;
   noprompt=0;  
   retval=2;
  }
else  
  {
   /* store file position and file name */
   ustr[user].file_posn += num_chars;
   ustr[user].line_count = line_pos;
   strcpy(ustr[user].page_file,filename);
   sprintf(mess,CONF_PROMPT,
        (int)(((float)((float)ustr[user].line_count/(float)file_lines)) * 100),
	ustr[user].line_count,file_lines);
/* ustr[user].file_posn,1); */
   write_str_nr(user,mess);
	telnet_write_eor(user);
   noprompt=1;
  }
  
SKIP:
  FCLOSE(fp);
  return retval;
}


/*--------------------------------------------------------------*/
/* selective line removal from files                            */
/*--------------------------------------------------------------*/
void remove_lines_from_file(int user, char *file, int lower, int upper)
{
int mode  = 0;
char temp[FILE_NAME_LEN];
FILE *bfp,*tfp;
   
/*---------------------------------------------------------*/
/* determine the mode for line deletion                    */
/*---------------------------------------------------------*/
      
if (lower == -1 && upper == -1)       mode = 1;  /* all lines         */
else if (lower == upper)              mode = 2;  /* one line          */
else if (lower > 0 && upper == 0)     mode = 3;  /* to end of file    */
else if (upper > 0 && lower == 0)     mode = 4;  /* from beginning    */
else if ((upper < lower) || (upper > 0 && lower == -1)) mode = 5;  /* leave middle      */
else if (upper > lower)               mode = 6;  /* delete between    */
      
/*---------------------------------------------------------*/
/* check to make sure the file exists                      */
/*---------------------------------------------------------*/
 
if (!(bfp=fopen(file,"r")))
  {
   write_str(user,"The file was empty, could not delete");   
   return;
  }   


/*-------------------------------------------*/
/* delete the entire file                    */
/*-------------------------------------------*/
if (mode == 1)
  {
   FCLOSE(bfp);
   remove(file);
   return;
  }
   
/*---------------------------------------------------------*/
/* make temp file                                          */
/*---------------------------------------------------------*/

strcpy(temp,get_temp_file());

if (!(tfp=fopen(temp,"w")))
  {
   write_str(user,BAD_FILEIO);
   write_log(ERRLOG,YESTIME,"Couldn't open tempfile(w) in remove_lines_from_file! %s\n",
   get_error());
   FCLOSE(tfp); 
   return;
  }
   
/*------------------------------------------------------*/   
/* get the right lines from the file                    */   
/*------------------------------------------------------*/   

switch(mode)
  {
    case 1: break;   /* already done */
   
    case 2:
            file_copy_lines(bfp, tfp, lower);
            file_skip_lines(bfp, 2 );
            file_copy_lines(bfp, tfp, 99999);
            break;
   
    case 3:
            file_copy_lines(bfp, tfp, lower);
            break;

    case 4:
            file_skip_lines(bfp, upper + 1);
            file_copy_lines(bfp, tfp, 99999);
            break;
   
    case 5:
            file_skip_lines(bfp, upper );
	    if (lower == -1) {
	     /* single line keep */
             file_copy_lines(bfp, tfp, 2);
	    }
	    else {
             file_copy_lines(bfp, tfp, (lower - upper) + 2 );
	    }
            break;

    case 6:
            file_copy_lines(bfp, tfp, lower);
            file_skip_lines(bfp, (upper - lower) + 2 );
            file_copy_lines(bfp, tfp, 99999);
            break;
            
    default: break;
  }
            
FCLOSE(bfp);
FCLOSE(tfp);
    
remove(file);
            
/*-----------------------------------------*/
/* copy temp file back into file           */
/*-----------------------------------------*/
            
if (!(bfp=fopen(file,"w")))
  {
   return;  
  }
   
if (!(tfp=fopen(temp,"r")))
  {
   FCLOSE(bfp);
   return;
  }
            
file_copy(tfp, bfp);

FCLOSE(bfp);
FCLOSE(tfp);
remove(temp);
   
}


/*---------------------------------------------*/
/* skip the number of lines specified          */
/*---------------------------------------------*/
void file_skip_lines(FILE *in_file, int lines)
{
int cnt = 1;
char c;     

 while( cnt < lines )
   {
    c=getc(in_file);
    if (feof(in_file)) return;
    if (c == '\n') cnt++;
   }
}  


/*-----------------------------------------------------------------*/
/* copy the number of lines specified from a file to a file        */
/*-----------------------------------------------------------------*/
void file_copy_lines(FILE *in_file, FILE *out_file, int lines)
{
int cnt = 1;
char c;
 
 while( cnt < lines )
   {
    c=getc(in_file);
    if (feof(in_file)) return;
    
    putc(c, out_file);
    if (c == '\n') cnt++;
   }
}


/*---------------------------------------------*/
/* copy a file to another file                 */
/*---------------------------------------------*/
void file_copy(FILE *in_file, FILE *out_file)
{
char c;
 c=getc(in_file);
 while( !feof(in_file) )
   {
    putc(c,out_file);
    c=getc(in_file);  
   }
}


/*---------------------------------------------*/
/* count lines in a file                       */
/*---------------------------------------------*/
int file_count_lines(char *file)
{
 int lines = 0;
 char c[257];
 FILE *bfp;
 
    
 if (!(bfp=fopen(file,"r")))
   {
    return 0;
   }
 
 fgets(c, 256, bfp);

 while( !feof(bfp) )
   {
    if (strchr(c, '\n') != NULL) lines ++;
    fgets(c, 256, bfp);
   }
 
 FCLOSE(bfp);
 return lines;
}


/*---------------------------------------------*/
/* count bytes in a file                       */
/*---------------------------------------------*/
int file_count_bytes(char *file, int mode, int mode2)
{
 unsigned int bytes = 0;
 int lines = 0;
 int lines_read = 0;
 char c[257];
 FILE *bfp;
 
if (mode==0) {
/* count total bytes */
 
 if (!(bfp=fopen(file,"r")))
   {
    return 0;
   }

 fgets(c, 256, bfp);
 bytes = strlen(c);
 while( !feof(bfp) )
   {
    fgets(c, 256, bfp);
    bytes += strlen(c);
   }

 FCLOSE(bfp);
 return bytes;
 
}   
else {
/* if mode2 is 0, count bytes in last X rows */
/* if mode2 is not 0, count bytes in last rows X to Y */
 
 if (!(lines=file_count_lines(file))) {
        return 0;   
 }  
    
 if (!(bfp=fopen(file,"r")))
   {
    return 0;
   }
 
 if (mode2) file_skip_lines(bfp, mode+1);
 else file_skip_lines(bfp, (lines-mode)+1);

 while( !feof(bfp) )
   {
    fgets(c, 256, bfp);
/*     sprintf(mess,"READIN %s",c);
     write_str(0,mess); */
    bytes += strlen(c);
    lines_read++;
     if (mode2) {
      if (lines_read == (mode2-mode)) break;
     }
   }
 
 FCLOSE(bfp);
 return bytes;

}
    
}


