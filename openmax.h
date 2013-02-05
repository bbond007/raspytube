/***************************************************************************
*            openmax.h
*
* Written by Anthong Sale Sept 2012
* Based on an original by Matt Ownby, August 2012
* You are free to use this for educational/non-commercial use
****************************************************************************/

#ifndef _OPTION_H_
#define _OPTION_H_

/*
      Defines the methods for interacting with openmax il and ilclient to decode
      jpeg images from the camera
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdbool.h>

#include "bcm_host.h"
#include "ilclient.h"

#define OMXJPEG_OK                  0
#define OMXJPEG_ERROR_ILCLIENT_INIT    -1024
#define OMXJPEG_ERROR_OMX_INIT         -1025
#define OMXJPEG_ERROR_MEMORY         -1026
#define OMXJPEG_ERROR_CREATING_COMP    -1027
#define OMXJPEG_ERROR_WRONG_NO_PORTS   -1028
#define OMXJPEG_ERROR_EXECUTING         -1029
#define OMXJPEG_ERROR_NOSETTINGS   -1030

typedef struct _COMPONENT_DETAILS
{
    COMPONENT_T* component;
    OMX_HANDLETYPE handle;
    int inPort;
    int outPort;
} COMPONENT_DETAILS;


struct _OPENMAX_JPEG_DECODER
{
    ILCLIENT_T* client;
    COMPONENT_DETAILS* imageDecoder;
    COMPONENT_DETAILS* imageResizer;
    OMX_BUFFERHEADERTYPE** ppInputBufferHeader;
    int inputBufferHeaderCount;
    OMX_BUFFERHEADERTYPE* pOutputBufferHeader;
};

typedef struct _OPENMAX_JPEG_DECODER OPENMAX_JPEG_DECODER;

//this function run the boilerplate to setup the openmax components;
int OpenMaxJPEG_setupDecoder(OPENMAX_JPEG_DECODER** decoder);

//this function passed the jpeg image buffer in, and returns the decoded image
int OpenMaxJPEG_decodeImage(OPENMAX_JPEG_DECODER* decoder,
                char* sourceImage, size_t imageSize, size_t outputWidth, size_t outputHeight);

//this function cleans up the decoder.
void OpenMaxJPEG_cleanup(OPENMAX_JPEG_DECODER* decoder);
void OpenMaxJPEG_setDebugLevel(bool value);
#endif
