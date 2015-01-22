#if defined(HAVE_CONFIG_H)
#include "../hdrfiles/config.h"
#endif

#include "../hdrfiles/includes.h"

/*
#include <stdio.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <errno.h>
#if defined(SOL_SYS) || defined(LINUX_SYS)
#include <string.h>
#else
#include <strings.h>
#endif
*/

#include "../hdrfiles/osdefs.h"
#include "../hdrfiles/constants.h"
#include "../hdrfiles/protos.h"

extern int down_time;
extern int num_of_users;
extern int atmos_on;
extern int signl;
extern int treboot;
extern char mess[ARR_SIZE+25];



void init_signals()
{

/* Damn zombie kids..get rid of them when they cry for help */
(void)setsignal(SIGCHLD, zombie_killer); /* does win32 like us? */
              
/*------------------------------------------------*/
/* the following signal ignores may be desireable */
/* to comment out for debugging                   */
/*------------------------------------------------*/
           
(void)setsignal(SIGTERM,handle_sig);
(void)setsignal(SIGUSR1,handle_sig); /* does win32 like us? */
(void)setsignal(SIGUSR2,handle_sig); /* does win32 like us? */
(void)setsignal(SIGSEGV,handle_sig);

(void)setsignal(SIGILL,handle_sig);
(void)setsignal(SIGINT,(handler_t)SIG_IGN);
(void)setsignal(SIGABRT,(handler_t)SIG_IGN);
(void)setsignal(SIGFPE,(handler_t)SIG_IGN);

#if !defined(WINDOWS)
  /* we don't care about SIGPIPE, we notice it in select() and write() */
  (void)setsignal(SIGPIPE, (handler_t)SIG_IGN);
  (void)setsignal(SIGBUS,handle_sig);
#if !defined(__CYGWIN32__)
  (void)setsignal(SIGIOT,(handler_t)SIG_IGN);
#endif
  (void)setsignal(SIGTSTP,(handler_t)SIG_IGN);
  (void)setsignal(SIGCONT,(handler_t)SIG_IGN);
  (void)setsignal(SIGHUP,(handler_t)SIG_IGN);
  (void)setsignal(SIGQUIT,handle_sig);
#if !defined(__CYGWIN32__)
  (void)setsignal(SIGURG,(handler_t)SIG_IGN);  
#endif
  (void)setsignal(SIGTTIN,(handler_t)SIG_IGN);
  (void)setsignal(SIGTTOU,(handler_t)SIG_IGN);
#if !defined(LINUX_SYS)
(void)setsignal(SIGEMT,(handler_t)SIG_IGN); /* does win32 like us? */
#endif
#endif

}

/*----------------------------------------------------------------------*/
/* Function to clean up the zombied child process from a fork           */
/*                                                                      */
/* Zombie'd child processes occur when the the child finishes before    */
/* the parent process. The kernel still keeps some of the information   */
/* about the child in case the parent might need it. To be able to get  */
/* this info, the parent calls waitpid(). When this happens, the kernel */
/* can discard the information. Zombies dont take up any resources,     */
/* other than a process table entry. If the parent terminates without   */
/* calling waitpid(), the child is adopted by init, which handles the   */
/* work necessary to cleanup after the child.                           */
/*----------------------------------------------------------------------*/
RETSIGTYPE zombie_killer(int sig)
{
int status, child_val, i=0;
pid_t child_pid=-1;
pid_t get_pid;

#if defined(ZOMBIE_DEBUG)
 write_log(DEBUGLOG,YESTIME,"ZOMBIE: Calling zombie killer..\n");
#endif

/* (void *) casts to avoid warnings on systems that mis-declare */
/* the argument type. */
while ( (get_pid = waitpid(
        -1,             /* Wait for any child */
        (void *) &status,
        WNOHANG         /* Don't block waiting */
       )) > 0) {
           if (get_pid <= 0) break;
           else child_pid = get_pid;
          }

/* No child to clean up, or error */
if (child_pid <= 0) {
        if (child_pid == -1) {
#if defined(ZOMBIE_DEBUG)
        write_log(DEBUGLOG,YESTIME,"ZOMBIE: waitpid() error! %s\n",get_error());
#endif   
        }
        (void)setsignal(SIGCHLD, zombie_killer);
        return;
        }
else {
#if defined(ZOMBIE_DEBUG)
        write_log(DEBUGLOG,YESTIME,"ZOMBIE: waitpid() returned pid %u\n",(unsigned int)child_pid);
#endif
        }
        
        
/* Reset handler                    */
/* Doing this before the waitpid()  */
/* can lead to an infinite loop     */
(void)setsignal(SIGCHLD, zombie_killer);

/* Negative child_val indicates some error    */
/* Zero child_val indicates no data available */
         
    /*  
     * We now have the info in 'status' and can manipulate it using
     * the macros in wait.h.
     */
    if (WIFEXITED(status))                /* did child exit normally? */
    {
        child_val = WEXITSTATUS(status); /* get child's exit status */
#if defined(ZOMBIE_DEBUG)
        write_log(DEBUGLOG,YESTIME,"ZOMBIE: Child exited normally with status %d\n", child_val);
#endif

     
        /* Find the user who had this child and let them do remote whos again */
        for (i=0;i<MAX_USERS;++i) {
                if (ustr[i].rwho==child_pid) {
                ustr[i].rwho=1;
                break;   
                } /* end of if */
        } /* end of for */
    }

}       

/*** switching function ***/
RETSIGTYPE sigcall(int sig)
{
              /*-------------------------*/
              /* process timed events    */
              /*-------------------------*/

              /*--------------------------------------*/
              /* check for out of date board messages */
              /*--------------------------------------*/
              check_mess(0);

		if (down_time > 0)
		{
			check_shut();
		}

		if (num_of_users)
		{
			check_idle();
			check_flood_limits();
		}

		if (LOGIN_LIMITING)
		{
			check_connlist_entries(-2);
		}

		check_misc_connects();

		check_smtp();

		if (atmos_on)
		{
			atmospherics();
		}

                check_total_users(0);
                 
#if !defined(WINDOWS)
reset_alarm();   
#endif
              
signl = 1;
}

/*** reset alarm - first called from add_user ***/
void reset_alarm()
{
(void)setsignal(SIGALRM, sigcall);
alarm( MAX_ATIME );
}

/**** START OF SIGNAL FUNCTIONS ****/
RETSIGTYPE handle_sig(int sig)
{
                 
switch(sig) {
        case SIGTERM:
                shutdown_error(log_error(10));
		break;
        case SIGSEGV:
                if (REBOOT_A_CRASH==1)
                 treboot=1;
                shutdown_error(log_error(11));
		break;
        case SIGBUS: 
                if (REBOOT_A_CRASH==1)
                 treboot=1;
                shutdown_error(log_error(12));
		break;
        case SIGUSR1:
		/* command line shutdown */
                shutdown_error(log_error(13));
		break;
        case SIGUSR2:
		/* re-init config file */
                read_init_data(1);
		messcount();
		sprintf(mess,"%s Config file reinit done from command line",STAFF_PREFIX);
		writeall_str(mess, WIZ_ONLY, -1, 0, -1, BOLD, WIZT, 0);
		break;
        case SIGQUIT:
		/* soft-reboot */
		/* this only really works ONCE inside of a hard startup, we dont know why, */
		/* so it's not really a documented feature */
		treboot=1;
                shutdown_error(log_error(13));
		break;
        case SIGILL:
                if (REBOOT_A_CRASH==1)
                 treboot=1;
                shutdown_error(log_error(15));
		break;
  }
		/* Reset handler                    */
		(void)setsignal(sig, handle_sig);
}


/*
 * An os independent signal() with BSD semantics, e.g. the signal
 * catcher is restored following service of the signal.
 *
 * When sigset() is available, signal() has SYSV semantics and sigset()
 * has BSD semantics and call interface. Unfortunately, Linux does not
 * have sigset() so we use the more complicated sigaction() interface
 * there.
 *
 * Did I mention that signals suck?
 */
/* (*setsignal (int sig, RETSIGTYPE (*func)(int)))(int) */
handler_t
setsignal(int sig, handler_t handler)
{
#if defined(HAVE_SIGACTION)
        struct sigaction old, new;
#elif defined(HAVE_SIGMASK)
	struct sigvec nsv,osv;
#endif
	handler_t oldh;

#if defined(HAVE_SIGACTION)
        memset(&new, 0, sizeof(new));
        new.sa_handler = handler;
#if defined(SA_RESTART)
        new.sa_flags |= SA_RESTART;
#endif
        if (sigaction(sig, &new, &old) < 0)
                return ((handler_t)SIG_ERR);
        oldh = old.sa_handler;

#elif defined(HAVE_SIGMASK)
        memset(&nsv, 0, sizeof(nsv));
    nsv.sv_handler = handler;
    nsv.sv_mask = 0;                    /* punt */
    nsv.sv_flags = SV_INTERRUPT;        /* punt */
    sigvec(sig, &nsv, &osv);
    oldh = osv.sv_handler;
#elif defined(HAVE_SIGSET)
        oldh = (handler_t)sigset(sig, handler);
#else
        oldh = signal(sig, handler);
#endif
	return oldh;
}

