#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>

const char *time_format_1(time_t dt);
const char *time_format_2(time_t dt);
char *itoa(int num);

int main(int argc, char **argv) {
time_t me;

printf("Time 1: %s\n",time_format_1((time_t)atol(argv[1])));
printf("Time 2: %s\n",time_format_2((time_t)atol(argv[1])));

}

const char *time_format_1(time_t dt)
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
sprintf(buf, "%7s%02ds","", delta->tm_sec);

}

  return buf;
}

const char *time_format_2(time_t dt)
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

char *itoa(int num) {
static char buf[10];

sprintf(buf,"%d",num);
return buf;
}
