
#include <windows.h>

#include "RoundTemplate.h"
#include "ModelsList.h"

extern ModelsStore Models;

extern HFONT hFont;

// Pi :)
#define Pi_10		3.1415926536

#define LBX_MODELPARTS			 15
#define BTN_LOADMODELS			 16
#define BTN_SHOWPREVIEW			 17
#define BTN_CENTERMODEL			 18
#define BTN_CLEARLIST			 19
#define CBX_COLORID				 20
#define CBX_REFLECTION			 21
#define CBX_WHEELSTEER			 22
#define CBX_SLOTID				 23
#define CBX_SLOTROTATION		 24
#define BTN_MODELPARAMSCLOSE	 25
#define BTN_MODELPARAMSAPPLY	 26
#define BTN_MODELPARAMSSHOW		 27
#define CBX_MODELTYPE			 28
#define BTN_MECHOUSPARAMSSHOW	 29
#define BTN_MECHPARAMSOK		 30
#define BTN_MECHPARAMSCANCEL	 31
#define BTN_SAVEMECHOUS			 32

// extern HWND hApplicationWnd;
extern HWND hExportWnd;
extern HWND hMainWnd;
	extern HWND hListBoxWnd;

	extern HWND hLoadModelsBtn;
	extern HWND hShowPreviewBtn;
	extern HWND hCenterModelBtn;
	extern HWND hClearListBtn;
	extern HWND hShowModelParamsBtn;
	extern HWND hShowMechousParamsBtn;
	extern HWND hSaveMechousBtn;

extern HWND hPreviewWnd;
	extern HWND hOpenGLWnd;
	extern HWND hOpenGLWnd_Lattice;
	extern HWND hOpenGLWnd_VerticesNormals;
	extern HWND hOpenGLWnd_FacesNormals;
	extern HWND hOpenGLWnd_FacesColors;
	
extern HWND hModelParamsWnd;
	extern HWND hColorIDCbx;
	extern HWND hReflectionCbx;
	extern HWND hWheelSteerCbx;
	extern HWND hDebrisPowerCbx;
	extern HWND hSlotIDCbx;
	extern HWND hSlotRotationCbx;
	extern HWND hModelParamsCloseBtn;
	extern HWND hModelParamsApplyBtn;
	extern HWND hModelTypeCbx;

extern HWND hMechousParamsWnd;
	extern HWND hMechousParamsOkBtn;
	extern HWND hMechousParamsCancelBtn;

struct ControlParams 
{
	LPCSTR Caption;
	RECT r;
	DWORD Style;
	HWND Parent;
	HMENU ControlID;
	HINSTANCE hInstance;
};

HWND ComboBoxWnd_CreateWnd(ControlParams *cp);
HWND ButtonWnd_CreateWnd(ControlParams *cp);
HWND StaticWnd_CreateWnd(ControlParams *cp);
HWND EditWnd_CreateWnd(ControlParams *cp);

void RedirectDropFiles(WPARAM wParam, LPARAM lParam);
void EnableWindowCheck(HWND hWnd, BOOL Enable);