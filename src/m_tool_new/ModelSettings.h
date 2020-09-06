
#ifndef DEF_MODEL_SETTINGS
#define DEF_MODEL_SETTINGS

#include <windows.h>

// Params of params window
struct ModelParamsWnd_Params
{
	struct ModelParamsStruct mp;
	ModelMaterials mm;
	ModelTypes mt;
};

void ModelParamsWnd_SetParams(Model *m);
BOOL CreateModelParamsWindow(HINSTANCE hInstance, HWND Parent);

#endif