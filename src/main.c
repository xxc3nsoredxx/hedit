#include <curses.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>

/* Character codes */
#define ESC 0x1B
#define UP 0x103
#define DOWN 0x102
#define LEFT 0x104
#define RIGHT 0x105

#define BYTES_PER_LINE 8
#define MAKE_PRINT(X) (((X) < 0x20 || (X) > 0x7E) ? '.' : (X))
#define MIN(X,Y) (((X) < (Y)) ? (X) : (Y))
#define MAX(X,Y) (((X) > (Y)) ? (X) : (Y))

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
void dump_file ();
WINDOW* create_win (window_t*);
void delete_win (WINDOW*);
int main (int, char**);

/* Cursor position */
int cur_x;
int cur_y;
/* Lines of text that fit on screen */
int text;
/* File descriptor for the input file */
int fdes;
/* The file size */
off_t fsize;
/* File is mmapped into here in 4096 byte chunks */
char *file;
/* Window objects */
WINDOW *pos_win;
WINDOW *hex_win;
WINDOW *ascii_win;
/* Dimensions of the windows */
int win_width;

/* The main editor loop */
int start () {
    int input;
    dump_file ();

    /* Initialize the cursor */
    cur_x = 1;
    cur_y = 1;
    wmove (hex_win, cur_y, cur_x);
    wrefresh (hex_win);
    curs_set (1);
    
    while ((input = getch ()) != ESC) {
        switch (input) {
            case UP:
                cur_y--;
                cur_y = MAX(cur_y, 1);
                wmove (hex_win, cur_y, cur_x);
                break;
            case DOWN:
                cur_y++;
                wmove (hex_win, cur_y, cur_x);
                break;
            case LEFT:
                cur_x--;
                cur_x = MAX(cur_x, 1);
                wmove (hex_win, cur_y, cur_x);
                break;
            case RIGHT:
                cur_x++;
                wmove (hex_win, cur_y, cur_x);
                break;
            default:
                mvwprintw (hex_win, cur_y, cur_x, "%c", input);
                cur_x++;
                cur_x = MAX(cur_x, 1);
                break;
        }

        /* Skip the blank spaces between bytes */
        if ((cur_x) % 3 == 0)
            cur_x++;
        wmove (hex_win, cur_y, cur_x);
        wrefresh (hex_win);
    }
    return 0;
}

/* Dumps a chunk of the file to the screen */
void dump_file () {
    off_t cx;

    /* Print the file to the screen */
    for (cx = 0; cx < fsize; cx++) {
        /* Every 8 bytes, move to the next line */
        if (cx % BYTES_PER_LINE == 0) {
            wmove (pos_win, (cx / BYTES_PER_LINE) + 1, 1);
            wmove (hex_win, (cx / BYTES_PER_LINE) + 1, 1);
            wmove (ascii_win, (cx / BYTES_PER_LINE) + 1, 1);
            wprintw (pos_win, "%08X:", cx);
        } else {
            wprintw (hex_win, " ");
        }

        wprintw (hex_win, "%02X", *(file + cx));

        wprintw (ascii_win, "%c", MAKE_PRINT(*(file + cx)));

        wrefresh (pos_win);
        wrefresh (hex_win);
        wrefresh (ascii_win);
    }
}

/* Creates a window from a window_t */
WINDOW* create_win (window_t *w) {
    WINDOW *ret;

    refresh ();
    ret = newwin (w->height, w->width, w->y, w->x);
    wborder (ret, *((w->border) + 0), *((w->border) + 0), *((w->border) + 1),
                  *((w->border) + 1), *((w->border) + 2), *((w->border) + 3),
                  *((w->border) + 4), *((w->border) + 5));

    wrefresh (ret);
    return ret;
}

/* Deletes a window */
void delete_win (WINDOW *win) {
    refresh ();
    wclear (win);
    wrefresh (win);
    delwin (win);
}

/* Just tests the arguments and sets things up */
int main (int argc, char **argv) {
    int ret;
    window_t *w;
    chtype border [] = {
        ACS_VLINE, ACS_HLINE, ACS_ULCORNER,
        ACS_URCORNER, ACS_LLCORNER, ACS_LRCORNER
    };

    if (argc != 2) {
        printf ("Invalid arguments! Usage:\n");
        printf ("    hedit [file]\n");
        return -1;
    }

    /* Initialize ncurses */
    initscr ();
    raw ();
    noecho ();
    keypad (stdscr, TRUE);

    /* Calculate the number of lines of text that fit on the screen
     * number of lines - borders
     */
    text = LINES - 2;

    /* Read a chunk of the file into memory
     * One chunk is at most 1 page
     */
    fdes = open (*(argv + 1), O_RDWR|O_CREAT|O_SYNC);
    fsize = lseek (fdes, 0, SEEK_END);
    fsize = MIN(fsize,text);
    file = mmap (0, fsize, PROT_READ|PROT_WRITE, MAP_SHARED, fdes, 0);

    /* Disable the cursor until it's needed again */
    curs_set (0);

    w = malloc (sizeof (window_t));
    /* Create the position window */
    w->x = 0;
    w->y = 0;
    /* Text field + border */
    w->width = 9 + 2;
    w->height = LINES;
    memcpy (w->border, border, 6 * sizeof (chtype));
    pos_win = create_win (w);
    mvwprintw (pos_win, 0, 1, "POS");
    wrefresh (pos_win);

    /* Create the hex window */
    memset (w, 0, sizeof (window_t));
    w->x = 11;
    w->y = 0;
    win_width = (COLS - 11) / 2;
    w->width = win_width;
    w->height = LINES;
    memcpy (w->border, border, 6 * sizeof (chtype));
    hex_win = create_win (w);
    mvwprintw (hex_win, 0, 1, "HEX");
    wrefresh (hex_win);

    /* Create the ascii window */
    memset (w, 0, sizeof (window_t));
    w->x = 11 + (COLS - 11) / 2;
    w->y = 0;
    w->width = win_width + 1;
    w->height = LINES;
    memcpy (w->border, border, 6 * sizeof (chtype));
    ascii_win = create_win (w);
    mvwprintw (ascii_win, 0, 1, "ASCII");
    wrefresh (ascii_win);
    free (w);

    /* Start the editor */
    ret = start ();

    /* Close ncurses */
    delete_win (hex_win);
    delete_win (ascii_win);
    endwin ();

    /* Free other used memory */
    munmap (file, fsize);
    close (fdes);

    return ret;
}
