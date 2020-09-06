
#ifndef ONCE_MODELS_LIST
#define ONCE_MODELS_LIST

#include <windows.h>

#include "ModelClass.h"

// Saving status printing proc
typedef void (*PrintStatusProc) (int StageIndex, int SubstageIndex);

// Saving stages
enum SaveStages
{
	// M3D Stages
	STAGE_BODY,
	STAGE_WHEEL,
	STAGE_DEBRIS,
	STAGE_BOUND,
	STAGE_SLOT,

	// OBJ Stages
	STAGE_EXPORTING,

	// Export result
	SAVING_OK,
	SAVING_TERMINATED,
	SAVING_ERROR
};

class ModelsStore
{
private:
	HANDLE hHeap;

	Model **ModelsList;
	int ModelsListSize;

public:
	HWND ModelsListBox;
	int ModelsCount;

	ModelsStore();
	~ModelsStore();

	BOOL LoadASE(const char *FileName);
	BOOL LoadASC(const char *FileName);
	BOOL LoadM3D(const char *FileName);

	Model *AddModel();
	
	Model *GetModel(int Index); 

	void DeleteModel(int Index);
	void ClearList();

	void AddListBoxModel(Model *m);
	
	void GetTypesCount(int *TypesCount);
	
	int GetWheelsModelsOffset(Vertex &WheelsOffset);
	void GetWheelsModelsBound(Vertex &WheelsMax, Vertex &WheelsMin);

	void BuildBodyModelsList(Model **Lis, BOOL AllWheels);
	BOOL IsModelBodyPart(Model *m, BOOL AllWheels);
	int GetBodyModelsListLen(BOOL AllWheels);
	
	BOOL TerminateSave;
	BOOL SaveBinaryBody(HANDLE hFile, Vertex &OutOffset, int *TypesCount, Model *BodyBound, float &OutScale);
	BOOL SaveBinaryWheels(HANDLE hFile, Vertex &Offset, int *TypesCount, float Scale);
	BOOL SaveBinaryDebris(HANDLE hFile, float Scale);
	BOOL SaveBinarySlots(HANDLE hFile, Vertex &Offset, int *TypesCount, float Scale);
	BOOL SaveBinary(HANDLE hFile);

	BOOL SaveAscii(HANDLE hFile, BOOL OnlySelected);

	PrintStatusProc PrintStatus;
	BOOL ExportBinaryM3D(const char *FileName);
	BOOL ExportAsciiOBJ(const char *FileName, BOOL OnlySelected);

	BOOL LoadBinaryBody(HANDLE hFile, int *TypesCount);
	BOOL LoadBinaryWheels(HANDLE hFile, int WheelsCount);
	BOOL LoadBinaryDebris(HANDLE hFile, int DebrisCount);
	BOOL LoadBinarySlots(HANDLE hFile);
	BOOL LoadBinary(HANDLE hFile);
};

#endif