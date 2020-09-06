
// ----------------------------------------------------
// This is the first unit I've ever written using C\C++
// ----------------------------------------------------

#include <windows.h>
#include "AsciiImport.h"

const char SYMBOLS_DIGITS[] = "0123456789";

// Constructor
AsciiImport::AsciiImport(HANDLE hProcessHeap)
{
	hHeap = hProcessHeap;
	
	FileBuffer = NULL;
	BufferPosition = NULL;
	BufferEnd = NULL;
	String = NULL; 
};

// Destructor
AsciiImport::~AsciiImport()
{
	if (FileBuffer) 
	{
		HeapFree(hHeap, 0, (LPVOID) FileBuffer);
		FileBuffer = NULL;
	}

	if (String) 
	{
		HeapFree(hHeap, 0, (LPVOID) String);
		String = NULL;
	}
};

// Fill buffer with file data and return TRUE if all is Ok
BOOL AsciiImport::LoadBufferFromFile(const char *FileName) 
{
	BOOL BufferReady = FALSE;

	HANDLE hFile = CreateFileA(FileName, GENERIC_READ, FILE_SHARE_READ, NULL,
		OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);

	if (hFile != INVALID_HANDLE_VALUE)
	{
		DWORD FileSize = GetFileSize(hFile, NULL);
		
		// Last char in the buffer will be always '\0'
		FileBuffer = (char *) HeapAlloc(hHeap, HEAP_ZERO_MEMORY, FileSize + 1);

		// Set it here...
		BufferPosition = FileBuffer;
		BufferEnd = FileBuffer + FileSize;

		if (FileBuffer) 
		{		
			DWORD BytesRead = 0;
			ReadFile(hFile, (LPVOID) FileBuffer, FileSize, &BytesRead, NULL);
			
			if (BytesRead == FileSize) BufferReady = TRUE;
			else 
			{
				HeapFree(hHeap, 0, (LPVOID) FileBuffer);
				FileBuffer = NULL;
			}
		}
		
		CloseHandle(hFile);
	}

	return BufferReady;
};

// Finds name in the buffer. Set Start to NULL 
// if want to search from beginning of the buffer.
// BufferPosition will be set at the next char after Name
char *AsciiImport::FindName(const char *Start, const char *Name)
{
	if (!Start) Start = FileBuffer;

	char *NamePosition = (char *) strstr(Start, Name);
	if (NamePosition) BufferPosition = NamePosition + strlen(Name);

	return NamePosition;
};

BOOL AsciiImport::IsNumeric(BYTE Value)
{
	BOOL Found = (strchr(SYMBOLS_DIGITS, Value) != NULL);
	return Found;
};

// Some stupid code...
char *AsciiImport::GetNumberEnd(const char *Start)
{
	BOOL MinusPlus = (*Start == '-') || (*Start == '+'); 
	if (MinusPlus) Start++;

	BOOL Numeric = IsNumeric((BYTE) *Start);
	BOOL Exponent = (*Start == 'e');
	BOOL Comma = (*Start == '.');
	
	if (Comma || Exponent) 
	{
		Numeric = TRUE;
		Start++;
	}

	while (Numeric) 
	{
		Start++;
		Numeric = IsNumeric((BYTE) *Start);

		if ((*Start == '.') && (!Comma))
		{
			Comma = TRUE;
			Numeric = TRUE;
		}

		if ((*Start == 'e') && (!Exponent))
		{
			Exponent = TRUE;
			Numeric = TRUE;

			const char *StartNext = Start + 1;
			if ((*StartNext == '-') || (*StartNext == '+')) 
				Start++;
		}

	}

	return (char *) Start;
};

// Checks whether char is whitespace
BOOL AsciiImport::IsWhitespace(BYTE Value)
{
	BOOL Result =	(Value == 0x20) || (Value == 0x09) ||
					(Value == 0x0A) || (Value == 0x0D);
	
	return Result;
};

// Actually just copy some chars...
char *AsciiImport::GetString(const char *Start)
{
	// Search beginning of the string
	BOOL Skip = IsWhitespace((BYTE) *Start);

    // Actually don't needed here: (Start < BufferEnd), but...
	while (Skip && (Start < BufferEnd))
	{
		Start++;
		Skip = IsWhitespace((BYTE) *Start);
	}
	
	BOOL Flag = (*Start == '"');
	if (Flag && (Start < BufferEnd)) Start++;

	BOOL Found;
	long Length = 0;
	const char *End = Start;	
	
	// Calculate length of the string
	if (Flag) Found = (*End == '"');
	else Found = IsWhitespace((BYTE) *End);

	while ((!Found) && (End < BufferEnd))
	{		
		End++;
		Length++;

		if (Flag) Found = (*End == '"');
		else Found = IsWhitespace((BYTE) *End);
	}
	
	if (Flag && (End < BufferEnd)) End++;
	BufferPosition = (char *) End;

	// Free memory of old string
	if (String) 
	{
		HeapFree(hHeap, 0, (LPVOID) String);
		String = NULL;
	}

	// Copy string...
	if (Length)
	{
		String = (char *) HeapAlloc(hHeap, HEAP_ZERO_MEMORY, Length + 1);
		if (String)	CopyMemory(String, Start, Length);
	} 
	
	return String;
};

// Returns a double value from Start position.
// Whitespace characters from beginning will be skipped.
// BufferPosition will be set after last valid number char
double AsciiImport::GetDouble(const char *Start)
{	
	while (IsWhitespace((BYTE) *Start)) Start++;
	double Value = atof(Start);
	
	BufferPosition = GetNumberEnd(Start);
	
	return Value;
};

