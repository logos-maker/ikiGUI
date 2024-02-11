//HINSTANCE hInstance; // https://stackoverflow.com/questions/15462064/hinstance-in-createwindow
// #define PRINT_ERROR(a, args...) printf("ERROR %s() %s Line %d: " a, __FUNCTION__, __FILE__, __LINE__, ##args);
#include <windows.h>
#include <stdbool.h>
#include "ikigui_goofy.h"

bool quit = false;

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

typedef struct ikigui_window{
        ikigui_image frame;
        struct mouse mouse;
        HWND window_handle;
        BITMAPINFO bitmap_info;
        HBITMAP bitmap;
        HDC bitmap_device_context;
        bool keyboard[256];
        bool has_focus;
        MSG message;     
} ikigui_window; 

int old_x;
int old_y;


enum { MOUSE_LEFT = 0b1, MOUSE_MIDDLE = 0b10, MOUSE_RIGHT = 0b100, MOUSE_X1 = 0b1000, MOUSE_X2 = 0b10000 };

LRESULT CALLBACK WindowProcessMessage(HWND window_handle, UINT message, WPARAM wParam, LPARAM lParam) {

	ikigui_window *mywin = (ikigui_window*)GetWindowLongPtr(window_handle, GWLP_USERDATA);

	switch(message) {
		case WM_CLOSE: break;// Someone has pressed x on window
		case WM_QUIT:  break;// This function is usually called when the main window receives WM_DESTROY
		case WM_DESTROY: DestroyWindow(window_handle); break;// Window is about to be destroyed, and can not be hindered.
		case WM_PAINT: {
			PAINTSTRUCT paint;
			HDC device_context;
			device_context = BeginPaint(window_handle, &paint);
			BitBlt(device_context, paint.rcPaint.left, paint.rcPaint.top, paint.rcPaint.right - paint.rcPaint.left, paint.rcPaint.bottom - paint.rcPaint.top, mywin->bitmap_device_context, paint.rcPaint.left, paint.rcPaint.top, SRCCOPY);
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
				mywin->mouse.pos.y = pos.y;
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
/*

*/
void ikigui_open_plugin_window(ikigui_window *mywin,void *ptr,int w, int h){

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

        mywin->window_handle = CreateWindow((PCSTR)window_class_name, "BitBlt Test Program ", WS_CHILD | WS_VISIBLE , CW_USEDEFAULT, CW_USEDEFAULT, w, h, (HWND)ptr, NULL, NULL, NULL);
	if(mywin->window_handle == NULL) {
	//	PRINT_ERROR("CreateWindow() failed. Returned NULL.\n");
	}

	mywin->frame.w = mywin->bitmap_info.bmiHeader.biWidth = w;
	mywin->frame.h = mywin->bitmap_info.bmiHeader.biHeight = h;
	if(mywin->bitmap) DeleteObject(mywin->bitmap);
	mywin->bitmap = CreateDIBSection(NULL, &mywin->bitmap_info, DIB_RGB_COLORS, (void**)&mywin->frame.pixels, 0, 0);
	SelectObject(mywin->bitmap_device_context, mywin->bitmap);
	SetWindowLongPtr(mywin->window_handle, GWLP_USERDATA, (LONG_PTR)mywin);
}

void ikigui_get_events(struct ikigui_window *mywin){


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

void ikigui_update_window(struct ikigui_window *mywin){
        InvalidateRect(mywin->window_handle, NULL, FALSE);
        UpdateWindow(mywin->window_handle);
}
