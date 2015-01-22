#include <unistd.h>
#include <signal.h>
#include <sysexits.h>

#ifdef  SIGTSTP
#include <sys/file.h>
#include <sys/ioctl.h>
#endif
 
/* normally included from paths.h */

#ifndef _PATH_TTY
#define _PATH_TTY "/dev/tty"
#endif
 
void    pdetach(void)
{
        int pid;
        int fd;

        /* if we are started from init, no need to become daemon */
 
        if(getppid() == 1)
                return;
 
#ifdef  SIGTTOU
        signal(SIGTTOU, SIG_IGN);
#endif
 
#ifdef  SIGTTIN
        signal(SIGTTIN, SIG_IGN);
#endif                                                                          

#ifdef  SIGTSTP
        signal(SIGTSTP, SIG_IGN);
#endif
 
        if((pid = fork()) < 0)
                exit(EX_OSERR);
        else if(pid > 0)
                exit(0);
 
 
#if     defined(SIGTSTP) && defined(TIOCNOTTY)
        if(setpgid(0, getpid()) == -1)
                exit(EX_OSERR);

        if((fd = open(_PATH_TTY, O_RDWR)) >= 0)
        {
                ioctl(fd, TIOCNOTTY, NULL);
                close(fd);
        }
#else
        if(setpgrp() == -1)
                exit(EX_OSERR);
 
        signal(SIGHUP, SIG_IGN);
 
        if((pid = fork()) < 0)
                exit(OS_ERR);
        else if(pid > 0)
                exit(0);
 
#endif
}

