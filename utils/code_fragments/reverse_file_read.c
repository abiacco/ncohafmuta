#include <stdio.h>
#include <stdlib.h>

/*

        This program is written to reverse the lines in a unix file.

        Written by James M. Rogers

        04 May 1999

*/

struct element{
    struct element *next;
    int value;
}element_size;

struct element *initialize () {
    struct element *add;
    if(add=(struct element *)malloc(sizeof(element_size))){
        add->next = (struct element *)NULL;
        return add;
    } else {
        return (struct element *)NULL;
    }
}

struct element *push (struct element *top, int value) {

    struct element *add;

    if (add=(struct element *)malloc(sizeof(element_size))){

        add->next=top;
        add->value=value;
        top=add;
        return top;
    } else {
        printf("Failed to push a value onto the stack, I am quiting.\n");
        exit (1);
    }
}

struct element *pop (struct element *top, int *value) {

    struct element *remove;

    fflush(stdout);

    *value=top->value;
    remove=top;
    top=top->next;
    free(remove);
    
    return top;
}

main(int argc, char *argv[]){

    struct element *stack;
    long            position;
    int             value,
                    x;
    FILE           *stream;            /* need a stream pointer of type FILE */ 

    if ( !(stack = initialize())){
        printf("Failed to initialize the stack, I am quiting.\n");
        exit (1);
    }

    /* ensure that the filename in argv[1] exists and is a file */

    /* open the file designated on the command line */
    if (!(stream = fopen(argv[1], "r"))){
        printf("Failed to open the designated file, I am quiting.\n");
        exit (1);
    } 

    /* read backwards thru the file a character at a time til reaching BOF */
    if(fseek(stream, 0, SEEK_END)) {
        printf("Failed to fseek to the end of the file.\n");
        exit (1);
    }

    if((position = ftell(stream))==EOF) {
        printf("Failed to fteek the file position.\n");
        exit (1);
    }
    position--;

    for ( ; position > -1 ; position--) {

        /* seek to the proper position and read in a character */
        if(fseek(stream, position, SEEK_SET)) {
            printf("Failed to fseek to the current file position.\n");
            exit (1);
        }

        if((value=fgetc(stream))==EOF) {
            printf("Failed to read a character from the current file position.\n");
            exit (1);
        }

        /* if the character is a newline then print the accumulated stack*/
        if(value=='\n'){
            while (stack = pop(stack, &x)){
                printf("%c", x);

            }
            stack = push(stack, '\n');
        }

        /* push the character onto the stack */
        stack = push(stack, value);

    }

    /* print the first line to finish up */
        while (stack = pop(stack, &x)){
            printf("%c", x);
        }

}

