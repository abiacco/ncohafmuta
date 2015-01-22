/* filecopy.c
  $Id: filecopy.c,v 1.1 2000/03/02 11:46:36 gwiley Exp $
  Glen Wiley, <gwiley@ieee.org>

  this function will reliably copy a file, detecting
  whether source/destination are the same file
  returns 0 on success
*/

#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>

/* size of the buffer used for copying the file */
static const int COPYBUFFSZ = 4096;

/*-------------------------------------- filecopy
 copy file to a separate file
 return 0 on success, errno on failure
*/
int
filecopy(const char *fn_src, const char *fn_dst)
{
        int   bytesr;
        int   retval  = 0;
        int   done    = 0;
        FILE  *fh_src = NULL;
        FILE  *fh_dst = NULL;
        char  *buff   = NULL;
        char  *p      = NULL;
        struct stat stat_s;
        struct stat stat_d;

        buff = (char *) malloc(COPYBUFFSZ);
        if(buff == NULL)
                return errno;

        if(stat(fn_src, &stat_s) != 0)
                retval = errno;

        // if we can stat the dest file then make sure it is not the same as src 
        if(stat(fn_dst, &stat_d) == 0)
        {
                if(stat_s.st_dev == stat_d.st_dev && stat_s.st_ino == stat_d.st_ino)
                        retval = EEXIST;
        }

        if(!retval)
        {
                fh_src = fopen(fn_src, "r");
                if(fh_src == NULL)
                        retval = errno;
        }

        if(!retval)
        {
                fh_dst = fopen(fn_dst, "w+");
                if(fh_dst == NULL)
                        retval = errno;
                else
                {
                        while(!retval)
                        {
                                bytesr = fread(buff, 1, COPYBUFFSZ, fh_src);
                                if(bytesr < 1)
                                {
                                        if(feof(fh_src))
                                                break;
                                        else
                                                retval = ferror(fh_src);
                                }
                                else
                                {
                                        if(fwrite(buff, 1, bytesr, fh_dst) != bytesr)
                                        {
                                                retval = ferror(fh_dst);
                                                break;
                                        }
                                }
                        }
                } // if fh_dst

                if(fh_dst)
                        fclose(fh_dst);

        } // if !retval

        if(fh_src)
                fclose(fh_src);

        if(buff)
                free(buff);

        return retval;
} // filecopy

/* filecopy */
