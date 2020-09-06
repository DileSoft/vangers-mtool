
#include <windows.h>
#include <float.h>
#include <stdio.h>

#include "Globals.h"
#include "MechousSettings.h"

const char MechousParamsWnd_Caption[] = " Mechous parameters";
const char MechousParamsWnd_ClassName[] = "TVangersMechousParamsClassName";

// Mechous params window sizes
const int MechousParamsWnd_Width = 638;
const int MechousParamsWnd_Height = 402;

const char MechousParamsWnd_OkBtn_Caption[] = "Apply";
const char MechousParamsWnd_CancelBtn_Caption[] = "Close";

const RECT MechousParamsWnd_OkBtn_Rect = {435, 335, 80, 28};
const RECT MechousParamsWnd_CancelBtn_Rect = {535, 335, 80, 28};

// Statics texts... *CRAZY* 
const char *MechousParamsStatics[MECHOUS_PARAMS_COUNT] =
{
	" Scale size:",		" Scale bound:",		" Scale box:",
	" Mass Z offset:",	" Speed on ground:",	" Speed in water:",
	" Speed in air:",	" Speed as mole:",		" Mobility factor:",
	" Archimedian:",	" Water traction:",		" Water rudder:",
	" Terra mover X:",	" Terra mover Y:",		" Terra mover Z:",
	" Defence front:",	" Defence back:",		" Defence side:",
	" Defence top:",	" Defence bottom:",		" Ram front:",
	" Ram back:",		" Ram side:",			" Ram top:", 
	" Ram bottom:",		" Color offset:",		" Color shift:",
	" Smooth angle:",	" Weld limit:"
};

// Edits defaults...
const char *MechousParamsDefaults[MECHOUS_PARAMS_COUNT] =
{
	"1",	"1",	"1",
	"0",	"1",	"0.5",
	"0.5",	"0.5",	"1",
	"0.5",	"1",	"1.5",
	"1",	"1",	"1",
	"200",	"100",	"50",
	"25",	"25",	"100",
	"50",	"50",	"25",
	"25",	"144",	"3",
	"60",	"2.5"
};

// Whether set value to edit as float...
const BOOL MechousParamFloat[MECHOUS_PARAMS_COUNT] =
{
	TRUE, TRUE, TRUE,
	TRUE, TRUE, TRUE,
	TRUE, TRUE, TRUE,
	TRUE, TRUE, TRUE,
	TRUE, TRUE, TRUE,
	FALSE, FALSE, FALSE,
	FALSE, FALSE, FALSE,
	FALSE, FALSE, FALSE,
	FALSE, FALSE, FALSE,
	FALSE, TRUE
};

// Mechous params store
float *MechousParams = NULL;

// Edits creation params
const int Edits_Left = 120;
const int Edits_Top = 18;
const int Edits_Width = 75;
const int Edits_Height = 20;
const int Edits_Top_Delta = 35;
const int Edits_TextLen = 10;

// Statics creation params
const int Statics_Left = 15;
const int Statics_Top = 21;
const int Statics_Width = 95;
const int Statics_Height = 16;

// Array of edits handles
HWND *EditsStore = NULL;

// Actually set params to MechousParams from edits
void MechousParamsWnd_GetEditParams(float *mp)
{
	char *EditText = new char [Edits_TextLen + 1];

	for (int i = 0; i < MECHOUS_PARAMS_COUNT; i++)
	{
		HWND hEdit = EditsStore[i];
		
		ZeroMemory(EditText, Edits_TextLen + 1);
		SendMessageA(hEdit, WM_GETTEXT, (WPARAM) Edits_TextLen + 1, (LPARAM) EditText);

		mp[i] = (float) atof(EditText);

		if (!MechousParamFloat[i])
		{
			if (mp[i] > INT_MAX) mp[i] = (float) INT_MAX;
			else 
				if (mp[i] < FLT_EPSILON) mp[i] = 0.0;
				else mp[i] = (float) Round(mp[i]);
		}
	}

	delete [] EditText;
};

// Send MechousParams to edits...
void MechousParamsWnd_SetEditParams(float *mp)
{
	char *EditText = new char [Edits_TextLen + 1];

	for (int i = 0; i < MECHOUS_PARAMS_COUNT; i++)
	{
		ZeroMemory(EditText, Edits_TextLen + 1);
		
		if (MechousParamFloat[i])
			_snprintf_s(EditText, Edits_TextLen + 1, _TRUNCATE, "%.5g", mp[i]);
		else
			_snprintf_s(EditText, Edits_TextLen + 1, _TRUNCATE, "%.0f", mp[i]); 
		
		SendMessageA(EditsStore[i], WM_SETTEXT, 0, (LPARAM) EditText);
	}
	
	delete [] EditText;
};

BOOL MechousParamsWnd_CreateEdits(HINSTANCE hInstance, HWND Parent)
{
	_set_output_format(_TWO_DIGIT_EXPONENT);

	EditsStore = new HWND [MECHOUS_PARAMS_COUNT];
	MechousParams = new float [MECHOUS_PARAMS_COUNT];

	if (!EditsStore || !MechousParams) return FALSE;

	ZeroMemory(EditsStore, sizeof(HWND) * MECHOUS_PARAMS_COUNT);
	ZeroMemory(MechousParams, sizeof(float) * MECHOUS_PARAMS_COUNT);
	
	ControlParams cpe, cps;
	cpe.Caption = "";
	cpe.ControlID = 0;
	cpe.hInstance = hInstance;
	cpe.Parent = Parent;
	cpe.Style = WS_TABSTOP;

	cpe.r.left = Edits_Left;
	cpe.r.right = Edits_Width;
	cpe.r.top = Edits_Top;
	cpe.r.bottom = Edits_Height;

	cps = cpe;
	cps.Style = 0;
	cps.r.left = Statics_Left;
	cps.r.right = Statics_Width;
	cps.r.top = Statics_Top;
	cps.r.bottom = Statics_Height;

	BOOL EditsCreated = TRUE;

	for (int i = 0; i < MECHOUS_PARAMS_COUNT; i++)
	{
		cpe.Caption = MechousParamsDefaults[i];
		HWND hWndEdit = EditWnd_CreateWnd(&cpe);
		
		cps.Caption = MechousParamsStatics[i];
		HWND hWndStatic = StaticWnd_CreateWnd(&cps);
		
		EditsCreated = (hWndEdit && hWndStatic);
		if (!EditsCreated) break;

		// Store edit's handle
		EditsStore[i] = hWndEdit;

		// Limit text in the edit
		SendMessageA(hWndEdit, EM_LIMITTEXT, (WPARAM) Edits_TextLen, 0);

		cpe.r.top += Edits_Top_Delta;
		cps.r.top += Edits_Top_Delta;
		
		BOOL NewColumn = (i != 0) && ((i + 1) % 10 == 0);

		if (NewColumn)
		{
			cpe.r.top = Edits_Top;
			cpe.r.left += Edits_Width + 135;

			cps.r.top = Statics_Top;
			cps.r.left += Edits_Width + 135;
		}
	}

	return EditsCreated;
};

// Free resources
void MechousParamsWnd_FreeEdits()
{
	if (EditsStore) delete [] EditsStore;
	if (MechousParams) delete [] MechousParams;
};

// Closes mechous params window
void MechousParamsWnd_HideWindow(HWND hWnd)
{
	SetForegroundWindow(hMainWnd);
	ShowWindow(hWnd, SW_HIDE);
};

// Restore saved params in the edits & hides window
void MechousParamsWnd_CancelHide(HWND hWnd)
{
	MechousParamsWnd_HideWindow(hWnd);
	MechousParamsWnd_SetEditParams(MechousParams);
};

// Ok acts like Apply button
void MechousParamsWnd_OkClick(HWND hWnd)
{
	// Apply entered values
	MechousParamsWnd_GetEditParams(MechousParams);
	
	// Restore processed values to avoid errors
	MechousParamsWnd_SetEditParams(MechousParams);
};

static void CheckSystemMenu(HWND hWnd)
{
	HMENU hMenu = GetSystemMenu(hWnd, FALSE);
	
	EnableMenuItem(hMenu, SC_MINIMIZE, MF_GRAYED);
	EnableMenuItem(hMenu, SC_MAXIMIZE, MF_GRAYED);
	EnableMenuItem(hMenu, SC_RESTORE, MF_GRAYED);
};

// No comments...
LRESULT CALLBACK MechousParamsWndWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
		case WM_COMMAND :
		{
			switch HIWORD(wParam)
			{
				case BN_CLICKED :
				{
					switch LOWORD(wParam)
					{
						case BTN_MECHPARAMSOK : { MechousParamsWnd_OkClick(hWnd); break; }	
						case BTN_MECHPARAMSCANCEL : { MechousParamsWnd_CancelHide(hWnd); break; }
					}
				
					break;
				}
			}
		
		break;
		}

		case WM_CLOSE : { MechousParamsWnd_CancelHide(hWnd); break; }
		case WM_DROPFILES : { RedirectDropFiles(wParam, lParam); break; }
		case WM_INITMENU : { CheckSystemMenu(hWnd); break; }
		case WM_DESTROY : { MechousParamsWnd_FreeEdits(); break; }

		default : return DefWindowProcA(hWnd, uMsg, wParam, lParam);
	}
	
	return FALSE;
};

HWND MechousParamsWnd_CreateWnd(HINSTANCE hInstance, HWND Parent)
{
	WNDCLASSEXA ws;

	ZeroMemory(&ws, sizeof(WNDCLASSEXA));

	ws.cbSize = sizeof(WNDCLASSEXA);
	ws.hbrBackground = HBRUSH(COLOR_BTNFACE + 1);
	ws.hCursor = LoadCursor(0, IDC_ARROW);
	ws.hInstance = hInstance;
	ws.lpfnWndProc = &MechousParamsWndWndProc;
	ws.lpszClassName = MechousParamsWnd_ClassName;
	ws.style = CS_OWNDC;

	if (!RegisterClassExA(&ws)) return 0;

	int Left = (GetSystemMetrics(SM_CXSCREEN) - MechousParamsWnd_Width) / 2; 
    int Top = (GetSystemMetrics(SM_CYSCREEN) - MechousParamsWnd_Height) / 2; 

	HWND hWnd = CreateWindowExA(WS_EX_ACCEPTFILES | WS_EX_TOOLWINDOW, 
					MechousParamsWnd_ClassName, MechousParamsWnd_Caption,
					WS_SYSMENU | WS_CAPTION, 
					Left, Top, MechousParamsWnd_Width, MechousParamsWnd_Height,
					Parent, 0, hInstance, NULL);

	if (!hWnd) UnregisterClassA(MechousParamsWnd_ClassName, hInstance);
	
	return hWnd;
};

// Creates close and apply buttons of mechous params window
BOOL MechousParamsWnd_CreateButtons(HINSTANCE hInstance, HWND Parent)
{
	ControlParams cp;

	cp.hInstance = hInstance;
	cp.Parent = Parent;
	cp.Style = BS_PUSHBUTTON | WS_TABSTOP;
	
	cp.r = MechousParamsWnd_OkBtn_Rect;
	cp.ControlID = (HMENU) BTN_MECHPARAMSOK;
	cp.Caption = MechousParamsWnd_OkBtn_Caption;
	hMechousParamsOkBtn = ButtonWnd_CreateWnd(&cp);

	cp.r = MechousParamsWnd_CancelBtn_Rect;
	cp.ControlID = (HMENU) BTN_MECHPARAMSCANCEL;
	cp.Caption = MechousParamsWnd_CancelBtn_Caption;
	hMechousParamsCancelBtn = ButtonWnd_CreateWnd(&cp);
	
	return (hMechousParamsCancelBtn && hMechousParamsOkBtn);
};

// Creates mechous params window and all of it's childrens
BOOL CreateMechousParamsWindow(HINSTANCE hInstance, HWND Parent)
{
	hMechousParamsWnd = MechousParamsWnd_CreateWnd(hInstance, Parent);
	
	BOOL EditsCreated = FALSE;
	BOOL ButtonsCreated = FALSE;

	if (hMechousParamsWnd) 
	{
		CheckSystemMenu(hMechousParamsWnd);

		EditsCreated = MechousParamsWnd_CreateEdits(hInstance, hMechousParamsWnd);
		MechousParamsWnd_GetEditParams(MechousParams);

		ButtonsCreated = MechousParamsWnd_CreateButtons(hInstance, hMechousParamsWnd);
	}

	return (hMechousParamsWnd && EditsCreated && ButtonsCreated);
};
