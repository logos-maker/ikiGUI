// to compile dor linux...
// gcc ikigui_standalone.c -o standalone -lX11
// for windows... (-mwindows gets rid of terminal opening)
// gcc ikigui_standalone.c -o standalone.exe -lgdi32 -mwindows
#define IKIGUI_DIAL
#define IKIGUI_IMAGE
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

	#define SCREEN_W 1024
	#define SCREEN_H 800
	ikigui_window_open(&mywin,1024,800); // Create and open window and specify window size

/*
http://www.programming4beginners.com/tutorial/graphics/more-drawings
You can get some other interesting shapes by changing the pair of numbers (-0.629, +0.4) in the given program to some other values. We will give you a few recommendations:

    (–0.37, +0.667),
    (+0.331, +0.043),
    (–0.22, –0.72),
    (–0.758, –0.05937),
    (+0.14, +0.6),
    ( 0.278, –0.0078)


*/
	  //  clear_to_color(surface, makecol(255,255,255));
    
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

	// Open a dialog to save generated fractal
	char * fractal = ikigui_dial_save_image(NULL); // fractal set to NULL if we cansel in save dialog
	if(fractal != NULL) ikigui_save_png(&mywin.image, ikigui_dial_save_image(NULL));

	while(1) { // Event loop! look for events forever...
	        ikigui_window_get_events(&mywin); // get the window events. Get events from mouse and window interactions. 
		if(mywin.mouse.buttons & MOUSE_LEFT){ ikigui_tile_fast(&mywin.image,&pic,mywin.mouse.x,mywin.mouse.y,NULL); } // blit image part to window if mouse down

                sprintf((char*)menubar.map,"%04d",mywin.mouse.y); // Generate text bytes
                ikigui_map_draw(&menubar,1,0,0);	// Draw menubar
		ikigui_breathe(34);			// Pause to give CPU power over to the OS
                ikigui_window_update(&mywin);		// update the window graphics
	}
}
