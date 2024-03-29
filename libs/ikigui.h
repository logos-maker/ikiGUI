/// @file ikigui.h Main file for ikiGUI. It has only platform agnostic/independent drawing rutines.

/// For a rectangle with position and dimensions
typedef struct{ 
        int x; ///< x coordinate for the upper left position (in pixels)
        int y; ///< y coordinate for the upper left position (in pixels)
        int w; ///< width of the rectangle (in pixels)
        int h; ///< hight of the rectangle (in pixels)
} ikigui_rect;

enum pixel_format { IKI_ARGB = 0, IKI_RGBA = 1, IKI_ARGB_PREMUL = 2}; /// ARGB is the native format for ikiGUI. ARGB_PREMUL can be used with eg. Cairo (CAIRO_FORMAT_ARGB32). And RGBA with PlutoVG and PlutoSVG. 
/// For a image with dimensions and pointer to pixel data
typedef struct {
	int w; ///< width of image (in pixels)
	int h; ///< hight of image (in pixels)
	unsigned int *pixels; ///< pointer to pixel buffer in ARGB8888 format
        unsigned int size;    ///< the size of the buffer
        unsigned int color;   ///< color that may be used of tile map drawing for filling background
	char pixel_format;    // Not used by anything yet (may be used by map functions to use different sourceformats later on).
} ikigui_image;

#include <stdint.h>
#include <stdlib.h>

#ifdef IKIGUI_STANDALONE
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

/// ARGB32 Color variable definition.
typedef uint32_t ikigui_color;

/// Make a color by it's separate components
uint32_t ikigui_color_make(uint8_t a, uint8_t r, uint8_t g, uint8_t b){ return (a << 24) + (r << 16) + (g << 8) + (b << 0) ;}

/// Mix 2 colors in ARGB format and return the new color
unsigned int alpha_channel(unsigned int color_bg,unsigned int color_fg){ // Internal for usage in other functions (done with fixed point math).	uint8_t a1 = (color_bot & 0xff000000)>>24;// alpha color_bot
	unsigned char alpha = color_fg >> 24;		// Alpha channel
	unsigned char alpha_inv = ~alpha;		// ~alpha;
	unsigned char af = (color_fg & 0xff000000)>>24;
	unsigned char rf = (color_fg & 0xff0000)>>16;	// Red forground
	unsigned char gf = (color_fg & 0xff00)>>8;	// Green forground
	unsigned char bf =  color_fg & 0xff;		// Blue forground
	unsigned char ab = (color_bg & 0xff000000)>>24;
	unsigned char rb = (color_bg & 0xff0000)>>16;	// Red beckground
	unsigned char gb = (color_bg & 0xff00)>>8;	// Red background
	unsigned char bb =  color_bg & 0xff;		// Blue background                
	unsigned char ro = (alpha_inv * rb + alpha * rf)>>8;   // background + forground
	unsigned char go = (alpha_inv * gb + alpha * gf)>>8;   // background + forground
	unsigned char bo = (alpha_inv * bb + alpha * bf)>>8;   // background + forground
	unsigned char ao = ((uint16_t)(alpha_inv*ab + alpha*af))>>8;   // background + forground
	return (unsigned int)((ao << 24)+(ro << 16) + (go<< 8) + bo); 
}

/// Get pixel ARGB value
uint32_t ikigui_pixel_get(ikigui_image* source, int x, int y){ 
	return source->pixels[source->w * y + x];
} 		
/// Set pixel ARGB value
void     ikigui_pixel_set(ikigui_image* dest, uint32_t color, int x, int y){
	dest->pixels[dest->w * y + x] = color;
}	
/// Draw transparent pixel
void     ikigui_pixel_draw(ikigui_image* dest, uint32_t color, int x, int y){
	dest->pixels[dest->w * y + x] = alpha_channel(dest->pixels[dest->w * y + x], color);
}

// -----------------------------------------------------------------------------------
//   Tile related operations - Used in special cases of pure tile related operations 

/// Draw area. Flexible to Blit in windows and pixel buffers.
void ikigui_draw_image(ikigui_image *dest,ikigui_image *source, int x, int y){ 
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
/// Draw area. Flexible to Blit in windows and pixel buffers.
void ikigui_draw_image_composite(ikigui_image *dest,ikigui_image *source, int x, int y){
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

/// Draw source area specified by rect, to the destination, if part is NULL we print the whole image instead. 
void ikigui_tile_alpha(ikigui_image *dest,ikigui_image *source, int x, int y, ikigui_rect *part){
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
/// Draw area - can be used if you whant to fill low alpha value with a solid color.
void ikigui_tile_filled(ikigui_image *dest,ikigui_image *source, int x, int y, ikigui_rect *part){
        if((x<0) || (y<0))return; // sheilding crash
        if(dest->w < (x+part->w))return; // shielding crash
        if(dest->h < (y+part->h))return; // shielding crash

        for(int j = 0 ; j < part->h ; j++){ // vertical
                for(int i = 0 ; i < part->w ; i++){   // horizontal
                        dest->pixels[x+i+(j+y)*dest->w] = alpha_channel(dest->color,source->pixels[i+part->x+source->w*(j+part->y)]);
                }
        }
}
/// Draw area - can be used if you whant to fill high alpha value with a solid color.
void ikigui_tile_hollow(ikigui_image *dest,ikigui_image *source, int x, int y, ikigui_rect *part){
        if((x<0) || (y<0))return; // sheilding crash
        if(dest->w < (x+part->w))return; // shielding crash
        if(dest->h < (y+part->h))return; // shielding crash

        for(int j = 0 ; j < part->h ; j++){ // vertical
                for(int i = 0 ; i < part->w ; i++){   // horizontal			
                	dest->pixels[x+i+(j+y)*dest->w] = alpha_channel(dest->pixels[x+i+(j+y)*dest->w],(source->color&0x00FFFFFF) | (0xFF000000 & source->pixels[i+part->x+source->w*(j+part->y)]));
                }
        }
}
/// Draw area - can be used if source has no alpha
void ikigui_tile_fast(ikigui_image *dest,ikigui_image *source, int x, int y, ikigui_rect *part){
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

// -----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//   Tile map related operations - That need the pure tile related operations and alpha. For making tilemaps that select image parts from a tile atlas, for oldschool monospace characters and graphic tiles.

/// For a tile map that can be drawn to images or windows, needs initiation by ikigui_map_init or manually 
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

/// tile map offsets for ikigui_map_draw()
enum map_offset {OFFSET_ASCII = -32, OFFSET_NORMAL = 0 }; // Used for setting the offset value in ikigui_map_inite later used by the ikigui_map_draw function.

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

/// To initialize ikigui_map structs with less parameters and allocate memory for the char map
int ikigui_map_init_simple(struct ikigui_map *display, ikigui_image *dest, ikigui_image *source, int width, int hight, int columns, int rows ){
   display->map = (char*)calloc(columns*rows,sizeof(char));
   display->offset = 0 ;      // index value offset to all values used in the map array.
   display->tile_width = width ;   // The width of the tiles. 
   display->tile_hight = hight ;   // The higth of the tiles.
   display->x_spacing = width ;    // The number of pixels between left most pixel in each tile.
   display->y_spacing = width ;    // The number of pixels between top  most pixel in each tile.
   display->columns = columns ;    // Set to a default value
   display->rows = rows ;          // Set to a default value
   display->max_index = (source->w / width) * (source->h / hight) - 1 ; //
   display->dest = dest ;          // Set to dest given as input by library user.
   display->source = source ;      // Set to source image given as input by library user.
   if(source->w == width) display->direction = 0; else display->direction = 1; // Automatic detection for tile-atlas direction (horizontal or vertical).
   return columns * rows ; // returns the total number of tiles in the character display
}

/// To copy spacing from another ikigui map. E.g. you use ikigui_map_init_simple() and want to use the same spacing as another ikigui_map
int ikigui_map_spacing_copy(struct ikigui_map *dest, struct ikigui_map *source){ // For making programs with more readable code.
	dest->x_spacing = source->x_spacing;
	dest->y_spacing = source->x_spacing;
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

/// get the char* pointer to the array that is used for the tile/character array
char* ikigui_map_layout(ikigui_map* find_text_pointer){ // A helper function
	return find_text_pointer->map;
}
/// Fill in the parameters of the destin_rect. Convinience to automatically get the destination rect area for a hypthetical blit operation parameters.
void ikigui_rect_find(ikigui_rect *destin_rect, int x, int y, ikigui_rect *source_rect){ // A helper function
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
	if( (x < ( col * display->x_spacing + display->tile_width)) && (y < ( row * display->y_spacing + display->tile_hight)) ) // inside the tile
	return (col + (row * display->columns)); // Returns the index of the character in your character array.
   };
   return -1; // Pixel coordinate is outside of the character display. 
}

/// Blit modes/types for ikigui_map_draw()
enum tile_type { TILE_APLHA = 0, TILE_FILLED = 1, TILE_SOLID = 2, TILE_HOLLOW = 3 }; 
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
void ikigui_map_draw_healing(struct ikigui_map *display, ikigui_image *bg_source, char tile_type, int x, int y){  // Draw tilemap and heal background before doing it, x y is pixel coordinate to draw it in the window

   ikigui_rect srcrect = { .w = display->tile_width, .h = display->tile_hight }; // , .x = 0, .y = 0,  } ;
   ikigui_rect dstrect = { .w = display->tile_width, .h = display->tile_hight };

   // Heal background
   ikigui_rect bgrect  = {  .x = x, .y = y, .w = display->x_spacing * display->columns, .h = display->y_spacing * display->rows};
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
void ikigui_map_draw_tile(struct ikigui_map *display, ikigui_image *bg_source, int index, char tile_type, int x, int y){ // Draw tile after healing background
   
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

/// Draw gradient in part of image or window image.
void ikigui_draw_gradient_4x(ikigui_image *dest, uint32_t color_top, uint32_t color_bot, uint32_t color_left, uint32_t color_right, ikigui_rect *part ){ 

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

/// Draw gradient in part of image or window image.
void ikigui_draw_gradient(ikigui_image *dest, uint32_t color_top, uint32_t color_bot, ikigui_rect *part ){ 

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

/// Draww a filled rect with shaddow.
void ikigui_draw_panel(ikigui_image *dest, uint32_t color, uint32_t light, uint32_t shadow, ikigui_rect *part ){ 
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
/// Draw a unfilled rect with shaddow.
void ikigui_draw_bevel(ikigui_image *dest, uint32_t color, uint32_t shadow, ikigui_rect *part ){ 
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
/// Draw a unfilled rect (has alpha support).
void ikigui_draw_rect(ikigui_image *dest, uint32_t color, ikigui_rect *part ){ 
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
/// Draw a filled rectangle (has alpha support)
void ikigui_draw_box_simple(ikigui_image *dest, uint32_t color, ikigui_rect *part ){ 
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

/// Draw vertical line fast (has alpha support), can't draw backwards
void ikigui_draw_line_v1y(ikigui_image *dest, uint32_t color, uint32_t x, uint32_t y, int32_t length ){ 
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
/// Draw vertical line fast (has alpha support), can't draw backwards
void ikigui_draw_line_v2y(ikigui_image *dest, uint32_t color, uint32_t x, uint32_t y1, uint32_t y2){ 
	if((x<0||y1<0||y2<dest->w))return; // crash blocking
	if((color & 0xFF000000) != 0xFF000000){ // Do we use alpha? True if no alpha (alpha is set to 0xFF in the color value)
		for(int i = y1 ; i <= y2 ; i++){ // draw vertical line with alpha
			dest->pixels[x + i * dest->w] = alpha_channel(dest->pixels[x + i * dest->w],color);
		}
	}else{	// It's a solid color
		for(int i = y1 ; i <= y2 ; i++){ // draw vertical line without alpha
			dest->pixels[x + i * dest->w] = color;
		}
	}
}
/// Draw horizontal line fast, 1 x coordinate and length (has alpha support), can't draw backwards
void ikigui_draw_line_h1x(ikigui_image *dest, uint32_t color, uint32_t x, uint32_t y, int32_t length ){ 
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
/// Draw horizontal line fast, 2 x coordinates (has alpha support), can't draw backwards
void ikigui_draw_line_h2x(ikigui_image *dest, uint32_t color, uint32_t x1, uint32_t x2, uint32_t y){ 
	if((x1<0||y<0))return; // crash blocking
	if((color & 0xFF000000) != 0xFF000000){ // Do we use alpha? True if no alpha (alpha is set to 0xFF in the color value)
		for(int i = x1 ; i <= x2 ; i++){ // draw horizontal line with alpha
			dest->pixels[i + y * dest->w] = alpha_channel(dest->pixels[i + y * dest->w],color);
		}
	}
	else{	// It's a solid color
		for(int i = x1 ; i <= x2 ; i++){ // draw horizontal line without alpha
			dest->pixels[i + y * dest->w] = color;
		}
	}
}
/// Draw line between two x,y coordinates.
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

/// Draw a rounded rectangle, with radius setting. It uses ikigui_draw_box_simple() if radius is 0.
void ikigui_draw_box(ikigui_image *dest, uint32_t color, ikigui_rect *part, unsigned int radius){
	if(radius==0) ikigui_draw_box_simple(dest, color, part);

	int w_middle = (part->w - radius * 2) -1;
	int h_middle = (part->h - radius * 2) ;
	uint32_t y0 = part->y + 1 + radius -1;
	uint32_t x0 = part->x + 1 + radius; 

	int f = 1 - radius;
	int ddF_x = 0;
	int ddF_y = -2 * radius;
	int x = 0;
	int y = radius;

	ikigui_color shaddow = ((color & 0xFE000000)>>1) | (color & 0x00FFFFFF);

	while(x < y){
		if(f >= 0){
			y--;
			ddF_y += 2;
			f += ddF_y;
		}
		x++;
		ddF_x += 2;
		f += ddF_x + 1;    

		// Top part of circle
		ikigui_draw_line_h2x(dest, color, x0, x0 + y + w_middle-2, y0 - x); // 1 , 1cuad //
		ikigui_draw_line_h2x(dest, color, x0, x0 + x + w_middle-2, y0 - y); // 2 , 1cuad //
		ikigui_pixel_draw(dest, shaddow, x0 + y +w_middle-1, y0 - x); // 1 , 1cuad //
		ikigui_pixel_draw(dest, shaddow, x0 + x +w_middle-1, y0 - y); // 2 , 1cuad //
		ikigui_draw_line_h2x(dest, color, x0 - x,   x0, y0 - y); // 3 , 2cuad //
		ikigui_draw_line_h2x(dest, color, x0 - y,   x0, y0 - x); // 4 , 2cuad //
		ikigui_pixel_draw(dest, shaddow, x0 - x-1, y0 - y); // 3 , 2cuad //
		ikigui_pixel_draw(dest, shaddow, x0 - y-1, y0 - x); // 4 , 2cuad //
		
		// Bottom part of circle
		ikigui_draw_line_h2x(dest, color, x0 - y,   x0, y0 + x + h_middle -1); // 5,  3cuad //
		ikigui_draw_line_h2x(dest, color, x0 - x,   x0, y0 + y + h_middle -1); // 6 , 3cuad //
		ikigui_pixel_draw(dest, shaddow, x0 - y-1, y0 + x + h_middle-1); // 5,  3cuad //
		ikigui_pixel_draw(dest, shaddow, x0 - x-1, y0 + y + h_middle-1); // 6 , 3cuad //
		ikigui_draw_line_h2x(dest, color, x0, x0 + x +w_middle-2, y0 + y+ h_middle-1); // 7 , 4cuad //
		ikigui_draw_line_h2x(dest, color, x0, x0 + y +w_middle-2, y0 + x+ h_middle-1); // 8,  4cuad //
		ikigui_pixel_draw(dest, shaddow, x0   + x+1 +w_middle-2, y0 + y+ h_middle-1); // 7 , 4cuad //
		ikigui_pixel_draw(dest, shaddow, x0   + y+1 +w_middle-2, y0 + x+ h_middle-1); // 8,  4cuad //
	}

	for(int i = y0 ; i < (h_middle + y0 ) ; i++){
		ikigui_draw_line_h1x(dest, color, x0-radius, i, part->w-2);
	}

	ikigui_draw_line_v1y(dest, shaddow, x0 - radius -1           , y0 ,h_middle);	// Left anti-a
	ikigui_draw_line_v1y(dest, shaddow, x0 + w_middle + radius-1 , y0 ,h_middle);	// Right anti-a
}

/// Draw a filled circle from it's x,y center, with radius setting.
void ikigui_draw_ball(ikigui_image *dest, uint32_t color, int x0, int y0, unsigned int radius){ 
	int f = 1 - radius;
	int ddF_x = 0;
	int ddF_y = -2 * radius;
	int x = 0;
	int y = radius;

	ikigui_draw_line_h2x(dest, color, x0, x0 + radius, y0); // right to center
	ikigui_draw_line_h2x(dest, color, x0 - radius, x0, y0); // left   to center 
	ikigui_pixel_set(dest, color, x0 + radius, y0); // right  NE  
	ikigui_pixel_set(dest, color, x0, y0 - radius); // Top    NW
	ikigui_pixel_set(dest, color, x0 - radius, y0); // left   SW
	ikigui_pixel_set(dest, color, x0, y0 + radius); // bottom SE

	ikigui_color shaddow = ((color & 0xFE000000)>>1) + (color & 0x00FFFFFF);

	while(x < y){
		if(f >= 0){
			y--;
			ddF_y += 2;
			f += ddF_y;
		}
		x++;
		ddF_x += 2;
		f += ddF_x + 1;    

		ikigui_draw_line_h2x(dest, color, x0, x0 + y-1, y0 - x); // 1 , 1cuad //
		ikigui_draw_line_h2x(dest, color, x0, x0 + x-1, y0 - y); // 2 , 1cuad //
		ikigui_draw_line_h2x(dest, color, x0 - x+1, x0, y0 - y); // 3 , 2cuad //
		ikigui_draw_line_h2x(dest, color, x0 - y+1, x0, y0 - x); // 4 , 2cuad //
		ikigui_draw_line_h2x(dest, color, x0 - y+1, x0, y0 + x); // 5,  3cuad //
		ikigui_draw_line_h2x(dest, color, x0 - x+1, x0, y0 + y); // 6 , 3cuad //
		ikigui_draw_line_h2x(dest, color, x0, x0 + x-1, y0 + y); // 7 , 4cuad //
		ikigui_draw_line_h2x(dest, color, x0, x0 + y-1, y0 + x); // 8,  4cuad //

		ikigui_pixel_draw(dest, shaddow, x0 + y, y0 - x); // 1 , 1cuad //
		ikigui_pixel_draw(dest, shaddow, x0 + x, y0 - y); // 2 , 1cuad //
		ikigui_pixel_draw(dest, shaddow, x0 - x, y0 - y); // 3 , 2cuad //
		ikigui_pixel_draw(dest, shaddow, x0 - y, y0 - x); // 4 , 2cuad //
		ikigui_pixel_draw(dest, shaddow, x0 - y, y0 + x); // 5,  3cuad //
		ikigui_pixel_draw(dest, shaddow, x0 - x, y0 + y); // 6 , 3cuad //
		ikigui_pixel_draw(dest, shaddow, x0 + x, y0 + y); // 7 , 4cuad //
		ikigui_pixel_draw(dest, shaddow, x0 + y, y0 + x); // 8,  4cuad //	
	}
}
/// Draw a circle from it's x,y center, with radius setting
void ikigui_draw_circle(ikigui_image *dest, uint32_t color, int x0, int y0, unsigned int radius){ 
	int f = 1 - radius;
	int ddF_x = 0;
	int ddF_y = -2 * radius;
	int x = 0;
	int y = radius;

	ikigui_pixel_set(dest, color, x0 + radius, y0); // right  NE  
	ikigui_pixel_set(dest, color, x0, y0 - radius); // Top    NW
	ikigui_pixel_set(dest, color, x0 - radius, y0); // left   SW
	ikigui_pixel_set(dest, color, x0, y0 + radius); // bottom SE

	while(x < y){
		if(f >= 0){
			y--;
			ddF_y += 2;
			f += ddF_y;
		}
		x++;
		ddF_x += 2;
		f += ddF_x + 1;    

		ikigui_pixel_set(dest, color, x0 + y, y0 - x); // 1 , 1cuad //
		ikigui_pixel_set(dest, color, x0 + x, y0 - y); // 2 , 1cuad //
		ikigui_pixel_set(dest, color, x0 - x, y0 - y); // 3 , 2cuad //
		ikigui_pixel_set(dest, color, x0 - y, y0 - x); // 4 , 2cuad //
		ikigui_pixel_set(dest, color, x0 - y, y0 + x); // 5,  3cuad //
		ikigui_pixel_set(dest, color, x0 - x, y0 + y); // 6 , 3cuad //
		ikigui_pixel_set(dest, color, x0 + x, y0 + y); // 7 , 4cuad //
		ikigui_pixel_set(dest, color, x0 + y, y0 + x); // 8,  4cuad //		
	}
}


// ------------------------------------------------------------------------------
//   ikigui_image related operations - Drawing functions for image manipulation


/// Fill image or window with a ARGB value.
void ikigui_image_solid(ikigui_image *dest, unsigned int color){ 
        for(int i = 0 ; i < dest->w * dest->h ; i++){	dest->pixels[i] = color; }
}
/// Fill background in destination. Doesn't overwrite the image.
void ikigui_image_solid_bg(ikigui_image *dest,unsigned int color){ 
        for(int i = 0 ; i < dest->size ; i++){		dest->pixels[i] = alpha_channel(color,dest->pixels[i]);	}
}
/// Fill image with a color gradient.
void ikigui_image_gradient(ikigui_image *dest, uint32_t color_top, uint32_t color_bot){ 
	ikigui_rect all = {.x=0,.y=0,.w= dest->w,.h= dest->h};
	ikigui_draw_gradient(dest,color_top,color_bot,&all);
}
/// Allocate pixel memory for a ikigui_image
void ikigui_image_make(ikigui_image *frame, uint32_t w,uint32_t h){ 
        frame->w = w;
        frame->h = h;
        frame->pixels = (unsigned int*)malloc(frame->w*frame->h*4);
        frame->size = frame->w * frame->h ;
}
/// Convert RGBA pixels to ARGB pixels. Pixels is the same size, to you kan use a RGBA pixel buffer and convert it and display it with ikiGUI.
void ikigui_image_RGBA_to_ARGB(ikigui_image *frame){
	for(int i = 0 ; i < (frame->w*frame->h); i++){ // Convert RGBA to ARGB
		frame->pixels[i] =  ((frame->pixels[i] & 0x00FF0000)>>16) + ((frame->pixels[i] & 0x000000FF)<<16) + (frame->pixels[i] & 0xFF00FF00) ;
	}
}

// Structure for a ARGB32 pixel
typedef struct {
    uint8_t a; // alpha channel
    uint8_t r; // red channel
    uint8_t g; // green channel
    uint8_t b; // blue channel
} ikigui_argb_pixel;

/// Convert ARGB pixels with premultiplied alpha, to "normal" ARGB without premultiplied alpha that is used by ikiGUI.
void ikigui_image_ARGB_unmultiply(ikigui_image *frame){ // To convert data from e.g. cairo  
	for(int i = 0 ; i < (frame->w*frame->h); i++){ // Uses 8.8 fixed-point math
		ikigui_argb_pixel *pixel = (ikigui_argb_pixel *)&frame->pixels[i];
		if (pixel->a != 0) {
		    uint16_t alpha = pixel->a;
		    uint16_t invAlpha = (1 << 8) / alpha;
		    pixel->r = (pixel->r * invAlpha + 0x80) >> 8; // Add rounding
		    pixel->g = (pixel->g * invAlpha + 0x80) >> 8;
		    pixel->b = (pixel->b * invAlpha + 0x80) >> 8;
		}
	}
}

/// Read BMP image in header file (or RAM) to a ikigui_image and allocate memory for it.
void ikigui_include_bmp(ikigui_image *dest,const unsigned char* bmp_incl){ // Do not use ikigui_image_create() on a ikigui_image struct, before you use ikigui_include_bmp() as you will allocate memory 2 times.
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
