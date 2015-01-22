#include "../../hdrfiles/config.h"
#include "../../hdrfiles/includes.h"
#include "../../hdrfiles/osdefs.h"
/*
#include "../../hdrfiles/authuser.h"
#include "../../hdrfiles/text.h"
*/
#define _DEFINING_CONSTANTS
#include "../../hdrfiles/constants.h"
#include "../../hdrfiles/protos.h"
#include "../../hdrfiles/resolver_clipon.h"

#define OPENBSD_SYS
#undef HAVE_STRFTIME

#if !defined(HAVE_STRFTIME)
char clocker_daylist[7][4] = {"Sun","Mon","Tue","Wed","Thu","Fri","Sat"};
char clocker_monthlist[12][4] = {"Jan","Feb","Mar","Apr","May","Jun","Jul","Aug","Sep","Oct","Nov","Dec"};
#endif

#if defined(OPENBSD_SYS)
char *ctime(const time_t *mytm);
#endif

int main(int argc, char **argv) {
time_t newtm;

time(&newtm);
printf("Time: %ld %s",(unsigned long)newtm,ctime(&newtm));

return 1;
}

#if defined(OPENBSD_SYS)
char *ctime(const time_t *mytm)
{
struct tm *clocker;
static char buf1[32];

clocker=localtime(mytm);
#if defined(HAVE_STRFTIME)
printf("strftime\n");
strftime(buf1,sizeof(buf1),"%a %b %d %H:%M:%S %Y\n",clocker);
#else
printf("snprintf\n");
snprintf(buf1,sizeof(buf1),"%s %s %d %.2d:%.2d:%.2d %d\n",
clocker_daylist[clocker->tm_wday],clocker_monthlist[clocker->tm_mon],
clocker->tm_mday,clocker->tm_hour,clocker->tm_min,clocker->tm_sec,
1900+clocker->tm_year);
#endif

return buf1;
}
#endif

