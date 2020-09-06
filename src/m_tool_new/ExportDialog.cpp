
#include <windows.h>
#include <stdio.h>

#include "Globals.h"
#include "ExportDialog.h"
#include "OpenSaveModule.h"
#include "OpenGLWindow.h"
#include "ModelsList.h"

HINSTANCE ExportDialog_hInstance = 0;
HWND ExportDialog_hStatusStatic = 0;
HANDLE ExportDialog_ExportThread = 0;

// What to save?
ExportDialog_Type ExportDialog_SavingType;

// Pure export procedure
// ExportDialog_ExportProc ExportDialog_Export = NULL;

// Seems like stupid code... And it is.
void ExportDialog_Status_MDL(int StageIndex, int SubstageIndex)
{
	char *Text = NULL;

	switch (StageIndex)
	{
		case STAGE_BODY : 
		{ 
			Text = " saving body model...";
			break; 
		}

		case STAGE_WHEEL :
		{
			char *Buffer = new char [25];
			
			_snprintf_s(Buffer, 25, _TRUNCATE, " saving wheel (%d)...", SubstageIndex);
			SetWindowText(ExportDialog_hStatusStatic, Buffer);

			delete [] Buffer;
			break;
		}

		case STAGE_DEBRIS :
		{
			char *Buffer = new char [25];

			_snprintf_s(Buffer, 25, _TRUNCATE, " saving debris (%d)...", SubstageIndex);
			SetWindowText(ExportDialog_hStatusStatic, Buffer);

			delete [] Buffer;
			break;
		}

		case STAGE_BOUND :
		{
			Text = " saving body bound...";
			break;
		}

		case STAGE_SLOT :
		{
			Text = " saving slots...";
			break;
		}
	}

	// Show status string
	if (Text) SetWindowTextA(ExportDialog_hStatusStatic, Text);
};

// Seems like stupid code... And it is.
void ExportDialog_Status_OBJ(int StageIndex, int SubstageIndex)
{
	switch (StageIndex)
	{
		case STAGE_EXPORTING : 
		{ 
			char *Buffer = new char [25];

			_snprintf_s(Buffer, 25, _TRUNCATE, " saving model (%d)...", SubstageIndex);
			SetWindowText(ExportDialog_hStatusStatic, Buffer);

			delete [] Buffer;
			break;
		}
	}
};

// This actually exports model
DWORD WINAPI ExportDialog_PureExportMechous(LPVOID lpParameter)
{

	BOOL Result = FALSE;
	
	// UI limitations...
	OpenGLWnd_Pause = TRUE;
	BOOL ApplyModelEnabled = IsWindowEnabled(hModelParamsApplyBtn);
	if (ApplyModelEnabled) EnableWindow(hModelParamsApplyBtn, FALSE);

	EnableWindow(hMechousParamsOkBtn, FALSE);

	// MAIN EXPORTING FUNCTION
	switch (ExportDialog_SavingType)
	{
		case SAVING_TYPE_MDL : 
		{
			Models.PrintStatus = ExportDialog_Status_MDL;
			Result = Models.ExportBinaryM3D(FileName_Save_MDL);
			break;
		}

		case SAVING_TYPE_OBJ_SEL :
		case SAVING_TYPE_OBJ_ALL :
		{
			// 'Export only selected' flag
			BOOL OnlySelected = ExportDialog_SavingType == SAVING_TYPE_OBJ_SEL;

			// Assing printing status proc
			Models.PrintStatus = ExportDialog_Status_OBJ;
			Result = Models.ExportAsciiOBJ(FileName_Save_MDL, OnlySelected);

			break;
		}
	}

	OpenGLWnd_Pause = FALSE;
	EnableWindowCheck(hModelParamsApplyBtn, ApplyModelEnabled);
	EnableWindow(hMechousParamsOkBtn, TRUE);

	// Say to the dialog box, that exporting finished or terminated
	PostMessageA((HWND) lpParameter, WM_COMMAND, (BN_CLICKED << 16) | IDABORT, 0);

	return Result;
};

// Print result of saving process
void ExportDialog_Status_Saving(int StageIndex)
{
	char *Text = NULL;

	switch (StageIndex)
	{
		case SAVING_OK :
		{
			Text = " successfully exported!";
			break;
		}

		case SAVING_TERMINATED :
		{
			Text = " export terminated!";
			break;
		}

		case SAVING_ERROR :
		{
			Text = " some error occured!";
			break;
		}
	}

	// Show status string
	if (Text) SetWindowTextA(ExportDialog_hStatusStatic, Text);
};

INT_PTR CALLBACK ExportDlgDlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
		case WM_INITDIALOG :
		{			
			// Handle of the static for status string
			ExportDialog_hStatusStatic = GetDlgItem(hwndDlg, 13);

			// Start export!
			ExportDialog_ExportThread = CreateThread(NULL, 0, 
				(LPTHREAD_START_ROUTINE) &ExportDialog_PureExportMechous,
				(LPVOID) hwndDlg, 0, NULL);

			// Handle of the Terminate button
			wParam = (WPARAM) GetDlgItem(hwndDlg, 2);

			return TRUE;
		}

		case WM_COMMAND :
		{
			switch HIWORD(wParam)
			{
				case BN_CLICKED :
				{ 
					WORD ControlID = LOWORD(wParam);
					
					switch (ControlID)
					{
						case IDCANCEL :
						{
							Models.TerminateSave = TRUE;
							HWND hButton = GetDlgItem(hwndDlg, IDCANCEL);
							SetWindowText(hButton, "Terminating...");
							break;
						}
					
						case IDABORT : 
						{
							HWND hButton = GetDlgItem(hwndDlg, IDCANCEL);

							// Wait for ending of the thread. This locks current 
							// thread and prevents closing handle of active thread
							WaitForSingleObject(ExportDialog_ExportThread, INFINITE);

							if (Models.TerminateSave)
							{
								SetWindowText(hButton, "Terminated");
								ExportDialog_Status_Saving(SAVING_TERMINATED);
							}
							else
							{
								DWORD ExitCode = 0;
								GetExitCodeThread(ExportDialog_ExportThread, &ExitCode);

								if (ExitCode)
									ExportDialog_Status_Saving(SAVING_OK);
								else
									ExportDialog_Status_Saving(SAVING_ERROR);
							}

							// Free export thread's handle
							CloseHandle(ExportDialog_ExportThread);

							// Some stuff...
							EnableWindow(hButton, FALSE);

							hButton = GetDlgItem(hwndDlg, IDOK);
							EnableWindow(hButton, TRUE);
							SetFocus(hButton);

							break;
						}
							
						case IDOK :
						{
							EndDialog(hwndDlg, ControlID);
							break;
						}
					}

					break;
				}
			}
		
			break;
		}		

		case WM_CLOSE :
		{
			SendMessageA(hwndDlg, WM_COMMAND, (BN_CLICKED << 16) | IDCANCEL, 0);
			return TRUE;
		}
	}
	
	return FALSE; 
};

// Start saving/exporting
void ExportDialog_Show(HWND Parent, ExportDialog_Type SavingType)
{
	// Assign saving type
	ExportDialog_SavingType = SavingType;
	
	// Start export!
	DialogBoxA(ExportDialog_hInstance, "ExportDialog", Parent, &ExportDlgDlgProc);
};

