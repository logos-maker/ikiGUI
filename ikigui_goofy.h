/// @file ikigui_goofy.h Drawing functions, where the first pixel is in the bottom left corner (used by Windows GDI) it uses a identical API as ikigui_regular.c

int hflip(int hight,int row){ // Internal for useage in other functions // invert vertical axis
        return (hight - row)-1;
}

unsigned int alpha_channel(unsigned int color,unsigned int temp){ // Internal for usage in other functions // done with fixed point math
	unsigned char alpha = temp >> 24; // Alpha channel
	unsigned char alpha_inv = ~alpha;
	unsigned char rf =  (temp&0xff0000)>>16;	// Red forground
	unsigned char gf =  (temp&0xff00)>>8;		// Green forground
	unsigned char bf = temp&0xff;			// Blue forground
	unsigned char rb = (color&0xff0000)>>16;	// Red beckground
	unsigned char gb = (color&0xff00)>>8;		// Red background
	unsigned char bb = color&0xff;			// Blue background                
	unsigned char ro = (alpha_inv*rb + alpha*rf)>>8;   // background + forground
	unsigned char go = (alpha_inv*gb + alpha*gf)>>8;   // background + forground
	unsigned char bo = (alpha_inv*bb + alpha*bf)>>8;   // background + forground
	return (unsigned int)((ro << 16) + (go<< 8) + bo); 
}

uint32_t ikigui_pixel_get(ikigui_image* source,int x, int y){ return source->pixels[source->w * hflip(source->h,y) + x];}  // not ready for usage for windows in all cases yet.
void     ikigui_pixel_set(ikigui_image* dest,int x, int y, uint32_t color){ dest->pixels[dest->w * hflip(dest->h,y) + x] = color;} // not ready for usage for windows in all cases yet.

void ikigui_draw_rect(ikigui_image *dest, uint32_t color, ikigui_rect *part ){ /// A unfilled rect (has alpha support)
	int x = part->x;
	int y = part->y;

        if((x<0) || (y<0))return; // sheilding crash
        if(dest->w < (x+part->w))return; // shielding crash
        if(dest->h < (y+part->h))return; // shielding crash
	if(!dest->composit){
		for(int i = part->x ; i < (part->x+part->w) ; i++){   // horizontal
			dest->pixels[i+hflip(dest->h, part->y) * dest->w] = 		alpha_channel(dest->pixels[i+hflip(dest->h, part->y)*dest->w], color);
			dest->pixels[i+hflip(dest->h, (part->y+part->h-1)) * dest->w] =	alpha_channel(dest->pixels[i+hflip(dest->h,(part->y+part->h-1))*dest->w], color);
		}

		for(int j = part->y ; j < (part->y + part->h -1) ; j++){ // vertical
			dest->pixels[ part->x+hflip(dest->h, j) *dest->w] =		alpha_channel(dest->pixels[ part->x+hflip(dest->h,j)*dest->w], color);
			dest->pixels[(part->x+part->w-1)+hflip(dest->h, j) * dest->w] =	alpha_channel(dest->pixels[(part->x+part->w-1)+hflip(dest->h, j)*dest->w], color);
		}
	}else{ // Draw With alpha
		for(int i = part->x ; i < (part->x+part->w) ; i++){   // horizontal
			dest->pixels[i+ part->y*dest->w] = 		alpha_channel(dest->pixels[i+ part->y*dest->w], color);
			dest->pixels[i+(part->y+part->h-1)*dest->w] =	alpha_channel(dest->pixels[i+(part->y+part->h-1)*dest->w], color);
		}

		for(int j = part->y ; j < (part->y + part->h -1) ; j++){ // vertical
			dest->pixels[ part->x+j*dest->w] =		alpha_channel(dest->pixels[ part->x+j*dest->w], color);
			dest->pixels[(part->x+part->w-1)+j*dest->w] =	alpha_channel(dest->pixels[(part->x+part->w-1)+j*dest->w], color);
		}
	}
}
void ikigui_draw_box(ikigui_image *dest, uint32_t color, ikigui_rect *part ){ /// To draw a filled rectangle (has alpha support)
	int x = part->x; // to make code easier to read
	int y = part->y; // to make code easier to read
        if((x<0) || (y<0))return; // sheilding crash
        if(dest->w < (x+part->w))return; // shielding crash
        if(dest->h < (y+part->h))return; // shielding crash
	if(!dest->composit){
		for(int j = 0 ; j < part->h ; j++){ // vertical
		        for(int i = 0 ; i < part->w ; i++){   // horizontal
				dest->pixels[(x+i+(hflip(dest->h,j+y))*dest->w)] = alpha_channel( dest->pixels[x+i+(j+y)*dest->w], color ); // with alpha
		        }
		}
	}else{
		for(int j = 0 ; j < part->h ; j++){ // vertical
		        for(int i = 0 ; i < part->w ; i++){   // horizontal
				dest->pixels[x+i+(j+y)*dest->w] = alpha_channel( dest->pixels[x+i+(j+y)*dest->w], color ); // with alpha
		        }
		}
	}
}
void ikigui_draw_line_v(ikigui_image *dest, uint32_t color, uint32_t x, uint32_t y, uint32_t length ){ /// Draw vertical line (has alpha support)
	if((x<0||y<0))return; // crash blocking
	if(!dest->composit){
		for(int i = y ; i < (y+length) ; i++){ // draw vertical line with alpha
			dest->pixels[x + hflip(dest->h,i) * dest->w] = alpha_channel(dest->pixels[x + hflip(dest->h,i) * dest->w], color);
		}
	}else{	// It's a solid color
		for(int i = y ; i < (y+length) ; i++){ // draw vertical line with alpha
			dest->pixels[x + i * dest->w] = alpha_channel(dest->pixels[x + i * dest->w], color);
		}
	}
}
void ikigui_draw_line_h(ikigui_image *dest, uint32_t color, uint32_t x, uint32_t y, uint32_t length ){ /// Draw horizontal line (has alpha support)
	if((x<0||y<0))return; // crash blocking
	if(!dest->composit){
		for(int i = x ; i < (x+length) ; i++){ // draw horizontal line with alpha
			dest->pixels[i + hflip(dest->h,y) * dest->w] = alpha_channel(dest->pixels[i + hflip(dest->h,y) * dest->w], color);
		}
	}
	else{	// It's a solid color
		for(int i = x ; i < (x+length) ; i++){ // draw horizontal line with alpha
			dest->pixels[i + y * dest->w] = alpha_channel(dest->pixels[i + y * dest->w], color);
		}
	}
}

// Functions that affects a part of a ikigui_image
void ikigui_draw_gradient(ikigui_image *dest, uint32_t color_top, uint32_t color_bot, ikigui_rect *part ){ /// Fill part of image or window with gradient.

	int x = part->x;
	int y = part->y;

        if((x<0) || (y<0))return; // sheilding crash
        if(dest->w < (x+part->w))return; // shielding crash
        if(dest->h < (y+part->h))return; // shielding crash

	uint8_t a1 = (color_bot & 0xff000000)>>24;// alpha color_bot
	uint8_t r1 = (color_bot & 0xff0000)>>16;  // Red color_bot
	uint8_t g1 = (color_bot & 0xff00)>>8;	  // Green color_bot
	uint8_t b1 =  color_bot & 0xff;		  // Blue color_bot

	uint8_t a2 = (color_top & 0xff000000)>>24;// alpha color_bot
	uint8_t r2 = (color_top & 0xff0000)>>16;  // Red color_top
	uint8_t g2 = (color_top & 0xff00)>>8;	  // Red color_top
	uint8_t b2 =  color_top & 0xff;		  // Blue color_top

	double line_const = (double)255/(double)part->h;
        for(int j = 0 ; j < part->h ; j++){ // vertical
		double rise = (double)j * line_const ;	// rising
		double fall = 255-rise;			// falling

                for(int i = 0 ; i < part->w ; i++){   // horizontal
			uint8_t ao = ((uint16_t)(rise*a1 + fall*a2))>>8;   // color_bot + color_top
			uint8_t ro = ((uint16_t)(rise*r1 + fall*r2))>>8;   // color_bot + color_top
			uint8_t go = ((uint16_t)(rise*g1 + fall*g2))>>8;   // color_bot + color_top
			uint8_t bo = ((uint16_t)(rise*b1 + fall*b2))>>8;   // color_bot + color_top

			if(!dest->composit){
				dest->pixels[(x+i+(hflip(dest->h,j+y))*dest->w)] = alpha_channel(dest->pixels[(x+i+(hflip(dest->h,j+y))*dest->w)],(ao << 24) + (ro << 16) + (go<< 8) + bo);
			}else{
				dest->pixels[x+i+(j+y)*dest->w] = alpha_channel(dest->pixels[x+i+(j+y)*dest->w],(ao << 24) + (ro << 16) + (go<< 8) + bo);
			}
                }
        }
}

void ikigui_draw_image(ikigui_image *dest,ikigui_image *source, int x, int y){ /// draw source image to the x,y coordinate in destination image
        for(int j = 0 ; j < source->h ; j++){ // vertical
                for(int i = 0 ; i < source->w ; i++){   // horizontal
			if(!dest->composit){
        			dest->pixels[(x+i+(hflip(dest->h,j+y))*dest->w)] = source->pixels[i+source->w*(j)];
			}else{
				dest->pixels[(x+i+(j+y)*dest->w)] = 		   source->pixels[i+source->w*(j)];
			}                       
                }
        }
}
void ikigui_draw_image_composite(ikigui_image *dest,ikigui_image *source, int x, int y){ /// draw source image to the x,y coordinate in destination image
        for(int j = 0 ; j < source->h ; j++){ // vertical
                for(int i = 0 ; i < source->w ; i++){   // horizontal
			if(!dest->composit){
        			dest->pixels[(x+i+(hflip(dest->h,j+y))*dest->w)] = alpha_channel(dest->pixels[(x+i+(hflip(dest->h,j+y))*dest->w)],source->pixels[i+source->w*(j)]);
			}else{
				dest->pixels[x+i+(j+y)*dest->w] = 		   alpha_channel(dest->pixels[x+i+(j+y)*dest->w],source->pixels[i+source->w*j]);
			}    
                }
        }
}

void ikigui_tile_alpha(ikigui_image *dest,ikigui_image *source, int x, int y, ikigui_rect *part){ /// Draw source area specified by rect, to the x,y, coordinate in the destination.
	if(NULL == part){ ikigui_draw_image_composite(dest,source,x,y); return; }
        if((x<0) || (y<0))return; // sheilding crash
        if(dest->w < (x+part->w))return; // shielding crash
        if(dest->h < (y+part->h))return; // shielding crash

        for(int j = 0 ; j < part->h ; j++){ // vertical
                for(int i = 0 ; i < part->w ; i++){   // horizontal
			if(!dest->composit){
				dest->pixels[(x+i+(hflip(dest->h,j+y))*dest->w)] 
				= alpha_channel(dest->pixels[(x+i+(hflip(dest->h,j+y))*dest->w)], source->pixels[i+part->x+source->w*(j+part->y)]);
			}else{
				dest->pixels[x+i+(j+y)*dest->w] 
				//dest->pixels[(x+i+(hflip(dest->h,j+y))*dest->w)]
				= alpha_channel(dest->pixels[x+i+(j+y)*dest->w], source->pixels[i+part->x+source->w*(j+part->y)]);
				//= alpha_channel(dest->pixels[(x+i+(hflip(dest->h,j+y))*dest->w)], source->pixels[i+part->x+source->w*(j+part->y)]);
			}
                }
        }
}
void ikigui_tile_filled(ikigui_image *dest,ikigui_image *source, int x, int y, ikigui_rect *part){ /// Draw source area specified by rect, to the x,y, coordinate in the destination. With a background color.
        if((x<0) || (y<0))return; // sheilding crash
        if(dest->w < (x+part->w))return; // shielding crash
        if(dest->h < (y+part->h))return; // shielding crash

        for(int j = 0 ; j < part->h ; j++){ // vertical
                for(int i = 0 ; i < part->w ; i++){   // horizontal
			if(!dest->composit){
				dest->pixels[(x+i+(hflip(dest->h,j+y))*dest->w)] 
				= alpha_channel(dest->color, source->pixels[i+part->x+source->w*(j+part->y)]);
			}else{
				dest->pixels[(x+i+(j+y)*dest->w)] 
				= alpha_channel(dest->color, source->pixels[i+part->x+source->w*(j+part->y)]);
			}	
                }
        }
}
void ikigui_tile_hollow(ikigui_image *dest,ikigui_image *source, int x, int y, ikigui_rect *part){ /// Draw area - can be used if you whant to fill 'high alpha value'/low transparancy with a solid color.
        if((x<0) || (y<0))return; // sheilding crash
        if(dest->w < (x+part->w))return; // shielding crash
        if(dest->h < (y+part->h))return; // shielding crash

        for(int j = 0 ; j < part->h ; j++){ // vertical
                for(int i = 0 ; i < part->w ; i++){   // horizontal
			if(!dest->composit){
				dest->pixels[(x+i+(hflip(dest->h,j+y))*dest->w)] 
				= alpha_channel(dest->pixels[(x+i+(hflip(dest->h,j+y))*dest->w)], (source->color&0x00FFFFFF) | (0xFF000000 & source->pixels[i+part->x+source->w*(j+part->y)]));
			}else{
				dest->pixels[x+i+(j+y)*dest->w] 
				= alpha_channel(dest->pixels[x+i+(j+y)*dest->w], (source->color&0x00FFFFFF) | (0xFF000000 & source->pixels[i+part->x+source->w*(j+part->y)]));
			}
                }
        }
}
void ikigui_tile_fast(ikigui_image *dest,ikigui_image *source, int x, int y, ikigui_rect *part){ /// Draw source area specified by rect, to the x,y, coordinate in the destination. Simple memory copying overwriting the dest pixels.
	if(NULL == part){ ikigui_draw_image(dest,source,x,y); return; }
        if((x<0) || (y<0))return; // shelding crash
        if(dest->w < (x+part->w))return; // shelding crash
        if(dest->h < (y+part->h))return; // shelding crash

        for(int j = 0 ; j < part->h ; j++){ // vertical
                for(int i = 0 ; i < part->w ; i++){   // horizontal
			if(!dest->composit){
        			dest->pixels[(x+i+(hflip(dest->h,j+y))*dest->w)] = source->pixels[i+part->x+source->w*(j+part->y)];
			}else{
				dest->pixels[(x+i+(j+y)*dest->w)] = 		   source->pixels[i+part->x+source->w*(j+part->y)];
			}
                }
        }
}

// Functions for image manipulation follows... 

void ikigui_image_solid(ikigui_image *dest, unsigned int color){ /// Fills the destination image/window with a specific ARGB color.
        for(int i = 0 ; i < dest->w * dest->h ; i++){ // All pixels      
                        dest->pixels[i] = color;
        }
}
void ikigui_image_solid_bg(ikigui_image *dest,unsigned int color){ /// Fill background in destination. Doesn't overwrite the image.
	// to precalc graphics for usage with ikigui_blit_part_fast() for faster graphics. Can be convinient in some cases.
        for(int i = 0 ; i < dest->size ; i++){
                dest->pixels[i] = alpha_channel(color,dest->pixels[i]);
        }
}
void ikigui_image_gradient(ikigui_image *dest, uint32_t color_top, uint32_t color_bot){ /// Fill image with a color gradient.
	ikigui_rect all = {.x=0,.y=0,.w= dest->w,.h= dest->h};
	ikigui_draw_gradient(dest,color_top,color_bot,&all);
}
void ikigui_image_make(ikigui_image *frame, uint32_t w,uint32_t h){ /// Allocates memory for a ikigui_image and initializes it, if ikigui_bmp_include function isn't used.
        frame->w = w;
        frame->h = h;
        frame->pixels = (unsigned int*)malloc(frame->w*frame->h*4);
        frame->size = frame->w * frame->h ;
	frame->composit = 1;
}
void ikigui_include_bmp(ikigui_image *dest,const unsigned char* bmp_incl){ /// Read BMP image in header file to a ikigui_image (and allocates memory for it)
        unsigned int start;
        dest->w = bmp_incl[0x12] + (bmp_incl[0x12+1]<<8) + (bmp_incl[0x12+2]<<16) + (bmp_incl[0x12+3]<<24);
        dest->h = bmp_incl[0x16] + (bmp_incl[0x16+1]<<8) + (bmp_incl[0x16+2]<<16) + (bmp_incl[0x16+3]<<24);
        start =    bmp_incl[0x0a] + (bmp_incl[0x0a+1]<<8) + (bmp_incl[0x0a+2]<<16) + (bmp_incl[0x0a+3]<<24);

	//uint32_t* pixels_to_free = dest->pixels ; 	
        dest->pixels = (unsigned int*)malloc(dest->w*dest->h*4); // Doesn't make sense if we read it in to a image that allready have allocated memory!!!
	//free(pixels_to_free);

        int counter = 0 ; 
        for(int j = dest->h -1 ; j >= 0 ; j--){ 
                for(int i = 0 ; i < dest->w ; i++){
                        int pixl_addr = (i+(j*dest->w))*4+start;
                        dest->pixels[counter++]= bmp_incl[pixl_addr]+ (bmp_incl[pixl_addr+1]<<8)+ (bmp_incl[pixl_addr+2]<<16)+ (bmp_incl[pixl_addr+3]<<24);
                        unsigned int temp = bmp_incl[pixl_addr]+ (bmp_incl[pixl_addr+1]<<8)+ (bmp_incl[pixl_addr+2]<<16)+ (bmp_incl[pixl_addr+3]<<24);
                }
        }
        dest->size = dest->w * dest->h ;
}

