![](./thelay.png)

# ikiGUI
The smallest cross platform audio plugin GUI library availible. Is about 500 lines of code in total. With a minimum of dependencies, and uses only old standard system libraries for maximum compability.
- Cross platform compatible to Linux and Windows
- Cross compiles 100% identical code to differnt platforms.
- Only uses the most compatible OS functionalliy availible.
- Uses no third party libraries.
- Super small code base
- Easy to animate graphics
- Easy to make columns or rows of knobs, buttons or even letters.
- Convenient for debuging, printing text and numbers directly on the plugin GUI.
- Uses 32bit pixels (24bit color and 8bit alpha channel).
- Optimized to be easy to learn and use.
- To be able to write incredibly fast plugins where it actually counts.
- Faster learing curve as C is many times easier language to learn than C++
- Faster deep knowledge to improve code base and do innovations by your ovn.
Example code for using it can be found here... https://github.com/logos-maker/VST2

## GUI lib in C for iki graphics
It's a new cross platform audio plugin GUI library for tiled graphics with alpha channel and animations, that uses almost no code at all. 

It's currently availible to Windows and Linux, and selects the platform specific code automatically.

For displaying and animate graphics it uses simple placement, and character maps (an old concept widely used in the 80's and in 2D computer games that also gives the animation capabilities). 

And it's well suited for displaying prerendered skeuomorphic or flat vector graphics or photos as plugin graphics, and gives 100% identical graphis for all platforms.

The core concept here is simplicity, and less is more. There is not much to learn, and it givs fewer compatibillity issues, and to get you more productive where it counts.

The name is a wordplay with the Japanese words 'iki' and 'ikigai', combined with GUI (Graphical User Interface).
## Reasons to use GUI lib for graphics
- Plugs that load and opens the editor incedibly fast.
- The small code base gives less bugs
- It's easy to understand
- It's easy to modify
- It's compatible to the highest degree
- It cross compiles with 100% identical code
- Easy to animate graphics
- Gives 24bit layered graphics with alpha channel
- Easy to implement  it in other languages 

## Quick overview of all functions
```
ikigui				// creates a ikigui object.
ikigui_init()			// allocates memory and configures your tilemap.
ikigui_free()			// release memory allocated with ikigui_init()
ikigui_draw()			// draws the graphics tile map. According to ikigui_init() 

ikigui_blit_part()		// Blits a part of an image, with alpha channel over the destination graphic
ikigui_blit_part_filled()	// Blits a part of an image, with a background color so the destination graphic gets overwritten.
ikigui_blit_part_fast()		// Blits a part of an image, without alpha channel.
ikigui_blit()			// Blits the whole image, to the destination, without alpha channel.
ikigui_fill_bg()		// Initializes and overwites an image alpha channel, using a color for the background.
ikigui_bmp_include()		// Imports BMP graphics that are included, to be used as graphics.
ikigui_open_plugin_window()	// open a plugin window (child window) in the used DAW.
ikigui_get_events()		// gets the window events like mouseclicks and so on.
ikigui_update_window()		// updates the graphics in the plugin window.
```

## Roadmap
- Support for more platforms.
- Support usage in more languages.
- Tutorials.
- More examples.
- Readymade graphics for easy usage.
- Tools for faster development and rapid GUI design.
- Better documentation.

## Tutorial on how to prepare  graphics for a plug with ikiGUI
For this you need som background graphics for your front panel.
And some assets like knobs and other graphics that can change it's appearance during usage of your audio plug. This changing graphics will be on top of the background graphics. 

Your assets needs to be saved in a image format that supports alpha channel.
Alpha channel gives the possibility for transparent graphics for shadows and edges.
So your assets on top of the background will look right.

One good place to find free suitable knobs for your plug is here...
https://www.g200kg.com/en/webknobman/gallery.php
Click on one knob you like and then press 'Easy Rendering'.
On the next screen press 'Export Exec'.
Right click on the .png image in your web-browser and save it.

Next you need to convert the image to a BMP file.
Open the image in a program like gimp or what you like, and export the image to a BMP file using ARGB formatting. After saving the image, you can convert it to a C-array that you can include in your project C code.
For conversion to a C-array, there is a program called asset_conv.c included in this repository if you don't have a program to do the job.

Then you also need a background image for your knobs to be displayed upon.
Use som type of graphics program like Gimp or what you like to use.
And place all your text, background color and everything you want visible on your front panel.
it's good to make usage of layers when designing, so you can move and change separate things without problems. If you use layers, you can also try out your assets in the top layers.

Then when you are ready, you can then export it all except your assets as a BMP image.
And then convert it to a c-array to include to your project.

## Licences
The code will soon be dual-licensed for a small fee for closed source projects. More info later.

