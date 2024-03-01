/// @file ikigui_image.h To be able to load images in formats like JPG, PNG, TGA, BMP, PSD, GIF, HDR, PIC.

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

int save_png_header(ikigui_image *image, const char* output) {
	// Generate image file name
	char output_filename_png[256];
	strcpy(output_filename_png, output);
	strcat(output_filename_png, ".png");

	// Generate header file name
	char output_filename_h[256];
	strcpy(output_filename_h, output);
	strcat(output_filename_h, ".h");

	// Save PNG using stbi_write_png
	stbi_write_png(output_filename_png, image->w, image->h, 4, image->pixels, image->w * sizeof(unsigned int));


	FILE *image_file, *fout;
	image_file = fopen(output_filename_png,"r");
	if(!image_file){ return 1; }

	fout = fopen(output_filename_h,"w");
	if(!fout){ return 1; }

	char name_array[50];
	char name_file[50];
	strcpy(name_array,output);
	strcpy(name_file,output_filename_h);

	fseek(image_file, 0L, SEEK_END);
	uint64_t sz = ftell(image_file);
	rewind(image_file);

	// Write const unsigned char array declaration
	fprintf(fout,"const unsigned char %s[%ld] = {\n",name_array,sz);

	int count = 0;
	for(int i = 0 ; i < sz ; i++){
		uint8_t op;
		fread(&op,1,1,image_file);
		fprintf(fout,"0x%02X, ",(uint8_t)op);
		if (++count % 16 == 0) fprintf(fout, "\n");
	}

	fprintf(fout,"\n}");

	fclose(image_file);
	fclose(fout);
	return 0;
}

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