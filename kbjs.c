#include <termios.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <linux/input.h>
#include "gfxlib.h"
#include "ui.h"
#include "kbjs.h"
#include "joystick.h"
#include "term.h"

//stuff for the keyboard.

static int ttflags;
static struct termios oldt;
static struct termios newt;
static int joystick_fd = -1;
static int mouse_fd = -1;
char jsDev[] = "/dev/input/js#";
char mouseDev[] = "/dev/input/event#";

int jsXAxis     = 0;
int jsYAxis     = 1;
int jsThreshold = 30000;
int jsSelect    = 2;
int jsBack      = 1;
int jsInfo      = 8;
int jsMenu      = 9;
bool mouseEnabled = true;
tPointXY mouseXY;
tPointXY clickXY;
int mouseBGImage = -1;
extern tFontDef fontDefs[];
extern int fontCount; 

int numMouseIndex    = 0;
int numJoystickIndex = 0;
int numPointerIndex  = 101;
int numPointerSize   = 90;
int numTimer         = -1;
int timerCount 	     = 0;

tPointXY pointerOffsetXY;
//------------------------------------------------------------------------------
int open_joystick()
{
    close_joystick();
    jsDev[sizeof(jsDev)-2] = '0' + (char) numJoystickIndex;
    joystick_fd = open(jsDev, O_RDONLY | O_NONBLOCK);
    if (joystick_fd < 0)
        return joystick_fd;
    return joystick_fd;
}
//------------------------------------------------------------------------------
int open_mouse()
{
    mouseDev[sizeof(mouseDev)-2] = '0' + (char) numMouseIndex;
    close_mouse();
    mouseXY.x = state->screen_width / 2;
    mouseXY.y = state->screen_height / 2;
    mouse_fd = open(mouseDev, O_RDONLY | O_NONBLOCK);
    if (mouse_fd < 0)
        return mouse_fd;
    return mouse_fd;
}

//------------------------------------------------------------------------------
void close_mouse()
{
    if (mouse_fd > 0)
        close(mouse_fd);
}

//------------------------------------------------------------------------------
bool read_mouse_event(struct input_event *mousee)
{
    int bytes;
    if(mouse_fd > 0)
    {
        bytes = read(mouse_fd, mousee, sizeof(struct input_event));
        if (bytes == -1)
            return false;
        if (bytes == sizeof(struct input_event))
            return true;
    }
    return false;
}
//------------------------------------------------------------------------------
bool read_joystick_event(struct js_event *jse)
{
    int bytes;
    if(joystick_fd > 0)
    {
        bytes = read(joystick_fd, jse, sizeof(*jse));
        if (bytes == -1)
            return false;
        if (bytes == sizeof(*jse))
            return true;
    }
    return false;
}

//------------------------------------------------------------------------------
void close_joystick(void)
{
    if(joystick_fd > 0)
        close(joystick_fd);
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
    ch = getchar();
    if(ch != EOF)
    {
        ungetc(ch, stdin);
        return true;
    }
    return false;
}

//------------------------------------------------------------------------------
bool jsESC(void)
{
    struct js_event jse;
    if(read_joystick_event(&jse))
    {
        if(jse.type == 1 && jse.value == 1 && jse.number == jsBack)
            return true;
    }
    return false;
}

//------------------------------------------------------------------------------
bool rbPressed(void)
{
    struct input_event mousee;
    if(read_mouse_event(&mousee))
    {
        if(mousee.type == 1 && mousee.code == 273 && mousee.value == 1)
            return true;
    }
    return false;
}
//------------------------------------------------------------------------------
inline void draw_mouse()
{
    char temp[14]; // (1920, 1024)"                
    snprintf(temp, sizeof(temp), "(%d, %d)", mouseXY.x, mouseXY.y);
    Text(&fontDefs[0], 0, 0, temp, numPointFontTiny, selectedColor, VG_FILL_PATH);
    int i;for (i=0;i<2;i++)
    Text_Char(&fontDefs[fontCount-1],
               mouseXY.x + pointerOffsetXY.x,
               mouseXY.y + pointerOffsetXY.y,
               numPointerIndex, 
               i==0?numPointerSize+5:numPointerSize-5, 
               1, &colorScheme[6-i],bgColor);
    //Roundrect(mouseXY.x, mouseXY.y,  10, 10, 20, 20, 1, rectColor, errorColor);
}
//------------------------------------------------------------------------------
inline bool handle_mouse(int * key)
{
    if(mouse_fd < 0) return false;
    struct input_event mousee;
    tPointXY oldMouseXY;
    memcpy(&oldMouseXY, &mouseXY, sizeof(tPointXY));
    while(read_mouse_event(&mousee))
    {
        switch(mousee.type)
        {
        case 1:
            switch (mousee.code)
            {
            case BTN_LEFT:
                if(mousee.value == 1)
                    *key = MOUSE_1;
                clickXY.x = mouseXY.x;
                clickXY.y = mouseXY.y;
                break;
            case BTN_RIGHT:
                if(mousee.value == 1)
                    *key = MOUSE_2;
                clickXY.x = mouseXY.x;
                clickXY.y = mouseXY.y;
                break;
            case 115: //BTN_FORWARD:
                *key = MOUSE_F;
                break;
            case 116: //BTN_BACK:
                *key = MOUSE_B;
                break;          
            }
            break;
        case 2:
            switch(mousee.code)
            {

            case 0 :
                if(mousee.value < 0)
                {
                    if(mouseXY.x + mousee.value > 0)
                        mouseXY.x += mousee.value;
                    else
                        mouseXY.x = 0;
                }
                if(mousee.value > 0)
                {
                    if(mouseXY.x + mousee.value < state->screen_width)
                        mouseXY.x += mousee.value;
                    else
                        mouseXY.y = state->screen_width;
                }
                break;
            case 1 :
                mousee.value = mousee.value * -1;
                if(mousee.value < 0)
                {
                    if(mouseXY.y + mousee.value > 0)
                        mouseXY.y += mousee.value;
                    else
                        mouseXY.y = 0;
                }
                if(mousee.value > 0)
                {
                    if(mouseXY.y + mousee.value < state->screen_height)
                        mouseXY.y += mousee.value;
                    else
                        mouseXY.y = state->screen_height;
                }
                break;
            }
            break;
        }
    }
    if (mouseBGImage != -1 && (oldMouseXY.x != mouseXY.x || oldMouseXY.y != mouseXY.y))
    {
        vgSetPixels(0,0, mouseBGImage, 0, 0, state->screen_width, state->screen_height);
        draw_mouse();
        eglSwapBuffers(state->display, state->surface);
        return true;
    }
    return false;
}

//------------------------------------------------------------------------------
void dumpMouse()
{
    struct input_event mousee;
    if(mouse_fd > 0)
    {
        while(read_mouse_event(&mousee))
        {
            // :)
        }
    }
}
//------------------------------------------------------------------------------
inline int readKb_loop(bool checkMouse)
{
    struct js_event jse;
    int key;
    do
    {   //timer is high prioiry...
        if(numTimer > 0)
        {
            if (++timerCount > numTimer)
            {
                timerCount = 0;
                return TIMER_M;
            }
        }
        else
            timerCount = 0;

        if(read_joystick_event(&jse))
        {
            switch(jse.type)
            {
            case 2:
                if (jse.number == jsXAxis)
                {
                    if (jse.value >= jsThreshold)
                        return CUR_R;
                    else if(jse.value <= -jsThreshold)
                        return CUR_L;
                }
                else if(jse.number ==  jsYAxis)
                {
                    if (jse.value >= jsThreshold)
                        return CUR_DWN;
                    else if(jse.value <= -jsThreshold)
                        return CUR_UP;
                }
                break;

            case 1:
                if(jse.value == 1)
                {
                    if (jse.number == jsBack)
                        return ESC_KEY;
                    else if (jse.number == jsSelect)
                        return JOY_1;
                    else if (jse.number == jsInfo)
                        return 'I';
                    else if (jse.number == jsMenu)
                        return 'M';
                }
                break;
            }
        }

        key = getchar();
        if(key == EOF && checkMouse && mouseEnabled)
           handle_mouse(&key);
        if(key == EOF)
            usleep(1000);
    }
    while (key == EOF);

    if (key == ESC_KEY)
    {
        key = handleESC();
    }
    return key;
}
//------------------------------------------------------------------------------
int readKb(void) //legacy function
{
    dumpKb();
    dumpJs();
    eglSwapBuffers(state->display, state->surface);
    return readKb_loop(false);
}

//------------------------------------------------------------------------------
int readKb_mouse(void)
{
    dumpKb();
    dumpJs();
    if (mouseEnabled && (mouse_fd > 0))
    {
        if(mouseBGImage > 0)
            vgGetPixels(mouseBGImage, 0,0, 0,0,state->screen_width, state->screen_height);
        else
            mouseBGImage = createImageFromScreen();
        draw_mouse();
    }
    eglSwapBuffers(state->display, state->surface);
    int key = readKb_loop((mouse_fd > 0)?mouseEnabled:false);
    return key;
}
//------------------------------------------------------------------------------
void dumpJs(void)
{
    if(joystick_fd > 0)
    {

        struct js_event jse;
        while (read_joystick_event(&jse))
        {
            //dumping joystick
        }
    }
}
//------------------------------------------------------------------------------
void dumpKb(void)
{
    while (getchar()!= EOF)
    {
        // dumping kb :)
    }

}
//------------------------------------------------------------------------------
void initKb(void)
{
    tcgetattr(STDIN_FILENO, &oldt); 		// store old settings
    newt = oldt; 				// copy old settings to new settings
    newt.c_lflag &= ~(ICANON | ECHO); 		// change settings
    tcsetattr(STDIN_FILENO, TCSANOW, &newt); 	// apply the new settings immediatly
    ttflags = fcntl(STDIN_FILENO, F_GETFL, 0);
//  load_sample(&asiKbClick, (uint8_t *) soundraw_data, soundraw_size, 8000, 16, 1, 1);
    fcntl(STDIN_FILENO, F_SETFL, ttflags | O_NONBLOCK);
//    open_joystick();
//    open_mouse();
    pointerOffsetXY.x = -7;
    pointerOffsetXY.y = -42;
}
//------------------------------------------------------------------------------
void restoreKb(void)
{
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt); 	// reapply the old settings
//  delete_sample(&asiKbClick);
    close_joystick();
    close_mouse();
    if(mouseBGImage != -1)
        vgDestroyImage(mouseBGImage);

}

//------------------------------------------------------------------------------
void do_joystick_test(void)
{
    tTermState ts;
    term_init(&ts, .70f, .95f, -1, -1);
    term_set_color(&ts, 7);
    term_put_str(&ts, "test:");
    term_put_str(&ts, jsDev);
    term_put_str(&ts, "\n");
    term_show(&ts, true);
    term_set_color(&ts, 5);
    char format[] =  "event: time %8u, value %8hd, type: %3u, axis/button: %u\n";
    size_t size = strlen(format) + 50;
    char * txt = malloc(size);
    int key = 0;
    if(joystick_fd > 0)
    {
        dumpJs();
        term_set_color(&ts, 0);
        struct js_event jse;
        do
        {
            if(read_joystick_event(&jse))
            {
                snprintf(txt, size, format, jse.time, jse.value, jse.type, jse.number);
                term_put_str(&ts, txt);
                term_show(&ts, true);
            }
            else
                usleep(1000);

            key = getchar();
            if (key == ESC_KEY)
                key = handleESC();
        }
        while (key != ESC_KEY);
    }
    else
    {
        snprintf(txt, size, "%s was not opened.", jsDev);
        term_put_str(&ts, txt);
        term_set_color(&ts, 5);
        term_put_str(&ts, "\n\n\nPress any key to continue...");
        term_show(&ts, true);
        readKb();
    }
    free(txt);
    term_free(&ts);
}


