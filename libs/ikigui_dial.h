#include "tinyfiledialogs.c"

void ikigui_popup(char * title_text, char * message_text){
	tinyfd_notifyPopup( title_text, message_text, NULL);
}

