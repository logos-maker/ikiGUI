/// @file ikigui.h Main file for ikiGUI. It has only platform agnostic/independent drawing rutines.

/// For a rectangle with position and dimensions
typedef struct{ 
        int x; ///< x coordinate for the upper left position (in pixels)
        int y; ///< y coordinate for the upper left position (in pixels)
        int w; ///< width of the rectangle (in pixels)
        int h; ///< hight of the rectangle (in pixels)
} ikigui_rect;

/// For a image with dimensions and pointer to pixel data
typedef struct {
	int w; ///< width of image (in pixels)
	int h; ///< hight of image (in pixels)
	unsigned int *pixels;   ///< pointer to pixel buffer in ARGB8888 format
        unsigned int size;      ///< the size of the buffer
        unsigned int color;  ///< color that may be used of tile map drawing for filling background
} ikigui_image;

#include <stdint.h>
#include <stdlib.h>
//#include "ikigui_regular.h" // Include platform agnostic/independent drawing rutines

#ifdef IKIGUI_STANDALONE
	#include <stdint.h>
	#include <stdio.h>
#endif
#ifdef IKIGUI_TTF
	#include "ikigui_ttf.h"
#endif
#ifdef IKIGUI_IMAGE
	#include "ikigui_image.h"
#endif
#ifdef IKIGUI_DIAL
	#include "ikigui_dial.h"
#endif

#ifdef IKIGUI_DRAW_ONLY // that declaration excludes all platform specific code, so it can be used for drawing into pixelbuffers only.
	void map_ikigui_to_sdl(ikigui_image * dest, Uint32 * pixels, int w , int h){ // One helper function that makes it easy to use with lib SDL
		dest->w = w;
		dest->h = h;
		dest->pixels = pixels; // copy pointer
	}
#else // Select the right platform specific files...
	#ifdef __linux__ //linux specific code goes here...
		#include "ikigui_lin.h"	// For window and graphics handling in this case for Linux.
	#elif _WIN32 // windows specific code goes here...
		#include "windows.h"
		#include "ikigui_win.h" // For window and graphics handling in this case for Windows.
	#endif
#endif

// ----------------------------------------------------------
//   Low level stuff like color, pixel and alpha operations

typedef uint32_t ikigui_color; /// An less unambiguous way to rembemer how to make a color variable and not color constants.
uint32_t ikigui_color_make(uint8_t a, uint8_t r, uint8_t g, uint8_t b){ return (a << 24) + (r << 16) + (g << 8) + (b << 0) ;} /// to make a color by it's separate components

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

uint32_t ikigui_pixel_get(ikigui_image* source,int x, int y){ return source->pixels[source->w * y + x];} 		/// Get pixel ARGB value
void     ikigui_pixel_set(ikigui_image* dest,int x, int y, uint32_t color){ dest->pixels[dest->w * y + x] = color;}	/// Set pixel ARGB value
void     ikigui_pixel_draw(ikigui_image* dest,int x, int y, uint32_t color){						/// Draw transparant pixel
	dest->pixels[dest->w * y + x] = alpha_channel(dest->pixels[dest->w * y + x],color);
}

// -----------------------------------------------------------------------------------
//   Tile related operations - Used in special cases of pure tile related operations 

void ikigui_draw_image(ikigui_image *dest,ikigui_image *source, int x, int y){ /// Draw area. Flexible to Blit in windows and pixel buffers.
	int inter_w = source->w ;
	int inter_h = source->h ;
	if(dest->w < (source->w + x)) inter_w = dest->w - (x) ; // clip off right  part of image and block crash
	if(dest->h < (source->h + y)) inter_h = dest->h - (y) ; // clip off bottom part of image and block crash

        for(int j = 0 ; j < inter_h ; j++){ // vertical
		for(int i = 0 ; i < inter_w ; i++){   // horizontal
                        dest->pixels[x+i+(j+y)*dest->w] = source->pixels[i+source->w*j];
                }
        }
}
void ikigui_draw_image_composite(ikigui_image *dest,ikigui_image *source, int x, int y){ /// Draw area. Flexible to Blit in windows and pixel buffers.
	int inter_w = source->w ;
	int inter_h = source->h ;
	if(dest->w < (source->w + x)) inter_w = dest->w - (x) ; // clip off right  part of image and block crash
	if(dest->h < (source->h + y)) inter_h = dest->h - (y) ; // clip off bottom part of image and block crash

	for(int j = 0 ; j < inter_h ; j++){ // vertical
                for(int i = 0 ; i < inter_w ; i++){   // horizontal         
                        dest->pixels[x+i+(j+y)*dest->w] = alpha_channel(dest->pixels[x+i+(j+y)*dest->w],source->pixels[i+source->w*j]);
                }
        }
}

// ---------------------------------------------------------
//   Pure tile related operations (they mostly need alpha)

void ikigui_tile_alpha(ikigui_image *dest,ikigui_image *source, int x, int y, ikigui_rect *part){ /// Draw source area specified by rect, to the destination, if part is NULL we print the whole image instead. 
	if(NULL == part){ ikigui_draw_image_composite(dest,source,x,y); return; } // if part is NULL we print the whole image instead. 
        if((x<0) || (y<0))return; // sheilding crash
        if(dest->w < (x+part->w))return; // shelding crash
        if(dest->h < (y+part->h))return; // shelding crash
        for(int j = 0 ; j < part->h ; j++){ // vertical
                for(int i = 0 ; i < part->w ; i++){   // horizontal
                        dest->pixels[x+i+(j+y)*dest->w] = alpha_channel(dest->pixels[x+i+(j+y)*dest->w],source->pixels[i+part->x+source->w*(j+part->y)]);
                }
        }
}
void ikigui_tile_filled(ikigui_image *dest,ikigui_image *source, int x, int y, ikigui_rect *part){ /// Draw area - can be used if you whant to fill low alpha value with a solid color.
        if((x<0) || (y<0))return; // sheilding crash
        if(dest->w < (x+part->w))return; // shielding crash
        if(dest->h < (y+part->h))return; // shielding crash

        for(int j = 0 ; j < part->h ; j++){ // vertical
                for(int i = 0 ; i < part->w ; i++){   // horizontal
                        dest->pixels[x+i+(j+y)*dest->w] = alpha_channel(dest->color,source->pixels[i+part->x+source->w*(j+part->y)]);
                }
        }
}
void ikigui_tile_hollow(ikigui_image *dest,ikigui_image *source, int x, int y, ikigui_rect *part){ /// Draw area - can be used if you whant to fill high alpha value with a solid color.
        if((x<0) || (y<0))return; // sheilding crash
        if(dest->w < (x+part->w))return; // shielding crash
        if(dest->h < (y+part->h))return; // shielding crash

        for(int j = 0 ; j < part->h ; j++){ // vertical
                for(int i = 0 ; i < part->w ; i++){   // horizontal			
                	dest->pixels[x+i+(j+y)*dest->w] = alpha_channel(dest->pixels[x+i+(j+y)*dest->w],(source->color&0x00FFFFFF) | (0xFF000000 & source->pixels[i+part->x+source->w*(j+part->y)]));
                }
        }
}
void ikigui_tile_fast(ikigui_image *dest,ikigui_image *source, int x, int y, ikigui_rect *part){ /// Draw area - can be used if source has no alpha
	if(NULL == part){ ikigui_draw_image(dest,source,x,y); return; }
        if((x<0) || (y<0))return; // sheilding crash
        if(dest->w < (x+part->w))return; // shelding crash
        if(dest->h < (y+part->h))return; // shelding crash
        for(int j = 0 ; j < part->h ; j++){ // vertical
                for(int i = 0 ; i < part->w ; i++){   // horizontal
                        dest->pixels[x+i+(j+y)*dest->w] = source->pixels[i+part->x+source->w*(j+part->y)];
                }
        }
}


// ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//   Map related operations - That need the pure tile related operations and alpha. For making tilemaps that select image parts from a tile atlas, for oldschool monospace characters and graphic tiles.

/// For a tile map that can be drawn to images or windows
typedef struct ikigui_map {
   ikigui_image   *dest;       ///< The destination image to use for drawing the character display
   ikigui_image   *source;     ///< The source image with graphics for tiles
   unsigned char  columns;     ///< Number of columns in the character display
   unsigned char  rows;        ///< Number of rows in the character display
   unsigned char  tile_width;  ///< The pixel width of the characters/tiles in the source image
   unsigned char  tile_hight;  ///< The pixel hight of the characters/tiles in the source image
   char		  *map;	       ///< The start address for the character display
   unsigned char  direction;   ///< The direction of the immages in the source image. Is autodetected by ikigui_map_init().
   uint16_t       max_index;   ///< The number of tiles in the map - 1.
   int		  x_spacing;   ///< The horizontal spacing between the left sides of the tiles in pixels.
   int		  y_spacing;   ///< The vertical spacing between the top sides of the tiles in pixels.
   signed char	  offset;      ///< The number offset for the numbers in the map. Can be used to fill it with ASCII text for example.
} ikigui_map;

enum offset {OFFSET_ASCII = -32, OFFSET_NORMAL = 0 }; // Used for setting the offset value in ikigui_map_inite later used by the ikigui_map_draw function.
/// To initialize ikigui_map structs and allocate memory for the char map
int ikigui_map_init(struct ikigui_map *display, ikigui_image *dest, ikigui_image *source,  int8_t offset, int x_spacing, int y_spacing, int width, int hight, int columns, int rows ){
   display->map = (char*)calloc(columns*rows,sizeof(char));
   display->offset = offset ;      // index value offset to all values used in the map array.
   display->tile_width = width ;   // The width of the tiles. 
   display->tile_hight = hight ;   // The higth of the tiles.
   display->x_spacing = x_spacing ;// The number of pixels between left most pixel in each tile.
   display->y_spacing = y_spacing ;// The number of pixels between top  most pixel in each tile.
   if(x_spacing == 0) display->x_spacing = display->tile_width;  // if x_spacing is zero, set it to the size of the tile so they are placed side by side.
   if(y_spacing == 0) display->y_spacing = display->tile_hight ; // if y_spacing is zero, set it to the size of the tile so they are placed side by side.
   display->columns = columns ;    // Set to a default value
   display->rows = rows ;          // Set to a default value
   display->max_index = (source->w / width) * (source->h / hight) - 1 ; //
   display->dest = dest ;          // Set to dest given as input by library user.
   display->source = source ;      // Set to source image given as input by library user.
   if(source->w == width) display->direction = 0; else display->direction = 1; // Automatic detection for tile-atlas direction (horizontal or vertical).
   return columns * rows ; // returns the total number of tiles in the character display
}

/// To free memory allocated to a ikigui_map with the ikigui_map_init function
void ikigui_map_free(struct ikigui_map *display){
   free(display->map); // Free memory that was allocated with ikigui init.
}
/// To check if a coordinate is inside a specific ikigui rect
int ikigui_mouse_pos_rect(ikigui_rect *box, int x, int y){ // is the x y coordinate inside the ikigui_rect
	if(x<box->x) return 0;
	if(x>(box->x+box->w)) return 0;
	if(y<box->y) return 0;
	if(y>(box->y+box->h)) return 0;
	return -1; // Return a hit/true value.
}
// A helper function
char* ikigui_map_layout(ikigui_map* find_text_pointer){ /// get the char* pointer to the array that is used for the tile/character array 
	return find_text_pointer->map;
}
// A helper function
void ikigui_rect_find(ikigui_rect *destin_rect, int x, int y, ikigui_rect *source_rect){ /// Fill in the parameters of the destin_rect. Convinience to automatically get the destination rect area for a hypthetical blit operation parameters.
	destin_rect->x = x ;
	destin_rect->y = y ;
	destin_rect->w = source_rect->w ;
	destin_rect->h = source_rect->h ; 
}
/// To give the index to a tile from a pixel coordinate
int ikigui_mouse_pos_map(struct ikigui_map *display, int x, int y){ // returns -1 if outside the character display
   if( (x < 0) || (y < 0) ) return -1;			// To the left or over the map
   if( ( x < (( display->x_spacing) * display->columns))// To the right of the map 
   &&  ( y < (( display->y_spacing) * display->rows))	// below the map 
   ){  
	int col = x / display->x_spacing ;
	int row = y / display->y_spacing ;
	if( (x < ( col * display->x_spacing  +display->tile_width)) && (y < ( row * display->y_spacing  +display->tile_hight)) ) // inside the tile
	return (col + (row * display->columns)); // Returns the index of the character in your character array.
   };
   return -1; // Pixel coordinate is outside of the character display. 
}

enum tile_type { TILE_APLHA = 0, TILE_FILLED = 1, TILE_SOLID = 2, TILE_HOLLOW = 3 }; /// Blit modes/types for ikigui_map_draw()
/// To draw a tile map to a window or image
void ikigui_map_draw(struct ikigui_map *display, char tile_type, int x, int y){  // x y is pixel coordinate to draw it in the window
   
   ikigui_rect srcrect = { .w = display->tile_width, .h = display->tile_hight }; // , .x = 0, .y = 0,  } ;
   ikigui_rect dstrect = { .w = display->tile_width, .h = display->tile_hight };

   int set_w;
   if(display->direction) set_w = srcrect.w ;
   else                   set_w = srcrect.h ;

   for(int i = 0 ; i < display->rows ; i++ ){	// draw all rows
      dstrect.y = i * display->y_spacing + y;
      for(int j = 0 ; j < display->columns ; j++ ){	// draw all columns
	  dstrect.x = (j * display->x_spacing) + x;
          int val = set_w * (display->map[i*display->columns + j] + display->offset) ;
	  if(val<0) continue ; // Don't draw tile if given lower value than 0. // val=0; 
	  if(display->direction)srcrect.x = val ; else srcrect.y = val ;
	  switch(tile_type){ // Draw the character buffer to window.
		  case 0:       ikigui_tile_alpha (display->dest,display->source, dstrect.x, dstrect.y, &srcrect);	break;
		  case 1:	ikigui_tile_filled(display->dest,display->source, dstrect.x, dstrect.y, &srcrect);	break;
		  case 2:	ikigui_tile_fast  (display->dest,display->source, dstrect.x, dstrect.y, &srcrect);	break;
		  case 3:	ikigui_tile_hollow(display->dest,display->source, dstrect.x, dstrect.y, &srcrect);	break;
	  }
      }
   }
}
/// To draw a tile map to a window or image, But firsts heals the background for retained mode
void ikigui_map_draw_healing(struct ikigui_map *display, ikigui_image *bg_source, char tile_type, int x, int y){  // x y is pixel coordinate to draw it in the window
   

   ikigui_rect srcrect = { .w = display->tile_width, .h = display->tile_hight }; // , .x = 0, .y = 0,  } ;
   ikigui_rect dstrect = { .w = display->tile_width, .h = display->tile_hight };

   // Heal background
   ikigui_rect bgrect  = { .w = display->x_spacing * display->columns, .h = display->y_spacing * display->rows, .x = x, .y = y };
   ikigui_tile_fast  (display->dest,bg_source, bgrect.x, bgrect.y, &bgrect); // Heal background

   int set_w;
   if(display->direction) set_w = srcrect.w ;
   else                   set_w = srcrect.h ;

   for(int i = 0 ; i < display->rows ; i++ ){	// draw all rows
      dstrect.y = i * display->y_spacing + y;
      for(int j = 0 ; j < display->columns ; j++ ){	// draw all columns
	  dstrect.x = (j * display->x_spacing) + x;
          int val = set_w * (display->map[i*display->columns + j] + display->offset) ;
	  if(val<0) continue ; // Don't draw tile if given lower value than 0. // val=0; 
	  if(display->direction)srcrect.x = val ; else srcrect.y = val ;
	  switch(tile_type){ // Draw the character buffer to window.
		  case 0:       ikigui_tile_alpha (display->dest,display->source, dstrect.x, dstrect.y, &srcrect);	break;
		  case 1:	ikigui_tile_filled(display->dest,display->source, dstrect.x, dstrect.y, &srcrect);	break;
		  case 2:	ikigui_tile_fast  (display->dest,display->source, dstrect.x, dstrect.y, &srcrect);	break;
		  case 3:	ikigui_tile_hollow(display->dest,display->source, dstrect.x, dstrect.y, &srcrect);	break;
	  }
      }
   }
}
/// To draw one tile in the tile map to a window or image but first draw a new background for that tile position (for retained mode)
void ikigui_map_draw_tile(struct ikigui_map *display, ikigui_image *bg_source, int index, char tile_type, int x, int y){ /// heal backgroud and draw the tile
   
	ikigui_rect srcrect = { .w = display->tile_width, .h = display->tile_hight }; // , .x = 0, .y = 0,  } ;
	ikigui_rect dstrect = { .w = display->tile_width, .h = display->tile_hight };

	int set_w;
	if(display->direction) set_w = srcrect.w ;
	else                   set_w = srcrect.h ;

	int i = index / display->columns ; // Row
	dstrect.y = i * display->y_spacing + y;
	int j = index % display->columns ; // Column
	dstrect.x = (j * display->x_spacing) + x;
	int val = set_w * (display->map[i*display->columns + j] + display->offset) ;
	if(val<0) return ; // Don't draw tile if given lower value than 0. // val=0; 
	if(display->direction)srcrect.x = val ; else srcrect.y = val ;
	ikigui_tile_fast  (display->dest,bg_source, dstrect.x, dstrect.y, &dstrect); // Heal background
	switch(tile_type){ // Draw the selected tile to window.
		case 0: ikigui_tile_alpha (display->dest,display->source, dstrect.x, dstrect.y, &srcrect);	break;
		case 1:	ikigui_tile_filled(display->dest,display->source, dstrect.x, dstrect.y, &srcrect);	break;
		case 2:	ikigui_tile_fast  (display->dest,display->source, dstrect.x, dstrect.y, &srcrect);	break;
		case 3:	ikigui_tile_hollow(display->dest,display->source, dstrect.x, dstrect.y, &srcrect);	break;
	}

}

// -------------------------------------------------------------------
//   Rect related operations - Drawing primitives using ikigui_rect 

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

	uint8_t a1 = (color_bot & 0xff000000)>>24;// alpha color_bot
	uint8_t r1 = (color_bot & 0xff0000)>>16;	// Red color_bot
	uint8_t g1 = (color_bot & 0xff00)>>8;	// Green color_bot
	uint8_t b1 =  color_bot & 0xff;		// Blue color_bot

	uint8_t a2 = (color_top & 0xff000000)>>24;// alpha color_bot
	uint8_t r2 = (color_top & 0xff0000)>>16;	// Red color_top
	uint8_t g2 = (color_top & 0xff00)>>8;	// Red color_top
	uint8_t b2 =  color_top & 0xff;		// Blue color_top

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

void ikigui_draw_panel(ikigui_image *dest, uint32_t color, uint32_t light, uint32_t shadow, ikigui_rect *part ){ /// A filled rect with shaddow
	int x = part->x;
	int y = part->y;

        if((x<0) || (y<0)) return; // sheilding crash
        if(dest->w < (x+part->w)) return; // shielding crash
        if(dest->h < (y+part->h)) return; // shielding crash

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

// ----------------------------------------
//   Drawing primitives - On pixel level


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

void ikigui_draw_line_bh(ikigui_image *dest, uint32_t color,int x0, int y0, int x1, int y1){
	//void plot(int x0,int y0){ dest->pixels[dest->w * y0 + x0] = color;}
	int dx, dy, p, x, y;

	dx=x1-x0;
	dy=y1-y0;

	x=x0;
	y=y0;

	p=2*dy-dx;
 
	while(x<x1){
		if(p>=0){
			ikigui_pixel_set(dest, x, y, color);
			y=y+1;
			p=p+2*dy-2*dx;
		}
		else{
			ikigui_pixel_set(dest, x, y, color);
			p=p+2*dy;
		}
		x=x+1;
	}
}

/*
#include <math.h>
void ikigui_draw_cubic_bezier(ikigui_image *dest, uint32_t color,
        unsigned int x1, unsigned int y1,
        unsigned int x2, unsigned int y2,
        unsigned int x3, unsigned int y3,
        unsigned int x4, unsigned int y4)
{
	void plot(int x0,int y0){ dest->pixels[dest->w * y0 + x0] = color;}

#define N_SEG 20 // number of segments for the curve
    unsigned int i;
    double pts[N_SEG+1][2];
    for (i=0; i <= N_SEG; ++i)
    {
        double t = (double)i / (double)N_SEG;

        double a = pow((1.0 - t), 3.0);
        double b = 3.0 * t * pow((1.0 - t), 2.0);
        double c = 3.0 * pow(t, 2.0) * (1.0 - t);
        double d = pow(t, 3.0);

        double x = a * x1 + b * x2 + c * x3 + d * x4;
        double y = a * y1 + b * y2 + c * y3 + d * y4;
        pts[i][0] = x;
        pts[i][1] = y;
    }
 

    // draw only points
    for (i=0; i <= N_SEG; ++i)
    {
        plot( pts[i][0],
              pts[i][1] );
    }
    // draw segments
    for (i=0; i < N_SEG; ++i){
        int j = i + 1;
	ikigui_draw_line(dest,color, pts[i][0], pts[i][1],pts[j][0], pts[j][1] );
    }
}
*/

void ikigui_draw_circle(ikigui_image *dest, uint32_t color, int x0, int y0, unsigned int radius){
	//void plot(int x0,int y0){ dest->pixels[dest->w * y0 + x0] = color;}

	int f = 1 - radius;
	int ddF_x = 0;
	int ddF_y = -2 * radius;
	int x = 0;
	int y = radius;

	ikigui_pixel_set(dest, x0 + radius, y0, color); // right  NE  
	ikigui_pixel_set(dest, x0, y0 - radius, color); // Top    NW
	ikigui_pixel_set(dest, x0 - radius, y0, color); // left   SW
	ikigui_pixel_set(dest, x0, y0 + radius, color); // bottom SE

	while(x < y){
		if(f >= 0){
			y--;
			ddF_y += 2;
			f += ddF_y;
		}
		x++;
		ddF_x += 2;
		f += ddF_x + 1;    

		ikigui_pixel_set(dest, x0 + y, y0 - x, color); // 1 , 1cuad //
		ikigui_pixel_set(dest, x0 + x, y0 - y, color); // 2 , 1cuad //
		ikigui_pixel_set(dest, x0 - x, y0 - y, color); // 3 , 2cuad //
		ikigui_pixel_set(dest, x0 - y, y0 - x, color); // 4 , 2cuad //
		ikigui_pixel_set(dest, x0 - y, y0 + x, color); // 5,  3cuad //
		ikigui_pixel_set(dest, x0 - x, y0 + y, color); // 6 , 3cuad //
		ikigui_pixel_set(dest, x0 + x, y0 + y, color); // 7 , 4cuad //
		ikigui_pixel_set(dest, x0 + y, y0 + x, color); // 8,  4cuad //		
	}
}


enum cuadrants{ NE = 1, NW = 2, SW = 4, SE = 8};
void ikigui_draw_circle_parts(ikigui_image *dest, int quad_flags, uint32_t color, int x0, int y0, unsigned int radius){

	int f = 1 - radius;
	int ddF_x = 0;
	int ddF_y = -2 * radius;
	int x = 0;
	int y = radius;

	if((quad_flags & 2) || (quad_flags & 1)) ikigui_pixel_set(dest, x0 + radius, y0, color); // right
	if((quad_flags & 2) || (quad_flags & 1)) ikigui_pixel_set(dest, x0, y0 - radius, color); // Top
	if((quad_flags & 3) || (quad_flags & 2)) ikigui_pixel_set(dest, x0 - radius, y0, color); // left
	if((quad_flags & 4) || (quad_flags & 3)) ikigui_pixel_set(dest, x0, y0 + radius, color); // bottom

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
			ikigui_pixel_set(dest, x0 + y, y0 - x, color); // 1 , 1cuad
			ikigui_pixel_set(dest, x0 + x, y0 - y, color); // 2 , 1cuad
		}
		if((quad_flags & 2)){
			ikigui_pixel_set(dest, x0 - x, y0 - y, color); // 3 , 2cuad
			ikigui_pixel_set(dest, x0 - y, y0 - x, color); // 4 , 2cuad
		}
		if((quad_flags & 4)){
			ikigui_pixel_set(dest, x0 - y, y0 + x, color); // 5,  3cuad
			ikigui_pixel_set(dest, x0 - x, y0 + y, color); // 6 , 3cuad
		}
		if((quad_flags & 8)){
			ikigui_pixel_set(dest, x0 + x, y0 + y, color); // 7 , 4cuad
			ikigui_pixel_set(dest, x0 + y, y0 + x, color); // 8,  4cuad
		}
	}
}


// ------------------------------------------------------------------------------
//   ikigui_image related operations - Drawing functions for image manipulation


void ikigui_image_solid(ikigui_image *dest, unsigned int color){ // Fill image or window.
        for(int i = 0 ; i < dest->w * dest->h ; i++){	dest->pixels[i] = color; }
}
void ikigui_image_solid_bg(ikigui_image *dest,unsigned int color){ /// Fill background in destination. Doesn't overwrite the image.
        for(int i = 0 ; i < dest->size ; i++){		dest->pixels[i] = alpha_channel(color,dest->pixels[i]);	}
}
void ikigui_image_gradient(ikigui_image *dest, uint32_t color_top, uint32_t color_bot){ /// Fill image with a color gradient.
	ikigui_rect all = {.x=0,.y=0,.w= dest->w,.h= dest->h};
	ikigui_draw_gradient(dest,color_top,color_bot,&all);
}
void ikigui_image_make(ikigui_image *frame, uint32_t w,uint32_t h){ /// Allocate pixel memory for a ikigui_image
        frame->w = w;
        frame->h = h;
        frame->pixels = (unsigned int*)malloc(frame->w*frame->h*4);
        frame->size = frame->w * frame->h ;
}
void ikigui_include_bmp(ikigui_image *dest,const unsigned char* bmp_incl){ /// Read BMP image in header file to a ikigui_image (and allocates memory for it)
        unsigned int start;
        dest->w = bmp_incl[0x12] + (bmp_incl[0x12+1]<<8) + (bmp_incl[0x12+2]<<16) + (bmp_incl[0x12+3]<<24);
        dest->h = bmp_incl[0x16] + (bmp_incl[0x16+1]<<8) + (bmp_incl[0x16+2]<<16) + (bmp_incl[0x16+3]<<24);
        start   = bmp_incl[0x0a] + (bmp_incl[0x0a+1]<<8) + (bmp_incl[0x0a+2]<<16) + (bmp_incl[0x0a+3]<<24);

	//uint32_t* pixels_to_free = dest->pixels ; 	
        dest->pixels = (unsigned int*)malloc(dest->w*dest->h*4); // Doesn't make sense if we read it in to a image that allready have allocated memory!!!
	//if(pixels_to_free != NULL)free(pixels_to_free);

        int counter = 0 ; 
        for(int j = dest->h -1 ; j >= 0 ; j--){
                for(int i = 0 ; i < dest->w ; i++){
                        int pixl_addr = (i+(j*dest->w))*4+start;
                        dest->pixels[counter++]= bmp_incl[pixl_addr]+ (bmp_incl[pixl_addr+1]<<8)+ (bmp_incl[pixl_addr+2]<<16)+ (bmp_incl[pixl_addr+3]<<24);
                }
        }
        dest->size = dest->w * dest->h ;
}

