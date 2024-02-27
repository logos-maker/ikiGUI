/// @file ikigui_dial.h To be able to open native file dialogs like load and save, with identical code in your programs.
// This API is not finished. It may change! Not tested on window yet
#include "tinyfiledialogs.c"

// ////////////////////////////////
//     Different type of popups
//     ...with similar API

void ikigui_dial_popup_info(char * title_text, char * message_text){ /// Notification popup for info
	tinyfd_notifyPopup( title_text, message_text, "info");
}
void ikigui_dial_popup_warning(char * title_text, char * message_text){ /// Notification popup for warnings
	tinyfd_notifyPopup( title_text, message_text, "warning");
}
void ikigui_dial_popup_error(char * title_text, char * message_text){ /// Notification popup for errors
	tinyfd_notifyPopup( title_text, message_text, "error");
}

// //////////////////////////////////////////////
//     Different type of text fields for input
//     ...with similar API

char * ikigui_dial_text_field(char * title_text, char * message_text){ ///  text input field
	tinyfd_inputBox( title_text, message_text, ""); // returns NULL on cancel 
}
char * ikigui_dial_text_pass(char * title_text, char * message_text, char * type){ ///  password input field
	tinyfd_inputBox( title_text, message_text, NULL); // returns NULL on cancel 
}

// /////////////////////////////
//     Dialogs for answering
//

int ikigui_dial_message(char * title_text, char * message_text, char * type, char * type2, int b_default){ /// enum type : "ok" "okcancel" "yesno" "yesnocancel"
// title text :  NULL or "" 
// message_text : NULL or "" may contain \n \t
// type :  "ok" "okcancel" "yesno" "yesnocancel"
// type2:  "info" "warning" "error" "question" 
// default: 0 for cancel/no , 1 for ok/yes , 2 for no in yesnocancel // Set default return alternative
	return tinyfd_messageBox( title_text, message_text, type, type2, b_default); // returns :  0 for cancel/no , 1 for ok/yes , 2 for no in yesnocancel
}

char *  ikigui_dial_load_image(char * path_text){  /// To load supported image files. Returns path, or NULL on cancel
	char const * lFilterPatterns[3]={"*.png","*.jpg","*.png"};
	return tinyfd_openFileDialog("Load image", path_text, 3,lFilterPatterns, NULL, 0 ); // returns NULL on cancel 
}
char * ikigui_dial_load_file(char * path_text){ /// To load any type of file. Returns path, or NULL on cancel
	char const * lFilterPatterns[3]={"*.png","*.jpg","*.png"};
	return tinyfd_openFileDialog("Load file", path_text, 0,NULL, NULL, 0 ); // returns NULL on cancel 
}
char * ikigui_dial_folder(char * path){ /// Let the user select a folder. Returns path, or NULL on cancel
	return	tinyfd_selectFolderDialog("Select folder",path ); // Path :  NULL or "" for your path
}
char *  ikigui_dial_save(char * title_text, char * path_text){ /// To save as any file type
//	char const * aTitle , // NULL or ""
//	char const * aDefaultPathAndOrFile , //  NULL or "" , ends with / to set only a directory 
//	int aNumOfFilterPatterns , // 0  (1 in the following example)
//	char const * const * aFilterPatterns , // NULL or char const * lFilterPatterns[1]={"*.txt"} 
//	char const * aSingleFilterDescription ) ; // NULL or "text files" 
		/* returns NULL on cancel */
	return tinyfd_saveFileDialog( title_text, path_text, 0, NULL, NULL); // Returns NULL on cancel
}

