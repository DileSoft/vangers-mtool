#include <windows.h>
#include <float.h>

#include "Globals.h"
#include "MainWindow.h"
#include "ListBoxWindow.h"
#include "OpenSaveModule.h"
#include "OpenGLWindow.h"
#include "ExportDialog.h"

const char MainWnd_Caption[] = " Mechous tool new 0.3";
const char MainWnd_ClassName[] = "TVangersMainWindowClassName";

const char *ApplicationWnd_Caption = MainWnd_Caption;
const char ApplicationWnd_ClassName[] = "TVangersApplicationClassName";

const int MainWnd_Width = 420;
const int MainWnd_Height = 350;

const int MainWnd_TopDelta = 43;
const int ListBoxWnd_Right = ListBoxWnd_Width + 2 * ListBoxWnd_Left;

const int MainWnd_ButtonsCount = 7;

const int ButtonsIDs[] =
{
	BTN_LOADMODELS,
	BTN_SAVEMECHOUS,
	BTN_SHOWPREVIEW,
	BTN_CENTERMODEL,
	BTN_CLEARLIST,
	BTN_MODELPARAMSSHOW,
	BTN_MECHOUSPARAMSSHOW
};

const RECT ButtonsRects[] =
{
	{ListBoxWnd_Right, MainWnd_TopDelta * 0 + 15, 120, 28},
	{ListBoxWnd_Right, MainWnd_TopDelta * 1 + 15, 120, 28},
	{ListBoxWnd_Right, MainWnd_TopDelta * 2 + 15, 120, 28},
	{ListBoxWnd_Right, MainWnd_TopDelta * 3 + 15, 120, 28},
	{ListBoxWnd_Right, MainWnd_TopDelta * 4 + 15, 120, 28},
	{ListBoxWnd_Right, MainWnd_TopDelta * 5 + 15, 120, 28},
	{ListBoxWnd_Right, MainWnd_TopDelta * 6 + 15, 120, 28}
};

const char *ButtonsCaptions[] =
{
	"Load model...",
	"Export model...",
	"Show preview",
	"Center all",
	"Clear list",
	"Model params",
	"Mechous params"
};

const DWORD ButtonsStyles[] =
{
	BS_PUSHBUTTON | WS_TABSTOP,
	BS_PUSHBUTTON | WS_TABSTOP,
	BS_PUSHBUTTON | WS_TABSTOP,
	BS_PUSHBUTTON | WS_TABSTOP,
	BS_PUSHBUTTON | WS_TABSTOP,
	BS_PUSHBUTTON | WS_TABSTOP,
	BS_PUSHBUTTON | WS_TABSTOP
};

void MainWnd_ShowPreview()
{
	if (!IsWindowVisible(hPreviewWnd))
		ShowWindow(hPreviewWnd, SW_SHOWNORMAL);
};

char *PathFindExtension(const char *Path)
{
	char *EndPath = (char *) Path + strlen(Path) - 1;

	while ((EndPath > Path) && (*EndPath != '.'))
		EndPath--;

	return EndPath;
};

BOOL MainWnd_PureLoadModel(char *FileName)
{
	BOOL Success = FALSE;

	char *Ext = PathFindExtension(FileName);
	
	// Lock OpenGL update
	OpenGLWnd_Pause = TRUE;

	if (lstrcmpiA(Ext, ".ASE") == 0)
		Success = Models.LoadASE(FileName);
	else
		if (lstrcmpi(Ext, ".ASC") == 0)
			Success = Models.LoadASC(FileName);
		else
			if (lstrcmpi(Ext, ".M3D") == 0)
				Success = Models.LoadM3D(FileName);

	// Unlock OpenGL update
	OpenGLWnd_Pause = FALSE;

	return Success;
};

/*
void MainWnd_EnableClearListButton()
{
	if ((Models.ModelsCount) && (!IsWindowEnabled(hClearListBtn)))
		EnableWindow(hClearListBtn, TRUE);
};
*/

void MainWnd_LoadModels()
{
	if (GetOpenFileName_ASC(FileName_Open_ASC))
	{
		char *FileName = FileName_Open_ASC;	
		SIZE_T FileNameLen = strlen(FileName);

		BOOL SingleFile = (FileNameLen > OpenFileInfo.nFileOffset);

		if (SingleFile)
			MainWnd_PureLoadModel(FileName);
		else
		{
			SIZE_T FullPathLen = 0;
			SIZE_T PathLen = FileNameLen + 1; // for '\'

			char *PureFileName = NULL;
			
			FileName += OpenFileInfo.nFileOffset;
			FileNameLen = strlen(FileName);

			while (FileNameLen)
			{
				FullPathLen = PathLen + FileNameLen + 1; // for '\0'
				PureFileName = new char [FullPathLen];
				ZeroMemory(PureFileName, FullPathLen);
				
				lstrcatA(PureFileName, FileName_Open_ASC);
				lstrcatA(PureFileName, "\\");
				lstrcatA(PureFileName, FileName);

				MainWnd_PureLoadModel(PureFileName);

				delete [] PureFileName;

				FileName += FileNameLen + 1;
				FileNameLen = strlen(FileName);
			}
		}			

		// Enable clear list button
		// MainWnd_EnableClearListButton();

		// Update 3D preview
		OpenGLWnd_UpdateWindow();
	}
};

// Export to m3d format
void MainWnd_ExportMechous()
{
	if (GetSaveFileName_MDL(FileName_Save_MDL))
	{
		switch (OpenFileInfo.nFilterIndex)
		{
			case SAVE_FILTER_MDL_INDEX : 
			{ 
				ExportDialog_Show(hMainWnd, SAVING_TYPE_MDL); 
				break; 
			}
			
			case SAVE_FILTER_OBJ_INDEX_SEL : 
			{
				ExportDialog_Show(hMainWnd, SAVING_TYPE_OBJ_SEL);
				break;
			}

			case SAVE_FILTER_OBJ_INDEX_ALL : 
			{
				ExportDialog_Show(hMainWnd, SAVING_TYPE_OBJ_ALL);
				break;
			}
		}	
			

	}
};

// Ñenter model in the space :)
void MainWnd_CenterModel()
{
	Vertex Offset;
	Vertex Min, Max;

	Max[0] = Max[1] = Max[2] = -FLT_MAX;
	Min[0] = Min[1] = Min[2] =  FLT_MAX;

	// Lock OpenGL update
	OpenGLWnd_Pause = TRUE;

	// Max and min of the models
	for (int i = 0; i < Models.ModelsCount; i++)
	{
		Model *m = Models.GetModel(i);

		for (int j = 0; j < 3; j++)
		{
			if (m -> Max[j] > Max[j]) Max[j] = m -> Max[j];
			if (m -> Min[j] < Min[j]) Min[j] = m -> Min[j];
		}
	}

	// Center of the body
	Offset[0] = (Max[0] + Min[0]) / 2;
	Offset[1] = (Max[1] + Min[1]) / 2;
	Offset[2] = (Max[2] + Min[2]) / 2;

	// Apply offset
	for (int i = 0; i < Models.ModelsCount; i++)
	{
		Model *m = Models.GetModel(i);
		m -> SetOffset(Offset);
	}

	// Unlock OpenGL update
	OpenGLWnd_Pause = FALSE;

	// Update 3D preview
	OpenGLWnd_UpdateWindow();
};

// Select items in the 3D preview
void MainWnd_SelectionChanged()
{
	ListBoxWnd_SelectionChanged(hListBoxWnd);
};

// Clear models list
void MainWnd_ClearList()
{
	if (MessageBox(hMainWnd, "Do you really want to clear list?    ", 
			"Confirm clearing list", MB_OKCANCEL | MB_ICONQUESTION) == IDOK)
	{
		SendMessageA(hListBoxWnd, LB_RESETCONTENT, 0, 0);
		ListBoxWnd_SelectionChanged(hListBoxWnd);
	
		// Update 3D preview
		OpenGLWnd_UpdateWindow();
	}
};

// Show model params
void MainWnd_ShowModelParams()
{
	ListBoxWnd_ModelParamsButton(hListBoxWnd);
};

void MainWnd_ShowMechousParams()
{
	ShowWindow(hMechousParamsWnd, SW_SHOW);
};

LRESULT CALLBACK MainWnd_WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
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
						case BTN_SHOWPREVIEW : { MainWnd_ShowPreview(); break; }
						case BTN_LOADMODELS : { MainWnd_LoadModels(); break; }
						case BTN_CENTERMODEL : { MainWnd_CenterModel(); break; }
						case BTN_CLEARLIST : { MainWnd_ClearList(); break; }
						case BTN_MODELPARAMSSHOW : { MainWnd_ShowModelParams(); break; }
						case BTN_MECHOUSPARAMSSHOW : { MainWnd_ShowMechousParams(); break; }
						case BTN_SAVEMECHOUS : { MainWnd_ExportMechous(); break; }
					}
				
					break;
				}

				case LBN_SELCHANGE :
				{
					switch LOWORD(wParam)
					{
						case LBX_MODELPARTS : { MainWnd_SelectionChanged(); break; }
					}

					break;
				}
			}
		
		break;
		}
		
		case WM_DRAWITEM :
		{       
			LPDRAWITEMSTRUCT lpdis = LPDRAWITEMSTRUCT(lParam);

			if ((lpdis -> CtlType == ODT_LISTBOX) && 
				(lpdis -> CtlID == LBX_MODELPARTS))
			{
				SendMessageA(hListBoxWnd, WM_DRAWITEM, wParam, lParam);
				return TRUE;
			}

			break;
		}
		
		case WM_COMPAREITEM :
		{
			LPCOMPAREITEMSTRUCT lpcis = LPCOMPAREITEMSTRUCT(lParam);
			
			if ((lpcis -> CtlType == ODT_LISTBOX) && 
				(lpcis -> CtlID == LBX_MODELPARTS))
			{
				return SendMessageA(hListBoxWnd, WM_COMPAREITEM, wParam, lParam);
			}

			break;
		}
		
		case WM_DELETEITEM :
		{
			LPDELETEITEMSTRUCT lpdel = LPDELETEITEMSTRUCT(lParam);

			if ((lpdel -> CtlType == ODT_LISTBOX) && 
				(lpdel -> CtlID == LBX_MODELPARTS))
			{
				ListBoxWnd_DeleteItem(lpdel);
				return TRUE;
			}

			break;
		};

		case WM_DROPFILES :
		{
			HDROP hDrop = (HDROP) wParam;

			UINT FilesCount = DragQueryFileA(hDrop, int (-1), NULL, 0);

			// Lock window update
			SendMessageA(hListBoxWnd, WM_SETREDRAW, (WPARAM) FALSE, 0);

			char *FileName = NULL;
			for (UINT i = 0; i < FilesCount; i++)
			{
				UINT FileNameLen = DragQueryFileA(hDrop, i, NULL, 0) + 1;

				FileName = new char [FileNameLen];
				DragQueryFileA(hDrop, i, FileName, FileNameLen);

				// No extension check needed.
				// It will be performed in MainWnd_PureLoadModel()
				MainWnd_PureLoadModel(FileName);

				delete [] FileName;
			}

			// Finish drag drop
			DragFinish(hDrop);
			
			// Redraw window
			SendMessageA(hListBoxWnd, WM_SETREDRAW, (WPARAM) TRUE, 0);
			InvalidateRect(hListBoxWnd, NULL, FALSE);

			// Enable clear list button
			// MainWnd_EnableClearListButton();

			// Update 3D preview
			OpenGLWnd_UpdateWindow();	

			// Bring window to top
			SetForegroundWindow(hWnd);

			break;
		}

		case WM_DESTROY : 
		{  
			ListBoxWnd_RemoveSubclass(hListBoxWnd);
			PostQuitMessage(0); 
			break; 
		}
		
		// ÊÎÌÏÎÒ 1 !!!
		default : return DefWindowProcA(hWnd, uMsg, wParam, lParam);
	}
	
	return FALSE;
};


HWND MainWnd_CreateWnd(HINSTANCE hInstance, HWND Parent)
{
	WNDCLASSEXA ws;

	ZeroMemory(&ws, sizeof(WNDCLASSEXA));

	ws.cbSize = sizeof(WNDCLASSEXA);
	ws.hbrBackground = HBRUSH(COLOR_BTNFACE + 1);
	ws.hCursor = LoadCursor(0, IDC_ARROW);
	ws.hIcon = LoadIcon(0, IDI_APPLICATION);
	ws.hInstance = hInstance;
	ws.lpfnWndProc = &MainWnd_WndProc;
	ws.lpszClassName = MainWnd_ClassName;
	ws.style = CS_OWNDC;

	if (!RegisterClassExA(&ws)) return 0;

	int Left = (GetSystemMetrics(SM_CXSCREEN) - MainWnd_Width) / 2; 
    int Top = (GetSystemMetrics(SM_CYSCREEN) - MainWnd_Height) / 2; 

	HWND hWnd = CreateWindowExA(WS_EX_ACCEPTFILES, MainWnd_ClassName, MainWnd_Caption,
					WS_SYSMENU | WS_CAPTION | WS_MINIMIZEBOX, 
					Left, Top, MainWnd_Width, MainWnd_Height,
					Parent, 0, hInstance, NULL);

	if (!hWnd) UnregisterClassA(MainWnd_ClassName, hInstance);
	
	return hWnd;
};

// Creates all buttons in main window
void MainWnd_CreateButtons(HINSTANCE hInstance, HWND Parent, HWND *ButtonsHandles)
{
	ControlParams cp;

	cp.hInstance = hInstance;
	cp.Parent = Parent;

	for (int i = 0; i < MainWnd_ButtonsCount; i++)
	{
		cp.ControlID = (HMENU) (INT_PTR) ButtonsIDs[i];
		cp.Caption = ButtonsCaptions[i];
		cp.Style = ButtonsStyles[i];
		cp.r = ButtonsRects[i];

		ButtonsHandles[i] = ButtonWnd_CreateWnd(&cp);
	}
};

// Creates MainWindow and all of it's childrens
BOOL CreateMainWindow(HINSTANCE hInstance, HWND Parent)
{
	hMainWnd = MainWnd_CreateWnd(hInstance, Parent);
	
	if (hMainWnd) 
	{
		hListBoxWnd = ListBoxWnd_CreateWnd(hInstance, hMainWnd);
		
		HWND ButtonsHandles[MainWnd_ButtonsCount];
		MainWnd_CreateButtons(hInstance, hMainWnd, ButtonsHandles);

		hLoadModelsBtn	 = ButtonsHandles[0];
		hSaveMechousBtn	 = ButtonsHandles[1];
		hShowPreviewBtn	 = ButtonsHandles[2];
		hCenterModelBtn	 = ButtonsHandles[3];
		hClearListBtn	 = ButtonsHandles[4];
		hShowModelParamsBtn = ButtonsHandles[5];
		hShowMechousParamsBtn = ButtonsHandles[6];
	}

	return (hMainWnd && hListBoxWnd);
};

/*
// ------------------
// APPLICATION WINDOW
// ------------------

LRESULT CALLBACK ApplicationWnd_WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	return DefWindowProcA(hWnd, uMsg, wParam, lParam);
};

HWND ApplicationWnd_CreateWnd(HINSTANCE hInstance)
{
	WNDCLASSEXA ws;

	ZeroMemory(&ws, sizeof(WNDCLASSEXA));

	ws.cbSize = sizeof(WNDCLASSEXA);
	ws.hbrBackground = HBRUSH(COLOR_BTNFACE + 1);
	ws.hCursor = LoadCursor(0, IDC_ARROW);
	ws.hIcon = LoadIcon(0, IDI_APPLICATION);
	ws.hInstance = hInstance;
	ws.lpfnWndProc = &ApplicationWnd_WndProc;
	ws.lpszClassName = ApplicationWnd_ClassName;
	ws.style = CS_OWNDC;

	if (!RegisterClassExA(&ws)) return 0;

	HWND hWnd = CreateWindowExA(0, ApplicationWnd_ClassName, ApplicationWnd_Caption,
					WS_POPUP | WS_SYSMENU | WS_MINIMIZE | WS_CAPTION, 0, 0, 0, 0, 0, 0, hInstance, NULL);

	if (hWnd) 
	{
		HMENU hMenu = GetSystemMenu(hWnd, FALSE);
		RemoveMenu(hMenu, SC_MOVE, MF_BYCOMMAND);
		RemoveMenu(hMenu, SC_SIZE, MF_BYCOMMAND);
		RemoveMenu(hMenu, SC_MAXIMIZE, MF_BYCOMMAND);
	}
	else 
		UnregisterClassA(ApplicationWnd_ClassName, hInstance);
	
	return hWnd;
};

BOOL CreateApplicationWindow(HINSTANCE hInstance)
{
	hApplicationWnd = ApplicationWnd_CreateWnd(hInstance);
	return (hApplicationWnd != 0);
};
*/
