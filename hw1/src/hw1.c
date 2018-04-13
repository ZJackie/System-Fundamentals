#include "hw1.h"
#include "stdlib.h"

#ifdef _STRING_H
#error "Do not #include <string.h>. You will get a ZERO."
#endif

#ifdef _STRINGS_H
#error "Do not #include <strings.h>. You will get a ZERO."
#endif

#ifdef _CTYPE_H
#error "Do not #include <ctype.h>. You will get a ZERO."
#endif

/**
 * @brief Validates command line arguments passed to the program.
 * @details This function will validate all the arguments passed to the program
 * and will return a unsigned short (2 bytes) that will contain the
 * information necessary for the proper execution of the program.
 *
 * IF -p is given but no (-r) ROWS or (-c) COLUMNS are specified this function
 * MUST set the lower bits to the default value of 10. If one or the other
 * (rows/columns) is specified then you MUST keep that value rather than assigning the default.
 *
 * @param argc The number of arguments passed to the program from the CLI.
 * @param argv The argument strings passed to the program from the CLI.
 * @return Refer to homework document for the return value of this function.
 */
int column = 10;

int compareString(char *x, char *y) {
int size = 1;
while(*(x + size) != '\0' || *(y + size) != '\0'){
    if((*(x + size) != *(y + size))){
        return 0;
    }
    size++;
}
return 1;
}

int compareMorse(const char *x, const char *y) {
int size = 0;
while(*(x + size) != '\0' || *(y + size) != '\0'){
    if((*(x + size) != *(y + size))){
        return 0;
    }
    size++;
}
return 1;
}

int repetitioncheck(char *x){
int size1 = 0;
int size2 = size1 + 1;
while(*(x + size1) != '\0'){
    while(*(x + size2) != '\0'){
        if(*(x + size1) == *(x + size2)){
        return 1;
        }
        size2++;
    }
    size1++;
    size2 = size1 + 1;
}
return 0;
}

int keycheck(char *x, const char *y){
if(repetitioncheck(x)){
    return 0;
}
else{
    int sizeofX = 0;
    int sizeofY = 0;
    int counter = 0;
    while(*(x + sizeofX)!= '\0'){
        while(*(y + sizeofY) != '\0'){
            if(*(x + sizeofX) == *(y + sizeofY)){
                counter++;
            }
        sizeofY++;
    }
        if(counter != 1){
            return 0;
        }
    sizeofX++;
    sizeofY = 0;
    counter = 0;
}
return 1;
}
}

int filltable(char *table, const char *alphabet, char *key){
int counter = 0;
for(int i = 0; i < 257; i++){
    if(*(key + i) != '\0'){
        *(table + i) = *(key + i);
        int size = 0;
        counter++;
    }
    else{
        break;
    }
}
int tablep = counter;
for(int i = 0; i < 257; i++){
    if(*(alphabet + i) != '\0'){
        for(int j = 0; j < counter; j++){
        if(*(alphabet + i) == *(table + j)){
            i++;
            j = 0;
        }
        }
        *(table + tablep) = *(alphabet + i);
        tablep++;
    }
    else{
        *(table + tablep) = '\0';
        tablep++;
    }
}
return 1;
}

int getposition(char *table, char target){
for(int i = 0; i < 257; i++){
    if(*(table + i) == target){
    return i;
    }
}
return 258;
}

int findMorse(const char *alphabet, char target){
int size = 0;
while(*(alphabet + size) != '\0'){
    if(*(alphabet + size) == target){
        return size;
    }
    size++;
    }
return -1;
}


int compare(const char *x, char *y){
for(int i = 0; i < 3; i++){
    if(*(x+i) != *(y+i)){
    return 0;
    }
}
return 1;
}

int clear(char *buffer){
for(int i = 0; *(buffer + i) != '\0'; i++){
*(buffer + i) = '\0';
}
return 1;
}


unsigned short validargs(int argc, char **argv) {
unsigned short mode = 0;
char *key;
key = "";
if(argc > 1){
    if(compareString(*(argv + 1),"-h")){
        mode = 1<<15;
    }
    else if(compareString(*(argv + 1),"-f")){
        mode = 1<<14;
        if(argc > 2){
            if(compareString(*(argv + 2),"-p")){
                return 0;
            }
            else if(compareString(*(argv + 2),"-d")){
                mode = mode | 1<<13;
            }
            else if(compareString(*(argv + 2),"-e")){
            }
            else{
                return 0;
            }
            for(int i = 3; i < argc; i++){
                if(compareString(*(argv + i),"-k")){
                    key = *(argv + i + 1);
                    if(keycheck(*(argv + i + 1), fm_alphabet) == 0){
                        return 0;
                    }
                    i++;
                }
                else{
                    return 0;
                }
            }
        }
        else{
            return 0;
        }
        filltable(fm_key, fm_alphabet, key);
    }
    else if(compareString(*(argv + 1),"-p")){
        mode = 10 | 10<<4;
        if(argc > 2){
            if(compareString(*(argv + 2),"-f")){
                return 0;
            }
            else if(compareString(*(argv + 2),"-d")){
                mode = mode | 1<<13;
            }
            else if(compareString(*(argv + 2),"-e")){
            }
            else{
                return 0;
            }
            for(int i = 3; i < argc; i++){
                if(compareString(*(argv + i),"-k")){
                    key = *(argv + i + 1);
                    if(keycheck(*(argv + i + 1), polybius_alphabet) == 0){
                        return 0;
                    }
                    i++;
                }
                else if(compareString(*(argv + i),"-r")){
                    int r = atoi(argv[i+1]);
                    //row  = r;
                    if(r > 8 && r < 16){
                        mode = mode | r<<4;
                    }
                    else{
                    return 0;
                    }
                    i++;
                }
                else if(compareString(*(argv + i),"-c")){
                    int c = atoi(argv[i+1]);
                    column = c;
                    if(c > 8 && c < 16){
                        mode = mode | c;
                    }
                    else{
                    return 0;
                    }
                    i++;
                }
                else{
                    return 0;
                }
            }
        }
        else{
            return 0;
        }
        filltable(polybius_table, polybius_alphabet, key);
    }
    else{
        return 0;
    }
}
return mode;
}


int poly_Encrypt(){
char input = getchar();
int index;
int xpos;
int ypos;
while(input != EOF){
    if(input == '\n'){
        printf("\n");
    }
    else if(input == ' '){
        printf(" ");
    }
    else if(input == '\t'){
        printf("\t");
    }
    else{
        if(getposition(polybius_table, input) == 258){
        return 0;
    }
    index = getposition(polybius_table, input);
    xpos =  index / column;
    ypos = index % column;
    printf("%X", xpos );
    printf("%X", ypos );
    }
    input = getchar();
    }
return 1;
}

int poly_Decrypt(){
char input = getchar();
char input2;
int location;
int xpos;
int ypos;
while(input != EOF){
    if(input == '\n'){
        printf("\n");
    }
    else if(input == ' '){
        printf(" ");
    }
    else if(input == '\t'){
        printf("\t");
    }
    else{
    input2 = getchar();
    location = (((input - '0') * column) + (input2 - '0'));
    printf("%c", *(polybius_table + location));
    }
input = getchar();
}
return 1;
}

int morse_Encrypt(){
int spaces = 0;
int tripletcounter = 0;
int morsesize = 0;
char input = getc(stdin);
const char* morsesingle;
while(input != EOF){
    morsesize = 0;
    if(input == ' ' || input == '\t' || input == '\n'){
        if(spaces != 1){
            spaces = 1;
            *(polybius_table + tripletcounter) = 'x';
            tripletcounter++;
            if(tripletcounter > 2){
                tripletcounter = 0;
                for(int i = 0; i< 26; i++){
                    if(compare(*(fractionated_table + i),polybius_table)){
                    printf("%c",*(fm_key+i));
        }
   }
}
}
    if(input == '\n'){
        tripletcounter = 0;
        clear(polybius_table);
        printf("\n");
    }
}
    else{
        spaces = 0;
    if(findMorse(polybius_alphabet,input) > -1){
        int index2 = findMorse(polybius_alphabet,input);
        morsesingle = *(morse_table + index2);
    if(*morsesingle == '\0'){
        return 0;
    }
    else{
        while(*(morsesingle + morsesize) != '\0'){
        *(polybius_table + tripletcounter) = *(morsesingle + morsesize);
        tripletcounter++;
    if(tripletcounter > 2){
        tripletcounter = 0;
        for(int i = 0; i< 26; i++){
            if(compare(*(fractionated_table + i), polybius_table)){
                printf("%c",*(fm_key+i));
    }
   };
}
morsesize++;
}

*(polybius_table + tripletcounter) = 'x';
tripletcounter++;
if(tripletcounter > 2){
   tripletcounter = 0;
   for(int i = 0; i< 26; i++){
    if(compare(*(fractionated_table + i),polybius_table)){
        printf("%c",*(fm_key+i));
    }
   }

}
}
}
else{
    return 0;
}
}
//printf("x");
input = getc(stdin);
}

*(polybius_table + tripletcounter) = 'x';
tripletcounter++;
if(tripletcounter > 2){
   tripletcounter = 0;
   for(int i = 0; i< 26; i++){
    if(compare(*(fractionated_table + i),polybius_table)){
        printf("%c",*(fm_key+i));
    }
   }
}
return 1;
}

int morse_Decrypt(){
int spaces = 0;
int tripletcounter = 0;
int morsesize = 0;
char input = getc(stdin);
const char* morsesingle;
int index3;
int index4;
int xcounter = 0;
while(input != EOF){
    morsesize = 0;
    if(input == '\n'){
        *(polybius_table + tripletcounter) = '\0';
        if(*polybius_table != '\0'){
            for(int i = 0; *(morse_table + i) != '\0'; i++){
                if(compareMorse(*(morse_table + i),polybius_table)){
                    printf("%c",*(polybius_alphabet + i));}
                    }
                    }
                    tripletcounter = 0;
                    printf("\n");
    }
    else{
    int index2 = findMorse(fm_key,input);
    morsesingle = *(fractionated_table + index2);
    while(*(morsesingle + morsesize) != '\0'){
    if(*(morsesingle + morsesize) == 'x'){
    if(xcounter == 1){
    xcounter = 0;
    *(polybius_table + tripletcounter) = '\0';
    if(*polybius_table != '\0'){
       // printf("%s\n",polybius_table);
    for(int i = 0; *(morse_table + i) != '\0'; i++){
    if(compareMorse(*(morse_table + i), polybius_table)){
    printf("%c",*(polybius_alphabet + i));
    }
    }
    }
    printf(" ");
    tripletcounter = 0;
    }
    else{
        xcounter++;
        }
        }
        else{
            if(xcounter == 1){
                *(polybius_table + tripletcounter) = '\0';
                if(*polybius_table != '\0'){
                    for(int i = 0; *(morse_table + i) != '\0'; i++){
                        if(compareMorse(*(morse_table + i),polybius_table)){
                            printf("%c",*(polybius_alphabet + i));
                            }
                    }
                }
                            tripletcounter = 0;
                            }
                            *(polybius_table + tripletcounter) = *(morsesingle + morsesize);
                            tripletcounter++;
                            xcounter = 0;
        }
        morsesize++;
        }
    }
    input = getc(stdin);
}

*(polybius_table + tripletcounter) = '\0';
if(*polybius_table != '\0'){
    for(int i = 0; *(morse_table + i) != '\0'; i++){
        if(compareMorse(*(morse_table + i),polybius_table)){
            printf("%c",*(polybius_alphabet + i));
    }
}
}
tripletcounter = 0;
return 1;
}