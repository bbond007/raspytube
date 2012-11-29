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
//#include "menus.h"
//stuff for the keyboard. 

static int ttflags;
static struct termios oldt;
static struct termios newt;
static AudioSampleInfo asiKbClick;
extern const signed char soundraw_data[];
extern const unsigned int soundraw_size;

struct result_rec * first_rec    = NULL;
struct result_rec * last_rec     = NULL;
struct result_rec * selected_rec = NULL;

int numPointFontTiny;
int numPointFontSmall;
int numPointFontMed;
int numPointFontLarge;
int numThumbWidth     = 13;
int numResults        = 10;
int numFormat         = 0;
int numStart          = 1;

enum tSoundOutput soundOutput = soHDMI;
enum tVideoPlayer videoPlayer = vpOMXPlayer;
enum tJpegDecoder jpegDecoder = jdOMX;
#define ERROR_POINT  (numPointFontMed)

VGfloat textColor[4]        = {  5,  5,  5,1};
VGfloat rectColor[4]        = {  0,  0,  5,1};
VGfloat rectColor2[4]       = {  0,  0,  0,1};
VGfloat outlineColor[4]     = {  0,  0,  3,1};
VGfloat outlineColor2[4]    = {  2,  5,  2,1};
VGfloat selectedColor[4]    = {  2,1.5,  0,1};
VGfloat bgColor[4]          = {  0,  0,  0,1};
VGfloat errorColor[4]       = {  4,  0,  0,1};

extern unsigned char *download_file(char * host, char * fileName, unsigned int * fileSize);
extern unsigned char *find_jpg_start(unsigned char * buf, unsigned int * bufSize);
extern VGImage OMXCreateImageFromBuf(unsigned char * buf, unsigned int bufLength, unsigned int outputWidth, unsigned int outputHeight);


//------------------------------------------------------------------------------
struct result_rec * init_result_rec()
{
    struct result_rec * new_rec = malloc(sizeof(struct result_rec));
    if (new_rec != NULL)
    {
        new_rec->image       = 0;
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

void init_ui_var()
{
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
}

//------------------------------------------------------------------------------

unsigned int HandleESC()
{
    int key;
    if (!kbHit())
    {
        return ESC_KEY;
    }
    else
    {
        key = readKb();
        switch(key)
        {
        case '[':
            if(kbHit())
            {
                key = readKb();
                switch(key) //cursor movement
                {
                case CUR_R:
                    return CUR_R;
                    break;

                case CUR_L:
                    return CUR_L;
                    break;

                case CUR_UP:
                    return CUR_UP;
                    break;

                case CUR_DWN:
                    return CUR_DWN;
                    break;
                }
            }
            break;
        case 'O' :
            if(kbHit()) //function keys
            {
                key = readKb();
                switch(key)
                {
                case FUN_1:
                    //eglSwapBuffers(state->display, state->surface);
                    DoSnapshot();
                    show_message("Snapshot Saved!", true, ERROR_POINT);
                    return FUN_1;
                    break;
                }
            }
            break;
        }
    }
    return 0;
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
void draw_txt_box(char * message, float widthP, float heightP, float boxYp, float tXp, float tYp, int points, bool swap)
{
    int width  = state->screen_width * widthP;
    int height = state->screen_height * heightP;
    int x = (state->screen_width  - width) / 2;
    int y = state->screen_height * boxYp;
    int tx = state->screen_width * tXp;
    int ty = state->screen_height * tYp;
    Roundrect(x,y, width, height, 20, 20, 10, rectColor, selectedColor);
    Text_DejaVuSans(tx, ty, message, points, textColor);
    if(swap)
        eglSwapBuffers(state->display, state->surface);
}

//------------------------------------------------------------------------------
void clear_screen(bool swap)
{
    glClear( GL_COLOR_BUFFER_BIT );
    vgSetfv(VG_CLEAR_COLOR, 4, bgColor);
    vgClear(0, 0, state->screen_width, state->screen_height);
    vgLoadIdentity();
    if(swap)
        eglSwapBuffers(state->display, state->surface);
}

//------------------------------------------------------------------------------
int show_selection_info(struct result_rec * rec)
{
    int key = 0x00;   
    if(rec->description)
    {
        redraw_results(false);
        show_big_message("Info: loading...", rec->description, false);
        eglSwapBuffers(state->display, state->surface);
    }
    else //description not found.
        show_message("OOPS! description == NULL", true, ERROR_POINT);

    if(rec->thumbLarge != NULL)
    {
        unsigned int image_width  = (state->screen_width  * .30f);
        unsigned int image_height = (state->screen_height * .35f);
        VGImage image = load_jpeg(rec->thumbLarge, image_width, image_height);
        image_width  = vgGetParameteri(image, VG_IMAGE_WIDTH);
        image_height = vgGetParameteri(image, VG_IMAGE_HEIGHT);

        char * infoStr = NULL;

        if(rec->date != NULL && rec->user != NULL && rec->id != NULL)
        {
            char * T = strchr(rec->date, 'T'); //remove after T - time unwanted
            if (T != NULL) *T= 0x00;
            char formatStr[] = "Info: #%s : %s : %s";
            int size = strlen(rec->id)   +
                       strlen(rec->date) +
                       strlen(rec->user) +
                       strlen(formatStr);
            infoStr = malloc(size);
            sprintf(infoStr, formatStr, rec->id, rec->user, rec->date);
            if (T != NULL) *T= 'T';
        }
        else
        {
            if(rec->description)
                show_big_message("Info: ???", rec->description, false);
        }


        int offsetY      = (state->screen_height * .02f);
        int offsetX      = (state->screen_width  * .02f);
        int imageY       = (state->screen_height - image_height) - offsetX;
        int imageX       = (state->screen_width  - image_width) / 2;
        int rect_height  = image_height + (offsetY * 2);
        int rect_width   = image_width  + (offsetX * 2);
        int rectX        = (state->screen_width  - rect_width) / 2;
        int rectY        = (state->screen_height - rect_height);

        do
        {
            redraw_results(false);
            if(infoStr != NULL && rec->description != NULL)
                show_big_message(infoStr, rec->description, false);    

            Roundrect(rectX,
                      rectY,
                      rect_width,
                      rect_height,
                      20, 20, 10, rectColor2, selectedColor);

            vgSetPixels(imageX,
                        imageY,
                        image,
                        0, 0,
                        image_width,
                        image_height);
            eglSwapBuffers(state->display, state->surface);
            key = readKb();
            if(key == ESC_KEY)
            {
                key = HandleESC();
                if(key == CUR_L || key == CUR_R || 
                   key == CUR_UP || key == CUR_DWN)
                    break;
            }  
        } while (key != ESC_KEY && key != RTN_KEY);
        if(infoStr != NULL) free(infoStr);
        vgDestroyImage(image);
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
    char temp[30];
    char cstr[2];
    cstr[1]=0x00;
    char validChars[] =
        "!@#$%^&*(()_<>?+=1234567890,.ABCDEFGHIJKLMNOPQRSTUVWXYZ ";

    char * save = malloc(max+1);
    strcpy(save, buf);
    do
    {
        clear_screen(false);
        //redraw_results(false);
        draw_txt_box(prompt, .95f, .50f, .05, .10f, .50f, numPointFontLarge, false);
        sprintf(temp, "%02X->%02X:%02X:%02X:%02X:%02X:%02X:%02X", key,
                lastkeys[0], lastkeys[1], lastkeys[2], lastkeys[3], lastkeys[4], lastkeys[5], lastkeys[6]);
        Text_DejaVuSans(state->screen_width * .10f, state->screen_height * .10f, temp, numPointFontMed, textColor);
        endPos = strlen(buf);
        buf[endPos] = '_';
        buf[endPos+1]= 0x00;
        Text_DejaVuSans(state->screen_width * .10f, state->screen_height * .30f, buf, numPointFontLarge, textColor);
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
            key = HandleESC();
            switch (key)
            {
            case ESC_KEY:
                strcpy(buf, save);
                break;

            case CUR_L:
                buf[0] = 0x00;
                break;

            case CUR_R:
                strcpy(buf, save);
                break;
            }
            break;

        case RTN_KEY:
            break;

        default:
            if(strchr(validChars, toupper(key)) != NULL &&  strlen(buf) < max - 3)
            {
                cstr[0] = (char) key;
                strcat(buf, cstr);
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
void show_big_message(char * title, char * message, bool pause)
{
    redraw_results(false);
    draw_txt_box(title, .95f, .50f, .05, .10f, .47f, numPointFontLarge, false);
    Text_DejaVuSans_Rollover(state->screen_width  * .10f,
                             state->screen_height * .40f,
                             state->screen_width  * .85f,
                             7, //max no of lines
                             state->screen_height * .05f,
                             message, numPointFontMed, textColor);

    if(pause)
    {
        dumpKb();
        eglSwapBuffers(state->display, state->surface);
        readKb();
    }
}

//------------------------------------------------------------------------------
void show_message(char * message, bool error, int points)
{
    int width  = state->screen_width * .8f;
    int height = state->screen_height * .4f;
    int x = (state->screen_width  - width) / 2;
    int y = (state->screen_height - height) / 2;
    int tx = state->screen_width * .15f;
    int ty = state->screen_height * .55f;
    
    int key = ESC_KEY;
    do
    {
        redraw_results(false);
        
        if(error)
            Roundrect(x,y, width, height, 20, 20, 10, errorColor, selectedColor);
        else
            Roundrect(x,y, width, height, 20, 20, 10, rectColor, selectedColor);
            Text_DejaVuSans_Rollover(tx, // X
                                     ty, // Y
                                     state->screen_width * .80f,
                                     5,
                                     state->screen_height * .05f,
                                     message, points, textColor);
                                     eglSwapBuffers(state->display, 
                                     state->surface);
        if(error)
        {
            dumpKb();
            key = readKb();
            if(key == ESC_KEY)
                key = HandleESC();
            dumpKb();
            redraw_results(true);
        }
        else
            break;
        
    }while (key != ESC_KEY && key != RTN_KEY);
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
    menu->titlePer.xPer = .15f;
    menu->titlePer.yPer = .87f;
    //menu->titlePer.wPer = 0;
    //menu->titlePer.hPer = 0;
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
    rectPer.yPer = .88f;
    rectPer.wPer = .04f;
    rectPer.hPer = .04f;    
    init_arrow(menu->upArrow, &rectPer, true);
    rectPer.yPer = .08f;
    init_arrow(menu->downArrow, &rectPer, false);      
    menu->drawHeader = NULL;
    menu->drawDetail = NULL;
    menu->drawFooter = NULL;
}

//------------------------------------------------------------------------------
void format_menu_header(tMenuState * menu)
{
    int x;
    for (x = 0; x < AFORMAT_WIDTH; x++)
            Text_DejaVuSans((x+1) * (state->screen_width /  (AFORMAT_WIDTH  + 2)),
                            menu->txtRaster.y + menu->yStep,
                            supported_formats[0][x], (x>=3)?numPointFontSmall:numPointFontMed, errorColor);
}

//------------------------------------------------------------------------------
void format_menu_detail(tMenuState * menu)
{
    int x;
    for (x = 1; x < AFORMAT_WIDTH; x++)
            Text_DejaVuSans((x+1) * (state->screen_width /  (AFORMAT_WIDTH  + 2)),
                            menu->txtRaster.y,
                            supported_formats[menu->selectedItem + 1][x],(x==3)?numPointFontSmall:numPointFontMed, textColor);
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
        //redraw_results(false);
        clear_screen(false);
        draw_txt_box(menu->title, 
                     menu->winPer.wPer, 
                     menu->winPer.hPer, 
                     //menu->winPer.xPer,
                     menu->winPer.yPer, 
                     menu->titlePer.xPer,
                     menu->titlePer.yPer, 
                     menu->numPointFontTitle, false);
  
        menu->selRect.y = state->screen_height - 
            (menu->selectedIndex * menu->yStep) - (menu->yStep / 3.0f) - menu->txtOffset.y;
        
        Roundrect(menu->selRect.x,
                  menu->selRect.y,
                  menu->selRect.w,
                  menu->selRect.h,
                  20, 20, 5, rectColor, selectedColor);
        
        int y = 0;
        int count = 0;
        
        menu->selectedItem = menu->scrollIndex;
        menu->txtRaster.x = menu->txtOffset.x;
        menu->txtRaster.y = SHOW_MENU_Y_CALC; 
                  
        if (menu->drawHeader != NULL)
            menu->drawHeader(menu);

        tMenuItem * currentItem = menu->menuItems;
        while(currentItem->key != NULL && currentItem->description != NULL)
        {
            if(count >= menu->scrollIndex)
            {
                               
                Text_DejaVuSans(menu->txtRaster.x, 
                                menu->txtRaster.y,
                                currentItem->description, 
                                numPointFontMed, 
                                textColor);
                if (menu->drawDetail != NULL)
                    menu->drawDetail(menu);
                y++;
                
                menu->selectedItem = y + menu->scrollIndex;
                menu->txtRaster.x = menu->txtOffset.x;
                menu->txtRaster.y = SHOW_MENU_Y_CALC; 
                
                if (y == menu->maxItems)
                    break;
            }
            currentItem++;count++;
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
            Poly(menu->downArrow, 4, 5, selectedColor, bgColor, VG_TRUE); 
        }   
        
        if (menu->scrollIndex > 0)
        {
            Poly(menu->upArrow, 4, 5, selectedColor, bgColor, VG_TRUE);
        }   
                
        if (menu->drawFooter != NULL)
            menu->drawFooter(menu);  
        eglSwapBuffers(state->display, state->surface);

        
        key = toupper(readKb());
        switch (key)
        {

        case ESC_KEY:
            key = HandleESC();
            switch(key)
            {
            case CUR_UP:
                if (menu->selectedIndex > 0)
                    menu->selectedIndex--;
                else
                    if(menu->scrollIndex > 0)
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
                    
                break;
            }
            break;
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
int readKb()
{
    int key = getchar(); 				// standard getchar call
    //play_sample(&asiKbClick, false);
    return key;
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
            show_message("unable to find jpeg start 0xFF 0xD8", true, ERROR_POINT);
        else
        {
            switch (jpegDecoder)
            {
            case jdOMX:
                vgImage = OMXCreateImageFromBuf(imageData, fileSize, width, height);
                break;

            case jdLibJpeg:
                vgImage = createImageFromBuf(imageData, fileSize, height);
                break;

            default:
                show_message("ERROR:\n\nbad jped decoder enum", true, ERROR_POINT);
                break;
            }
        }
        if(downloadData != NULL) free(downloadData);
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
    //jpegWidth = (int)((jpegWidth / 16)) * 16;
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
            Roundrect(rectOffset, y, rectWidth, rectHeight, 20, 20, 6, rectColor, selectedColor);
            if(temp->title != NULL)
                Text_DejaVuSans_Rollover(txtXoffset, y + halfStep, txtXmax, 2, txtYstep, temp->title, numPointFontMed, selectedColor);
        }
        else
        {
            Roundrect(rectOffset, y, rectWidth, rectHeight, 20, 20, 2, rectColor, outlineColor);
            if(temp->title != NULL)
                Text_DejaVuSans_Rollover(txtXoffset, y + halfStep, txtXmax, 2, txtYstep, temp->title, numPointFontMed, textColor);
        }

        Roundrect(rectOffset, y - rectDiff, rectWidth2, step, 20, 20, 2, rectColor2, outlineColor2);

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
        Text_DejaVuSans(0,state->screen_height * .98f, "[OMXPlayer]", numPointFontTiny, textColor);
        break;
    case vpMPlayer:
        Text_DejaVuSans(0, state->screen_height * .98f, "[MPlayer]",  numPointFontTiny, textColor);
        break;
    }

    switch(jpegDecoder)
    {
    case jdOMX:
        Text_DejaVuSans(0,state->screen_height * .96f, "[OMXJPEG]", numPointFontTiny, textColor);
        break;
    case jdLibJpeg:
        Text_DejaVuSans(0, state->screen_height * .96f, "[LIBJPEG]", numPointFontTiny, textColor);
        break;
    }

    switch(soundOutput)
    {
    case soHDMI :
        Text_DejaVuSans(0, state->screen_height * .94, "(((HDMI)))",  numPointFontSmall, textColor);
        break;
    case soLOCAL:
        Text_DejaVuSans(0, state->screen_height * .94, "(((LOCAL)))", numPointFontSmall, textColor);
        break;
    }

    if (numStart != 1)
    {
        char numStartStr[10];
        sprintf(numStartStr, "<-%d", numStart-1);
        Text_DejaVuSans(0, state->screen_height * .50f, numStartStr, numPointFontMed, textColor);
    }

    if(swap)
        eglSwapBuffers(state->display, state->surface);
}
