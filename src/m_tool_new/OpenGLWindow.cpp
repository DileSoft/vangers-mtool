#include <windows.h>
#include <Gl.h>
#include <Glu.h>
#include <math.h>

#include "Globals.h"
#include "OpenGLWindow.h"

#define OPENGL_TIMER_ID			1
#define OPENGL_SPIN_DELTA		0.5f
#define OPENGL_INTERVAL			30
#define OPENGL_SLOPE_GRANULAR	0.5f

const char OpenGLWnd_ClassName[] = "TVangersMechousOpenGLClassName";

const int OpenGLWnd_Left = 15;	// extern
const int OpenGLWnd_Top = 10;	// extern

// This locks drawing of the models.
// Set to TRUE when loading new one
BOOL OpenGLWnd_Pause = FALSE;	// extern

int OpenGLWnd_SelectedModelIndex = -1; // extern

int OpenGLWnd_Width = 250;
int OpenGLWnd_Height = 250;

HDC OpenGLWnd_DC = 0;
HGLRC OpenGLWnd_RC = 0;

// State variables. All to class?
BOOL OpenGLWnd_Ready = FALSE;
BOOL OpenGLWnd_Dragging = FALSE;
BOOL OpenGLWnd_TimerActive = FALSE;

// Drawing variabled
BOOL OpenGLWnd_DrawModelVerticesNormals = FALSE;
BOOL OpenGLWnd_DrawModelFacesNormals = FALSE;
BOOL OpenGLWnd_DrawModelFacesColors = FALSE;
BOOL OpenGLWnd_DrawModelLattice = FALSE;

// Scale of view
float OpenGLWnd_Scale = 1.0f;
float OpenGLWnd_Pos[3] = {0.0, 0.0, -500.0};
float OpenGLWnd_SlopeAngle = -45.0f;
float OpenGLWnd_Spin = 180.0f;
float OpenGLWnd_ClearLevel = 0.6f;

POINT OpenGLWnd_ClickPoint = {0, 0};
POINT OpenGLWnd_DragPos = {0, 0};

// Predifined geometry colors
GLfloat OpenGLWnd_Color_Box[3]				= {1.00f, 1.00f, 1.00f};
GLfloat OpenGLWnd_Color_FacesNormals[3]		= {1.00f, 1.00f, 0.25f};
GLfloat OpenGLWnd_Color_VerticesNormals[3]	= {1.00f, 0.50f, 0.25f};
GLfloat OpenGLWnd_Color_Lattice[3]			= {0.25f, 1.00f, 1.00f};

// Core drawing function
void OpenGLWnd_DrawModels()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();

	// Axis correction...
	glTranslatef(OpenGLWnd_Pos[0], OpenGLWnd_Pos[1], OpenGLWnd_Pos[2]);
	glScalef(OpenGLWnd_Scale, OpenGLWnd_Scale, OpenGLWnd_Scale);
	glRotatef(OpenGLWnd_SlopeAngle, 1.0, 0.0, 0.0);
	
	// Auto-rotation
	glRotatef(OpenGLWnd_Spin, 0.0, 0.0, 1.0);

	// Draw models & boxes
	for (int i = 0; i < Models.ModelsCount; i++)
	{
		Model *m = Models.GetModel(i);
		if (m -> ModelEnabled) 
			m -> DrawModel(OpenGLWnd_DrawModelFacesColors);

		if (m -> ModelBoxed) 
		{
			glDisable(GL_LIGHTING);

			glColor3fv(OpenGLWnd_Color_Box);
			m -> DrawModelBox();

			if (OpenGLWnd_DrawModelFacesNormals)
			{
				glColor3fv(OpenGLWnd_Color_FacesNormals);
				m -> DrawModelFacesNormals(OpenGLWnd_Scale);
			}

			if (OpenGLWnd_DrawModelVerticesNormals)
			{
				glColor3fv(OpenGLWnd_Color_VerticesNormals);
				m -> DrawModelVerticesNormals(OpenGLWnd_Scale);
			}

			if (OpenGLWnd_DrawModelLattice)
			{
				glColor3fv(OpenGLWnd_Color_Lattice);
				m -> DrawModelLattice();
				
				/* 
				// Alternative...
				glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
				m -> DrawModel(OpenGLWnd_DrawModelFacesColors);
				glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
				*/
			}

			glEnable(GL_LIGHTING);
		}
	}
	
	glFlush();
};

void OpenGLWnd_UpdateWindow()
{
	if (!OpenGLWnd_Ready) return ;

	if (!OpenGLWnd_TimerActive)
	{
		OpenGLWnd_DrawModels();
		SwapBuffers(OpenGLWnd_DC);
	}
};

void OpenGLWnd_FixedPerspective(GLdouble fovy, GLdouble aspect, 
								GLdouble zNear, GLdouble zFar)
{
    // left = -right
	// bottom = -top
	// tg(angley/2) = top/near
	// aspect = right/top.

	GLdouble fovy2 = fovy * Pi_10 / 180;

	GLdouble Top = 0; 
	GLdouble Right = 0;
		
	// AIM OF THE FIX
	if (aspect > 1.0) 
	{
		Top = zNear * tan(fovy2 / 2); 
		Right = Top * aspect;
	}
	else
	{ 
		Right = zNear * tan(fovy2 / 2); 
		Top = Right / aspect;
	}

	GLdouble Left = -Right;
	GLdouble Bottom = -Top;

	glFrustum(Left, Right, Bottom, Top, zNear, zFar);
};


void OpenGLWnd_SetProjection(int Width, int Height)
{
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	
	OpenGLWnd_FixedPerspective(60.0, (GLdouble) Width / Height, 0.5, 1000.0);

	glMatrixMode(GL_MODELVIEW);
};

void OpenGLWnd_SetWindowSize(int Width, int Height)
{
	OpenGLWnd_Width = Width;
	OpenGLWnd_Height = Height;
	
	glViewport(0, 0, Width, Height);
	OpenGLWnd_SetProjection(Width, Height);
	
	if (OpenGLWnd_Ready)
	{
		OpenGLWnd_DrawModels();
		SwapBuffers(OpenGLWnd_DC);
	}
};

void OpenGLWnd_InitDrawing()
{
	glClearColor(OpenGLWnd_ClearLevel, 
		OpenGLWnd_ClearLevel, 
		OpenGLWnd_ClearLevel, 
		1.0);
	
	glLineWidth(1.0);
	glPointSize(2.0);

	GLfloat LightPos[3];
	LightPos[0] = 0.0;
	LightPos[1] = 0.0;
	LightPos[2] = 0.0;

	glLightf(GL_LIGHT0, GL_POSITION, LightPos[0]);
 
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);

	glShadeModel(GL_FLAT);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);

	glEnable(GL_NORMALIZE);
};

BOOL OpenGLWnd_InitOpenGL(HDC hWndDC)
{
	PIXELFORMATDESCRIPTOR pfd;
	GLuint PixelFormat;

	ZeroMemory(&pfd, sizeof(PIXELFORMATDESCRIPTOR));

	pfd.nSize		 = sizeof(PIXELFORMATDESCRIPTOR);
	pfd.nVersion	 = 1;
	pfd.dwFlags		 = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
	pfd.iPixelType	 = PFD_TYPE_RGBA;
	pfd.cColorBits	 = 32;
	pfd.cDepthBits	 = 16;
	pfd.iLayerType	 = PFD_MAIN_PLANE;

	PixelFormat = ChoosePixelFormat(hWndDC, &pfd);

	if (!PixelFormat) return FALSE;

	if (!SetPixelFormat(hWndDC, PixelFormat, &pfd)) return FALSE;

	OpenGLWnd_RC = wglCreateContext(hWndDC);

	if (!OpenGLWnd_RC) return FALSE;

	if (!wglMakeCurrent(hWndDC, OpenGLWnd_RC))
	{
		wglDeleteContext(OpenGLWnd_RC);
		OpenGLWnd_RC = 0;

		return FALSE;
	}

	return TRUE;
};

void OpenGLWnd_FreeOpenGL()
{
	if (OpenGLWnd_Ready)
	{
		wglMakeCurrent(OpenGLWnd_DC, 0);
		wglDeleteContext(OpenGLWnd_RC);
	}
};

void OpenGLWnd_ZoomWindow(BOOL ZoomIn)
{
	float Scale = 0.95f;
	if (ZoomIn) Scale = 1.05f;

	// New scale
	OpenGLWnd_Scale *= Scale; 

	// Update 3D preview
    OpenGLWnd_UpdateWindow();
};

// Not true, but simple drag...
void OpenGLWnd_DragView(short X, short Y)
{
	float dx = (float) X - OpenGLWnd_DragPos.x;
	float dy = (float) OpenGLWnd_DragPos.y - Y;

	float deltax = (float) (350 * dx / OpenGLWnd_Width);
	float deltay = (float) (350 * dy / OpenGLWnd_Height);

	OpenGLWnd_Pos[0] += deltax;
	OpenGLWnd_Pos[1] += deltay; 
	
	// Update 3D preview
    OpenGLWnd_UpdateWindow();
};

// Just slope view. 
// Simple, but it's not a 3D editor
void OpenGLWnd_SlopeView(short X, short Y)
{
	float dy = (float) OpenGLWnd_DragPos.y - Y;
	float deltay = (float) (350 * dy / OpenGLWnd_Height);

	OpenGLWnd_SlopeAngle -= deltay * OPENGL_SLOPE_GRANULAR;
	if (OpenGLWnd_SlopeAngle < 0.0) OpenGLWnd_SlopeAngle += 360.0;

	// Update 3D preview
    OpenGLWnd_UpdateWindow();
};

// On/off timer, i.e. start/stop rotation
void OpenGLWnd_EnableTimer(BOOL Enable)
{
	if (Enable != OpenGLWnd_TimerActive)
	{
		if (Enable) 
			SetTimer(hOpenGLWnd, OPENGL_TIMER_ID, OPENGL_INTERVAL, NULL);
		else
			KillTimer(hOpenGLWnd, OPENGL_TIMER_ID);

		OpenGLWnd_TimerActive = Enable;
	}
};

// Core redrawing event
void OpenGLWnd_TimerEvent()
{
	OpenGLWnd_Spin += OPENGL_SPIN_DELTA;

	if (OpenGLWnd_Spin > 360.0)	
		OpenGLWnd_Spin -= 360.0;

	// Update 3D preview
	OpenGLWnd_DrawModels();
	SwapBuffers(OpenGLWnd_DC);
};

// Simple?
void OpenGLWnd_ToggleState(BOOL &State, HWND hStatic, char *Symbol)
{
	State = !State;

	if (State)
		SetWindowText(hStatic, Symbol);
	else
		SetWindowText(hStatic, "\226");
};

void OpenGLWnd_ProcessKeys(HWND hWnd, WPARAM wParam)
{
	// Do not process keys if locked
	if (OpenGLWnd_Pause) return;

	// Process only if anything selected in ListBox
	// int SelCount = (int) SendMessageA(hListBoxWnd, LB_GETSELCOUNT, 0, 0);
	// if (!SelCount && (wParam != VK_SPACE || wParam != 'C')) return;

	switch (wParam)
	{
		case 'L' : { OpenGLWnd_ToggleState(OpenGLWnd_DrawModelLattice, hOpenGLWnd_Lattice, "L"); break; }
		case 'F' : { OpenGLWnd_ToggleState(OpenGLWnd_DrawModelFacesNormals, hOpenGLWnd_FacesNormals, "F"); break; }
		case 'V' : { OpenGLWnd_ToggleState(OpenGLWnd_DrawModelVerticesNormals, hOpenGLWnd_VerticesNormals, "V"); break; }
		case 'C' : { OpenGLWnd_ToggleState(OpenGLWnd_DrawModelFacesColors, hOpenGLWnd_FacesColors, "C"); break; }
		case VK_SPACE : { OpenGLWnd_EnableTimer(!OpenGLWnd_TimerActive); break; }
	}

	if (!OpenGLWnd_TimerActive)
		OpenGLWnd_UpdateWindow();
};

// Adjust clear color from white to black
void OpenGLWnd_AdjustClearColor(BOOL Direction)
{
	if (Direction)
		OpenGLWnd_ClearLevel -= 0.05f;
	else
		OpenGLWnd_ClearLevel += 0.05f;

	if (OpenGLWnd_ClearLevel > 1.0f) OpenGLWnd_ClearLevel = 1.0f;
	if (OpenGLWnd_ClearLevel < 0.0f) OpenGLWnd_ClearLevel = 0.0f;

	glClearColor(OpenGLWnd_ClearLevel, 
		OpenGLWnd_ClearLevel,
		OpenGLWnd_ClearLevel,
		1.0);
};

LRESULT CALLBACK OpenGLWndWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	
	switch (uMsg)
	{
		case WM_ERASEBKGND : { return TRUE; }
		case WM_PAINT :
		{
			PAINTSTRUCT ps;
			BeginPaint(hWnd, &ps);
			
			if (OpenGLWnd_Ready) 
				SwapBuffers(ps.hdc);

			EndPaint(hWnd, &ps);
			break;
		}
		
		case WM_TIMER :
		{
			if (OpenGLWnd_Ready && !OpenGLWnd_Pause) OpenGLWnd_TimerEvent();
			break;
		}

		case WM_MOUSEMOVE :
		{
			if (OpenGLWnd_Ready && OpenGLWnd_Dragging)
			{	
				short X = LOWORD(lParam);
				short Y = HIWORD(lParam);
				
				if (wParam & MK_LBUTTON)
					if (wParam & MK_CONTROL)
						OpenGLWnd_DragView(X, Y);
					else 
						OpenGLWnd_SlopeView(X, Y);
			
				OpenGLWnd_DragPos.x = X;
				OpenGLWnd_DragPos.y = Y;
			}

			break;
		}

		case WM_SIZE :
		{
			if (OpenGLWnd_Ready) 
				OpenGLWnd_SetWindowSize(LOWORD(lParam), HIWORD(lParam));

			break;
		}
		
		case WM_MOUSEWHEEL :
		{
			if (OpenGLWnd_Ready)
			{
				BOOL Direction = (short) HIWORD(wParam) < 0;
				
				if (LOWORD(wParam) == MK_CONTROL)
					OpenGLWnd_AdjustClearColor(Direction);
				else
					OpenGLWnd_ZoomWindow(Direction);
			}

			break;
		}
		
		case WM_LBUTTONDOWN :
		{
			SetFocus(hWnd);
			SetCapture(hWnd);
			
			if (wParam == (MK_SHIFT | MK_LBUTTON))
				OpenGLWnd_EnableTimer(!OpenGLWnd_TimerActive);
			else
			{
				OpenGLWnd_ClickPoint.x = LOWORD(lParam);
				OpenGLWnd_ClickPoint.y = HIWORD(lParam);
			
				OpenGLWnd_DragPos.x = OpenGLWnd_ClickPoint.x;
				OpenGLWnd_DragPos.y = OpenGLWnd_ClickPoint.y;

				OpenGLWnd_Dragging = TRUE;
			}

			break;
		}

		case WM_KEYDOWN : 
		{ 
			OpenGLWnd_ProcessKeys(hWnd, wParam);
			break;
		}

		case WM_RBUTTONDOWN :
		{
			SetFocus(hWnd);
			SetCapture(hWnd);
			
			OpenGLWnd_Spin = 180.0f;
			OpenGLWnd_Scale = 1.0f;
			OpenGLWnd_SlopeAngle = -45.0f;
			OpenGLWnd_Pos[0] = 0.0f;
			OpenGLWnd_Pos[1] = 0.0f;
			OpenGLWnd_Pos[2] = -500.0f;
			
			// Update 3D preview
			OpenGLWnd_UpdateWindow();

			break;
		}

		case WM_RBUTTONUP :
		{
			ReleaseCapture();
			break;
		}

		case WM_KILLFOCUS :
		case WM_LBUTTONUP :
		{
			ReleaseCapture();
			OpenGLWnd_Dragging = FALSE;

			break;
		}
		
		case WM_LBUTTONDBLCLK :
		{
			HWND hWndParent = GetParent(hWnd);

			if (IsZoomed(hWndParent))
				ShowWindow(hWndParent, SW_RESTORE);
			else
				ShowWindow(hWndParent, SW_MAXIMIZE);

			break;
		}
			
		case WM_CREATE :
		{
			OpenGLWnd_DC = GetDC(hWnd);
			break;
		}

		case WM_DESTROY :
		{
			if (OpenGLWnd_Ready) OpenGLWnd_FreeOpenGL();
			ReleaseDC(hWnd, OpenGLWnd_DC);
			
			OpenGLWnd_EnableTimer(FALSE);
			
			break;
		}

		default : return DefWindowProc(hWnd, uMsg, wParam, lParam);
	}
	
	return FALSE;
};

HWND OpenGLWnd_CreateWnd(HINSTANCE hInstance, HWND Parent)
{
	WNDCLASSEXA ws;

	ZeroMemory(&ws, sizeof(WNDCLASSEXA));

	ws.cbSize = sizeof(WNDCLASSEXA);
	ws.hbrBackground = 0; 
	ws.hCursor = LoadCursor(0, IDC_ARROW);
	ws.hInstance = hInstance;
	ws.lpfnWndProc = &OpenGLWndWndProc;
	ws.lpszClassName = OpenGLWnd_ClassName;
	ws.style = CS_OWNDC | CS_DBLCLKS;

	if (!RegisterClassExA(&ws)) return 0;
	
	HWND hWnd = CreateWindowExA(WS_EX_STATICEDGE, 
					OpenGLWnd_ClassName, NULL,
					WS_CHILD | WS_VISIBLE,
					OpenGLWnd_Left, OpenGLWnd_Top, OpenGLWnd_Width, OpenGLWnd_Height,
					Parent, 0, hInstance, NULL);

	if (!hWnd) UnregisterClassA(OpenGLWnd_ClassName, hInstance);
	
	if (hWnd)
	{
		OpenGLWnd_Ready = OpenGLWnd_InitOpenGL(OpenGLWnd_DC);
		if (OpenGLWnd_Ready) OpenGLWnd_InitDrawing();
	}

	return hWnd;
};