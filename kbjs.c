#include <termios.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include "kbjs.h"
#include "gfxlib.h"
#include "ui.h"
#include "joystick.h"

//stuff for the keyboard.

static int ttflags;
static struct termios oldt;
static struct termios newt;
static int joystick_fd = -1;

//------------------------------------------------------------------------------
int open_joystick(char *joystick_device)
{
    joystick_fd = open(joystick_device, O_RDONLY | O_NONBLOCK); /* read write for force feedback? */
    if (joystick_fd < 0)
        return joystick_fd;
    /* maybe ioctls to interrogate features here? */
    return joystick_fd;
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
bool jsESC(void)
{
    struct js_event jse;
    if(read_joystick_event(&jse))
    {
        if(jse.type == 1 && jse.value == 1 && jse.number == 1)
            return true;
    }
    return false;
}

//------------------------------------------------------------------------------
bool jsRTN;
bool kbRTN;

int readKb(void)
{
    struct js_event jse;
    int key;
    dumpKb();
    dumpJs();
    jsRTN = false;
    kbRTN = false;
    
    #define JS_THRESHOLD 30000
    do
    {
        if(read_joystick_event(&jse))
        {
            //printf("Event: time %8u, value %8hd, type: %3u, axis/button: %u\n",
            //       jse.time, jse.value, jse.type, jse.number);

            switch(jse.type)
            {
            case 2:
                switch(jse.number)
                {
                case 0:
                case 5:
                    if (jse.value > JS_THRESHOLD)
                        return CUR_R;
                    else if(jse.value < -JS_THRESHOLD)
                        return CUR_L;
                case 1:
                case 6:
                    if (jse.value > JS_THRESHOLD)
                        return CUR_DWN;
                    else if(jse.value < -JS_THRESHOLD)
                        return CUR_UP;
                }
                break;

            case 1:
                if(jse.value == 1)
                {
                    switch(jse.number)
                    {
                    case 1:
                        return ESC_KEY;
                    case 2:
                        jsRTN = true;
                        return RTN_KEY;
                    case 3:
                    case 8:
                        return 'i';
                    case 4:
                    case 9:
                        return 'm';
                    }
                }
                break;
            }
        }

        key = getchar();
        usleep(1000);
    } while (key == EOF);

    if (key == ESC_KEY)
    {
        //fcntl(STDIN_FILENO, F_SETFL, ttflags | O_NONBLOCK);
        key = handleESC();
        // fcntl(STDIN_FILENO, F_SETFL, ttflags & ~O_NONBLOCK);
    }
    else if(key == RTN_KEY)
        kbRTN = true;
    //else play_sample(&asiKbClick, false);
    return key;
}

//------------------------------------------------------------------------------
void dumpJs(void)
{
    if(joystick_fd > 0)
    {
        struct js_event jse;
        while(read_joystick_event(&jse))
        {    
        //dumping joystick...
        }
    }
}
//------------------------------------------------------------------------------
void dumpKb(void)
{
    //fcntl(STDIN_FILENO, F_SETFL, ttflags | O_NONBLOCK);
    while (getchar()!= EOF)
    {
        // dumping kb :)
    }
    //fcntl(STDIN_FILENO, F_SETFL, ttflags & ~O_NONBLOCK);   

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
    open_joystick("/dev/input/js0");

}
//------------------------------------------------------------------------------
void restoreKb(void)
{
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt); 	// reapply the old settings
//  delete_sample(&asiKbClick);
    close_joystick();
}



