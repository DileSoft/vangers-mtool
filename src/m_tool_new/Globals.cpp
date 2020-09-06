#include <windows.h>

#include "Globals.h"

// Main definition
ModelsStore Models;

// Common font
HFONT hFont = 0;

// Handles...
// HWND hApplicationWnd = 0;
HWND hExportWnd = 0;
HWND hMainWnd = 0;
	HWND hListBoxWnd = 0;
	HWND hLoadModelsBtn = 0;
	HWND hCenterModelBtn = 0;
	HWND hShowPreviewBtn = 0;
	HWND hClearListBtn = 0;
	HWND hShowModelParamsBtn = 0;
	HWND hShowMechousParamsBtn = 0;
	HWND hSaveMechousBtn = 0;

HWND hPreviewWnd = 0;
	HWND hOpenGLWnd = 0;
	HWND hOpenGLWnd_Lattice = 0;
	HWND hOpenGLWnd_VerticesNormals = 0;
	HWND hOpenGLWnd_FacesNormals = 0;
	HWND hOpenGLWnd_FacesColors = 0;

HWND hModelParamsWnd = 0;
	HWND hColorIDCbx = 0;
	HWND hReflectionCbx = 0;
	HWND hWheelSteerCbx = 0;
	HWND hDebrisPowerCbx = 0;
	HWND hSlotIDCbx = 0;
	HWND hSlotRotationCbx = 0;
	HWND hModelParamsCloseBtn = 0;
	HWND hModelParamsApplyBtn = 0;
	HWND hModelTypeCbx = 0;

HWND hMechousParamsWnd = 0;
	HWND hMechousParamsOkBtn = 0;
	HWND hMechousParamsCancelBtn = 0;



// Button window
HWND ButtonWnd_CreateWnd(ControlParams *cp)
{
	HWND hWnd = CreateWindowExA(0, "BUTTON", cp -> Caption, 
					WS_VISIBLE | WS_CHILD | cp -> Style,
					cp -> r.left, cp -> r.top, cp -> r.right, cp -> r.bottom, 
					cp -> Parent, cp -> ControlID, cp -> hInstance, NULL);

	if (hWnd) SendMessage(hWnd, WM_SETFONT, (WPARAM) hFont, 0);
	return hWnd;
};

// Edit window
HWND EditWnd_CreateWnd(ControlParams *cp)
{
	HWND hWnd = CreateWindowExA(WS_EX_CLIENTEDGE, "EDIT", cp -> Caption, 
					WS_VISIBLE | WS_CHILD | cp -> Style,
					cp -> r.left, cp -> r.top, cp -> r.right, cp -> r.bottom, 
					cp -> Parent, cp -> ControlID, cp -> hInstance, NULL);

	if (hWnd) SendMessage(hWnd, WM_SETFONT, (WPARAM) hFont, 0);
	return hWnd;
};

// Combobox window
HWND ComboBoxWnd_CreateWnd(ControlParams *cp)
{
	HWND hWnd = CreateWindowExA(WS_EX_CLIENTEDGE, "COMBOBOX", cp -> Caption, 
					WS_VISIBLE | WS_CHILD | CBS_DISABLENOSCROLL | CBS_HASSTRINGS | CBS_DROPDOWNLIST | WS_VSCROLL | cp -> Style,
					cp -> r.left, cp -> r.top, cp -> r.right, cp -> r.bottom, 
					cp -> Parent, cp -> ControlID, cp -> hInstance, NULL);

	if (hWnd) SendMessage(hWnd, WM_SETFONT, (WPARAM) hFont, 0);
	return hWnd;
};

// Static window
HWND StaticWnd_CreateWnd(ControlParams *cp)
{
	HWND hWnd = CreateWindowExA(0, "STATIC", cp -> Caption, 
					WS_VISIBLE | WS_CHILD | cp -> Style,
					cp -> r.left, cp -> r.top, cp -> r.right, cp -> r.bottom, 
					cp -> Parent, cp -> ControlID, cp -> hInstance, NULL);

	if (hWnd) SendMessage(hWnd, WM_SETFONT, (WPARAM) hFont, 0);
	return hWnd;
};

// Redirect all files to main window
void RedirectDropFiles(WPARAM wParam, LPARAM lParam)
{
	SendMessageA(hMainWnd, WM_DROPFILES, wParam, lParam);
};

// Do not enable window twise
void EnableWindowCheck(HWND hWnd, BOOL Enable)
{
	if (IsWindowEnabled(hWnd) != Enable)
		EnableWindow(hWnd, Enable);
};