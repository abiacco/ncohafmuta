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

extern int bot;
extern char mess[ARR_SIZE+25];
extern char t_mess[ARR_SIZE+25];
extern char web_opts[11][64];
extern char *our_delimiters;
extern struct command_struct sys[];
extern struct botcommand_struct botsys[];


/*---------------------------------------*/
/* Get a temporary file for data storage */
/*---------------------------------------*/
char *get_temp_file()
{
static char tempname[FILE_NAME_LEN];

sprintf(tempname,"junk/temp_%d.%d",rand()%500,rand()%500);
return tempname;
}


/* Get the abbreviation number for the emote command */
int get_emote(int user)
{
int i=0;
 
for (i=0;i<NUM_ABBRS;++i) {
    if (!strcmp(ustr[user].custAbbrs[i].com,".emote")) {
      return i;
     }
    else continue;
   }

return -1;
}   


/* Get time in a certain way and return it as a string */
/* mode 0 is to get rid of the year string and the carriage return */
/* mode 1 is to get rid of just the carriage return */ 
char *get_time(time_t ref,int mode)
{
time_t tm;
static char mrtime[30];

if ((int)ref==0) {
   time(&tm);
   strcpy(mrtime,ctime(&tm));
  }
else {
   strcpy(mrtime,ctime(&ref));
  }

if (mode==0)
   mrtime[strlen(mrtime)-6]=0; /* get rid of newline and year */
else if (mode==1) 
   mrtime[strlen(mrtime)-1]=0; /* get rid of newline */
 
   return mrtime;
}
   

char *get_error(void)
{
static char errstr[256];

sprintf(errstr,"(%d:%s)",errno,strerror(errno));
return errstr;
}


/*** get user number using name ***/
int get_user_num(char *i_name, int user)
{
int u;
int found = 0, last = -1;

strtolower(i_name);
        
t_mess[0] = 0;
   
for (u=0; u<MAX_USERS; ++u)
        if ( !strcmp(ustr[u].name,i_name) && ustr[u].area != -1)
        return u;
  
for (u=0; u<MAX_USERS; ++u)
  {
   if (instr2(0, ustr[u].name, i_name, 0) != -1)
    {
      strcat(t_mess, ustr[u].say_name);
      strcat(t_mess, " ");
      found++;
      last= u;
    }
  }

if (found == 0) return -1;
   
if (found >1)
  {
   sprintf(mess, NOT_UNIQUE, t_mess);
   write_str(user,mess);
   return -1; 
  }  
 else
  return last;
}


/*** get user number using name ***/
int get_user_num_exact(char *i_name, int user)
{
int u;

strtolower(i_name);
t_mess[0] = 0;

for (u=0;u<MAX_USERS;++u)
        if (!strcmp(ustr[u].name,i_name) && ustr[u].area != -1)
        return u;

return -1;
}


/*---------------------------------------------------------------*/
/* this is a simple token parser. I, Cygnus, have made it more   */
/* complex (of course). No, really, it's cool. It takes abbrevia */
/* -tions or/and the first command on and checks to see if it is */
/* a valid command. And NOW since command structure abbreviation */
/* defaults are copied to a new user, it looks at them instead   */
/*---------------------------------------------------------------*/
int get_com_num(int user, char *inpstr)
{
char comstr[ARR_SIZE];  
char tstr[ARR_SIZE+25];
int f=0;
int i=0;
int found=0;
  
if (ustr[user].white_space)
 {
  while(inpstr[0] == ' ') inpstr++;
 }
                        
comstr[0]=inpstr[0];
comstr[1]=0;
                                 
/* Bot commands start with a _ */
if (!strcmp(ustr[user].name,BOT_ID) ||
    !strcmp(ustr[user].name,ROOT_ID)) {
  if (!strcmp(comstr,"_")) {
    sscanf(inpstr,"%s",comstr);
    for (f=0; botsys[f].jump_vector != -1; ++f)
      if (!instr2(0,botsys[f].command,comstr,0) && strlen(comstr)>1) return f;
    return -1;
   }
 } /* end of if bot */
      
if (!strcmp(comstr,"."))  
 {
   sscanf(inpstr,"%s",comstr);
 }   
 else
 {
   if (ustr[user].abbrs)
     {

     /* check user abbreviations first */
                        for (found=0, i=0;i<NUM_ABBRS && found == 0;++i)
                        {
                              if (comstr[0] == ustr[user].custAbbrs[i].abbr[0])
                                {
                                        strcpy(comstr, ustr[user].custAbbrs[i].com);
                                        found = 1;
                                }
                        }   
                        if (!found)
                        {
                                return -1;
                        }
    
      tstr[0] = 0;
      inpstr[0] = ' ';
      strcpy(tstr,comstr);
      strcat(tstr,inpstr);
      strcpy(inpstr,tstr);
    }
 }   
  
for (f=0; sys[f].su_com != -1; ++f)
        if (!instr2(0,sys[f].command,comstr,0) && strlen(comstr)>1) return f;
return -1;
}


int get_com_num_plain(char *inpstr)
{
int f=0;

for (f=0; sys[f].su_com != -1; ++f)
        if (!instr2(0,sys[f].command,inpstr,0) && strlen(inpstr)>1) return sys[f].jump_vector;
        
return -1;
}
 

int get_rank(char *inpstr)
{
int f=0;

for (f=0; sys[f].su_com != -1; ++f)
        if (!instr2(0,sys[f].command,inpstr,0) && strlen(inpstr)>1) return sys[f].su_com;
        
return 0;
}

/*---------------------------------------*/
/* Figure out full help filename from    */
/* inputed abbreviation..                */
/* something like get_user_num()         */
/*---------------------------------------*/
char *get_help_file(char *i_name, int user)
{
int found = 0;
static char last[64];
char name[ARR_SIZE];
char small_buff[64];
char z_mess[600];
char filerid[FILE_NAME_LEN];
struct dirent *dp;  
DIR  *dirp;
         
strcpy(z_mess,"");
strcpy(last,"");
strcpy(name,i_name);
          
 sprintf(t_mess,"%s",HELPDIR);
 strncpy(filerid,t_mess,FILE_NAME_LEN);
         
 dirp=opendir((char *)filerid);
    
 if (dirp == NULL)
   {
    write_str(user,"Directory information not found.");
    write_log(ERRLOG,YESTIME,"Directory information not found for directory \"%s\" in get_help_file %s\n",
    filerid,get_error());
    strcpy(last,"failed2");
    return last;
   }
    
 while ((dp = readdir(dirp)) != NULL)
   {
    sprintf(small_buff,"%s",dp->d_name);
    if (small_buff[0]!='.') {
       if (!strcmp(small_buff,name))
        {
         found = -1;
         break;
        }
       if ((instr2(0, small_buff, name, 1) != -1) && (found != -1))
        {
          strcat(z_mess, small_buff);
          strcat(z_mess, " ");
          found++;
          strcpy(last,small_buff);
        }
    }  
    small_buff[0]=0;
   }       /* End of while */
   
 if (found == -1) {
    small_buff[0]=0;
    strcpy(last,name);
    (void) closedir(dirp);
    return last;
    }
    
 if (found == 0) {
    (void) closedir(dirp);   
    strcpy(last,"failed");
    return last;
    }

if (found >1)
  {
   sprintf(mess, "String was not unique, matched: %s", z_mess);
   write_str(user,mess);
   (void) closedir(dirp);
   strcpy(last,"failed2");
   return last;
  }
 else {
  (void) closedir(dirp);
  return last;
  }
}


char *get_username_from_delims(char *str, int pos) {
int j=0;
static char a_mess[SAYNAME_LEN+2];

/* possible length plus 2 delimiters */
midcpy(str,a_mess,pos,SAYNAME_LEN+2);
for (j=0;j<strlen(a_mess);++j) {
        if (a_mess[j]==our_delimiters[0] && a_mess[j+1]==our_delimiters[1])
                break;
}
midcpy(a_mess,a_mess,0,j-1);
return a_mess;
}


char *get_reaction(char *inpstr) {

switch(PARSE_END_PUNC) {
case 0:
	return ""; break;
case 1:
	return (SAY_REACTION); break;
case 2:
	if (inpstr[strlen(inpstr)-1]=='?') return(QUESTION_REACTION);
	else if (inpstr[strlen(inpstr)-1]=='!') return(EXCLAMATION_REACTION);
	else return(SAY_REACTION);
	break;
default:
	return ""; break;
} /* end of switch */

return(SAY_REACTION);
}


/* Get length/size of output file */
int get_length(char *filen)
{
int size=0,n=0;
char line[257];
FILE *fp;

line[0]=0;
           
if (!(fp=fopen(filen,"rb"))) {
 write_log(ERRLOG,YESTIME,"Can't open binary file(rb) \"%s\" in get_length! %s\n",filen,     
 get_error());
 return -1;
}

while ((n = fread(line, 1, sizeof(line), fp)) != 0) {
        size+=n;
} /* end of while */
 
fclose(fp);

return size;
}


/* Get type of file requested by extension */
char *get_mime_type(char *filen)
{       
int i=0,found=0,count=1;
char ext[10];
char fileext[10];
char buf1[81];
char mime[80];  
char filename[256];
static char type[40];
FILE *tfp;
        
mime[0]=0;
buf1[0]=0;
ext[0]=0;
fileext[0]=0;
        
/* Find where extension ends to the left */
if (strstr(filen,".")) {
for (i=strlen(filen)-1;i!=0;--i) {
        if (filen[i]=='.') break;
   }
}
else {
        strcpy(type,"text/plain");
        return type;
}
 
/* Were not gonna take extra long extensions */
if ((strlen(filen)-i) > 9) {
        strcpy(type,"bad");
        return type;
        }
        
/* Copy the extension to memory */
midcpy(filen,ext,i+1,ARR_SIZE);   
i=0;
        
strtolower(ext);

sprintf(t_mess,"%s/%s",WEBFILES,web_opts[1]);
strncpy(filename,t_mess,FILE_NAME_LEN);
        
/* Open up the mime types file and read in each line, searching */
/* for a matching extension, and returning the associated type  */
if (!(tfp=fopen(filename,"r"))) {
        write_log(ERRLOG,YESTIME,"WWW: Couldn't open file \"%s\" in get_mime_type! %s\n",filename,get_error());
        strcpy(type,"text/plain");
        return type;
        }

while (fgets(buf1,80,tfp) != NULL) {
        buf1[strlen(buf1)-1]=0;
        if (!strlen(buf1)) continue;
        if (buf1[0]=='#') continue;
        sscanf(buf1,"%s",mime);
        remove_first(buf1);
        /* Count up spaces in extension part ot see how many we get */
        /* to check for a match */
        for (i=0;i<strlen(buf1);++i) {
        if (buf1[i]==' ') count++;
        } /* end of for */
        i=0;
        /* Now scan them in one by one and try to match */
        for (i=0;i<count;++i) {
        sscanf(buf1,"%s",fileext);
        remove_first(buf1);
        if (!strcmp(ext,fileext)) { strcpy(type,mime); found=1; break; }
        } /* end of for */
        i=0; count=1; mime[0]=0; fileext[0]=0;
        if (!found) {
          /* No match, bummer! well return a default */
          strcpy(type,"text/plain");
          }
        else {
          /* Ah ha! Jackpot! */
          break;
          }
        buf1[0]=0;
  } /* end of while */
fclose(tfp);
i=0;
found=0;
count=1;
        
return type;
}


/* get value of a particular key in a web request keypair */
char *get_value(char *keypair, char *ukey) {
int found1=0,a=0,b=0,done=0;   
static char keypair_temp[ARR_SIZE];
        
        while (!done) {
          for (;a<strlen(keypair);++a) {
             if (keypair[a]=='=') { found1=1; break; }
             }
          if (found1) {
          /* found = sign, check if key we're looking for */
          found1=0;
          midcpy(keypair,keypair_temp,b,a-1);
          if (!strcmp(keypair_temp,ukey)) done=1;
          else {
                /* no key here, go to ampersand and continue in loop */
                /* if no ampersand, end of keypair, return NULL  */
                a++;  
                for (;a<strlen(keypair);++a) {
                if (keypair[a]=='&') { found1=1; break; }
                }
                if (!found1) return NULL;
                else { found1=0; a++; b=a; continue; }
           } /* end of else */
          } /* end of found if */
          else {
                /* didn't find a = sign in the keypair! return NULL */
                return NULL;   
                }
        } /* end of while */
        
/* found key in keypair, get value for it up */
/* to ampersand or end of keypair            */
done=0;
a++;
b=a;
for (;a<strlen(keypair);++a) {
 if (keypair[a]=='&') { if (strcmp(ukey,"password")) break; }
}
          
midcpy(keypair,keypair_temp,b,a-1);
found1=0;
a=0;
b=0;
                
if (strlen(keypair_temp))
 return keypair_temp;
else if (!strlen(keypair_temp) && (!strcmp(ukey,"username") ||
!strcmp(ukey,"password") || !strcmp(ukey,"search") ||
!strcmp(ukey,"option") || !strcmp(ukey,"change") ) )
 return NULL;   
else
 return keypair_temp;
}


/*-----------------------------------*/
/* get a random number based on rank */ 
/*-----------------------------------*/
int get_odds_value(int user)
{
return( (rand() % ranks[ustr[user].super].odds) + 1 );
}


/*---------------------------------------------------------*/
/* get flags position                                      */
/*---------------------------------------------------------*/
int get_flag_num(char *inpstr)
{
char comstr[ARR_SIZE];
int f;

sscanf(inpstr,"%s",comstr);
if (strlen(comstr)<2) return(-1);

for (f=0; flag_names[f].text[0]; ++f)
  {
   if (!instr2(0,flag_names[f].text,comstr,0) )
     return f;
  }

return -1;
}


/*-----------------------------------------------------*/
/* Read an int with return from a file                 */
/*-----------------------------------------------------*/
int get_int(FILE *f)
{
int val1=0;
char temp[1000];

temp[0]=0;
if (fgets((char *)temp,1000,f) == NULL) {
        return 0;
        }

temp[strlen(temp)-1]=0;
sscanf(temp,"%d",&val1);
return val1;
}


/*-----------------------------------------------------*/
/* Read a char with return from a file                 */
/*-----------------------------------------------------*/
char get_onechar(FILE *f) {
char char1;
char temp[1000];

temp[0]=0;
if (fgets((char *)temp,1000,f) == NULL) {
        return 0;
        }
         
temp[strlen(temp)-1]=0;
sscanf(temp,"%c",&char1);
return char1;
}


/*-----------------------------------------------------*/
/* Read a long value with return from a file           */
/*-----------------------------------------------------*/
long get_long(FILE *f)
{
long val2=0;
char temp[1000];

temp[0]=0;
if (fgets((char *)temp,1000,f) == NULL) {
        return 0;
        }
         
temp[strlen(temp)-1]=0;
sscanf(temp,"%ld",&val2);
return val2;
}


/*-----------------------------------------------------*/
/* Read a string with return from a file               */
/*-----------------------------------------------------*/
void get_buf(FILE *f, char *buf2, int buflen)
{
char temp[ARR_SIZE];

temp[0]=0;
if (fgets((char *)temp,ARR_SIZE,f) == NULL) { 
        return;
        }
   
temp[strlen(temp)-1]=0;

if (temp[strlen(temp)-2]=='@' && temp[strlen(temp)-1]=='@')
	temp[strlen(temp)-2]=0;

if (buflen != -1) {
  if (strlen(temp) >= buflen)
    temp[buflen-1]=0;
 }

strcpy(buf2,temp);
}
