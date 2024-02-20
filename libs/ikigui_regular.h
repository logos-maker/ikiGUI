/// @file ikigui_regular.h Drawing functions, where the first pixel is in the top left corner (used by X11 on Linux)

#ifdef IKIGUI_DRAW_ONLY
void map_ikigui_to_sdl(ikigui_image * dest, Uint32 * pixels, int w , int h){
	dest->w = w;
	dest->h = h;
	dest->pixels = pixels; // copy pointer
}
#endif

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

void ikigui_draw_panel(ikigui_image *dest, uint32_t color, uint32_t light, uint32_t shadow, ikigui_rect *part ){ /// A filled rect with shaddow
	int x = part->x;
	int y = part->y;

        if((x<0) || (y<0))return; // sheilding crash
        if(dest->w < (x+part->w))return; // shielding crash
        if(dest->h < (y+part->h))return; // shielding crash

        for(int j = 0 ; j < part->h ; j++){ // vertical
                for(int i = 0 ; i < part->w ; i++){   // horizontal
			dest->pixels[x+i+(j+y)*dest->w] = color ;
                }
	}

	for(int i = part->x ; i < (part->x+part->w) ; i++){   // horizontal
		dest->pixels[i+part->y*dest->w] = light;
        }

        for(int j = part->y ; j < (part->y + part->h -1) ; j++){ // vertical
		dest->pixels[part->x+j*dest->w] = light;
		dest->pixels[(part->x+part->w-1)+j*dest->w] = shadow;
	}

        for(int i = part->x; i < (part->x+part->w ); i++){   // horizontal
		dest->pixels[i+(part->y+part->h-1)*dest->w] = shadow;
        }
}
void ikigui_draw_bevel(ikigui_image *dest, uint32_t color, uint32_t shadow, ikigui_rect *part ){ /// A unfilled rect with shaddow
	int x = part->x;
	int y = part->y;

        if((x<0) || (y<0))return; // sheilding crash
        if(dest->w < (x+part->w))return; // shielding crash
        if(dest->h < (y+part->h))return; // shielding crash

        for(int i = part->x ; i < (part->x+part->w) ; i++){   // horizontal
		dest->pixels[i+part->y*dest->w] = color;
        }

        for(int j = part->y ; j < (part->y + part->h -1) ; j++){ // vertical
		dest->pixels[part->x+j*dest->w] = color;
		dest->pixels[(part->x+part->w-1)+j*dest->w] = shadow;
	}

        for(int i = part->x; i < (part->x+part->w ); i++){   // horizontal
		dest->pixels[i+(part->y+part->h-1)*dest->w] = shadow;
        }
}

void ikigui_draw_rect(ikigui_image *dest, uint32_t color, ikigui_rect *part ){ /// A unfilled rect (has alpha support)
	int x = part->x;
	int y = part->y;

        if((x<0) || (y<0))return; // sheilding crash
        if(dest->w < (x+part->w))return; // shielding crash
        if(dest->h < (y+part->h))return; // shielding crash
	if((color & 0xFF000000) == 0xFF000000){ // Do we use alpha? True if no alpha (alpha is set to 0xFF in the color value)
		for(int i = part->x ; i < (part->x+part->w) ; i++){   // horizontal
			dest->pixels[i+part->y*dest->w] = color;
			dest->pixels[i+(part->y+part->h-1)*dest->w] = color;
		}

		for(int j = part->y ; j < (part->y + part->h -1) ; j++){ // vertical
			dest->pixels[part->x+j*dest->w] = color;
			dest->pixels[(part->x+part->w-1)+j*dest->w] = color;
		}
	}else{ // Draw With alpha
		for(int i = part->x ; i < (part->x+part->w) ; i++){   // horizontal
			dest->pixels[i+part->y*dest->w] = 		alpha_channel(dest->pixels[i+ part->y*dest->w],color);
			dest->pixels[i+(part->y+part->h-1)*dest->w] =	alpha_channel(dest->pixels[i+(part->y+part->h-1)*dest->w],color);
		}

		for(int j = part->y ; j < (part->y + part->h -1) ; j++){ // vertical
			dest->pixels[part->x+j*dest->w] =		alpha_channel(dest->pixels[part->x+j*dest->w],color);
			dest->pixels[(part->x+part->w-1)+j*dest->w] =	alpha_channel(dest->pixels[(part->x+part->w-1)+j*dest->w],color);
		}
	}
}

void ikigui_draw_box(ikigui_image *dest, uint32_t color, ikigui_rect *part ){ /// To draw a filled rectangle (has alpha support)
	int x = part->x; // to make code easier to read
	int y = part->y; // to make code easier to read
        if((x<0) || (y<0))return; // sheilding crash
        if(dest->w < (x+part->w))return; // shielding crash
        if(dest->h < (y+part->h))return; // shielding crash
	if((color & 0xFF000000) == 0xFF000000){ // Do we use alpha? True if no alpha (alpha is set to 0xFF in the color value)
		for(int j = 0 ; j < part->h ; j++){ // vertical
		        for(int i = 0 ; i < part->w ; i++){   // horizontal
				dest->pixels[x+i+(j+y)*dest->w] = color ; // without alpha
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
	if((color & 0xFF000000) != 0xFF000000){ // Do we use alpha? True if no alpha (alpha is set to 0xFF in the color value)
		for(int i = y ; i < (y+length) ; i++){ // draw vertical line with alpha
			dest->pixels[x + i * dest->w] = alpha_channel(dest->pixels[x + i * dest->w],color);
		}
	}else{	// It's a solid color
		for(int i = y ; i < (y+length) ; i++){ // draw vertical line without alpha
			dest->pixels[x + i * dest->w] = color;
		}
	}
}
void ikigui_draw_line_h(ikigui_image *dest, uint32_t color, uint32_t x, uint32_t y, uint32_t length ){ /// Draw horizontal line (has alpha support)
	if((x<0||y<0))return; // crash blocking
	if((color & 0xFF000000) != 0xFF000000){ // Do we use alpha? True if no alpha (alpha is set to 0xFF in the color value)
		for(int i = x ; i < (x+length) ; i++){ // draw horizontal line with alpha
			dest->pixels[i + y * dest->w] = alpha_channel(dest->pixels[i + y * dest->w],color);
		}
	}
	else{	// It's a solid color
		for(int i = x ; i < (x+length) ; i++){ // draw horizontal line without alpha
			dest->pixels[i + y * dest->w] = color;
		}
	}
}

void ikigui_draw_line(ikigui_image *dest, uint32_t color, int x0, int y0, int x1, int y1) {
	int dx = abs(x1-x0), sx = x0<x1 ? 1 : -1;
	int dy = abs(y1-y0), sy = y0<y1 ? 1 : -1; 
	int err = (dx>dy ? dx : -dy)/2, e2;

	for(;;){
		dest->pixels[dest->w * y0 + x0] = color;
		if(x0==x1 && y0==y1) break;
		e2 = err;
		if(e2 >-dx) { err -= dy; x0 += sx; }
		if(e2 < dy) { err += dx; y0 += sy; }
	}
}


void ikigui_draw_circle(ikigui_image *dest, uint32_t color, int x0, int y0, unsigned int radius){
	void plot(int x0,int y0){ dest->pixels[dest->w * y0 + x0] = color;}

	int f = 1 - radius;
	int ddF_x = 0;
	int ddF_y = -2 * radius;
	int x = 0;
	int y = radius;

	plot(x0 + radius, y0); // right  NE  
	plot(x0, y0 - radius); // Top    NW
	plot(x0 - radius, y0); // left   SW
	plot(x0, y0 + radius); // bottom SE

	while(x < y){
		if(f >= 0){
			y--;
			ddF_y += 2;
			f += ddF_y;
		}
		x++;
		ddF_x += 2;
		f += ddF_x + 1;    

		plot(x0 + y, y0 - x); // 1 , 1cuad //
		plot(x0 + x, y0 - y); // 2 , 1cuad //
		plot(x0 - x, y0 - y); // 3 , 2cuad //
		plot(x0 - y, y0 - x); // 4 , 2cuad //
		plot(x0 - y, y0 + x); // 5,  3cuad //
		plot(x0 - x, y0 + y); // 6 , 3cuad //
		plot(x0 + x, y0 + y); // 7 , 4cuad //
		plot(x0 + y, y0 + x); // 8,  4cuad //		
	}
}


enum cuadrants{ NE = 1, NW = 2, SW = 4, SE = 8};
void ikigui_draw_circle_parts(ikigui_image *dest, int quad_flags, uint32_t color, int x0, int y0, unsigned int radius){
	void plot(int x0,int y0){ dest->pixels[dest->w * y0 + x0] = color;}

	int f = 1 - radius;
	int ddF_x = 0;
	int ddF_y = -2 * radius;
	int x = 0;
	int y = radius;

	if((quad_flags & 2) || (quad_flags & 1)) plot(x0 + radius, y0); // right
	if((quad_flags & 2) || (quad_flags & 1)) plot(x0, y0 - radius); // Top
	if((quad_flags & 3) || (quad_flags & 2))plot(x0 - radius, y0); // left
	if((quad_flags & 4) || (quad_flags & 3))plot(x0, y0 + radius); // bottom

	while(x < y){
		if(f >= 0){
			y--;
			ddF_y += 2;
			f += ddF_y;
		}
		x++;
		ddF_x += 2;
		f += ddF_x + 1;    

		if((quad_flags & 1)){
			plot(x0 + y, y0 - x); // 1 , 1cuad
			plot(x0 + x, y0 - y); // 2 , 1cuad
		}
		if((quad_flags & 2)){
			plot(x0 - x, y0 - y); // 3 , 2cuad
			plot(x0 - y, y0 - x); // 4 , 2cuad
		}
		if((quad_flags & 4)){
			plot(x0 - y, y0 + x); // 5,  3cuad
			plot(x0 - x, y0 + y); // 6 , 3cuad
		}
		if((quad_flags & 8)){
			plot(x0 + x, y0 + y); // 7 , 4cuad
			plot(x0 + y, y0 + x); // 8,  4cuad
		}
	}
}

void ikigui_draw_gradient_4x(ikigui_image *dest, uint32_t color_top, uint32_t color_bot, uint32_t color_left, uint32_t color_right, ikigui_rect *part ){ /// Fill part of image or window with gradient.

	int x = part->x;
	int y = part->y;

        if((x<0) || (y<0))return;	  // sheilding crash
        if(dest->w < (x+part->w))return; // shielding crash
        if(dest->h < (y+part->h))return; // shielding crash

	uint8_t r1 = (color_bot&0xff0000)>>16;	// Red color_bot
	uint8_t g1 = (color_bot&0xff00)>>8;	// Green color_bot
	uint8_t b1 = color_bot&0xff;		// Blue color_bot
	uint8_t r2 = (color_top&0xff0000)>>16;	// Red color_top
	uint8_t g2 = (color_top&0xff00)>>8;	// Red color_top
	uint8_t b2 = color_top&0xff;		// Blue color_top

	uint8_t r3 = (color_left&0xff0000)>>16;	// Red color_top
	uint8_t g3 = (color_left&0xff00)>>8;	// Red color_top
	uint8_t b3 = color_left&0xff;		// Blue color_top
	uint8_t r4 = (color_right&0xff0000)>>16;// Red color_bot
	uint8_t g4 = (color_right&0xff00)>>8;	// Green color_bot
	uint8_t b4 = color_right&0xff;		// Blue color_bot


	double line_const = (double)255/(double)part->h;
	double column_const = (double)255/(double)part->w;

        for(int j = 0 ; j < part->h ; j++){ // vertical
		double rise = (double)j * line_const ;	// rising
		double fall = 255-rise;			// falling

                for(int i = 0 ; i < part->w ; i++){   // horizontal
			double right = (double)i * column_const ;// rising
			double left = 255-right;		// falling

			uint8_t ro = ((uint16_t)(rise*r1 + fall*r2 + left*r3 + right*r4))>>8;   // color_bot + color_top
			uint8_t go = ((uint16_t)(rise*g1 + fall*g2 + left*g3 + right*g4))>>8;   // color_bot + color_top
			uint8_t bo = ((uint16_t)(rise*b1 + fall*b2 + left*b3 + right*b4))>>8;   // color_bot + color_top

			dest->pixels[x+i+(j+y)*dest->w] = (255<<24) + (ro << 16) + (go<< 8) + bo;
                }
        }
}

void ikigui_draw_gradient(ikigui_image *dest, uint32_t color_top, uint32_t color_bot, ikigui_rect *part ){ /// Fill part of image or window with gradient.

	int x = part->x;
	int y = part->y;

        if((x<0) || (y<0))return; // sheilding crash
        if(dest->w < (x+part->w))return; // shielding crash
        if(dest->h < (y+part->h))return; // shielding crash

	uint8_t a1 = (color_bot&0xff000000)>>24;// alpha color_bot
	uint8_t r1 = (color_bot&0xff0000)>>16;	// Red color_bot
	uint8_t g1 = (color_bot&0xff00)>>8;	// Green color_bot
	uint8_t b1 = color_bot&0xff;		// Blue color_bot

	uint8_t a2 = (color_top&0xff000000)>>24;// alpha color_bot
	uint8_t r2 = (color_top&0xff0000)>>16;	// Red color_top
	uint8_t g2 = (color_top&0xff00)>>8;	// Red color_top
	uint8_t b2 = color_top&0xff;		// Blue color_top

	double line_const = (double)255/(double)part->h;
        for(int j = 0 ; j < part->h ; j++){ // vertical
		double rise = (double)j * line_const ;	// rising
		double fall = 255-rise;			// falling

                for(int i = 0 ; i < part->w ; i++){   // horizontal
			uint8_t ao = ((uint16_t)(rise*a1 + fall*a2))>>8;   // color_bot + color_top
			uint8_t ro = ((uint16_t)(rise*r1 + fall*r2))>>8;   // color_bot + color_top
			uint8_t go = ((uint16_t)(rise*g1 + fall*g2))>>8;   // color_bot + color_top
			uint8_t bo = ((uint16_t)(rise*b1 + fall*b2))>>8;   // color_bot + color_top

			dest->pixels[x+i+(j+y)*dest->w] = alpha_channel(dest->pixels[x+i+(j+y)*dest->w],(ao << 24) + (ro << 16) + (go<< 8) + bo);
                }
        }
}

void ikigui_blit_alpha(ikigui_image *dest,ikigui_image *source, int x, int y, ikigui_rect *part){ /// Draw source area specified by rect, to the x,y, coordinate in the destination.
        if((x<0) || (y<0))return; // sheilding crash
        if(dest->w < (x+part->w))return; // shelding crash
        if(dest->h < (y+part->h))return; // shelding crash
        for(int j = 0 ; j < part->h ; j++){ // vertical
                for(int i = 0 ; i < part->w ; i++){   // horizontal
                        dest->pixels[x+i+(j+y)*dest->w] = alpha_channel(dest->pixels[x+i+(j+y)*dest->w],source->pixels[i+part->x+source->w*(j+part->y)]);
                }
        }
}
void ikigui_blit_filled(ikigui_image *dest,ikigui_image *source, int x, int y, ikigui_rect *part){ /// Draw area - can be used if you whant to fill low alpha value with a solid color.
        if((x<0) || (y<0))return; // sheilding crash
        if(dest->w < (x+part->w))return; // shielding crash
        if(dest->h < (y+part->h))return; // shielding crash

        for(int j = 0 ; j < part->h ; j++){ // vertical
                for(int i = 0 ; i < part->w ; i++){   // horizontal
                        dest->pixels[x+i+(j+y)*dest->w] = alpha_channel(dest->color,source->pixels[i+part->x+source->w*(j+part->y)]);
                }
        }
}
void ikigui_blit_hollow(ikigui_image *dest,ikigui_image *source, int x, int y, ikigui_rect *part){ /// Draw area - can be used if you whant to fill high alpha value with a solid color.
        if((x<0) || (y<0))return; // sheilding crash
        if(dest->w < (x+part->w))return; // shielding crash
        if(dest->h < (y+part->h))return; // shielding crash

        for(int j = 0 ; j < part->h ; j++){ // vertical
                for(int i = 0 ; i < part->w ; i++){   // horizontal			
                	dest->pixels[x+i+(j+y)*dest->w] = alpha_channel(dest->pixels[x+i+(j+y)*dest->w],(source->color&0x00FFFFFF) | (0xFF000000 & source->pixels[i+part->x+source->w*(j+part->y)]));
                }
        }
}
void ikigui_blit_fast(ikigui_image *dest,ikigui_image *source, int x, int y, ikigui_rect *part){ /// Draw area - can be used if source has no alpha
        if((x<0) || (y<0))return; // sheilding crash
        if(dest->w < (x+part->w))return; // shelding crash
        if(dest->h < (y+part->h))return; // shelding crash
        for(int j = 0 ; j < part->h ; j++){ // vertical
                for(int i = 0 ; i < part->w ; i++){   // horizontal
                        dest->pixels[x+i+(j+y)*dest->w] = source->pixels[i+part->x+source->w*(j+part->y)];
                }
        }
}


void ikigui_draw_image(ikigui_image *dest,ikigui_image *source, int x, int y){ /// Draw area. Flexible to Blit in windows and pixel buffers.
        for(int j = 0 ; j < source->h ; j++){ // vertical
                for(int i = 0 ; i < source->w ; i++){   // horizontal         
                        dest->pixels[x+i+(j+y)*dest->w] = source->pixels[i+source->w*j];
                }
        }
}
void ikigui_draw_image_composite(ikigui_image *dest,ikigui_image *source, int x, int y){ /// Draw area. Flexible to Blit in windows and pixel buffers.
        for(int j = 0 ; j < source->h ; j++){ // vertical
                for(int i = 0 ; i < source->w ; i++){   // horizontal         
                        dest->pixels[x+i+(j+y)*dest->w] = alpha_channel(dest->pixels[x+i+(j+y)*dest->w],source->pixels[i+source->w*j]);
                }
        }
}

// Functions for image manipulation follows... 

void ikigui_image_solid(ikigui_image *dest, unsigned int color){ // Fill image or window.
        for(int i = 0 ; i < dest->w * dest->h ; i++){	dest->pixels[i] = color; }
}
void ikigui_image_solid_bg(ikigui_image *dest,unsigned int color){ /// A background color for automatic filling of transparent pixels.
        for(int i = 0 ; i < dest->size ; i++){		dest->pixels[i] = alpha_channel(color,dest->pixels[i]);	}
}
void ikigui_image_gradient(ikigui_image *dest, uint32_t color_top, uint32_t color_bot){ /// Fill image with a color gradient.
	ikigui_rect all = {.x=0,.y=0,.w= dest->w,.h= dest->h};
	ikigui_draw_gradient(dest,color_top,color_bot,&all);
}
void ikigui_image_create(ikigui_image *frame, uint32_t w,uint32_t h){ /// Allocate pixel memory for a ikigui_image
        frame->w = w;
        frame->h = h;
        frame->pixels = (unsigned int*)malloc(frame->w*frame->h*4);
        frame->size = frame->w * frame->h ;
}
void ikigui_include_bmp(ikigui_image *dest,const unsigned char* bmp_incl){ /// Read BMP image in header file
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
                }
        }
        dest->size = dest->w * dest->h ;
}

