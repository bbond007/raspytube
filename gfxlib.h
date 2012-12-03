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
} STATE_T;


void load_font(tFontDef * fontDef);
void unload_font(tFontDef * fontDef);
void ResizeBitmapRGBA(BITMAP * src, BITMAP * dst);
void Rect(VGfloat x, VGfloat y, VGfloat w, VGfloat h, VGfloat sw, VGfloat fill[4], VGfloat stroke[4]);
void Roundrect(VGfloat x, VGfloat y,VGfloat w, VGfloat h, VGfloat rw, VGfloat rh, VGfloat sw, VGfloat fill[4], VGfloat stroke[4]);
void loadfont(const int *Points, const int *PointIndices, const unsigned char *Instructions, const int *InstructionIndices, const int *InstructionCounts, int ng, VGPath *glyphs);
void init_ogl(STATE_T *state);
void exit_func(void);
void Text(tFontDef * fontDef, VGfloat x, VGfloat y, const char* s, int pointsize, VGfloat fillcolor[4], VGbitfield renderFlags);
void Text_Rollover(tFontDef * fontDef, VGfloat x, VGfloat y, VGfloat maxLength, int maxLines, 
  VGfloat yStep, const char* s, int pointsize, VGfloat fillcolor[4], VGbitfield renderFlags);
void Poly(VGfloat *xy, VGint n, VGfloat sw, VGfloat fill[4], VGfloat stroke[4], VGboolean dofill);
void setfill(float color[4]);
void setstroke(float color[4], float width);
void Roundrect(VGfloat x, VGfloat y,VGfloat w, VGfloat h, VGfloat rw, VGfloat rh, VGfloat sw, VGfloat fill[4], VGfloat stroke[4]);
void unloadfont(VGPath *glyphs, int n);
void DoSnapshot();  
VGImage ResizeImage(VGImage vgImageSource, int width, int desired_height);
VGImage createImageFromPNG(const char *filename, int desired_width, int desired_height);
VGImage createImageFromJpeg(const char *filename, int desired_width, int desired_height);
VGImage createImageFromBuf(unsigned char *buf, unsigned int bufSize, int desired_width, int desired_height);
VGImage createImageFromScreen();

VGPath newpath();
extern STATE_T _state, *state;

