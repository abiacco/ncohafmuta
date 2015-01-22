#include <stdio.h>
#include <stdlib.h>

#define MAX_TELLEDUSERS	3
char array[MAX_TELLEDUSERS][80];

int main()
{
int point=0,count=0,i=0,lastspace=0,lastcomma=0,gotchar=0;
int point2=0;
char inpstr[500];
char buf[500];

for (i=0;i<10;++i) array[i][0]=0;

i=0;
strcpy(inpstr,"user,user2");
printf("Gonna parse: \"%s\"\n",inpstr);

for (i=0;i<strlen(inpstr);++i) {
	if (inpstr[i]==' ') {
		if (lastspace && !gotchar) { point++; point2++; continue; }
		if (!gotchar) { point++; point2++; }
		lastspace=1;
		continue;
	  } /* end of if space */
	else if (inpstr[i]==',') {
		if (!gotchar) {
			lastcomma=1;
			continue;
			point++;
		}
		else { 
		if (count <= MAX_TELLEDUSERS-1) {
		midcpy(inpstr,array[count],point,point2-1);
		count++;
		}
		point=i+1;
		point2=point;
		gotchar=0;
		lastcomma=1;
		continue;
		}

	} /* end of if comma */
	if ((inpstr[i-1]==' ') && (gotchar)) {
		if (count <= MAX_TELLEDUSERS-1) {
		midcpy(inpstr,array[count],point,point2-1);
		count++;
		}
		break;
	}
	gotchar=1;
	lastcomma=0;
	lastspace=0;
	point2++;
} /* end of for */
midcpy(inpstr,buf,i,500);
i=0;

printf("Done parsing\n");
printf("Array0: \"%s\"\n",array[0]);
printf("Array1: \"%s\"\n",array[1]);
printf("Array2: \"%s\"\n",array[2]);
printf("Array3: \"%s\"\n",array[3]);
printf("Array4: \"%s\"\n",array[4]);
printf("Array5: \"%s\"\n",array[5]);
printf("Mess  : \"%s\"\n",buf);
printf("Count : %d\n",count);

/* check inpstr */

for (i=0;i<count;++i) {

} /* end of for */

} /* end */

midcpy(strf,strt,fr,to)
char *strf,*strt;
int fr,to;
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


