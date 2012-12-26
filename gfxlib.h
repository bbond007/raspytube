#include "VG/openvg.h"
#include "VG/vgu.h"
#include "EGL/egl.h"
#include "GLES/gl.h"
#define VGfloat float

typedef struct tFontDef
{
    char * name;
    const int *Points; 
    const int *PointIndices; 
    const unsigned char *Instructions; 
    const int *InstructionIndices; 
    const int *InstructionCounts;
    const int *glyphAdvances;
    const short * characterMap; 
    const int glyphCount;
    VGPath *glyphs;
} tFontDef;

typedef struct tColorDef
{
   VGfloat R;
   VGfloat G;
   VGfloat B;
   VGfloat A;
} tColorDef;


typedef struct BITMAP            /* a bitmap structure */
{
    int w, h;                     /* width and height in pixels */
    int stride;
    int bpp;
    unsigned char * data;
} BITMAP;

typedef struct
{
    uint32_t screen_width;
    uint32_t screen_height;
    // OpenGL|ES objects
    EGLDisplay display;
    EGLSurface surface;
    EGLContext context;
    DISPMANX_ELEMENT_HANDLE_T dispman_element;
    DISPMANX_DISPLAY_HANDLE_T dispman_display;
    DISPMANX_UPDATE_HANDLE_T dispman_update;
    EGL_DISPMANX_WINDOW_T nativewindow;
    VC_RECT_T dst_rect;
    VC_RECT_T src_rect;
} STATE_T;

void load_font(tFontDef * fontDef);
void unload_font(tFontDef * fontDef);
void ResizeBitmapRGBA(BITMAP * src, BITMAP * dst);
void Rect(VGfloat x, VGfloat y, VGfloat w, VGfloat h, VGfloat sw, tColorDef * fill, tColorDef *  stroke);
void Roundrect(VGfloat x, VGfloat y,VGfloat w, VGfloat h, VGfloat rw, VGfloat rh, VGfloat sw, tColorDef * fill, tColorDef * stroke);
void loadfont(const int *Points, const int *PointIndices, const unsigned char *Instructions, const int *InstructionIndices, const int *InstructionCounts, int ng, VGPath *glyphs);
void init_ogl(STATE_T *state, bool bQScreen);
void exit_func(void);
void Text(tFontDef * fontDef, VGfloat x, VGfloat y, const char* s, int pointsize, tColorDef * fillcolor, VGbitfield renderFlags);
void Text_Rollover(tFontDef * fontDef, VGfloat x, VGfloat y, VGfloat brkLength, VGfloat maxLength, int maxLines, 
  VGfloat yStep, const char* s, int pointsize, tColorDef * fillcolor, VGbitfield renderFlags, bool bRichTXT);
void Text_Char(tFontDef * fontDef, VGfloat x, VGfloat y, int c, int pointsize, 
               VGfloat sw, tColorDef * fill,  tColorDef *stroke);
void Poly(VGfloat *xy, VGint n, VGfloat sw, tColorDef * fill, tColorDef * stroke, VGboolean dofill);
void setfill(tColorDef * color);
void setstroke(tColorDef *color, float width);
void Roundrect(VGfloat x, VGfloat y,VGfloat w, VGfloat h, VGfloat rw, VGfloat rh, VGfloat sw, tColorDef * fill,  tColorDef * stroke);
void unloadfont(VGPath *glyphs, int n);
void DoSnapshot();  
bool move_window(int x, int y);
VGImage ResizeImage(VGImage vgImageSource, int width, int desired_height);
VGImage createImageFromPNG(const char *filename, int desired_width, int desired_height);
VGImage createImageFromJpeg(const char *filename, int desired_width, int desired_height);
VGImage createImageFromBuf(unsigned char *buf, unsigned int bufSize, int desired_width, int desired_height);
VGImage createImageFromScreen();

VGPath newpath();
extern STATE_T _state, *state;

