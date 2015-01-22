#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <errno.h>
#include <malloc.h>

struct listelement {
    int id;
    char dataitem[30];
    struct listelement *prev,*next;
};
typedef struct listelement* ListPtr;
ListPtr firstpointer=NULL,lastpointer=NULL;

ListPtr InitListItem (ListPtr listpointer);
void PrintListItems (ListPtr listpointer);


int main(void) {
ListPtr mypointer;
char mystr[20];
int count=0;

mypointer=NULL;

while (count < 5) {
sprintf(mystr,"test%d",count);
printf("allocing\n");
mypointer = InitListItem (mypointer);
printf("alloced\n");
mypointer->id = count;
printf("ided\n");
/* mypointer->dataitem = mystr; */
mypointer->dataitem[0]='\0';
strncpy(mypointer->dataitem,mystr,sizeof(mypointer->dataitem));
printf("copied\n");
printf("DATA: %d\n",mypointer->id);
count++;
}

PrintListItems(mypointer);

}


ListPtr InitListItem (ListPtr listpointer) {
ListPtr newitem=NULL;

printf("init0a\n");

if ((newitem = (ListPtr) malloc (sizeof (ListPtr)))==NULL) {
printf("Failed! errno: %d\n",errno);
exit(0);
}

printf("init0b\n");
if (firstpointer==NULL) {
printf("init1a\n");
	firstpointer = newitem;
	newitem->prev = NULL;
printf("init2a\n");
}
else {
printf("init1b\n");
printf("init2b\n");
	lastpointer->next = newitem;
printf("init3b\n");
	newitem->prev = lastpointer;
/*	listpointer->next = newitem; */
}

newitem->next = NULL;
lastpointer = newitem;
return newitem;
}


void PrintListItems (ListPtr listpointer) {
ListPtr templist;

for (templist=firstpointer;templist!=NULL;templist=templist->next) {
 printf("%d\n",templist->id);
}


}
