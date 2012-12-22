#include "VG/openvg.h"
#include "VG/vgu.h"

#define VGfloat float

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
    VGImage largeImage; 
    struct result_rec * next;
    struct result_rec * prev;
};

//------------------------------------------------------------------------------
typedef struct _tMenuItem
{
    char * description;
    char * key;
    int special;
} tMenuItem;

typedef struct _tPointPer
{
  float xPer;
  float yPer;
} tPointPer;

typedef struct _tPointXY
{
  int x;
  int y;
} tPointXY;

typedef struct _tRectPer
{
  float xPer;
  float yPer;
  float wPer;
  float hPer; 
} tRectPer;

typedef struct _tRectBounds
{
  int x;
  int y;
  int w;
  int h; 
} tRectBounds;

typedef struct _tMenuState
{
    char * title;
    tPointPer titlePer;
    tPointXY titlePos;
    tMenuItem * menuItems;
    int selectedIndex;
    int maxItems;
    int scrollIndex;
    int yStep;
    tPointXY txtOffset;
    tPointXY txtRaster;
    int selectedItem;
    int numPointFont;
    int numPointFontTitle;
    tRectPer winPer;
    tRectPer selPer;
    tRectBounds winRect;
    tRectBounds selRect;
    VGfloat upArrow[8];
    VGfloat downArrow[8]; 
    tRectPer upArrowPer;
    tRectPer downArrowPer;
    tRectBounds upArrowRect;
    tRectBounds downArrowRect;        
    bool bCenterX;
    bool bCenterY;
    void (*drawHeader) (struct _tMenuState * menu);
    void (*drawDetail) (struct _tMenuState * menu);
    void (*drawFooter) (struct _tMenuState * menu);
    void (*keyPress)   (struct _tMenuState * menu, int key);
} tMenuState; 
//------------------------------------------------------------------------------
    
typedef enum tVideoPlayer {vpMPlayer   = 1, vpOMXPlayer = 0} tVideoPlayer;
typedef enum tSoundOutput {soHDMI      = 1, soLOCAL     = 0} tSoundOutput;
typedef enum tJpegDecoder {jdLibJpeg   = 1, jdOMX       = 0} tJpegDecoder;

//------------------------------------------------------------------------------

void init_ui_var();
void free_ui_var();
struct result_rec * init_result_rec();
void free_result_rec(struct result_rec * rec);
void draw_txt_box_cen(
                  char * message, 
                  float widthP, 
                  float heightP, 
                  float boxYp, 
                  float tXp, 
                  float tYp, 
                  int points);

void clear_screen(bool swap);
bool input_string(char * prompt, char * buf, int max);
void show_big_message(char * title, char * message);
int  show_selection_info(struct result_rec * rec);
void show_message(char * message, int error, int points);
void clear_output();
void redraw_results(bool swap);
void replace_char_str(char * buf,  char old, char new);
char *parse_url(char * url, char ** server, char ** page);
char ** get_lastrec_column(int iBracket, int iBrace, char * key); 
int show_menu(tMenuState * menu);
void calc_point_xy(tPointPer * pointPer, tPointXY * pointXY);
void calc_rect_bounds(tRectPer * rectPer, tRectBounds * rectBounds);
bool point_in_rect(tPointXY * point, tRectBounds * rect);
void init_font_menus();
void init_big_menu(tMenuState * menu, char * title);
void init_format_menu(tMenuState * menu);
void init_small_menu(tMenuState * menu, char * title);
void set_menu_value(tMenuState * menu, int value);
int show_format_menu(tMenuState * menu);
VGImage load_jpeg(char * url, unsigned int width, unsigned int height);
VGImage load_jpeg2(char * url, unsigned int width, unsigned int height, 
    unsigned char ** downloadData, unsigned char ** imageData, unsigned int * imageDataSize);
void set_font(int font);
void set_title_font(int font);
int get_font();
int get_title_font();
void drawBGImage();
void setBGImage();
void main_menu_detail(tMenuState * menu);
void font_menu_detail(tMenuState * menu);
void jskb_menu_detail(tMenuState * menu);
void gui_menu_detail(tMenuState * menu);
void gui_menu_keypress(tMenuState * menu, int key);
void jskb_menu_keypress(tMenuState * menu, int key);
bool yes_no_dialog(char * prompt, bool value);
extern struct result_rec * first_rec;
extern struct result_rec * last_rec;
extern struct result_rec * selected_rec;
extern int numPointFontTiny;
extern int numPointFontSmall;
extern int numPointFontMed;
extern int numPointFontLarge; 
extern int numThumbWidth;
extern int numRow;
extern int numCol;
extern int numResults;
extern int numFormat;
extern int numStart;
extern int numRectPenSize;
//extern int numResultsReturned;
extern int numFontSpacing;
extern enum tSoundOutput soundOutput;
extern enum tVideoPlayer videoPlayer;
extern enum tJpegDecoder jpegDecoder;

#define ERROR_POINT (numPointFontMed)

extern tColorDef * textColor;
extern tColorDef * rectColor;
extern tColorDef * rectColor2;
extern tColorDef * rectColor3;
extern tColorDef * outlineColor;
extern tColorDef * outlineColor2;
extern tColorDef * selectedColor;
extern tColorDef * bgColor;
extern tColorDef * errorColor;

#define ESC_KEY 0x1b
#define RTN_KEY 0x0a
#define DEL_KEY 0x7f
#define BSL_KEY 0x5c
#define TERM_CUR_UP  'A'
#define TERM_CUR_DWN 'B'
#define TERM_CUR_R   'C'
#define TERM_CUR_L   'D'
#define TERM_FUN_1   'P'
#define TERM_FUN_2   'Q'
#define CUR_UP  17
#define CUR_DWN 18
#define CUR_R   19
#define CUR_L   20
#define FUN_1 	1 
#define FUN_2	2
#define JOY_1   3
#define MOUSE_1 4
#define MOUSE_2 5
#define MOUSE_M 6


