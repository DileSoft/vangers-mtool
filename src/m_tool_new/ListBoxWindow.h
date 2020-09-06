#include <windows.h>

#include "ModelClass.h"
#include "ModelSettings.h"

extern const int ListBoxWnd_Width;
extern const int ListBoxWnd_Height;
extern const int ListBoxWnd_Left;
extern const int ListBoxWnd_Top;

struct ListBoxItem
{
	char *ItemName;		// Name in the list box
	Model *ItemModel;	// Address of the linked model
};

void ListBoxWnd_ApplyParams(HWND hWnd, ModelParamsWnd_Params &mpwp, BOOL KeepParams);
void ListBoxWnd_DeleteItem(LPDELETEITEMSTRUCT lpdel);
void ListBoxWnd_SelectionChanged(HWND hWnd);
void ListBoxWnd_RemoveSubclass(HWND hWnd);
void ListBoxWnd_ModelParamsButton(HWND hWnd);
void ListBoxWnd_MakeModelName(ListBoxItem *Item);

HWND ListBoxWnd_CreateWnd(HINSTANCE hInstance, HWND Parent);