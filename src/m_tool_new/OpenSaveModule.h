
// File names
extern char *FileName_Open_ASC;
extern char *FileName_Save_MDL;

// Filters indices in SaveDialog
#define SAVE_FILTER_MDL_INDEX		1
#define SAVE_FILTER_OBJ_INDEX_SEL	2
#define SAVE_FILTER_OBJ_INDEX_ALL	3

// Last used OpenFileInfo 
extern OPENFILENAMEA OpenFileInfo;

void InitOpenFileInfo(HINSTANCE hInstance);
void FreeOpenFileInfo();

BOOL GetOpenFileName_ASC(char *FileName);
BOOL GetSaveFileName_MDL(char *FileName);
