
// ASE identificators support...
const char NAME_GEOMOBJECT[]		 = "*GEOMOBJECT";
const char NAME_NODE_NAME[]			 = "*NODE_NAME";

const char NAME_MESH[]				 = "*MESH";
const char NAME_MESH_NUMVERTEX[]	 = "*MESH_NUMVERTEX";
const char NAME_MESH_NUMFACES[]		 = "*MESH_NUMFACES";

const char NAME_MESH_FACE[]			 = "*MESH_FACE";
const char NAME_MESH_FACE_LIST[]	 = "*MESH_FACE_LIST";
const char NAME_MESH_FACENORMAL[]	 = "*MESH_FACENORMAL";
const char NAME_MESH_NORMALS[]		 = "*MESH_NORMALS";
const char NAME_MESH_A[]			 = "A:";
const char NAME_MESH_B[]			 = "B:";
const char NAME_MESH_C[]			 = "C:";

const char NAME_MESH_VERTEX[]		 = "*MESH_VERTEX";
const char NAME_MESH_VERTEX_LIST[]	 = "*MESH_VERTEX_LIST";

// ASC identificators support...
const char NAME_NAMED_OBJECT[]	 = "Named Object:";
const char NAME_VERTICES[]		 = "Vertices:";
const char NAME_FACES[]			 = "Faces:";

const char NAME_VERTEX_LIST[]	 = "Vertex list:";
const char NAME_VERTEX[]		 = "Vertex";

const char NAME_VERTEX_X[]		 = "X:";
const char NAME_VERTEX_Y[]		 = "Y:";
const char NAME_VERTEX_Z[]		 = "Z:";

const char NAME_FACE_LIST[]		 = "Face list:";
const char NAME_FACE[]			 = "Face";


class AsciiImport
{
private:
	HANDLE hHeap;
	
	char *BufferEnd; 
    char *String;

	BOOL IsWhitespace(BYTE Value);
    BOOL IsNumeric(BYTE Value);
    char *GetNumberEnd(const char *Start);

public:
	// Buffer
	char *FileBuffer;
	char *BufferPosition;
	
	AsciiImport(HANDLE hProcessHeap);
	~AsciiImport();

	// Read data from file to buffer
	BOOL LoadBufferFromFile(const char *FileName);
	char *FindName(const char *Start, const char *Name);
	
	double GetDouble(const char *Start);
    char *GetString(const char *Start);
};