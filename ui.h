#include "VG/openvg.h"
#include "VG/vgu.h"

//------------------------------------------------------------------------------

struct result_rec 
{
    char * id;
    char * title;
    char * date;
    char * category;
    char * user;
    char * description;
    char * url;
    char * thumbSmall;
    char * thumbLarge;
    VGImage image; 
    struct result_rec * next;
    struct result_rec * prev;
};

//------------------------------------------------------------------------------
    
typedef enum tVideoPlayer {vpOMXPlayer, vpMPlayer};
typedef enum tSoundOutput {soHDMI, soLOCAL};
typedef enum tJpegDecoder {jdLibJpeg, jdOMX};
//------------------------------------------------------------------------------

void init_ui_var();
struct result_rec * init_result_rec();
void free_result_rec(struct result_rec * rec);
void draw_txt_box(char * message, float widthP, float heightP, float boxYp, float tXp, float tYp, int points, bool swap);
void clear_screen(bool swap);
bool input_string(char * prompt, char * buf, int max);
void show_big_message(char * title, char * message, bool Pause);
void show_selection_info(struct result_rec * rec);
void show_message(char * message, bool error, int points);
void show_youtube_formats();
void clear_output();
void redraw_results(bool swap);
void replace_char_str(char * buf,  char old, char new);
char *parse_url(char * url, char ** server, char ** page);
char ** get_lastrec_column(int iBracket, int iBrace, char * key); 
VGImage load_jpeg(char * url, unsigned int width, unsigned int height);

bool kbHit(void);
int readKb();
void dumpKb();

extern struct result_rec * first_rec;
extern struct result_rec * last_rec;
extern struct result_rec * selected_rec;
extern int numPointFontTiny;
extern int numPointFontSmall;
extern int numPointFontMed;
extern int numPointFontlarge; 
extern int numThumbWidth;
extern int numResults;
extern int numFormat;
extern int numStart;

extern enum tSoundOutput soundOutput;
extern enum tVideoPlayer videoPlayer;
extern enum tJpegDecoder jpegDecoder;

#define ERROR_POINT (numPointFontMed)

extern VGfloat textColor[];
extern VGfloat rectColor[];
extern VGfloat rectColor2[];
extern VGfloat outlineColor[];
extern VGfloat outlineColor2[];
extern VGfloat selectedColor[];
extern VGfloat bgColor[];
extern VGfloat errorColor[];

#define ESC_KEY 0x1b
#define RTN_KEY 0x0a
#define DEL_KEY 0x7f
#define BSL_KEY 0x5c
