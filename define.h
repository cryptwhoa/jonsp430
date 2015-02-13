#ifndef JONSP430_DEFINE_H_
#define JONSP430_DEFINE_H_

#include <stdint.h>

#define BLACK(s) "\e[0;30m"s"\e[0m"
#define RED(s) "\e[0;31m"s"\e[0m"
#define GREEN(s) "\e[0;32m"s"\e[0m"
#define YELLOW(s) "\e[0;33m"s"\e[0m"
#define BLUE(s) "\e[0;34m"s"\e[0m"
#define PINK(s) "\e[0;35m"s"\e[0m"
#define CYAN(s) "\e[0;36m"s"\e[0m"
#define WHITE(s) "\e[0;37m"s"\e[0m"

#define BOLD_GREY(s) "\e[1;30m"s"\e[0m"
#define BOLD_RED(s) "\e[1;31m"s"\e[0m"
#define BOLD_GREEN(s) "\e[1;32m"s"\e[0m"
#define BOLD_YELLOW(s) "\e[1;33m"s"\e[0m"
#define BOLD_BLUE(s) "\e[1;34m"s"\e[0m"
#define BOLD_PINK(s) "\e[1;35m"s"\e[0m"
#define BOLD_CYAN(s) "\e[1;36m"s"\e[0m"
#define BOLD_WHITE(s) "\e[1;37m"s"\e[0m"

#define UNDERLINE_GREY(s) "\e[4;30m"s"\e[0m"
#define UNDERLINE_RED(s) "\e[4;31m"s"\e[0m"
#define UNDERLINE_GREEN(s) "\e[4;32m"s"\e[0m"
#define UNDERLINE_YELLOW(s) "\e[4;33m"s"\e[0m"
#define UNDERLINE_BLUE(s) "\e[4;34m"s"\e[0m"
#define UNDERLINE_PINK(s) "\e[4;35m"s"\e[0m"
#define UNDERLINE_CYAN(s) "\e[4;36m"s"\e[0m"
#define UNDERLINE_WHITE(s) "\e[4;37m"s"\e[0m"

#define BLOCK_GREY(s) "\e[7;30m"s"\e[0m"
#define BLOCK_RED(s) "\e[7;31m"s"\e[0m"
#define BLOCK_GREEN(s) "\e[7;32m"s"\e[0m"
#define BLOCK_YELLOW(s) "\e[7;33m"s"\e[0m"
#define BLOCK_BLUE(s) "\e[7;34m"s"\e[0m"
#define BLOCK_PINK(s) "\e[7;35m"s"\e[0m"
#define BLOCK_CYAN(s) "\e[7;36m"s"\e[0m"
#define BLOCK_WHITE(s) "\e[7;37m"s"\e[0m"

#define FROM_CONSOLE(s) BOLD_RED(s)
#define FROM_ERROR(s) BOLD_CYAN(s)
#define TO_CONSOLE(s) BOLD_GREEN(s)

#define MAX_FILENAME_LENGTH 50

enum result { RESULT_SUCCESS, RESULT_FAILURE };

typedef uint16_t word_T;

extern const word_T FLAG_N;
extern const word_T FLAG_Z;
extern const word_T FLAG_C;
extern const word_T FLAG_V;
extern const word_T FLAG_CPUOFF;

struct debug_state;
struct state;
struct instruction;

#endif
