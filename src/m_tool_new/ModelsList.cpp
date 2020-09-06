#include <windows.h>
#include <float.h>
#include <stdio.h>

#include "Globals.h"
#include "ModelClass.h"
#include "AsciiImport.h"
#include "ModelsList.h"
#include "ListBoxWindow.h"
#include "MechousSettings.h"

#define MAX_SLOTS	3
#define PI_SLOTS	2048
#define BOUND_SEGMENTS	2

// Debug defines
// #define SHOW_DEBUG_MODELS

// Set structs align
#pragma pack(push, 1)

// Mechous params
struct MechousSavingParamsStruct
{
	// Defined - fully
	int xMax, yMax, zMax;
	int rMax;
	
	int WheelsCount, DebrisCount;
	int BodyColorOffset, BodyColorShift;
};

struct WheelSavingParamsStruct
{
	// Defined - fully
	int Steer;
	DBV WheelVector;

	int Width, Radius;
	int BoundIndex; // Not used?
};

struct SlotSavingParamsStruct
{
	// Defined - fully
	Vector SlotVector;
	int SlotAngle;
};

// Restore structs align
#pragma pack(pop)


ModelsStore::ModelsStore()
{
	hHeap = GetProcessHeap();

	ModelsCount = 0;
	ModelsListBox = 0;
	
	ModelsListSize = 1;
	ModelsList = (Model **) HeapAlloc(hHeap, 0, sizeof(Model *));

	TerminateSave = FALSE;
	PrintStatus = NULL;
};

ModelsStore::~ModelsStore()
{
	if (ModelsList)
	{		
		// Automatically calls destructor
		for (int i = 0; i < ModelsCount; i++)
			delete ModelsList[i];

		HeapFree(hHeap, 0, (LPVOID) ModelsList);
		ModelsList = NULL;
	}
};


// Returns model from the list by its index
Model *ModelsStore::GetModel(int Index)
{
	/*
	if (Index >= 0) && (Index < ModelsCount)
		return ModelsList[Index];
	else 
		return NULL; 
	*/

	// Speedup?..
	return ModelsList[Index];
};

void ModelsStore::DeleteModel(int Index)
{
	// Exit if no items...
	if (!ModelsCount) return;

	delete ModelsList[Index];

	for (int i = Index; i < ModelsCount - 1; i++)
	{
		ModelsList[i] = ModelsList[i + 1];
		ModelsList[i] -> ModelIndex = i;
	}
	
	ModelsCount--;

	// HeapReAlloc() here? That's the question!
	// No, for a while...
};

void ModelsStore::ClearList()
{
	for (int i = 0; i < ModelsCount; i++)
		delete ModelsList[i];

	ModelsCount = 0;
	ModelsListSize = 1;

	// Free-and-Alloc!
	// Don't want HeapReAlloc() here
	HeapFree(hHeap, 0, (LPVOID) ModelsList);
	ModelsList = (Model **) HeapAlloc(hHeap, 0, sizeof(Model *));
};

// Adds new model to the end of list.
// If list is too small, then allocates some 
// additional memory for number of other models
Model *ModelsStore::AddModel()
{
	if (ModelsCount + 1 > ModelsListSize)
	{	
		// Growing list delta
		const int ModelsDelta = 4;

		// New size of the list
		long New_ModelsListSize = ModelsCount + ModelsDelta;
	
		// Reallocate some memory
		Model **New_ModelsList = (Model **) HeapReAlloc(hHeap, 0, (LPVOID) ModelsList, 
											sizeof(Model *) * New_ModelsListSize);

		// Check result
		if (New_ModelsList)
		{
			ModelsList = New_ModelsList;
			ModelsListSize = New_ModelsListSize;
		}
		else 
			return NULL;
	}

	// Ok
	ModelsCount++;		
	ModelsList[ModelsCount - 1] = new Model;
	ModelsList[ModelsCount - 1] -> ModelIndex = ModelsCount - 1;

	return ModelsList[ModelsCount - 1];
};

void ModelsStore::AddListBoxModel(Model *m)
{
	if (ModelsListBox)
	{
		// New item
		ListBoxItem *Item = new ListBoxItem;

		// Set item model
		Item -> ItemModel = m;
		Item -> ItemName = NULL;

		// Set item name
		ListBoxWnd_MakeModelName(Item);
		SendMessageA(ModelsListBox, LB_ADDSTRING, 0, (LPARAM) Item);
	}
};

// Something like try-throw-catch...
// I can use { return FALSE; } but...
#define TRY(op) { if ((op##) == NULL) { ErrorOccured = TRUE; goto SomeError; } }
#define CHECK(op) { if ((op##) == FALSE) { ErrorOccured = TRUE; goto SomeError; } }
#define CHECK_VERTEX_INDEX(op) { if ((op##) >= (UINT) m -> VerticesCount) { ErrorOccured = TRUE; goto SomeError; } }
#define CHECK_FACE_INDEX(op) { if ((op##) >= m -> FacesCount) { ErrorOccured = TRUE; goto SomeError; } }

// Fills Vertex, Faces and Normals arrays with file data.
// Set model's name provided in the file by *NODE_NAME.
// Returns FALSE if something will go wrong...
BOOL ModelsStore::LoadASE(const char *FileName)
{
	BOOL ErrorOccured = FALSE;

	// Parser...
	AsciiImport Import(hHeap);
	
	// Exit if file not loaded or some error occured...
	if (!Import.LoadBufferFromFile(FileName)) return FALSE;

	// Is here any objects?
	BOOL ObjectFound = (Import.FindName(NULL, NAME_GEOMOBJECT) != NULL);

	while (ObjectFound)
	{
		// Add new model to be filled with data
		Model *m = AddModel();
		
		// Exit if no model added
		if (!m) return FALSE; 

		TRY(Import.FindName(Import.BufferPosition, NAME_NODE_NAME));
		char *Name = Import.GetString(Import.BufferPosition);
		TRY(Name); // Exit if no name found or not enougth memory...
		
		// Set model name and autoparams
		m -> SetModelName(Name, TRUE);
		
		TRY(Import.FindName(Import.BufferPosition, NAME_MESH));
	
		TRY(Import.FindName(Import.BufferPosition, NAME_MESH_NUMVERTEX));
		m -> VerticesCount = (int) Import.GetDouble(Import.BufferPosition);
		m -> Vertices = new Vertex[m -> VerticesCount];
		TRY(m -> Vertices); // Exit if not enougth memory...
	
		TRY(Import.FindName(Import.BufferPosition, NAME_MESH_NUMFACES));
		m -> FacesCount = (int) Import.GetDouble(Import.BufferPosition);
		m -> Faces = new Face[m -> FacesCount];
		TRY(m -> Faces); // Exit if not enougth memory...
		
		// Read vertices list...
		TRY(Import.FindName(Import.BufferPosition, NAME_MESH_VERTEX_LIST));
	
		int Index = 0;
		for (int i = 0; i < m -> VerticesCount; i++)
		{
			TRY(Import.FindName(Import.BufferPosition, NAME_MESH_VERTEX));
			Index = (int) Import.GetDouble(Import.BufferPosition);

			CHECK_VERTEX_INDEX((UINT) Index);
			Vertex &v = m -> Vertices[Index];

			v[0] = (GLfloat) Import.GetDouble(Import.BufferPosition);
			v[1] = (GLfloat) Import.GetDouble(Import.BufferPosition);
			v[2] = (GLfloat) Import.GetDouble(Import.BufferPosition);
		}	

		// Read faces list...
		TRY(Import.FindName(Import.BufferPosition, NAME_MESH_FACE_LIST));
	
		Index = 0;
		for (int i = 0; i < m -> FacesCount; i++)
		{
			TRY(Import.FindName(Import.BufferPosition, NAME_MESH_FACE));
			Index = (int) Import.GetDouble(Import.BufferPosition);
		
			CHECK_FACE_INDEX(Index);
			Face &f = m -> Faces[Index];

			TRY(Import.FindName(Import.BufferPosition, NAME_MESH_A));
			f.Vertices[0] = (UINT) Import.GetDouble(Import.BufferPosition);
				
			TRY(Import.FindName(Import.BufferPosition, NAME_MESH_B));
			f.Vertices[1] = (UINT) Import.GetDouble(Import.BufferPosition);

			TRY(Import.FindName(Import.BufferPosition, NAME_MESH_C));
			f.Vertices[2] = (UINT) Import.GetDouble(Import.BufferPosition);

			// Check indexes
			CHECK_VERTEX_INDEX(f.Vertices[0]);
			CHECK_VERTEX_INDEX(f.Vertices[1]);
			CHECK_VERTEX_INDEX(f.Vertices[2]);

			// Params
			f.ColorID = m -> ModelParams.ColorID;
			f.Reflection = m -> ModelParams.Reflection;
		}

		// All geometry readed.
		// Change axis orientation
		m -> InvertAxis(TRUE, TRUE, FALSE);

		// Normals can present in the file but 
		// it's to long to read all these values. Much 
		// faster is to calculate them from geometry

		// Calculate faces normals
		CHECK(m -> MakeFacesNormals());

		// Calculate simple vertices normals
		CHECK(m -> MakeVerticesNormals());

		// Some calculations
		m -> CalculateDims();

		// Add to list box
		AddListBoxModel(m);
		
		// Next object...
		ObjectFound = (Import.FindName(Import.BufferPosition, NAME_GEOMOBJECT) != NULL);
	}

	// Write some error handler here.
	// Would you like to add try-throw-catch chain here?
	if (ErrorOccured) 
		SomeError : 
		{
			DeleteModel(ModelsCount - 1);	
			return FALSE;
		}

	return TRUE;
};

// Fills Vertex and Faces arrays with file data.
// Set model's name provided in the file by "Named Object:".
// Returns FALSE if something will go wrong...
BOOL ModelsStore::LoadASC(const char *FileName)
{
	BOOL ErrorOccured = FALSE;

	// Parser...
	AsciiImport Import(hHeap);
	
	// Exit if file not loaded or some error occured...
	if (!Import.LoadBufferFromFile(FileName)) return FALSE;

	// Is here any objects?
	BOOL ObjectFound = (Import.FindName(NULL, NAME_NAMED_OBJECT) != NULL);

	while (ObjectFound)
	{
		// Add new model to be filled with data
		Model *m = AddModel();
		
		// Exit if no model added
		if (!m) return FALSE; 

		char *Name = Import.GetString(Import.BufferPosition);
		TRY(Name); // Exit if no name found or not enougth memory...

		// Set model name and autoparams
		m -> SetModelName(Name, TRUE);

		TRY(Import.FindName(Import.BufferPosition, NAME_VERTICES));
		m -> VerticesCount = (int) Import.GetDouble(Import.BufferPosition);
		m -> Vertices = new Vertex[m -> VerticesCount];
		TRY(m -> Vertices); // Exit if not enougth memory...
	
		TRY(Import.FindName(Import.BufferPosition, NAME_FACES));
		m -> FacesCount = (int) Import.GetDouble(Import.BufferPosition);
		m -> Faces = new Face[m -> FacesCount];
		TRY(m -> Faces); // Exit if not enougth memory...
		
		// Read vertices list...
		TRY(Import.FindName(Import.BufferPosition, NAME_VERTEX_LIST));
	
		int Index = 0;
		for (int i = 0; i < m -> VerticesCount; i++)
		{
			TRY(Import.FindName(Import.BufferPosition, NAME_VERTEX));
			Index = (int) Import.GetDouble(Import.BufferPosition);

			CHECK_VERTEX_INDEX((UINT) Index);
			Vertex &v = m -> Vertices[Index];
			
			TRY(Import.FindName(Import.BufferPosition, NAME_VERTEX_X));
			v[0] = (GLfloat) Import.GetDouble(Import.BufferPosition);
			
			TRY(Import.FindName(Import.BufferPosition, NAME_VERTEX_Y));
			v[1] = (GLfloat) Import.GetDouble(Import.BufferPosition);
			
			TRY(Import.FindName(Import.BufferPosition, NAME_VERTEX_Z));
			v[2] = (GLfloat) Import.GetDouble(Import.BufferPosition);
		}	

		// Read faces list...
		TRY(Import.FindName(Import.BufferPosition, NAME_FACE_LIST));
	
		Index = 0;
		for (int i = 0; i < m -> FacesCount; i++)
		{
			TRY(Import.FindName(Import.BufferPosition, NAME_FACE));
			Index = (int) Import.GetDouble(Import.BufferPosition);

			CHECK_FACE_INDEX(Index);
			Face &f = m -> Faces[Index];

			TRY(Import.FindName(Import.BufferPosition, NAME_MESH_A));
			f.Vertices[0] = (UINT) Import.GetDouble(Import.BufferPosition);
				
			TRY(Import.FindName(Import.BufferPosition, NAME_MESH_B));
			f.Vertices[1] = (UINT) Import.GetDouble(Import.BufferPosition);

			TRY(Import.FindName(Import.BufferPosition, NAME_MESH_C));
			f.Vertices[2] = (UINT) Import.GetDouble(Import.BufferPosition);

			// Check indexes
			CHECK_VERTEX_INDEX(f.Vertices[0]);
			CHECK_VERTEX_INDEX(f.Vertices[1]);
			CHECK_VERTEX_INDEX(f.Vertices[2]);
			
			// Params of model maked in
			// m -> SetModelName() before
			f.ColorID = m -> ModelParams.ColorID;
			f.Reflection = m -> ModelParams.Reflection;
		}
		
		// All geometry readed, invert
		m -> InvertAxis(TRUE, TRUE, FALSE);

		// Calculate faces normals
		CHECK(m -> MakeFacesNormals());

		// Calculate simple vertices normals
		CHECK(m -> MakeVerticesNormals());

		// Some calculations
		m -> CalculateDims();

		// Add to list box
		AddListBoxModel(m);

		// Next object...
		ObjectFound = (Import.FindName(Import.BufferPosition, NAME_NAMED_OBJECT) != NULL);
	}

	// Write some error handler here.
	// Would you like to add try-throw-catch chain here?
	if (ErrorOccured) 
		SomeError : 
		{
			DeleteModel(ModelsCount - 1);	
			return FALSE;
		}

	return TRUE;
};

// Get count of types of models 
void ModelsStore::GetTypesCount(int *TypesCount)
{
	for (int i = 0; i < ModelsCount; i++)
	{
		Model *m = ModelsList[i];
		TypesCount[m -> ModelType]++;
	}
};

BOOL ModelsStore::IsModelBodyPart(Model *m, BOOL AllWheels)
{
	BOOL WheelSteer = m -> ModelParams.WheelSteer;
	BOOL ModelBody = m -> ModelType == MODEL_BODY;
	BOOL ModelWheel = m -> ModelType == MODEL_WHEEL;

	if (!AllWheels) return (ModelBody || (ModelWheel && (!WheelSteer)));
	else return (ModelBody || ModelWheel);
};



int ModelsStore::GetBodyModelsListLen(BOOL AllWheels)
{
	int PartsCount = 0;
	
	for (int i = 0; i < ModelsCount; i++)
	{
		Model *m = ModelsList[i];
		if (IsModelBodyPart(m, AllWheels)) PartsCount++;
	}

	return PartsCount;
};

void ModelsStore::BuildBodyModelsList(Model **List, BOOL AllWheels)
{
	int j = 0;

	for (int i = 0; i < ModelsCount; i++)
	{
		Model *m = ModelsList[i];
		if (IsModelBodyPart(m, AllWheels)) 
		{
			List[j] = m;
			j++;
		}
	}
};

int ModelsStore::GetWheelsModelsOffset(Vertex &WheelsOffset)
{
	int WheelsCount = 0;

	WheelsOffset[0] = 0.0;
	WheelsOffset[1] = 0.0;
	WheelsOffset[2] = 0.0;

	for (int i = 0; i < Models.ModelsCount; i++)
	{
		Model *m = Models.GetModel(i);

		if (m -> ModelType == MODEL_WHEEL)
		{
			WheelsCount++;
			
			for (int j = 0; j < 3; j++)
				WheelsOffset[j] += m -> Offset[j];
		}
	}
	
	if (WheelsCount > 0)
	{
		WheelsOffset[0] /= WheelsCount;
		WheelsOffset[1] /= WheelsCount;
		WheelsOffset[2] /= WheelsCount;
	}

	return WheelsCount;
};

void ModelsStore::GetWheelsModelsBound(Vertex &WheelsMax, Vertex &WheelsMin)
{
	// Initialize
	for (int j = 0; j < 3; j++)
	{
		WheelsMax[j] = -FLT_MAX;
		WheelsMin[j] =  FLT_MAX;
	}

	for (int i = 0; i < Models.ModelsCount; i++)
	{
		Model *m = Models.GetModel(i);

		if (m -> ModelType == MODEL_WHEEL)
		{
			for (int j = 0; j < 3; j++)
			{
				if (m -> Max[j] > WheelsMax[j]) WheelsMax[j] = m -> Max[j];
				if (m -> Min[j] < WheelsMin[j]) WheelsMin[j] = m -> Min[j];
			}
		}
	}

};

// Function writes header of the file and all body parts.
// Also function RETURNS Offset vector of the model
BOOL ModelsStore::SaveBinaryBody(HANDLE hFile, Vertex &OutOffset, int *TypesCount, Model *BodyBound, float &OutScale)
{
	BOOL StatusOk = TRUE;

	// -------------------------------
	// MAKE BODY MODEL WITH ALL WHEELS
	// -------------------------------

	int ListLen = GetBodyModelsListLen(TRUE);
	if (!ListLen) return FALSE;
	
	Model **BodyModels = new Model * [ListLen];
	if (!BodyModels) return FALSE;

	// List of the body parts
	BuildBodyModelsList(BodyModels, TRUE);

	Model Body;
	StatusOk = Body.JoinModels(BodyModels, ListLen);

	// Don't need any more
	delete [] BodyModels;

	// Force return
	if (!StatusOk) return FALSE;
	
	Vertex WheelsOffset, MinusOffset;
	int WheelsCount = GetWheelsModelsOffset(WheelsOffset);
	
	if (WheelsCount > 0)
	{
		MinusOffset[0] = WheelsOffset[0];
		MinusOffset[1] = WheelsOffset[1];
	}
	else
	{
		MinusOffset[0] = Body.Offset[0];
		MinusOffset[1] = Body.Offset[1];
	}

	MinusOffset[2] = Body.Offset[2];

	// Return offset
	OutOffset[0] = MinusOffset[0];
	OutOffset[1] = MinusOffset[1];
	OutOffset[2] = MinusOffset[2];

	// Move to global center
	// before calculating radius
	Body.SetOffset(MinusOffset);

	// Return scale
	float Radius = Body.CalculateRadius(FALSE);
	OutScale = 124.0f / Radius;

	// Apply scale
	Body.SetScale(OutScale);

	// Optimize geometry
	StatusOk = Body.OptimizeGeometry(FALSE);

	// Some needed values
	Vertex BodyMax = {Body.Max[0], Body.Max[1], Body.Max[2]};
	float BodyRadius = Body.CalculateRadius(FALSE);
	
	// Create and optimize bound. Hmm... Stupid code?
	if (StatusOk) StatusOk = Body.MakeModelBound(BodyBound, BOUND_SEGMENTS);
	if (StatusOk) StatusOk = BodyBound -> OptimizeGeometry(TRUE);
	if (StatusOk) StatusOk = Body.MorphModelBound(BodyBound, TRUE);

	// Apply offset
	BodyBound -> Offset[0] = 0.0f;
	BodyBound -> Offset[1] = 0.0f;
	BodyBound -> Offset[2] = 0.0f;
	
// Debug section	
#ifdef SHOW_DEBUG_MODELS	
	Model *m = Models.AddModel();
	BodyBound -> CloneModel(m);
	m -> SetModelName("Debug model: BodyBound", FALSE);
	m -> ModelType = MODEL_UNKNOWN;
	AddListBoxModel(m);
#endif

	// Don't need geometry any more.
	// No further work with Body
	Body.EmptyModel();
	
	// Force return
	if (!StatusOk) return FALSE;

	// ------------------------------------
	// MAKE BODY MODEL WITHOUT STEER WHEELS
	// ------------------------------------

	ListLen = GetBodyModelsListLen(FALSE);
	if (!ListLen) return FALSE;

	BodyModels = new Model * [ListLen];
	if (!BodyModels) return FALSE;

	// List of the body parts
	BuildBodyModelsList(BodyModels, FALSE);

	Model Body2;
	StatusOk = Body2.JoinModels(BodyModels, ListLen);
	
	if (StatusOk)
	{
		Body2.SetOffset(MinusOffset);
		Body2.SetScale(OutScale);
		StatusOk = Body2.OptimizeGeometry(FALSE);

// Debug section	
#ifdef SHOW_DEBUG_MODELS	
		Model *m = Models.AddModel();
		Body2.CloneModel(m);
		m -> SetModelName("Debug model: Body2", FALSE);
		m -> ModelType = MODEL_UNKNOWN;
		AddListBoxModel(m);
#endif

		// Required by M_TOOL
		Body2.Offset[0] = 0.0;
		Body2.Offset[1] = 0.0;
		Body2.Offset[2] = 0.0;
	}

	// Don't need any more
	delete [] BodyModels;

	if (StatusOk) StatusOk = Body2.SaveBinary(hFile, FALSE);	

	// -------------------
	// SAVE MECHOUS PARAMS
	// -------------------

	if (StatusOk)
	{
		MechousSavingParamsStruct mss;
		
		mss.xMax = Round(BodyMax[0]);
		mss.yMax = Round(BodyMax[1]);
		mss.zMax = Round(BodyMax[2]);

		mss.BodyColorShift = Round(MechousParams[MECHOUS_PARAM_COLOR_SHIFT]);
		mss.BodyColorOffset = Round(MechousParams[MECHOUS_PARAM_COLOR_OFFSET]);

		mss.rMax = Round(BodyRadius);
		mss.DebrisCount = TypesCount[MODEL_DEBRIS];
		mss.WheelsCount = WheelsCount;

		DWORD w = 0;
		DWORD Size = sizeof(MechousSavingParamsStruct);
		WriteFile(hFile, (LPVOID) &mss, Size, &w, NULL);
	
		BOOL StatusOk = (w == Size);
	}

	return StatusOk;
};

BOOL ModelsStore::SaveBinaryWheels(HANDLE hFile, Vertex &Offset, int *TypesCount, float Scale)
{
	BOOL StatusOk = TRUE;

	int WheelNumber = 0;
	for (int i = 0; i < ModelsCount; i++)
	{
		Model *m = ModelsList[i];

		if (m -> ModelType == MODEL_WHEEL)
		{
			// Print save status
			if (PrintStatus) PrintStatus(STAGE_WHEEL, ++WheelNumber);
			
			// Go!
			Model Wheel;
			StatusOk = m -> CloneModel(&Wheel);
			if (!StatusOk) break;
			
			// Move to body offset 
			// and scale as needed
			Wheel.SetOffset(Offset);
			Wheel.SetScale(Scale);

			WheelSavingParamsStruct ws;

			ws.Steer = Wheel.ModelParams.WheelSteer;
			ws.Width = Round(Wheel.Max[0] - Wheel.Min[0]);
			ws.Radius = Round((Wheel.Max[2] - Wheel.Min[2]) / 2);
			
			ws.WheelVector[0] = Wheel.Offset[0];
			ws.WheelVector[1] = Wheel.Offset[1];
			ws.WheelVector[2] = Wheel.Offset[2];

			Vertex KeepOffset;	

			// I need to keep offset of the
			// wheel for a while as in M_TOOL
			KeepOffset[0] = Wheel.Offset[0];
			KeepOffset[1] = Wheel.Offset[1];
			KeepOffset[2] = Wheel.Offset[2];

			// Move to global center to
			// calculate rMax correctly
			Wheel.SetOffset(Wheel.Offset);
			
			// Restore original offset 
			// as tells me M_TOOL original
			Wheel.Offset[0] = KeepOffset[0];
			Wheel.Offset[1] = KeepOffset[1];
			Wheel.Offset[2] = KeepOffset[2];

			// Seems this parameter is not used
			ws.BoundIndex = 0;
			
			DWORD w = 0;
			const DWORD Size = sizeof(WheelSavingParamsStruct);
			WriteFile(hFile, (LPVOID) &ws, Size, &w, NULL);

			StatusOk = (w == Size);
			
			if (StatusOk && Wheel.ModelParams.WheelSteer)
			{
					StatusOk = Wheel.OptimizeGeometry(FALSE);
					
					/*
					// DEBUG START
					Model *m2 = Models.AddModel();
					Wheel.CloneModel(m2);
					m2 -> SetModelName("Debug model : Wheel");
					m2 -> ModelType = MODEL_UNKNOWN;
					AddListBoxModel(m2);
					// DEBUG END
					*/

					if (StatusOk) StatusOk = Wheel.SaveBinary(hFile, FALSE);
			}

			if (!StatusOk) break;
		}
		
		// Force exit
		if (TerminateSave)
		{
			StatusOk = FALSE;
			break;
		}
	}

	return StatusOk;
};

BOOL ModelsStore::SaveBinaryDebris(HANDLE hFile, float Scale)
{
	BOOL StatusOk = TRUE;

	int DebrisNumber = 0;
	for (int i = 0; i < ModelsCount; i++)
	{
		Model *m = ModelsList[i];
		if (m -> ModelType == MODEL_DEBRIS)
		{
			// Print save status
			if (PrintStatus) PrintStatus(STAGE_DEBRIS, ++DebrisNumber);
		
			// Go!
			Model Debris;
			StatusOk = m -> CloneModel(&Debris);
			if (!StatusOk) break;
			
			Debris.SetOffset(Debris.Offset);
			Debris.SetScale(Scale);
			
			StatusOk = Debris.OptimizeGeometry(FALSE);
			if (StatusOk) StatusOk = Debris.SaveBinary(hFile, FALSE);
			
			if (StatusOk)
			{
				Model DebrisBox;
				StatusOk = Debris.MakeModelBound(&DebrisBox, BOUND_SEGMENTS);
				if (StatusOk) StatusOk = DebrisBox.OptimizeGeometry(TRUE);
				if (StatusOk) StatusOk = Debris.MorphModelBound(&DebrisBox, FALSE);

				// Apply offset
				DebrisBox.Offset[0] = 0.0f;
				DebrisBox.Offset[1] = 0.0f;
				DebrisBox.Offset[2] = 0.0f;

				if (StatusOk) StatusOk = DebrisBox.SaveBinary(hFile, TRUE);			
			}

			if (!StatusOk) break;
		}

		// Force exit
		if (TerminateSave)
		{
			StatusOk = FALSE;
			break;
		}
	}

	return StatusOk;
};

BOOL ModelsStore::SaveBinarySlots(HANDLE hFile, Vertex &Offset, int *TypesCount, float Scale)
{
	BOOL StatusOk = TRUE;
	
	int SlotsExisting = 0;
	int SlotsCount = MAX_SLOTS;

	SlotSavingParamsStruct *ssp = new SlotSavingParamsStruct [SlotsCount];
	if (!ssp) return FALSE;

	// Clear store
	for (int i = 0; i < SlotsCount; i++)
	{
		SlotSavingParamsStruct &sspp = ssp[i];
		sspp.SlotAngle = 0;

		sspp.SlotVector[0] = 0;
		sspp.SlotVector[1] = 0;
		sspp.SlotVector[2] = 0;
	}
	
	int j = 0;
	for (int i = 0; i < ModelsCount; i++)
	{
		Model *m = ModelsList[i];
		if (m -> ModelType == MODEL_SLOT)
		{
			Model Slot;
			StatusOk = m -> CloneModel(&Slot);
			if (!StatusOk) break;

			// Move and scale
			Slot.SetOffset(Offset);
			Slot.SetScale(Scale);
			
			// Is slot with such ID not occupied?
			int SlotValue = 1 << (Slot.ModelParams.SlotID);
			StatusOk = !(SlotsExisting & SlotValue);
			if (!StatusOk) break;

			// Existence of slots
			SlotsExisting |= SlotValue;

			// Slot in saving struct
			SlotSavingParamsStruct &sspp = ssp[j];

			// Fill slot data
			switch (Slot.ModelParams.SlotAngle) 
			{
				case 0 : { sspp.SlotAngle = Round(-90 * PI_SLOTS / 180); break; }
				case 1 : { sspp.SlotAngle = Round( 90 * PI_SLOTS / 180); break; }
				case 2 : { sspp.SlotAngle = 0; break; }
			}
			
			for (int k = 0; k < 3; k++)
				sspp.SlotVector[k] = Round(Slot.Offset[k]);
			
			if (!StatusOk) break;

			j++;
		}

		// Force exit
		if (TerminateSave)
		{
			StatusOk = FALSE;
			break;
		}
	}

	DWORD w = 0;
	WriteFile(hFile, &SlotsExisting, sizeof(int), &w, NULL);
	StatusOk = (w == sizeof(int));
	
	if (StatusOk)
	{
		w = 0;
		const DWORD Size = sizeof(SlotSavingParamsStruct) * SlotsCount;
		WriteFile(hFile, (LPVOID) ssp, Size, &w, NULL);

		StatusOk = (w == Size);
	}

	delete [] ssp;

	return StatusOk;
};

// Do not want to write it much times...
#define CHECK_TERMINATED { if (TerminateSave) return FALSE; }

BOOL ModelsStore::SaveBinary(HANDLE hFile) 
{
	// Initialize flag
	TerminateSave = FALSE;
	
	// Count of models
	int TypesCount[5];
	ZeroMemory(&TypesCount, sizeof(int) * 5);
	GetTypesCount(TypesCount);

	Model BodyBound;
	float ModelsScale = 1.0f;
	Vertex Offset = {0.0f, 0.0f, 0.0f};

	BOOL Result = TRUE;
	CHECK_TERMINATED;

	if (TypesCount[MODEL_BODY])
	{
		// This function returns:
		// 1. Offset for all models
		// 2. Bounding box of complete model
		// 3. Scale of the complete model
		
		if (PrintStatus) PrintStatus(STAGE_BODY, 0);
		Result = SaveBinaryBody(hFile, Offset, TypesCount, &BodyBound, ModelsScale);
		if (!Result) return FALSE;
	}

	CHECK_TERMINATED;

	if (TypesCount[MODEL_WHEEL])
	{
		Result = SaveBinaryWheels(hFile, Offset, TypesCount, ModelsScale);
		if (!Result) return FALSE;
	}

	CHECK_TERMINATED;

	if (TypesCount[MODEL_DEBRIS])
	{
		Result = SaveBinaryDebris(hFile, ModelsScale);
		if (!Result) return FALSE;
	}

	CHECK_TERMINATED;

	// Save body bound here.
	// BodyBound will be invalid
	// if no body parts available
	if (TypesCount[MODEL_BODY])
	{
		if (PrintStatus) PrintStatus(STAGE_BOUND, 0);		
		Result = BodyBound.SaveBinary(hFile, TRUE);
		if (!Result) return FALSE;
	}

	CHECK_TERMINATED;

	if (TypesCount[MODEL_SLOT])
	{
		if (PrintStatus) PrintStatus(STAGE_SLOT, 0);
		Result = SaveBinarySlots(hFile, Offset, TypesCount, ModelsScale);
		if (!Result) return FALSE;
	}

	return TRUE;
};

BOOL ModelsStore::ExportBinaryM3D(const char *FileName)
{
	HANDLE hFile = CreateFileA(FileName, GENERIC_WRITE, 0, NULL, 
		CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);

	BOOL Result = (hFile != INVALID_HANDLE_VALUE);
	
	if (Result)
	{
		// Export...
		Result = Models.SaveBinary(hFile);
		CloseHandle(hFile);
	}

	return Result;
};

// ----------
// SAVE ASCII
// ----------

BOOL ModelsStore::SaveAscii(HANDLE hFile, BOOL OnlySelected)
{
	BOOL Result = TRUE;
	DWORD StartVertexIndex = 0;
	int ExportCount = 0;

	for (int i = 0; i < ModelsCount; i++)
	{
		Model *m = ModelsList[i];

		BOOL NeedSave = (OnlySelected && m -> ModelBoxed) || !OnlySelected;
		
		if (NeedSave) 
		{
			// Show status string
			if (PrintStatus) PrintStatus(STAGE_EXPORTING, ++ExportCount);

			// Save model
			Result = m -> SaveAscii(hFile, StartVertexIndex);
			StartVertexIndex += m -> VerticesCount;

			if (!Result) break;
		}
	}

	return Result;
};

BOOL ModelsStore::ExportAsciiOBJ(const char *FileName, BOOL OnlySelected)
{
	HANDLE hFile = CreateFileA(FileName, GENERIC_WRITE, 0, NULL, 
		CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);

	BOOL Result = (hFile != INVALID_HANDLE_VALUE);
	
	if (Result)
	{
		// Export...
		Result = Models.SaveAscii(hFile, OnlySelected);
		CloseHandle(hFile);
	}

	return Result;
};

// ------------
// LOAD SECTION
// ------------

BOOL ModelsStore::LoadBinaryBody(HANDLE hFile, int *TypesCount)
{
	Model *m = AddModel();
	if (!m) return FALSE;
	
	BOOL Result = TRUE;
	Result = m -> LoadBinary(hFile);
	
	if (Result)
	{
		m -> SetModelName("Body", TRUE);
		AddListBoxModel(m);

		MechousSavingParamsStruct mss;

		DWORD w = 0;
		DWORD Size = sizeof(MechousSavingParamsStruct);
		ReadFile(hFile, (LPVOID) &mss, Size, &w, NULL);

		Result = (w == Size);

		if (Result)
		{
			// Do not change these values without user notification...
			// MechousParams[MECHOUS_PARAM_COLOR_SHIFT] = (float) mss.BodyColorShift;
			// MechousParams[MECHOUS_PARAM_COLOR_OFFSET] = (float) mss.BodyColorOffset;

			TypesCount[MODEL_DEBRIS] = mss.DebrisCount;
			TypesCount[MODEL_WHEEL] = mss.WheelsCount;
		}
	}
	else
		DeleteModel(ModelsCount - 1);

	return Result;
};

BOOL ModelsStore::LoadBinaryWheels(HANDLE hFile, int WheelsCount)
{
	BOOL Result = TRUE;

	for (int i = 0; i < WheelsCount; i++)
	{
		WheelSavingParamsStruct ws;
		
		DWORD w = 0;
		const DWORD Size = sizeof(WheelSavingParamsStruct);
		ReadFile(hFile, (LPVOID) &ws, Size, &w, NULL);
		
		if (w != Size) 
		{
			Result = FALSE;
			break;
		}
	
		if (ws.Steer)
		{
			Model *m = AddModel();
			Result = (m != NULL);
			
			if (Result) Result = m -> LoadBinary(hFile);
				
			if (Result) 
			{
				Vertex WheelOffset;
				WheelOffset[0] = (GLfloat) -ws.WheelVector[0];
				WheelOffset[1] = (GLfloat) -ws.WheelVector[1];
				WheelOffset[2] = (GLfloat) -ws.WheelVector[2];

				m -> SetOffset(WheelOffset);
				m -> ModelParams.WheelSteer = ws.Steer;
					
				char Name[9];
				sprintf_s(Name, 9, "Wheel %d", i);
				m -> SetModelName(Name, TRUE);

				AddListBoxModel(m);
			}
			else
			{
				DeleteModel(ModelsCount - 1);
				break;
			}
		}
	}

	return Result;
};

BOOL ModelsStore::LoadBinaryDebris(HANDLE hFile, int DebrisCount)
{
	BOOL Result = TRUE;

	for (int i = 0; i < DebrisCount; i++)
	{
		// Debris itself
		Model *md = AddModel();
		Result = (md != NULL);
	
		if (Result) Result = md -> LoadBinary(hFile);

		if (Result)
		{
			char Name[16];
			sprintf_s(Name, 16, "Debris %d", i);
			md -> SetModelName(Name, TRUE);
			
			AddListBoxModel(md);
		}
		else
		{
			DeleteModel(ModelsCount - 1);
			break;
		}

		// Bound of debris
		Model *mdb = AddModel();
		Result = (mdb != NULL);
	
		if (Result) Result = mdb -> LoadBinary(hFile);
		
		if (Result) 
		{
			char Name[18];
			sprintf_s(Name, 18, "Debris %d - bound", i);
			mdb -> SetModelName(Name, TRUE);

			AddListBoxModel(mdb);
		}
		else
		{
			DeleteModel(ModelsCount - 1);	
			break;
		}

	}

	return Result;
};

BOOL ModelsStore::LoadBinarySlots(HANDLE hFile)
{
	BOOL Result = TRUE;

	int SlotsExisting = 0;
	int SlotsCount = 0;

	DWORD w = 0;
	ReadFile(hFile, &SlotsExisting, sizeof(int), &w, NULL);
	if (w != sizeof(int)) return FALSE;

	for (int i = 0; i < MAX_SLOTS; i++)
	{
		if (SlotsExisting & (1 << i))
			SlotsCount++;
	}

	w = 0;
	SlotSavingParamsStruct *ssp = new SlotSavingParamsStruct [SlotsCount];
	DWORD Size = sizeof(SlotSavingParamsStruct) * SlotsCount;
	ReadFile(hFile, (LPVOID) ssp, Size, &w, NULL);

	if (w != Size) Result = FALSE;

	if (Result)
	{
		// Add slot models
	}

	delete [] ssp;

	return Result;
};

BOOL ModelsStore::LoadBinary(HANDLE hFile)
{
	BOOL Result = TRUE;
    
	int TypesCount[5];
	ZeroMemory(&TypesCount, sizeof(int) * 5);

	Result = LoadBinaryBody(hFile, TypesCount);
	if (!Result) return FALSE;

	int WheelsCount = TypesCount[MODEL_WHEEL];
	if (WheelsCount) 
	{
		Result = LoadBinaryWheels(hFile, WheelsCount);
		if (!Result) return FALSE;
	}

	int DebrisCount = TypesCount[MODEL_DEBRIS];
	if (DebrisCount)
	{
		Result = LoadBinaryDebris(hFile, DebrisCount);
		if (!Result) return FALSE;
	}

	Model *BodyBound = AddModel();
	Result = (BodyBound != NULL);

	if (Result) 
	{
		Result = BodyBound -> LoadBinary(hFile);
		
		if (Result)
		{
			BodyBound -> SetModelName("Body - bound", TRUE);
			AddListBoxModel(BodyBound);

			Result = LoadBinarySlots(hFile);
		}
		else
		{
			DeleteModel(ModelsCount - 1);
			return FALSE;
		}
	}

	return Result;
};

BOOL ModelsStore::LoadM3D(const char *FileName)
{
	HANDLE hFile = CreateFileA(FileName, GENERIC_READ, FILE_SHARE_READ, NULL,
		OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);

	if (hFile == INVALID_HANDLE_VALUE) return FALSE;
	
	// LOADING HERE!!!
	BOOL StatusOk = LoadBinary(hFile);

	CloseHandle(hFile);

	return StatusOk;
};