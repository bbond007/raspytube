#include <stdio.h>
#include <stdbool.h>
#include "EGL/egl.h"
#include "GLES/gl.h"
#include "VG/openvg.h"
#include "VG/vgu.h"
#include "gfxlib.h"
#include "ui.h";
#include "config.h"

#define VERSION_NUMBER 11
#define CONFIG_FILE "~.raspytube.cfg.bin"
typedef struct tConfigRec
{
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
void loadConfig()
{
      FILE * cfgFile;      
      cfgFile = fopen(CONFIG_FILE, "rb");
      if (cfgFile == NULL)
      {	
          //show_message("~5LoadConfig():failed", true, ERROR_POINT);
          return;
      }
      tConfigRec configRec;
      size_t bytesRead = fread(&configRec, 1, sizeof(tConfigRec), cfgFile);
      fclose(cfgFile);
      if(bytesRead != sizeof(tConfigRec) || configRec.numVersion != VERSION_NUMBER)
      {
          show_message("~5LoadConfig():failed->file corrupt", true, ERROR_POINT);
          return;
      }
       
     numPointFontTiny 		     = configRec.numPointFontTiny;
     numPointFontSmall               = configRec.numPointFontSmall;
     numPointFontMed                 = configRec.numPointFontMed;
     numPointFontLarge               = configRec.numPointFontLarge;
     numThumbWidth                   = configRec.numThumbWidth;
     numResults                      = configRec.numResults;   
     numFormat                       = configRec.numFormat;  
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
     configRec.numVersion            = VERSION_NUMBER;
     
     FILE * cfgFile;      
     cfgFile = fopen(CONFIG_FILE, "wb");
     if (cfgFile == NULL)
     {	
          show_message("~5SaveConfig() -> failed", true, ERROR_POINT);
          return;
     }
     size_t bytesWritten = fwrite(&configRec, 1, sizeof(tConfigRec), cfgFile);
     fclose(cfgFile);
     if(bytesWritten != sizeof(tConfigRec))
     {	
          show_message("~5SaveConfig() -> fwrite failed", true, ERROR_POINT);
          return;
     }
     show_message("~3Config file saved...\n\n~5press [ESC]", false, ERROR_POINT);
     dumpKb();
     readKb();
}