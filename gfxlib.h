typedef struct
{
    uint32_t screen_width;
    uint32_t screen_height;
    // OpenGL|ES objects
    EGLDisplay display;
    EGLSurface surface;
    EGLContext context;
} STATE_T;


void Roundrect(VGfloat x, VGfloat y,VGfloat w, VGfloat h, VGfloat rw, VGfloat rh, VGfloat sw, VGfloat fill[4], VGfloat stroke[4]);
void loadfont(const int *Points, const int *PointIndices, const unsigned char *Instructions, const int *InstructionIndices, const int *InstructionCounts, int ng, VGPath *glyphs);
void init_ogl(STATE_T *state);
void exit_func(void);
void Text(VGfloat x, VGfloat y, const char* s, int pointsize, VGfloat fillcolor[4], VGPath *glyphs, const short *characterMap, const int *glyphAdvances, VGbitfield renderFlags);
void Text_Rollover(VGfloat x, VGfloat y, VGfloat maxLength, int maxLines, VGfloat yStep, const char* s, int pointsize, VGfloat fillcolor[4], VGPath *glyphs, const short *characterMap, const int *glyphAdvances, VGbitfield renderFlags);
void setfill(float color[4]);
void setstroke(float color[4], float width);
void Roundrect(VGfloat x, VGfloat y,VGfloat w, VGfloat h, VGfloat rw, VGfloat rh, VGfloat sw, VGfloat fill[4], VGfloat stroke[4]);
void unloadfont(VGPath *glyphs, int n);
void load_DejaVuSans_font();
void unload_DejaVuSans_font();
void Text_DejaVuSans(VGfloat x, VGfloat y, const char* s, int pointsize, VGfloat fillcolor[4]);
void Text_DejaVuSans_Rollover(VGfloat x, VGfloat y,VGfloat maxLength, int maxLines, VGfloat yStep, const char* s, int pointsize, VGfloat fillcolor[4]);
  
VGImage ResizeImage(VGImage vgImageSource, int width, int desired_height);
VGImage createImageFromJpeg(const char *filename, int height);
VGImage createImageFromBuf(unsigned char *buf, unsigned int bufSize, int desired_height);
VGPath newpath();
extern STATE_T _state, *state;

