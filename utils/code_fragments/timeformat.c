
const char *time_format_1(time_t dt)
{
  register struct tm *delta;
  static char buf[64];
  if (dt < 0)
    dt = 0;

  delta = gmtime(&dt);
  if (delta->tm_yday > 0) {
    sprintf(buf, "%dd %02d:%02d",
	    delta->tm_yday, delta->tm_hour, delta->tm_min);
  } else {
    sprintf(buf, "%02d:%02d",
	    delta->tm_hour, delta->tm_min);
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
    sprintf(buf, "%dd", delta->tm_yday);
  } else if (delta->tm_hour > 0) {
    sprintf(buf, "%dh", delta->tm_hour);
  } else if (delta->tm_min > 0) {
    sprintf(buf, "%dm", delta->tm_min);
  } else {
    sprintf(buf, "%ds", delta->tm_sec);
  }
  return buf;
}
