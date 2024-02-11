typedef struct {
        int x;
        int y;
        int w;
        int h;
} ikigui_rect ;

typedef struct {
	int w; // width
	int h; // hight
	unsigned int *pixels;
        unsigned int size;
        unsigned int bg_color; // for filling background
} ikigui_image;

#ifdef IKIGUI_DRAW_ONLY
void map_ikigui_to_sdl(ikigui_image * mywin, Uint32 * pixels, int w , int h){
	mywin->w = w;
	mywin->h = h;
	mywin->pixels = pixels;
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

void ikigui_fill_bg(ikigui_image *frame,unsigned int color){// A background color for automatic filling of transparent pixels.
	// to precalc graphics for usage with ikigui_blit_part_fast() for faster graphics. Can be convinient in some cases.
        for(int i = 0 ; i < frame->size ; i++){
                frame->pixels[i] = alpha_channel(color,frame->pixels[i]);
        }
}

void ikigui_image_create(ikigui_image *frame, uint32_t w,uint32_t h){ // Allocate pixel memory for a ikigui_image
        frame->w = w;
        frame->h = h;
        frame->pixels = (unsigned int*)malloc(frame->w*frame->h*4);
        frame->size = frame->w * frame->h ;
}

void ikigui_bmp_include(ikigui_image *frame,const unsigned char* bmp_incl){
        unsigned int start;
        frame->w = bmp_incl[0x12] + (bmp_incl[0x12+1]<<8) + (bmp_incl[0x12+2]<<16) + (bmp_incl[0x12+3]<<24);
        frame->h = bmp_incl[0x16] + (bmp_incl[0x16+1]<<8) + (bmp_incl[0x16+2]<<16) + (bmp_incl[0x16+3]<<24);
        start =    bmp_incl[0x0a] + (bmp_incl[0x0a+1]<<8) + (bmp_incl[0x0a+2]<<16) + (bmp_incl[0x0a+3]<<24);

        frame->pixels = (unsigned int*)malloc(frame->w*frame->h*4);

        int counter = 0 ; 
        for(int j = frame->h -1 ; j >= 0 ; j--){
                for(int i = 0 ; i < frame->w ; i++){
                        int pixl_addr = (i+(j*frame->w))*4+start;
                        frame->pixels[counter++]= bmp_incl[pixl_addr]+ (bmp_incl[pixl_addr+1]<<8)+ (bmp_incl[pixl_addr+2]<<16)+ (bmp_incl[pixl_addr+3]<<24);
                }
        }
        frame->size = frame->w * frame->h ;
}

void ikigui_draw_panel(ikigui_image *mywin, uint32_t color, uint32_t light, uint32_t shadow, ikigui_rect *part ){ // A filled rect with shaddow
	int x = part->x;
	int y = part->y;

        if((x<0) || (y<0))return; // sheilding crash
        if(mywin->w < (x+part->w))return; // shielding crash
        if(mywin->h < (y+part->h))return; // shielding crash

        for(int j = 0 ; j < part->h ; j++){ // vertical

                for(int i = 0 ; i < part->w ; i++){   // horizontal
			mywin->pixels[x+i+(j+y)*mywin->w] = color ;
                }
	}

  for(int i = part->x ; i < (part->x+part->w) ; i++){   // horizontal
		mywin->pixels[i+part->y*mywin->w] = light;
        }

        for(int j = part->y ; j < (part->y + part->h -1) ; j++){ // vertical
		mywin->pixels[part->x+j*mywin->w] = light;
		mywin->pixels[(part->x+part->w-1)+j*mywin->w] = shadow;
	}

        for(int i = part->x; i < (part->x+part->w ); i++){   // horizontal
		mywin->pixels[i+(part->y+part->h-1)*mywin->w] = shadow;
        }
}
void ikigui_draw_bevel(ikigui_image *mywin, uint32_t color, uint32_t shadow, ikigui_rect *part ){ // A unfilled rect with shaddow
	int x = part->x;
	int y = part->y;

        if((x<0) || (y<0))return; // sheilding crash
        if(mywin->w < (x+part->w))return; // shielding crash
        if(mywin->h < (y+part->h))return; // shielding crash

        for(int i = part->x ; i < (part->x+part->w) ; i++){   // horizontal
		mywin->pixels[i+part->y*mywin->w] = color;
        }

        for(int j = part->y ; j < (part->y + part->h -1) ; j++){ // vertical
		mywin->pixels[part->x+j*mywin->w] = color;
		mywin->pixels[(part->x+part->w-1)+j*mywin->w] = shadow;
	}

        for(int i = part->x; i < (part->x+part->w ); i++){   // horizontal
		mywin->pixels[i+(part->y+part->h-1)*mywin->w] = shadow;
        }
}

void ikigui_draw_rect(ikigui_image *mywin, uint32_t color, ikigui_rect *part ){ // A unfilled rect (has alpha support)
	int x = part->x;
	int y = part->y;

        if((x<0) || (y<0))return; // sheilding crash
        if(mywin->w < (x+part->w))return; // shielding crash
        if(mywin->h < (y+part->h))return; // shielding crash
	if((color & 0xFF000000) != 0xFF000000){ // Do we use alpha? True if no alpha (alpha is set to 0xFF in the color value)
		for(int i = part->x ; i < (part->x+part->w) ; i++){   // horizontal
			mywin->pixels[i+part->y*mywin->w] = color;
		}

		for(int j = part->y ; j < (part->y + part->h -1) ; j++){ // vertical
			mywin->pixels[part->x+j*mywin->w] = color;
			mywin->pixels[(part->x+part->w-1)+j*mywin->w] = color;
		}

		for(int i = part->x; i < (part->x+part->w ); i++){   // horizontal
			mywin->pixels[i+(part->y+part->h-1)*mywin->w] = color;
		}
	}else{ // Draw With alpha
		for(int i = part->x ; i < (part->x+part->w) ; i++){   // horizontal
			mywin->pixels[i+part->y*mywin->w] = alpha_channel(mywin->pixels[i+part->y*mywin->w],color);
		}

		for(int j = part->y ; j < (part->y + part->h -1) ; j++){ // vertical
			mywin->pixels[part->x+j*mywin->w] = alpha_channel(mywin->pixels[part->x+j*mywin->w],color);
			mywin->pixels[(part->x+part->w-1)+j*mywin->w] = alpha_channel(mywin->pixels[(part->x+part->w-1)+j*mywin->w],color);
		}

		for(int i = part->x; i < (part->x+part->w ); i++){   // horizontal
			mywin->pixels[i+(part->y+part->h-1)*mywin->w] = alpha_channel(mywin->pixels[i+(part->y+part->h-1)*mywin->w],color);
		}

	}
}

void ikigui_draw_box(ikigui_image *mywin, uint32_t color, ikigui_rect *part ){ // To draw a filled rectangle (has alpha support)
	int x = part->x; // to make code easier to read
	int y = part->y; // to make code easier to read
        if((x<0) || (y<0))return; // sheilding crash
        if(mywin->w < (x+part->w))return; // shielding crash
        if(mywin->h < (y+part->h))return; // shielding crash
	if((color & 0xFF000000) != 0xFF000000){ // Do we use alpha? True if no alpha (alpha is set to 0xFF in the color value)
		for(int j = 0 ; j < part->h ; j++){ // vertical
		        for(int i = 0 ; i < part->w ; i++){   // horizontal
				mywin->pixels[x+i+(j+y)*mywin->w] = color ; // without alpha
		        }
		}
	}else{
		for(int j = 0 ; j < part->h ; j++){ // vertical
		        for(int i = 0 ; i < part->w ; i++){   // horizontal
				mywin->pixels[x+i+(j+y)*mywin->w] = alpha_channel( mywin->pixels[x+i+(j+y)*mywin->w], color ); // with alpha
		        }
		}
	}
}
void ikigui_draw_line_v(ikigui_image *canvas, uint32_t color, uint32_t x, uint32_t y, uint32_t length ){ // Draw vertical line (has alpha support)
	if((x<0||y<0))return; // crash blocking
	if((color & 0xFF000000) != 0xFF000000){ // Do we use alpha? True if no alpha (alpha is set to 0xFF in the color value)
		for(int i = y ; i < (y+length) ; i++){ // draw vertical line with alpha
			canvas->pixels[x + i * canvas->w] = alpha_channel(canvas->pixels[x + i * canvas->w],color);
		}
	}else{	// It's a solid color
		for(int i = y ; i < (y+length) ; i++){ // draw vertical line without alpha
			canvas->pixels[x + i * canvas->w] = color;
		}
	}
}
void ikigui_draw_line_h(ikigui_image *canvas, uint32_t color, uint32_t x, uint32_t y, uint32_t length ){ // Draw horizontal line (has alpha support)
	if((x<0||y<0))return; // crash blocking
	if((color & 0xFF000000) != 0xFF000000){ // Do we use alpha? True if no alpha (alpha is set to 0xFF in the color value)
		for(int i = x ; i < (x+length) ; i++){ // draw horizontal line with alpha
			canvas->pixels[i + y * canvas->w] = alpha_channel(canvas->pixels[i + y * canvas->w],color);
		}
	}
	else{	// It's a solid color
		for(int i = x ; i < (x+length) ; i++){ // draw horizontal line without alpha
			canvas->pixels[i + y * canvas->w] = color;
		}
	}
}

void ikigui_draw_gradient_4x(ikigui_image *mywin, uint32_t color_top, uint32_t color_bot, uint32_t color_left, uint32_t color_right, ikigui_rect *part ){ // Fill part of image or window with gradient.

	int x = part->x;
	int y = part->y;

        if((x<0) || (y<0))return;	  // sheilding crash
        if(mywin->w < (x+part->w))return; // shielding crash
        if(mywin->h < (y+part->h))return; // shielding crash

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

			mywin->pixels[x+i+(j+y)*mywin->w] = (255<<24) + (ro << 16) + (go<< 8) + bo;
                }
        }
}

void ikigui_draw_gradient(ikigui_image *mywin, uint32_t color_top, uint32_t color_bot, ikigui_rect *part ){ // Fill part of image or window with gradient.

	int x = part->x;
	int y = part->y;

        if((x<0) || (y<0))return; // sheilding crash
        if(mywin->w < (x+part->w))return; // shielding crash
        if(mywin->h < (y+part->h))return; // shielding crash

	uint8_t r1 = (color_bot&0xff0000)>>16;	// Red color_bot
	uint8_t g1 = (color_bot&0xff00)>>8;	// Green color_bot
	uint8_t b1 = color_bot&0xff;		// Blue color_bot
	uint8_t r2 = (color_top&0xff0000)>>16;	// Red color_top
	uint8_t g2 = (color_top&0xff00)>>8;	// Red color_top
	uint8_t b2 = color_top&0xff;		// Blue color_top


	double line_const = (double)255/(double)part->h;
        for(int j = 0 ; j < part->h ; j++){ // vertical
		double rise = (double)j * line_const ;	// rising
		double fall = 255-rise;			// falling

                for(int i = 0 ; i < part->w ; i++){   // horizontal
			uint8_t ro = ((uint16_t)(rise*r1 + fall*r2))>>8;   // color_bot + color_top
			uint8_t go = ((uint16_t)(rise*g1 + fall*g2))>>8;   // color_bot + color_top
			uint8_t bo = ((uint16_t)(rise*b1 + fall*b2))>>8;   // color_bot + color_top

			mywin->pixels[x+i+(j+y)*mywin->w] = (255<<24) + (ro << 16) + (go<< 8) + bo;
                }
        }
}
void ikigui_alpha_gradient(ikigui_image *mywin, uint32_t color_top, uint32_t color_bot, ikigui_rect *part ){ // Fill part of image or window with gradient.

	int x = part->x;
	int y = part->y;

        if((x<0) || (y<0))return; // sheilding crash
        if(mywin->w < (x+part->w))return; // shielding crash
        if(mywin->h < (y+part->h))return; // shielding crash

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

			mywin->pixels[x+i+(j+y)*mywin->w] = alpha_channel(mywin->pixels[x+i+(j+y)*mywin->w],(ro << 24) + (ro << 16) + (go<< 8) + bo);
                }
        }
}

void ikigui_blit_alpha(ikigui_image *mywin,ikigui_image *frame, int x, int y, ikigui_rect *part){ // Draw area
        if((x<0) || (y<0))return; // sheilding crash
        if(mywin->w < (x+part->w))return; // shelding crash
        if(mywin->h < (y+part->h))return; // shelding crash
        for(int j = 0 ; j < part->h ; j++){ // vertical
                for(int i = 0 ; i < part->w ; i++){   // horizontal
                        mywin->pixels[x+i+(j+y)*mywin->w] = alpha_channel(mywin->pixels[x+i+(j+y)*mywin->w],frame->pixels[i+part->x+frame->w*(j+part->y)]);
                }
        }
}
void ikigui_blit_filled(ikigui_image *mywin,ikigui_image *frame, int x, int y, ikigui_rect *part){ // Draw area - can be used if you whant to fill low alpha value with a solid color.
        if((x<0) || (y<0))return; // sheilding crash
        if(mywin->w < (x+part->w))return; // shielding crash
        if(mywin->h < (y+part->h))return; // shielding crash

        for(int j = 0 ; j < part->h ; j++){ // vertical
                for(int i = 0 ; i < part->w ; i++){   // horizontal
                        mywin->pixels[x+i+(j+y)*mywin->w] = alpha_channel(mywin->bg_color,frame->pixels[i+part->x+frame->w*(j+part->y)]);
                }
        }
}
void ikigui_blit_hollow(ikigui_image *mywin,ikigui_image *frame, int x, int y, ikigui_rect *part){ // Draw area - can be used if you whant to fill high alpha value with a solid color.
        if((x<0) || (y<0))return; // sheilding crash
        if(mywin->w < (x+part->w))return; // shielding crash
        if(mywin->h < (y+part->h))return; // shielding crash

        for(int j = 0 ; j < part->h ; j++){ // vertical
                for(int i = 0 ; i < part->w ; i++){   // horizontal			
                	mywin->pixels[x+i+(j+y)*mywin->w] = alpha_channel(mywin->pixels[x+i+(j+y)*mywin->w],(frame->bg_color&0x00FFFFFF) | (0xFF000000 & frame->pixels[i+part->x+frame->w*(j+part->y)]));
                }
        }
}
void ikigui_blit_fast(ikigui_image *mywin,ikigui_image *frame, int x, int y, ikigui_rect *part){ // Draw area - can be used if source has no alpha
        if((x<0) || (y<0))return; // sheilding crash
        if(mywin->w < (x+part->w))return; // shelding crash
        if(mywin->h < (y+part->h))return; // shelding crash
        for(int j = 0 ; j < part->h ; j++){ // vertical
                for(int i = 0 ; i < part->w ; i++){   // horizontal
                        mywin->pixels[x+i+(j+y)*mywin->w] = frame->pixels[i+part->x+frame->w*(j+part->y)];
                }
        }
}
void ikigui_image_draw(ikigui_image *mywin,ikigui_image *frame, int x, int y){ // Draw area. Flexible to Blit in windows and pixel buffers. Can be optimized greatly.
        for(int j = 0 ; j < frame->h ; j++){ // vertical
                for(int i = 0 ; i < frame->w ; i++){   // horizontal         
                        mywin->pixels[x+i+(j+y)*mywin->w] = frame->pixels[i+frame->w*j];
                }
        }
}
void ikigui_image_composite(ikigui_image *mywin,ikigui_image *frame, int x, int y){ // Draw area. Flexible to Blit in windows and pixel buffers. Can be optimized greatly.
        for(int j = 0 ; j < frame->h ; j++){ // vertical
                for(int i = 0 ; i < frame->w ; i++){   // horizontal         
                        mywin->pixels[x+i+(j+y)*mywin->w] = alpha_channel(mywin->pixels[x+i+(j+y)*mywin->w],frame->pixels[i+frame->w*j]);
                }
        }
}
void ikigui_image_solid(ikigui_image *mywin, unsigned int color){ // Fill image or window.
        for(int i = 0 ; i < mywin->w * mywin->h ; i++){ // All pixels      
                        mywin->pixels[i] = color;
        }
}
void ikigui_image_gradient(ikigui_image *mywin, uint32_t color_top, uint32_t color_bot){ // Fill image or window.
	double line_const = (double)255/(double)mywin->h;
        for(int j = 0 ; j < mywin->h ; j++){ // vertical
		double rise = (double)j * line_const ;	// rising
		double fall = 255-rise;			// falling

                for(int i = 0 ; i < mywin->w ; i++){   // horizontal
			uint8_t r1 = (color_bot&0xff0000)>>16;	// Red color_bot
			uint8_t g1 = (color_bot&0xff00)>>8;	// Green color_bot
			uint8_t b1 = color_bot&0xff;		// Blue color_bot
			uint8_t r2 = (color_top&0xff0000)>>16;	// Red color_top
			uint8_t g2 = (color_top&0xff00)>>8;	// Red color_top
			uint8_t b2 = color_top&0xff;		// Blue color_top

			uint8_t ro = ((uint16_t)(rise*r1 + fall*r2))>>8;   // color_bot + color_top
			uint8_t go = ((uint16_t)(rise*g1 + fall*g2))>>8;   // color_bot + color_top
			uint8_t bo = ((uint16_t)(rise*b1 + fall*b2))>>8;   // color_bot + color_top

			mywin->pixels[i+j*mywin->w] = (255<<24) + (ro << 16) + (go<< 8) + bo;
                }
        }
}