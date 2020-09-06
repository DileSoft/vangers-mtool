#include <windows.h>

#include "Globals.h"
#include "OpenSaveModule.h"

#define FILE_NAME_MAX	(1024 * 5)

char *FileName_Open_ASC = NULL;
char *FileName_Save_MDL = NULL;

const char OpenDialog_Title_Open_ASC[] = "Open some ascii file...";
const char OpenDialog_Filter_ASC[] = " All types (ASC, ASE, M3D)\0*.asc;*.ase;*.m3d\0\0";

const char OpenDialog_Title_Save_MDL[] = "Save model as...";
const char OpenDialog_Filter_MDL[] = " Vangers M3D model (assembly model)\0*.m3d\0\
 Wavefront OBJ model (selected only)\0*.obj\0\
 Wavefront OBJ model (all models)\0*.obj\0\0";

OPENFILENAMEA OpenFileInfo;

DWORD OpenDialog_Flags = OFN_DONTADDTORECENT | OFN_ENABLESIZING | OFN_HIDEREADONLY | OFN_NONETWORKBUTTON |
							OFN_PATHMUSTEXIST | OFN_ENABLEHOOK | OFN_EXPLORER | OFN_ALLOWMULTISELECT;

DWORD OpenDialog_Flags_Open = OFN_FILEMUSTEXIST;
DWORD OpenDialog_Flags_Save = OFN_OVERWRITEPROMPT;

UINT_PTR CALLBACK DlgHookProc(HWND hdlg, UINT uiMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uiMsg) 
	{
		case WM_INITDIALOG :
		{
			RECT r;

			HWND hDlg = GetParent(hdlg);
			GetWindowRect(hDlg, &r);

			int w = r.right - r.left;
			int h = r.bottom - r.top;

			int ws = GetSystemMetrics(SM_CXSCREEN);
			int hs = GetSystemMetrics(SM_CYSCREEN);

			SetWindowPos(hDlg, HWND_TOP,
							(ws - w) / 2,
							(hs - h) / 2,
							0, 0, SWP_NOSIZE);

			break;
		}
	}

  return 0;
}

void InitOpenFileInfo(HINSTANCE hInstance)
{
	ZeroMemory(&OpenFileInfo, sizeof(OPENFILENAMEA));

	OpenFileInfo.hwndOwner = hMainWnd;
	OpenFileInfo.lStructSize = sizeof(OPENFILENAMEA);
	OpenFileInfo.hInstance = hInstance;
	OpenFileInfo.nFilterIndex = 1;
	OpenFileInfo.nMaxFile = FILE_NAME_MAX - 1;
	OpenFileInfo.lpstrInitialDir = ".";
	OpenFileInfo.lpfnHook = DlgHookProc;
	OpenFileInfo.lpstrDefExt = "";

	FileName_Open_ASC = new char [FILE_NAME_MAX];
	ZeroMemory(FileName_Open_ASC, FILE_NAME_MAX);

	FileName_Save_MDL = new char [FILE_NAME_MAX];
	ZeroMemory(FileName_Save_MDL, FILE_NAME_MAX);
};

void FreeOpenFileInfo()
{
	delete [] FileName_Open_ASC;
	delete [] FileName_Save_MDL;
};

BOOL GetOpenFileName_ASC(char *FileName)
{
  OpenFileInfo.lpstrFilter = OpenDialog_Filter_ASC;
  OpenFileInfo.lpstrFile = FileName;
  OpenFileInfo.lpstrTitle = OpenDialog_Title_Open_ASC;
  OpenFileInfo.lpstrDefExt = "";
  OpenFileInfo.Flags = OpenDialog_Flags | OpenDialog_Flags_Open;
	
  return GetOpenFileNameA(&OpenFileInfo);
};

BOOL GetSaveFileName_MDL(char *FileName)
{
  OpenFileInfo.lpstrFilter = OpenDialog_Filter_MDL;
  OpenFileInfo.lpstrFile = FileName;
  OpenFileInfo.lpstrTitle = OpenDialog_Title_Save_MDL;
  OpenFileInfo.lpstrDefExt = "";
  OpenFileInfo.Flags = OpenDialog_Flags | OpenDialog_Flags_Save;

  return GetSaveFileNameA(&OpenFileInfo);
};