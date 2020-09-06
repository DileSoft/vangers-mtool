#include <windows.h>
#include <stdio.h>
#include <CommCtrl.h>

#include "Globals.h"
#include "ModelSettings.h"
#include "ListBoxWindow.h"

const char ModelParamsWnd_Caption[] = " Model params";
const char ModelParamsWnd_Caption_Empty[] = " Model params \226 Nothing selected";

const char ModelParamsWnd_ClassName[] = "TVangersModelParamsClassName";

const int ModelParamsWnd_Width = 255;
const int ModelParamsWnd_Height = 300;

const int ModelParamsWnd_TopDelta = 35;
const int ModelParamsWnd_Right = 110;

// Hhhhmmm... o_O :)
const RECT ModelTypeCbx_Rect	 = {ModelParamsWnd_Right, ModelParamsWnd_TopDelta * 0 + 15, 120, 28};
const RECT ColorIDCbx_Rect		 = {ModelParamsWnd_Right, ModelParamsWnd_TopDelta * 1 + 15, 120, 28};
const RECT ReflectionCbx_Rect	 = {ModelParamsWnd_Right, ModelParamsWnd_TopDelta * 2 + 15, 120, 28};
const RECT WheelSteerCbx_Rect	 = {ModelParamsWnd_Right, ModelParamsWnd_TopDelta * 3 + 15, 120, 28};
const RECT SlotIDCbx_Rect		 = {ModelParamsWnd_Right, ModelParamsWnd_TopDelta * 4 + 15, 120, 28};
const RECT SlotRotationCbx_Rect	 = {ModelParamsWnd_Right, ModelParamsWnd_TopDelta * 5 + 15, 120, 28};

const char ModelParamsApplyBtn_Caption[] = "Apply";
const RECT ModelParamsApplyBtn_Rect = {27, 231, 85, 28};

const char ModelParamsCloseBtn_Caption[] = "Close";
const RECT ModelParamsCloseBtn_Rect = {137, 231, 85, 28};

const int ModelParamsWnd_ParamsCount = 6;
const char *ModelParamsWnd_ParamsNames[] = 
{
	" Model type:", " Color ID:", " Reflection:",
	" Wheel steer:", " Slot ID:",
	" Slot angle:"
};

// Combobox combinations
#define NEED_COLORID		0x0001
#define NEED_REFLECTION		0x0002
#define NEED_WHEELSTEER		0x0004
#define NEED_SLOTID			0x0008
#define NEED_SLOTANGLE		0x0010

#define NEED_ALL			0x001F
#define NEED_BODY			0x0003
#define NEED_WHEEL			0x0007
#define NEED_DEBRIS			NEED_BODY
#define NEED_SLOT			0x0018

// Common rect for all statics
RECT ModelParamsWnd_Static_Rect = {15, 18, ModelParamsWnd_Right - 20, 16};

// Enable comboboxes marked as needed
void ModelParamsWnd_EnableNeeded(int Need)
{
	EnableWindowCheck(hColorIDCbx, Need & NEED_COLORID);
	EnableWindowCheck(hReflectionCbx, Need & NEED_REFLECTION);
	EnableWindowCheck(hWheelSteerCbx, Need & NEED_WHEELSTEER);
	EnableWindowCheck(hSlotIDCbx, Need & NEED_SLOTID);
	EnableWindowCheck(hSlotRotationCbx, Need & NEED_SLOTANGLE);
};

// Get combination of needed comboboxes and return new ColorID
int MadelParamsWnd_ModelTypeChangedIndex(int Index)
{
	int NeedBoxes = 0;
	int NewColorID = 0;

	switch ((ModelMaterials) Index)
	{
		case MODEL_MATERIAL_BODY :		{ NeedBoxes = NEED_BODY; NewColorID = COLOR_BODY; break; }
		case MODEL_MATERIAL_GLASS :		{ NeedBoxes = NEED_BODY; NewColorID = COLOR_GLASS; break; }
		case MODEL_MATERIAL_METAL :		{ NeedBoxes = NEED_BODY; NewColorID = COLOR_METAL; break; }
		case MODEL_MATERIAL_TUBE :		{ NeedBoxes = NEED_BODY; NewColorID = COLOR_TUBE; break; }
		
		case MODEL_MATERIAL_WHEEL :		{ NeedBoxes = NEED_WHEEL;  NewColorID = COLOR_WHEEL; break; }		
		case MODEL_MATERIAL_DEBRIS :	{ NeedBoxes = NEED_DEBRIS; NewColorID = COLOR_BODY; break; }
		case MODEL_MATERIAL_SLOT :		{ NeedBoxes = NEED_SLOT;   NewColorID = COLOR_SLOT; break; }
		case MODEL_MATERIAL_UNKNOWN :	{ NeedBoxes = NEED_ALL;    NewColorID = COLOR_RESERVED; break; }
	}

	ModelParamsWnd_EnableNeeded(NeedBoxes);
	return NewColorID;
};

// Call to enable particular comboboxes based on type
void ModelParamsWnd_ModelTypeChanged(HWND hWnd)
{
	int Index = (int) SendMessageA(hModelTypeCbx, CB_GETCURSEL, 0, 0);
	MadelParamsWnd_ModelTypeChangedIndex(Index);
	// SendMessageA(hColorIDCbx, CB_SETCURSEL, (WPARAM) NewColorID, 0);
};

// Set model's params TO controls
void ModelParamsWnd_SetParams(Model *m)
{
	if (!m) 
	{
		SetWindowTextA(hModelParamsWnd, ModelParamsWnd_Caption_Empty);
		EnableWindowCheck(hModelParamsApplyBtn, FALSE);
		return;
	}
	else
		EnableWindowCheck(hModelParamsApplyBtn, TRUE);

	// Go!
	int Index = 0;

	// PARAMETER (0) - MODEL TYPE
	Index = m -> ModelMaterial;
	SendMessageA(hModelTypeCbx, CB_SETCURSEL, (WPARAM) Index, 0);
	MadelParamsWnd_ModelTypeChangedIndex(Index);

	// PARAMETER (1) - COLOR ID
	Index = m -> ModelParams.ColorID; 
	SendMessageA(hColorIDCbx, CB_SETCURSEL, (WPARAM) Index, 0);

	// PARAMETER (2) - REFLECTION
	// 0, 2, 3, ... -> 0, 1, 2 ... 
	Index = m -> ModelParams.Reflection;
	if (Index > 0) Index--; // Because "1" is skipped
	
	SendMessageA(hReflectionCbx, CB_SETCURSEL, (WPARAM) Index, 0);

	// PARAMETER (3) - WHEEL STEER
	Index = (int) (m -> ModelParams.WheelSteer);
	SendMessageA(hWheelSteerCbx, CB_SETCURSEL, (WPARAM) Index, 0);

	// PARAMETER (4) - SLOT ID
	Index = m -> ModelParams.SlotID;
	SendMessageA(hSlotIDCbx, CB_SETCURSEL, (WPARAM) Index, 0);

	// PARAMETER (5) - SLOT ROTATION
	Index = m -> ModelParams.SlotAngle;
	SendMessageA(hSlotRotationCbx, CB_SETCURSEL, (WPARAM) Index, 0);

	SIZE_T CaptionLen = strlen(ModelParamsWnd_Caption) + strlen(m -> ModelName) + 4;
	char *Caption = new char [CaptionLen];

	if (Caption)
	{
		ZeroMemory(Caption, CaptionLen);
		lstrcat(Caption, ModelParamsWnd_Caption);
		lstrcat(Caption, " \226 ");
		lstrcat(Caption, m -> ModelName);

		SetWindowTextA(hModelParamsWnd, Caption);	
		delete [] Caption;
	}
};

// Set model's params FROM controls
void ModelParamsWnd_GetParams(ModelParamsWnd_Params &mpwp)
{
	int Index = 0;

	// PARAMETER (0) - MODEL TYPE
	Index = (int) SendMessageA(hModelTypeCbx, CB_GETCURSEL, 0, 0);
	mpwp.mm = (ModelMaterials) Index;
	
	switch (mpwp.mm)
	{
		case MODEL_MATERIAL_BODY : 
		case MODEL_MATERIAL_GLASS : 
		case MODEL_MATERIAL_METAL : 
		case MODEL_MATERIAL_TUBE :		{ mpwp.mt = MODEL_BODY; break; }
		
		case MODEL_MATERIAL_WHEEL :		{ mpwp.mt = MODEL_WHEEL; break; }		
		case MODEL_MATERIAL_DEBRIS :	{ mpwp.mt = MODEL_DEBRIS; break; }
		case MODEL_MATERIAL_SLOT :		{ mpwp.mt = MODEL_SLOT; break; }
		case MODEL_MATERIAL_UNKNOWN :	{ mpwp.mt = MODEL_UNKNOWN; break; }
	}

	// PARAMETER (1) - COLOR ID
	Index = (int) SendMessageA(hColorIDCbx, CB_GETCURSEL, 0, 0);
	mpwp.mp.ColorID = (ColorsIDs) Index; 	

	// PARAMETER (2) - REFLECTION
	Index = (int) SendMessageA(hReflectionCbx, CB_GETCURSEL, 0, 0);

	if (Index > 0) Index++;
	mpwp.mp.Reflection = Index;

	// PARAMETER (3) - WHEEL STEER
	Index = (int) SendMessageA(hWheelSteerCbx, CB_GETCURSEL, 0, 0);
	mpwp.mp.WheelSteer = (BOOL) Index;
	
	// PARAMETER (4) - SLOT ID
	Index = (int) SendMessageA(hSlotIDCbx, CB_GETCURSEL, 0, 0);
	mpwp.mp.SlotID = Index;

	// PARAMETER (5) - SLOT ROTATION
	Index = (int) SendMessageA(hSlotRotationCbx, CB_GETCURSEL, 0, 0);
	mpwp.mp.SlotAngle = Index;
};

static void CheckSystemMenu(HWND hWnd)
{
	HMENU hMenu = GetSystemMenu(hWnd, FALSE);
	
	EnableMenuItem(hMenu, SC_MINIMIZE, MF_GRAYED);
	EnableMenuItem(hMenu, SC_MAXIMIZE, MF_GRAYED);
	EnableMenuItem(hMenu, SC_RESTORE, MF_GRAYED);
};

// Apply params to all selected items
void ModelParamsWnd_ApplyParams(HWND hWnd)
{
	ModelParamsWnd_Params mpwp;
	ModelParamsWnd_GetParams(mpwp);

	// Set params to list box...
	ListBoxWnd_ApplyParams(hListBoxWnd, mpwp, FALSE);
};

void ModelParamsWnd_HideWindow(HWND hWnd)
{
	SetForegroundWindow(hMainWnd); 
	ShowWindow(hWnd, SW_HIDE);
};

LRESULT CALLBACK ModelParamsWndWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
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
						case BTN_MODELPARAMSCLOSE : { ModelParamsWnd_HideWindow(hWnd); break; }
						case BTN_MODELPARAMSAPPLY : { ModelParamsWnd_ApplyParams(hListBoxWnd); break; }
					}
				
					break;
				}

				case CBN_SELENDOK :
				{
					switch LOWORD(wParam)
					{
						case CBX_MODELTYPE : { ModelParamsWnd_ModelTypeChanged(hWnd); break; }
					}
				
					break;
				}
			}
		
		break;
		}
	
		case WM_CLOSE : { ModelParamsWnd_HideWindow(hWnd); break; }
		case WM_INITMENU : { CheckSystemMenu(hWnd); break; }
		case WM_DROPFILES : { RedirectDropFiles(wParam, lParam); break; }

		// case WM_MOUSEWHEEL : { SendMessageA(hOpenGLWnd, WM_MOUSEWHEEL, wParam, lParam); break; }
		// case WM_SHOWWINDOW : { OpenGLWnd_EnableTimer((BOOL) wParam); break; }

		default : return DefWindowProcA(hWnd, uMsg, wParam, lParam);
	}
	
	return FALSE;
};

HWND ModelParamsWnd_CreateWnd(HINSTANCE hInstance, HWND Parent)
{
	WNDCLASSEXA ws;

	ZeroMemory(&ws, sizeof(WNDCLASSEXA));

	ws.cbSize = sizeof(WNDCLASSEXA);
	ws.hbrBackground = HBRUSH(COLOR_BTNFACE + 1); 
	ws.hCursor = LoadCursor(0, IDC_ARROW);
	ws.hInstance = hInstance;
	ws.lpfnWndProc = &ModelParamsWndWndProc;
	ws.lpszClassName = ModelParamsWnd_ClassName;
	ws.style = CS_OWNDC;

	if (!RegisterClassExA(&ws)) return 0;

	int Left = (GetSystemMetrics(SM_CXSCREEN) - ModelParamsWnd_Width) / 2; 
    int Top = (GetSystemMetrics(SM_CYSCREEN) - ModelParamsWnd_Height) / 2; 

	HWND hWnd = CreateWindowExA(WS_EX_TOOLWINDOW | WS_EX_ACCEPTFILES, 
					ModelParamsWnd_ClassName, ModelParamsWnd_Caption_Empty,
					WS_SYSMENU | WS_CAPTION,
					Left, Top, ModelParamsWnd_Width, ModelParamsWnd_Height,
					Parent, 0, hInstance, NULL);

	if (!hWnd) UnregisterClassA(ModelParamsWnd_ClassName, hInstance);
	
	return hWnd;
};

// ColorID combobox
HWND ColorIDCbx_CreateWnd(HINSTANCE hInstance, HWND Parent)
{
	ControlParams cp;

	cp.Caption = "";
	cp.hInstance = hInstance;
	cp.Parent = Parent;
	cp.r = ColorIDCbx_Rect;
	cp.Style = WS_TABSTOP;
	cp.ControlID = (HMENU) CBX_COLORID;

	HWND hWnd = ComboBoxWnd_CreateWnd(&cp);

	if (hWnd)
	{
		SendMessageA(hWnd, CB_ADDSTRING, 0, (LPARAM) " (0)  Zero");
		SendMessageA(hWnd, CB_ADDSTRING, 0, (LPARAM) " (1)  Body");
		SendMessageA(hWnd, CB_ADDSTRING, 0, (LPARAM) " (2)  Glass");
		SendMessageA(hWnd, CB_ADDSTRING, 0, (LPARAM) " (3)  Wheel");
		SendMessageA(hWnd, CB_ADDSTRING, 0, (LPARAM) " (4)  Metal");
		SendMessageA(hWnd, CB_ADDSTRING, 0, (LPARAM) " (5)  Slot");
		SendMessageA(hWnd, CB_ADDSTRING, 0, (LPARAM) " (6)  Tube");
		
		/*
		// Shall we use this?
		SendMessageA(hWnd, CB_ADDSTRING, 0, (LPARAM) " Body RED");
		SendMessageA(hWnd, CB_ADDSTRING, 0, (LPARAM) " Body BLUE");
		SendMessageA(hWnd, CB_ADDSTRING, 0, (LPARAM) " Body YELLOW");
		SendMessageA(hWnd, CB_ADDSTRING, 0, (LPARAM) " Body GRAY");
		SendMessageA(hWnd, CB_ADDSTRING, 0, (LPARAM) " Yellow charged);
		SendMessageA(hWnd, CB_ADDSTRING, 0, (LPARAM) " Material 0");
		SendMessageA(hWnd, CB_ADDSTRING, 0, (LPARAM) " Material 1");
		SendMessageA(hWnd, CB_ADDSTRING, 0, (LPARAM) " Material 2");
		SendMessageA(hWnd, CB_ADDSTRING, 0, (LPARAM) " Material 3");
		SendMessageA(hWnd, CB_ADDSTRING, 0, (LPARAM) " Material 4");
		SendMessageA(hWnd, CB_ADDSTRING, 0, (LPARAM) " Material 5");
		SendMessageA(hWnd, CB_ADDSTRING, 0, (LPARAM) " Material 6");
		SendMessageA(hWnd, CB_ADDSTRING, 0, (LPARAM) " Material 7");
		*/
		
		SendMessageA(hWnd, CB_SETCURSEL, 0, 0);
	}

	return hWnd;
};

// Reflection combobox
HWND ReflectionCbx_CreateWnd(HINSTANCE hInstance, HWND Parent)
{
	ControlParams cp;

	cp.Caption = "";
	cp.hInstance = hInstance;
	cp.Parent = Parent;
	cp.r = ReflectionCbx_Rect;
	cp.Style = WS_TABSTOP;
	cp.ControlID = (HMENU) CBX_REFLECTION;

	HWND hWnd = ComboBoxWnd_CreateWnd(&cp);

	if (hWnd)
	{	
		SendMessageA(hWnd, CB_SETMINVISIBLE, 8, 0);

		char *MetalLevel = ")  Strength ";
		const SIZE_T MetalLevelLen = strlen(MetalLevel);

		char *Level = new char [3];

		char *Line = "";
		SIZE_T LineLen = 0;

		SendMessageA(hWnd, CB_ADDSTRING, 0, (LPARAM) " (0)  Diffuse");

		for (int i = 2; i < 17; i++)
		{
			_itoa_s(i, Level, 3, 10);
			LineLen = MetalLevelLen + 2 * strlen(Level) + 3;

			Line = new char [LineLen];
			ZeroMemory(Line, LineLen);

			lstrcat(Line, " (");
			lstrcat(Line, Level);
			lstrcat(Line, MetalLevel);
			lstrcat(Line, Level);

			SendMessageA(hWnd, CB_ADDSTRING, 0, (LPARAM) Line);
			
			delete [] Line;
		}

		delete [] Level;
		SendMessageA(hWnd, CB_SETCURSEL, 0, 0);
	}

	return hWnd;
};

// Wheel steer combobox
HWND WheelSteerCbx_CreateWnd(HINSTANCE hInstance, HWND Parent)
{
	ControlParams cp;

	cp.Caption = "";
	cp.hInstance = hInstance;
	cp.Parent = Parent;
	cp.r = WheelSteerCbx_Rect;
	cp.Style = WS_TABSTOP;
	cp.ControlID = (HMENU) CBX_WHEELSTEER;

	HWND hWnd = ComboBoxWnd_CreateWnd(&cp);

	if (hWnd)
	{
		SendMessageA(hWnd, CB_ADDSTRING, 0, (LPARAM) " (0)  No");
		SendMessageA(hWnd, CB_ADDSTRING, 0, (LPARAM) " (1)  Yes");
		SendMessageA(hWnd, CB_SETCURSEL, 0, 0);
	}

	return hWnd;
};

// Slot ID combobox
HWND SlotIDCbx_CreateWnd(HINSTANCE hInstance, HWND Parent)
{
	ControlParams cp;

	cp.Caption = "";
	cp.hInstance = hInstance;
	cp.Parent = Parent;
	cp.r = SlotIDCbx_Rect;
	cp.Style = WS_TABSTOP;
	cp.ControlID = (HMENU) CBX_SLOTID;

	HWND hWnd = ComboBoxWnd_CreateWnd(&cp);

	if (hWnd)
	{
		SendMessageA(hWnd, CB_ADDSTRING, 0, (LPARAM) " (0)  Left");
		SendMessageA(hWnd, CB_ADDSTRING, 0, (LPARAM) " (1)  Right");
		SendMessageA(hWnd, CB_ADDSTRING, 0, (LPARAM) " (2)  Middle");
		SendMessageA(hWnd, CB_SETCURSEL, 2, 0);
	}
	
	return hWnd;
};

// Slot rotation combobox
HWND SlotRotationCbx_CreateWnd(HINSTANCE hInstance, HWND Parent)
{
	ControlParams cp;

	cp.Caption = "";
	cp.hInstance = hInstance;
	cp.Parent = Parent;
	cp.r = SlotRotationCbx_Rect;
	cp.Style = WS_TABSTOP;
	cp.ControlID = (HMENU) CBX_SLOTROTATION;

	HWND hWnd = ComboBoxWnd_CreateWnd(&cp);

	if (hWnd)
	{
		SendMessageA(hWnd, CB_ADDSTRING, 0, (LPARAM) " (0)  Lefthand 90");
		SendMessageA(hWnd, CB_ADDSTRING, 0, (LPARAM) " (1)  Righthand 90");
		SendMessageA(hWnd, CB_ADDSTRING, 0, (LPARAM) " (2)  None");
		SendMessageA(hWnd, CB_SETCURSEL, 2, 0);
	}

	return hWnd;
};

// Button closes model params window
HWND ModelParamsCloseBtn_CreateWnd(HINSTANCE hInstance, HWND Parent)
{
	ControlParams cp;

	cp.Caption = ModelParamsCloseBtn_Caption;
	cp.hInstance = hInstance;
	cp.Parent = Parent;
	cp.r = ModelParamsCloseBtn_Rect;
	cp.Style = BS_PUSHBUTTON | WS_TABSTOP;
	cp.ControlID = (HMENU) BTN_MODELPARAMSCLOSE;

	HWND hWnd = ButtonWnd_CreateWnd(&cp);
	return hWnd;
};

// Button closes model params window
HWND ModelParamsApplyBtn_CreateWnd(HINSTANCE hInstance, HWND Parent)
{
	ControlParams cp;

	cp.Caption = ModelParamsApplyBtn_Caption;
	cp.hInstance = hInstance;
	cp.Parent = Parent;
	cp.r = ModelParamsApplyBtn_Rect;
	cp.Style = BS_PUSHBUTTON | WS_DISABLED | WS_TABSTOP;
	cp.ControlID = (HMENU) BTN_MODELPARAMSAPPLY;

	HWND hWnd = ButtonWnd_CreateWnd(&cp);
	return hWnd;
};

// Model type combobox
HWND ModelTypeCbx_CreateWnd(HINSTANCE hInstance, HWND Parent)
{
	ControlParams cp;

	cp.Caption = "";
	cp.hInstance = hInstance;
	cp.Parent = Parent;
	cp.r = ModelTypeCbx_Rect;
	cp.Style = WS_TABSTOP;
	cp.ControlID = (HMENU) CBX_MODELTYPE;

	HWND hWnd = ComboBoxWnd_CreateWnd(&cp);

	if (hWnd)
	{
		SendMessageA(hWnd, CB_ADDSTRING, 0, (LPARAM) " (0)  Unknown");
		SendMessageA(hWnd, CB_ADDSTRING, 0, (LPARAM) " (1)  Body (body)");
		SendMessageA(hWnd, CB_ADDSTRING, 0, (LPARAM) " (2)  Body (glass)");
		SendMessageA(hWnd, CB_ADDSTRING, 0, (LPARAM) " (3)  Body (metal)");
		SendMessageA(hWnd, CB_ADDSTRING, 0, (LPARAM) " (4)  Body (tube)");
		SendMessageA(hWnd, CB_ADDSTRING, 0, (LPARAM) " (5)  Wheel");
		SendMessageA(hWnd, CB_ADDSTRING, 0, (LPARAM) " (6)  Debris");
		SendMessageA(hWnd, CB_ADDSTRING, 0, (LPARAM) " (7)  Slot");
		SendMessageA(hWnd, CB_SETCURSEL, 0, 0);
	}

	return hWnd;
};

// Creates names of params and it's owners
void ModelParamsWnd_CreateStatics(HINSTANCE hInstance, HWND Parent)
{
	ControlParams cpNames;

	cpNames.hInstance = hInstance;
	cpNames.Parent = Parent;
	cpNames.ControlID = 0;
	cpNames.Style = SS_NOPREFIX;
	cpNames.r = ModelParamsWnd_Static_Rect;

	for (int i = 0; i < ModelParamsWnd_ParamsCount; i++)
	{
		cpNames.Caption = ModelParamsWnd_ParamsNames[i];
		StaticWnd_CreateWnd(&cpNames);
		cpNames.r.top += ModelParamsWnd_TopDelta;
	}
};

BOOL CreateModelParamsWindow(HINSTANCE hInstance, HWND Parent)
{
	hModelParamsWnd = ModelParamsWnd_CreateWnd(hInstance, Parent);

	if (hModelParamsWnd)
	{
		CheckSystemMenu(hModelParamsWnd);

		// Input controls
		hModelTypeCbx = ModelTypeCbx_CreateWnd(hInstance, hModelParamsWnd);
		hColorIDCbx = ColorIDCbx_CreateWnd(hInstance, hModelParamsWnd);
		hReflectionCbx = ReflectionCbx_CreateWnd(hInstance, hModelParamsWnd);
		hWheelSteerCbx = WheelSteerCbx_CreateWnd(hInstance, hModelParamsWnd);
		hSlotIDCbx = SlotIDCbx_CreateWnd(hInstance, hModelParamsWnd);
		hSlotRotationCbx = SlotRotationCbx_CreateWnd(hInstance, hModelParamsWnd);
		
		// Statics
		ModelParamsWnd_CreateStatics(hInstance, hModelParamsWnd);

		// Buttons
		hModelParamsApplyBtn = ModelParamsApplyBtn_CreateWnd(hInstance, hModelParamsWnd);
		hModelParamsCloseBtn = ModelParamsCloseBtn_CreateWnd(hInstance, hModelParamsWnd);
	};

	return (hModelParamsWnd != 0);
};	