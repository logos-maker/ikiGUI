/// @file ikigui_win.h Things that are platform specific to Windows (using GDI from the WIN32 API)
// HINSTANCE hInstance; // https://stackoverflow.com/questions/15462064/hinstance-in-createwindow
// #define PRINT_ERROR(a, args...) printf("ERROR %s() %s Line %d: " a, __FUNCTION__, __FILE__, __LINE__, ##args);

// missing function: void ikigui_window_close(ikigui_window *mywin) // Not thought about it as it's done automatically by the DAW

#include <windows.h>
#include <stdbool.h>

bool quit = false;

/// Is referenced and is a member of the window struct, and don't need to be declared separatly.
struct mouse{
	int x, y;
	int x_intern, y_intern;
	unsigned char buttons;
	unsigned char buttons_intern;
	int pressed ;
	int old_x ;
	int old_y ;
	int old_button_press;
	char left_click;  	// mouse down events
	char middle_click;  	// mouse down events
	char right_click; 	// mouse down events
	char left_release;  	// mouse down events
	char middle_release;  	// mouse down events
	char right_release; 	// mouse down events
	POINT pos;
} ;

/// A window struct, that can be used to open a window with the ikigui_open_plugin_window function.
typedef struct ikigui_window{
        ikigui_image image;
        struct mouse mouse;
        HWND window_handle;
        BITMAPINFO bitmap_info;
        HBITMAP bitmap;
        HDC bitmap_device_context;
        bool keyboard[256];
        bool has_focus;
        MSG message;
	char name[32];     
} ikigui_window; 

int old_x;
int old_y;


enum { MOUSE_LEFT = 0b1, MOUSE_MIDDLE = 0b10, MOUSE_RIGHT = 0b100, MOUSE_X1 = 0b1000, MOUSE_X2 = 0b10000 };

/// For internal usage only
LRESULT CALLBACK WindowProcessMessage(HWND window_handle, UINT message, WPARAM wParam, LPARAM lParam) { 

	ikigui_window *mywin = (ikigui_window*)GetWindowLongPtr(window_handle, GWLP_USERDATA);

	switch(message) {
		case WM_CLOSE: DestroyWindow(window_handle); break;// Someone has pressed x on window
		case WM_QUIT:     break;// This function is usually called when the main window receives WM_DESTROY
		case WM_DESTROY: DestroyWindow(window_handle); break;// Window is about to be destroyed, and can not be hindered.
		case WM_PAINT: {
			PAINTSTRUCT paint;
			HDC device_context;
			device_context = BeginPaint(window_handle, &paint);

			// The functions BitBlt and StretchBlt have identical inputs, but has 2 more parameters in the end before the last parameter
			//BitBlt(device_context, paint.rcPaint.left, paint.rcPaint.top, paint.rcPaint.right - paint.rcPaint.left, paint.rcPaint.bottom - paint.rcPaint.top, mywin->bitmap_device_context, paint.rcPaint.left, paint.rcPaint.top, SRCCOPY);

			StretchBlt(
			device_context,
			0,//paint.rcPaint.left, 
			mywin->image.h-1,//0,//paint.rcPaint.bottom - paint.rcPaint.top -1, 
			mywin->image.w, //paint.rcPaint.right - paint.rcPaint.left, 
			-mywin->image.h, //- paint.rcPaint.bottom - paint.rcPaint.top,
			mywin->bitmap_device_context,
			0,//paint.rcPaint.left, 
			0,//paint.rcPaint.top,
			mywin->image.w, //paint.rcPaint.right - paint.rcPaint.left, 
			mywin->image.h, //paint.rcPaint.bottom - paint.rcPaint.top,
			SRCCOPY);

			/*
			BOOL StretchBlt(
			  [in] HDC   hdcDest,
			  [in] int   xDest,
			  [in] int   yDest,
			  [in] int   wDest,
			  [in] int   hDest,
			  [in] HDC   hdcSrc,
			  [in] int   xSrc,
			  [in] int   ySrc,
			  [in] int   wSrc,
			  [in] int   hSrc,
			  [in] DWORD rop
			);
			*/
			EndPaint(window_handle,&paint);
		} break;
		case WM_KILLFOCUS: mywin->has_focus = false; break;
		case WM_SETFOCUS: mywin->has_focus = true; break;

		case WM_KEYDOWN: break;
		case WM_KEYUP:   break;

		case WM_MOUSEMOVE: { POINT pos;
			if(mywin->mouse.buttons == MOUSE_LEFT){ // SetCursorPos();
				GetCursorPos(&pos);				// absolute coordinate on screen
				mywin->mouse.y_intern += pos.y - mywin->mouse.pos.y;
				mywin->mouse.pos.y = pos.y ;
				mywin->mouse.x_intern += pos.x - mywin->mouse.pos.x;	
				mywin->mouse.pos.x = pos.x;			
			
			}else{
				mywin->mouse.x_intern = LOWORD(lParam);		// coordniate relative to window
				mywin->mouse.y_intern = HIWORD(lParam);
			}
		} break;
		case WM_MOUSELEAVE: break;
		case WM_LBUTTONDOWN: mywin->mouse.buttons_intern |=  MOUSE_LEFT;   SetCapture(window_handle); GetCursorPos(&mywin->mouse.pos);break;
		case WM_LBUTTONUP:   mywin->mouse.buttons_intern &= ~MOUSE_LEFT;   ReleaseCapture();break;
		case WM_MBUTTONDOWN: mywin->mouse.buttons_intern |=  MOUSE_MIDDLE; break;
		case WM_MBUTTONUP:   mywin->mouse.buttons_intern &= ~MOUSE_MIDDLE; break;
		case WM_RBUTTONDOWN: mywin->mouse.buttons_intern |=  MOUSE_RIGHT;  break;
		case WM_RBUTTONUP:   mywin->mouse.buttons_intern &= ~MOUSE_RIGHT;  break;

		case WM_MOUSEWHEEL: {
			// printf("%s\n", wParam & 0b10000000000000000000000000000000 ? "Down" : "Up");
		} break;

		default: return DefWindowProc(window_handle, message, wParam, lParam);
	}
	return 0;
}

/// A helper function that return the pointer to the ikigui_image inside a ikigui_window
ikigui_image*	ikigui_image_of_window(ikigui_window* window){ return &window->image; }


/// Open a child window (for a audio plugin window)
void ikigui_window_open_editor(ikigui_window *mywin,void *ptr, int w, int h){

	const wchar_t window_class_name[] = L"Window Class";
	static WNDCLASS window_class = { 0 };

	window_class.lpfnWndProc = WindowProcessMessage;
        window_class.hInstance = GetModuleHandle(NULL);
        window_class.hCursor = LoadCursor(NULL, IDC_ARROW);
	window_class.lpszClassName = (PCSTR)window_class_name;
        window_class.hbrBackground = (HBRUSH) GetStockObject (WHITE_BRUSH);
        window_class.lpszMenuName = NULL;
	RegisterClass(&window_class);

        // To do before opening window!
	mywin->bitmap_info.bmiHeader.biSize = sizeof(mywin->bitmap_info.bmiHeader);
	mywin->bitmap_info.bmiHeader.biPlanes = 1;
	mywin->bitmap_info.bmiHeader.biBitCount = 32;
	mywin->bitmap_info.bmiHeader.biCompression = BI_RGB;
	mywin->bitmap_device_context = CreateCompatibleDC(0);

	if(ptr == NULL)mywin->window_handle = CreateWindow((PCSTR)window_class_name, mywin->name, WS_OVERLAPPEDWINDOW | WS_VISIBLE | WS_BORDER, CW_USEDEFAULT, CW_USEDEFAULT, w, h, NULL, NULL, GetModuleHandle(NULL), NULL);
        else mywin->window_handle = CreateWindow((PCSTR)window_class_name, mywin->name, WS_CHILD | WS_VISIBLE , CW_USEDEFAULT, CW_USEDEFAULT, w, h, (HWND)ptr, NULL, NULL, NULL);
	if(mywin->window_handle == NULL) {
	//	PRINT_ERROR("CreateWindow() failed. Returned NULL.\n");
	}

	mywin->image.w = mywin->bitmap_info.bmiHeader.biWidth = w;
	mywin->image.h = mywin->bitmap_info.bmiHeader.biHeight = h;
	if(mywin->bitmap) DeleteObject(mywin->bitmap);
	mywin->bitmap = CreateDIBSection(NULL, &mywin->bitmap_info, DIB_RGB_COLORS, (void**)&mywin->image.pixels, 0, 0);
	SelectObject(mywin->bitmap_device_context, mywin->bitmap);
	SetWindowLongPtr(mywin->window_handle, GWLP_USERDATA, (LONG_PTR)mywin);
	mywin->image.composit = 0 ;
}

/// Update the event data for the Window
void ikigui_window_get_events(struct ikigui_window *mywin){


        while(PeekMessage(&mywin->message, NULL, 0, 0, PM_REMOVE)) { DispatchMessage(&mywin->message); }

        // values for recognicing changes in mousemovements and mouse buttons.
        mywin->mouse.old_x = mywin->mouse.x ;     // old value for x coordinate.
        mywin->mouse.old_y = mywin->mouse.y ;     // old value for y coodrinate.
	mywin->mouse.x=mywin->mouse.x_intern;
	mywin->mouse.y=mywin->mouse.y_intern;
	mywin->mouse.old_button_press = mywin->mouse.buttons;  // old value for buttons. For finding changes later on.
	mywin->mouse.buttons = mywin->mouse.buttons_intern; 
	mywin->mouse.left_click   = (mywin->mouse.old_button_press == 0) && (mywin->mouse.buttons & MOUSE_LEFT);
	mywin->mouse.middle_click = (mywin->mouse.old_button_press == 0) && (mywin->mouse.buttons & MOUSE_MIDDLE);
	mywin->mouse.right_click  = (mywin->mouse.old_button_press == 0) && (mywin->mouse.buttons & MOUSE_RIGHT);
	mywin->mouse.left_release   = (mywin->mouse.old_button_press == 1) && (!(mywin->mouse.buttons & MOUSE_LEFT));
	mywin->mouse.middle_release = (mywin->mouse.old_button_press == 1) && (!(mywin->mouse.buttons & MOUSE_MIDDLE));
	mywin->mouse.right_release  = (mywin->mouse.old_button_press == 1) && (!(mywin->mouse.buttons & MOUSE_RIGHT));

}
/// Updates the Window graphics with new things drawn
void ikigui_window_update(struct ikigui_window *mywin){
        InvalidateRect(mywin->window_handle, NULL, FALSE);
        UpdateWindow(mywin->window_handle);
}

#ifdef IKIGUI_STANDALONE
	void ikigui_breathe(int milisec){ Sleep(milisec); } // Pause function, gives CPU over to the OS

	/// Open window (for a standalone application)
	void ikigui_window_open(ikigui_window *mywin, char *name, int w, int h){
		for(int i = 0 ; name[i] ; i++) mywin->name[i] = name[i] ;
		ikigui_window_open_editor(mywin,NULL, w, h);
		mywin->mouse.buttons_intern = 0 ;
		mywin->image.composit = 0 ;
	}
	void ikigui_window_till(ikigui_window* win, int delay){ /// Helper for simplicity
		ikigui_breathe(delay);
		ikigui_window_get_events(win);
		ikigui_window_update(win);
	}
#endif
