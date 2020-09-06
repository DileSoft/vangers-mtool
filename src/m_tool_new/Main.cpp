#include <windows.h>

#include "Globals.h"
#include "MainWindow.h"
#include "PreviewWindow.h"
#include "ModelSettings.h"
#include "MechousSettings.h"
#include "OpenSaveModule.h"
#include "ExportDialog.h"

#define INTERFACE_ERROR				"Whoooa! Interface error!"
#define INTERFACE_ERROR_MAIN		"Some problems with main window..."
#define INTERFACE_ERROR_PREVIEW		"Some problems with preview window..."
#define INTERFACE_ERROR_SETTINGS	"Some problems with model params window..."
#define INTERFACE_ERROR_MECHOUS		"Some problems with mechous params window..."

// Creates normal application font or 
// gets default font if an error occured
BOOL CreatePrettyFonts()
{
	hFont = CreateFontA(13, 0, 0, 0, FW_NORMAL, 0, 0, 0, DEFAULT_CHARSET,
				OUT_TT_PRECIS, CLIP_DEFAULT_PRECIS, PROOF_QUALITY,
				DEFAULT_PITCH | FF_DONTCARE, "Tahoma");

	if (!hFont)	
	{
		hFont = (HFONT) GetStockObject(ANSI_VAR_FONT);
		return FALSE;
	}

	return TRUE;
};

void InterfaceError(BYTE Code)
{
	switch (Code)
	{
		case 0 : MessageBox(0, INTERFACE_ERROR, INTERFACE_ERROR_MAIN, MB_OK | MB_ICONSTOP); break;
		case 1 : MessageBox(0, INTERFACE_ERROR, INTERFACE_ERROR_PREVIEW, MB_OK | MB_ICONSTOP); break;
		case 2 : MessageBox(0, INTERFACE_ERROR, INTERFACE_ERROR_SETTINGS, MB_OK | MB_ICONSTOP); break;
		case 3 : MessageBox(0, INTERFACE_ERROR, INTERFACE_ERROR_MECHOUS, MB_OK | MB_ICONSTOP); break;
	}
};

// This is the ENTRY POINT to the application
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInst, LPSTR CmdString, int CmdShow)
{		
	BOOL InterfaceOk = FALSE;
	BOOL PrettyFonts = CreatePrettyFonts();
	
	/*
	BOOL AppCreated = CreateApplicationWindow(hInstance);
	if (AppCreated)	InterfaceOk = CreateMainWindow(hInstance, hApplicationWnd);
	*/

	InterfaceOk = CreateMainWindow(hInstance, 0);

	if (InterfaceOk)
	{
		// Important!
		Models.ModelsListBox = hListBoxWnd;

		BOOL PreviewCreated = CreatePreviewWindow(hInstance, hMainWnd);
		BOOL ModelParamsCreated = CreateModelParamsWindow(hInstance, hMainWnd);
		BOOL MechousParamsCreated = CreateMechousParamsWindow(hInstance, hMainWnd);

		if (!PreviewCreated) 
		{
			DestroyWindow(hMainWnd);
			InterfaceError(1);
		}

		if (!ModelParamsCreated) 
		{
			DestroyWindow(hMainWnd);
			InterfaceError(2);
		}

		if (!MechousParamsCreated) 
		{
			DestroyWindow(hMainWnd);
			InterfaceError(3);
		}

		InterfaceOk = (PreviewCreated &&
			ModelParamsCreated &&
			MechousParamsCreated);
	}
	else
		InterfaceError(0);

	if (InterfaceOk) 
	{
		InitOpenFileInfo(hInstance);
		ExportDialog_hInstance = hInstance;

		// ShowWindow(hApplicationWnd, CmdShow);
		ShowWindow(hMainWnd, CmdShow);

		// Testing...
		// ExportDialog_Show(hMainWnd);

		MSG Msg;
		while (GetMessageA(&Msg, 0, 0, 0))
			if (!(IsDialogMessage(hMainWnd, &Msg) || 
				  IsDialogMessage(hModelParamsWnd, &Msg) ||
				  IsDialogMessage(hMechousParamsWnd, &Msg)))
			{		
				TranslateMessage(&Msg);
				DispatchMessageA(&Msg);
			}

		FreeOpenFileInfo();
	}

	if (PrettyFonts) 
	{
		if (hFont) DeleteObject(hFont);
	}

	return 0;
};