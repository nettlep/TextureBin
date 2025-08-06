// Originally released under a custom license.
// This historical re-release is provided under the MIT License.
// See the LICENSE file in the repo root for details.
//
// https://github.com/nettlep
//

#ifndef	_H_3DS
#define	_H_3DS

#pragma pack(1)

// ----------------------------------------------------------------------------

typedef struct
{
	float		r, g, b;
} sRGB;

// ----------------------------------------------------------------------------

typedef struct
{
	float		x, y, z;
} sPoint3;

// ----------------------------------------------------------------------------

typedef struct
{
	float		x, y, z;
} sP3D;

// ----------------------------------------------------------------------------

typedef struct
{
	float		x, y;
} sP2D;

// ----------------------------------------------------------------------------

typedef struct
{
	unsigned short	a, b, c, flags;
	unsigned short	materialIndex;
	sP3D		normal;
} sFACE;

// ----------------------------------------------------------------------------

typedef struct matrix_struct
{
	char		name[80];
	sRGB		ambient;
	sRGB		diffuse;
	char		texture[80];
	float		shininess;
	unsigned char	transparent;
} sMAT;

// ----------------------------------------------------------------------------

typedef struct
{
	unsigned short	ID;
	unsigned long	length;
} sCHK;

// ----------------------------------------------------------------------------

typedef	struct
{
	short		revision;
	char		filename[80];
	int		animLen;
	int		start, end;
	int		curFrame;
} sKFHDR;

// ----------------------------------------------------------------------------

typedef	struct
{
	char		name[80];
	short		flags1;
	short		flags2;
	short		hierarchy;
} sNODEHDR;

// ----------------------------------------------------------------------------

typedef struct
{
	short		id;
} sNODEID;

// ----------------------------------------------------------------------------

typedef struct
{
	short		framenum;
	long		unknown;
	sP3D		pos;
} sPOS;

// ----------------------------------------------------------------------------

typedef struct
{
	short		flags;
	short		unknown[4];
	short		keys;
	short		unknown2;
	sPOS		*data;
} sPOSTRACK;

// ----------------------------------------------------------------------------

typedef struct
{
	short		framenum;
	long		unknown;
	float		rotRad;
	sP3D		axis;
} sROT;

// ----------------------------------------------------------------------------

typedef struct
{

	short		flags;
	short		unknown[4];
	short		keys;
	short		unknown2;
	sROT		*data;
} sROTTRACK;

// ----------------------------------------------------------------------------

typedef struct
{
	short		framenum;
	long		unknown;
	sP3D		scale;
} sSCL;

// ----------------------------------------------------------------------------

typedef struct
{
	short		flags;
	short		unknown[4];
	short		keys;
	short		unknown2;
	sSCL		*data;
} sSCLTRACK;

// ----------------------------------------------------------------------------

typedef struct
{
	short		framenum;
	long		unknown;
	float		fov;
} sFOV;

// ----------------------------------------------------------------------------

typedef struct
{
	short		flags;
	short		unknown[4];
	short		keys;
	short		unknown2;
	sFOV		*data;
} sFOVTRACK;

// ----------------------------------------------------------------------------

typedef struct
{
	short		framenum;
	long		unknown;
	float		roll;
} sROL;

// ----------------------------------------------------------------------------

typedef struct
{
	short		flags;
	short		unknown[4];
	short		keys;
	short		unknown2;
	sROL		*data;
} sROLTRACK;

// ----------------------------------------------------------------------------

typedef struct color_struct
{
	short		framenum;
	long		unknown;
	sRGB		rgb;
} sCOL;

// ----------------------------------------------------------------------------

typedef struct
{
	short		flags;
	short		unknown[4];
	short		keys;
	short		unknown2;
	sCOL		*data;
} sCOLTRACK;

// ----------------------------------------------------------------------------

typedef struct
{
	short		framenum;
	long		unknown;
	char		objName[80];
} sMOR;

// ----------------------------------------------------------------------------

typedef struct
{
	short		flags;
	short		unknown[4];
	short		keys;
	short		unknown2;
	sMOR		*data;
} sMORTRACK;

// ----------------------------------------------------------------------------

typedef struct
{
	short		framenum;
	long		unknown;
	float		hotSpotAngle;
} sHOT;

// ----------------------------------------------------------------------------

typedef struct
{
	short		flags;
	short		unknown[4];
	short		keys;
	short		unknown2;
	sHOT		*data;
} sHOTTRACK;

// ----------------------------------------------------------------------------

typedef struct
{
	short		framenum;
	long		unknown;
	float		fallOffAngle;
} sFAL;

// ----------------------------------------------------------------------------

typedef struct
{
	short		flags;
	short		unknown[4];
	short		keys;
	short		unknown2;
	sFAL		*data;
} sFALTRACK;

// ----------------------------------------------------------------------------

typedef struct
{
	sNODEHDR	nodeHeader;
	sP3D		pivot;
	sPOSTRACK	position;
	sROTTRACK	rotation;
	sSCLTRACK	scale;
} sOBJNODE;

// ----------------------------------------------------------------------------

typedef struct
{
	sNODEHDR	nodeHeader;
	sPOSTRACK	position;
	sFOVTRACK	fov;
	sROLTRACK	roll;
} sCAMNODE;

// ----------------------------------------------------------------------------

typedef struct
{
	sNODEHDR	nodeHeader;
	sPOSTRACK	position;
} sTGTNODE;

// ----------------------------------------------------------------------------

typedef struct
{
	sNODEHDR	nodeHeader;
	sPOSTRACK	position;
	sCOLTRACK	color;
} sLGTNODE;

// ----------------------------------------------------------------------------

typedef struct
{
	sNODEID		nodeID;
	sNODEHDR	nodeHeader;
	sPOSTRACK	position;
} sLTGTNODE;

// ----------------------------------------------------------------------------

typedef struct
{
	sNODEID		nodeID;
	sNODEHDR	nodeHeader;
	sPOSTRACK	position;
	sHOTTRACK	hotspot;
	sFALTRACK	falloff;
	sROLTRACK	roll;
	sCOLTRACK	color;
} sSPTNODE;

// ----------------------------------------------------------------------------

typedef struct light_struct
{
	sP3D		loc;
	char		unknown[6];
	sRGB		rgb;
	unsigned char	noLight, spotLight;
	float		innerRange, outerRange;
	float		hotSpot, fallOff;
	sPoint3		target;
	float		intensity;
	char		name[80];
} sLGT;

// ----------------------------------------------------------------------------

typedef struct
{
	sP3D		loc;
	sP3D		target;
	float		bank;
	float		lens;
	char		name[80];
} sCAM;

// ----------------------------------------------------------------------------

typedef struct
{
	sOBJNODE	*node;

	unsigned short	vCount;
	sP3D		*vList;

	unsigned short	fCount;
	sFACE		*fList;

	unsigned short	uvCount;
	sP2D		*uvList;

	char		name[80];
} sMSH;

// ----------------------------------------------------------------------------

typedef struct
{
	unsigned short	matCount, meshCount, camCount, lightCount;

	sMAT		*mat;
	sMSH		*mesh;
	sCAM		*cam;
	sLGT		*light;

	sKFHDR		kfInfo;
	int		objectNodeCount;
	int		cameraNodeCount;
	int		targetNodeCount;
	int		lightNodeCount;
	sOBJNODE	*objectNodeList;
	sCAMNODE	*cameraNodeList;
	sTGTNODE	*targetNodeList;
	sLGTNODE	*lightNodeList;
} sD3DS;

// ----------------------------------------------------------------------------

#pragma pack()

class	C3DS
{
private:
	enum	{DEF_STR_LEN = 512};
	enum	{BELL =	7};
	enum	{DOT_TIME = 2};

	enum	{CHUNK_HEADER_SIZE = 6};

	enum	{CHUNK_3DSFILE     = 0x4D4D};	// 3DS file descriptor
	enum	{CHUNK_MLIFILE     = 0x3DAA};	// MLI file descriptor
	enum	{CHUNK_PRJFILE     = 0xC23D};	// PRJ file descriptor
	enum	{CHUNK_MESH        = 0x3D3D};	// Mesh chunk
	enum	{CHUNK_AMBIENTCLR  = 0x2100};	// Ambient Color Block
	enum	{CHUNK_NAMEDOBJECT = 0x4000};	// Named Object
	enum	{CHUNK_TRIMESH     = 0x4100};	// Tri-Mesh
	enum	{CHUNK_VERTEXLIST  = 0x4110};	// Vertex Coordinate List Chunk
	enum	{CHUNK_FACELIST    = 0x4120};	// Face List Chunk
	enum	{CHUNK_MATERIALAPP = 0x4130};	// Material Application Chunk
	enum	{CHUNK_UVLIST      = 0x4140};	// Mapping Coordinate List Chunk
	enum	{CHUNK_TRANSMATRIX = 0x4160};	// Translation Matrix
	enum	{CHUNK_LIGHT       = 0x4600};	// Light
	enum	{CHUNK_RGB         = 0x0010};	// RGB Color
	enum	{CHUNK_24BIT       = 0x0011};	// 24-bit Color
	enum	{CHUNK_SPOTLIGHT   = 0x4610};	// SpotLight
	enum	{CHUNK_NOLIGHT     = 0x4620};	// Light is turned off
	enum	{CHUNK_INNER_RANGE = 0x4659};	// Inner range for spotlight
	enum	{CHUNK_OUTER_RANGE = 0x465A};	// Outer range for spotlight
	enum	{CHUNK_MULTIPLIER  = 0x465B};	// Light intensity
	enum	{CHUNK_CAMERA      = 0x4700};	// Camera
	enum	{CHUNK_MATERIAL    = 0xAFFF};	// Material chunk
	enum	{CHUNK_MATNAME     = 0xA000};	// Material name
	enum	{CHUNK_MATAMBIENT  = 0xA010};	// Material Ambient color
	enum	{CHUNK_MATDIFFUSE  = 0xA020};	// Material Diffuse color
	enum	{CHUNK_MATSPECULAR = 0xA030};	// Material Specular color
	enum	{CHUNK_MATSHINE    = 0xA040};	// Material Shininess
	enum	{CHUNK_MATTRANS    = 0xA050};	// Material Transparency
	enum	{CHUNK_MATTRANSFAL = 0xA052};	// Material Transparency fall-off
	enum	{CHUNK_MATREFBLUR  = 0xA053};	// Material Reflection blur
	enum	{CHUNK_MATTEXTURE  = 0xA200};	// Material Texture map
	enum	{CHUNK_MATOPACITY  = 0xA210};	// Material Opacity map
	enum	{CHUNK_MATTRANSFE  = 0xA240};	// Material Transparency fall-off enable
	enum	{CHUNK_MATREFBE    = 0xA250};	// Material Reflection blur enable
	enum	{CHUNK_KFDATA      = 0xB000};	// Keyframe data
	enum	{CHUNK_KFHDR       = 0xB00A};	// Keyframe header
	enum	{CHUNK_KFNODEHDR   = 0xB010};	// Keyframe node header
	enum	{CHUNK_KFSEG       = 0xB008};	// Keyframe start end
	enum	{CHUNK_KFCURTIME   = 0xB009};	// Keyframe current frame
	enum	{CHUNK_KFAMBNODE   = 0xB001};	// Keyframe ambient node
	enum	{CHUNK_KFOBJNODE   = 0xB002};	// Keyframe object node
	enum	{CHUNK_KFCAMNODE   = 0xB003};	// Keyframe camera node
	enum	{CHUNK_KFTGTNODE   = 0xB004};	// Keyframe target node
	enum	{CHUNK_KFLGTNODE   = 0xB005};	// Keyframe light node
	enum	{CHUNK_KFPIVOT     = 0xB013};	// Keyframe pivot point
	enum	{CHUNK_KFPOSTRACK  = 0xB020};	// Keyframe position info
	enum	{CHUNK_KFROTTRACK  = 0xB021};	// Keyframe rotation info
	enum	{CHUNK_KFSCLTRACK  = 0xB022};	// Keyframe scale info
	enum	{CHUNK_KFFOVTRACK  = 0xB023};	// Keyframe fov info
	enum	{CHUNK_KFROLTRACK  = 0xB024};	// Keyframe roll info
	enum	{CHUNK_KFCOLTRACK  = 0xB025};	// Keyframe color info
	enum	{CHUNK_KFMORTRACK  = 0xB026};	// Keyframe morph info

	char	currentNamedObject[80];

	// Material, Tri-Mesh, Light, and Camera information
	
	unsigned short	matCount;
	sMAT		*matList;
	unsigned short	meshCount;
	sMSH		*meshList;
	unsigned short	camCount;
	sCAM		*camList;
	unsigned short	lightCount;
	sLGT		*lightList;

	// Keyframe information

	sKFHDR		kfInfo;
	int		objectNodeCount;
	int		cameraNodeCount;
	int		targetNodeCount;
	int		lightNodeCount;
	sOBJNODE	*objectNodeList;
	sCAMNODE	*cameraNodeList;
	sTGTNODE	*targetNodeList;
	sLGTNODE	*lightNodeList;

	int		read3DS(FILE *InFile);
	int		process3DS(FILE *InFile, int ChunkLength);
	int		readAmbientColor(FILE *InFile, int ChunkLength);	
	int		readMeshChunk(FILE *InFile, int ChunkLength);
	int		readMaterial(FILE *InFile, int ChunkLength);
	int		readMaterialName(FILE *InFile, int ChunkLength);
	int		readMaterialAmbient(FILE *InFile, int ChunkLength);
	int		readMaterialColor(FILE *InFile, int ChunkLength);
	int		readMaterialShine(FILE *InFile, int ChunkLength);
	int		readMaterialTrans(FILE *InFile, int ChunkLength);
	int		readMaterialTexture(FILE *InFile, int ChunkLength);
	int		readKeyframeChunk(FILE *InFile, int ChunkLength);
	int		readKeyframeHeader(FILE *InFile, int ChunkLength);
	int		readObjectNode(FILE *InFile, int ChunkLength);
	int		readCameraNode(FILE *InFile, int ChunkLength);
	int		readTargetNode(FILE *InFile, int ChunkLength);
	int		readLightNode(FILE *InFile, int ChunkLength);
	int		readNamedObject(FILE *InFile, int ChunkLength);
	int		readTriMesh(FILE *InFile, int ChunkLength);
	int		readVertexList(FILE *InFile, int ChunkLength);
	int		readFaceList(FILE *InFile, int ChunkLength);
	int		readMaterialApp(FILE *InFile, int ChunkLength);
	int		readUVList(FILE *InFile, int ChunkLength);
	int		readTranslationMatrix(FILE *InFile, int ChunkLength);
	int		readLight(FILE *InFile, int ChunkLength);
	int		readLightRGB(FILE *InFile, int ChunkLength);
	int		readLight24Bit(FILE *InFile, int ChunkLength);
	int		readRGB(FILE *InFile, int ChunkLength);
	int		read24Bit(FILE *InFile, int ChunkLength);
	int		readSpotLight(FILE *InFile, int ChunkLength);
	int		readNoLight(FILE *InFile, int ChunkLength);
	int		readInnerRange(FILE *inFile, int chunkLength);
	int		readOuterRange(FILE *inFile, int chunkLength);
	int		readMultiplier(FILE *inFile, int chunkLength);
	int		readCamera(FILE *InFile, int ChunkLength);
	int		getChunkID(FILE *File, sCHK *Chunk);
	void		skipChunk(FILE *File, unsigned long Length);
	void		backup(FILE *File);
	int		myRead(void *Buffer, int Length, FILE *File);
	int		getString(FILE *InFile, char *String);

public:
			C3DS();
			~C3DS();

	int		load(const char *InName, sD3DS *Info);
};

#endif
