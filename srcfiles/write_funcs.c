#if defined(HAVE_CONFIG_H)
#include "../hdrfiles/config.h"
#endif

#include "../hdrfiles/includes.h"

#include "../hdrfiles/osdefs.h"
#include "../hdrfiles/constants.h"
#include "../hdrfiles/protos.h"

extern int bot;
extern int syslog_on;
extern char mess[ARR_SIZE+25];
char *our_delimiters="!|";

void write_cygnus(int user)
{

sprintf(mess,"Flag 0: %s",flag_names[0].text);
write_str(user,mess);
sprintf(mess,"Flag 1: %s",flag_names[1].text);
write_str(user,mess);
sprintf(mess,"Bot   : %d",bot);
write_str(user,mess);
sprintf(mess,"name  : %s",ustr[user].say_name);
write_str(user,mess);

}


/*--------------------------------*/
/*   Write out a hilited string   */
/*--------------------------------*/
void write_hilite(int user, char *str)
{ 
char str2[ARR_SIZE];

if (ustr[user].hilite)
  {
   strcpy(str2,str);
   add_hilight(str2);
   write_str(user,str2);
  }
 else
  {
   write_str(user,str);
  }

}

/*--------------------------------------------------------------------------*/
/* write out a raw data (ascii and non-ascii) no special processing         */
/*--------------------------------------------------------------------------*/
void write_raw(int user, unsigned char *str, int len)
{
/* S_WRITE(ustr[user].sock, str, len); */
queue_write(user,(char *)str,-1);
}


/*** Write a NULL terminated string to a socket ***/
void write_it(int sock, char *str)
{
S_WRITE(sock, str, strlen(str));
}


/*--------------------------------------------------------*/
/*   Write out a hilited string with no carriage return   */
/*--------------------------------------------------------*/
void write_hilite_nr(int user, char *str)
{ 
char str2[ARR_SIZE];

if (ustr[user].hilite)
  {
    strcpy(str2,str);
    add_hilight(str2);
    write_str_nr(user,str2);
  }
 else
  {
   write_str_nr(user,str);
  }

}

/*------------------------------------------------*/
/* Send a string to the bot..this calls write_str */
/*------------------------------------------------*/
void write_bot(char *fmt)
{
if (bot!=-5) write_str(bot, fmt);
}

/*------------------------------------------------------------------*/
/* Send a string to the bot without the CR..this calls write_str_nr */
/*------------------------------------------------------------------*/
void write_bot_nr(char *fmt)
{
if (bot!=-5) write_str_nr(bot, fmt);
}

/*----------------------------------------*/
/*   write_str sends string down socket   */
/*----------------------------------------*/
void write_str(int user, char *str)
{
char buff[500], tp[3], hi_on[10], hi_off[10];
char tempst[ARR_SIZE];
int  stepper,num=0;
int  left=strlen(str);
int  i, count=0, number_of_carets=0;

/* Check for bot write */
/* if (!strcmp(ustr[user].name,BOT_ID)) return; */

/*--------------------------------------------------------*/
/* pick reasonable range for width                        */
/*--------------------------------------------------------*/

if (ustr[user].cols < 15 || ustr[user].cols > 256) 
  stepper = 80;
 else
  stepper = ustr[user].cols;
  
/*-------------------------------------------*/
/* Convert string to hilited if carets exist */
/*-------------------------------------------*/
if (ustr[user].hilite || ustr[user].color) {
        strcpy(hi_on, "\033[1m");
        strcpy(hi_off, "\033[0m");
        }
else {
        strcpy(hi_on, "");
        strcpy(hi_off, "");
        }

tempst[0]=0;

for(i=0; i<left; i++) {
        if (str[i]==' ') {
                strcat(tempst, " ");
                continue;
                }
        if (str[i]=='@' && str[i+1]=='@') { i++; continue; }
        if (str[i]==our_delimiters[0] && str[i+1]==our_delimiters[1]) {
	if (!user_wants_message(user,COLORNAME)) {
	strcpy(buff,get_username_from_delims(str,i+2));
/* write_log(ERRLOG,YESTIME,"doing strip_color in write_str\n"); */
	strcat(tempst,strip_color(buff));
	/* increments positioner username length plus delimiters + 1 */
	i+=strlen(buff)+3;
	}
	else {
	/* user wants color */
		if (!strcmp(ustr[user].name,BOT_ID)) {
		/* we are writing to the bot, so we have to */
		/* send the string out with the carets, not the */
		/* translated version */
		strcpy(buff,get_username_from_delims(str,i+2));
		if (ustr[user].color) strcat(tempst,buff);
		else strcat(tempst,strip_color(buff));
		/* increments positioner username length plus delimiters + 1 */
		i+=strlen(buff)+3;
		}
		else i++;
	} /* end of wants color else */
	continue;
	} /* end of username check */

        if (str[i]=='^') {
	number_of_carets++;
                if (count) {
                        strcat(tempst, hi_off);
                        count=0;
                        continue;
                        }
                else {
                        count=1;
                        i++;
                         if (i == left) break;
                 if (str[i]=='H') {
                    i++;
                     if (i == left) break;
                     if (str[i]=='R') {
                       if (ustr[user].color)
                        strcat(tempst,"\033[1;31m");
                       else
                        strcat(tempst,hi_on);
                      }
                     else if (str[i]=='G') {
                       if (ustr[user].color)
                        strcat(tempst,"\033[1;32m");
                       else
                        strcat(tempst,hi_on);
                       }
                     else if (str[i]=='Y') {
                       if (ustr[user].color)
                        strcat(tempst,"\033[1;33m");
                       else
                        strcat(tempst,hi_on);
                       }
                     else if (str[i]=='B') {
                       if (ustr[user].color)
                        strcat(tempst,"\033[1;34m");
                       else
                        strcat(tempst,hi_on);
                       }
                     else if (str[i]=='M') {
                       if (ustr[user].color)
                        strcat(tempst,"\033[1;35m");
                       else
                        strcat(tempst,hi_on);
                       }
                     else if (str[i]=='C') {
                       if (ustr[user].color)
                        strcat(tempst,"\033[1;36m");
                       else
                        strcat(tempst,hi_on);
                       }
                     else if (str[i]=='W') {
                       if (ustr[user].color)
                        strcat(tempst,"\033[1;37m");
                       else
                        strcat(tempst,hi_on);
                       }
                     else { i--; tp[0]=str[i]; tp[1]=0;
                            strcat(tempst,hi_on);
                            strcat(tempst,tp);
                            }
                   }
                 else if (str[i]=='L') {
                    i++;
                     if (i == left) break;
                     if (str[i]=='R') {
                       if (ustr[user].color)
                        strcat(tempst,"\033[31m");
                       else
                        strcat(tempst,hi_on);
                      }
                     else if (str[i]=='G') {
                       if (ustr[user].color)
                        strcat(tempst,"\033[32m");
                       else
                        strcat(tempst,hi_on);
                       }
                     else if (str[i]=='Y') {
                       if (ustr[user].color)
                        strcat(tempst,"\033[33m");
                       else
                        strcat(tempst,hi_on);
                       }
                     else if (str[i]=='B') {
                       if (ustr[user].color)
                        strcat(tempst,"\033[34m");
                       else
                        strcat(tempst,hi_on);
                       }
                     else if (str[i]=='M') {
                       if (ustr[user].color)
                        strcat(tempst,"\033[35m");
                       else
                        strcat(tempst,hi_on);
                       }
                     else if (str[i]=='C') {
                       if (ustr[user].color)
                        strcat(tempst,"\033[36m");
                       else
                        strcat(tempst,hi_on);
                       }
                     else if (str[i]=='W') {
                       if (ustr[user].color)
                        strcat(tempst,"\033[37m");
                       else
                        strcat(tempst,hi_on);
                       }
                     else { i--; tp[0]=str[i]; tp[1]=0;
                            strcat(tempst,hi_on);
                            strcat(tempst,tp);
                            }
                   }
                 else if (str[i]=='B') {
                    i++;
                     if (i == left) break;
                     if (str[i]=='L') {
                   if (ustr[user].color && 
                       strcmp(ustr[user].name,"llo"))
                        strcat(tempst,"\033[5m");
                       else
                        strcat(tempst,hi_on);
                      }
                     else { i--; tp[0]=str[i]; tp[1]=0;
                            strcat(tempst,hi_on);
                            strcat(tempst,tp);
                            }
                   }
                 else if (str[i]=='U') {
                    i++;
                     if (i == left) break;
                     if (str[i]=='L') {
                       if (ustr[user].color)
                        strcat(tempst,"\033[4m");
                       else
                        strcat(tempst,hi_on);
                      }
                     else { i--; tp[0]=str[i]; tp[1]=0;
                            strcat(tempst,hi_on);
                            strcat(tempst,tp);
                            }
                   }
                 else if (str[i]=='R') {
                    i++;
                     if (i == left) break;
                     if (str[i]=='V') {
                       if (ustr[user].color)
                        strcat(tempst,"\033[7m");
                       else
                        strcat(tempst,hi_on);
                      }
                     else { i--; tp[0]=str[i]; tp[1]=0;
                            strcat(tempst,hi_on);
                            strcat(tempst,tp);
                            }
                    }
                 else { tp[0]=str[i]; tp[1]=0;
                        strcat(tempst,hi_on);
                        strcat(tempst,tp);
                        }
                        continue;
                    }  /* end of count else */
                }  /* end of if caret */
        tp[0]=str[i];
        tp[1]=0;
        strcat(tempst, tp);
        }
if (number_of_carets%2) strcat(tempst,hi_off);
count=0;
i=0;

if (left == 0 )
  {
/*
   if (ustr[user].car_return && ustr[user].afk<2) 
     S_WRITE(ustr[user].sock, "\r\n", 2);
    else
     S_WRITE(ustr[user].sock, "\n", 1);
*/
   if (ustr[user].car_return && ustr[user].afk<2) 
     queue_write(user, "\r\n", -1);
    else
     queue_write(user, "\n", -1);
   return;
  }

buff[0]=0;
num=0;

  str=tempst;
  left=strlen(str);

for(; left > 0; left -= stepper )
  {
   strncpy(buff, str, stepper);
      
   str += stepper;
   buff[stepper] = 0;
   
   if (ustr[user].car_return) 
     strcat(buff,"\r\n");
    else
     strcat(buff,"\n");
  
   if (ustr[user].afk<2)
      queue_write(user, buff, -1);
  
   }

buff[0]=0;
tempst[0]=0;
num=0;
}

/*--------------------------------------------------------------------------*/
/* write out a string to a user with no carriage return (note: this should  */
/* do a call to write_str but doesnt.)  This is currently like write_raw    */
/* but should be like write_str.                                            */
/*--------------------------------------------------------------------------*/
void write_str_nr(int user, char *str)
{
char tp[3], hi_on[10], hi_off[10], buff[500];
char tempst[ARR_SIZE];
int  left=strlen(str);
int  i, count=0, number_of_carets=0;

/* Check for bot write */
/* if (!strcmp(ustr[user].name,BOT_ID)) return; */

/*-------------------------------------------*/
/* Convert string to hilited if carets exist */
/*-------------------------------------------*/
if (ustr[user].hilite || ustr[user].color) {
        strcpy(hi_on, "\033[1m");
        strcpy(hi_off, "\033[0m");
        }
else {
        strcpy(hi_on, "");
        strcpy(hi_off, "");
        }

tempst[0]=0;

for(i=0; i<left; i++) {
        if (str[i]==' ') {
                strcat(tempst, " ");
                continue;
                }
        if (str[i]=='@' && str[i+1]=='@') { i++; continue; }
        if (str[i]==our_delimiters[0] && str[i+1]==our_delimiters[1]) {
	if (!user_wants_message(user,COLORNAME)) {
	strcpy(buff,get_username_from_delims(str,i+2));
/* write_log(ERRLOG,YESTIME,"doing strip_color in write_str_nr\n"); */
	strcat(tempst,strip_color(buff));
	/* increments positioner username length plus delimiters + 1 */
	i+=strlen(buff)+3;
	}
	else {
	/* user wants color */
		if (!strcmp(ustr[user].name,BOT_ID)) {
		/* we are writing to the bot, so we have to */
		/* send the string out with the carets, not the */
		/* translated version */
		strcpy(buff,get_username_from_delims(str,i+2));
		if (ustr[user].color) strcat(tempst,buff);
		else strcat(tempst,strip_color(buff));
		/* increments positioner username length plus delimiters + 1 */
		i+=strlen(buff)+3;
		}
		else i++;
	} /* end of wants color else */
	continue;
	} /* end of username check */
        if (str[i]=='^') {
	number_of_carets++;
                if (count) {
                        strcat(tempst, hi_off);
                        count=0;
                        continue;
                        }
                else {
                        count=1;
                        i++;
                         if (i == left) break;
                 if (str[i]=='H') {
                    i++;
                     if (i == left) break;
                     if (str[i]=='R') {
                       if (ustr[user].color)
                        strcat(tempst,"\033[1;31m");
                       else
                        strcat(tempst,hi_on);
                      }
                     else if (str[i]=='G') {
                       if (ustr[user].color)
                        strcat(tempst,"\033[1;32m");
                       else
                        strcat(tempst,hi_on);
                       }
                     else if (str[i]=='Y') {
                       if (ustr[user].color)
                        strcat(tempst,"\033[1;33m");
                       else
                        strcat(tempst,hi_on);
                       }
                     else if (str[i]=='B') {
                       if (ustr[user].color)
                        strcat(tempst,"\033[1;34m");
                       else
                        strcat(tempst,hi_on);
                       }
                     else if (str[i]=='M') {
                       if (ustr[user].color)
                        strcat(tempst,"\033[1;35m");
                       else
                        strcat(tempst,hi_on);
                       }
                     else if (str[i]=='C') {
                       if (ustr[user].color)
                        strcat(tempst,"\033[1;36m");
                       else
                        strcat(tempst,hi_on);
                       }
                     else if (str[i]=='W') {
                       if (ustr[user].color)
                        strcat(tempst,"\033[1;37m");
                       else
                        strcat(tempst,hi_on);
                       }
                     else { i--; tp[0]=str[i]; tp[1]=0;
                            strcat(tempst,hi_on);
                            strcat(tempst,tp);
                            }
                   }
                 else if (str[i]=='L') {
                    i++;
                     if (i == left) break;
                     if (str[i]=='R') {
                       if (ustr[user].color)
                        strcat(tempst,"\033[31m");
                       else
                        strcat(tempst,hi_on);
                      }
                     else if (str[i]=='G') {
                       if (ustr[user].color)
                        strcat(tempst,"\033[32m");
                       else
                        strcat(tempst,hi_on);
                       }
                     else if (str[i]=='Y') {
                       if (ustr[user].color)
                        strcat(tempst,"\033[33m");
                       else
                        strcat(tempst,hi_on);
                       }
                     else if (str[i]=='B') {
                       if (ustr[user].color)
                        strcat(tempst,"\033[34m");
                       else
                        strcat(tempst,hi_on);
                       }
                     else if (str[i]=='M') {
                       if (ustr[user].color)
                        strcat(tempst,"\033[35m");
                       else
                        strcat(tempst,hi_on);
                       }
                     else if (str[i]=='C') {
                       if (ustr[user].color)
                        strcat(tempst,"\033[36m");
                       else
                        strcat(tempst,hi_on);
                       }
                     else if (str[i]=='W') {
                       if (ustr[user].color)
                        strcat(tempst,"\033[37m");
                       else
                        strcat(tempst,hi_on);
                       }
                     else { i--; tp[0]=str[i]; tp[1]=0;
                            strcat(tempst,hi_on);
                            strcat(tempst,tp);
                            }
                   }
                 else if (str[i]=='B') {
                    i++;
                     if (i == left) break;
                     if (str[i]=='L') {
                   if (ustr[user].color && 
                       strcmp(ustr[user].name,"llo"))
                        strcat(tempst,"\033[5m");
                       else
                        strcat(tempst,hi_on);
                      }
                     else { i--; tp[0]=str[i]; tp[1]=0;
                            strcat(tempst,hi_on);
                            strcat(tempst,tp);
                            }
                   }
                 else if (str[i]=='U') {
                    i++;
                     if (i == left) break;
                     if (str[i]=='L') {
                       if (ustr[user].color)
                        strcat(tempst,"\033[4m");
                       else
                        strcat(tempst,hi_on);
                      }
                     else { i--; tp[0]=str[i]; tp[1]=0;
                            strcat(tempst,hi_on);
                            strcat(tempst,tp);
                            }
                   }
                 else if (str[i]=='R') {
                    i++;
                     if (i == left) break;
                     if (str[i]=='V') {
                       if (ustr[user].color)
                        strcat(tempst,"\033[7m");
                       else
                        strcat(tempst,hi_on);
                      }
                     else { i--; tp[0]=str[i]; tp[1]=0;
                            strcat(tempst,hi_on);
                            strcat(tempst,tp);
                            }
                    }
                 else { tp[0]=str[i]; tp[1]=0;
                        strcat(tempst,hi_on);
                        strcat(tempst,tp);
                        }
                        continue;
                    }
                }
        tp[0]=str[i];
        tp[1]=0;
        strcat(tempst, tp);
        } 
if (number_of_carets%2) strcat(tempst,hi_off);

count=0;
i=0;

if (ustr[user].afk<2)
     queue_write(user, tempst, -1);

tempst[0]=0;
}


/*--------------------------------------------------------------------------*/
/* write out a string to a web users queue with no carriage return          */
/*--------------------------------------------------------------------------*/
void write_str_www(int user, char *str, int size)
{

/* Check for bot write */
if (!strcmp(ustr[user].name,BOT_ID) && bot==-5) return;

queue_write_www(user, str, size);
}


/* Write string and arguments to a specific logging facility */
void write_log(int type, int wanttime, char *str, ...)
{
char z_mess[ARR_SIZE*2];
char logfile[FILE_NAME_LEN];
va_list args;
FILE *fp;

if (!syslog_on) return;

z_mess[0]=0;

sprintf(z_mess,logfacil[type].file,LOGDIR);
strncpy(logfile,z_mess,FILE_NAME_LEN);

 if (!(fp=fopen(logfile,"a")))
   {
    sprintf(z_mess,"%s LOGGING: Couldn't open file(a) \"%s\"! %s",STAFF_PREFIX,logfile,get_error());
    writeall_str(z_mess, WIZ_ONLY, -1, 0, -1, BOLD, NONE, 0);
    return;
   }
 else {
    va_start(args,str);
    if (wanttime) {
    sprintf(z_mess,"%s: ",get_time(0,0));
    vsprintf(z_mess+strlen(z_mess),str,args);
    }
    else
    vsprintf(z_mess,str,args);

    va_end(args);
    strcpy(z_mess, strip_color(z_mess));
    fputs(z_mess,fp);
    FCLOSE(fp);
   }
}


/* Strip colors from string */
char *strip_color(char *str)
{
char tp[3];
static char stripped[ARR_SIZE*2];
int  left=strlen(str);
int  i, count=0;

stripped[0]=0;

for(i=0; i<left; i++) {
        if (str[i]==' ') {
                strcat(stripped, " ");
                continue;
                }
	if (str[i]==our_delimiters[0] && str[i+1]==our_delimiters[1]) { i++; continue; }
      if (str[i]=='@') { 
                i++;
                if (str[i]=='@') {
                 count=0; continue;
                }
                else { i--;
                       tp[0]=str[i]; tp[1]=0;
                       strcat(stripped,tp);
                       continue;
                     }
                }
        if (str[i]=='^') {
                if (count) {
                        count=0;
                        continue;
                        }
                else {
                        count=1;
                        i++;
                         if (i == left) {
                            count=0;
                            break;
                           }
                 if (str[i]=='H') {
                    i++;
                     if (i == left) {
                            count=0;
                            break;
                           }
                     if (str[i]=='R') {
                       continue;
                      }
                     else if (str[i]=='G') {
                       continue;
                       }
                     else if (str[i]=='Y') {
                       continue;
                       }
                     else if (str[i]=='B') {
                       continue;
                       }
                     else if (str[i]=='M') {
                       continue;
                       }
                     else if (str[i]=='C') {
                       continue;
                       }
                     else if (str[i]=='W') {
                       continue;
                       }
                     else { i--; tp[0]=str[i]; tp[1]=0;
                            strcat(stripped,tp);
                            }
                   }
                 else if (str[i]=='L') {
                    i++;
                     if (i == left) {
                            count=0;
                            break;
                           }
                     if (str[i]=='R') {
                        continue;
                      }
                     else if (str[i]=='G') {
                        continue;
                       }
                     else if (str[i]=='Y') {
                        continue;
                       }
                     else if (str[i]=='B') {
                        continue;
                       }
                     else if (str[i]=='M') {
                        continue;
                       }
                     else if (str[i]=='C') {
                        continue;
                       }
                     else if (str[i]=='W') {
                        continue;
                       }
                     else { i--; tp[0]=str[i]; tp[1]=0;
                            strcat(stripped,tp);
                            }
                   }
                 else if (str[i]=='B') {
                    i++;
                     if (i == left) {
                            count=0;
                            break;
                           }
                     if (str[i]=='L') {
                        continue;
                      }
                     else { i--; tp[0]=str[i]; tp[1]=0;
                            strcat(stripped,tp);
                            }
                   }
                 else if (str[i]=='U') {
                    i++;
                     if (i == left) {
                            count=0;
                            break;
                           }
                     if (str[i]=='L') {
                        continue;
                      }
                     else { i--; tp[0]=str[i]; tp[1]=0;
                            strcat(stripped,tp);
                            }
                   }
                 else if (str[i]=='R') {
                    i++;
                     if (i == left) {
                            count=0;
                            break;
                           }
                     if (str[i]=='V') {
                        continue;
                      }
                     else { i--; tp[0]=str[i]; tp[1]=0;
                            strcat(stripped,tp);
                            }
                    }
                 else { tp[0]=str[i]; tp[1]=0;
                        strcat(stripped,tp);
                        }
                        continue;
                    }
                }
        tp[0]=str[i];
        tp[1]=0;
        strcat(stripped, tp);
        } 
count=0;
i=0;

return stripped;
}

/* Convert color codes to HTML color codes */
char *convert_color(char *str)
{
char tp[3], hi_on[10], hi_off[10], font_off[10];
char blink_on[10], blink_off[10], under_on[10], under_off[10];
static char tempst[500];
int  left=strlen(str);
int  i, count=0, space=0, font=0, hi=0, blink=0, under=0;

/*---------------------------------------------*/
/* Convert string to bold if just carets exist */
/*---------------------------------------------*/
strcpy(hi_on, "<b>");
strcpy(hi_off, "</b>");
strcpy(font_off,"</font>");
strcpy(blink_on,"<blink>");
strcpy(blink_off,"</blink>");
strcpy(under_on,"<u>");
strcpy(under_off,"</u>");

tempst[0]=0;

for(i=0; i<left; i++) {
        if (str[i]==' ') {
		if (space)
                strcat(tempst, "&nbsp;");
		else
		strcat(tempst, " ");
		space=1;
                continue;
                }
	else space=0;

        if (str[i]=='@' && str[i+1]=='@') { i++; continue; }
	if (str[i]==our_delimiters[0] && str[i+1]==our_delimiters[1]) { i++; continue; }
        if (str[i]=='^') {
                if (count) {
		if (hi)
                 strcat(tempst,hi_off);
		if (font)
                 strcat(tempst,font_off);
		if (blink)
                 strcat(tempst,blink_off);
		if (under)
                 strcat(tempst,under_off);
                        count=0;
			font=0;
			hi=0;
                        continue;
                        }
                else {
                        count=1;
                        i++;
                         if (i == left) {
			if (hi)
       		          strcat(tempst,hi_off);
			if (font)
       		          strcat(tempst,font_off);
			if (blink)
	       	          strcat(tempst,blink_off);
			if (under)
       		          strcat(tempst,under_off);
                        count=0;
			font=0;
			hi=0;
                            break;
                           }
                 if (str[i]=='H') {
                    i++;
                     if (i == left) {
			if (hi)
       		          strcat(tempst,hi_off);
			if (font)
       		          strcat(tempst,font_off);
			if (blink)
	       	          strcat(tempst,blink_off);
			if (under)
       		          strcat(tempst,under_off);
                        count=0;
			font=0;
			hi=0;
                            break;
                           }
                     if (str[i]=='R') {
			strcat(tempst,"<font color=\"red\">");
			font=1;
                      }
                     else if (str[i]=='G') {
			strcat(tempst,"<font color=\"#00FF00\">");
			font=1;
                       }
                     else if (str[i]=='Y') {
			strcat(tempst,"<font color=\"yellow\">");
			font=1;
                       }
                     else if (str[i]=='B') {
			strcat(tempst,"<font color=\"#0000FF\">");
			font=1;
                       }
                     else if (str[i]=='M') {
			strcat(tempst,"<font color=\"#FF00FF\">");
			font=1;
                       }
                     else if (str[i]=='C') {
			strcat(tempst,"<font color=\"cyan\">");
			font=1;
                       }
                     else if (str[i]=='W') {
			strcat(tempst,"<font color=\"white\">");
			strcat(tempst,hi_on);
			font=1;
			hi=1;
                       }
                     else { i--; tp[0]=str[i]; tp[1]=0;
                            strcat(tempst,hi_on);
                            strcat(tempst,tp);
			    hi=1;
                            }
                   }
                 else if (str[i]=='L') {
                    i++;
                     if (i == left) {
			if (hi)
       		          strcat(tempst,hi_off);
			if (font)
       		          strcat(tempst,font_off);
			if (blink)
	       	          strcat(tempst,blink_off);
			if (under)
       		          strcat(tempst,under_off);
                        count=0;
			font=0;
			hi=0;
                            break;
                           }
                     if (str[i]=='R') {
			strcat(tempst,"<font color=\"#8E2323\">");
			font=1;
                      }
                     else if (str[i]=='G') {
			strcat(tempst,"<font color=\"green\">");
			font=1;
                       }
                     else if (str[i]=='Y') {
			strcat(tempst,"<font color=\"#CFB53B\">");
			font=1;
                       }
                     else if (str[i]=='B') {
			strcat(tempst,"<font color=\"#3232CD\">");
			font=1;
                       }
                     else if (str[i]=='M') {
			strcat(tempst,"<font color=\"#9932CD\">");
			font=1;
                       }
                     else if (str[i]=='C') {
			strcat(tempst,"<font color=\"#7093DB\">");
			font=1;
                       }
                     else if (str[i]=='W') {
			strcat(tempst,"<font color=\"white\">");
			font=1;
                       }
                     else { i--; tp[0]=str[i]; tp[1]=0;
                            strcat(tempst,hi_on);
                            strcat(tempst,tp);
			    hi=1;
                            }
                   }
                 else if (str[i]=='B') {
                    i++;
                     if (i == left) {
			if (hi)
       		          strcat(tempst,hi_off);
			if (font)
       		          strcat(tempst,font_off);
			if (blink)
	       	          strcat(tempst,blink_off);
			if (under)
       		          strcat(tempst,under_off);
                        count=0;
			font=0;
			hi=0;
                            break;
                           }
                     if (str[i]=='L') {
                        strcat(tempst,blink_on);
			blink=1;
                      }
                     else { i--; tp[0]=str[i]; tp[1]=0;
                            strcat(tempst,hi_on);
                            strcat(tempst,tp);
			    hi=1;
                            }
                   }
                 else if (str[i]=='U') {
                    i++;
                     if (i == left) {
			if (hi)
       		          strcat(tempst,hi_off);
			if (font)
       		          strcat(tempst,font_off);
			if (blink)
	       	          strcat(tempst,blink_off);
			if (under)
       		          strcat(tempst,under_off);
                        count=0;
			font=0;
			hi=0;
                            break;
                           }
                     if (str[i]=='L') {
                        strcat(tempst,under_on);
			under=1;
                      }
                     else { i--; tp[0]=str[i]; tp[1]=0;
                            strcat(tempst,hi_on);
                            strcat(tempst,tp);
			    hi=1;
                            }
                   }
                 else if (str[i]=='R') {
                    i++;
                     if (i == left) {
			if (hi)
       		          strcat(tempst,hi_off);
			if (font)
       		          strcat(tempst,font_off);
			if (blink)
	       	          strcat(tempst,blink_off);
			if (under)
       		          strcat(tempst,under_off);
                        count=0;
			font=0;
			hi=0;
                            break;
                           }
                     if (str[i]=='V') {
                        strcat(tempst,hi_on);
			hi=1;
                      }
                     else { i--; tp[0]=str[i]; tp[1]=0;
                            strcat(tempst,hi_on);
                            strcat(tempst,tp);
			    hi=1;
                            }
                    }
                 else { tp[0]=str[i]; tp[1]=0;
                        strcat(tempst,hi_on);
                        strcat(tempst,tp);
			hi=1;
                        }
                        continue;
                    }
                }
        tp[0]=str[i];
        tp[1]=0;
        strcat(tempst, tp);
        } 
count=0;
i=0;

return tempst;
}


/* queue data to go out in the user's output_data pointer */
void queue_write(int user, char *queue_str, int length)
{

if (length==-1) length = strlen(queue_str);

#if defined(QWRITE_DEBUG)
write_log(DEBUGLOG,NOTIME,"QW : Want to write %d chars to buffer\n",length);
#endif
if (ustr[user].alloced_size) {
#if defined(QWRITE_DEBUG)
	write_log(DEBUGLOG,NOTIME,"QW : reallocing data of current size %d (malloced: %d)\n",strlen(ustr[user].output_data),ustr[user].alloced_size);
	write_log(DEBUGLOG,NOTIME,"QW : to new malloced size %d\n",ustr[user].alloced_size+length);
#endif
realloc_str( &ustr[user].output_data, ustr[user].alloced_size + length);
}
else {
  ustr[user].output_data	= NULL;
  ustr[user].write_offset	= 0;
  ustr[user].alloced_size	= 0;
  ustr[user].output_data	= (char *)malloc(length+1);
#if defined(QWRITE_DEBUG)
  write_log(DEBUGLOG,NOTIME,"QW : QUEUED-NEW %d chars to buffer\n",length);
#endif
  }
(void) memcpy( &(ustr[user].output_data[ustr[user].alloced_size]), queue_str, length );
ustr[user].alloced_size += length;

}


/* user's socket is writable, start writing output_data pointer to socket */
int queue_flush(int user)
{
int cnt=0;

if (!ustr[user].alloced_size) return 1;

#if defined(QFLUSH_DEBUG)
write_log(DEBUGLOG,NOTIME,"QF : Trying to FLUSH %d chars, of data size %d\n",ustr[user].alloced_size - ustr[user].write_offset,ustr[user].alloced_size);
#endif

cnt = S_WRITE(ustr[user].sock,
	      ustr[user].output_data + ustr[user].write_offset,
	      ustr[user].alloced_size - ustr[user].write_offset);
if (cnt >= (ustr[user].alloced_size - ustr[user].write_offset)) {
#if defined(QFLUSH_DEBUG)
	write_log(DEBUGLOG,NOTIME,"QF : Flushed ALL %d chars\n",cnt);
#endif
	free(ustr[user].output_data);
	ustr[user].output_data	= NULL;
	ustr[user].alloced_size	= ustr[user].write_offset = 0;
  }
else if (cnt < 0) {
	write_log(ERRLOG,YESTIME,"ERRNO %s on pid %u sock %d in queue_flush writing %d chars for %s! %s\n",
		get_error(),(unsigned int)getpid(),ustr[user].sock,ustr[user].alloced_size - ustr[user].write_offset,ustr[user].say_name,ustr[user].output_data);
	if (errno == EWOULDBLOCK || errno == EAGAIN)
	  return 1;                       
	free(ustr[user].output_data);
	ustr[user].output_data	= NULL;
	ustr[user].alloced_size	= ustr[user].write_offset = 0;
	return 0;
  }
else {
#if defined(QFLUSH_DEBUG)
	write_log(DEBUGLOG,NOTIME,"QF : Flushed SOME %d chars of %d\n",cnt,ustr[user].alloced_size);
#endif
  ustr[user].write_offset += cnt;

#if defined(QFLUSH_DEBUG)
write_log(DEBUGLOG,NOTIME,"QF: %d chars left\n",ustr[user].alloced_size-ustr[user].write_offset);
#endif
  return 1;
  }
return 1;
}


/* queue data to go out in the web port's output_data pointer */
void queue_write_www(int user, char *new_str, int length)
{

if (length==-1) length = strlen(new_str);

#if defined(QWRITE_DEBUG)
write_log(DEBUGLOG,NOTIME,"QWW: Want to write %d chars to buffer\n",length);
#endif
if (wwwport[user].alloced_size) {
#if defined(QWRITE_DEBUG)
	write_log(DEBUGLOG,NOTIME,"QWW: reallocing data of current size %d (malloced: %d)\n",strlen(wwwport[user].output_data),wwwport[user].alloced_size);
	write_log(DEBUGLOG,NOTIME,"QWW: to new malloced size %d\n",wwwport[user].alloced_size+length);
#endif
realloc_str( &wwwport[user].output_data, wwwport[user].alloced_size + length);
}
else {
  wwwport[user].output_data	= NULL;
  wwwport[user].write_offset	= 0;
  wwwport[user].alloced_size	= 0;
  wwwport[user].output_data	= (char *)malloc(length+1);
#if defined(QWRITE_DEBUG)
  write_log(DEBUGLOG,NOTIME,"QWW: QUEUED-NEW %d chars to buffer\n",length);
#endif
  }
(void) memcpy( &(wwwport[user].output_data[wwwport[user].alloced_size]), new_str, length );
wwwport[user].alloced_size += length;

}


/* web port's socket is writable, start writing output_data pointer to socket */
int queue_flush_www(int user)
{
int cnt=0;

if (!wwwport[user].alloced_size) return 1;

#if defined(QFLUSH_DEBUG)
write_log(DEBUGLOG,NOTIME,"QFW: Trying to FLUSH %d chars, data size %d\n",wwwport[user].alloced_size - wwwport[user].write_offset,wwwport[user].alloced_size);
#endif

cnt = S_WRITE(wwwport[user].sock,
	      wwwport[user].output_data + wwwport[user].write_offset,
	      wwwport[user].alloced_size - wwwport[user].write_offset);
if (cnt >= (wwwport[user].alloced_size - wwwport[user].write_offset)) {
#if defined(QFLUSH_DEBUG)
	write_log(DEBUGLOG,NOTIME,"QFW: Flushed ALL %d chars\n",cnt);
#endif
	free(wwwport[user].output_data);
	wwwport[user].output_data  = NULL;
	wwwport[user].alloced_size = wwwport[user].write_offset = 0;
	free_sock(user,'4');
  }
else if (cnt < 0) {
	write_log(ERRLOG,YESTIME,"ERRNO %s on pid %u in queue_flush_www writing %d chars!\n",
		get_error(),(unsigned int)getpid(),wwwport[user].alloced_size - wwwport[user].write_offset);
	if (errno == EWOULDBLOCK || errno == EAGAIN)
	  return 1;
	free(wwwport[user].output_data);
	wwwport[user].output_data  = NULL;
	wwwport[user].alloced_size = wwwport[user].write_offset = 0;
	free_sock(user,'4');
	return 0;
  }
else {
#if defined(QFLUSH_DEBUG)
	write_log(DEBUGLOG,NOTIME,"QFW: Flushed SOME %d chars of %d\n",cnt,wwwport[user].alloced_size);
#endif
  wwwport[user].write_offset += cnt;

#if defined(QFLUSH_DEBUG)
write_log(DEBUGLOG,NOTIME,"QFW: %d chars left\n",wwwport[user].alloced_size - wwwport[user].write_offset);
#endif
  return 1;
  }
return 1;
}


/*** sends output to all users if area==0                       ***/
/*** else only users in same area                               ***/
/*----------------------------------------------------------------*/
/* str  - what to print                                           */
/* area - -1 = login, -2 = pre-login, -5 = wizards only,          */
/*         0 and above - any room                                 */
/* user - the one who did it                                      */
/* send_to_user = 0 all on system 1 = in room                     */
/* who_did = user                                                 */
/* mode = normal - 0, bold = 1                                    */
/* type = message type                                            */
/*----------------------------------------------------------------*/

void writeall_str(char *str, int area, int user, int send_to_user, int who_did, int mode, int type, int sw)
{
int u,i=0,z=0;
int gagged=0,gravoked=0;
char str2[ARR_SIZE];

str2[0]=0;

/* we do this because in a muzzle the victim is the username in	*/
/* the string, not the user who DID the command			*/
/*
if (type == MUZZLE) strcpy(name,strip_color(ustr[user].say_name));
else strcpy(name,strip_color(ustr[who_did].say_name));
*/

/* if (str[0]!=' ') str[0]=toupper(str[0]); */

/*---------------------------------------*/
/* added for btell                       */
/*---------------------------------------*/

 if (area == WIZ_ONLY)
   {
     for (u=0;u<MAX_USERS;++u)
       {
        if (!user_wants_message(u,type)) continue;

/* WRITE CODE TO CHECK THIS FOR ONLY TYPE OF WIZT */
if (type == WIZT) {
/* Check if command was revoked from user */
 for (z=0;z<MAX_GRAVOKES;++z) {
	if (!is_revoke(ustr[u].revokes[z])) continue;
	if (strip_com(ustr[u].revokes[z])==get_com_num_plain(".wiztell")) {
		gravoked=1; break;
	}
   }
if (gravoked==1) { gravoked=0; continue; }
gravoked=0;
z=0;

/* Check if command was granted to user */
 if (ustr[u].super < WIZ_LEVEL && !ustr[u].logging_in && u != user && ustr[u].area!=-1) {
  for (z=0;z<MAX_GRAVOKES;++z) {
        if (!is_grant(ustr[u].revokes[z])) continue;
        if (strip_com(ustr[u].revokes[z])==get_com_num_plain(".wiztell")) {
                gravoked=1; break;
          }
  } /* end of for */
 } /* end of if lower level */
} /* end of if WIZT */
        
	if (((ustr[u].super >= WIZ_LEVEL) || (gravoked==1)) && !ustr[u].logging_in && u != user && ustr[u].area!=-1)  
	  {
	   if (mode == BOLD)
	     {
	       write_hilite(u,str);
	     }
	    else
	     {
	      write_str(u,str);
	     }
          } /* end of if wiz level */
	gravoked=0;
        } /* end of for */
    return;
   } /* end of if area */
   
/*---------------------------------------*/
/* normal write to all users             */
/*---------------------------------------*/
   
for (u=0; u<MAX_USERS; ++u) {
str2[0]=0;
        if ((ustr[u].logging_in==11) || (ustr[u].logging_in==12)) continue;

	if ((!send_to_user && user==u) ||  ustr[u].area== -1) continue;
	    
	if (!user_wants_message(u,type)) continue;
    
        if (!strcmp(ustr[user].mutter,ustr[u].say_name)) continue;

        /* An incoming connection message for monitoring people */
        if (area == -2) {
           if (ustr[u].monitor > 1) {
            if (mode == BOLD)
             write_hilite(u,str);
            else
             write_str(u,str);
            }
           continue;
          } /* end of if area == -2 */

        /* A normal login */
        if (type == LOGIO)
           {
                write_str(u,str);
                continue;
           }

        /* See if u has user gagged */
        for (i=0; i<NUM_IGN_FLAGS; ++i) {
           if (type==gagged_types[i]) {
             if (!check_gag(user,u,1)) { gagged=1; break; }
             }
          } /* end of for */
        if (gagged) {
          gagged=0; i=0;
          continue;
          }
        i=0;

        if (ustr[u].area==ustr[user].area || !area)  
	  { 
	    if ((ustr[u].monitor==1) || (ustr[u].monitor==3)) {
		if (!ustr[who_did].vis || 
		     type == ECHOM     || 
		     type == BCAST     || 
		     type == KILL      || 
		     type == MOVE      || 
		     type == PICTURE   ||
		     type == GREET)
		  {
		   sprintf(str2,"<%s> ",
			mode==BOLD?strip_color(ustr[who_did].say_name):ustr[who_did].say_name);
		  }
		else strcpy(str2,"");

	    strcat(str2,str);
	    } /* end of monitoring if */
	    else {
	    strcpy(str2,str);
	    } /* end of else not monitoring */

	       if (mode == BOLD)
	         {
	          write_hilite(u,str2);
	         }
	        else if (mode == BEEPS)
	         {
                  if (user_wants_message(u,BEEPS))
                   strcat(str2,"\07");
	          write_str(u,str2);
	         }
	        else
	         {
	          write_str(u,str2);
	         }

	  } /* end of if same area */
	} /* end of main user for */
}

/* CYGNUS1 */
/*-----------------------------------------------------*/
/* Conversation buffer routines                        */
/*-----------------------------------------------------*/
void addto_conv_buffer(ConvPtr buf, char *str)
{
        if (buf == NULL)
        {
                return;
        }

        strncpy(buf->conv[buf->count],str,MAX_LINE_LEN);
        buf->count = (++(buf->count))%NUM_LINES;
}

void init_conv_buffer(ConvPtr buf)
{
        int i;

        if (buf == NULL)
        {
                return;
        }

        buf->count = 0;
        for (i=0;i<NUM_LINES;++i)
        {
                buf->conv[i][0] = 0;
        }
}

void write_conv_buffer(int user, ConvPtr buf)
{
        int pos, f, new_pos = 0;

        if (buf == NULL)
        {
                write_str(user," There is no buffer due to memory exhaustion");
                return;
        }

        pos = (buf->count)%NUM_LINES;
        for (f=0;f<NUM_LINES;++f)
        {
                if (strlen(buf->conv[pos]) )
                {
                        write_str(user,buf->conv[pos]);
                }
                new_pos = ( ++pos ) % NUM_LINES;
		pos = new_pos;
        }
}

void init_macro_buffer(MacroPtr buf)
{
        int i;

        if (buf == NULL) return;

        for (i=0;i<NUM_MACROS;++i)
        {
                buf->name[i][0]=0;
                buf->body[i][0]=0;
        }
}

