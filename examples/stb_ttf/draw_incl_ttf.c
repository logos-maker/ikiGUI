
#include <stdio.h>
#include <stdlib.h>

#define IKIGUI_TTF
#define IKIGUI_STANDALONE
#include "../../libs/ikigui.h"
#include "./font/font.h"

int main(int argc, const char * argv[]){
	
	ikigui_font info;
	if (!ikigui_font_init(&info, font_array, 0)) // load font from included file
	{
	printf("failed\n");
	}
    
	int b_w = 512; /* bitmap width */
	int b_h = 128; /* bitmap height */
	int l_h = 64; /* line height */

	ikigui_image iki_image;
	ikigui_image_make(&iki_image,b_w,b_h);
	ikigui_draw_font(&iki_image, &info, 45, 31, l_h, "the ikiGUI library");

	ikigui_window iki_win;
	ikigui_window_open(&iki_win,"TTF Loaded from a include file",b_w,b_h);
	ikigui_image_solid(&iki_win.image,0xFFDDDDDD);

	ikigui_draw_image_composite(&iki_win.image,&iki_image,0,0);

	while(1){
		ikigui_window_update(&iki_win);
		ikigui_window_get_events(&iki_win);
		ikigui_breathe(32);
	}

	return 0;
}

