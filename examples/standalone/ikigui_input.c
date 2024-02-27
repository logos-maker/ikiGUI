#define IKIGUI_DIAL
#define IKIGUI_STANDALONE
#include "../../libs/ikigui.h"

void main(){
	ikigui_window mywin;
	ikigui_window_open(&mywin,400,100);
	ikigui_image_solid(&mywin.image,ikigui_color_make(255,255,0,255));
	ikigui_dial_text_field("My title","Hello World!");

	while(1){
		ikigui_window_till(&mywin,34);
	}
}