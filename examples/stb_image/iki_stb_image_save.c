// to compile dor linux...
// gcc ikigui_standalone.c -o standalone -lX11
// for windows... (-mwindows gets rid of terminal opening)
// gcc ikigui_standalone.c -o standalone.exe -lgdi32 -mwindows
#define IKIGUI_DIAL
#define IKIGUI_IMAGE
#define IKIGUI_STANDALONE // define before we include ikigui.h
#include "../../libs/ikigui.h"
#include <stdio.h>

void main(){
	char hello[13] = "HELLO WORLD!";
	ikigui_window mywin ; // Create a struct for the window

	ikigui_image pic ;    // Create a struct for the image to load	

        ikigui_map menubar;   // Create a struct for the tile map to paint as text
	ikigui_map_init(&menubar,&mywin.image,&pic,OFFSET_ASCII,0,0,8,8,48,1); // init the setings for the tilemap used as text area

	#define SCREEN_W 1024
	#define SCREEN_H 800
	ikigui_window_open(&mywin,"Save image",1024,800); // Create and open window and specify window size
   
	// Draw fractal background
	for (int y=0;y<SCREEN_H;y++){
		for (int x=0;x<SCREEN_W;x++) {
		    double re = 1.0*x/SCREEN_H - 0.5;
		    double im = 1.0*y/SCREEN_H - 0.5;
		    int n = 255;
		    while (n>0 && re*re+im*im<4) {
			double reSq = re*re - im*im;
			double imSq = 2*re*im;
			re = reSq -0.629;
			im = imSq +0.4;
			n--;
		    }
		    ikigui_pixel_set(&mywin.image, x, y, ikigui_color_make(255,n,n,n));
		}
	}
	
	ikigui_save_png(&mywin.image, "fractal.png");

	while(1) { // Event loop! look for events forever...
	        ikigui_window_get_events(&mywin); // get the window events. Get events from mouse and window interactions. 
		ikigui_breathe(34);			// Pause to give CPU power over to the OS
                ikigui_window_update(&mywin);		// update the window graphics
	}
}