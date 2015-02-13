#include <assert.h>
#include <stdio.h>

#include "define.h"
#include "util.h"

char ascii2byte(char *ascii) {
        char ret = 0x00;
        ret |= ascii2nibble(ascii[0]);
        ret <<= 4;
        ret |= ascii2nibble(ascii[1]);

        return ret;
}

char ascii2nibble(char ascii) {
        // printf ("%c\n", ascii);
        if ('0' <= ascii && ascii <= '9') {
                return ascii - '0';
        } else if ('A' <= ascii && ascii <= 'F') {
                return ascii - 'A' + 0xa;
        } else if ('a' <= ascii && ascii <= 'f') {
                return ascii - 'a' + 0xa;
        }

        // shouldn't get here
        // assert(0);
        return 0x00;
}

char nibble2ascii(char nibble) {
        assert ((nibble & 0x0f) == nibble);
        if (0x00 <= nibble && nibble <= 0x09) {
                return nibble + '0';
        } else if (0x0a <= nibble && nibble <= 0x0f) {
                return (nibble - 0x0a) + 'a';
        }

        // shouldn't get here
        // assert(0);
        return 0x00;
}

void byte2ascii(char byte, char *ascii) {
        ascii[0] = nibble2ascii((byte & 0xf0) >> 4);
        ascii[1] = nibble2ascii(byte & 0x0f);
}

void word2ascii(word_T word, char *ascii) {
        // little endian --> last byte goes in first byte
        byte2ascii(word & 0xff, ascii);
        byte2ascii((word & 0xff00) >> 8, ascii + 2);
}
