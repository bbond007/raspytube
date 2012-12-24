// parts of thisKfFill come from "shapes.c":
// shapes: minimal program to explore OpenVG
// Anthony Starks (ajstarks@gmail.com)
// and "test_image.c"
// ShivaVG / Ivan Leben <ivan.leben@gmail.com>
// ShivaVG - an open-source LGPL ANSI C implementation of the OpenVG specificatio


#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <termios.h>
#include <assert.h>
#include <jpeglib.h>
#include <ctype.h>
#include "VG/openvg.h"
#include "VG/vgu.h"
#include "EGL/egl.h"
#include "GLES/gl.h"
#include "gfxlib.h"
#include "lodepng.h"

STATE_T _state, *state=&_state;
#define ERROR_POINT (numPointFontMed)
void show_message(char * message, bool error, int points);
extern int numPointFontMed;

VGImageFormat getRGBAFormat()
{
    unsigned int lilEndianTest = 1;
    // Check for endianness
    if (((unsigned char *)&lilEndianTest)[0] == 1)
        return VG_sABGR_8888;
    else
        return VG_sRGBA_8888;
}

/*
VGImage createImageFromPNG(const char *filename, int desired_width, int desired_height)
{
    VGImage img = 0;
    unsigned int error;
    BITMAP bitmap;
    error = lodepng_decode32_file(&bitmap.data, &bitmap.w, &bitmap.h, filename);
    char errorStr[128];
    if(error)
    {
        snprintf(errorStr, sizeof(errorStr),
                 "error %u: %s\n", error, lodepng_error_text(error));
        show_message(errorStr, TRUE, ERROR_POINT);
        return 0;
    }
    bitmap.bpp    = 4;
    bitmap.stride = bitmap.w * bitmap.bpp;
    BITMAP * pBitmap = &bitmap;
    if((desired_width != bitmap.w || desired_height != bitmap.h) && 
       (desired_width != 0 && desired_height != 0))
    {
        BITMAP newBM;
        newBM.w = desired_width;
        newBM.h = desired_height;
        newBM.bpp = 4;
        newBM.stride = newBM.w * newBM.bpp;
        newBM.data = malloc(newBM.stride * newBM.h);
        ResizeBitmapRGBA(&bitmap, &newBM);
        pBitmap = &newBM;
        free(bitmap.data);
    }
    img = vgCreateImage(getRGBAFormat(), pBitmap->w, pBitmap->h, VG_IMAGE_QUALITY_BETTER);
    vgImageSubData(img, &pBitmap->data[pBitmap->h * pBitmap->stride],
                   pBitmap->stride * -1, getRGBAFormat(), 0, 0, pBitmap->w, pBitmap->h);
    free(pBitmap->data);
    return img;
}
*/
// createImageFromJpeg decompresses a JPEG image to the standard image format
// source: https://github.com/ileben/ShivaVG/blob/master/examples/test_image.c
VGImage createImageFromJpeg(const char *filename, int desired_width, int desired_height)
{
    FILE *infile;
    struct jpeg_decompress_struct jdc;
    struct jpeg_error_mgr jerr;
    JSAMPARRAY buffer;
    unsigned int bstride;
    unsigned int bbpp;

    VGImage img;
    
    BITMAP bitmap;
    BITMAP * pBitmap = &bitmap;
    VGubyte *brow;
    VGubyte *drow;
    unsigned int x;
    // Try to open image file
    infile = fopen(filename, "rb");
    if (infile == NULL)
    {
        printf("Failed opening '%s' for reading!\n", filename);
        return VG_INVALID_HANDLE;
    }

    // Setup default error handling
    jdc.err = jpeg_std_error(&jerr);
    jpeg_create_decompress(&jdc);

    // Set input file
    jpeg_stdio_src(&jdc, infile);


    // Read header and start
    jpeg_read_header(&jdc, TRUE);
    jdc.scale_num = 1;
    jdc.scale_denom = 1;

    jpeg_start_decompress(&jdc);
    bitmap.w  = jdc.output_width;
    bitmap.h = jdc.output_height;


    // Allocate buffer using jpeg allocator
    bbpp = jdc.output_components;
    bstride = bitmap.w * bbpp;
    buffer = (*jdc.mem->alloc_sarray)
             ((j_common_ptr) & jdc, JPOOL_IMAGE, bstride, 1);

    // Allocate image data buffer
    bitmap.bpp = 4;
    bitmap.stride = bitmap.w * bitmap.bpp;
    bitmap.data = (VGubyte *) malloc(bitmap.stride * bitmap.h);

    // Iterate until all scanlines processed
    while (jdc.output_scanline < bitmap.h)
    {

        // Read scanline into buffer
        jpeg_read_scanlines(&jdc, buffer, 1);
        drow = bitmap.data + (bitmap.h - jdc.output_scanline) * bitmap.stride;
        brow = buffer[0];
        // Expand to RGBA
        for (x = 0; x < bitmap.w; ++x, drow += bitmap.bpp, brow += bbpp)
        {
            switch (bbpp)
            {
            case 4:
                drow[0] = brow[0];
                drow[1] = brow[1];
                drow[2] = brow[2];
                drow[3] = brow[3];
                break;
            case 3:
                drow[0] = brow[0];
                drow[1] = brow[1];
                drow[2] = brow[2];
                drow[3] = 255;
                break;
            }
        }
    }

    if((desired_width != bitmap.w || desired_height != bitmap.h) && 
       (desired_width != 0 && desired_height != 0))
   
    {
        BITMAP newBM;
        newBM.w = desired_width;
        newBM.h = desired_height;
        newBM.bpp = 4;
        newBM.stride = newBM.w * newBM.bpp;
        newBM.data = malloc(newBM.stride * newBM.h);
        ResizeBitmapRGBA(&bitmap, &newBM);
        pBitmap = &newBM;
        free(bitmap.data);
    }

    img = vgCreateImage(getRGBAFormat(), pBitmap->w, pBitmap->h, VG_IMAGE_QUALITY_BETTER);
    vgImageSubData(img, pBitmap->data, pBitmap->stride, getRGBAFormat(), 0, 0, pBitmap->w, pBitmap->h);
    // Cleanup
    jpeg_destroy_decompress(&jdc);
    free(pBitmap->data);

    return img;
}

VGImage createImageFromBuf(unsigned char *buf, unsigned int bufSize, int desired_width, int desired_height)
{
    struct jpeg_decompress_struct jdc;
    struct jpeg_error_mgr jerr;
    JSAMPARRAY buffer;
    unsigned int bstride;
    unsigned int bbpp;

    VGImage img;
    BITMAP  bitmap;
    BITMAP * pBitmap = &bitmap;
    VGubyte *brow;
    VGubyte *drow;
    unsigned int x;

    // Setup default error handling
    jdc.err = jpeg_std_error(&jerr);
    jpeg_create_decompress(&jdc);

    // Set input file
    jpeg_mem_src(&jdc, buf, bufSize);


    // Read header and start
    jpeg_read_header(&jdc, TRUE);
    jdc.scale_num = 1;
    jdc.scale_denom = 1;

    jpeg_start_decompress(&jdc);
    bitmap.w = jdc.output_width;
    bitmap.h = jdc.output_height;


    // Allocate buffer using jpeg allocator
    bbpp = jdc.output_components;
    bstride = bitmap.w * bbpp;
    buffer = (*jdc.mem->alloc_sarray)
             ((j_common_ptr) & jdc, JPOOL_IMAGE, bstride, 1);

    // Allocate image data buffer
    bitmap.bpp = 4;
    bitmap.stride = bitmap.w * bitmap.bpp;
    bitmap.data = malloc(bitmap.stride * bitmap.h);

    // Iterate until all scanlines processed
    while (jdc.output_scanline < bitmap.h)
    {

        // Read scanline into buffer
        jpeg_read_scanlines(&jdc, buffer, 1);
        drow = bitmap.data + (bitmap.h - jdc.output_scanline) * bitmap.stride;
        brow = buffer[0];
        // Expand to RGBA
        for (x = 0; x < bitmap.w; ++x, drow += bitmap.bpp, brow += bbpp)
        {
            switch (bbpp)
            {
            case 4:
                drow[0] = brow[0];
                drow[1] = brow[1];
                drow[2] = brow[2];
                drow[3] = brow[3];
                break;
            case 3:
                drow[0] = brow[0];
                drow[1] = brow[1];
                drow[2] = brow[2];
                drow[3] = 255;
                break;
            }
        }
    }

    if((desired_width != bitmap.w || desired_height != bitmap.h) && 
       (desired_width != 0 && desired_height != 0))
    {
        BITMAP newBM;
        newBM.w = desired_width;
        newBM.h = desired_height;
        newBM.bpp = 4;
        newBM.stride = newBM.w * newBM.bpp;
        newBM.data = malloc(newBM.stride * newBM.h);
        ResizeBitmapRGBA(&bitmap, &newBM);
        pBitmap = &newBM;
        free(bitmap.data);
    }

    img = vgCreateImage(getRGBAFormat(), pBitmap->w, pBitmap->h, VG_IMAGE_QUALITY_BETTER);
    vgImageSubData(img, pBitmap->data, pBitmap->stride, getRGBAFormat(), 0, 0, pBitmap->w, pBitmap->h);
    // Cleanup
    jpeg_destroy_decompress(&jdc);
    free(pBitmap->data);
    return img;
}

//------------------------------------------------------------------------------
void ResizeBitmapRGBA(BITMAP * src, BITMAP * dst)
{
    // EDIT: added +1 to account for an early rounding problem
    int x_ratio = (int)((src->w<<16)/dst->w) +1;
    int y_ratio = (int)((src->h<<16)/dst->h) +1;
    int x2, y2, i, j;
    unsigned int * psrc = (unsigned int *) src->data;
    unsigned int * pdst = (unsigned int *) dst->data;
    
    for (i=0; i<dst->h; i++)
    {
        for (j=0; j<dst->w; j++)
        {
            x2 = ((j*x_ratio)>>16) ;
            y2 = ((i*y_ratio)>>16) ;
            pdst[(i*dst->w)+j] = psrc[(y2*src->w)+x2] ;
        }
    }
}

//------------------------------------------------------------------------------
void ResizeBitmap8BITIDX(BITMAP * src, BITMAP * dst)
{
    // EDIT: added +1 to account for an early rounding problem
    int x_ratio = (int)((src->w<<16)/dst->w) +1;
    int y_ratio = (int)((src->h<<16)/dst->h) +1;
    int x2, y2, i, j;
    unsigned char * psrc = (unsigned char *) src->data;
    unsigned char * pdst = (unsigned char *) dst->data;
    
    for (i=0; i<dst->h; i++)
    {
        for (j=0; j<dst->w; j++)
        {
            x2 = ((j*x_ratio)>>16) ;
            y2 = ((i*y_ratio)>>16) ;
            pdst[(i*dst->w)+j] = psrc[(y2*src->w)+x2] ;
        }
    }
}

//------------------------------------------------------------------------------
//
VGImage ResizeImage(VGImage src, int width, int height)
{
/*
    int orig_width = vgGetParameteri(src, VG_IMAGE_WIDTH);
    int orig_height = vgGetParameteri(src, VG_IMAGE_HEIGHT);
    printf("(%d, %d)\n", orig_width, orig_height);

    VGImage dst = vgCreateImage(getRGBAFormat(), width, height, VG_IMAGE_QUALITY_BETTER);
    
    	vgCopyImage(dst, VGint dx, VGint dy,
                                 VGImage src, VGint sx, VGint sy,
                                 VGint width, VGint height,
                                 VGboolean dither) VG_API_EXIT;
                                
 */
    return src;
}
//------------------------------------------------------------------------------
// Roundrect makes an rounded rectangle at the specified location and dimensions, applying style
void Roundrect(
    VGfloat x, VGfloat y,
    VGfloat w, VGfloat h,
    VGfloat rw, VGfloat rh,
    VGfloat sw, tColorDef * fill,  tColorDef *stroke)
{
    VGPath path = newpath();
    vguRoundRect(path, x, y, w, h, rw, rh);
    setfill(fill);
    setstroke(stroke, sw);
    vgDrawPath(path, VG_FILL_PATH | VG_STROKE_PATH);
    vgDestroyPath(path);
}

//------------------------------------------------------------------------------
// Rect makes a rectangle  at the specified location and dimensions, applying style
void Rect(VGfloat x, VGfloat y, VGfloat w, VGfloat h, VGfloat sw, tColorDef * fill, tColorDef * stroke) 
{
	VGPath path = newpath();
	vguRect(path, x, y, w, h);
	setfill(fill);
	setstroke(stroke, sw);
	vgDrawPath(path, VG_FILL_PATH | VG_STROKE_PATH);
	vgDestroyPath(path);
}

//------------------------------------------------------------------------------
// loadfont loads font path data
void  loadfont(const int *Points, const int *PointIndices, const unsigned char *Instructions, const int *InstructionIndices, const int *InstructionCounts, int ng, VGPath *glyphs)
{
    int i;
    memset(glyphs, 0, ng*sizeof(VGPath));
    for(i=0; i < ng; i++)
    {
        const int* p = &Points[PointIndices[i]*2];
        const unsigned char* instructions = &Instructions[InstructionIndices[i]];
        int ic = InstructionCounts[i];

        VGPath path = vgCreatePath(VG_PATH_FORMAT_STANDARD, VG_PATH_DATATYPE_S_32, 1.0f/65536.0f, 0.0f, 0, 0, VG_PATH_CAPABILITY_ALL);
        glyphs[i] = path;
        if(ic)
        {
            vgAppendPathData(path, ic, instructions, p);
        }
    }
}

//------------------------------------------------------------------------------
// loadfont loads font path data
void  load_font(tFontDef * fontDef)
{
    int i;
    size_t size = fontDef->glyphCount*sizeof(VGPath);
    fontDef->glyphs = malloc(size);
    memset(fontDef->glyphs, 0, size);
    for(i=0; i < fontDef->glyphCount; i++)
    {
        const int* p = &fontDef->Points[fontDef->PointIndices[i]*2];
        const unsigned char* instructions = &fontDef->Instructions[fontDef->InstructionIndices[i]];
        int ic = fontDef->InstructionCounts[i];

        VGPath path = vgCreatePath(VG_PATH_FORMAT_STANDARD, VG_PATH_DATATYPE_S_32, 1.0f/65536.0f, 0.0f, 0, 0, VG_PATH_CAPABILITY_ALL);
        fontDef->glyphs[i] = path;
        if(ic)
        {
            vgAppendPathData(path, ic, instructions, p);
        }
    }
}

//------------------------------------------------------------------------------

VGPath newpath()
{
    return vgCreatePath(VG_PATH_FORMAT_STANDARD, VG_PATH_DATATYPE_F, 1.0f, 0.0f, 0, 0, VG_PATH_CAPABILITY_ALL);
}


//------------------------------------------------------------------------------
// unloadfont frees font path data
void unloadfont(VGPath *glyphs, int n)
{
    int i;
    for(i=0; i<n; i++)
    {
        vgDestroyPath(glyphs[i]);
    }
}

//------------------------------------------------------------------------------
// unloadfont frees font path data
void unload_font(tFontDef * fontDef)
{
    int i;
    for(i=0; i<fontDef->glyphCount; i++)
    {
        vgDestroyPath(fontDef->glyphs[i]);
    }
    free(fontDef->glyphs);
}


//------------------------------------------------------------------------------
// setfill sets the fill color
void setfill(tColorDef * color)
{
    VGPaint fillPaint = vgCreatePaint();
    vgSetParameteri(fillPaint, VG_PAINT_TYPE, VG_PAINT_TYPE_COLOR);
    vgSetParameterfv(fillPaint, VG_PAINT_COLOR, 4, (VGfloat *) color);
    vgSetPaint(fillPaint, VG_FILL_PATH);
    vgDestroyPaint(fillPaint);
}

//------------------------------------------------------------------------------
// setstroke sets the stroke color and width
void setstroke(tColorDef * color, float width)
{
    VGPaint strokePaint = vgCreatePaint();
    vgSetParameteri(strokePaint, VG_PAINT_TYPE, VG_PAINT_TYPE_COLOR);
    vgSetParameterfv(strokePaint, VG_PAINT_COLOR, 4, (VGfloat *) color);
    vgSetPaint(strokePaint, VG_STROKE_PATH);
    vgSetf(VG_STROKE_LINE_WIDTH, width);
    vgSeti(VG_STROKE_CAP_STYLE, VG_CAP_BUTT);
    vgSeti(VG_STROKE_JOIN_STYLE, VG_JOIN_MITER);
    vgDestroyPaint(strokePaint);
}

//------------------------------------------------------------------------------
// Text renders a string of text at a specified location, using the specified font glyphs
void Text(tFontDef * fontDef, VGfloat x, VGfloat y, const char* s, int pointsize, tColorDef * fillcolor, VGbitfield renderFlags)
{
    float size = (float)pointsize;
    float xx = x;
    float mm[9];
    int i;
    vgGetMatrix(mm);
    setfill(fillcolor);
    for(i=0; i < (int)strlen(s); i++)
    {
        unsigned int character = (unsigned int)s[i];
        int glyph = fontDef->characterMap[character];
        if( glyph == -1 )
        {
            continue;	//glyph is undefined
        }

        VGfloat mat[9] =
        {
            size,	0.0f,	0.0f,
            0.0f,	size,	0.0f,
            xx,		y,		1.0f
        };

        vgLoadMatrix(mm);
        vgMultMatrix(mat);
        vgDrawPath(fontDef->glyphs[glyph], renderFlags);
        xx += size * fontDef->glyphAdvances[glyph] / 65536.0f;
    }
    vgLoadMatrix(mm);
}
//------------------------------------------------------------------------------ 
// Text renders a string of text at a specified location, using the specified font glyphs
void Text_Char(tFontDef * fontDef, VGfloat x, VGfloat y, int c, int pointsize, 
               VGfloat sw, tColorDef * fill,  tColorDef *stroke)
{
    int glyph = fontDef->characterMap[c];
    if( glyph != -1 )
    {
        float size = (float)pointsize;
        float mm[9];
        vgGetMatrix(mm);
        setfill(fill);
        VGfloat mat[9] =
        {
            size,	0.0f,	0.0f,
            0.0f,	size,	0.0f,
            x,		y,		1.0f
        };
        vgLoadMatrix(mm);
        vgMultMatrix(mat);
        vgDrawPath(fontDef->glyphs[glyph], VG_FILL_PATH);// | VG_STROKE_PATH);
        vgLoadMatrix(mm);
    }
}

//------------------------------------------------------------------------------
void Text_getHeight(tFontDef * fontDef, int pointsize)
{


}
//------------------------------------------------------------------------------
// Text renders a string of text at a specified location, using the specified font glyphs
void Text_Rollover(tFontDef * fontDef, VGfloat x, VGfloat y, VGfloat brkLength, VGfloat maxLength, int maxLines, 
  VGfloat yStep, const char* s, int pointsize, tColorDef * fillcolor, VGbitfield renderFlags, bool bRichTXT)
{
    float size = (float)pointsize;
    float xx = x;
    float yy = y;
    float mm[9];
    int i;
    int iLines = 0;
    vgGetMatrix(mm);
    setfill(&fillcolor[0]);

    int max = (int)strlen(s);
    
    for(i=0; i < max; i++)
    {
        unsigned int character = (unsigned int)s[i];
        int glyphWidth = 0;
        if(bRichTXT && character == '~' && (i+1) < max)
        {
             char sColor[2] = {s[++i], 0x00};
             int color = atoi(&sColor[0]);
             setfill(&fillcolor[color]);
        }
        else
        {
            int glyph = fontDef->characterMap[character];            
            if( glyph != -1 && (character != ' ' || xx != x || yy == y))
            {	
                VGfloat mat[9] =
                {
                   size,	0.0f,	0.0f,
                   0.0f,	size,	0.0f,
                   xx,		yy,	1.0f
                };

                vgLoadMatrix(mm);
                vgMultMatrix(mat);
                vgDrawPath(fontDef->glyphs[glyph], renderFlags);
                glyphWidth = size * fontDef->glyphAdvances[glyph] / 65536.0f;
                xx += glyphWidth;
            }
        }
        if((character == '\n' || (xx + glyphWidth) >= maxLength)  || (xx >= brkLength && !isalnum(character))) //autoroll
        {
            xx = x;
            yy -= yStep;
            iLines++;
            if (maxLines > 0 && iLines == maxLines)
                break;
        }
        
    }
    vgLoadMatrix(mm);
}

//------------------------------------------------------------------------------
// Poly makes a stroked polyline or a stroked and filled polygon
void Poly(VGfloat *xy, VGint n, VGfloat sw, tColorDef * fill, tColorDef * stroke, VGboolean dofill)
{
    VGPath path = newpath();
    VGbitfield pflag;

    //interleave(x, y, n, points);
    vguPolygon(path, xy, n, VG_FALSE);
    if (dofill)
    {
        setfill(fill);
        pflag = VG_FILL_PATH | VG_STROKE_PATH;
    }
    else
    {
        pflag = VG_STROKE_PATH;
    }

    setstroke(stroke, sw);
    vgDrawPath(path, pflag);
    vgDestroyPath(path);
}

//------------------------------------------------------------------------------
// init_ogl sets the display, OpenGL|ES context and screen information
// state holds the OGLES model information
void init_ogl(STATE_T *state, bool bQScreen)
{
    int32_t success = 0;
    EGLBoolean result;
    EGLint num_config;

    static EGL_DISPMANX_WINDOW_T nativewindow;

    DISPMANX_ELEMENT_HANDLE_T dispman_element;
    DISPMANX_DISPLAY_HANDLE_T dispman_display;
    DISPMANX_UPDATE_HANDLE_T dispman_update;
    VC_RECT_T dst_rect;
    VC_RECT_T src_rect;

    static const EGLint attribute_list[] =
    {
        EGL_RED_SIZE, 8,
        EGL_GREEN_SIZE, 8,
        EGL_BLUE_SIZE, 8,
        EGL_ALPHA_SIZE, 8,
        EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
        EGL_NONE
    };

    EGLConfig config;

    // get an EGL display connection
    state->display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    assert(state->display!=EGL_NO_DISPLAY);

    // initialize the EGL display connection
    result = eglInitialize(state->display, NULL, NULL);
    assert(EGL_FALSE != result);

    // bind OpenVG API
    eglBindAPI(EGL_OPENVG_API);

    // get an appropriate EGL frame buffer configuration
    result = eglChooseConfig(state->display, attribute_list, &config, 1, &num_config);
    assert(EGL_FALSE != result);

    // create an EGL rendering context
    state->context = eglCreateContext(state->display, config, EGL_NO_CONTEXT, NULL);
    assert(state->context!=EGL_NO_CONTEXT);

    // create an EGL window surface
    success = graphics_get_display_size(0 /* LCD */, &state->screen_width, &state->screen_height);
    if(bQScreen)
    {
        state->screen_width = state->screen_width / 2;
        state->screen_height = state->screen_height / 2;
    }
    
    assert( success >= 0 );

    dst_rect.x = 0;
    dst_rect.y = 0;
    dst_rect.width = state->screen_width;
    dst_rect.height = state->screen_height;

    src_rect.x = 0;
    src_rect.y = 0;
    src_rect.width = state->screen_width << 16;
    src_rect.height = state->screen_height << 16;

    dispman_display = vc_dispmanx_display_open( 0 /* LCD */);
    dispman_update = vc_dispmanx_update_start( 0 );

    dispman_element = vc_dispmanx_element_add ( dispman_update, dispman_display,
                      0/*layer*/, &dst_rect, 0/*src*/,
                      &src_rect, DISPMANX_PROTECTION_NONE, 0 /*alpha*/, 0/*clamp*/, 0/*transform*/);

    nativewindow.element = dispman_element;
    nativewindow.width = state->screen_width;
    nativewindow.height = state->screen_height;
    vc_dispmanx_update_submit_sync( dispman_update );

    state->surface = eglCreateWindowSurface( state->display, config, &nativewindow, NULL );
    assert(state->surface != EGL_NO_SURFACE);

    // connect the context to the surface
    result = eglMakeCurrent(state->display, state->surface, state->surface, state->context);
    assert(EGL_FALSE != result);

    //DAVE - Set up screen ratio
    glViewport(0, 0, (GLsizei)state->screen_width, (GLsizei)state->screen_height);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    float ratio = (float)state->screen_width / (float)state->screen_height;
    glFrustumf(-ratio, ratio, -1.0f, 1.0f, 1.0f, 10.0f);
}

//------------------------------------------------------------------------------
// exit_func cleans up
void exit_func(void)
{
    // clear screen
    glClear( GL_COLOR_BUFFER_BIT );
    eglSwapBuffers(state->display, state->surface);

    // Release OpenGL resources
    eglMakeCurrent( state->display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT );
    eglDestroySurface( state->display, state->surface );
    eglDestroyContext( state->display, state->context );
    eglTerminate( state->display );
}

//------------------------------------------------------------------------------
int write_jpeg_file(FILE * outputFile, unsigned char * bitmapData, unsigned int Width, unsigned int Height)
{
    struct jpeg_compress_struct cinfo;
    struct jpeg_error_mgr jerr;

    /*thisisapointertoonerowofimagedata*/
    JSAMPROW row_pointer[1];
    cinfo.err=jpeg_std_error(&jerr);
    jpeg_create_compress(&cinfo);
    jpeg_stdio_dest(&cinfo,outputFile);

    /*Settingtheparametersoftheoutputfilehere*/
    cinfo.image_width=Width;
    cinfo.image_height=Height;
    cinfo.input_components=3;
    cinfo.in_color_space=JCS_RGB;
    /*defaultcompressionparameters,weshouldn'tbeworriedaboutthese*/
    jpeg_set_defaults(&cinfo);
    /*Nowdothecompression..*/
    jpeg_start_compress(&cinfo,TRUE);
    /*likereadingafile,thistimewriteonerowatatime*/
    unsigned char * dst;
    unsigned char * src;
    unsigned int src_stride = cinfo.image_width * 4;
    unsigned int dst_stride = cinfo.image_width * 3;

    unsigned char * outputBuf = malloc(dst_stride);

    while(cinfo.next_scanline<cinfo.image_height)
    {
        row_pointer[0]=outputBuf;
        dst = outputBuf;
        src = &bitmapData[(cinfo.image_height - cinfo.next_scanline -1) * src_stride];
        unsigned int count;
        for (count = 0; count < cinfo.image_width; count++)
        {
            *dst = *src;
            dst++;
            src++;
            *dst = *src;
            dst++;
            src++;
            *dst = *src;
            src++;
            dst++;
            src++;
        }
        //bitmapData[cinfo.next_scanline*cinfo.image_width*cinfo.input_components];
        jpeg_write_scanlines(&cinfo,row_pointer,1);
    }
    free(outputBuf);
    /*similartoreadfile,cleanupafterwe'redonecompressing*/
    jpeg_finish_compress(&cinfo);
    jpeg_destroy_compress(&cinfo);
    /*successcodeis1!*/
    return 1;
}

//------------------------------------------------------------------------------
void DoSnapshot()
{

    unsigned int bitmapSize = state->screen_width * state->screen_height * 4;
    unsigned char * bitmapData = malloc(bitmapSize);;
    vgReadPixels(bitmapData, state->screen_width * 4,
                 VG_sABGR_8888,
                 0,0,
                 state->screen_width, state->screen_height);

    FILE *outputFile = fopen("snapshot.jpg", "wb");
    if (!outputFile)
    {
        show_message("bogus!\nfopen failed", true, ERROR_POINT);
        return;
    }

    write_jpeg_file(outputFile, bitmapData, state->screen_width, state->screen_height);
    fclose(outputFile);
    free(bitmapData);
}

//------------------------------------------------------------------------------
VGImage createImageFromScreen()
{
    VGImage vgImage = vgCreateImage(getRGBAFormat(), state->screen_width, state->screen_height, VG_IMAGE_QUALITY_BETTER);
    vgGetPixels(vgImage, 0,0, 0,0,state->screen_width, state->screen_height);
    return vgImage;
}
