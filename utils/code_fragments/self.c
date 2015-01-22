/*----------------------------------------------------
  

  self.c

  (P) 1996 Laurentiu Cristofor (laur72_98@yahoo.com)

  a sample program that modifies its code on disk; 
  can be used as a base for creating a program that
  keeps track of how many times it has been executed.

  The technique of reading the file from disk in two
  buffers is very nice and I have used it in several
  projects.

  I have developed this code starting from a public 
  domain source code. That source code had several 
  bugs that I have removed and I think that now the 
  portion that reads the file from disk is quite ok.

------------------------------------------------------*/

#include "stdio.h"
#include "stdlib.h"
#include "string.h"

#define FALSE 0
#define TRUE  1

struct  control 
{
  char code[8];
  unsigned long offset;
  unsigned long runtimes;
};

struct control  berserk = {"BERSERK\0", 0, 0};

int main (int argc, char *argv[])
{
  char   fname[80];
  char   buff[1024];           /* Data buffer */
  int    found;                /* True if control code found */
  size_t rsize;                /* Amount of data read into buffer */
  int    sector,offset;        /* Position where control code found */
  FILE   *fn;

  if (argc == 2 && !strcmp(argv[1],"s"))
    {
      printf("This program was run %d time(s)\n", berserk.runtimes);
      exit(0);
    }

  strcpy(fname, argv[0]);

  if ((fn = fopen(fname,"rb+")) == NULL)
    {
      puts ("Cannot open file!\n" );
      exit (1);
    }

  if (!berserk.offset)
    {
      sector = 0;
      found = FALSE;
      rsize = fread(buff,sizeof(char),512,fn);
      
      while (!feof(fn) && !found)
        {
          sector++;
          rsize = fread(&buff[513],sizeof(char),512,fn);
          for (offset=0; offset < 512; offset++)
            {
              if (strncmp(&buff[offset], berserk.code, 
                          strlen(berserk.code)) == 0)
                {
                  found = TRUE;
                  break;
                }
            }
          memcpy(buff,&buff[513],512);
        }

      if (!found)
        {
          sector++;
          for (offset=0; offset < rsize - strlen(berserk.code); offset++)
            {
              if (strncmp(&buff[offset], berserk.code, 
                          strlen(berserk.code)) == 0)
                {
                  found = TRUE;
                  break;
                }
            }
        }
      
      if (!found)
        {
          puts("Program code is corrupted!\n");
          exit(1);
        }
      
      berserk.offset = (sector-1) * 512L + offset;
    }
  
  berserk.runtimes++;
  fseek (fn, berserk.offset, SEEK_SET);
  fwrite(&berserk, sizeof(struct control), 1, fn);
  fclose(fn);
}

