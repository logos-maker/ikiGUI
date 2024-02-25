// to compile dor linux...
// gcc ikigui_standalone.c -o standalone -lX11
// for windows... (-mwindows gets rid of terminal opening)
// gcc ikigui_standalone.c -o standalone.exe -lgdi32 -mwindows

#define IKIGUI_STANDALONE // define before we include ikigui.h
#include "../../libs/ikigui.h"
#include "bmp.h"
#include <stdio.h>

void main(){
	char hello[13] = "HELLO WORLD!";
	ikigui_window mywin ; // Create a struct for the window

	ikigui_image pic ;    // Create a struct for the image to load	

        ikigui_map menubar;   // Create a struct for the tile map to paint as text
	ikigui_map_init(&menubar,&mywin.image,&pic,OFFSET_ASCII,0,0,8,8,48,1); // init the setings for the tilemap used as text area

	ikigui_include_bmp(&pic,bmp_array); // load the bmp image in the header file
	ikigui_rect rect = { .w= 8, .h=8, .x=1224, .y=0 }; // selection of a part of the image to paint with mouse

	ikigui_window_open(&mywin,1024,800); // Create and open window and specify window size

	while(1) { // Event loop! look for events forever...
	        ikigui_window_get_events(&mywin); // get the window events. Get events from mouse and window interactions. 
		if(mywin.mouse.buttons & MOUSE_LEFT){ ikigui_tile_fast(&mywin.image,&pic,mywin.mouse.x,mywin.mouse.y,NULL); } // blit image part to window if mouse down

                sprintf((char*)menubar.map,"%04d",mywin.mouse.y); // Generate text bytes
                ikigui_map_draw(&menubar,1,0,0);	// Draw menubar
		ikigui_breathe(34);			// Pause to give CPU power over to the OS
                ikigui_window_update(&mywin);		// update the window graphics
	}
}
