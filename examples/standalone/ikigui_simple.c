
#define IKIGUI_STANDALONE
#include "../../libs/ikigui.h"

void main(){
	ikigui_window mywin;
	ikigui_window_open(&mywin, "Simple", 400, 100);
	ikigui_image_solid(&mywin.image,ikigui_color_make(255,255,0,255));

	while(1){
		ikigui_window_till(&mywin,34);
	}
}