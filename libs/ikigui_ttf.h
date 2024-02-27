/// @file ikigui_ttf.h To be able to use true type fonts.
#define STB_TRUETYPE_IMPLEMENTATION 
#include "stb_truetype.h" /* http://nothings.org/stb/stb_truetype.h */


int ikigui_draw_font(ikigui_image *iki_image, stbtt_fontinfo* info, int x, int y_start, int l_h, char* word){

	int w = iki_image->w ; 
	int h = iki_image->h ;

		
	unsigned char* bitmap = calloc(w * h, sizeof(unsigned char)); /* create a bitmap for the phrase */

	float scale = stbtt_ScaleForPixelHeight(info, l_h); // calculate font scaling 

	int ascent, descent, lineGap;
	stbtt_GetFontVMetrics(info, &ascent, &descent, &lineGap);

	ascent = roundf(ascent * scale);
	descent = roundf(descent * scale);

	for (int i = 0; i < strlen(word); ++i){
		
		int char_next, char_width;  // expressed in unscaled coordinates.
		stbtt_GetCodepointHMetrics(info, word[i], &char_next, &char_width); // how wide is this character
		
		int c_x1, c_y1, c_x2, c_y2; // get bounding box for character (may be offset to account for chars that dip above or below the line)
		stbtt_GetCodepointBitmapBox(info, word[i], scale, scale, &c_x1, &c_y1, &c_x2, &c_y2); // hämta coordinater
		
		int y = ascent + c_y1 + y_start; // compute y (different characters have different heights)

		// render character (stride and offset is important here)
		int byteOffset = x + roundf(char_width * scale) + (y * w);
		stbtt_MakeCodepointBitmap(info, bitmap + byteOffset, c_x2 - c_x1, c_y2 - c_y1, w, scale, scale, word[i]);

		x += roundf(char_next * scale); // advance x

		// add kerning
		int kern = stbtt_GetCodepointKernAdvance(info, word[i], word[i + 1]);
		x += roundf(kern * scale);
	}

	// överför bild
	for(int i = 0 ; i< (w * h) ; i++){
		iki_image->pixels[i] = (uint32_t)(bitmap[i] << 24);
	}

	//free(fontBuffer);
	free(bitmap);
}

typedef stbtt_fontinfo ikigui_font ;
int ikigui_font_init(ikigui_font *info, const unsigned char *data, int offset){
	return stbtt_InitFont(info, data, offset);
}
int ikigui_load_font(ikigui_font *info, char* path){

	/* load font file */
	long size;
	unsigned char* font_buffer;

	FILE* fontFile = fopen(path, "rb");
	fseek(fontFile, 0, SEEK_END);
	size = ftell(fontFile); /* how long is the file ? */
	fseek(fontFile, 0, SEEK_SET); /* reset */

	font_buffer = malloc(size);

	fread(font_buffer, size, 1, fontFile);
	fclose(fontFile);

	/* prepare font */
	return ikigui_font_init(info, font_buffer, 0);
}
