/// @file ikigui_goofy.h Drawing functions, where the first pixel is in the bottom left corner (used by Windows GDI)

int hflip(int hight,int row){ // invert vertical axis
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

void ikigui_fill_bg(ikigui_image *dest,unsigned int color){ /// A background color for automatic filling of transparent pixels.
	// to precalc graphics for usage with ikigui_blit_part_fast() for faster graphics. Can be convinient in some cases.
        for(int i = 0 ; i < dest->size ; i++){
                dest->pixels[i] = alpha_channel(color,dest->pixels[i]);
        }
}

void ikigui_image_create(ikigui_image *frame, uint32_t w,uint32_t h){ /// Allocates memory for a ikigui_image and initializes it, if ikigui_bmp_include function isn't used.
        frame->w = w;
        frame->h = h;
        frame->pixels = (unsigned int*)malloc(frame->w*frame->h*4);
        frame->size = frame->w * frame->h ;
	frame->composit = 1;
}

void ikigui_draw_gradient(ikigui_image *dest, uint32_t color_top, uint32_t color_bot, ikigui_rect *part ){ /// Fill part of image or window with gradient.

	int x = part->x;
	int y = part->y;

        if((x<0) || (y<0))return; // sheilding crash
        if(dest->w < (x+part->w))return; // shielding crash
        if(dest->h < (y+part->h))return; // shielding crash

	double line_const = (double)255/(double)part->h;
        for(int j = 0 ; j < part->h ; j++){ // vertical
		double rise = (double)j * line_const ;	// rising
		double fall = 255-rise;			// falling

                for(int i = 0 ; i < part->w ; i++){   // horizontal
			uint8_t r1 = (color_bot&0xff0000)>>16;	// Red color_bot
			uint8_t g1 = (color_bot&0xff00)>>8;	// Green color_bot
			uint8_t b1 = color_bot&0xff;		// Blue color_bot
			uint8_t r2 = (color_top&0xff0000)>>16;	// Red color_top
			uint8_t g2 = (color_top&0xff00)>>8;	// Red color_top
			uint8_t b2 = color_top&0xff;		// Blue color_top

			uint8_t ro = ((uint16_t)(rise*r1 + fall*r2))>>8;   // color_bot + color_top
			uint8_t go = ((uint16_t)(rise*g1 + fall*g2))>>8;   // color_bot + color_top
			uint8_t bo = ((uint16_t)(rise*b1 + fall*b2))>>8;   // color_bot + color_top

			if(!dest->composit){
				 dest->pixels[(x+i+(hflip(dest->h,j+y))*dest->w)] = (255<<24) + (ro << 16) + (go<< 8) + bo;
			}else{
				dest->pixels[x+i+(j+y)*dest->w] = (255<<24) + (ro << 16) + (go<< 8) + bo;
			}
                }
        }
}

void ikigui_blit_alpha(ikigui_image *dest,ikigui_image *source, int x, int y, ikigui_rect *part){ /// Draw source area specified by rect, to the x,y, coordinate in the destination.
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
void ikigui_blit_filled(ikigui_image *dest,ikigui_image *source, int x, int y, ikigui_rect *part){ /// Draw source area specified by rect, to the x,y, coordinate in the destination. With a background color.
        if((x<0) || (y<0))return; // sheilding crash
        if(dest->w < (x+part->w))return; // shielding crash
        if(dest->h < (y+part->h))return; // shielding crash

        for(int j = 0 ; j < part->h ; j++){ // vertical
                for(int i = 0 ; i < part->w ; i++){   // horizontal
			if(!dest->composit){
				dest->pixels[(x+i+(hflip(dest->h,j+y))*dest->w)] 
				= alpha_channel(dest->bg_color, source->pixels[i+part->x+source->w*(j+part->y)]);
			}else{
				dest->pixels[(x+i+(j+y)*dest->w)] 
				= alpha_channel(dest->bg_color, source->pixels[i+part->x+source->w*(j+part->y)]);
			}	
                }
        }
}
void ikigui_blit_hollow(ikigui_image *dest,ikigui_image *source, int x, int y, ikigui_rect *part){ /// Draw area - can be used if you whant to fill 'high alpha value'/low transparancy with a solid color.
        if((x<0) || (y<0))return; // sheilding crash
        if(dest->w < (x+part->w))return; // shielding crash
        if(dest->h < (y+part->h))return; // shielding crash

        for(int j = 0 ; j < part->h ; j++){ // vertical
                for(int i = 0 ; i < part->w ; i++){   // horizontal
			if(!dest->composit){
				dest->pixels[(x+i+(hflip(dest->h,j+y))*dest->w)] 
				= alpha_channel(dest->pixels[(x+i+(hflip(dest->h,j+y))*dest->w)], (source->bg_color&0x00FFFFFF) | (0xFF000000 & source->pixels[i+part->x+source->w*(j+part->y)]));
			}else{
				dest->pixels[x+i+(j+y)*dest->w] 
				= alpha_channel(dest->pixels[x+i+(j+y)*dest->w], (source->bg_color&0x00FFFFFF) | (0xFF000000 & source->pixels[i+part->x+source->w*(j+part->y)]));
			}
                }
        }
}
void ikigui_blit_fast(ikigui_image *dest,ikigui_image *source, int x, int y, ikigui_rect *part){ /// Draw source area specified by rect, to the x,y, coordinate in the destination. Simple memory copying overwriting the dest pixels.
        if((x<0) || (y<0))return; // shelding crash
        if(dest->w < (x+part->w))return; // shelding crash
        if(dest->h < (y+part->h))return; // shelding crash

        for(int j = 0 ; j < part->h ; j++){ // vertical
                for(int i = 0 ; i < part->w ; i++){   // horizontal
			if(!dest->composit){
        			dest->pixels[(x+i+(hflip(dest->h,j+y))*dest->w)] = source->pixels[i+part->x+source->w*(j+part->y)];
			}else{
				dest->pixels[(x+i+(j+y)*dest->w)] = source->pixels[i+part->x+source->w*(j+part->y)];
			}
                }
        }
}

void ikigui_image_draw(ikigui_image *dest,ikigui_image *source, int x, int y){ /// draw source image to the x,y coordinate in destination image
        for(int j = 0 ; j < source->h ; j++){ // vertical
                for(int i = 0 ; i < source->w ; i++){   // horizontal
                        dest->pixels[(x+i+(hflip(dest->h,j+y))*dest->w)] = source->pixels[i+source->w*(j)];
                }
        }
}
void ikigui_image_solid(ikigui_image *dest, unsigned int color){ /// Fill destination image/window with a specific ARGB color
        for(int i = 0 ; i < dest->w * dest->h ; i++){ // All pixels      
                        dest->pixels[i] = color;
        }
}
void ikigui_image_gradient(ikigui_image *dest, uint32_t color_top, uint32_t color_bot){ /// Fill destination image/window with a ARGB color gradient
	double line_const = (double)255/(double)dest->h;
        for(int j = 0 ; j < dest->h ; j++){ // vertical
		double rise = (double)j * line_const ;	// rising
		double fall = 255-rise;			// falling

                for(int i = 0 ; i < dest->w ; i++){   // horizontal
			uint8_t r1 = (color_bot&0xff0000)>>16;	// Red color_bot
			uint8_t g1 = (color_bot&0xff00)>>8;	// Green color_bot
			uint8_t b1 = color_bot&0xff;		// Blue color_bot
			uint8_t r2 = (color_top&0xff0000)>>16;	// Red color_top
			uint8_t g2 = (color_top&0xff00)>>8;	// Red color_top
			uint8_t b2 = color_top&0xff;		// Blue color_top

			uint8_t ro = ((uint16_t)(rise*r1 + fall*r2))>>8;   // color_bot + color_top
			uint8_t go = ((uint16_t)(rise*g1 + fall*g2))>>8;   // color_bot + color_top
			uint8_t bo = ((uint16_t)(rise*b1 + fall*b2))>>8;   // color_bot + color_top
			if(!dest->composit)
				dest->pixels[i+hflip(dest->h,j)*dest->w] = (255<<24) + (ro << 16) + (go<< 8) + bo;
			else{
				dest->pixels[i+(j*dest->w)] = (255<<24) + (ro << 16) + (go<< 8) + bo;
			}
                }
        }
}

void ikigui_bmp_include(ikigui_image *dest,const unsigned char* bmp_incl){ /// Read BMP image in header file to the destination image/window
        unsigned int start;
        dest->w = bmp_incl[0x12] + (bmp_incl[0x12+1]<<8) + (bmp_incl[0x12+2]<<16) + (bmp_incl[0x12+3]<<24);
        dest->h = bmp_incl[0x16] + (bmp_incl[0x16+1]<<8) + (bmp_incl[0x16+2]<<16) + (bmp_incl[0x16+3]<<24);
        start =    bmp_incl[0x0a] + (bmp_incl[0x0a+1]<<8) + (bmp_incl[0x0a+2]<<16) + (bmp_incl[0x0a+3]<<24);

        dest->pixels = (unsigned int*)malloc(dest->w*dest->h*4);

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

