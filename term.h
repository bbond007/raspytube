#define VTERM_WIDTH  80
#define VTERM_HEIGHT 25
#define VTERM_CLR    0x00 

typedef struct tScrChr
{
    char c;
    unsigned char color;
    unsigned char attr;
} tScrChr;

typedef struct tTermState
{
    tPointXY offsetXY;
    int tv_width;
    int tv_height;
    tPointXY tvXY;   
    int image_height;
    int image_width;
    tPointXY imageXY;
    tPointXY txtXY;
    int image;
    tScrChr * term_vbuff;
    int term_color;
    int term_cur_x;
    int term_cur_y;
    int term_w;
    int term_h;
    int term_x_inc;
    int term_y_inc;
    int numPointFont;
    int numFont;
} tTermState;
                                         
//virtual terminal functions.         
void term_init     (tTermState * ts, float widthPer, float heightPer, int width, int height);
void term_put_str  (tTermState * ts, char * str);
void term_putc     (tTermState * ts, char c);
void term_putc_xy  (tTermState * ts, int x, int y, char c, unsigned char color);
void term_scroll   (tTermState * ts);
void term_clear    (tTermState * ts, char c);
void term_goto_xy  (tTermState * ts, int x, int y);
void term_del      (tTermState * ts);
void term_set_color(tTermState * ts, unsigned char color);
void term_show     (tTermState * ts, bool swap);
void term_free     (tTermState * ts);
void term_command  (tTermState * ts, char * command);

