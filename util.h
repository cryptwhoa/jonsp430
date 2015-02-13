#ifndef JONSP430_UTIL_H_
#define JONSP430_UTIL_H_

#include <stdint.h>

#include "define.h"

// takes an ascii byte value
// and changes it to the actual hex value
// returns 0xff if a can't be represented as a hex value
char ascii2nibble(char a);
char ascii2byte(char *a);

char nibble2ascii(char b);
void byte2ascii(char b, char *a);
// void word2ascii(word_T w, char *a);


#endif
