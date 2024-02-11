
#include <SDL2/SDL.h>
#include <stdbool.h>
#define IKIGUI_DRAW_ONLY
#include "../libs/ikigui.h"
#include "../gfx/font.h"

#define width 640
#define hight 480

ikigui_image bg;	// Global graphics for background art.
Uint32 pixels[width * hight];

struct data{  // 
	ikigui_map font_map; // for textbased statusbar for debugging.
	ikigui_image mywin; // A plugin window declaration.
	ikigui_image font;
} data;


int main(int argc, char ** argv){

	bool leftMouseButtonDown = false;
	bool quit = false;
	SDL_Event event;

	SDL_Init(SDL_INIT_VIDEO);

	SDL_Window * window = SDL_CreateWindow("SDL2 ikiGUI Pixel Example",
	SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width, hight, 0);

	SDL_Renderer * renderer = SDL_CreateRenderer(window, -1, 0);
	SDL_Texture * texture = SDL_CreateTexture(renderer,
	SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STATIC, width, hight);


	// Create a background image - a color gradient
	ikigui_image_create(&bg, width,hight );
	ikigui_image_gradient(&bg,0x00eeeedd, 0x00999999);

	// Draw text
	ikigui_bmp_include(&data.font,font_array); // We are later only going to use the alpha channel of that image.
	ikigui_map_init(&data.font_map, &bg, &data.font, ASCII,8,8,8,8,16,1);
	sprintf(data.font_map.map,"HELLO WORLD");

	data.font.bg_color = 0x00aa0000;
	ikigui_map_draw(&data.font_map,HOLLOW,10,10); // Draw text debugging text. 	// enum blit_type { APLHA = 0, FILLED = 1, SOLID = 2, HOLLOW = 3 };

	map_ikigui_to_sdl(&data.mywin,(unsigned int *)&(pixels)+-3,width,hight);


	ikigui_image_draw(&data.mywin,&bg,0,0);	// Draw background.

	while (!quit){
		SDL_UpdateTexture(texture, NULL, pixels, width * sizeof(Uint32));

		SDL_WaitEvent(&event);

		switch (event.type){
			case SDL_QUIT: quit = true;	break;
			case SDL_MOUSEBUTTONUP:		if (event.button.button == SDL_BUTTON_LEFT)	leftMouseButtonDown = false;	break;
			case SDL_MOUSEBUTTONDOWN:	if (event.button.button == SDL_BUTTON_LEFT)	leftMouseButtonDown = true;
			case SDL_MOUSEMOTION:		
				if (leftMouseButtonDown){
					int mouseX = event.motion.x;
					int mouseY = event.motion.y;
					pixels[mouseY * width + mouseX] = 0x00FF0000;
				}
			break;
		}

		SDL_RenderClear(renderer);
		SDL_RenderCopy(renderer, texture, NULL, NULL);
		SDL_RenderPresent(renderer);
	}

	SDL_DestroyTexture(texture);
	SDL_DestroyRenderer(renderer);

	SDL_DestroyWindow(window);
	SDL_Quit();

	return 0;
}

//
//	
/*
	ikigui_bmp_include(&data.font,font_array);
	ikigui_map_init(&data.font_map, &data.mywin, &data.font, ASCII,8,8,8,8,16,1);
	sprintf(data.font_map.map,"HELLO WORLD");
	ikigui_map_draw(&data.font_map,0,0,0); // Draw text debugging text.
*/
