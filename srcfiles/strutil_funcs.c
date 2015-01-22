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

extern char *our_delimiters;



/*** put string terminate char. at first char < 32 ***/
void terminate(int user, char *str)
{
int u;
int bell = 7;
int tab  = 9;

/*----------------------------------------------------------------*/
/* only allow cntl-g from users rank >= WIZ_LEVEL                 */
/*----------------------------------------------------------------*/

if (ustr[user].super < WIZ_LEVEL) bell = tab;

for (u = 0; u<ARR_SIZE; ++u)  
  {
   if ((*(str+u) < 32 &&       /* terminate line on first control char */
       *(str+u) != bell &&     /* except for bell                      */     
       *(str+u) != tab) ||     /* and tab                              */
       *(str+u) > 126  )       /* special chars over 126               */
     {
      *(str+u)=0; 
      u=ARR_SIZE;
     }
  }
}


/*** convert string to lower case ***/
void strtolower(char *str)
{
while(*str) 
  { 
    *str=tolower((int)*str);
    str++; 
  }
}



/*** check for empty string ***/
int nospeech(char *str)
{
while(*str) 
  {  
    if (*str > ' ')  
       return 0;  
    str++;  
  }
return 1;
}

/*--------------------------------------------------------------------*/
/* This function converts minutes into days hours minutes for output  */
/*--------------------------------------------------------------------*/
char *converttime(long mins)
{
int d,h,m;
static char convstr[35];

d=(int)mins/1440;
m=(int)mins%1440;
h=m/60;
m%=60;

sprintf(convstr,"%d day%s %d hour%s %d minut%s",
        d,d == 1 ? "," : "s,",
        h,h == 1 ? "," : "s,",
        m,m == 1 ? "e" : "es");
return convstr;
}

/* Count colors in a string for correct line formatting		*/
/* if mode 0 just count the codes				*/
/* if mode 1 count space that will be used my replacing codes	*/
int count_color(char *str, int mode)
{
int found=0,i=0,count=0;
int left=strlen(str);

	for(i=0; i<left; i++) {
        if (str[i]==' ') {
                continue;
                }
	if (str[i]==our_delimiters[0] && str[i+1]==our_delimiters[1]) { i++; count+=2; continue; }
        if (str[i]=='@') { 
                i++;
                if (str[i]=='@') {
		 if (!mode) count+=2;
		 else count+=4;
		 found=0;
                 continue;
                }
                else { i--;
                       continue;
                     }
                }
        if (str[i]=='^') {
                if (found) {
                        found=0;
			 if (!mode) count++;
			 else count+=4;
                        continue;
                        }
                else {
                        found=1;
			i++;
                 if (str[i]=='H') {
                    i++;
                     if (i == left) {
			 if (!mode) count++;
			 else count+=4;
                         found=0;
                            break;
                           }
                     if (str[i]=='R') {
			if (!mode) count+=3;
			else count+=7;
                       continue;
                      }
                     else if (str[i]=='G') {
			if (!mode) count+=3;
			else count+=7;
                       continue;
                       }
                     else if (str[i]=='Y') {
			if (!mode) count+=3;
			else count+=7;
                       continue;
                       }
                     else if (str[i]=='B') {
			if (!mode) count+=3;
			else count+=7;
                       continue;
                       }
                     else if (str[i]=='M') {
			if (!mode) count+=3;
			else count+=7;
                       continue;
                       }
                     else if (str[i]=='C') {
			if (!mode) count+=3;
			else count+=7;
                       continue;
                       }
                     else if (str[i]=='W') {
			if (!mode) count+=3;
			else count+=7;
                       continue;
                       }
                     else {
				i--;
			 if (!mode) count++;
			 else count+=4;
			  }
                   }
                 else if (str[i]=='L') {
                    i++;
                     if (i == left) {
			 if (!mode) count++;
			 else count+=4;
                         found=0;
                            break;
                           }
                     if (str[i]=='R') {
			if (!mode) count+=3;
			else count+=7;
                        continue;
                      }
                     else if (str[i]=='G') {
			if (!mode) count+=3;
			else count+=7;
                        continue;
                       }
                     else if (str[i]=='Y') {
			if (!mode) count+=3;
			else count+=7;
                        continue;
                       }
                     else if (str[i]=='B') {
			if (!mode) count+=3;
			else count+=7;
                        continue;
                       }
                     else if (str[i]=='M') {
			if (!mode) count+=3;
			else count+=7;
                        continue;
                       }
                     else if (str[i]=='C') {
			if (!mode) count+=3;
			else count+=7;
                        continue;
                       }
                     else if (str[i]=='W') {
			if (!mode) count+=3;
			else count+=7;
                        continue;
                       }
                     else {
				i--;
			 if (!mode) count++;
			 else count+=4;
                          }
                   }
                 else if (str[i]=='B') {
                    i++;
                     if (i == left) {
			 if (!mode) count++;
			 else count+=4;
                         found=0;
                            break;
                           }
                     if (str[i]=='L') {
			if (!mode) count+=3;
			else count+=7;
                        continue;
                      }
		     else {
			i--; 
			 if (!mode) count++;
			 else count+=4;
			}
                   }
                 else if (str[i]=='U') {
                    i++;
                     if (i == left) {
			 if (!mode) count++;
			 else count+=4;
                         found=0;
                            break;
                           }
                     if (str[i]=='L') {
			if (!mode) count+=3;
			else count+=7;
                        continue;
                      }
		     else {
			i--;
			 if (!mode) count++;
			 else count+=4;
			}
                   }
                 else if (str[i]=='R') {
                    i++;
                     if (i == left) {
			 if (!mode) count++;
			 else count+=4;
                         found=0;
                            break;
                           }
                     if (str[i]=='V') {
			if (!mode) count+=3;
			else count+=7;
                        continue;
                      }
		     else {
			i--;
			 if (!mode) count++;
			 else count+=4;
			}
                    }
                 else {
			 if (!mode) count++;
			 else count+=4;
                      }
		} /* end of found else */
            } /* end if if ^ */
        } /* end of for */
found=0;

	return count;
}


/* strip command number from gravoke struct */
int strip_com(char *str)
{
char junkstr[4];
int junk=0;

junkstr[0]=0;
sscanf(str,"%s ",junkstr);
junk=atoi(junkstr);
return junk;
}

/* strip level to make from gravoke struct */
int strip_level(char *str)
{
char junkstr[4];
char junkmain[NAME_LEN];
int junk=0;

junkstr[0]=0;
junkmain[0]=0;
strcpy(junkmain,str);
remove_first(junkmain); /* com num */
remove_first(junkmain); /* the + sign */
sscanf(junkmain,"%s",junkstr);
junk=atoi(junkstr);
return junk;
}


/*** removes first word at front of string and moves rest down ***/
void remove_first(char *inpstr)
{
int newpos,oldpos;

newpos=0;  oldpos=0;
/* find first word */
while(inpstr[oldpos]==' ') {
        if (!inpstr[oldpos]) { inpstr[0]=0;  return; }
        oldpos++;
        }
/* find end of first word */
while(inpstr[oldpos]!=' ') {
        if (!inpstr[oldpos]) { inpstr[0]=0;  return; }
        oldpos++;
        }
/* find second word */
while(inpstr[oldpos]==' ') {
        if (!inpstr[oldpos]) { inpstr[0]=0;  return; }
        oldpos++;
        }
while(inpstr[oldpos]!=0)
        inpstr[newpos++]=inpstr[oldpos++];
inpstr[newpos]='\0';
}


/* mid copy copies chunk from string strf to string strt */
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


/*----------------------------------------------------------*/
/* given two string: ss and sf, and a position in ss,       */
/* determine if sf is present in ss                         */
/* return position if it is present, or -1 if it is not     */
/*                                                          */
/* note: this is obscure, but functional code               */
/*----------------------------------------------------------*/
int instr2(int pos, char *ss, char *sf, int mode)
{
int f;
int g;
int nofirst=0;

for (f=pos; *(ss+f); ++f)
  {
        for (g=0;;++g)
                {
                if (*(sf+g)=='\0' && g>0)
                  {
                  return f;
                  }

                if (*(sf+g)!=*(ss+f+g))
                  {
                   if (mode==1 && f==0)
                    nofirst=1;
                   break;
                  }
                }
    if (mode==1 && nofirst==1) break;
   }
return -1;
}


char *time_format_1(time_t dt)
{
  register struct tm *delta;
  static char buf[64];
  if (dt < 0)
    dt = 0;

  delta = gmtime(&dt);

if (delta->tm_min > 0) {
    sprintf(buf, "%3s%s %02d:%02d",
            delta->tm_yday>0?itoa((int)delta->tm_yday):"",
        delta->tm_yday>0?"d":" ",
        delta->tm_hour, delta->tm_min);
}
else {
sprintf(buf, "%7s%2ds","", delta->tm_sec);

}

  return buf;
}


char *time_format_2(time_t dt)
{
  register struct tm *delta;
  static char buf[64];
  if (dt < 0)
    dt = 0;

  delta = gmtime(&dt);
  if (delta->tm_yday > 0) {
    sprintf(buf, "%3dd", delta->tm_yday);
  } else if (delta->tm_hour > 0) {
    sprintf(buf, "%3dh", delta->tm_hour);
  } else if (delta->tm_min > 0) {
    sprintf(buf, "%3dm", delta->tm_min);
  } else {
    sprintf(buf, "%3ds", delta->tm_sec);
  }
  return buf;
}


#if defined(OPENBSD_SYS)
char *ctime(const time_t *mytm)
{
struct tm *clocker;
static char buf1[32];

clocker=localtime(mytm);
#if defined(HAVE_STRFTIME)
strftime(buf1,sizeof(buf1),"%a %b %d %H:%M:%S %Y\n",clocker);
#else
snprintf(buf1,sizeof(buf1),"%s %s %d %.2d:%.2d:%.2d %d\n",
clocker_daylist[clocker->tm_wday],clocker_monthlist[clocker->tm_mon],
clocker->tm_mday,clocker->tm_hour,clocker->tm_min,clocker->tm_sec,
1900+clocker->tm_year);
#endif

return buf1;
}
#endif


char *my_strptime(char *buf, char *fmt, struct tm *tmStruct)
{
  char c;
  char* ptr;
  int i;
  int len=0;
struct dtconv {
  char  *abbrev_month_names[12];
  char  *month_names[12];
  char  *abbrev_weekday_names[7];
  char  *weekday_names[7];
  char  *time_format;
  char  *sdate_format;
  char  *dtime_format;
  char  *am_string;
  char  *pm_string;
  char  *ldate_format;
};
struct dtconv    US_English_fmt = {
  { "Jan", "Feb", "Mar", "Apr", "May", "Jun",
    "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" },
  { "January", "February", "March", "April",
    "May", "June", "July", "August",
    "September", "October", "November", "December" },
  { "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat" },
  { "Sunday", "Monday", "Tuesday", "Wednesday",
    "Thursday", "Friday", "Saturday" },
  "%H:%M:%S",
  "%m/%d/%y",
  "%a %b %e %T %Z %Y",
  "AM",
  "PM",
  "%A, %B, %e, %Y"
};


  ptr = fmt;
  while (*ptr != 0)
  {
    if (*buf == 0)
      break;

    c = *ptr++;

    if (c != '%')
    {
      if (isspace((int)c))
        while (*buf != 0 && isspace((int)*buf))
          buf++;
      else if (c != *buf++)
        return 0;
      continue;
    }

    c = *ptr++;
    switch (c)
    {
    case 0:
    case '%':
      if (*buf++ != '%')
        return 0;
      break;

    case 'C':
      buf = my_strptime(buf, US_English_fmt.ldate_format, tmStruct);
      if (buf == 0)
        return 0;
      break;

    case 'c':
      buf = my_strptime(buf, "%x %X", tmStruct);
      if (buf == 0)
        return 0;               /*  */
      break;

    case 'D':
      buf = my_strptime(buf, "%m/%d/%y", tmStruct);
      if (buf == 0)
        return 0;
      break;

    {
      i *= 10;
      i += *buf - '0';
    }
      if (i > 365)
        return 0;

      tmStruct->tm_yday = i;
      break;

    case 'M':
    case 'S':
      if (*buf == 0 || isspace((int)*buf))
        break;

      if (!isdigit((int)*buf))
        return 0;

      for (i = 0; *buf != 0 && isdigit((int)*buf); buf++)
      {
        i *= 10;
        i += *buf - '0';
      }
      if (i > 59)
        return 0;

      if (c == 'M')
        tmStruct->tm_min = i;
      else
        tmStruct->tm_sec = i;

      if (*buf != 0 && isspace((int)*buf))
        while (*ptr != 0 && !isspace((int)*ptr))
          ptr++;
      break;

    case 'H':
    case 'I':
    case 'k':
    case 'l':
      if (!isdigit((int)*buf))
        return 0;

      for (i = 0; *buf != 0 && isdigit((int)*buf); buf++)
      {
        i *= 10;
        i += *buf - '0';
      }
      if (c == 'H' || c == 'k')
      {
        if (i > 23)
          return 0;
      } else if (i > 11)
        return 0;

      tmStruct->tm_hour = i;

      if (*buf != 0 && isspace((int)*buf))
        while (*ptr != 0 && !isspace((int)*ptr))
          ptr++;
      break;

    case 'p':
      len = strlen(US_English_fmt.am_string);
      if (strncasecmp(buf, US_English_fmt.am_string, len) == 0)
      {
        if (tmStruct->tm_hour > 12)
          return 0;
        if (tmStruct->tm_hour == 12)
          tmStruct->tm_hour = 0;
        buf += len;
        break;
      }

      len = strlen(US_English_fmt.pm_string);
      if (strncasecmp(buf, US_English_fmt.pm_string, len) == 0)
      {
        if (tmStruct->tm_hour > 12)
          return 0;
        if (tmStruct->tm_hour != 12)
          tmStruct->tm_hour += 12;
        buf += len;
        break;
      }

      return 0;

    case 'A':
    case 'a':
      for (i = 0; i < asizeof(US_English_fmt.weekday_names); i++)
      {
        len = strlen(US_English_fmt.weekday_names[i]);
        if (strncasecmp(buf,
                        US_English_fmt.weekday_names[i],
                        len) == 0)
          break;

        len = strlen(US_English_fmt.abbrev_weekday_names[i]);
        if (strncasecmp(buf,
                        US_English_fmt.abbrev_weekday_names[i],
                        len) == 0)
          break;
      }
      if (i == asizeof(US_English_fmt.weekday_names))
        return 0;

      tmStruct->tm_wday = i;
      buf += len;
      break;

    case 'd':
    case 'e':
      if (!isdigit((int)*buf))
        return 0;

      for (i = 0; *buf != 0 && isdigit((int)*buf); buf++)
      {
        i *= 10;
        i += *buf - '0';
      }
      if (i > 31)
        return 0;

      tmStruct->tm_mday = i;

      if (*buf != 0 && isspace((int)*buf))
        while (*ptr != 0 && !isspace((int)*ptr))
          ptr++;
      break;

    case 'B':
    case 'b':
    case 'h':
      for (i = 0; i < asizeof(US_English_fmt.month_names); i++)
      {
        len = strlen(US_English_fmt.month_names[i]);
        if (strncasecmp(buf,
                        US_English_fmt.month_names[i],
                        len) == 0)
          break;

        len = strlen(US_English_fmt.abbrev_month_names[i]);
        if (strncasecmp(buf,
                        US_English_fmt.abbrev_month_names[i],
                        len) == 0)
          break;
      }
      if (i == asizeof(US_English_fmt.month_names))
        return 0;

      tmStruct->tm_mon = i;
      buf += len;
      break;

    case 'm':
      if (!isdigit((int)*buf))
        return 0;

      for (i = 0; *buf != 0 && isdigit((int)*buf); buf++)
      {
        i *= 10;
        i += *buf - '0';
      }
      if (i < 1 || i > 12)
        return 0;

      tmStruct->tm_mon = i - 1;

      if (*buf != 0 && isspace((int)*buf))
        while (*ptr != 0 && !isspace((int)*ptr))
          ptr++;
      break;

    case 'Y':
    case 'y':
      if (*buf == 0 || isspace((int)*buf))
        break;

      if (!isdigit((int)*buf))
        return 0;

      for (i = 0; *buf != 0 && isdigit((int)*buf); buf++)
      {
        i *= 10;
        i += *buf - '0';
      }
      if (c == 'Y')
        i -= 1900;
      if (i < 0)
        return 0;

      tmStruct->tm_year = i;

      if (*buf != 0 && isspace((int)*buf))
        while (*ptr != 0 && !isspace((int)*ptr))
          ptr++;
      break;
    }
  }

  return buf;
}


#if !defined(HAVE_STRERROR)
char *strerror(int err)
{
        static char     buf[64];
# if defined(HAVE_SYS_ERRLIST)
#  if !defined(SYS_ERRLIST_DECLARED)
        extern int      sys_nerr;
        extern char     *sys_errlist[];
#  endif
        char            *p;

        if (err < 0 || err >= sys_nerr)
                snprintf(p = buf, sizeof(buf), "Unknown system error %d", err);
        else
                p = sys_errlist[err];
        return p;
# else /* HAVE_SYS_ERRLIST */

#if defined(WINDOWS) && !defined(__CYGWIN32__)
switch(err) {
  case WSAEINTR: return "Interrupted system call";
  case WSAEBADF: return "Bad file number";
  case WSAEACCES: return "Permission denied";
  case WSAEFAULT: return "Bad Address";
  case WSAEINVAL: return "Invalid argument";
  case WSAEMFILE: return "Too many open files";
  case WSAEWOULDBLOCK: return "Operation would block";
  case WSAEINPROGRESS: return "Operation now in progress";
  case WSAEALREADY: return "Operation already in progress";
  case WSAENOTSOCK: return "Socket operation on nonsocket";
  case WSAEDESTADDRREQ: return "Destination address required";
  case WSAEMSGSIZE: return "Message too long";
  case WSAEPROTOTYPE: return "Protocol wrong type for socket";
  case WSAENOPROTOOPT: return "Protocol not available";
  case WSAEPROTONOSUPPORT: return "Protocol not supported";
  case WSAESOCKTNOSUPPORT: return "Socket type not supported";
  case WSAEOPNOTSUPP: return "Operation not supported on socket";
  case WSAEPFNOSUPPORT: return "Protocol family not supported";
  case WSAEAFNOSUPPORT: return "Address family not supported by protocol family";
  case WSAEADDRINUSE: return "Address already in use";
  case WSAEADDRNOTAVAIL: return "Can't assign requested address";
  case WSAENETDOWN: return "Network is down";
  case WSAENETUNREACH: return "Network in unreachable";
  case WSAENETRESET: return "Network dropped connection on reset";
  case WSAECONNABORTED: return "Software caused connection abort";
  case WSAECONNRESET: return "Connection reset by peer";
  case WSAENOBUFS: return "No buffer space available";
  case WSAEISCONN: return "Socket is already connected";
  case WSAENOTCONN: return "Socket is not connected";
  case WSAESHUTDOWN: return "Can't send after socket shutdown";
  case WSAETOOMANYREFS: return "Too many references can't splice";
  case WSAETIMEDOUT: return "Connection timed out";
  case WSAECONNREFUSED: return "Connection refused";
  case WSAELOOP: return "Too many levels of symbolic links";
  case WSAENAMETOOLONG: return "File name too long";
  case WSAEHOSTDOWN: return "Host is down";
  case WSAEHOSTUNREACH: return "No route to host";
  case WSAENOTEMPTY: return "Directory not empty";
  case WSAEPROCLIM: return "Too many processes";
  case WSAEUSERS: return "Too many users";
  case WSAEDQUOT: return "Disk quota exceeded";
  case WSAESTALE: return "Stale NFS file handle";
  case WSAEREMOTE: return "Too many levels of remote in path";
  case WSASYSNOTREADY: return "Network subsystem is unusable";
  case WSAVERNOTSUPPORTED: return "WinSock DLL cannot support this application";
  case WSANOTINITIALISED: return "WinSock not initialized";
  case WSAEDISCON: return "Disconnect";
  case WSAHOST_NOT_FOUND: return "Host not found";
  case WSATRY_AGAIN: return "Nonauthoritative host not found";
  case WSANO_RECOVERY: return "Nonrecoverable error";
  case WSANO_DATA: return "Valid name, no data record of requested type";
  default:
                snprintf(buf, sizeof(buf), "Unknown windows system error %d", err);
                return buf;
 }
#endif

        switch (err) {
          case EINVAL:
                return "Invalid argument";
          case EACCES:
                return "Permission denied";
          case ESRCH:
                return "No such process";
          case EPERM:
                return "Not owner";
          case ENOENT:
                 return "No such file or directory";
          case ENOTDIR:
                return "Not a directory";
          case ENOEXEC:
                return "Exec format error";
          case ENOMEM:
                return "Not enough memory";
          case E2BIG:
                return "Argument list too long";
          default:
                snprintf(buf, sizeof(buf), "Unknown system error %d", err);
                return buf;
        }
# endif /* HAVE_SYS_ERRLIST */
}
#endif /* !HAVE_STRERROR */


#if !defined(HAVE_STRSTR)
char *strstr(const char *s, const char *p)
{
        int len;

        if (s && p)
                for (len = strlen(p); *s; s++)
                        if (*s == *p && strncmp(s, p, len) == 0)
                                return (char *) s;

        return 0;
}
#endif /* HAVE_STRSTR */

