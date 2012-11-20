// Written by Matt Ownby, August 2012
// You are free to use this for educational/non-commercial use

#include <stdio.h>
#include <stdbool.h>
#include "Logger.h"
#include "OMXJPEG.h"
#include "VG/openvg.h"
#include "VG/vgu.h"

static Logger logstdout;
static ILogger *pLogger = &logstdout; 
static OMXJPEG  OMXjpeg(pLogger);
    
extern "C" VGImage OMXCreateImageFromBuf(unsigned char * buf, unsigned int bufLength, unsigned int outputWidth, unsigned int outputHeight)
{	
    return OMXjpeg.CreateImageFromBuf(buf, bufLength, outputWidth, outputHeight);
}


