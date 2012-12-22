#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <netdb.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>
#include <time.h>
#include <ctype.h>
#include "gfxlib.h"
#include "videoformats.h"
#include "ui.h"
#include "audio.h"
#include "fonts.inc"
#include "config.h"
#include "menu_arrow_up.inc"
#include "menu_arrow_down.inc"
#include "kbjs.h"
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
int numThumbWidth     = 10;
int numRow	      = 6;
int numCol	      = 2;
int numResults        = 12;
int numFormat         = 0;
int numStart          = 1;
int numFontSpacing    = 24;
int numRectPenSize;

//int numResultsReturned;

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
    {4.0f,  0.0f,  0.0f,  1.0f},
    {0.0f,  0.5f,  0.0f,  1.0f},
};

tColorDef * textColor        = &colorScheme[0];
tColorDef * rectColor        = &colorScheme[1];
tColorDef * rectColor2       = &colorScheme[2];
tColorDef * outlineColor     = &colorScheme[3];
tColorDef * outlineColor2    = &colorScheme[4];
tColorDef * selectedColor    = &colorScheme[5];
tColorDef * bgColor          = &colorScheme[6];
tColorDef * errorColor       = &colorScheme[7];
tColorDef * rectColor3       = &colorScheme[8];

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
        vgGetPixels(bgImage, 0,0, 0,0,state->screen_width, state->screen_height);
    else
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
void textXY_Rollover (VGfloat x, VGfloat y,VGfloat brkLength, VGfloat maxLength, int maxLines, VGfloat yStep, const char* s, int pointsize, tColorDef * fillcolor)
{
    Text_Rollover(&fontDefs[fontMenu.selectedItem], x, y, brkLength, maxLength, maxLines, yStep, s, pointsize, fillcolor, VG_FILL_PATH, false);
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
    set_font(0);
    set_title_font(4);
}

//------------------------------------------------------------------------------
void init_ui_var()
{
    int w, h;
    if(state->screen_width >= 1920)
    {
        numPointFontTiny  = 10;
        numPointFontSmall = 12;
        numPointFontMed   = 18;
        numPointFontLarge = 40;
    }
    else if (state->screen_width >= 1280)
    {
        numPointFontTiny  = 7;
        numPointFontSmall = 9;
        numPointFontMed   = 13;
        numPointFontLarge = 30;
    }
    else
    {
        numPointFontTiny  = 5;
        numPointFontSmall = 6;
        numPointFontMed   = 12;
        numPointFontLarge = 25;
    }
    if(tvImage == 0)
    {
        w  = (state->screen_width  * .35f);
        h  = (state->screen_height * .45f);
        tvImage = create_image_from_buf((unsigned char *)
                                        tv_jpeg_raw_data, tv_jpeg_raw_size, w, h);
    }
    numRectPenSize  = state->screen_width  * .005f;
    loadConfig();
    open_mouse();
    open_joystick();
    if(upArrowImage == 0)
    {
        w  = (state->screen_width  * .05f);
        h  = (state->screen_height * .07f);

        upArrowImage = create_image_from_buf((unsigned char *)
                                             menu_arrow_up_raw_data,  menu_arrow_up_raw_size, w, h);

        downArrowImage = create_image_from_buf((unsigned char *)
                                               menu_arrow_down_raw_data,  menu_arrow_down_raw_size, w, h);
    }
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
              30, 20, numRectPenSize, rectColor, selectedColor);

    Roundrect(menu->selRect.x,
              menu->selRect.y,
              menu->selRect.w,
              menu->selRect.h,
              20, 20, numRectPenSize / 2, rectColor, selectedColor);

    Text(&fontDefs[titleFontMenu.selectedItem],
         menu->titlePos.x, menu->titlePos.y,
         menu->title,
         //menu->numPointFontTitle,
         numPointFontLarge,
         selectedColor, VG_FILL_PATH);
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
    int txtBrk = width + x - numRectPenSize;
    Roundrect(x,y, width, height, 20, 20, numRectPenSize, rectColor, selectedColor);
    Text_Rollover(&fontDefs[titleFontMenu.selectedItem],tx, ty, txtBrk, txtBrk, 1, 0, message, points, selectedColor, VG_FILL_PATH, false);
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

    tPointXY offsetXY;
    offsetXY.y       = (state->screen_height * .060f);
    offsetXY.x       = (state->screen_width  * .035f);
    tRectBounds tvRect;
    tvRect.w         = vgGetParameteri(tvImage, VG_IMAGE_WIDTH);
    tvRect.h         = vgGetParameteri(tvImage, VG_IMAGE_HEIGHT);
    tvRect.x         = (state->screen_width  - tvRect.w) / 2;
    tvRect.y         = (state->screen_height - tvRect.h);
    tRectBounds imageRect;
    imageRect.h      = tvRect.h  - (offsetXY.y * 2);
    imageRect.w      = tvRect.w  - (offsetXY.x * 2);
    imageRect.x      = (state->screen_width - imageRect.w) / 2;
    imageRect.y      = (state->screen_height - imageRect.h) - (tvRect.h - imageRect.h) / 2;
    unsigned char * downloadData = NULL;
    unsigned char * imageData = NULL;
    unsigned int imageDataSize;

    if(rec->description)
    {
        redraw_results(false);
        show_big_message("Info: loading...", rec->description);
        vgSetPixels(tvRect.x,
                    tvRect.y,
                    tvImage,
                    0, 0,
                    tvRect.w,
                    tvRect.h);

        eglSwapBuffers(state->display, state->surface);
    }
    else //description not found.
        show_message("OOPS! description == NULL", true, ERROR_POINT);

    if(rec->thumbLarge != NULL)
    {
        if (rec->largeImage == 0)
            rec->largeImage = load_jpeg2(rec->thumbLarge, imageRect.w, imageRect.h,
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

            vgSetPixels(tvRect.x,
                        tvRect.y,
                        tvImage,
                        0, 0,
                        tvRect.w,
                        tvRect.h);

            vgSetPixels(imageRect.x,
                        imageRect.y,
                        rec->largeImage,
                        0, 0,
                        imageRect.w,
                        imageRect.h);

            key = toupper(readKb_mouse());
            if (key == 'H' && imageData != NULL)
            {
                vgDestroyImage(-rec->largeImage);
                rec->largeImage = OMXCreateImageFromBuf((unsigned char *)
                                                        imageData, imageDataSize, imageRect.w, imageRect.h);
            }
            else if (key == 'S' && imageData != NULL)
            {
                vgDestroyImage(rec->largeImage);
                rec->largeImage = createImageFromBuf((unsigned char *)
                                                     imageData, imageDataSize, imageRect.w, imageRect.h);
            }
            else if (key == MOUSE_1)
            {
                //handle the mouse
                if(point_in_rect(&clickXY, &tvRect))
                    key = RTN_KEY;
                else
                    key = ESC_KEY;
            }
            else if (key == MOUSE_2)
                key = ESC_KEY;
            else if (key== CUR_L || key == CUR_R ||
                     key == CUR_UP || key == CUR_DWN)
                break;
        }
        while ( key != ESC_KEY &&
                key != RTN_KEY &&
                key != JOY_1 &&
                key != CUR_L &&
                key != CUR_R &&
                key != CUR_UP &&
                key != CUR_DWN &&
                key != 'I' &&
                key != 'F');
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
#define COLOR_SELECTED selectedColor
#define COLOR_NORMAL   bgColor
#define TEXT_SELECTED  selectedColor
#define TEXT_NORMAL    textColor

#define OSK_KEY 30
#define OSK_DEL 31
#define OSK_SPC 32
#define OSK_CLR 33
#define OSK_RTN 34
//------------------------------------------------------------------------------
static char * oskKeyMap[2][30] =
{
    {
        "Q",  "W", "E", "R", "T", "Y", "U", "I", "O",  "P",
        "A",  "S", "D", "F", "G", "H", "J", "K", "L",  "\"",
        "`",   "Z", "X", "C", "V", "B", "N", "M", ",",  "."
    },
    {
        "!", "@", "#", "$", "%",  "^", "&", "*", "(",  ")",
        "1", "2", "3", "4", "5",  "6", "7", "8", "9",  "0",
        "~", "<", ">", "?", "[",  "]", "{", "}", "\\", "/"
    }
};
//------------------------------------------------------------------------------
static int osk_key_index(int * page, int c)
{
    int index;
    for ((*page) = 0; (*page) < 2; (*page)++)
        for(index = 0; index < 30; index++)
            if (oskKeyMap[(*page)][index][0] == c ||
                    oskKeyMap[(*page)][index][0] == toupper(c))
                return index;
    return -1;
}
//------------------------------------------------------------------------------
bool input_string(char * prompt, char * buf, int max)
{
    tRectBounds keyRect;
    int std_key_width  = state->screen_width / 14;
    int std_key_w = std_key_width * .90f;
    int large_key_width = std_key_width * 1.5f;
    int large_key_w = large_key_width * .90f;
    int key_height = state->screen_height / 10;
    keyRect.h = key_height * .90f;
    int space_width = std_key_width * 4;
    int space_w = space_width * .97f;
    tPointXY offsetXY;
    int key = 0x00;
    int x, y;
    int sel = 0;
    int old;
    int result, page;
    int keyMapIndex = 0;
    offsetXY.x = std_key_width / 3;
    offsetXY.y = key_height / 3;
    int offsetX2 = std_key_width / 6;
    char * save = malloc(max+1);
    strcpy(save, buf);
    do
    {
        if(key != MOUSE_1) clear_screen(false);
        int endPos = strlen(buf);
        keyRect.w = std_key_w;
        int index = 0;
        for(y = 0; y < 3; y++)
        {
            for (x = 0; x < 10; x++)
            {
                keyRect.x = (x + 2) * std_key_width;
                keyRect.y = state->screen_height - ((y + 1) * key_height);
                if (key == MOUSE_1)
                {
                    if (point_in_rect(&clickXY, &keyRect))sel=index;
                }
                else
                {
                    Roundrect(keyRect.x, keyRect.y,  keyRect.w, keyRect.h, 20, 20, numRectPenSize, rectColor, (sel==index)?COLOR_SELECTED:COLOR_NORMAL);
                    textXY(keyRect.x + offsetXY.x, keyRect.y + offsetXY.y, oskKeyMap[keyMapIndex][index], numPointFontLarge,  (sel==index)?TEXT_SELECTED:TEXT_NORMAL);
                }
                index++;
            }
        }
        keyRect.x = (2) * std_key_width;
        keyRect.y = state->screen_height - ((y + 1) * key_height);
        keyRect.w = large_key_w;
        if (key == MOUSE_1)
        {
            if (point_in_rect(&clickXY, &keyRect))sel=OSK_KEY;
            keyRect.x += large_key_width;
            if (point_in_rect(&clickXY, &keyRect))sel=OSK_DEL;
            keyRect.x += large_key_width;
            keyRect.w = space_w;
            if (point_in_rect(&clickXY, &keyRect))sel=OSK_SPC;
            keyRect.x += space_width;
            keyRect.w = large_key_w;
            if (point_in_rect(&clickXY, &keyRect))sel=OSK_CLR;
            keyRect.x += large_key_width;
            if (point_in_rect(&clickXY, &keyRect))sel=OSK_RTN;
            if(sel == - 1)
            {
                sel = old;
                key = 0x00;
            }
            else
                key = JOY_1;
        }
        else
        {
            Roundrect(keyRect.x, keyRect.y,  keyRect.w, keyRect.h, 20, 20, numRectPenSize, (keyMapIndex==0)?rectColor:rectColor3, (sel==OSK_KEY)?COLOR_SELECTED:COLOR_NORMAL);
            textXY(keyRect.x + offsetX2, keyRect.y + offsetXY.y,  "!@#", numPointFontLarge,  (sel==OSK_KEY)?TEXT_SELECTED:TEXT_NORMAL);
            keyRect.x += large_key_width;
            Roundrect(keyRect.x, keyRect.y,  keyRect.w, keyRect.h, 20, 20, numRectPenSize, rectColor,   (sel==OSK_DEL)?COLOR_SELECTED:COLOR_NORMAL);
            textXY(keyRect.x + offsetXY.x, keyRect.y + offsetXY.y,  "DEL", numPointFontLarge,  (sel==OSK_DEL)?TEXT_SELECTED:TEXT_NORMAL);
            keyRect.x += large_key_width;
            keyRect.w = space_w;
            Roundrect(keyRect.x, keyRect.y,  space_w, keyRect.h, 20, 20, numRectPenSize, rectColor, (sel==OSK_SPC)?COLOR_SELECTED:COLOR_NORMAL);
            keyRect.x += space_width;
            keyRect.w = large_key_w;
            Roundrect(keyRect.x, keyRect.y,  keyRect.w, keyRect.h, 20, 20, numRectPenSize, rectColor,   (sel==OSK_CLR)?COLOR_SELECTED:COLOR_NORMAL);
            textXY(keyRect.x + offsetX2, keyRect.y + offsetXY.y,  "CLR", numPointFontLarge,    (sel==OSK_CLR)?TEXT_SELECTED:TEXT_NORMAL);
            keyRect.x += large_key_width;
            Roundrect(keyRect.x, keyRect.y,  keyRect.w, keyRect.h, 20, 20, numRectPenSize, rectColor,   (sel==OSK_RTN)?COLOR_SELECTED:COLOR_NORMAL);
            textXY(keyRect.x + offsetX2, keyRect.y + offsetXY.y,  "RTN", numPointFontLarge,  (sel==OSK_RTN)?TEXT_SELECTED:TEXT_NORMAL);
            draw_txt_box_cen(prompt, .95f, .50f, .05, .10f, .50f, numPointFontLarge);
            buf[endPos] = '_';
            buf[endPos+1]= 0x00;
            textXY(state->screen_width * .10f, state->screen_height * .30f, buf, numPointFontLarge, textColor);
            buf[endPos] = 0x00;
            key = readKb_mouse();
        }

        switch(key)
        {
        case MOUSE_1:
            old = sel;
            sel = -1;
            break;

        case CUR_L:
            if (sel > 0)
                sel--;
            else
                sel = OSK_RTN;
            break;

        case CUR_R:
            if(sel < OSK_RTN)
                sel++;
            else
                sel = 0;
            break;

        case CUR_DWN:
            if(sel < 20)
                sel += 10;
            else if (sel == 20 || sel == 21)
                sel = OSK_KEY;
            else if(sel == 22)
                sel = OSK_DEL;
            else if(sel >= 23 && sel <= 26)
                sel = OSK_SPC;
            else if(sel == 27)
                sel = OSK_CLR;
            else if(sel == 28 || sel == 29)
                sel = OSK_RTN;
            break;

        case CUR_UP:
            if(sel >= 10 && sel <= 30)
                sel -= 10;
            else if(sel == OSK_DEL)
                sel = 22;
            else if(sel == OSK_SPC)
                sel = 23;
            else if(sel == OSK_CLR)
                sel = 27;
            else if(sel == OSK_RTN)
                sel = 29;
            break;

        case JOY_1:
            if(sel == OSK_RTN)
            {
                key = RTN_KEY;
            }
            else if(sel == OSK_KEY)
            {
                keyMapIndex += 1;
                if(keyMapIndex > 1)
                    keyMapIndex = 0;
            }
            else if(sel == OSK_DEL)
            {
                if(endPos > 0)
                    buf[endPos-1] = 0x00;
            }
            else if(sel == OSK_CLR)
            {
                buf[0] = 0x00;
            }
            else if ((strlen(buf) + 3) < max)
            {
                if(sel >= 0 && sel < OSK_KEY)
                    strcat(buf, oskKeyMap[keyMapIndex][sel]);
                else if(sel == OSK_SPC)
                    strcat(buf, " ");
            }
            break;

        case MOUSE_2:
            key = ESC_KEY;
        case ESC_KEY:
            strcpy(buf, save);
            break;

        case DEL_KEY:
            if(endPos > 0)
                buf[endPos-1] = 0x00;
            sel = OSK_DEL;
            break;

        default:
            result = osk_key_index(&page,key);
            if(result != -1)
            {
                keyMapIndex = page;
                if(keyMapIndex == page)
                    sel = result;
            }
            if(result != -1 || key == ' ')
            {
                if ((strlen(buf) + 3) < max)
                {
                    result = strlen(buf);
                    buf[result] = key;
                    buf[result+1] = 0x00;
                }
                if (key == ' ')
                    sel = OSK_SPC;
            }
        }
    }
    while (key != ESC_KEY && key != RTN_KEY);
    free(save);
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
                    state->screen_width  * .90f,
                    7, //max no of lines
                    state->screen_height * numFontSpacing / 1000.0f,
                    message, numPointFontMed, textColor);
}

//------------------------------------------------------------------------------
static tRectBounds imageRect;
void show_message(char * message, int error, int points)
{
    tPointXY offsetXY;
    //tRectBounds imageRect;
    tRectBounds guruRect;
    tRectBounds tvRect;
    offsetXY.y   = (state->screen_height * .060f);
    offsetXY.x   = (state->screen_width  * .035f);
    tvRect.w     = vgGetParameteri(tvImage, VG_IMAGE_WIDTH);
    tvRect.h     = vgGetParameteri(tvImage, VG_IMAGE_HEIGHT);
    tvRect.x     = (state->screen_width  - tvRect.w) / 2;
    tvRect.y     = (state->screen_height - tvRect.h) / 2;
    imageRect.h  = tvRect.h -  (offsetXY.y * 2);
    imageRect.w  = tvRect.w  - (offsetXY.x * 2);
    imageRect.x  = (state->screen_width - imageRect.w)   / 2;
    imageRect.y  = (state->screen_height - imageRect.h) / 2;
    int tx       = imageRect.x  + state->screen_width  * .03f;
    int ty       = imageRect.y  + imageRect.h - offsetXY.x;
    guruRect.w   = imageRect.w * .95f;
    guruRect.h   = imageRect.h * .25f;
    guruRect.x   = (state->screen_width - guruRect.w)   / 2;
    guruRect.y   = imageRect.y  + imageRect.h - guruRect.h -  (state->screen_width  * .007f);

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

        vgSetPixels(tvRect.x,
                    tvRect.y,
                    tvImage,
                    0, 0,
                    tvRect.w,
                    tvRect.h);

        if(error)
        {
            Roundrect(imageRect.x, imageRect.y,  imageRect.w, imageRect.h, 20, 20, numRectPenSize, bgColor, bgColor);
            Rect(guruRect.x, guruRect.y, guruRect.w, guruRect.h, numRectPenSize, bgColor, errorColor);
            Text_Rollover ( &fontDefs[1], //Topaz font
                            tx, // X
                            ty, // Y
                            state->screen_width  * .80f,
                            state->screen_width  * .90f,
                            6,
                            state->screen_height * .05f,
                            errorStr, points, &colorScheme[0], VG_FILL_PATH, true);
        }
        else
        {
            Roundrect(imageRect.x,  imageRect.y, imageRect.w, imageRect.h, 20, 20, numRectPenSize, rectColor, bgColor);
            Text_Rollover ( &fontDefs[1], //Topaz font
                            tx, // X
                            ty, // Y
                            state->screen_width  * .80f,
                            state->screen_width  * .90f,
                            8,
                            state->screen_height * .05f,
                            message, points, &colorScheme[0], VG_FILL_PATH, true);
        }
        if(error)
        {
            key = readKb_mouse();
            redraw_results(true);
        }
        else
            break;

    }
    while (key != ESC_KEY &&
            key != RTN_KEY &&
            key != JOY_1 &&
            key != MOUSE_1 &&
            key != MOUSE_2);
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
    tRectPer rectPer;
    rectPer.xPer = .88f;
    rectPer.yPer = .83f;
    rectPer.wPer = .04f;
    rectPer.hPer = .04f;
    init_arrow(menu->upArrow, &rectPer, true);
    rectPer.yPer = .10f;
    init_arrow(menu->downArrow, &rectPer, false);

    menu->upArrowPer.xPer = .88f;
    menu->upArrowPer.yPer = .83f;
    calc_rect_bounds(&menu->upArrowPer, &menu->upArrowRect);
    menu->downArrowPer.xPer = .88f;
    menu->downArrowPer.yPer = .10f;
    calc_rect_bounds(&menu->downArrowPer, &menu->downArrowRect);
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
    tRectPer rectPer;
    rectPer.xPer = .47f;
    rectPer.yPer = .83f;
    rectPer.wPer = .04f;
    rectPer.hPer = .04f;
    init_arrow(menu->upArrow, &rectPer, true);
    rectPer.yPer = .53f;
    init_arrow(menu->downArrow, &rectPer, false);
    menu->upArrowPer.xPer = .47;
    menu->upArrowPer.yPer = .83f;
    calc_rect_bounds(&menu->upArrowPer, &menu->upArrowRect);
    menu->downArrowPer.xPer = .47f;
    menu->downArrowPer.yPer = .53f;
    calc_rect_bounds(&menu->downArrowPer, &menu->downArrowRect);
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
void jskb_menu_detail(tMenuState * menu)
{
    if(menu->menuItems[menu->selectedItem].special > 0)
    {
        char * descr = NULL;
        char temp[10];
        switch(menu->menuItems[menu->selectedItem].special)
        {
        case 1:
            snprintf(temp, sizeof(temp), "[%d]", jsXAxis);
            descr = temp;
            break;
        case 2:
            snprintf(temp, sizeof(temp), "[%d]", jsYAxis);
            descr = temp;
            break;
        case 3:
            snprintf(temp, sizeof(temp), "[%d]", jsThreshold);
            descr = temp;
            break;
        case 4:
            snprintf(temp, sizeof(temp), "[%d]", jsInfo);
            descr = temp;
            break;
        case 5:
            snprintf(temp, sizeof(temp), "[%d]", jsMenu);
            descr = temp;
            break;
        case 6:
            snprintf(temp, sizeof(temp), "[%d]", jsSelect);
            descr = temp;
            break;
        case 7:
            snprintf(temp, sizeof(temp), "[%d]", jsBack);
            descr = temp;
            break;
        case 8:
            descr = mouseEnabled?"[true]":"[false]";
            break;
        case 9:
            descr = mouseDev;
            break;
        case 10:
            descr = jsDev;
            break;
        case 11:
            snprintf(temp, sizeof(temp), "[%d]", numPointerIndex);
            descr = temp;
            break;
        case 12:
            snprintf(temp, sizeof(temp), "[%d]", numPointerSize);
            descr = temp;
            break;
        case 13:
            snprintf(temp, sizeof(temp), "[%d]", pointerOffsetXY.x);
            descr = temp;
            break;
        case 14:
            snprintf(temp, sizeof(temp), "[%d]", pointerOffsetXY.y);
            descr = temp;
            break;
        }
        if(descr != NULL)
            textXY(state->screen_width * .30,
                   menu->txtRaster.y,
                   descr,
                   numPointFontMed, errorColor);
    }
}
//------------------------------------------------------------------------------
void gui_menu_detail(tMenuState * menu)
{
    if(menu->menuItems[menu->selectedItem].special > 0)
    {
        char * descr = NULL;
        char temp[10];
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

        case 6:
            snprintf(temp, sizeof(temp), "[%d]", numRow);
            descr = temp;
            break;

        case 7:
            snprintf(temp, sizeof(temp), "[%d]", numCol);
            descr = temp;
            break;

        case 8:
            snprintf(temp, sizeof(temp), "[1/%d]", numThumbWidth);
            descr = temp;
            break;

        case 9:
            snprintf(temp, sizeof(temp), "[%d]", numPointFontTiny);
            descr = temp;
            break;

        case 10:
            snprintf(temp, sizeof(temp), "[%d]", numPointFontSmall);
            descr = temp;
            break;

        case 11:
            snprintf(temp, sizeof(temp), "[%d]", numPointFontMed);
            descr = temp;
            break;

        case 12:
            snprintf(temp, sizeof(temp), "[%d]", numPointFontLarge);
            descr = temp;
            break;

        case 13:
            snprintf(temp, sizeof(temp), "[%d]", numFontSpacing);
            descr = temp;
            break;

        }

        if(descr != NULL)
            textXY(state->screen_width * .30,
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
            textXY(state->screen_width * .30,
                   menu->txtRaster.y,
                   descr,
                   numPointFontMed, errorColor);
        if(videoFormat != NULL)
            free(videoFormat);
    }
}
//------------------------------------------------------------------------------
bool set_int(int min, int max, int offset, int * value)
{
    int oldValue = *value;

    if(offset > 0)
    {
        if (*value + offset <= max)
            *value += offset;
        else
            *value = max;
    }
    else
    {
        if (*value + offset >= min)
            *value += offset;
        else
            *value = min;
    }
    if(oldValue != *value)
        return true;
    else
        return false;
}
//------------------------------------------------------------------------------
#define REDRAW_GUI_KEYPRESS {redraw_results(false);setBGImage();dumpKb();}
void gui_menu_keypress(tMenuState * menu, int key)
{
    if(key == CUR_R || key == CUR_L)
    {
        int offset = 1;
        if(key == CUR_L)
            offset = -1;
        switch(menu->menuItems[menu->selectedItem].special)
        {
        case 6:
            if(set_int(2, 15, offset, &numRow))
                numResults = numRow * numCol;
            REDRAW_GUI_KEYPRESS;
            break;

        case 7:
            if(set_int(1, 4, offset, &numCol))
                numResults = numRow * numCol;
            REDRAW_GUI_KEYPRESS;
            break;

        case 8:
            if(set_int(3, 20, offset * -1, &numThumbWidth))
                REDRAW_GUI_KEYPRESS;
            break;
        case 9 :
            if(set_int(5, 15, offset, &numPointFontTiny))
                REDRAW_GUI_KEYPRESS;
            break;
        case 10:
            if(set_int(5, 20, offset, &numPointFontSmall))
                REDRAW_GUI_KEYPRESS;
            break;
        case 11:
            if(set_int(10, 40, offset, &numPointFontMed))
                REDRAW_GUI_KEYPRESS;
            break;
        case 12:
            set_int(15, 50, offset, &numPointFontLarge);
            break;
        case 13:
            set_int(15, 50, offset, &numFontSpacing);
            REDRAW_GUI_KEYPRESS;
            break;
        }
    }
}
//------------------------------------------------------------------------------
void jskb_menu_keypress(tMenuState * menu, int key)
{
    if(key == CUR_R || key == CUR_L)
    {
        int offset = 1;

        if (strcmp(menu->menuItems[menu->selectedItem].key, "TH") == 0)
            offset = 1000;

        if(key == CUR_L)
            offset *= -1;

        switch(menu->menuItems[menu->selectedItem].special)
        {
        case 1:
            set_int(0,    15,    offset, &jsXAxis    );
            break;
        case 2:
            set_int(0,    15,    offset, &jsYAxis    );
            break;
        case 3:
            set_int(1, 32767,    offset, &jsThreshold);
            break;
        case 4:
            set_int(0,    15,    offset, &jsInfo     );
            break;
        case 5:
            set_int(0,    15,    offset, &jsMenu     );
            break;
        case 6:
            set_int(0,    15,    offset, &jsSelect   );
            break;
        case 7:
            set_int(0,    15,    offset, &jsBack     );
            break;
        case 8:
            //mouseEnabled = !mouseEnabled;
            break;
        case 9:
            set_int(0,    5,    offset, &numMouseIndex);
            open_mouse();
            break;
        case 10:
            set_int(0,    5,    offset, &numJoystickIndex);
            open_joystick();
            break;
        case 11:
            set_int(0, 255,     offset, &numPointerIndex);
            break;
        case 12:
            set_int(10, 100,    offset, &numPointerSize);
            break;
        case 13:
            set_int(-50, 50,     offset, &pointerOffsetXY.x);
            break;
        case 14:
            set_int(-50, 50,    offset, &pointerOffsetXY.y);
            break;
        };
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
    int clickIndex;
    menu->downArrowRect.w = vgGetParameteri(downArrowImage, VG_IMAGE_WIDTH);
    menu->downArrowRect.h = vgGetParameteri(downArrowImage, VG_IMAGE_HEIGHT);
    menu->upArrowRect.w = vgGetParameteri(upArrowImage, VG_IMAGE_WIDTH);
    menu->upArrowRect.h = vgGetParameteri(upArrowImage, VG_IMAGE_HEIGHT);

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

                menu->selectedItem = y + menu->scrollIndex;

                textXY(menu->txtRaster.x,
                       menu->txtRaster.y,
                       currentItem->description,
                       numPointFontMed,
                       textColor);

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
            if(downArrowImage > 0)
                vgSetPixels(menu->downArrowRect.x, menu->downArrowRect.y,
                        downArrowImage, 0,0,
                        menu->downArrowRect.w, menu->downArrowRect.h);
            else
                Poly(menu->downArrow, 4, numRectPenSize / 2, selectedColor, bgColor, VG_TRUE);
        }

        if (menu->scrollIndex > 0)
        {
            if(upArrowImage > 0)
                vgSetPixels(menu->upArrowRect.x, menu->upArrowRect.y,
                        upArrowImage, 0,0,
                        menu->downArrowRect.w, menu->downArrowRect.h);
            else
                Poly(menu->upArrow, 4, numRectPenSize / 2, selectedColor, bgColor, VG_TRUE);
        }

        if (menu->drawFooter != NULL)
            menu->drawFooter(menu);


        key = toupper(readKb_mouse());
        if(menu->keyPress != NULL)
            menu->keyPress(menu, key);

        switch (key)
        {

        case MOUSE_1:
            if(point_in_rect(&clickXY, &menu->winRect))
            {
                if(point_in_rect(&clickXY, &menu->upArrowRect))
                {
                    goto LCUR_UP;
                }
                else if(point_in_rect(&clickXY, &menu->downArrowRect))
                    goto LCUR_DWN;

                clickIndex = (state->screen_height - clickXY.y - menu->txtOffset.y) / menu->yStep;
                //printf("Got click (%d, %d):\n", clickXY.x, clickXY.y);
                //printf("menu->menu->yStep = %d\n", menu->yStep);
                //printf("menu->txtOffset.y = %d\n", menu->txtOffset.y);
                //printf("clickIndex = %d\n", clickIndex);
                if (clickIndex < menu->maxItems && clickIndex >= 0)
                {
                    if (clickIndex < menu->selectedIndex)
                    {
                        menu->selectedIndex = clickIndex;
                    }
                    else if (clickIndex > menu->selectedIndex)
                    {
                        while(clickIndex > menu->selectedIndex &&
                                (&menu->menuItems[menu->selectedIndex + 1].key != NULL &&
                                 &menu->menuItems[menu->selectedIndex + 1].description != NULL))
                        {
                            menu->selectedIndex++;
                        };
                    }
                    else if (clickIndex == menu->selectedIndex)
                        key = RTN_KEY;
                }
            }
            else
                key = ESC_KEY;
            break;
        case CUR_UP:
             LCUR_UP:
            if (menu->selectedIndex > 0)
                menu->selectedIndex--;
            else if(menu->scrollIndex > 0)
                menu->scrollIndex--;
            break;

        case CUR_DWN:
             LCUR_DWN:
            if (menu->selectedIndex < menu->maxItems -1)
            {
                currentItem = &menu->menuItems[menu->selectedIndex + menu->scrollIndex + 1];
                if(currentItem->key != NULL || currentItem->description != NULL)
                    menu->selectedIndex++;
            }
            else if(bMoreItems)
                menu->scrollIndex++;

            break;
        }
    }

    while (key != RTN_KEY && key != JOY_1 && key != ESC_KEY && key != MOUSE_2);
    if(key == ESC_KEY || key == MOUSE_2)
    {
        //restore previous value
        menu->scrollIndex = scrollIndexSave;
        menu->selectedIndex = selectedIndexSave;
        return -1;
    }
    return menu->selectedItem;
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
bool point_in_rect(tPointXY * point, tRectBounds * rect)
{
    if ((point->x >= rect->x && point->x <= (rect->x + rect->w)) &&
            (point->y >= rect->y && point->y <= (rect->y + rect->h)))
        return true;
    return false;
}
//------------------------------------------------------------------------------
int mouse_select(tPointXY * point)
{
    tPointXY    offset;
    tPointXY    step;
    tRectBounds rbTest;
    int numRectPenSize2 = numRectPenSize  * 2;
    offset.x = (int) (state->screen_width * .05f);
    offset.y = (int) (state->screen_height * numFontSpacing / 1000.0f);
    step.x   = (state->screen_width  - (offset.x * 2)) / numCol;
    step.y   = (state->screen_height / numRow);
    rbTest.h = step.y - numRectPenSize2;
    rbTest.w = step.x - numRectPenSize2;
    rbTest.x = offset.x;
    struct result_rec * temp = first_rec;
    int rowCount = 0;
    while (temp != NULL)
    {
        rbTest.y   = state->screen_height - ((rowCount+1) * step.y);
        if(point_in_rect(point, &rbTest))
        {
            selected_rec = temp;
            return 0;
        }

        temp = temp->next;
        if(++rowCount == numRow)
        {
            rowCount = 0;
            rbTest.x   += step.x;
        }
    }
    if (point->x < offset.x)
        return -1;
    else if (point->x > offset.x + rbTest.w * numCol)
        return 1;
    else
        return 0;
}
//------------------------------------------------------------------------------
void redraw_results(bool swap)
{
    tPointXY    offset;
    tPointXY    step;
    tRectBounds rbMain;
    tRectBounds rbPic;
    tRectBounds rbJpg;
    tRectBounds rbTxt;
    int numRectPenSize2 = numRectPenSize  * 2;
    int numRectPenSize5 = numRectPenSize  * 5;
    offset.x = (int) (state->screen_width * .05f);
    offset.y = (int) (state->screen_height * numFontSpacing / 1000.0f);
    step.x   = (state->screen_width  - (offset.x * 2)) / numCol;
    step.y   = (state->screen_height / numRow);
    rbMain.h = step.y - numRectPenSize2;
    rbMain.w = step.x - numRectPenSize2;
    rbMain.x = offset.x;
    rbPic.x  = rbMain.x;
    rbPic.w  = state->screen_width / numThumbWidth;
    rbPic.h  = rbMain.h;
    rbJpg.x  = rbPic.x + numRectPenSize;
    rbJpg.w  = rbPic.w - numRectPenSize2;
    rbJpg.h  = rbPic.h - numRectPenSize2;
    rbTxt.w  = rbMain.w - rbPic.w  - numRectPenSize2;
    rbTxt.h  = rbMain.h - numRectPenSize2;
    rbTxt.x  = rbMain.x + rbPic.w  + numRectPenSize;
    int txtBrk = rbTxt.w * .85f;
    int maxLines = rbTxt.h / offset.y;
    int rowCount = 0;
    int count = 0;
    clear_screen(false);
    struct result_rec * temp = first_rec;
    while (temp != NULL)
    {
        if (selected_rec == NULL)
            selected_rec = temp;

        rbMain.y   = state->screen_height - ((rowCount+1) * step.y);
        rbPic.y    = rbJpg.y = rbMain.y;
        rbJpg.y   += numRectPenSize;
        rbTxt.y    = rbMain.y + step.y - numRectPenSize5;
        if (temp == selected_rec)
        {
            Roundrect(rbMain.x, rbMain.y, rbMain.w, rbMain.h, 20, 20, numRectPenSize, rectColor, selectedColor);
            if(temp->title != NULL)
                textXY_Rollover(rbTxt.x, rbTxt.y , rbTxt.x + txtBrk, rbTxt.x + rbTxt.w, maxLines, offset.y, temp->title, numPointFontMed, selectedColor);
        }
        else
        {
            Roundrect(rbMain.x, rbMain.y, rbMain.w, rbMain.h, 20, 20, numRectPenSize, rectColor, outlineColor);
            if(temp->title != NULL)
                textXY_Rollover(rbTxt.x, rbTxt.y , rbTxt.x + txtBrk, rbTxt.x + rbTxt.w, maxLines, offset.y, temp->title, numPointFontMed, textColor);
        }

        Roundrect(rbPic.x, rbPic.y, rbPic.w, rbPic.h, 20, 20, numRectPenSize / 2, rectColor2, outlineColor2);

        if(temp->thumbSmall != NULL)
        {
            if(temp->image == 0)
                temp->image = load_jpeg(temp->thumbSmall, rbJpg.w, rbJpg.h);
            vgSetPixels(rbJpg.x, rbJpg.y, temp->image, 0,0, rbJpg.w, rbJpg.h);
            //  vgGetParameteri(temp->image, VG_IMAGE_WIDTH),
            //  vgGetParameteri(temp->image, VG_IMAGE_HEIGHT));
        }
        temp = temp->next;
        if(++rowCount == numRow)
        {
            rowCount = 0;
            rbMain.x   += step.x;
            rbTxt.x    += step.x;
            rbPic.x    += step.x;
            rbJpg.x    += step.x;
        }
        if(++count == numResults)
            break;
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

//------------------------------------------------------------------------------
bool yes_no_dialog(char * prompt, bool val)
{
    char formatNo[]  = "\n%s\n\n ~7yes      ~5[no]";
    char formatYes[] = "\n%s\n\n ~5[yes]      ~7no";
    size_t size = strlen(formatYes) + strlen(prompt);
    char * temp = malloc(size);
    int key;
    bool old;
    do
    {
        drawBGImage();
        if(val)
            snprintf(temp, size, formatYes, prompt);
        else
            snprintf(temp, size, formatNo, prompt);

        show_message(temp, false, numPointFontLarge);
        key = toupper(readKb_mouse());
        switch(key)
        {
        case MOUSE_1:
            if(point_in_rect(&clickXY, &imageRect))
            {
                old = val;
                if (clickXY.x - imageRect.x > imageRect.w / 2)
                    val = false;
                else
                    val = true;
                if(val == old)
                    key = RTN_KEY;
            }
            break;
        case 'Y':
            val = true;
            key = RTN_KEY;
            break;
        case 'N':
            val = true;
            key = RTN_KEY;
            break;
        case CUR_L:
            val = true;
            break;
        case CUR_R:
            val = false;
            break;
        case ESC_KEY:
            val = false;
            break;
        case JOY_1:
        case RTN_KEY:
            key = RTN_KEY;
            break;
        }
    }
    while (key != ESC_KEY && key != RTN_KEY);
    free(temp);
    return val;
}

