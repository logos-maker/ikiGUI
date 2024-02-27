#define IKIGUI_DIAL
#define IKIGUI_STANDALONE
#include "../../libs/ikigui.h"

void main(){
	ikigui_window mywin;
	ikigui_window_open(&mywin,400,100);
	ikigui_image_solid(&mywin.image,ikigui_color_make(255,255,0,255));
	ikigui_dial_message("A question","Do you want to eat?","yesno","warning",1); // Change "" to NULL for password box

	while(1){
		ikigui_window_till(&mywin,34);
	}
}

// void ikigui_dial_message(char * title_text, char * message_text, char * type, char * type2, int b_default){ /// enum type : "ok" "okcancel" "yesno" "yesnocancel"
// title text :  NULL or "" 
// message_text : NULL or "" may contain \n \t
// type :  "ok" "okcancel" "yesno" "yesnocancel"
// type2:  "info" "warning" "error" "question" 
// default: 0 for cancel/no , 1 for ok/yes , 2 for no in yesnocancel