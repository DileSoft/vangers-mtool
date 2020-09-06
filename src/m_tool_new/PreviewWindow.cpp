#include <windows.h>

#include "Globals.h"
#include "PreviewWindow.h"
#include "OpenGLWindow.h"

const char PreviewWnd_Caption[] = " Mechous preview";
const char PreviewWnd_ClassName[] = "TVangersMechousPreviewClassName";

const int PreviewWnd_Width = 335;
const int PreviewWnd_Height = 348;

// Fixes system menu bug which appears 
// with WS_EX_TOOLWINDOW style
static void CheckSystemMenu(HWND hWnd)
{
	HMENU hMenu = GetSystemMenu(hWnd, FALSE);
	
	// Comment it and uncomment below if WS_MINIMIZEBOX specified
	EnableMenuItem(hMenu, SC_MINIMIZE, MF_GRAYED);

	if (IsZoomed(hWnd))
	{
		EnableMenuItem(hMenu, SC_RESTORE, MF_ENABLED);
		EnableMenuItem(hMenu, SC_MAXIMIZE, MF_GRAYED);
		// EnableMenuItem(hMenu, SC_MINIMIZE, MF_ENABLED);
	}
	else
		if (IsIconic(hWnd))
		{
			EnableMenuItem(hMenu, SC_RESTORE, MF_ENABLED);
			EnableMenuItem(hMenu, SC_MAXIMIZE, MF_ENABLED);
			// EnableMenuItem(hMenu, SC_MINIMIZE, MF_GRAYED);
		}
		else
		{
			EnableMenuItem(hMenu, SC_RESTORE, MF_GRAYED);
			EnableMenuItem(hMenu, SC_MAXIMIZE, MF_ENABLED);
			// EnableMenuItem(hMenu, SC_MINIMIZE, MF_ENABLED);
		}
};

/*
// Makes normal iconic window for WS_EX_TOOLWINDOW style
void CheckExStyle(HWND hWnd, WPARAM wParam)
{
	LONG ExStyle = GetWindowLongA(hWnd, GWL_EXSTYLE);
	BOOL ToolWindow = ExStyle & WS_EX_TOOLWINDOW;

	if (wParam == SIZE_MINIMIZED)
	{
		if (ToolWindow)
			SetWindowLongA(hWnd, GWL_EXSTYLE, ExStyle & !WS_EX_TOOLWINDOW);
	}
	else
	{
		if (!ToolWindow)
			SetWindowLongA(hWnd, GWL_EXSTYLE, ExStyle | WS_EX_TOOLWINDOW);
	}
};
*/

// Used to resize or move some controls when 
// parent (PreviewWnd) size changes 
void CheckControlsPos(HWND hWnd)
{
	RECT r;
	GetClientRect(hWnd, &r);

	SetWindowPos(hOpenGLWnd, 0, 0, 0, 
		r.right - OpenGLWnd_Left - 10,
		r.bottom - 2 * OpenGLWnd_Top,
		SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);
};

void ClickClosePreview(HWND hWnd)
{
	SetForegroundWindow(hMainWnd);
	ShowWindow(hWnd, SW_HIDE);
};

LRESULT CALLBACK PreviewWndWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
		case WM_CLOSE : { ClickClosePreview(hWnd); break; }
		case WM_INITMENU : { CheckSystemMenu(hWnd); break; }
		case WM_SIZE : { CheckControlsPos(hWnd); break; }
		case WM_DROPFILES : { RedirectDropFiles(wParam, lParam); break; }
		case WM_MOUSEWHEEL : { SendMessageA(hOpenGLWnd, WM_MOUSEWHEEL, wParam, lParam); break; }
		case WM_KEYDOWN : { SendMessageA(hOpenGLWnd, WM_KEYDOWN, wParam, lParam); break; }
		case WM_SHOWWINDOW : { OpenGLWnd_EnableTimer((BOOL) wParam); break; }

		default : return DefWindowProcA(hWnd, uMsg, wParam, lParam);
	}
	
	return FALSE;
};

HWND PreviewWnd_CreateWnd(HINSTANCE hInstance, HWND Parent)
{
	WNDCLASSEXA ws;

	ZeroMemory(&ws, sizeof(WNDCLASSEXA));

	ws.cbSize = sizeof(WNDCLASSEXA);
	ws.hbrBackground = HBRUSH(COLOR_BTNFACE + 1);
	ws.hCursor = LoadCursor(0, IDC_ARROW);
	ws.hInstance = hInstance;
	ws.lpfnWndProc = &PreviewWndWndProc;
	ws.lpszClassName = PreviewWnd_ClassName;
	ws.style = CS_OWNDC;

	if (!RegisterClassExA(&ws)) return 0;

	int Left = (GetSystemMetrics(SM_CXSCREEN) - PreviewWnd_Width) / 2; 
    int Top = (GetSystemMetrics(SM_CYSCREEN) - PreviewWnd_Height) / 2; 

	HWND hWnd = CreateWindowExA(WS_EX_ACCEPTFILES | WS_EX_TOOLWINDOW, 
					PreviewWnd_ClassName, PreviewWnd_Caption,
					WS_SYSMENU | WS_CAPTION | WS_MAXIMIZEBOX | WS_SIZEBOX | WS_CLIPCHILDREN, 
					Left, Top, PreviewWnd_Width, PreviewWnd_Height,
					Parent, 0, hInstance, NULL);

	if (!hWnd) UnregisterClassA(PreviewWnd_ClassName, hInstance);
	
	return hWnd;
};

BOOL PreviewWnd_CreateCaptions(HINSTANCE hInstance, HWND Parent)
{
	ControlParams cp;
	cp.hInstance = hInstance;
	cp.Parent = hPreviewWnd;
	cp.ControlID = 0;
	cp.Style = SS_CENTER;

	cp.r.left = 1;
	cp.r.right = 14;
	cp.r.bottom = 14;
	cp.Caption = "\226";

	cp.r.top = 20;
	hOpenGLWnd_Lattice = StaticWnd_CreateWnd(&cp);

	cp.r.top += 20;
	hOpenGLWnd_FacesNormals = StaticWnd_CreateWnd(&cp);

	cp.r.top += 20;
	hOpenGLWnd_VerticesNormals = StaticWnd_CreateWnd(&cp);

	cp.r.top += 20;
	hOpenGLWnd_FacesColors = StaticWnd_CreateWnd(&cp);

	return (hOpenGLWnd_Lattice && hOpenGLWnd_FacesNormals &&
		hOpenGLWnd_FacesColors && hOpenGLWnd_VerticesNormals);
};

// Creates PreviewWindow and all of it's childrens
BOOL CreatePreviewWindow(HINSTANCE hInstance, HWND Parent)
{
	hPreviewWnd = PreviewWnd_CreateWnd(hInstance, Parent);
	
	BOOL CaptionsOk = FALSE;

	if (hPreviewWnd) 
	{
		HMENU hMenu = GetSystemMenu(hPreviewWnd, FALSE);
		EnableMenuItem(hMenu, SC_MINIMIZE, MF_GRAYED);	
		EnableMenuItem(hMenu, SC_RESTORE, MF_GRAYED);
		EnableMenuItem(hMenu, SC_MAXIMIZE, MF_ENABLED);

		hOpenGLWnd = OpenGLWnd_CreateWnd(hInstance, hPreviewWnd);
		
		CaptionsOk = PreviewWnd_CreateCaptions(hInstance, hPreviewWnd);
	}

	return (hPreviewWnd && hOpenGLWnd && CaptionsOk);
};