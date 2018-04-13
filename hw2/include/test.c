#include "utf.h"
#include "wrappers.h"

void main(){
    char *a[] = {"hello", "world", NULL};

    char *s =  join_string_array(2, a);

    free(s);
}
