#include <fcntl.h>
#include <stdio.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include "gfxlib.h"
#include "ui.h"
#include "kbjs.h"
#include "term.h"
#include "GFXDATA/boing.h"

extern const char tv_jpeg_raw_data[];
extern const unsigned int tv_jpeg_raw_size;
extern tFontDef fontDefs[];
extern tColorDef colorScheme[];
//------------------------------------------------------------------------------
void term_clear(tTermState * ts, char c)
{
    memset(ts->term_vbuff, c, ts->term_w * ts->term_h * sizeof(tScrChr));
    ts->term_cur_x = 0;
    ts->term_cur_y = 0;
}
//------------------------------------------------------------------------------
void term_scroll(tTermState * ts)
{
    int y = 0;
    tScrChr * dst = &ts->term_vbuff[0];
    tScrChr * src = &ts->term_vbuff[ts->term_w];
    size_t size = ts->term_w * sizeof(tScrChr);
    for (y = 0; y < ts->term_h - 1; y++)
    {
        memcpy(dst, src, size);
        dst = src;
        src+= ts->term_w;
    }
    memset(dst,VTERM_CLR, size);
}
//------------------------------------------------------------------------------
void term_putc(tTermState * ts, char c)
{
    int pos = ts->term_cur_x + (ts->term_cur_y * ts->term_w);
    ts->term_vbuff[pos].c = c;
    ts->term_vbuff[pos].color = ts->term_color;
    ts->term_cur_x++;

    if(ts->term_cur_x >= ts->term_w || c == RTN_KEY)
    {
        ts->term_cur_x = 0;
        if(ts->term_cur_y < ts->term_h -1)
            ts->term_cur_y++;
        else
            term_scroll(ts);
    }
}
//------------------------------------------------------------------------------
void term_putc_xy(tTermState * ts, int x, int y, char c, unsigned char color)
{
    if (x < 0)
        x = ts->term_w - 1;
    if (y < 0)
        y = ts->term_h - 1;
    int pos = x + y * ts->term_w;
    ts->term_vbuff[pos].c = c;
    ts->term_vbuff[pos].color = color;
}
//------------------------------------------------------------------------------
void term_goto_xy(tTermState * ts, int x, int y)
{
    ts->term_cur_x = x;
    ts->term_cur_y = y;
}
//------------------------------------------------------------------------------
void term_del(tTermState * ts)
{
    if(ts->term_cur_x > 0)
        ts->term_cur_x--;
    else if(ts->term_cur_y > 0)
    {
        ts->term_cur_y--;
        ts->term_cur_x = (ts->term_w  - 1);
    }
    int pos = ts->term_cur_x + ts->term_cur_y * ts->term_w;
    ts->term_vbuff[pos].c = VTERM_CLR;
    ts->term_vbuff[pos].color = VTERM_CLR;
}
//------------------------------------------------------------------------------
void term_put_str(tTermState * ts, char * str)
{
    while(*str != 0x00)
    {
        if(*str == DEL_KEY)
            term_del(ts);
        else if(*str == '\r')
            ts->term_cur_x = 0;
        else
            term_putc(ts, *str);
        str++;
    }
}

//------------------------------------------------------------------------------
void term_put_c(tTermState * ts, char c)
{
    if(c == DEL_KEY)
        term_del(ts);
    else if(c == '\r')
       ts->term_cur_x = 0;
    else
       term_putc(ts, c);
}
//------------------------------------------------------------------------------
void term_init(tTermState * ts, float widthPer, float heightPer, int width, int height)
{
    ts->image = 0;
    if(ts->image == 0)
    {
        if(width <= 0 || height <= 0)
        {
            ts->term_w = VTERM_WIDTH;
            ts->term_h = VTERM_HEIGHT;
        }
        else
        {
            ts->term_w = width;
            ts->term_h = height;
        }
        ts->term_vbuff = malloc(ts->term_w * ts->term_h * sizeof(tScrChr));
        int w  = (state->screen_width  * widthPer);
        int h  = (state->screen_height * heightPer);
        ts->image = create_image_from_buf((unsigned char *) tv_jpeg_raw_data, tv_jpeg_raw_size, w, h);
        term_clear(ts, VTERM_CLR);
        ts->tv_width     = vgGetParameteri(ts->image, VG_IMAGE_WIDTH);
        ts->tv_height    = vgGetParameteri(ts->image, VG_IMAGE_HEIGHT);
        ts->offsetXY.y   = (ts->tv_height * .14f);
        ts->offsetXY.x   = (ts->tv_width  * .10f);
        ts->tvXY.x       = (state->screen_width  - ts->tv_width) / 2;
        ts->tvXY.y       = (state->screen_height - ts->tv_height) / 2;
        ts->image_height = ts->tv_height - (ts->offsetXY.y * 2);
        ts->image_width  = ts->tv_width  - (ts->offsetXY.x * 2);
        ts->imageXY.x    = (state->screen_width - ts->image_width)   / 2;
        ts->imageXY.y    = (state->screen_height - ts->image_height) / 2;
        ts->term_x_inc   = ts->image_width  / ts->term_w;
        ts->term_y_inc   = ts->image_height / ts->term_h;
        ts->txtXY.x      = ts->imageXY.x + ts->term_x_inc;
        ts->txtXY.y      = state->screen_height - ts->imageXY.y - ts->term_y_inc * 1.5;
        ts->numPointFont = numPointFontMed; //med default
        ts->numFont      = 1; //topaz font
        ts->term_color   = 0;
    }
}
//------------------------------------------------------------------------------
void term_set_color(tTermState * ts, unsigned char color)
{
    ts->term_color = color;
}
//------------------------------------------------------------------------------
void term_free(tTermState * ts)
{
    if(ts->image != 0)
    {
        vgDestroyImage(ts->image);
        ts->image = 0;
        free(ts->term_vbuff);
    }
}

//------------------------------------------------------------------------------
void term_show(tTermState * ts, bool swap)
{
    if (ts->image == 0) return;
    clear_screen(false);

    vgSetPixels(ts->tvXY.x,
                ts->tvXY.y,
                ts->image,
                0, 0,
                ts->tv_width,
                ts->tv_height);

    Roundrect(ts->imageXY.x,
              ts->imageXY.y,
              ts->image_width,
              ts->image_height,
              20, 20,
              numRectPenSize,
              rectColor,
              bgColor);

    int x, y;
    char temp[2];
    temp[1] = 0x00;
    tPointXY pos;
    pos.x = ts->txtXY.x;
    pos.y = ts->txtXY.y;
    tScrChr * pBuff = &ts->term_vbuff[0];
    for (y = 0; y < ts->term_h ; y++)
    {
        for(x = 0; x < ts->term_w ; x++)
        {
            temp[0] = pBuff->c;
            Text(&fontDefs[ts->numFont], //Topaz font
                 pos.x, pos.y, temp, ts->numPointFont, &colorScheme[pBuff->color], VG_FILL_PATH);
            pos.x += ts->term_x_inc;
            pBuff++;
        }
        pos.y -= ts->term_y_inc;
        pos.x = ts->txtXY.x;
    }
    if(swap)
     eglSwapBuffers(state->display, state->surface);
}
//------------------------------------------------------------------------------
void term_command(tTermState * ts, char * command)
{
    tPointPer boingPer;
    tPointPer boingSizePer;
    tPointXY  boingXY;
    tPointXY  boingSize;
    boingPer.xPer = .50f;
    boingPer.yPer = .20f;
    calc_point_xy(&boingPer, &boingXY);
    boingSizePer.xPer = 0.18f;   
    boingSizePer.yPer = 0.30f;
    calc_point_xy(&boingSizePer, &boingSize);
    //boingSize.x = (int)((boingSize.x / 16)) * 16;
    boingXY.x = (state->screen_width - boingSize.x) / 2;
    char buf[1024];
    FILE * pipe = popen(command,"r");
    if(pipe == NULL)
    {
        snprintf(buf, sizeof(buf), "invoking:%s failed:%s\n",command,strerror(errno));
        term_put_str(ts, buf);
        term_show(ts, true);
        readKb();
    }
    int pipefd = fileno(pipe);
    int flags = fcntl(pipefd, F_GETFL, 0);
    int key = 0x00;
    flags |= O_NONBLOCK;
    fcntl(pipefd, F_SETFL, flags);
    while(!feof(pipe))
    {
        if(fgets(buf,sizeof(buf),pipe)!=0)
            term_put_str(ts, buf);
        else
            usleep(500);
        term_show(ts, false);
        draw_boing(boingXY.x, boingXY.y, boingSize.x, boingSize.y, true);
        eglSwapBuffers(state->display, state->surface);
        if (bQScreen)
            x_window_loop(&key, false);
    }
    pclose(pipe);
}