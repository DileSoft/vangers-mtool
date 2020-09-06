#include <windows.h>

extern HINSTANCE ExportDialog_hInstance;

enum ExportDialog_Type 
{
	SAVING_TYPE_MDL, 
	SAVING_TYPE_OBJ_SEL,
	SAVING_TYPE_OBJ_ALL
};

void ExportDialog_Show(HWND Parent, ExportDialog_Type SavingType);
