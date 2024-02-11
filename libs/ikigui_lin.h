// include the X library headers
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>
#include "ikigui_regular.h"

enum { MOUSE_LEFT = 0b1, MOUSE_MIDDLE = 0b10, MOUSE_RIGHT = 0b100, MOUSE_X1 = 0b1000, MOUSE_X2 = 0b10000 };

struct mouse{
	int x, y, x_rel, y_rel;
	unsigned char buttons;
    // new
	int pressed ;
	int old_x ;
	int old_y ;
	int old_button_press;

    // mouse down events
	char left_click; 
	char middle_click; 
	char right_click;
	char left_release;  	// mouse down events
	char middle_release;  	// mouse down events
	char right_release; 	// mouse down events 
} ;

typedef struct {
        // here are our X variables
        Display *dis;           // X Server connection
        int screen;             // X11 Screen
        Window win;             // the X11 window       
        GC gc;                  // X11 Graphic Context
	XEvent event;		// the XEvent declaration !!! */
        Atom wm_delete_window;  // the window close event

        // to handle keypress events
	KeySym key;		/* a dealie-bob to handle KeyPress Events */	
	char text[255];		/* a char buffer for KeyPress Events */

        // to handle mouse events
        struct mouse mouse;

        // To have pixels
	XImage* image;
	Pixmap bitmap;
        ikigui_image frame;
} ikigui_window;                     

int old_x;
int old_y;

void ikigui_open_plugin_window(ikigui_window *mywin,void *ptr,int w, int h){
        mywin->frame.w = w;
        mywin->frame.h = h;
	mywin->dis=XOpenDisplay((char *)0);     // Get the display
   	mywin->screen=DefaultScreen(mywin->dis); // Get the screen
        int x = 0;
        int y = 0;
	unsigned long black = BlackPixel(mywin->dis,mywin->screen);// get the color black

        mywin->win=XCreateSimpleWindow(mywin->dis,DefaultRootWindow(mywin->dis),x,y,w, h, 5,black, black); // Create window

        // To get event from closing window
        mywin->wm_delete_window = XInternAtom(mywin->dis, "WM_DELETE_WINDOW", False);
        XSetWMProtocols(mywin->dis, mywin->win, &mywin->wm_delete_window, 1);

        // Event types to recive
	XSelectInput(mywin->dis, mywin->win, ExposureMask|PointerMotionMask|ButtonPressMask|ButtonReleaseMask|KeyPressMask|KeyReleaseMask|FocusChangeMask|EnterWindowMask|LeaveWindowMask); // Select event types to get

        mywin->gc=DefaultGC(mywin->dis,mywin->screen);

	XMapRaised(mywin->dis, mywin->win);

        XWindowAttributes wa = {0};
        XGetWindowAttributes(mywin->dis, mywin->win, &wa);

	mywin->frame.pixels = (unsigned int *)malloc(w * h * 4);
	mywin->bitmap = XCreatePixmap(mywin->dis, mywin->win, w, h, 1);
	mywin->image = XCreateImage(mywin->dis, wa.visual, wa.depth, ZPixmap, 0, (char*)mywin->frame.pixels, w, h, 32, w * 4);

        XReparentWindow(mywin->dis, mywin->win,(Window)ptr, 0, 0);
        XFlush(mywin->dis);
}

void ikigui_close_window(ikigui_window *mywin){
	XDestroyWindow(mywin->dis,mywin->win);
	XCloseDisplay(mywin->dis);				
};

void ikigui_update_window(ikigui_window *mywin){
        XPutImage(mywin->dis, mywin->win, mywin->gc, mywin->image, 0, 0, 0, 0, mywin->frame.w, mywin->frame.h);
};

void ikigui_get_events(ikigui_window *mywin){
	// values for recognicing changes in mousemovements and mouse buttons.
	
        mywin->mouse.old_button_press =  mywin->mouse.buttons;  // old value for buttons. For finding changes later on.
        mywin->mouse.old_x =  mywin->mouse.x ;     // old value for x coordinate.
        mywin->mouse.old_y =  mywin->mouse.y ;     // old value for y coodrinate.
	
        while( XPending(mywin->dis) > 0 ){ // no of events in que
                XNextEvent(mywin->dis, &mywin->event); // Get next event

                if (mywin->event.type== ClientMessage){ // User Closes window
                        if ((Atom) mywin->event.xclient.data.l[0] == mywin->wm_delete_window) {
                            		ikigui_close_window(mywin);
	                                exit(1);
                        }
                }
	        if (mywin->event.type==Expose && mywin->event.xexpose.count==0) ikigui_update_window(mywin); // Window graphics needs a redraw
                if (mywin->event.type==KeyPress&& XLookupString(&mywin->event.xkey,mywin->text,255,&mywin->key,0)==1) { // the XLookupString routine to converts the KeyPress event data into regular text.
		        //printf("You pressed the %c key!\n",mywin.text[0]);
	        }

                switch(mywin->event.type){
                        case MotionNotify:{// Mouse movement
                                mywin->mouse.x_rel =  mywin->event.xmotion.x -old_x ;
                                mywin->mouse.y_rel =  mywin->event.xmotion.y -old_y ;
                            
                                old_x = mywin->mouse.x = mywin->event.xmotion.x ;
                                old_y = mywin->mouse.y = mywin->event.xmotion.y ;
                        }
                        break;
                        case ButtonPress:   //printf("Mouse botton down x %d y %d ",mywin.event.xbutton.x,mywin.event.xbutton.y);
                                switch(mywin->event.xbutton.button){
                                        case 1: mywin->mouse.buttons |=  MOUSE_LEFT;   break; // printf("Left\n"); break;
                                        case 2: mywin->mouse.buttons |=  MOUSE_MIDDLE; break; // printf("Middle\n");break;
                                        case 3: mywin->mouse.buttons |=  MOUSE_RIGHT;  break; // printf("Right\n");break;
                                        //default: printf("???%d",mywin.event.xbutton.button);break;
                                }
                        break;
                        case ButtonRelease: //printf("Mouse botton up x %d y %d \n",mywin.event.xbutton.x,mywin.event.xbutton.y); break;
                                switch(mywin->event.xbutton.button){
                                        case 1: mywin->mouse.buttons &= ~MOUSE_LEFT;   break; // printf("Left\n"); break;
                                        case 2: mywin->mouse.buttons &= ~MOUSE_MIDDLE; break; // printf("Middle\n");break;
                                        case 3: mywin->mouse.buttons &= ~MOUSE_RIGHT;  break; // printf("Right\n");break;
                                        //default: printf("???%d",mywin.event.xbutton.button);break;
                                }
                        break;
                }
        } // while loop end

	mywin->mouse.left_click   = (mywin->mouse.old_button_press == 0) && (mywin->mouse.buttons & MOUSE_LEFT);
	mywin->mouse.middle_click = (mywin->mouse.old_button_press == 0) && (mywin->mouse.buttons & MOUSE_MIDDLE);
	mywin->mouse.right_click  = (mywin->mouse.old_button_press == 0) && (mywin->mouse.buttons & MOUSE_RIGHT);
	mywin->mouse.left_release   = (mywin->mouse.old_button_press == 1) && (!(mywin->mouse.buttons & MOUSE_LEFT));
	mywin->mouse.middle_release = (mywin->mouse.old_button_press == 1) && (!(mywin->mouse.buttons & MOUSE_MIDDLE));
	mywin->mouse.right_release  = (mywin->mouse.old_button_press == 1) && (!(mywin->mouse.buttons & MOUSE_RIGHT));
};
