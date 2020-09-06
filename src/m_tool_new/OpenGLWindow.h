#include <windows.h>

extern int OpenGLWnd_SelectedModelIndex;

extern const int OpenGLWnd_Left;
extern const int OpenGLWnd_Top;

extern BOOL OpenGLWnd_Pause;

HWND OpenGLWnd_CreateWnd(HINSTANCE hInstance, HWND Parent);
void OpenGLWnd_EnableTimer(BOOL Enable);
void OpenGLWnd_UpdateWindow();

