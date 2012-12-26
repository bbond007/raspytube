#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>
#include "EGL/egl.h"
#include "GLES/gl.h"
#include "VG/openvg.h"
#include "VG/vgu.h"
#include "gfxlib.h"
#include "ui.h"
#include "config.h"
#include "kbjs.h"

#define VERSION_NUMBER 0x00068000
#define CONFIG_FILE ".rt.cfg.bin"
typedef struct tConfigRec
{
    tJpegDecoder jpegDecoder;
    tVideoPlayer videoPlayer;
    tSoundOutput soundOutput;
    int font;
    int titleFont;
    int numPointFontTiny_QS;
    int numPointFontSmall_QS;
    int numPointFontMed_QS;
    int numPointFontLarge_QS;
    int numFontSpacing_QS;
    int numPointFontTiny_FS;
    int numPointFontSmall_FS;
    int numPointFontMed_FS;
    int numPointFontLarge_FS;
    int numFontSpacing_FS;
    int numThumbWidth;
    int numRow;
    int numCol;
    int numFormat;
    bool mouseEnabled;
    int jsXAxis;
    int jsYAxis;
    int jsThreshold;
    int jsSelect;
    int jsBack;
    int jsInfo;
    int jsMenu;
    int numMouseIndex;
    int numJoystickIndex;
    int numPointerIndex;
    int numPointerSize_FS;
    int numPointerSize_QS;
    tPointXY pointerOffsetXY_FS;
    tPointXY pointerOffsetXY_QS;
    int numVersion;
} tConfigRec;

tConfigRec configRec;
extern bool bQScreen;

//----------------------------------------------------------------------------
char * getFileName()
{
    char formatStr[] = "%s/%s";
    struct passwd * pw = getpwuid(getuid());
    size_t size = strlen(formatStr) + strlen(CONFIG_FILE) + strlen(pw->pw_dir);
    char * fileName = malloc(size);
    snprintf(fileName, size, formatStr, pw->pw_dir, CONFIG_FILE);
    return fileName;
}

//----------------------------------------------------------------------------
char * setMessage(char * message, char * fileName)
{
    char formatStr[] = "%s~0\nfilename = %s";
    size_t size = strlen(fileName) + strlen(formatStr) + strlen(message);
    char * tempStr = malloc(size);
    snprintf(tempStr, size, formatStr, message, fileName);
    return tempStr;
}
//----------------------------------------------------------------------------
void setRezSpecific()
{
    if (bQScreen)
    {
        numPointFontTiny     = configRec.numPointFontTiny_QS;
        numPointFontSmall    = configRec.numPointFontSmall_QS;
        numPointFontMed      = configRec.numPointFontMed_QS;
        numPointFontLarge    = configRec.numPointFontLarge_QS;
        numFontSpacing	     = configRec.numFontSpacing_QS;
        numPointerSize       = configRec.numPointerSize_QS;
        memcpy(&pointerOffsetXY, &configRec.pointerOffsetXY_QS, sizeof(tPointXY));        
    }
    else
    {
        numPointFontTiny     = configRec.numPointFontTiny_FS;
        numPointFontSmall    = configRec.numPointFontSmall_FS;
        numPointFontMed      = configRec.numPointFontMed_FS;
        numPointFontLarge    = configRec.numPointFontLarge_FS;
        numFontSpacing	     = configRec.numFontSpacing_FS;
        numPointerSize       = configRec.numPointerSize_FS;
        memcpy(&pointerOffsetXY, &configRec.pointerOffsetXY_FS, sizeof(tPointXY));
    }   
}
//----------------------------------------------------------------------------
void saveRezSpecific()
{
    if(bQScreen)
    {
        memcpy(&configRec.pointerOffsetXY_QS, &pointerOffsetXY, sizeof(tPointXY));        
        configRec.numPointerSize_QS     = numPointerSize;
        configRec.numPointFontTiny_QS   = numPointFontTiny;
        configRec.numPointFontSmall_QS  = numPointFontSmall;
        configRec.numPointFontMed_QS    = numPointFontMed;
        configRec.numPointFontLarge_QS  = numPointFontLarge;
        configRec.numFontSpacing_QS     = numFontSpacing;
    }
    else
    {
        memcpy(&configRec.pointerOffsetXY_FS, &pointerOffsetXY, sizeof(tPointXY));
        configRec.numPointerSize_FS     = numPointerSize;
        configRec.numPointFontTiny_FS   = numPointFontTiny;
        configRec.numPointFontSmall_FS  = numPointFontSmall;
        configRec.numPointFontMed_FS    = numPointFontMed;
        configRec.numPointFontLarge_FS  = numPointFontLarge;
        configRec.numFontSpacing_FS     = numFontSpacing;
    }
}
//----------------------------------------------------------------------------
void setUIDefaults();
bool loadConfig()
{
    setUIDefaults();
    FILE * cfgFile;
    char * fileName = getFileName();
    cfgFile = fopen(fileName, "rb");
    
    if (cfgFile == NULL)
    {
        //show_message("~5LoadConfig():failed", true, ERROR_POINT);
        free(fileName);
        return false;
    }
    size_t bytesRead = fread(&configRec, 1, sizeof(tConfigRec), cfgFile);
    fclose(cfgFile);
    if(bytesRead != sizeof(tConfigRec) || configRec.numVersion != VERSION_NUMBER)
    {
        char * message = setMessage("FILE CORRUPT", fileName);
        show_message(message, true, ERROR_POINT);
        free(message);
        free(fileName);
        return false;
    }
    setRezSpecific();
    free(fileName);
    numThumbWidth                    = configRec.numThumbWidth;
    numRow                           = configRec.numRow;
    numCol                           = configRec.numCol;
    numResults			     = configRec.numRow * configRec.numCol;
    jpegDecoder 		     = configRec.jpegDecoder;
    videoPlayer 		     = configRec.videoPlayer;
    soundOutput		             = configRec.soundOutput;
    numFormat                        = configRec.numFormat;
    jsXAxis			     = configRec.jsXAxis;
    jsYAxis			     = configRec.jsYAxis;
    jsThreshold		             = configRec.jsThreshold;
    jsSelect		     	     = configRec.jsSelect;
    jsBack		     	     = configRec.jsBack;
    jsInfo		     	     = configRec.jsInfo;
    jsMenu		     	     = configRec.jsMenu;
    mouseEnabled	             = configRec.mouseEnabled;
    numJoystickIndex                 = configRec.numJoystickIndex;
    numMouseIndex                    = configRec.numMouseIndex;
    numPointerIndex 		     = configRec.numPointerIndex;
    set_font(configRec.font);
    set_title_font(configRec.titleFont);
    return true;
}

//----------------------------------------------------------------------------
void saveConfig()
{
    saveRezSpecific();
    configRec.numThumbWidth          = numThumbWidth;
    configRec.numRow	             = numRow;
    configRec.numCol	             = numCol;
    configRec.numFormat              = numFormat;
    configRec.jpegDecoder            = jpegDecoder;
    configRec.videoPlayer            = videoPlayer;
    configRec.soundOutput            = soundOutput;
    configRec.font		     = get_font();
    configRec.titleFont	             = get_title_font();
    configRec.jsXAxis		     = jsXAxis;
    configRec.jsYAxis		     = jsYAxis;
    configRec.jsThreshold	     = jsThreshold;
    configRec.jsSelect		     = jsSelect;
    configRec.jsBack	     	     = jsBack;
    configRec.jsInfo	     	     = jsInfo;
    configRec.jsMenu	 	     = jsMenu;
    configRec.mouseEnabled	     = mouseEnabled;
    configRec.numJoystickIndex       = numJoystickIndex;
    configRec.numMouseIndex          = numMouseIndex;
    configRec.numPointerIndex 	     = numPointerIndex;
    configRec.numVersion             = VERSION_NUMBER;

    FILE * cfgFile;
    char * fileName = getFileName();
    char * message = NULL;
    cfgFile = fopen(fileName, "wb");
    if (cfgFile == NULL)
    {
        message = setMessage("saveConfig failed", fileName);
        free(fileName);
        show_message(message, true, ERROR_POINT);
        free(message);
        return;
    }
    size_t bytesWritten = fwrite(&configRec, 1, sizeof(tConfigRec), cfgFile);
    fclose(cfgFile);
    if(bytesWritten != sizeof(tConfigRec))
    {
        message = setMessage("saveConfig fwrite failed", fileName);
        show_message(message, true, ERROR_POINT);
        free(message);
        return;
    }

    message = setMessage("Config file saved...\n\n~5press any key...", fileName);
    free(fileName);
    show_message(message,  false, ERROR_POINT);
    free(message);
    dumpKb();
    readKb_mouse();
}

//----------------------------------------------------------------------------
void setUIDefaults()
{
/*
    printf("setUIDefaults(%d, %d):bQScreen:%s\n", 
            state->screen_width,
            state->screen_height,
            bQScreen?"TRUE":"FALSE");
*/
    if(state->screen_width >= 1920)
    {
        configRec.numPointFontTiny_FS  = 10;
        configRec.numPointFontSmall_FS = 12;
        configRec.numPointFontMed_FS   = 18;
        configRec.numPointFontLarge_FS = 40;
    }
    else if (state->screen_width >= 1280)
    {
        configRec.numPointFontTiny_FS  = 7;
        configRec.numPointFontSmall_FS = 9;
        configRec.numPointFontMed_FS   = 13;
        configRec.numPointFontLarge_FS = 30;
    }
    else
    {
        configRec.numPointFontTiny_FS  = 5;
        configRec.numPointFontSmall_FS = 6;
        configRec.numPointFontMed_FS   = 12;
        configRec.numPointFontLarge_FS = 25;
    }

    configRec.numPointFontTiny_QS    = configRec.numPointFontTiny_FS  / 2;
    configRec.numPointFontSmall_QS   = configRec.numPointFontSmall_FS / 2;
    configRec.numPointFontMed_QS     = configRec.numPointFontMed_FS   / 2;
    configRec.numPointFontLarge_QS   = configRec.numPointFontLarge_FS / 2;
    configRec.numFontSpacing_QS      = 24;
    configRec.numFontSpacing_FS      = 24;
    configRec.numPointerSize_FS      = 90;
    configRec.pointerOffsetXY_FS.x   = -7;
    configRec.pointerOffsetXY_FS.y   = -84;
    configRec.numPointerSize_QS      = 45;
    configRec.pointerOffsetXY_QS.x   = -4;
    configRec.pointerOffsetXY_QS.y   = -42;
    setRezSpecific();
    numThumbWidth     = 10;
    numRow            = 6;
    numCol            = 3;
    numResults        = numRow * numCol;
    numFormat         = 0;
    numStart          = 1;
    numJoystickIndex  = 0;
    numPointerIndex   = 101;
    numMouseIndex     = 0; 
    soundOutput       = soHDMI;
    videoPlayer       = vpOMXPlayer;
    jpegDecoder       = jdLibJpeg;
    
}