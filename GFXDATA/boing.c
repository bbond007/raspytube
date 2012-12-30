#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>


#include "boing/IMG000.c"
//#include "boing/IMG001.c"
#include "boing/IMG002.c"
//#include "boing/IMG003.c"
#include "boing/IMG004.c"
//#include "boing/IMG005.c"
#include "boing/IMG006.c"
//#include "boing/IMG007.c"
#include "boing/IMG008.c"
//#include "boing/IMG009.c"
#include "boing/IMG010.c"
//#include "boing/IMG011.c"
#include "boing/IMG012.c"
//#include "boing/IMG013.c"
#include "boing/IMG014.c"
//#include "boing/IMG015.c"
#include "boing/IMG016.c"
//#include "boing/IMG017.c"
#include "boing/IMG018.c"
//#include "boing/IMG019.c"
#include "boing/IMG020.c"
//#include "boing/IMG021.c"
#include "boing/IMG022.c"
//#include "boing/IMG023.c"
#include "boing/IMG024.c"
//#include "boing/IMG025.c"
#include "boing/IMG026.c"
//#include "boing/IMG027.c"
#include "boing/IMG028.c"
//#include "boing/IMG029.c"
#include "boing/IMG030.c"
//#include "boing/IMG031.c"
#include "boing/IMG032.c"
//#include "boing/IMG033.c"
#include "boing/IMG034.c"
//#include "boing/IMG035.c"
#include "boing/IMG036.c"
//#include "boing/IMG037.c"
#include "boing/IMG038.c"
//#include "boing/IMG039.c"
#include "boing/IMG040.c"
//#include "boing/IMG041.c"
#include "boing/IMG042.c"
//#include "boing/IMG043.c"
#include "boing.h"

tAnimFrame boingAnim[] =
{
    {-1, boingIMG000_data, &boingIMG000_size},
//  {-1, boingIMG001_data, &boingIMG001_size},
    {-1, boingIMG002_data, &boingIMG002_size},
//  {-1, boingIMG003_data, &boingIMG003_size},
    {-1, boingIMG004_data, &boingIMG004_size},
//  {-1, boingIMG005_data, &boingIMG005_size},
    {-1, boingIMG006_data, &boingIMG006_size},
//  {-1, boingIMG007_data, &boingIMG007_size},
    {-1, boingIMG008_data, &boingIMG008_size},
//  {-1, boingIMG009_data, &boingIMG009_size},
    {-1, boingIMG010_data, &boingIMG010_size},
//  {-1, boingIMG011_data, &boingIMG011_size},
    {-1, boingIMG012_data, &boingIMG012_size},
//  {-1, boingIMG013_data, &boingIMG013_size},
    {-1, boingIMG014_data, &boingIMG014_size},
//  {-1, boingIMG015_data, &boingIMG015_size},
    {-1, boingIMG016_data, &boingIMG016_size},
//  {-1, boingIMG017_data, &boingIMG017_size},
    {-1, boingIMG018_data, &boingIMG018_size},
//  {-1, boingIMG019_data, &boingIMG019_size},
    {-1, boingIMG020_data, &boingIMG020_size},
//  {-1, boingIMG021_data, &boingIMG021_size},
    {-1, boingIMG022_data, &boingIMG022_size},
//  {-1, boingIMG023_data, &boingIMG023_size},
    {-1, boingIMG024_data, &boingIMG024_size},
//  {-1, boingIMG025_data, &boingIMG025_size},
    {-1, boingIMG026_data, &boingIMG026_size},
//  {-1, boingIMG027_data, &boingIMG027_size},
    {-1, boingIMG028_data, &boingIMG028_size},
//  {-1, boingIMG029_data, &boingIMG029_size},
    {-1, boingIMG030_data, &boingIMG030_size},
//  {-1, boingIMG031_data, &boingIMG031_size},
    {-1, boingIMG032_data, &boingIMG032_size},
//  {-1, boingIMG033_data, &boingIMG033_size},
    {-1, boingIMG034_data, &boingIMG034_size},
//  {-1, boingIMG035_data, &boingIMG035_size},
    {-1, boingIMG036_data, &boingIMG036_size},
//  {-1, boingIMG037_data, &boingIMG037_size},
    {-1, boingIMG038_data, &boingIMG038_size},
//  {-1, boingIMG039_data, &boingIMG039_size},
    {-1, boingIMG040_data, &boingIMG040_size},
//  {-1, boingIMG041_data, &boingIMG041_size},
    {-1, boingIMG042_data, &boingIMG042_size},
//  {-1, boingIMG043_data, &boingIMG043_size},
    {-2, NULL		 , 0		    }
 };

//------------------------------------------------------------------------------
void free_boing()
{
    int frame = 0;
    do
    {
        if(boingAnim[frame].image != -1)
        {
            vgDestroyImage(boingAnim[frame].image);    
            boingAnim[frame].image= -1;
        }
    } while (boingAnim[++frame].image == -2);
}  
//------------------------------------------------------------------------------
bool draw_boing(int x, int y, int width, int height, bool next)
{
    static int frame = 0;
    int result = true;
    if(boingAnim[frame].image == -1)
    {
        //boingAnim[frame].image = create_image_from_buf(
        boingAnim[frame].image = createImageFromBuf(
        //boingAnim[frame].image = OMXCreateImageFromBuf(
                    boingAnim[frame].rawData,
                    boingAnim[frame].rawDataSize,
                    width, 
                    height);
        result = false;
    }
    if(boingAnim[frame].image > 0)
        vgSetPixels(x, y, boingAnim[frame].image, 0, 0, width, height);                               
    if(next && boingAnim[++frame].image == -2)
        frame = 0;
    return result;
}
    
    
