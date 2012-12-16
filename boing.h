#include <stdbool.h>
#include "VG/openvg.h"
#include "VG/vgu.h" 


typedef struct tAnimFrame
{
    VGImage image;
    const char * rawData;
    const unsigned int * rawDataSize;
} tAnimFrame;

extern tAnimFrame boingAnim[];
bool draw_boing(int x, int y, int width, int height, bool next);
void free_boing();