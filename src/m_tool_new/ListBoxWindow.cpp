#include <windows.h>
#include <stdio.h>

#include "Globals.h"
#include "ListBoxWindow.h"
#include "OpenGLWindow.h"


const int LISTBOX_ITEMS_COUNT = 18;
const int LISTBOX_ITEM_HEIGHT = 16;

// All ListBoxWnd_xxx to namespace?..
// Am I WinAPI fan???

const int ListBoxWnd_Width = 250;
const int ListBoxWnd_Height = LISTBOX_ITEMS_COUNT * LISTBOX_ITEM_HEIGHT + 6;
const int ListBoxWnd_Left = 15;
const int ListBoxWnd_Top = 15;

int ListBoxWnd_ClickedItem = -1;

WNDPROC ListBoxWnd_WndProcOrig;

// Popup menu
HMENU ListBoxWnd_Menu = 0;
int ListBoxWnd_Menu_CheckedID = 0;

// Menu item's text
const char *ListBoxWnd_MenuItems[] = 
{
	"Toggle visible",
	"Body parts",
	"Wheel",
	"Debris",
	"Slot",
	"Unknown",
	"Delete model",
	"Show params"
};

const char *ListBoxWnd_SubMenuItems[] = 
{
	"Body",
	"Glass",
	"Metal",
	"Tube"
};

// Menu items IDs
// 1..8 is (MODEL_MATERIAL_X + 1)
// 0 is reserved for separator
#define MENU_HIDE_MODEL		9
#define MENU_BODY_BODY		(MODEL_MATERIAL_BODY	 + 1)
#define MENU_BODY_GLASS		(MODEL_MATERIAL_GLASS	 + 1)
#define MENU_BODY_METAL		(MODEL_MATERIAL_METAL	 + 1)
#define MENU_BODY_TUBE		(MODEL_MATERIAL_TUBE	 + 1)
#define MENU_WHEEL			(MODEL_MATERIAL_WHEEL	 + 1)
#define MENU_DEBRIS			(MODEL_MATERIAL_DEBRIS	 + 1)
#define MENU_SLOT			(MODEL_MATERIAL_SLOT	 + 1)
#define MENU_UNKNOWN		(MODEL_MATERIAL_UNKNOWN	 + 1)
#define MENU_DELETE_MODEL	10
#define MENU_SHOW_PARAMS	11

// Draw enabled flag. Eah! Really simple :) 
// Gimmeh GDI+ here! It's so smoooth... :)
void ListBoxWnd_DrawEnabledFlag(BOOL Enabled, HDC DC, RECT *r)
{
	HPEN hPen = (HPEN) GetStockObject(WHITE_PEN);
	HPEN hOldPen = (HPEN) SelectObject(DC, hPen);
	
	HBRUSH hBrush;	

	if (Enabled)
		hBrush = CreateSolidBrush(RGB(0, 200, 0));
	else
		hBrush = CreateSolidBrush(RGB(200, 0, 0));

	HBRUSH hOldBrush = (HBRUSH) SelectObject(DC, hBrush);
	
	Ellipse(DC, r -> left + 4, r -> top + 3, r -> left + 14, r -> top + 13);

	SelectObject(DC, hOldPen);
	SelectObject(DC, hOldBrush);

	DeleteObject(hPen);
	DeleteObject(hBrush);
};

void ListBoxWnd_DrawItem(HWND hWnd, LPDRAWITEMSTRUCT lpdis)
{
	// Hmm...
	if ((int) lpdis -> itemID < 0)
	{
		SetTextColor(lpdis -> hDC, GetSysColor(COLOR_WINDOWTEXT));			
		DrawFocusRect(lpdis -> hDC, &(lpdis -> rcItem));		
		return ;
	}

	// Get item's data
	ListBoxItem *Item = (ListBoxItem *) (lpdis -> itemData);

	// Checks whether item is selected
	switch (lpdis -> itemState & ODS_SELECTED)
	{	
		case ODS_SELECTED :	
		{
			if (GetFocus() == hWnd) 
			{
				SetTextColor(lpdis -> hDC, GetSysColor(COLOR_HIGHLIGHTTEXT));
				FillRect(lpdis -> hDC, &(lpdis -> rcItem), HBRUSH(COLOR_HIGHLIGHT + 1));
			}
			else 
			{
				SetTextColor(lpdis -> hDC, GetSysColor(COLOR_INACTIVECAPTIONTEXT));
				FillRect(lpdis -> hDC, &(lpdis -> rcItem), HBRUSH(COLOR_INACTIVECAPTION + 1));		
			}

			break;
		}

		case 0 :
		{
			SetTextColor(lpdis -> hDC, GetSysColor(COLOR_WINDOWTEXT));		
			
			// Fill color based on model's type
			COLORREF Color = ModelColors[Item -> ItemModel -> ModelType];
			
			HBRUSH hBrush = CreateSolidBrush(Color);
			FillRect(lpdis -> hDC, &(lpdis -> rcItem), hBrush);	
			DeleteObject(hBrush);
	
			break;
		}
	}

	// Call it here...
	ListBoxWnd_DrawEnabledFlag(Item -> ItemModel -> ModelEnabled, lpdis -> hDC, &(lpdis -> rcItem));

	// Text drawing code...
	int oldBkMode = SetBkMode(lpdis -> hDC, TRANSPARENT);
	
		// Draw model's name
		int x = lpdis -> rcItem.left + 19;
		int y = lpdis -> rcItem.top + 1;		

		RECT NameRect = {x, y, 170, 14};

		SetTextAlign(lpdis -> hDC, TA_LEFT | TA_TOP);

		// Validate model's name
		char *Text = ((ListBoxItem *) (lpdis -> itemData)) -> ItemModel -> ModelName;
		if (!Text) Text = "";

		DrawTextA(lpdis -> hDC, Text, (int) strlen(Text), &NameRect, DT_NOPREFIX | DT_NOCLIP | DT_WORD_ELLIPSIS);
	
		// Draw model's params		
		x = lpdis -> rcItem.left + 225;

		SetTextAlign(lpdis -> hDC, TA_RIGHT | TA_TOP);
		Text = ((ListBoxItem *) (lpdis -> itemData)) -> ItemName;
		if (Text) TextOutA(lpdis -> hDC, x, y, Text, (int) strlen(Text));

	SetBkMode(lpdis -> hDC, oldBkMode);

	// Checks wheter item is focused
	if (lpdis -> itemAction & ODA_FOCUS)
	{
		if (lpdis -> itemState & ODS_FOCUS)	
		{
			SetTextColor(lpdis -> hDC, GetSysColor(COLOR_WINDOWTEXT));			
			DrawFocusRect(lpdis -> hDC, &(lpdis -> rcItem));
		}
	}
};

// Determine group of the item and sort inside it's group.
// If ungrouped item (MODEL_UNKNOWN) then just sort alphabetically
int ListBoxWnd_CompareItem(LPCOMPAREITEMSTRUCT lpcis)
{
	Model *m1 = ((ListBoxItem *) (lpcis -> itemData1)) -> ItemModel;
	Model *m2 = ((ListBoxItem *) (lpcis -> itemData2)) -> ItemModel;

	if ((m1 -> ModelType) > (m2 -> ModelType)) return -1;
	else
		if ((m1 -> ModelType) < (m2 -> ModelType)) return 1;
		else 
			return lstrcmpiA(m1 -> ModelName, m2 -> ModelName);
};

// Select focused item
void ListBoxWnd_SelectFocused(HWND hWnd)
{
	long Index = (long) SendMessageA(hWnd, LB_GETCARETINDEX, 0, 0);				
	if (Index != LB_ERR) SendMessageA(hWnd, LB_SETSEL, (WPARAM) TRUE, (LPARAM) Index);

	ListBoxWnd_SelectionChanged(hWnd);
};

// Select item at Index position.
// Function handles Index overflow
void ListBoxWnd_SelectItem(HWND hWnd, int Index)
{
	long Count = (long) SendMessageA(hWnd, LB_GETCOUNT, 0, 0);
	
	if (Count > 0)
	{
		if (Index >= Count) Index = Count - 1;
		SendMessageA(hWnd, LB_SETSEL, (WPARAM) TRUE, (LPARAM) Index);
	}

	ListBoxWnd_SelectionChanged(hWnd);
};

void ListBoxWnd_DeleteKey(HWND hWnd, WPARAM wParam)
{
	// Is anything selected?
	long SelCount = (long) SendMessageA(hWnd, LB_GETSELCOUNT, 0, 0);
	long SelIndex = (long) SendMessageA(hWnd, LB_GETCARETINDEX, 0, 0);

	// Exit if nothing selected
	if (SelCount <= 0) return ; 

	// Lock window update
	SendMessageA(hWnd, WM_SETREDRAW, (WPARAM) FALSE, 0);
	OpenGLWnd_Pause = TRUE;
	
	// Array of indices of selected items
	int *ItemsIndices = new int [SelCount];
			
	// Fill array with indexes of selected items
	SendMessageA(hWnd, LB_GETSELITEMS, (WPARAM) SelCount, (LPARAM) ItemsIndices);
	
	for (int i = SelCount; i > 0; i--) 
	{
		int Index = ItemsIndices[i - 1];
		SendMessageA(hWnd, LB_DELETESTRING, (WPARAM) Index, 0);				
	}

	delete [] ItemsIndices;

	// Select item at deleted position
	ListBoxWnd_SelectItem(hWnd, SelIndex);

	// Redraw window
	SendMessageA(hWnd, WM_SETREDRAW, (WPARAM) TRUE, 0);
	InvalidateRect(hWnd, NULL, FALSE);

	// Update 3D preview
	OpenGLWnd_Pause = FALSE;
	OpenGLWnd_UpdateWindow();
};

// Reset boxed state of all models
void ListBoxWnd_ResetModelsSelection()
{
	for (int i = 0; i < Models.ModelsCount; i++)
	{
		Model *m = Models.GetModel(i);
		m -> ModelBoxed = FALSE;
	}
};

// Deselect all items
void ListBoxWnd_SelectNone(HWND hWnd)
{
	SendMessageA(hWnd, LB_SETSEL, (WPARAM) FALSE, (LPARAM) -1);
	
	// Reset models selection
	ListBoxWnd_ResetModelsSelection();
	ListBoxWnd_SelectionChanged(hWnd);
};

// Deselect all and select only focused
void ListBoxWnd_SelectOne(HWND hWnd)
{
	SendMessageA(hWnd, LB_SETSEL, (WPARAM) FALSE, (LPARAM) -1);
	ListBoxWnd_SelectFocused(hWnd);
};

// Select all items
void ListBoxWnd_SelectAll(HWND hWnd)
{
	SendMessageA(hWnd, LB_SETSEL, (WPARAM) TRUE, (LPARAM) -1);
	ListBoxWnd_SelectionChanged(hWnd);
};


// Mmm... Same code... Bad...
void ListBoxWnd_ToggleEnable(HWND hWnd)
{
	long SelCount = (long) SendMessageA(hWnd, LB_GETSELCOUNT, 0, 0);
	if (SelCount <= 0) return ; 

	int *ItemsIndices = new int [SelCount];	
	SendMessageA(hWnd, LB_GETSELITEMS, (WPARAM) SelCount, (LPARAM) ItemsIndices);

	RECT r;
	HDC hWndDC = GetDC(hWnd);
	long TopIndex = (long) SendMessageA(hWnd, LB_GETTOPINDEX, 0, 0);

	for (int i = SelCount; i > 0; i--) 
	{
		int Index = ItemsIndices[i - 1];
		ListBoxItem *Item = (ListBoxItem *) SendMessageA(hWnd, LB_GETITEMDATA, (WPARAM) Index, 0);				
		
		Item -> ItemModel -> ModelEnabled = !Item -> ItemModel -> ModelEnabled;
		
		if ((Index >= TopIndex) && (Index < TopIndex + LISTBOX_ITEMS_COUNT))
		{
			SendMessageA(hWnd, LB_GETITEMRECT, (WPARAM) Index, (LPARAM) &r);
			ListBoxWnd_DrawEnabledFlag(Item -> ItemModel -> ModelEnabled, hWndDC, &r);
		}
	}

	ReleaseDC(hWnd, hWndDC);
	delete [] ItemsIndices;

	// Update 3D preview
	OpenGLWnd_UpdateWindow();
};

void ListBoxWnd_InvertSelection(HWND hWnd)
{
	long Count = (long) SendMessageA(hWnd, LB_GETCOUNT, 0, 0);
	long SelCount = (long) SendMessageA(hWnd, LB_GETSELCOUNT, 0, 0);

	// Nothing in the list box
	if (!Count) return;
	
	if (SelCount == 0) ListBoxWnd_SelectAll(hWnd);
	else
		if (SelCount == Count) ListBoxWnd_SelectNone(hWnd);
		else
		{
			int *ItemsIndices = new int [SelCount];	
			SendMessageA(hWnd, LB_GETSELITEMS, (WPARAM) SelCount, (LPARAM) ItemsIndices);
			
			// Lock window update
			SendMessageA(hWnd, WM_SETREDRAW, (WPARAM) FALSE, 0);

			// Select all and ...
			SendMessageA(hWnd, LB_SETSEL, (WPARAM) TRUE, (LPARAM) -1);

			// ... deselect selected
			for (int i = SelCount; i > 0; i--) 
			{
				int Index = ItemsIndices[i - 1];
				SendMessageA(hWnd, LB_SETSEL, (WPARAM) FALSE, (LPARAM) Index);
			}

			delete [] ItemsIndices;

			// Unlock window update
			SendMessageA(hWnd, WM_SETREDRAW, (WPARAM) TRUE, 0);

			// Update selection in 3D
			ListBoxWnd_SelectionChanged(hWnd);
		}
};

void ListBoxWnd_ProcessKeys(HWND hWnd, WPARAM wParam)
{
	switch (wParam)
	{
		case VK_DELETE : { ListBoxWnd_DeleteKey(hWnd, wParam); break; }
		case 'A' : { ListBoxWnd_SelectAll(hWnd); break; }
		case 'O' : { ListBoxWnd_SelectOne(hWnd); break; }
		case 'I' : { ListBoxWnd_InvertSelection(hWnd); break; }
		case VK_SPACE : { ListBoxWnd_ToggleEnable(hWnd); break; }
	}
};

// Called by "Model params" button
void ListBoxWnd_ModelParamsButton(HWND hWnd)
{
	long Index = (long) SendMessageA(hWnd, LB_GETCARETINDEX, 0, 0);
	long Count = (long) SendMessageA(hWnd, LB_GETCOUNT, 0, 0);
	long SelCount = (long) SendMessageA(hWnd, LB_GETSELCOUNT, 0, 0);

	if ((Index < Count) && (Index >= 0) && (SelCount > 0))
	{
		LONG_PTR Data = SendMessageA(hWnd, LB_GETITEMDATA, (WPARAM) Index, 0);
		if (Data == LB_ERR) return ;
			
		ListBoxItem *Item = (ListBoxItem *) Data;
		ModelParamsWnd_SetParams(Item -> ItemModel);
	}
	else
		ModelParamsWnd_SetParams(NULL);

	if (!IsWindowVisible(hModelParamsWnd))
		ShowWindow(hModelParamsWnd, SW_SHOW);
};

void ListBoxWnd_ProcessDblClick(HWND hWnd, LPARAM lParam)
{
	POINT p = { LOWORD(lParam), HIWORD(lParam) };

	long Index = (long) SendMessageA(hWnd, LB_GETCARETINDEX, 0, 0);
	long Count = (long) SendMessageA(hWnd, LB_GETCOUNT, 0, 0);

	if ((Index < Count) && (Index >= 0))
	{
		RECT r;
		SendMessageA(hWnd, LB_GETITEMRECT, (WPARAM) Index, (LPARAM) &r);

		if (PtInRect(&r, p))
		{
			LONG_PTR Data = SendMessageA(hWnd, LB_GETITEMDATA, (WPARAM) Index, 0);
			if (Data == LB_ERR) return ;
			
			ListBoxItem *Item = (ListBoxItem *) Data;
			ModelParamsWnd_SetParams(Item -> ItemModel);

			// Show settings window
			if (!IsWindowVisible(hModelParamsWnd))
				ShowWindow(hModelParamsWnd, SW_SHOW);
		}
	}
};

// Checks whether user have clicked on the enable circle
int ListBoxWnd_EnableItemClickAccepted(HWND hWnd, LPARAM lParam)
{
	POINT p = { LOWORD(lParam), HIWORD(lParam) };

	long Index = (long) SendMessageA(hWnd, LB_GETCARETINDEX, 0, 0);
	long Count = (long) SendMessageA(hWnd, LB_GETCOUNT, 0, 0);

	if ((Index < Count) && (Index >= 0))
	{
		RECT r, rt;
		SendMessageA(hWnd, LB_GETITEMRECT, (WPARAM) Index, (LPARAM) &r);

		rt.left = r.left + 3;
		rt.top = r.top + 1;
		rt.right = rt.left + 12;
		rt.bottom = rt.top + 12;

		if (PtInRect(&rt, p)) return Index;
		else return -1;
	}

	return -1;
}

// Toggle enable particular model, linked to item with Index
void ListBoxWnd_ProcessEnabledClick(HWND hWnd, int Index)
{
	LONG_PTR Data = SendMessageA(hWnd, LB_GETITEMDATA, (WPARAM) Index, 0);
	if (Data == LB_ERR) return ;
			
	ListBoxItem *Item = (ListBoxItem *) Data;
	Item -> ItemModel -> ModelEnabled = !Item -> ItemModel -> ModelEnabled;

	RECT r;
	SendMessageA(hWnd, LB_GETITEMRECT, (WPARAM) Index, (LPARAM) &r);

	HDC hWndDC = GetDC(hWnd);
	ListBoxWnd_DrawEnabledFlag(Item -> ItemModel -> ModelEnabled, hWndDC, &r);
	ReleaseDC(hWnd, hWndDC);

	// Update 3D preview
	OpenGLWnd_UpdateWindow();
};

void ListBoxWnd_DeleteItem(LPDELETEITEMSTRUCT lpdel)
{
	ListBoxItem *Item = (ListBoxItem *) (lpdel -> itemData);
	
	if (Item)
	{
		// Delete model
		Models.DeleteModel(Item -> ItemModel -> ModelIndex);

		// Delete string
		delete [] (Item -> ItemName);

		// Delete item itself
		delete Item;
	}
};

// Select items in the 3D preview
void ListBoxWnd_SelectionChanged(HWND hWnd)
{
	long CaretIndex = (long) SendMessageA(hWnd, LB_GETCARETINDEX, 0, 0);
	long SelCount = (long) SendMessageA(hWnd, LB_GETSELCOUNT, 0, 0);
	
	if (SelCount > 0)
	{
		int *ItemsIndices = new int [SelCount];	
		SendMessageA(hWnd, LB_GETSELITEMS, (WPARAM) SelCount, (LPARAM) ItemsIndices);

		// Delta between 
		// focused and selected
		int MinDelta = INT_MAX;
		
		// Reset models selection
		ListBoxWnd_ResetModelsSelection();
		
		for (int i = SelCount; i > 0; i--) 
		{
			int Index = ItemsIndices[i - 1];

			int CurrentDelta = Index - CaretIndex;
			if (abs(CurrentDelta) < MinDelta) MinDelta = CurrentDelta;

			ListBoxItem *Item = (ListBoxItem *) SendMessageA(hWnd, LB_GETITEMDATA, (WPARAM) Index, 0);				
			Item -> ItemModel -> ModelBoxed = TRUE;
		}

		// Correct focus
		if (MinDelta != 0)
			SendMessageA(hWnd, LB_SETCARETINDEX, (WPARAM) CaretIndex + MinDelta, (LPARAM) FALSE);

		// Update params window if visible
		if (IsWindowVisible(hModelParamsWnd))
		{
			long Index = (long) SendMessageA(hWnd, LB_GETCARETINDEX, 0, 0);
			long Count = (long) SendMessageA(hWnd, LB_GETCOUNT, 0, 0);

			if ((Index < Count) && (Index >= 0))
			{
				LONG_PTR Data = SendMessageA(hWnd, LB_GETITEMDATA, (WPARAM) Index, 0);
				
				if (Data != LB_ERR)
				{
					// Cast item...
					ListBoxItem *Item = (ListBoxItem *) Data;
					ModelParamsWnd_SetParams(Item -> ItemModel);
				}
			}
		}

		// Update 3D preview
		OpenGLWnd_UpdateWindow();
	}
	else
		ModelParamsWnd_SetParams(NULL);
};

/*
	ColorIDs ColorID;	// Body, Wheel, Debris
	BYTE Reflection;	// Body, Wheel
	
	BOOL WheelSteer;	// Wheel only
	int DebrisPower;	// Debris only
	int SlotID;			// Slot only
	int SlotAngle;		// Slot only
*/

// Make name based on item's parameters
void ListBoxWnd_MakeModelName(ListBoxItem *Item)
{
	// Name of the item in listbox
	const int ListBoxNameLen = 17;
	char *ListBoxName = new char [ListBoxNameLen];
	ZeroMemory(ListBoxName, ListBoxNameLen);

	ModelParamsStruct &mps = Item -> ItemModel -> ModelParams;

	switch (Item -> ItemModel -> ModelType) 
	{

		case MODEL_BODY :
		{
			ModelMaterials mm = Item -> ItemModel -> ModelMaterial;

			_snprintf_s(ListBoxName, ListBoxNameLen, _TRUNCATE, 
				"(%d, %d, %d)", mm, mps.ColorID, mps.Reflection);
			break;
		}

		case MODEL_WHEEL :
		{
			_snprintf_s(ListBoxName, ListBoxNameLen, _TRUNCATE, 
				"(%d, %d, %d)", mps.ColorID, mps.Reflection, mps.WheelSteer);
			break;
		}

		case MODEL_DEBRIS :
		{
			_snprintf_s(ListBoxName, ListBoxNameLen, _TRUNCATE, 
				"(%d, %d)", mps.ColorID, mps.Reflection);
				
			break;
		}

		case MODEL_SLOT :
		{
			_snprintf_s(ListBoxName, ListBoxNameLen, _TRUNCATE, 
				"(%d, %d)", mps.SlotID, mps.SlotAngle);
				
			break;
		}

		case MODEL_UNKNOWN :
		{
			// Do nothing...
			break;
		}
	}

	// Delete old name
	if (Item -> ItemName) delete [] Item -> ItemName;
	
	// Create new one
	SIZE_T ListBoxNameRealLen = strlen(ListBoxName) + 1;
	Item -> ItemName = new char [ListBoxNameRealLen];
	
	// Copy to new place
	lstrcpyA(Item -> ItemName, ListBoxName);

	delete [] ListBoxName;
};

// Apply params to selected list box items
void ListBoxWnd_ApplyParams(HWND hWnd, ModelParamsWnd_Params &mpwp, BOOL KeepParams)
{
	long SelCount = (long) SendMessageA(hWnd, LB_GETSELCOUNT, 0, 0);
	if (SelCount <= 0) return; 

	int *ItemsIndices = new int [SelCount];	
	SendMessageA(hWnd, LB_GETSELITEMS, (WPARAM) SelCount, (LPARAM) ItemsIndices);

	// Lock redraw
	SendMessageA(hWnd, WM_SETREDRAW, (WPARAM) FALSE, 0);
	
	// Items that needs new place
	ListBoxItem **DeletedItems = new ListBoxItem * [SelCount];

	// Each selected item...
	for (int i = SelCount; i > 0; i--) 
	{
		int Index = ItemsIndices[i - 1];
		ListBoxItem *Item = (ListBoxItem *) SendMessageA(hWnd, LB_GETITEMDATA, (WPARAM) Index, 0);				
		
		// Is type changed? 
		// If TRUE - model needs new place, other - keep old place
		BOOL TypeChanged = (Item -> ItemModel -> ModelType != mpwp.mt);
		
		// Item's model
		Model *m = Item -> ItemModel;
		
		// Apply ColorID or Reflection to faces
		BOOL ChangeColorID = (mpwp.mp.ColorID != m -> ModelParams.ColorID);
		BOOL ChangeReflection = (mpwp.mp.Reflection != m -> ModelParams.Reflection);

		if (ChangeColorID || ChangeReflection)
			for (int j = 0; j < m -> FacesCount; j++)
			{
				Face &f = m -> Faces[j];

				if (ChangeColorID) f.ColorID = mpwp.mp.ColorID;
				if (ChangeReflection) f.Reflection = mpwp.mp.Reflection;
			}

		// Change params...
		if (!KeepParams) Item -> ItemModel -> ModelParams = mpwp.mp;
		else
		{
			m -> ModelParams.ColorID = mpwp.mp.ColorID;
			m -> ModelParams.Reflection = mpwp.mp.Reflection;
		}

		// Other params...
		Item -> ItemModel -> ModelType = mpwp.mt;
		Item -> ItemModel -> ModelMaterial = mpwp.mm;

		// Set new item's name
		ListBoxWnd_MakeModelName(Item);

		if (TypeChanged)
		{
			// Save item's data
			DeletedItems[SelCount - i] = Item;

			// Avoid item's model deletion
			SendMessageA(hWnd, LB_SETITEMDATA, (WPARAM) Index, 0); 
		
			// Delete item
			SendMessageA(hWnd, LB_DELETESTRING, (WPARAM) Index, 0);
		}
		else
			DeletedItems[SelCount - i] = NULL;
	}

	delete [] ItemsIndices;

	// Add new items inplace deleted
	for (int i = 0; i < SelCount; i++)
	{
		ListBoxItem *Item = DeletedItems[i];
		if (Item) 
		{
			int Index = (int) SendMessageA(hWnd, LB_ADDSTRING, 0, (LPARAM) Item);
			if (Index >= 0)	SendMessageA(hWnd, LB_SETSEL, (WPARAM) TRUE, Index);
		}
	}
	
	delete [] DeletedItems;

	// Redraw window
	SendMessageA(hWnd, WM_SETREDRAW, (WPARAM) TRUE, 0);
	InvalidateRect(hWnd, NULL, FALSE);

	// Update 3D preview
	OpenGLWnd_UpdateWindow();
};

void ListBoxWnd_CorrectLostFocus(HWND hWnd, LPARAM lParam)
{
	POINT p = { LOWORD(lParam), HIWORD(lParam) };
	
	long Index = (long) SendMessageA(hWnd, LB_GETCARETINDEX, 0, 0);
	long Count = (long) SendMessageA(hWnd, LB_GETCOUNT, 0, 0);

	if ((Index < Count) && (Index >= 0))
	{
		BOOL Selected = (SendMessageA(hWnd, LB_GETSEL, Index, 0) > 0);
		if (!Selected) ListBoxWnd_SelectOne(hWnd);
	}
};

void ListBoxWnd_PopupSetType(HWND hWnd, int ItemID)
{
	ModelParamsWnd_Params mpwp;

	// Yes, I need (-1) here
	mpwp.mm = (ModelMaterials) (ItemID - 1);
	
	// Default value...
	mpwp.mp.Reflection = 0;

	switch (mpwp.mm)
	{
		case MODEL_MATERIAL_BODY :
		{ 
			mpwp.mt = MODEL_BODY; 
			mpwp.mp.ColorID = COLOR_BODY; 
			break; 
		}

		case MODEL_MATERIAL_GLASS :
		{ 
			mpwp.mt = MODEL_BODY; 
			mpwp.mp.ColorID = COLOR_GLASS; 
			mpwp.mp.Reflection = 8;
			break; 
		}
		
		case MODEL_MATERIAL_METAL :
		{
			mpwp.mt = MODEL_BODY;
			mpwp.mp.ColorID = COLOR_METAL;
			mpwp.mp.Reflection = 4;
			break; 
		}
		
		case MODEL_MATERIAL_TUBE : 
		{
			mpwp.mt = MODEL_BODY;
			mpwp.mp.ColorID = COLOR_TUBE;
			mpwp.mp.Reflection = 2;
			break;
		}

		case MODEL_MATERIAL_WHEEL :
		{
			mpwp.mt = MODEL_WHEEL;
			mpwp.mp.ColorID = COLOR_WHEEL;
			break;
		}		
		
		case MODEL_MATERIAL_DEBRIS : 
		{
			mpwp.mt = MODEL_DEBRIS;
			mpwp.mp.ColorID = COLOR_BODY;
			break; 
		}
		
		case MODEL_MATERIAL_SLOT :
		{ 
			mpwp.mt = MODEL_SLOT;
			mpwp.mp.ColorID = COLOR_SLOT;
			break;
		}

		case MODEL_MATERIAL_UNKNOWN :
		{
			mpwp.mt = MODEL_UNKNOWN;
			mpwp.mp.ColorID = COLOR_RESERVED;
			break;
		}
	}	

	ListBoxWnd_ApplyParams(hWnd, mpwp, TRUE);
	
	if (IsWindowVisible(hModelParamsWnd))
	{
		// Ok, params window visible...
		long Index = (long) SendMessageA(hWnd, LB_GETCARETINDEX, 0, 0);
		long Count = (long) SendMessageA(hWnd, LB_GETCOUNT, 0, 0);

		// Do I really need these checks?
		if ((Index < Count) && (Index >= 0))
		{
			LONG_PTR Data = SendMessageA(hWnd, LB_GETITEMDATA, (WPARAM) Index, 0);
			if (Data != LB_ERR)
			{
				ListBoxItem *Item = (ListBoxItem *) Data;
	 			ModelParamsWnd_SetParams(Item -> ItemModel);
			}
		}
	}
};

void ListBoxWnd_ShowPopup(HWND hWnd, int x, int y)
{
	int SelCount = (int) SendMessage(hWnd, LB_GETSELCOUNT, 0, 0);
	int CaretIndex = (int) SendMessageA(hWnd, LB_GETCARETINDEX, 0, 0);

	if ((CaretIndex >= 0) && (SelCount > 0))
	{
		LONG_PTR Data = SendMessageA(hWnd, LB_GETITEMDATA, (WPARAM) CaretIndex, 0);
				
		if (Data != LB_ERR)
		{
			ListBoxItem *Item = (ListBoxItem *) Data;			

			// Yes, I need (+1) here
			UINT CheckID = (Item -> ItemModel -> ModelMaterial) + 1;
		
			if (CheckID != ListBoxWnd_Menu_CheckedID)
			{
				CheckMenuItem(ListBoxWnd_Menu, ListBoxWnd_Menu_CheckedID, MF_UNCHECKED | MF_BYCOMMAND);
				CheckMenuItem(ListBoxWnd_Menu, CheckID, MF_CHECKED | MF_BYCOMMAND);
	
				ListBoxWnd_Menu_CheckedID = CheckID;
			}
			
			// Show popup
			TrackPopupMenu(ListBoxWnd_Menu, TPM_LEFTBUTTON, x, y, 0, hWnd, NULL); 
		}
	}
};

// Subclassed ListBox WNDPROC...
LRESULT CALLBACK ListBoxWnd_WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
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
						case 0 : break; // Separator selected somehow...
						case MENU_HIDE_MODEL : { ListBoxWnd_ProcessKeys(hWnd, VK_SPACE); break; }
						case MENU_DELETE_MODEL : { ListBoxWnd_ProcessKeys(hWnd, VK_DELETE); break; }
						case MENU_SHOW_PARAMS : { ListBoxWnd_ModelParamsButton(hWnd); break; }
						
						// All other
						default : { ListBoxWnd_PopupSetType(hWnd, LOWORD(wParam)); break; }						
					}
				
					break;
				}
			}
		
			break;
		}

		case WM_DRAWITEM : { ListBoxWnd_DrawItem(hWnd, LPDRAWITEMSTRUCT(lParam)); break; }
		case WM_COMPAREITEM : { return ListBoxWnd_CompareItem(LPCOMPAREITEMSTRUCT(lParam)); }
		case WM_LBUTTONDOWN :
		{
			CallWindowProcA(ListBoxWnd_WndProcOrig, hWnd, uMsg, wParam, lParam);
			
			// If click below last item, then it's just focused, but not selected.
			// This function fixes this problem. This is needed only if MK_LBUTTON
			if (wParam == MK_LBUTTON) ListBoxWnd_CorrectLostFocus(hWnd, lParam);
			
			ListBoxWnd_ClickedItem = ListBoxWnd_EnableItemClickAccepted(hWnd, lParam);
			
			break;
		}

		case WM_LBUTTONUP :
		{
			CallWindowProcA(ListBoxWnd_WndProcOrig, hWnd, uMsg, wParam, lParam);

			int Index = ListBoxWnd_EnableItemClickAccepted(hWnd, lParam);

			if ((Index >= 0) && (Index == ListBoxWnd_ClickedItem))
				ListBoxWnd_ProcessEnabledClick(hWnd, Index);

			ListBoxWnd_ClickedItem = -1;

			break;
		}

		case WM_KEYDOWN : 
		{ 
			ListBoxWnd_ProcessKeys(hWnd, wParam);
		
			if ((wParam >= VK_PRIOR) && (wParam <= VK_DOWN))
				CallWindowProcA(ListBoxWnd_WndProcOrig, hWnd, uMsg, wParam, lParam);
			
			break;
		}
		
		case WM_SETFOCUS : 
		case WM_KILLFOCUS : 
		{ 
			InvalidateRect(hWnd, NULL, FALSE);
			return CallWindowProcA(ListBoxWnd_WndProcOrig, hWnd, uMsg, wParam, lParam);
		}
		
		case WM_LBUTTONDBLCLK :
		{		
			ListBoxWnd_ClickedItem = ListBoxWnd_EnableItemClickAccepted(hWnd, lParam);
			if (ListBoxWnd_ClickedItem < 0) ListBoxWnd_ProcessDblClick(hWnd, lParam);

			break;
		}
		
		case WM_CONTEXTMENU :
		{
			// Verify list box focused
			if (GetFocus() != hWnd) SetFocus(hWnd);

			int x = LOWORD(lParam);
			int y = HIWORD(lParam);

			ListBoxWnd_ShowPopup(hWnd, x, y);
			
			break;
		}

		case WM_DESTROY :
		{
			if (ListBoxWnd_Menu) DestroyMenu(ListBoxWnd_Menu);
			break;
		}

		default : return CallWindowProcA(ListBoxWnd_WndProcOrig, hWnd, uMsg, wParam, lParam);
	}
	
	return FALSE;
};

void ListBoxWnd_RemoveSubclass(HWND hWnd)
{
	if (hWnd) SetWindowLongA(hWnd, GWL_WNDPROC, (LONG) (LONG_PTR) ListBoxWnd_WndProcOrig);
};

// Hardcoding... idiotic again?
void ListBoxWnd_CreatePopup()
{
	// Submenu
	HMENU hSubMenu = CreatePopupMenu();
	if (hSubMenu)
	{
		DWORD MF_RADIOTEXT = MF_STRING | MFT_RADIOCHECK;
		AppendMenuA(hSubMenu, MF_RADIOTEXT, MENU_BODY_BODY, ListBoxWnd_SubMenuItems[0]);
		AppendMenuA(hSubMenu, MF_RADIOTEXT, MENU_BODY_GLASS, ListBoxWnd_SubMenuItems[1]);
		AppendMenuA(hSubMenu, MF_RADIOTEXT, MENU_BODY_METAL, ListBoxWnd_SubMenuItems[2]);
		AppendMenuA(hSubMenu, MF_RADIOTEXT, MENU_BODY_TUBE, ListBoxWnd_SubMenuItems[3]);
	
		// Menu
		HMENU hMenu = CreatePopupMenu();
		if (hMenu)
		{
			AppendMenuA(hMenu, MF_STRING, MENU_HIDE_MODEL, ListBoxWnd_MenuItems[0]);
			AppendMenuA(hMenu, MF_SEPARATOR, 0, NULL);
			AppendMenuA(hMenu, MF_STRING, MENU_SHOW_PARAMS, ListBoxWnd_MenuItems[7]);
			AppendMenuA(hMenu, MF_SEPARATOR, 0, NULL);
			AppendMenuA(hMenu, MF_STRING | MF_POPUP, (UINT_PTR) hSubMenu, ListBoxWnd_MenuItems[1]);
			AppendMenuA(hMenu, MF_RADIOTEXT, MENU_WHEEL, ListBoxWnd_MenuItems[2]);
			AppendMenuA(hMenu, MF_RADIOTEXT, MENU_DEBRIS, ListBoxWnd_MenuItems[3]);
			AppendMenuA(hMenu, MF_RADIOTEXT, MENU_SLOT, ListBoxWnd_MenuItems[4]);
			AppendMenuA(hMenu, MF_SEPARATOR, 0, NULL);
			AppendMenuA(hMenu, MF_RADIOTEXT | MF_CHECKED, MENU_UNKNOWN, ListBoxWnd_MenuItems[5]);
			AppendMenuA(hMenu, MF_SEPARATOR, 0, NULL);
			AppendMenuA(hMenu, MF_STRING, MENU_DELETE_MODEL, ListBoxWnd_MenuItems[6]);

			ListBoxWnd_Menu_CheckedID = MENU_UNKNOWN;
			ListBoxWnd_Menu = hMenu;
		}
		else 
			DestroyMenu(hSubMenu);
	}
};

// Hardcoding... idiotic?
HWND ListBoxWnd_CreateWnd(HINSTANCE hInstance, HWND Parent)
{
	HWND hWnd = CreateWindowExA(WS_EX_STATICEDGE, "LISTBOX", NULL,
					WS_VSCROLL | WS_CHILD | WS_VISIBLE | LBS_DISABLENOSCROLL |
					LBS_OWNERDRAWFIXED | LBS_SORT | LBS_EXTENDEDSEL | LBS_NOTIFY | WS_TABSTOP, 
					ListBoxWnd_Left, ListBoxWnd_Top, ListBoxWnd_Width, ListBoxWnd_Height,
					Parent, (HMENU) LBX_MODELPARTS, hInstance, NULL);

    // Set new WNDPROC for ListBox
    ListBoxWnd_WndProcOrig = (WNDPROC) (LONG_PTR) GetWindowLongA(hWnd, GWL_WNDPROC);
    SetWindowLongA(hWnd, GWL_WNDPROC, (LONG) (LONG_PTR) ListBoxWnd_WndProc);

	// Set item height...
	if (hWnd) 
	{
		ListBoxWnd_CreatePopup();

		SendMessageA(hWnd, WM_SETFONT, (WPARAM) hFont, 0);
		SendMessageA(hWnd, LB_SETITEMHEIGHT, 0, LPARAM(LISTBOX_ITEM_HEIGHT));
	}
	
	return hWnd;
};
