#ifndef WINDOW
#define WINDOW

#include <windows.h>

typedef struct Window{
	int x, y;
	int w, h;
	float r;
}Window;

typedef struct WindowHandle{
	
	// window
	HWND hwnd;
	HDC hdc;
	Window window;
	int move;
	int resize;
	
	// input
	int lclick;
	int rclick;
	int esc;
}WindowHandle;

// window
void setupPixelFormat(HDC hdc);
void windowHandleFill(WindowHandle *handle);
int windowSetup(WindowHandle *handle, HINSTANCE hInstance, int iCmdShow);
int windowUpdate(int *outputMessage, WindowHandle *handle, int *screenUpdated, int *escKey, int *leftMouseClick, int *rightMouseClick, float *windowSizeX, float *windowSizeY, float *cursorPosX, float *cursorPosY);
void windowDisplay(WindowHandle *handle);
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

// window operations
void windowResize(Window* window, float *sizeX, float *sizeY);
void windowPos(HWND hwnd, Window *window);
void windowCursorPos(WindowHandle *handle, float *cursorX, float *cursorY);

// window functions

void setupPixelFormat(HDC hdc){
	static PIXELFORMATDESCRIPTOR pfd={
		sizeof(PIXELFORMATDESCRIPTOR),1,
		PFD_DRAW_TO_WINDOW|PFD_SUPPORT_OPENGL|PFD_DOUBLEBUFFER,
		PFD_TYPE_RGBA,32,0,0,0,0,0,0,0,0,0,0,0,0,0,
		16,0,0,PFD_MAIN_PLANE,0,0,0,0
	};
	SetPixelFormat(hdc,ChoosePixelFormat(hdc,&pfd),&pfd);
}

void windowHandleFill(WindowHandle *handle){
	handle->hdc = NULL;
	handle->window.w = 600;
	handle->window.h = 480;
	handle->window.x = 660;
	handle->window.y = 300;
	handle->window.r = 0;
	handle->move = 0;
	handle->resize = 0;
	handle->lclick = 0;
	handle->rclick = 0;
	handle->esc = 0;
}

int windowSetup(WindowHandle *handle, HINSTANCE hInstance, int iCmdShow){
	windowHandleFill(handle);
	WNDCLASSEX wc;
	wc.cbSize = sizeof(WNDCLASSEX);
	wc.style = 0;
	wc.lpfnWndProc = WndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = hInstance;
	wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	wc.lpszMenuName = NULL;
	wc.lpszClassName = "CLASS";
	wc.hIconSm = LoadIcon(NULL,IDI_APPLICATION);
	if(!RegisterClassEx(&wc)){
		MessageBox(NULL, "Window Reg Fail", "Error", MB_ICONEXCLAMATION | MB_OK);
		return 0;
	}
	if((handle->hwnd = CreateWindowEx(WS_EX_CLIENTEDGE, "CLASS", "Minesweeper", WS_OVERLAPPEDWINDOW, 
		handle->window.x, handle->window.y, handle->window.w, handle->window.h, 
		NULL, NULL, hInstance, handle)) == NULL){
		MessageBox(NULL, "Window Create Fail", "Error", MB_ICONEXCLAMATION | MB_OK);
		return 0;
	}
	ShowWindow(handle->hwnd, iCmdShow);
	return 1;
}

int windowUpdate(int *outputMessage, WindowHandle *handle, int *screenUpdated, int *escKey, int *leftMouseClick, int *rightMouseClick, float *windowSizeX, float *windowSizeY, float *cursorPosX, float *cursorPosY){
	MSG msg;
	PeekMessage(&msg, NULL, 0, 0, PM_REMOVE | PM_NOYIELD);
	*outputMessage = msg.wParam;
	if(msg.message == WM_QUIT)
		return 0;
	if(handle->move){
		windowPos(handle->hwnd, &handle->window);
		handle->move = 0;
	}
	if(handle->resize){
		windowPos(handle->hwnd, &handle->window);
		windowResize(&handle->window, windowSizeX, windowSizeY);
		*screenUpdated = 1;
		handle->resize = 0;
	}
	
	// input update
	if(handle->esc == 1){
		*escKey = 1;
		handle->esc = -1;
	}
	if(handle->lclick == 1){
		*leftMouseClick = 1;
		windowCursorPos(handle, cursorPosX, cursorPosY);
		handle->lclick = -1;
	}
	if(handle->rclick == 1){
		*rightMouseClick = 1;
		windowCursorPos(handle, cursorPosX, cursorPosY);
		handle->rclick = -1;
	}
	TranslateMessage(&msg);
	DispatchMessage(&msg);
	return 1;
}

void windowDisplay(WindowHandle *handle){
	SwapBuffers(handle->hdc);
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam){
	WindowHandle *handle;
	HDC hdc = NULL;
	HGLRC hrc = NULL;
	PAINTSTRUCT ps;
	
	if(msg == WM_CREATE){
		CREATESTRUCT *create = (CREATESTRUCT*)lParam;
		handle = (WindowHandle*)create->lpCreateParams;
		SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)handle);
	}
	else{
		LONG_PTR p = GetWindowLongPtr(hwnd, GWLP_USERDATA);
		handle = (WindowHandle*)p;
	}
	
	switch(msg){
		case WM_CREATE:
			hdc = BeginPaint(hwnd, &ps);
			handle->hdc = hdc;
			setupPixelFormat(hdc);
			hrc = wglCreateContext(hdc);
			wglMakeCurrent(hdc, hrc);
			break;
		case WM_CLOSE:
			hrc = wglGetCurrentContext();
			wglMakeCurrent(hdc, NULL);
			wglDeleteContext(hrc);
			EndPaint(hwnd, &ps);
			DestroyWindow(hwnd);
		case WM_DESTROY:
			PostQuitMessage(0);
			break;
		case WM_SIZE:
			handle->window.w = LOWORD(lParam);
			handle->window.h = HIWORD(lParam);
			handle->resize = 1;
			break;
		case WM_MOVE:
			handle->move = 1;
			break;
		case WM_LBUTTONDOWN:
			handle->lclick = handle->lclick == 0 ? 1 : -1;
			break;
		case WM_RBUTTONDOWN:
			handle->rclick = handle->rclick == 0 ? 1 : -1;
			break;
		case WM_LBUTTONUP:
			handle->lclick = 0;
			break;
		case WM_RBUTTONUP:
			handle->rclick = 0;
			break;
		case WM_KEYDOWN:
			if(wParam == VK_ESCAPE){
				if(handle->esc == 0)
					handle->esc = 1;
				else
					handle->esc = -1;
			}
			break;
		case WM_KEYUP:
			if(wParam == VK_ESCAPE)
				handle->esc = 0;
			break;
		default:
			return DefWindowProc(hwnd, msg, wParam, lParam);
	}
	return 0;
}

// window operation functions

void windowResize(Window* window, float *sizeX, float *sizeY){
	
	// window size
	window->r = (float)window->w / window->h;
	if(window->r >= 1.f){
		*sizeX = window->r;
		*sizeY = 1.f;
	}
	else{
		*sizeX = 1.f;
		*sizeY = 1.f / window->r;
	}
}

void windowPos(HWND hwnd, Window *window){
	RECT rect;
	GetWindowRect(hwnd, &rect);
	int borderWidth = (rect.right - rect.left - window->w) / 2.f;
	int titleHeight = rect.bottom - rect.top - borderWidth - window->h;
	window->x = (int)rect.left + borderWidth;
	window->y = (int)rect.top + titleHeight;
}

void windowCursorPos(WindowHandle *handle, float *cursorX, float *cursorY){
	
	// screen position
	POINT p;
	GetCursorPos(&p);
	float x = p.x;
	float y = p.y;
	
	// window-relative position
	x -= handle->window.x;
	y -= handle->window.y;
	
	// projection-relative position
	x = ((x * 2.f / handle->window.w) - 1.f);
	y = (1.f - (y * 2.f / handle->window.h));
	
	*cursorX = x;
	*cursorY = y;
}

#endif