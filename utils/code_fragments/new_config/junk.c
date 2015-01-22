#include <stdio.h>
#include <string.h>

void remove_first(char *inpstr);

int main(void) {
char line[80];
char line2[2];
char tempvalue[80];
int ret=0;

strcpy(line,"this is a test");
line2[0]=0;

while (strlen(line)) {
sscanf(line,"%s",tempvalue);
remove_first(line);
if (strlen(line2)) strncat(line2," ",sizeof(line2)-strlen(line2));
strncat(line2,tempvalue,sizeof(line2)-strlen(line2));
if (strlen(line2)>=sizeof(line2)) break;
}
printf("%s\n",line2);

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


