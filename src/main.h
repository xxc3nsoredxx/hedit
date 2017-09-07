#ifndef MAIN_H_20170907_162420
#define MAIN_H_20170907_162420

/* Character codes */
#define ESC 0x1B
#define UP 0x103
#define DOWN 0x102
#define LEFT 0x104
#define RIGHT 0x105

/* Helper macros */
#define BYTES_PER_LINE 8
#define MAKE_ASCII(X) (((X) < 10) ? ((X) + 0x30) : ((X) - 9 + 0x40))
#define MAKE_HEX(X) (((X) >= 0x30 && (X) <= 0x39)                      \
                     ? ((X) - 0x30)                                    \
                     : ((X) - 0x40 + 9))
#define MAKE_PRINT(X) (((X) < 0x20 || (X) > 0x7E) ? '.' : (X))
#define MIN(X,Y) (((X) < (Y)) ? (X) : (Y))
#define MAX(X,Y) (((X) > (Y)) ? (X) : (Y))
#define UCASE(X) (((X) > 0x60 && (X) < 0x67) ? ((X) - 0x20) : (X))

/* A type to help create ncurses windows */
typedef struct window {
    int width;
    int height;
    int y;
    int x;
    /* Border format:
     *     Vert, Horiz, TL, TR, BL, BR
     */
    chtype border [6];
} window_t;

/* The functions used */
int start ();
void write_file ();
void dump_file ();
WINDOW* create_win (window_t*);
void delete_win (WINDOW*);
int main (int, char**);

#endif
