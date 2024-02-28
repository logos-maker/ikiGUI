// to compile dor linux...
// gcc ikigui_pixel.c -o pixel -lX11
// for windows... (-mwindows gets rid of terminal opening)
// gcc ikigui_pixel.c -o pixel.exe -lgdi32 -mwindows

#define IKIGUI_DIAL
#define IKIGUI_STANDALONE // define before we include ikigui.h
#include "../../libs/ikigui.h"
#include "bmp.h"

void main(){
	ikigui_window mywin ; // Create a struct for the window
	ikigui_image pic ;    // Create a struct for the image to load	
        ikigui_map menubar;   // Create a struct for the tile map to paint as text
	ikigui_color color_draw = 0x80FF00FF;
	ikigui_map_init(&menubar,&mywin.image,&pic,OFFSET_ASCII,0,0,8,8,48,1); // init the setings for the tilemap used as text area

	ikigui_include_bmp(&pic,bmp_array); // load the bmp image in the header file

	ikigui_window_open(&mywin,1024,800); // Create and open window and specify window size

	while(1) { // Event loop! look for events forever...
	        ikigui_window_get_events(&mywin); // get the window events. Get events from mouse and window interactions.

		if(mywin.mouse.buttons & MOUSE_LEFT){ 
			ikigui_pixel_draw(&mywin.image, mywin.mouse.x, mywin.mouse.y, color_draw); // Draw on pixel with transparent color.
		}
		if(mywin.mouse.right_click){
			ikigui_pixel_get(&mywin.image, mywin.mouse.x, mywin.mouse.y);
			ikigui_dial_color_select(color_draw,&color_draw);
			//ikigui_image_solid(&mywin.image,color_draw);
		}
                sprintf((char*)menubar.map,"%04d",mywin.mouse.y); // Generate text bytes
                ikigui_map_draw(&menubar,1,0,0);	// Draw menubar
		ikigui_breathe(34);			// Pause to give CPU power over to the OS
                ikigui_window_update(&mywin);		// update the window graphics
	}
}
