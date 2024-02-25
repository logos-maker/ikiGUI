#define OLIVEC_IMPLEMENTATION
#include "./olive.c"
#define IKIGUI_STANDALONE
#include "../../libs/ikigui.h"

#define WIDTH 900
#define HEIGHT 600

uint32_t pixels[WIDTH*HEIGHT];

int main(void){

	Olivec_Canvas oc = olivec_canvas(pixels, WIDTH, HEIGHT, WIDTH);
	olivec_fill(oc, 0xFFFFFFFF);
	olivec_circle(oc, WIDTH/2, HEIGHT/2, 180, 0xFF2D00BC);

	ikigui_window win;
	ikigui_window_open(&win,WIDTH,HEIGHT);
	ikigui_image img;
	img.w = WIDTH;
	img.h = HEIGHT;
	img.pixels = pixels ;

	for(int i = 0 ; i < (WIDTH*HEIGHT); i++){
		pixels[i] =  ((pixels[i] & 0x00FF0000)>>16) + ((pixels[i] & 0x000000FF)<<16) + (pixels[i] & 0xFF00FF00) ;
	}

	ikigui_draw_image(&win.image,&img,0,0);

	while(1){
		ikigui_window_update(&win);
		ikigui_window_get_events(&win);
		ikigui_breathe(34);
	}

    return 0;
}