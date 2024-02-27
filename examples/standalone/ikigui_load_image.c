#define IKIGUI_DIAL
#define IKIGUI_STANDALONE
#include "../../libs/ikigui.h"

void main(){
	ikigui_window mywin;
	ikigui_window_open(&mywin,400,100);
	ikigui_image_solid(&mywin.image,ikigui_color_make(255,255,0,255));
	ikigui_dial_load_image(NULL); // Change "" to NULL for password box

	while(1){
		ikigui_window_till(&mywin,34);
	}
}

// void ikigui_dial_load_image(char * path_text) 
// path_text :  NULL or "" , ends with / to set only a directory