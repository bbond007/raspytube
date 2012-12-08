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
#include "ui.h";
#include "config.h"

#define VERSION_NUMBER 14
#define CONFIG_FILE ".rt.cfg.bin"
typedef struct tConfigRec
{
      tJpegDecoder jpegDecoder;
      tVideoPlayer videoPlayer;
      tSoundOutput soundOutput;
      int font;
      int titleFont;
      int numPointFontTiny;
      int numPointFontSmall;
      int numPointFontMed;
      int numPointFontLarge;
      int numThumbWidth;
      int numResults;
      int numFormat;
      int numVersion;
} tConfigRec;

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
bool loadConfig()
{
      FILE * cfgFile; 
      char * fileName = getFileName();     
      cfgFile = fopen(fileName, "rb");
      free(fileName);
      if (cfgFile == NULL)
      {	
          //show_message("~5LoadConfig():failed", true, ERROR_POINT);
          return false;
      }
      tConfigRec configRec;
      size_t bytesRead = fread(&configRec, 1, sizeof(tConfigRec), cfgFile);
      fclose(cfgFile);
      if(bytesRead != sizeof(tConfigRec) || configRec.numVersion != VERSION_NUMBER)
      {
          show_message("~5LoadConfig():failed->file corrupt", true, ERROR_POINT);
          return false;
      }
       
     numPointFontTiny 		     = configRec.numPointFontTiny;
     numPointFontSmall               = configRec.numPointFontSmall;
     numPointFontMed                 = configRec.numPointFontMed;
     numPointFontLarge               = configRec.numPointFontLarge;
     numThumbWidth                   = configRec.numThumbWidth;
     numResults                      = configRec.numResults;   
     jpegDecoder 		     = configRec.jpegDecoder;
     videoPlayer 		     = configRec.videoPlayer;
     soundOutput		     = configRec.soundOutput;
     numFormat                       = configRec.numFormat; 
     set_font(configRec.font);
     set_title_font(configRec.titleFont);
          
     return true;
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
void saveConfig()
{
     tConfigRec configRec;
     configRec.numPointFontTiny      = numPointFontTiny;
     configRec.numPointFontSmall     = numPointFontSmall;
     configRec.numPointFontMed       = numPointFontMed;  
     configRec.numPointFontLarge     = numPointFontLarge;    
     configRec.numThumbWidth         = numThumbWidth;              
     configRec.numResults            = numResults;                          
     configRec.numFormat             = numFormat;
     configRec.jpegDecoder           = jpegDecoder;
     configRec.videoPlayer           = videoPlayer;
     configRec.soundOutput           = soundOutput; 
     configRec.font		     = get_font();
     configRec.titleFont	     = get_title_font();
     configRec.numVersion            = VERSION_NUMBER;
     
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
     
     message = setMessage("Config file saved...\n\n~5press [ESC]", fileName);
     free(fileName);
     show_message(message,  false, ERROR_POINT);
     free(message);
     dumpKb();
     readKb();
}