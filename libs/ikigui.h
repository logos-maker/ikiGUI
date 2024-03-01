/// @file ikigui.h Main file for ikiGUI. Has drawing functions, for making tilemaps that select image parts from a tile atlas, for oldschool monospace characters and graphic tiles.

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
	unsigned char composit; ///< flag for internal usage only (for compabillity reasons).
} ikigui_image;

#include <stdlib.h>
#include "ikigui_regular.h" // Include platform agnostic/independent drawing rutines

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

#ifndef IKIGUI_DRAW_ONLY // that declatation excludes all platform specific code, so it can be used for drawing into pixelbuffers only.
#ifdef __linux__ //linux specific code goes here...
	#include "ikigui_lin.h"	// For window and graphics handling in this case for Linux.
#elif _WIN32 // windows specific code goes here...
	#include "windows.h"
	#include "ikigui_win.h" // For window and graphics handling in this case for Windows.
#endif
#endif

typedef uint32_t ikigui_color; /// An less unambiguous way to rembemer how to make a color variable and not color constants.

/// to make a color by it's separate components
uint32_t ikigui_color_make(uint8_t a, uint8_t r, uint8_t g, uint8_t b){ return (a << 24) + (r << 16) + (g << 8) + (b << 0) ;}

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