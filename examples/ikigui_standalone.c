// to compile dor linux...
// gcc ikigui_standalone.c -o standalone -lX11
// for windows... (-mwindows gets rid of terminal opening)
// gcc ikigui_standalone.c -o standalone.exe -lgdi32 -mwindows

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include "../libs/ikigui.h"
//#include "font.h"
#include "bmp.h"
#define IKIGUI_STANDALONE

#ifdef __linux__
	#include <unistd.h>
	void Sleep(int milisec){ usleep(milisec *1000); } //pass in microseconds
#endif
/*
void main(){
	ikigui_window mywin;
	ikigui_window_open(&mywin,200,100);

	while(1) { // look for events forever...
	        ikigui_window_get_events(&mywin); // get the next event.  Note: only events we set the mask for are detected!
		Sleep(34);
                ikigui_window_update(&mywin);
	}
}
*/

void main () {
        ikigui_image pic ;
	ikigui_window mywin ; // create our variables/struct
	ikigui_window_open(&mywin,1024,800); // Create window
        ikigui_rect rect = {
                .w= 8, .h=8, .x=1224, .y=0
        };

        ikigui_include_bmp(&pic,bmp_array);
	//ikigui_include_bmp(&pic,font_array);
        
        ikigui_map menubar; //ikigui_map_init(&menubar, &mywin.frame,&pic,48,1);// init... menu menubar
	ikigui_map_init(&menubar,&mywin.frame,&pic,OFFSET_ASCII,0,0,8,8,48,1);

        //sprintf((char*)menubar.map,"| | FILE  | EDIT  | VIEW | TOOLS | SETTINGS | HELP |"); // Statusbar apperance
        sprintf((char*)menubar.map,"FINE |");
        ikigui_map_draw(&menubar,1,0,0);           // Draw menubar

	while(1) { // look for events forever...
	        ikigui_window_get_events(&mywin); // get the next event.  Note: only events we set the mask for are detected!
		if(mywin.mouse.buttons & MOUSE_LEFT){
                        ikigui_blit_fast(&mywin.frame,&pic,mywin.mouse.x,mywin.mouse.y,&rect);
                        //mywin.frame.pixels[mywin.mouse.x + mywin.mouse.y*mywin.frame.w] = 0xffffffff;
                }
                //if(mywin.mouse.buttons & MOUSE_LEFT) mywin.pixels[mywin.mouse.x + mywin.mouse.y*mywin.w] = 0x00ffffff;
                sprintf((char*)menubar.map,"%04d",mywin.mouse.y);
                ikigui_map_draw(&menubar,1,0,0);           // Draw menubar
		Sleep(34);
                ikigui_window_update(&mywin);
	}
}
