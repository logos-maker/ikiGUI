/// @file ikigui_image.h To be able to load imageuse in formats like JPG, PNG, TGA, BMP, PSD, GIF, HDR, PIC.
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "stb_image_write.h"
int ikigui_save_png(ikigui_image* source, char * file_path_name){ /// Not ready, need conversion from ARGB32 to RGBA32, channels will get mixed up in generated file.
    if (!stbi_write_png(file_path_name, source->w, source->h, 4, source->pixels, sizeof(uint32_t)*source->w)) return 1;
    else return 0;
}
int ikigui_image_load(ikigui_image* dest, char * file_path_name){ /// Load PNG, JPG, BPM file from disk
	int w, h, comp;

	uint32_t *pixels = (uint32_t*)stbi_load( file_path_name, &w, &h, &comp, 4);
	if(!pixels)return 0;
	for(int i = 0 ; i < (w*h); i++){
		pixels[i] =  ((pixels[i] & 0x00FF0000)>>16) + ((pixels[i] & 0x000000FF)<<16) + (pixels[i] & 0xFF00FF00) ;
	}
	dest->w = w;
	dest->h = h;
	dest->pixels = pixels ;
	return 1; // succeded
}
int ikigui_include_image(ikigui_image *dest,const unsigned char* image_incl, int image_len){ /// Load PNG, JPG, BPM file header file.
	int w, h, comp, channels;
	int *p_channels = &channels;
	//extern char _image_start, _image_len;
	uint32_t *pixels = (uint32_t*)stbi_load_from_memory( image_incl, image_len, &w, &h, p_channels, 4);
	if(!pixels)return 0;
	for(int i = 0 ; i < (w*h); i++){
		pixels[i] =  ((pixels[i] & 0x00FF0000)>>16) + ((pixels[i] & 0x000000FF)<<16) + (pixels[i] & 0xFF00FF00) ;
	}
	dest->w = w;
	dest->h = h;
	dest->pixels = pixels ;
	return 1; // succeded
}