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

struct result_rec * first_rec    = NULL;
struct result_rec * last_rec     = NULL;
struct result_rec * selected_rec = NULL;


int numResults   = 10;
int numFormat    = 0;
int numStart     = 1;
enum tSoundOutput soundOutput = soHDMI;
enum tVideoPlayer videoPlayer = vpOMXPlayer;
enum tJpegDecoder jpegDecoder = jdLibJpeg;
#define ERROR_POINT 40

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
void show_selection_info(struct result_rec * rec)
{

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
        unsigned int image_width  = 0;
        unsigned int image_height = (state->screen_height * .50f);
        VGImage image = load_jpeg(rec->thumbLarge, &image_width, &image_height);
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

        redraw_results(false);
        if(infoStr != NULL && rec->description != NULL)
            show_big_message(infoStr, rec->description, false);

        if(infoStr != NULL) free(infoStr);
            
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
        vgDestroyImage(image);
        readKb();
        redraw_results(true);
    }
    else
        show_message("OOPS! rec->thumbLarge == NULL", true, ERROR_POINT);
    
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
        draw_txt_box(prompt, .95f, .50f, .05, .10f, .50f, 40, false);
        sprintf(temp, "%02X->%02X:%02X:%02X:%02X:%02X:%02X:%02X", key,
                lastkeys[0], lastkeys[1], lastkeys[2], lastkeys[3], lastkeys[4], lastkeys[5], lastkeys[6]);
        Text_DejaVuSans(state->screen_width * .10f, state->screen_height * .10f, temp, 16, textColor);
        endPos = strlen(buf);
        buf[endPos] = '_';
        buf[endPos+1]= 0x00;
        Text_DejaVuSans(state->screen_width * .10f, state->screen_height * .30f, buf, 40, textColor);
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
            if (!kbHit()) 
                strcpy(buf, save);
            break;

        case RTN_KEY:
            break;


        case 'A':
        case 'B':
        case 'C':
        case 'D':
        
            if (lastkeys[1] == ESC_KEY)
            {
                switch(key)
                {
                    case 'D':  
                        buf[0] = 0x00;
                        break;
                    case 'C':
                        strcpy(buf, save);
                        break;
                }
                break;
            }
            
        default:
            if(strchr(validChars, toupper(key)) != NULL &&  strlen(buf) < max - 3)
            {
                cstr[0] = (char) key;
                strcat(buf, cstr);
            }
            break;
        }
    }
    while (key != RTN_KEY && (key != ESC_KEY || kbHit()));
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
    draw_txt_box(title, .95f, .50f, .05, .10f, .47f, 40, false);
    Text_DejaVuSans_Rollover(state->screen_width  * .10f,
                             state->screen_height * .40f,
                             state->screen_width  * .85f,
                             7, //max no of lines
                             state->screen_height * .05f,
                             message, 22, textColor);

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
    redraw_results(false);
    int width  = state->screen_width * .8f;
    int height = state->screen_height * .4f;
    int x = (state->screen_width  - width) / 2;
    int y = (state->screen_height - height) / 2;
    int tx = state->screen_width * .15f;
    int ty = state->screen_height * .55f;
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
    eglSwapBuffers(state->display, state->surface);

    if(error)
    {
        dumpKb();
        readKb();
        dumpKb();
        redraw_results(true);
    }
}
//------------------------------------------------------------------------------
void show_youtube_formats()
{
    int key;
    int numFormatSave = numFormat;

    do
    {
        //redraw_results(false);
        clear_screen(false);
        draw_txt_box("Youtube supported video formats:", .95f, .95f, .025, .10f, .92f, 40, false);

        int step = state->screen_height / (AFORMAT_HEIGHT + 5);
        Roundrect(.085f * state->screen_width,
                  (AFORMAT_HEIGHT - numFormat) * step - (step / 3.0f),
                  state->screen_width * .75f,
                  state->screen_height / (AFORMAT_HEIGHT + 4),
                  20, 20, 5, rectColor, selectedColor);

        int x, y;
        for (x = 0; x < AFORMAT_WIDTH; x++)
            for(y = 0; y < AFORMAT_HEIGHT; y++)
                Text_DejaVuSans((x+1) * (state->screen_width /  (AFORMAT_WIDTH  + 2)),
                                (y+2) * step,
                                supported_formats[(AFORMAT_HEIGHT - 1) -y][x], 16, textColor);


        eglSwapBuffers(state->display, state->surface);

        key = toupper(readKb());
        switch (key)
        {
        case 'A' :
            if (numFormat > 0)
                numFormat--;
            break;

        case 'B' :
            if (numFormat < AFORMAT_HEIGHT - 2)
                numFormat++;
            break;
        }
    }
    while (key != 'Q' && key != RTN_KEY && (key != ESC_KEY || kbHit()));
    if(key == 'Q' || key == ESC_KEY)
        numFormat = numFormatSave;

    redraw_results(true);
    dumpKb();
}

//------------------------------------------------------------------------------

bool kbHit(void)
{
    struct termios oldt, newt;
    int ch;
    int oldf;
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
    fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);
    ch = getchar();
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    fcntl(STDIN_FILENO, F_SETFL, oldf);
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
    while(kbHit())
       readKb();
}
//------------------------------------------------------------------------------
int readKb()
{
    int ch;
    struct termios oldt;
    struct termios newt;
    tcgetattr(STDIN_FILENO, &oldt); 		// store old settings
    newt = oldt; 				// copy old settings to new settings 
    newt.c_lflag &= ~(ICANON | ECHO); 		// change settings 
    tcsetattr(STDIN_FILENO, TCSANOW, &newt); 	// apply the new settings immediatly 
    ch = getchar(); 				// standard getchar call 
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt); 	// reapply the old settings 
    return ch; 					// return received char 
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
VGImage load_jpeg(char * url, unsigned int * outputWidth, unsigned int * outputHeight)
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
                    vgImage = OMXCreateImageFromBuf(imageData, fileSize, 
                        (*outputWidth), (*outputHeight));
                break;
                
                case jdLibJpeg:
                    vgImage = createImageFromBuf(imageData, fileSize, (*outputHeight));     
                break;
                
                default:
                     show_message("ERROR:\n\nbad jped decoder enum", true, ERROR_POINT);
                break;
            }
            
            *outputWidth  = vgGetParameteri(vgImage, VG_IMAGE_WIDTH);
            *outputHeight = vgGetParameteri(vgImage, VG_IMAGE_HEIGHT);    
        
            
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
    int rectWidth2 = state->screen_width / 12;
    int jpegWidth = state->screen_width / 13;
    int txtXoffset = state->screen_width / 6;
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
                Text_DejaVuSans_Rollover(txtXoffset, y + halfStep, txtXmax, 2, txtYstep, temp->title, 24, selectedColor);
        }
        else
        {
            Roundrect(rectOffset, y, rectWidth, rectHeight, 20, 20, 2, rectColor, outlineColor);
            if(temp->title != NULL)
                Text_DejaVuSans_Rollover(txtXoffset, y + halfStep, txtXmax, 2, txtYstep, temp->title, 24, textColor);
        }

        Roundrect(rectOffset, y - rectDiff, rectWidth2, step, 20, 20, 2, rectColor2, outlineColor2);

        if(temp->thumbSmall != NULL)
        {
            if(temp->image == 0)
            {
                unsigned int outputWidth  = rectWidth2;
                unsigned int outputHeight = rectHeight;
                temp->image = load_jpeg(temp->thumbSmall, &outputWidth, &outputHeight);                
            }
            vgSetPixels(jpegOffset, y, temp->image, 0,0, jpegWidth, rectHeight);
        }
        temp = temp->next;
    }

    
    switch(videoPlayer)
    {
    case vpOMXPlayer:
        Text_DejaVuSans(0,state->screen_height * .98f, "[OMXPlayer]", 10, textColor);
        break;
    case vpMPlayer:
        Text_DejaVuSans(0, state->screen_height * .98f, "[MPlayer]", 10, textColor);
        break;
    }
    
    switch(jpegDecoder)
    {
    case jdOMX:
        Text_DejaVuSans(0,state->screen_height * .96f, "[OMXJPEG]", 10, textColor);
        break;
    case jdLibJpeg:
        Text_DejaVuSans(0, state->screen_height * .96f, "[LIBJPEG]", 10, textColor);
        break;
    }
    
    switch(soundOutput)
    {
    case soHDMI :
        Text_DejaVuSans(0, state->screen_height * .94, "(((HDMI)))", 12, textColor);
        break;
    case soLOCAL:
        Text_DejaVuSans(0, state->screen_height * .94, "(((LOCAL)))", 12, textColor);
        break;
    }

    if (numStart != 1)
    {
        char numStartStr[10];
        sprintf(numStartStr, "<-%d", numStart-1);
        Text_DejaVuSans(0, state->screen_height * .50f, numStartStr, 20, textColor);
    }

    if(swap)
        eglSwapBuffers(state->display, state->surface);
}
