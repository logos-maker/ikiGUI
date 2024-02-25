#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

int ikigui_image_load(ikigui_image* dest, char * name){
	int w, h, comp;
	uint32_t *pixels = (uint32_t*)stbi_load( name, &w, &h, &comp, 4);
	if(!pixels)return 0;
	for(int i = 0 ; i < (w*h); i++){
		pixels[i] =  ((pixels[i] & 0x00FF0000)>>16) + ((pixels[i] & 0x000000FF)<<16) + (pixels[i] & 0xFF00FF00) ;
	}
	dest->w = w;
	dest->h = h;
	dest->pixels = pixels ;
	return 1; // succeded
}