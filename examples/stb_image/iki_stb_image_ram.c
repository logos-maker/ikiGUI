#define IKIGUI_IMAGE
#define IKIGUI_STANDALONE
#include "../../libs/ikigui.h"
#include "./img/image.h"
#include <stdint.h>

int main(void){

    ikigui_window win;
    ikigui_image img;
    ikigui_include_image(&img,image_array,sizeof(image_array));
    ikigui_window_open(&win,img.w,img.h);
    ikigui_draw_image(&win.image,&img,0,0);

    while(1){
	ikigui_window_till(&win,34);
    }
    return 0;
}    
