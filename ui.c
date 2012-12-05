#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <netdb.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>
#include <time.h>
#include <termios.h>
#include <fcntl.h>
#include <ctype.h>
#include "VG/openvg.h"
#include "VG/vgu.h"
#include "EGL/egl.h"
#include "GLES/gl.h"
#include "gfxlib.h"
#include "videoformats.h"
#include "ui.h"
#include "audio.h"
#include "fonts.inc"
#include "menu_arrow_up.inc"
#include "menu_arrow_down.inc"

//#include "menus.h"
//stuff for the keyboard.

static int ttflags;
static struct termios oldt;
static struct termios newt;
/* sounds disabled because it sucked
static AudioSampleInfo asiKbClick;
extern const signed char soundraw_data[];
extern const unsigned int soundraw_size;
*/
static VGImage bgImage           = 0;
static VGImage tvImage           = 0;
static VGImage upArrowImage      = 0;
static VGImage downArrowImage    = 0;
struct result_rec * first_rec    = NULL;
struct result_rec * last_rec     = NULL;
struct result_rec * selected_rec = NULL;

int numPointFontTiny;
int numPointFontSmall;
int numPointFontMed;
int numPointFontLarge;
int numThumbWidth     = 15;
int numResults        = 10;
int numFormat         = 0;
int numStart          = 1;
int noRectPenSize;
enum tSoundOutput soundOutput = soHDMI;
enum tVideoPlayer videoPlayer = vpOMXPlayer;
enum tJpegDecoder jpegDecoder = jdOMX;
#define ERROR_POINT  (numPointFontMed)

tColorDef colorScheme[] =
{
    {5.0f,  5.0f,  5.0f,  1.0f},
    {0.0f,  0.0f,  5.0f,  1.0f},
    {0.0f,  0.0f,  0.0f,  1.0f},
    {0.0f,  0.0f,  3.0f,  1.0f},
    {2.0f,  5.0f,  2.0f,  1.0f},
    {2.0f,  1.5f,  0.0f,  1.0f},
    {0.0f,  0.0f,  0.0f,  1.0f},
    {4.0f,  0.0f,  0.0f,  1.0f}
};

tColorDef * textColor        = &colorScheme[0];
tColorDef * rectColor        = &colorScheme[1];
tColorDef * rectColor2       = &colorScheme[2];
tColorDef * outlineColor     = &colorScheme[3];
tColorDef * outlineColor2    = &colorScheme[4];
tColorDef * selectedColor    = &colorScheme[5];
tColorDef * bgColor          = &colorScheme[6];
tColorDef * errorColor       = &colorScheme[7];

extern unsigned char *download_file(char * host, char * fileName, unsigned int * fileSize);
extern unsigned char *find_jpg_start(unsigned char * buf, unsigned int * bufSize);
extern VGImage OMXCreateImageFromBuf(unsigned char * buf, unsigned int bufLength, unsigned int outputWidth, unsigned int outputHeight);
extern tMenuState regionMenu;
extern tMenuState fontMenu;
extern tMenuState mainMenu;
extern tMenuState titleFontMenu;
extern tMenuItem videoMenuItems[];
extern tMenuItem jpegMenuItems[];
extern tMenuItem audioMenuItems[];
extern tMenuItem fontMenuItems[]; 
extern const char tv_jpeg_raw_data[];
extern const unsigned int tv_jpeg_raw_size;
//------------------------------------------------------------------------------
void setBGImage()
{
    if(bgImage != 0)
        vgDestroyImage(bgImage);
    bgImage = createImageFromScreen();
}
//------------------------------------------------------------------------------
void drawBGImage()
{
    if(bgImage != 0)
        vgSetPixels(0,0, bgImage, 0, 0, state->screen_width, state->screen_height);
}
//------------------------------------------------------------------------------
struct result_rec * init_result_rec()
{
    struct result_rec * new_rec = malloc(sizeof(struct result_rec));
    if (new_rec != NULL)
    {
        new_rec->image       = 0;
        new_rec->largeImage  = 0;
        new_rec->id          = NULL;
        new_rec->title       = NULL;
        new_rec->date        = NULL;
        new_rec->category    = NULL;
        new_rec->user 	     = NULL;
        new_rec->description = NULL;
        new_rec->url         = NULL;
        new_rec->thumbSmall  = NULL;
        new_rec->thumbLarge  = NULL;
        new_rec->next        = (struct result_rec *) NULL;
        new_rec->prev        = (struct result_rec *) NULL;
    }
    return new_rec;
}
//------------------------------------------------------------------------------
void free_result_rec(struct result_rec * rec)
{

    if(rec != NULL)
    {
        if(rec->id != NULL)
            free(rec->id);
        if(rec->title != NULL)
            free(rec->title);
        if(rec->date != NULL)
            free(rec->date);
        if(rec->category != NULL)
            free(rec->category);
        if(rec->user != NULL)
            free(rec->user);
        if(rec->description != NULL)
            free(rec->description);
        if(rec->url != NULL)
            free(rec->url);
        if(rec->thumbSmall != NULL)
            free(rec->thumbSmall);
        if(rec->thumbLarge != NULL)
            free(rec->thumbLarge);
        if(rec->image > 0)
            vgDestroyImage(rec->image);
        if(rec->image > 0)
            vgDestroyImage(rec->largeImage);
        free(rec);
    }
}
//------------------------------------------------------------------------------
char ** get_lastrec_column(int iBracket, int iBrace, char * key)
{
    if(last_rec != NULL)
    {
        if (iBracket == 3 && strcmp(key, "id") == 0)
            return &last_rec->id;
        else if (iBracket == 3 && strcmp(key, "title") == 0)
            return &last_rec->title;
        else if (iBracket == 3 && strcmp(key, "category") == 0)
            return &last_rec->category;
        else if (iBracket == 3 && strcmp(key, "uploader") == 0)
            return &last_rec->user;
        else if (iBracket == 3 && strcmp(key, "uploaded") == 0)
            return &last_rec->date;
        else if (iBracket == 3 && strcmp(key, "description") == 0)
            return &last_rec->description;
        else if (iBracket == 4 && strcmp(key, "sqDefault") == 0)
            return &last_rec->thumbSmall;
        else if (iBracket == 4 && strcmp(key, "hqDefault") == 0)
            return &last_rec->thumbLarge;
        else if (iBracket == 4 && strcmp(key, "default") == 0)
            return &last_rec->url;
        else
            return NULL;
    }
    else
        return NULL;
}
//------------------------------------------------------------------------------
VGImage create_image_from_buf(unsigned char *buf, unsigned int bufSize, int desired_width, int desired_height)
{
  
    switch (jpegDecoder)
    {
            case jdOMX:
                return OMXCreateImageFromBuf((unsigned char *)
                        buf, bufSize, desired_width, desired_height);
                break;

            case jdLibJpeg:
                return createImageFromBuf((unsigned char *)
                        buf, bufSize, desired_width, desired_height);
                break;
            default:
                show_message("ERROR:\n\nbad jped decoder enum", true, ERROR_POINT);
                break;
    }
    return 0;
}
//------------------------------------------------------------------------------
void textXY(VGfloat x, VGfloat y, const char* s, int pointsize, tColorDef * fillcolor)
{
    Text(&fontDefs[fontMenu.selectedItem], x, y, s, pointsize, fillcolor, VG_FILL_PATH);
}
//------------------------------------------------------------------------------
void textXY_Rollover (VGfloat x, VGfloat y,VGfloat maxLength, int maxLines, VGfloat yStep, const char* s, int pointsize, tColorDef * fillcolor)
{
    Text_Rollover(&fontDefs[fontMenu.selectedItem], x, y, maxLength, maxLines, yStep, s, pointsize, fillcolor, VG_FILL_PATH, false);
}
//------------------------------------------------------------------------------
void free_ui_var()
{
    if (tvImage > 0)
        vgDestroyImage(tvImage);
        
    if (bgImage > 0)
        vgDestroyImage(bgImage);    
    
    if(upArrowImage > 0)
        vgDestroyImage(upArrowImage);
    
    if(downArrowImage > 0)
        vgDestroyImage(downArrowImage);
        
    int i;
    for(i=0; i < fontCount; i++)
        unload_font(&fontDefs[i]);
    if(fontMenu.menuItems!= NULL)
        free(fontMenu.menuItems);       
}
//------------------------------------------------------------------------------
void set_menu_value(tMenuState * menu, int value)
{    
    if(value > menu->maxItems)
    {
        menu->selectedIndex = value % menu->maxItems;
        menu->scrollIndex = value / menu->maxItems;
    }
    else
    {
        menu->selectedIndex = value;
        menu->scrollIndex = 0;
    }
    menu->selectedItem = value;
}
//------------------------------------------------------------------------------
void set_font(int font)
{
    set_menu_value(&fontMenu, font);
}
//------------------------------------------------------------------------------
void set_title_font(int font)
{
     set_menu_value(&titleFontMenu, font);
}
//------------------------------------------------------------------------------
int get_font()
{
     return fontMenu.selectedIndex;
}
//------------------------------------------------------------------------------
int get_title_font()
{
     return titleFontMenu.selectedItem;
}

//------------------------------------------------------------------------------
void init_font_menus()
{
    init_small_menu(&fontMenu, "Font menu:");
    init_small_menu(&titleFontMenu, "Title Font menu:");
    
    int i;   
    fontMenu.menuItems = malloc(sizeof(tMenuItem) * (fontCount + 1));
    for (i = 0; i < fontCount; i++)
    {
         load_font(&fontDefs[i]);
         fontMenu.menuItems[i].key = fontDefs[i].name;
         fontMenu.menuItems[i].description = fontDefs[i].name;
    }
    fontMenu.menuItems[i].key = NULL;
    fontMenu.menuItems[i].description = NULL;
    fontMenu.drawDetail = &font_menu_detail;
    titleFontMenu.menuItems    = fontMenu.menuItems;
    titleFontMenu.drawDetail   = fontMenu.drawDetail;    
}
    
//------------------------------------------------------------------------------
void init_ui_var()
{   
    int w, h;
    if(tvImage == 0)
    {
        w  = (state->screen_width  * .35f);
        h  = (state->screen_height * .45f);
           tvImage = create_image_from_buf((unsigned char *)
           tv_jpeg_raw_data, tv_jpeg_raw_size, w, h);
    }   
    
    if(upArrowImage == 0)
    {
        w  = (state->screen_width  * .05f);
        h  = (state->screen_height * .07f);
        
        upArrowImage = create_image_from_buf((unsigned char *)
            menu_arrow_up_raw_data,  menu_arrow_up_raw_size, w, h);
            
        downArrowImage = create_image_from_buf((unsigned char *)
            menu_arrow_down_raw_data,  menu_arrow_down_raw_size, w, h);
    }   
    
    
    if(state->screen_width >= 1920)
    {
        numPointFontTiny  = 10;
        numPointFontSmall = 12;
        numPointFontMed   = 20;
        numPointFontLarge = 40;
        //  numThumbWidth     = 12;
        //  numResults        = 10;
    }
    else if (state->screen_width >= 1280)
    {
        numPointFontTiny  = 6;
        numPointFontSmall = 8;
        numPointFontMed   = 15;
        numPointFontLarge = 30;
        //   numThumbWidth     = 10;
        //   numResults        = 8;
    }
    else
    {
        numPointFontTiny  = 4;
        numPointFontSmall = 6;
        numPointFontMed   = 13;
        numPointFontLarge = 25;
        //   numThumbWidth     = 10;
        //   numResults        = 8;
    }
    
    noRectPenSize  = state->screen_width  * .005f;
}
//------------------------------------------------------------------------------
//
char * parse_url(char * url, char ** server, char ** page)
{
    const char sStr[] = "//";
    char * buff = malloc(strlen(url) + 1);
    strcpy(buff, url);
    *page = 0x00;
    *server = buff;
    char * temp = strstr(buff, sStr);
    if(temp != NULL)
        *server = temp + strlen(sStr);
    temp = strstr(*server, "/");
    if(temp != NULL)
        *page = temp + 1;
    *temp = 0x00;
    return buff;
}
//------------------------------------------------------------------------------
void draw_menu(tMenuState * menu)
{
    
    Roundrect(menu->bCenterX?(state->screen_width - menu->winRect.w) / 2:menu->winRect.x,
              menu->winRect.y,
              menu->winRect.w,
              menu->winRect.h, 
              30, 20, noRectPenSize, rectColor, selectedColor);

    Roundrect(menu->selRect.x,
              menu->selRect.y,
              menu->selRect.w,
              menu->selRect.h,
              20, 20, noRectPenSize / 2, rectColor, selectedColor);
              
    Text(&fontDefs[titleFontMenu.selectedItem],
        menu->titlePos.x, menu->titlePos.y, 
        menu->title, menu->numPointFontTitle, selectedColor, VG_FILL_PATH);        
}
//------------------------------------------------------------------------------
void draw_txt_box_cen(char * message, float widthP, float heightP, float boxYp, float tXp, float tYp, int points)
{
    int width  = state->screen_width * widthP;
    int height = state->screen_height * heightP;
    int x = (state->screen_width  - width) / 2;
    int y = state->screen_height * boxYp;
    int tx = state->screen_width * tXp;
    int ty = state->screen_height * tYp;
    Roundrect(x,y, width, height, 20, 20, noRectPenSize, rectColor, selectedColor);
    Text(&fontDefs[titleFontMenu.selectedItem],tx, ty, message, points, selectedColor, VG_FILL_PATH);
}
//------------------------------------------------------------------------------
void clear_screen(bool swap)
{
    glClear( GL_COLOR_BUFFER_BIT );
    vgSetfv(VG_CLEAR_COLOR, 4, (VGfloat *) bgColor);
    vgClear(0, 0, state->screen_width, state->screen_height);
    vgLoadIdentity();
    if(swap)
        eglSwapBuffers(state->display, state->surface);
}
//------------------------------------------------------------------------------
int show_selection_info(struct result_rec * rec)
{
    int key = 0x00;
    
    int offsetY      = (state->screen_height * .060f);
    int offsetX      = (state->screen_width  * .035f);
    int tv_width     = vgGetParameteri(tvImage, VG_IMAGE_WIDTH);
    int tv_height    = vgGetParameteri(tvImage, VG_IMAGE_HEIGHT);
    int tvX          = (state->screen_width  - tv_width) / 2;
    int tvY          = (state->screen_height - tv_height);
    int image_height = tv_height - (offsetY * 2);
    int image_width  = tv_width  - (offsetX * 2);
    int imageX       = (state->screen_width - image_width) / 2;
    int imageY       = (state->screen_height - image_height) - (tv_height - image_height) / 2;
    unsigned char * downloadData = NULL;
    unsigned char * imageData = NULL;
    unsigned int imageDataSize;
    
    if(rec->description)
    {
        redraw_results(false);
        show_big_message("Info: loading...", rec->description);    
        vgSetPixels(tvX,
                    tvY,
                    tvImage,
                    0, 0,
                    tv_width,
                    tv_height);
       
        eglSwapBuffers(state->display, state->surface);
    }
    else //description not found.
        show_message("OOPS! description == NULL", true, ERROR_POINT);

    if(rec->thumbLarge != NULL)
    {
        if (rec->largeImage == 0)
            rec->largeImage = load_jpeg2(rec->thumbLarge, image_width, image_height, 
                &downloadData, &imageData, &imageDataSize);

        char * infoStr = NULL;

        if(rec->date != NULL && rec->user != NULL && rec->id != NULL)
        {
            char * T = strchr(rec->date, 'T'); //remove after T - time unwanted
            if (T != NULL) *T= 0x00;
            char formatStr[] = "Info: #%s : %s : %s";
            size_t size = strlen(rec->id)   +
                       strlen(rec->date) +
                       strlen(rec->user) +
                       strlen(formatStr);
            infoStr = malloc(size);
            snprintf(infoStr, size, formatStr, rec->id, rec->user, rec->date);
            if (T != NULL) *T= 'T';
        }
        else
        {
            if(rec->description)
                show_big_message("Info: ???", rec->description);
        }

        do
        {
            redraw_results(false);
            if(infoStr != NULL && rec->description != NULL)
                show_big_message(infoStr, rec->description);

            vgSetPixels(tvX,
                        tvY,
                        tvImage,
                        0, 0,
                        tv_width,
                        tv_height);
                   
            vgSetPixels(imageX,
                        imageY,
                        rec->largeImage,
                        0, 0,
                        image_width,
                        image_height);

            eglSwapBuffers(state->display, state->surface);

            key = toupper(readKb());
            if (key == 'H' && imageData != NULL)
            {
                vgDestroyImage(-rec->largeImage);
                     rec->largeImage = OMXCreateImageFromBuf((unsigned char *)
                        imageData, imageDataSize, image_width, image_height);
            }
            else
            if (key == 'S' && imageData != NULL)
            {
                vgDestroyImage(rec->largeImage);
                     rec->largeImage = createImageFromBuf((unsigned char *)
                        imageData, imageDataSize, image_width, image_height);
            }
            else if (key== CUR_L || key == CUR_R ||
                     key == CUR_UP || key == CUR_DWN)
                break;
        }
        while ( key != ESC_KEY &&
                key != RTN_KEY &&
                key != CUR_L &&
                key != CUR_R &&
                key != CUR_UP &&
                key != CUR_DWN);
        if(infoStr != NULL) free(infoStr);
        //vgDestroyImage(image);
        if(downloadData != NULL)
            free(downloadData);
        redraw_results(true);
    }
    else
        show_message("OOPS! rec->thumbLarge == NULL", true, ERROR_POINT);
    return key;
}
//------------------------------------------------------------------------------
bool input_string(char * prompt, char * buf, int max)
{
    int key = 0x00;
    int endPos;
    int lastkeys[7] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    char formatStr[] = "%02X->%02X:%02X:%02X:%02X:%02X:%02X:%02X";
    char validChars[] =
        "!@#$%^&*(()_<>?+=1234567890,.ABCDEFGHIJKLMNOPQRSTUVWXYZ ";
    char temp[strlen(formatStr) + 1];
    char cstr[2];
    cstr[1]=0x00;
    char * save = malloc(max+1);
    strcpy(save, buf);
    do
    {
        drawBGImage();
        draw_txt_box_cen(prompt, .95f, .50f, .05, .10f, .50f, numPointFontLarge);
        snprintf(temp, sizeof(formatStr),formatStr, key,
                 lastkeys[0], lastkeys[1], lastkeys[2], lastkeys[3], lastkeys[4], lastkeys[5], lastkeys[6]);
        textXY(state->screen_width * .10f, state->screen_height * .10f, temp, numPointFontMed, textColor);
        endPos = strlen(buf);
        buf[endPos] = '_';
        buf[endPos+1]= 0x00;
        textXY(state->screen_width * .10f, state->screen_height * .30f, buf, numPointFontLarge, textColor);
        buf[endPos] = 0x00;
        eglSwapBuffers(state->display, state->surface);
        int i = 0;

        for (i = (sizeof(lastkeys) / sizeof(int))-1; i > 0; i--)
            lastkeys[i] = lastkeys[i-1];

        lastkeys[0] = key;
        key = readKb();
        switch (key)
        {
        case DEL_KEY:

            if(endPos > 0)
                buf[endPos-1] = 0x00;
            break;

        case ESC_KEY:
            strcpy(buf, save);
            break;

        case CUR_L:
            buf[0] = 0x00;
            break;

        case CUR_R:
            strcpy(buf, save);
            break;

        case RTN_KEY:
            break;

        default:
            if(strchr(validChars, toupper(key)) != NULL &&  strlen(buf) < max - 3)
            {
                cstr[0] = (char) key;
                strncat(buf, cstr,  max);
            }
            break;
        }
    }
    while (key != RTN_KEY && key != ESC_KEY);
    free(save);
    dumpKb();
    if(key == ESC_KEY)
        return false;
    else
        return true;
}
//------------------------------------------------------------------------------
void show_big_message(char * title, char * message)
{
    redraw_results(false);
    draw_txt_box_cen(title, .95f, .47f, .04, .10f, .45f, numPointFontLarge);
    textXY_Rollover(state->screen_width  * .10f,
                    state->screen_height * .40f,
                    state->screen_width  * .85f,
                    7, //max no of lines
                    state->screen_height * .05f,
                    message, numPointFontMed, textColor);
}
//------------------------------------------------------------------------------
void show_message(char * message, int error, int points)
{
    int offsetY      = (state->screen_height * .060f);
    int offsetX      = (state->screen_width  * .035f);
    int tv_width     = vgGetParameteri(tvImage, VG_IMAGE_WIDTH);
    int tv_height    = vgGetParameteri(tvImage, VG_IMAGE_HEIGHT);
    int tvX          = (state->screen_width  - tv_width) / 2;
    int tvY          = (state->screen_height - tv_height) / 2;
    int image_height = tv_height - (offsetY * 2);
    int image_width  = tv_width  - (offsetX * 2);
    int imageX       = (state->screen_width - image_width)   / 2;
    int imageY       = (state->screen_height - image_height) / 2;
    int tx 	     = imageX  + state->screen_width  * .03f;
    int ty 	     = imageY  + image_height - offsetX;
    int guru_width   = image_width  * .95f;
    int guru_height  = image_height * .25f;
    int guruX        = (state->screen_width - guru_width)   / 2;
    int guruY        = imageY  + image_height - guru_height -  (state->screen_width  * .007f);

    char * errorStr = NULL; 
    int key = ESC_KEY;
    if(error)
    {
         char formatStr[] = "~7GURU MEDITATION #%08X\n\n~0%s";
         size_t sErrorStr = strlen(formatStr) + strlen(message) + 8;
         errorStr = malloc(sErrorStr);
         snprintf(errorStr, sErrorStr, formatStr, error, message);
         printf("ERROR->%d\n", error);
    }
           
    do
    {
        if(error)
            redraw_results(false);
           
        vgSetPixels(tvX,
                tvY,
                tvImage,
                0, 0,
                tv_width,
                tv_height);
       
        if(error)
        {
            Roundrect(imageX, imageY,  image_width, image_height, 20, 20, noRectPenSize, bgColor, bgColor);
            Rect(guruX, guruY, guru_width, guru_height, noRectPenSize, bgColor, errorColor);
            Text_Rollover ( &fontDefs[1], //Topaz font
                        tx, // X
                        ty, // Y
                        state->screen_width * .80f,
                        6,
                        state->screen_height * .05f,
                        errorStr, points, &colorScheme[0], VG_FILL_PATH, true);
        }
        else
        {
            Roundrect(imageX,  imageY, image_width, image_height, 20, 20, noRectPenSize, rectColor, bgColor);
            Text_Rollover ( &fontDefs[1], //Topaz font
                        tx, // X
                        ty, // Y
                        state->screen_width * .80f,
                        8,
                        state->screen_height * .05f,
                        message, points, &colorScheme[0], VG_FILL_PATH, true);
        }
                       
        eglSwapBuffers(state->display,
                       state->surface);
        if(error)
        {
            dumpKb();
            key = readKb();
            dumpKb();
            redraw_results(true);
        }
        else
            break;

    }
    while (key != ESC_KEY && key != RTN_KEY);
    if(errorStr != NULL)
        free(errorStr);
}
//------------------------------------------------------------------------------
void calc_rect_bounds(tRectPer * rectPer, tRectBounds * rectBounds)
{
    rectBounds->x = rectPer->xPer * state->screen_width;
    rectBounds->y = rectPer->yPer * state->screen_height;
    rectBounds->w = rectPer->wPer * state->screen_width;
    rectBounds->h = rectPer->hPer * state->screen_height;
}
//------------------------------------------------------------------------------
void calc_point_xy(tPointPer * pointPer, tPointXY * pointXY)
{
    pointXY->x = pointPer->xPer * state->screen_width;
    pointXY->y = pointPer->yPer * state->screen_height;
}

//------------------------------------------------------------------------------
void init_arrow(VGfloat * xy, tRectPer * rectPer, bool bFlip)
{
    tPointXY p1;
    tPointXY p2;
    tPointXY p3;
    int x1 = state->screen_width  * rectPer->xPer;
    int x2 = state->screen_width  * (rectPer->xPer + rectPer->wPer);
    int x3 = x1 + ((x2 - x1) / 2);
    int y1 = state->screen_height * rectPer->yPer;
    int y2 = state->screen_height * rectPer->yPer;
    int yOffset = rectPer->hPer * state->screen_height;

    if(bFlip)
        y1 += yOffset;
    else
        y2 += yOffset;

    p1.x = x3;
    p1.y = y1;
    p2.x = x1;
    p2.y = y2;
    p3.x = x2;
    p3.y = y2;
    xy[0] = p1.x;
    xy[1] = p1.y;
    xy[2] = p2.x;
    xy[3] = p2.y;
    xy[4] = p3.x;
    xy[5] = p3.y;
    xy[6] = p1.x;
    xy[7] = p1.y;
}
//------------------------------------------------------------------------------
void init_big_menu(tMenuState * menu, char * title)
{
    menu->title = title;
    menu->titlePer.xPer = .10f;
    menu->titlePer.yPer = .87f;
    calc_point_xy(&menu->titlePer, &menu->titlePos);
    menu->selectedIndex = 0;
    menu->scrollIndex 	= 0;
    menu->maxItems = 18;
    menu->txtOffset.x = state->screen_height * .20f;
    menu->txtOffset.y = state->screen_width  * .10f;
    menu->winPer.xPer = .10f;
    menu->winPer.yPer = .05f;
    menu->winPer.wPer = .92f;
    menu->winPer.hPer = .90f;
    calc_rect_bounds(&menu->winPer, &menu->winRect);
    menu->numPointFontTitle = numPointFontLarge;
    menu->numPointFont = numPointFontMed;
    menu->selPer.xPer =  .085f;
    menu->yStep = state->screen_height * .04f;
    menu->selPer.wPer = .50f;
    menu->selPer.hPer = .04f;
    calc_rect_bounds(&menu->selPer, &menu->selRect);
/*
    tRectPer rectPer;
    rectPer.xPer = .88f;
    rectPer.yPer = .88f;
    rectPer.wPer = .04f;
    rectPer.hPer = .04f;
    init_arrow(menu->upArrow, &rectPer, true);
    rectPer.yPer = .08f;
    init_arrow(menu->downArrow, &rectPer, false);    
*/
    menu->upArrowPer.xPer = .88f;
    menu->upArrowPer.yPer = .83f;
    calc_point_xy(&menu->upArrowPer, &menu->upArrowPos);
    menu->downArrowPer.xPer = .88f;
    menu->downArrowPer.yPer = .10f;
    calc_point_xy(&menu->downArrowPer, &menu->downArrowPos); 
    menu->drawHeader = NULL;
    menu->drawDetail = NULL;
    menu->drawFooter = NULL;
    menu->keyPress = NULL;
    menu->bCenterX = true;
    menu->bCenterY = false;
}
//------------------------------------------------------------------------------
void init_small_menu(tMenuState * menu, char * title)
{
    menu->title = title;
    menu->titlePer.xPer = .10f;
    menu->titlePer.yPer = .87f;
    calc_point_xy(&menu->titlePer, &menu->titlePos);
    menu->selectedIndex = 0;
    menu->scrollIndex 	= 0;
    menu->maxItems = 8;
    menu->txtOffset.x = state->screen_height * .20f;
    menu->txtOffset.y = state->screen_width  * .10f;
    menu->winPer.xPer = .05f;
    menu->winPer.yPer = .47f;
    menu->winPer.wPer = .50f;
    menu->winPer.hPer = .50f;
    calc_rect_bounds(&menu->winPer, &menu->winRect);
    menu->numPointFontTitle = numPointFontLarge;
    menu->numPointFont = numPointFontMed;
    menu->selPer.xPer =  .085f;
    menu->yStep = state->screen_height * .04f;
    menu->selPer.wPer = .35f;
    menu->selPer.hPer = .04f;
    calc_rect_bounds(&menu->selPer, &menu->selRect);
/*
    tRectPer rectPer;
    rectPer.xPer = .45f;
    rectPer.yPer = .88f;
    rectPer.wPer = .04f;
    rectPer.hPer = .04f;
    init_arrow(menu->upArrow, &rectPer, true);
    rectPer.yPer = .53f;
    init_arrow(menu->downArrow, &rectPer, false);
*/
    menu->upArrowPer.xPer = .45f;
    menu->upArrowPer.yPer = .83f;
    calc_point_xy(&menu->upArrowPer, &menu->upArrowPos);
    menu->downArrowPer.xPer = .45f;
    menu->downArrowPer.yPer = .53f;
    calc_point_xy(&menu->downArrowPer, &menu->downArrowPos);
    
    menu->drawHeader = NULL;
    menu->drawDetail = NULL;
    menu->drawFooter = NULL;
    menu->keyPress = NULL;
    menu->bCenterX = false;
    menu->bCenterY = false;
}
//------------------------------------------------------------------------------
void format_menu_header(tMenuState * menu)
{
    int x;
    for (x = 0; x < AFORMAT_WIDTH; x++)
        textXY((x+1) * (state->screen_width /  (AFORMAT_WIDTH  + 2)),
                        menu->txtRaster.y + menu->yStep,
                        supported_formats[0][x], (x>=3)?numPointFontSmall:numPointFontMed, errorColor);
}
//------------------------------------------------------------------------------
void format_menu_detail(tMenuState * menu)
{
    int x;
    for (x = 1; x < AFORMAT_WIDTH; x++)
        textXY((x+1) * (state->screen_width /  (AFORMAT_WIDTH  + 2)),
               menu->txtRaster.y,
               supported_formats[menu->selectedItem + 1][x],(x==3)?numPointFontSmall:numPointFontMed, textColor);
}
//------------------------------------------------------------------------------
void gui_menu_detail(tMenuState * menu)
{
    if(menu->menuItems[menu->selectedItem].special > 0)
    {
        char * descr = NULL;
        switch(menu->menuItems[menu->selectedItem].special)
        {
        case 1:
            descr = videoMenuItems[(int) videoPlayer].description;
            break;
            
        case 2:
            descr = audioMenuItems[(int) soundOutput].description;
            break;
            
        case 3:
            descr = jpegMenuItems[(int) jpegDecoder].description;
            break;            

        case 4:
            descr = fontMenu.menuItems[(int) get_font()].description;
            break;            

        case 5:
            descr = titleFontMenu.menuItems[(int) get_title_font()].description;
            break; 
        }
        
        if(descr != NULL)
            textXY(state->screen_width * .25,
                 menu->txtRaster.y,
                 descr,
                 numPointFontMed, errorColor);
    }       
}
//------------------------------------------------------------------------------
void main_menu_detail(tMenuState * menu)
{
    char * videoFormat = NULL;
    char * resolution;
    char * container;
    char * number;
    char formatStr[] = "%s / %s / %s";

    if(menu->menuItems[menu->selectedItem].special > 0)
    {
        char * descr = NULL;
        switch(menu->menuItems[menu->selectedItem].special)
        {

        case 1:
            number     = supported_formats[numFormat + 1][0];
            container  = supported_formats[numFormat + 1][1];
            resolution = supported_formats[numFormat + 1][2];
            size_t size = strlen(container)  +
                          strlen(resolution) +
                          strlen(number) +
                          strlen(formatStr) - 5;
            videoFormat = malloc(size);
            snprintf(videoFormat, size, formatStr, number, container, resolution);
            descr = videoFormat;
            break;;

        case 2:
            descr = regionMenu.menuItems[regionMenu.selectedItem].description;
            break;
       
        case 3:
            descr = regionMenu.menuItems[regionMenu.selectedItem].key;
            break;
       
        }

        if(descr != NULL)
            textXY(state->screen_width * .25,
                 menu->txtRaster.y,
                 descr,
                 numPointFontMed, errorColor);
        if(videoFormat != NULL)
            free(videoFormat);
    }
}
//------------------------------------------------------------------------------
void font_menu_detail(tMenuState * menu)
{
     Text(&fontDefs[menu->selectedItem], state->screen_width * .25,
             menu->txtRaster.y,
             "abcdefgABCDEFG01234...",
             numPointFontMed, errorColor, VG_FILL_PATH);
}
//------------------------------------------------------------------------------
void init_format_menu(tMenuState * menu)
{
    init_big_menu(menu, "Select format:");
    menu->drawHeader = &format_menu_header;
    menu->drawDetail = &format_menu_detail;
    menu->menuItems = NULL;
    menu->txtOffset.y = state->screen_width  * .12f;
    menu->selPer.wPer = .76f;
    calc_rect_bounds(&menu->selPer, &menu->selRect);
}

//------------------------------------------------------------------------------
int show_format_menu(tMenuState * menu)
{
    if(menu->menuItems == NULL)
    {
        menu->menuItems = malloc(sizeof(tMenuItem) * (AFORMAT_HEIGHT + 1));
        int i;
        for (i = 1; i < AFORMAT_HEIGHT; i++)
        {
            menu->menuItems[i-1].key = supported_formats[i][0];
            menu->menuItems[i-1].description = supported_formats[i][0];
        }
        menu->menuItems[i-1].key = NULL;
        menu->menuItems[i-1].description = NULL;
    }
    int result = show_menu(menu);
    if(result != -1)
        numFormat =  result;
    return result;
}
//------------------------------------------------------------------------------
#define SHOW_MENU_Y_CALC (state->screen_height - (y * menu->yStep) - menu->txtOffset.y)
int show_menu(tMenuState * menu)
{
    int scrollIndexSave   = menu->scrollIndex;
    int selectedIndexSave = menu->selectedIndex;
    int key;
    do
    {
        drawBGImage();

        menu->selRect.y = state->screen_height -
                          (menu->selectedIndex * menu->yStep) - (menu->yStep / 3.0f) - menu->txtOffset.y;
        int y = 0;
        int count = 0;
        menu->selectedItem = menu->selectedIndex + menu->scrollIndex;
        draw_menu(menu);
        menu->txtRaster.x = menu->txtOffset.x;
        menu->txtRaster.y = SHOW_MENU_Y_CALC;
        if (menu->drawHeader != NULL)
            menu->drawHeader(menu);

        tMenuItem * currentItem = menu->menuItems;
        while(currentItem->key != NULL && currentItem->description != NULL)
        {
            if(count >= menu->scrollIndex)
            {

                textXY(menu->txtRaster.x,
                     menu->txtRaster.y,
                     currentItem->description,
                     numPointFontMed,
                     textColor);
                
                menu->selectedItem = y + menu->scrollIndex;
                
                if (menu->drawDetail != NULL)
                    menu->drawDetail(menu);
                y++;

                menu->txtRaster.x = menu->txtOffset.x;
                menu->txtRaster.y = SHOW_MENU_Y_CALC;

                if (y == menu->maxItems)
                    break;
            }
            currentItem++;
            count++;
        }

        menu->selectedItem = menu->selectedIndex + menu->scrollIndex;

        bool bMoreItems = false;
        if (currentItem->key != NULL || currentItem->description != NULL)
        {
            currentItem++;
            if(currentItem->key != NULL || currentItem->description != NULL)
                bMoreItems = true;
        }

        if (bMoreItems)
        {
            vgSetPixels(menu->downArrowPos.x, menu->downArrowPos.y, 
                        downArrowImage, 0,0,
                        vgGetParameteri(downArrowImage, VG_IMAGE_WIDTH),
                        vgGetParameteri(downArrowImage, VG_IMAGE_HEIGHT));
            //Poly(menu->downArrow, 4, noRectPenSize / 2, selectedColor, bgColor, VG_TRUE);
        }

        if (menu->scrollIndex > 0)
        {
             vgSetPixels(menu->upArrowPos.x, menu->upArrowPos.y, 
                        upArrowImage, 0,0,
                        vgGetParameteri(upArrowImage, VG_IMAGE_WIDTH),
                        vgGetParameteri(upArrowImage, VG_IMAGE_HEIGHT));
            //Poly(menu->upArrow, 4, noRectPenSize / 2, selectedColor, bgColor, VG_TRUE);
        }

        if (menu->drawFooter != NULL)
            menu->drawFooter(menu);
        eglSwapBuffers(state->display, state->surface);


        key = toupper(readKb());
        switch (key)
        {

        case CUR_UP:
            if (menu->selectedIndex > 0)
                menu->selectedIndex--;
            else if(menu->scrollIndex > 0)
                menu->scrollIndex--;
            break;

        case CUR_DWN:
            if (menu->selectedIndex < menu->maxItems -1)
            {
                currentItem = &menu->menuItems[menu->selectedIndex + menu->scrollIndex + 1];
                if(currentItem->key != NULL || currentItem->description != NULL)
                    menu->selectedIndex++;
            }
            else if(bMoreItems)
                menu->scrollIndex++;

            break;;
        }
    }
    while (key != 'Q' && key != RTN_KEY && key != ESC_KEY);
    if(key == 'Q' || key == ESC_KEY)
    {
        //restore previous value
        menu->scrollIndex = scrollIndexSave;
        menu->selectedIndex = selectedIndexSave;
        return -1;
    }
    dumpKb();
    return menu->selectedItem;
}
//------------------------------------------------------------------------------
int handleESC()
{
    int key = getchar();
    if (key == EOF)
    {
        return ESC_KEY;
    }
    else
    {
        switch(key)
        {
        case '[':
            if(key != EOF)
            {
                key = getchar();
                switch(key) //cursor movement
                {
                case TERM_CUR_R:
                    return CUR_R;
                    break;

                case TERM_CUR_L:
                    return CUR_L;
                    break;

                case TERM_CUR_UP:
                    return CUR_UP;
                    break;

                case TERM_CUR_DWN:
                    return CUR_DWN;
                    break;
                }
            }
            break;
        case 'O' :
            if(key != EOF) //function keys
            {
                key = getchar();
                switch(key)
                {
                case TERM_FUN_1:
                    DoSnapshot();
                    show_message("Snapshot Saved!", true, ERROR_POINT);
                    return FUN_1;
                    break;
                case TERM_FUN_2:
                    return FUN_2;
                default:
                    while(getchar() != EOF);
                }
            }
            break;
        }
    }
    return 0;
}
//------------------------------------------------------------------------------
bool kbHit(void)
{
    int ch;
    fcntl(STDIN_FILENO, F_SETFL, ttflags | O_NONBLOCK);
    ch = getchar();
    fcntl(STDIN_FILENO, F_SETFL, ttflags & ~O_NONBLOCK);
    if(ch != EOF)
    {
        ungetc(ch, stdin);
        return true;
    }
    return false;
}
//------------------------------------------------------------------------------
int readKb()
{
    int key = getchar();
    if (key == ESC_KEY)
    {
        fcntl(STDIN_FILENO, F_SETFL, ttflags | O_NONBLOCK);
        key = handleESC();
        fcntl(STDIN_FILENO, F_SETFL, ttflags & ~O_NONBLOCK);
    }
    //else play_sample(&asiKbClick, false);
    return key;
}
//------------------------------------------------------------------------------
void dumpKb()
{
    fcntl(STDIN_FILENO, F_SETFL, ttflags | O_NONBLOCK);
    while (getchar()!= EOF)
    {
        // :)
    }
    fcntl(STDIN_FILENO, F_SETFL, ttflags & ~O_NONBLOCK);
}
//------------------------------------------------------------------------------
void initKb()
{
    tcgetattr(STDIN_FILENO, &oldt); 		// store old settings
    newt = oldt; 				// copy old settings to new settings
    newt.c_lflag &= ~(ICANON | ECHO); 		// change settings
    tcsetattr(STDIN_FILENO, TCSANOW, &newt); 	// apply the new settings immediatly
    ttflags = fcntl(STDIN_FILENO, F_GETFL, 0);
//  load_sample(&asiKbClick, (uint8_t *) soundraw_data, soundraw_size, 8000, 16, 1, 1);
}
//------------------------------------------------------------------------------
void restoreKb()
{
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt); 	// reapply the old settings
//  delete_sample(&asiKbClick);
}
//------------------------------------------------------------------------------
void clear_output()
{
    struct result_rec * temp_rec = first_rec;
    struct result_rec * next_rec;
    while (temp_rec != NULL)
    {
        next_rec = temp_rec->next;
        free_result_rec(temp_rec);
        temp_rec = next_rec;
    }

    first_rec    = NULL;
    last_rec     = NULL;
    selected_rec = NULL;
}
//------------------------------------------------------------------------------
void replace_char_str(char * buf,  char oldChar, char newChar)
{
    int i;
    for(i = 0; i < strlen(buf); i++)
        if(buf[i] == oldChar)
            buf[i] = newChar;
}
//------------------------------------------------------------------------------
VGImage load_jpeg(char * url, unsigned int width, unsigned int height)
{

    VGImage vgImage = 0;
    unsigned int fileSize = 0;
    unsigned char * downloadData;
    char * server = NULL;
    char * page = NULL;
    char * freeMe = parse_url(url, &server, &page);
    if(server != NULL && page != NULL)
    {
        downloadData = download_file(server, page, &fileSize);
        unsigned char * imageData = find_jpg_start(downloadData, &fileSize);
        if (imageData == NULL)
            show_message("LJ1:unable to find jpeg start 0xFF 0xD8", true, ERROR_POINT);
        else
        {
            vgImage = create_image_from_buf(imageData, fileSize, width, height);
        }
        if(downloadData != NULL) free(downloadData);
    }
    if(freeMe != NULL)free(freeMe);
    return vgImage;
}

//------------------------------------------------------------------------------
VGImage load_jpeg2(char * url, unsigned int width, unsigned int height, 
    unsigned char ** downloadData, unsigned char ** imageData, unsigned int * imageDataSize)
{
    VGImage vgImage  = 0;
    char * server    = NULL;
    char * page      = NULL;
    (*downloadData)  = NULL;
    (*imageData)     = NULL;
    (*imageDataSize) = 0;
    char * freeMe    = parse_url(url, &server, &page);
    if(server != NULL && page != NULL)
    {
        (*downloadData)  = download_file(server, page, imageDataSize);
        (*imageData)     = find_jpg_start(*downloadData, imageDataSize);
        if ((*imageData) == NULL)
            show_message("LJ2:unable to find jpeg start 0xFF 0xD8", true, ERROR_POINT);
        else
        {
            vgImage = create_image_from_buf((*imageData), (*imageDataSize), width, height);
        }
    }
    if(freeMe != NULL)free(freeMe);
    return vgImage;
}

//------------------------------------------------------------------------------
void redraw_results(bool swap)
{
    int step = state->screen_height / numResults;
    int halfStep = step / 2;
    int rectHeight = (int) ((float) step * .9f);
    int rectOffset = (int) ((float) state->screen_width * .05);
    int rectWidth = state->screen_width - (rectOffset * 2);
    int rectWidth2 = state->screen_width / (numThumbWidth - 2);
    int jpegWidth =  state->screen_width / numThumbWidth;
    jpegWidth = (int)((jpegWidth / 16)) * 16;
    int txtXoffset = rectWidth2 + (rectOffset * 1.2);
    int iLine = 0;
    int rectDiff = (step - rectHeight) / 2;
    int jpegOffset = rectOffset + ((rectWidth2 - jpegWidth) / 2);
    int txtYstep  = state->screen_height * .04f;
    int txtXmax   = state->screen_width * .85f;

    clear_screen(false);
    struct result_rec * temp = first_rec;
    while (temp != NULL)
    {
        int y;
        if (selected_rec == NULL)
            selected_rec = temp;

        iLine++;
        y = state->screen_height - (iLine * step);

        if (temp == selected_rec)
        {
            Roundrect(rectOffset, y, rectWidth, rectHeight, 20, 20, noRectPenSize, rectColor, selectedColor);
            if(temp->title != NULL)
                textXY_Rollover(txtXoffset, y + halfStep, txtXmax, 2, txtYstep, temp->title, numPointFontMed, selectedColor);
        }
        else
        {
            Roundrect(rectOffset, y, rectWidth, rectHeight, 20, 20, noRectPenSize, rectColor, outlineColor);
            if(temp->title != NULL)
                textXY_Rollover(txtXoffset, y + halfStep, txtXmax, 2, txtYstep, temp->title, numPointFontMed, textColor);
        }

        Roundrect(rectOffset, y - rectDiff, rectWidth2, step, 20, 20, noRectPenSize / 2, rectColor2, outlineColor2);

        if(temp->thumbSmall != NULL)
        {
            if(temp->image == 0)
                temp->image = load_jpeg(temp->thumbSmall, jpegWidth, rectHeight);
            vgSetPixels(jpegOffset, y, temp->image, 0,0,
                        vgGetParameteri(temp->image, VG_IMAGE_WIDTH),
                        vgGetParameteri(temp->image, VG_IMAGE_HEIGHT));
        }
        temp = temp->next;
    }

    switch(videoPlayer)
    {
    case vpOMXPlayer:
        textXY(0,state->screen_height * .98f, "[OMXPlayer]", numPointFontTiny, textColor);
        break;
    case vpMPlayer:
        textXY(0, state->screen_height * .98f, "[MPlayer]",  numPointFontTiny, textColor);
        break;
    }

    switch(jpegDecoder)
    {
    case jdOMX:
        textXY(0,state->screen_height * .96f, "[OMXJPEG]", numPointFontTiny, textColor);
        break;
    case jdLibJpeg:
        textXY(0, state->screen_height * .96f, "[LIBJPEG]", numPointFontTiny, textColor);
        break;
    }

    switch(soundOutput)
    {
    case soHDMI :
        textXY(0, state->screen_height * .94, "(((HDMI)))",  numPointFontSmall, textColor);
        break;
    case soLOCAL:
        textXY(0, state->screen_height * .94, "(((LOCAL)))", numPointFontSmall, textColor);
        break;
    }

    if (numStart != 1)
    {
        char numStartStr[10];
        snprintf(numStartStr, sizeof(numStartStr), "<-%d", numStart-1);
        textXY(0, state->screen_height * .50f, numStartStr, numPointFontMed, textColor);
    }

    if(swap)
        eglSwapBuffers(state->display, state->surface);
}
