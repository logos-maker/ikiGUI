/* Created By: Justin Meiners (2013) */
// https://despairlabs.com/blog/posts/2011-05-18-using-stb-truetype-with-sdl/
// https://github.com/justinmeiners/stb-truetype-example/tree/master
#include <stdio.h>
#include <stdlib.h>

#define IKIGUI_STANDALONE
#include "../../libs/ikigui.h"

#define STB_TRUETYPE_IMPLEMENTATION 
#include "../../libs/stb_truetype.h" /* http://nothings.org/stb/stb_truetype.h */

int main(int argc, const char * argv[]){
	/* load font file */
	long size;
	unsigned char* fontBuffer;

	FILE* fontFile = fopen("font/cmunrm.ttf", "rb");
	fseek(fontFile, 0, SEEK_END);
	size = ftell(fontFile); /* how long is the file ? */
	fseek(fontFile, 0, SEEK_SET); /* reset */

	fontBuffer = malloc(size);

	fread(fontBuffer, size, 1, fontFile);
	fclose(fontFile);

	/* prepare font */
	stbtt_fontinfo info;
	if (!stbtt_InitFont(&info, fontBuffer, 0))
	{
	printf("failed\n");
	}
    
	int b_w = 512; /* bitmap width */
	int b_h = 128; /* bitmap height */
	int l_h = 64; /* line height */

	/* create a bitmap for the phrase */
	unsigned char* bitmap = calloc(b_w * b_h, sizeof(unsigned char));

	/* calculate font scaling */
	float scale = stbtt_ScaleForPixelHeight(&info, l_h);

	char* word = "the ikiGUI library";

	int x = 45;
	int y_start = 31;

	int ascent, descent, lineGap;
	stbtt_GetFontVMetrics(&info, &ascent, &descent, &lineGap);

	ascent = roundf(ascent * scale);
	descent = roundf(descent * scale);

	for(int letter,i = 0 ; letter = word[i] ; ){
		
		int char_next, char_width;  // expressed in unscaled coordinates.
		stbtt_GetCodepointHMetrics(&info, letter, &char_next, &char_width); // how wide is this character
		
		int c_x1, c_y1, c_x2, c_y2; // get bounding box for character (may be offset to account for chars that dip above or below the line)
		stbtt_GetCodepointBitmapBox(&info, letter, scale, scale, &c_x1, &c_y1, &c_x2, &c_y2); // get coordinates
		
		int y = ascent + c_y1 + y_start; // compute y (different characters have different heights)
		printf("%d\n",c_y1);
		// render character (stride and offset is important here)
		int byteOffset = x + roundf(char_width * scale) + (y * b_w);
		stbtt_MakeCodepointBitmap(&info, bitmap + byteOffset, c_x2 - c_x1, c_y2 - c_y1, b_w, scale, scale, letter);
		
		x += roundf(char_next * scale); // advance x

		// add kerning
		int kern = stbtt_GetCodepointKernAdvance(&info, letter, word[++i]);
		x += roundf(kern * scale);
	}
  
	ikigui_image iki_image;
	ikigui_image_make(&iki_image,b_w,b_h);

	ikigui_window iki_win;
	ikigui_window_open(&iki_win,b_w,b_h);
	ikigui_image_solid(&iki_win.image,0xFFDDDDDD);

	// överför bild
	for(int i = 0 ; i< (b_w * b_h) ; i++){
		iki_image.pixels[i] = (uint32_t)(bitmap[i] << 24);
	}

	free(fontBuffer);
	free(bitmap);

	ikigui_draw_image_composite(&iki_win.image,&iki_image,0,0);

	while(1){
		ikigui_window_update(&iki_win);
		ikigui_window_get_events(&iki_win);
		ikigui_breathe(32);
	}

	return 0;
}

