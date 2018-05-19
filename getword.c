#include "getword.h"

int getword(char *w) {
    int counter = 0;
    int backslashcounter = 0;
    int i = 0;
    int iochar; 

    for(;;) {
    	iochar = getchar();
        //Checking if there is a maximum amount of characters.
        if (counter == STORAGE-1) {
            w[counter] = '\0';
            ungetc(iochar, stdin);
            return counter;
        }
        //Checking if there is a backslash. 
        if (iochar == '\\') {
            backslashcounter = 1;
            iochar = getchar();
        }

        if (iochar != ' ') {
            //If any of these characters are returned without a backslash, then return -1, otherwise treat as a regular character
            if (iochar == '<' || iochar == '>' || iochar == '|' || iochar == '&' || iochar == '#') {
               //If there is a backslash before any of these characters, then return the metacharacter as a regular character. 
                if (backslashcounter == 1) {
                    w[counter++] = iochar;
                    backslashcounter = 0;

                   //Code check for conditions specified for the "#" symbol
                } else if (iochar == '#') {
                    if (counter == 0) {
                        w[counter++] = iochar;
                        w[counter] = '\0';
                        return -1;
                    } else {
                        w[counter++] = iochar;
                    }
                    //Code check for conditions specified for the '|' symbol

                } else if (iochar == '|') {
                    if (counter == 0) {
                        w[counter++] = iochar;
                    } else {
                        ungetc(iochar, stdin);
                        return counter;
                    }
                        iochar = getchar();
                    //Code check for conditions if the pipeand symbol is shown '|&'
                    if (iochar == '&') {
                        w[counter++] = iochar;
                        w[counter] = '\0';
                        return -2;
                    } else {
                        ungetc(iochar,stdin);
                        w[counter] = '\0';
                        return -1;
                    }
                    //Metacharacters are all returned -1 if not a backslash is there before
                } else if (counter != 0) {
                    ungetc(iochar, stdin);
                    w[counter] = '\0';
                    return counter;
                } else {
                    w[counter++] = iochar;
                    w[counter] = '\0';
                    return -1;
                }

             //Code condition for handling a newline. If there is no other chars left then return -10 for newline. 

            } else if (iochar == '\n') {
                if (counter == 0) {
                    w[counter] = '\0';
                    return -10;
                }
                //If there is a newline then if there is some more chars left, then do ungetc and put new line back on stream. 
                ungetc(iochar, stdin);
                w[counter] = '\0';
                return counter;

                //If there is an EOF, then return the counter if there are more chars. 
            } else if (iochar == EOF) {
                if (counter == 0) {
                    w[counter] = '\0';
                    return 0;
                }
                ungetc(iochar, stdin);
                w[counter] = '\0';
                return counter;

            } else {
                w[counter++] = iochar;
            }
            //Code conditions for spaces in between words. 

        }
        if (iochar == ' ') {
            if (backslashcounter == 0 && counter != 0) {
                w[counter++] = '\0';
                return counter;

            }
            if(backslashcounter == 1)
                w[counter++] = iochar;
                backslashcounter = 0;
        }
    }
}
