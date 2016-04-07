#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/process.h>
#include <sysutil/sysutil_sysparam.h>
#include <cell/codec.h>
#include <cell/gcm.h>

#include "cellutil.h"
#include "gcmutil.h"
#include "padutil.h"

SYS_PROCESS_PARAM(1001, 0x10000);

using namespace cell::Gcm;

void* host_addr = NULL;
#define HOST_SIZE (16*1024*1024)
#define COLOR_BUFFER_NUM 2
#define VERTEX_SIZE 512

typedef struct
{
	float Px, Py, Pz;
	uint32_t RGBA; 
	float u, v;
} Vertex_t;

uint32_t g_Width;
uint32_t g_Height; 
uint32_t g_ColorDepth = 4; // ARGB8
uint32_t g_ZDepth = 4;     // COMPONENT24

// address of Frame/Z
uint32_t g_FrameBaseOffset;
uint32_t g_FrameOffset[2];
uint32_t g_FramePitch;
uint32_t g_DepthBaseOffset;
uint32_t g_DepthOffset;
uint32_t g_DepthPitch;

// Shader
extern uint32_t     _binary_vp_shader_vpo_start;
extern uint32_t     _binary_vp_shader_vpo_end;
extern uint32_t     _binary_fp_shader_fpo_start;
extern uint32_t     _binary_fp_shader_fpo_end;

// Vertex buffer					
static Vertex_t *   g_VertexBuffer;             // This is vidmem
uint32_t            g_VertexBufferOffset;       // This is vidmem

static CGprogram    g_CGVertexProgram;          // CG binary program
static CGprogram    g_CGFragmentProgram;        // CG binary program
static void *       g_VertexProgramUCode;       // This is sysmem
static void *       g_FragmentProgramUCode;     // This is vidmem

static uint32_t     g_FrameIndex = 0;
static float        g_MVP[16];


// Textures
#define TEX_SIZE 256
static uint32_t  g_TextureWidth = TEX_SIZE;
static uint32_t  g_TextureHeight = TEX_SIZE;

// TODO: What about X32_FLOAT and TEXTURE_W32_Z32_Y32_X32_FLOAT ?
const uint32_t g_FormatCount = 25;

const uint32_t g_ColorFormats[g_FormatCount] = {
	CELL_GCM_TEXTURE_B8, CELL_GCM_TEXTURE_A1R5G5B5, CELL_GCM_TEXTURE_A4R4G4B4, CELL_GCM_TEXTURE_R5G6B5,
	CELL_GCM_TEXTURE_A8R8G8B8, CELL_GCM_TEXTURE_COMPRESSED_DXT1, CELL_GCM_TEXTURE_COMPRESSED_DXT23, CELL_GCM_TEXTURE_COMPRESSED_DXT45,
	CELL_GCM_TEXTURE_G8B8, CELL_GCM_TEXTURE_R6G5B5, CELL_GCM_TEXTURE_DEPTH24_D8, CELL_GCM_TEXTURE_DEPTH24_D8_FLOAT,
	CELL_GCM_TEXTURE_DEPTH16, CELL_GCM_TEXTURE_DEPTH16_FLOAT, CELL_GCM_TEXTURE_X16, CELL_GCM_TEXTURE_Y16_X16,
	CELL_GCM_TEXTURE_R5G5B5A1, CELL_GCM_TEXTURE_COMPRESSED_HILO8, CELL_GCM_TEXTURE_COMPRESSED_HILO_S8, CELL_GCM_TEXTURE_W16_Z16_Y16_X16_FLOAT,
	CELL_GCM_TEXTURE_D1R5G5B5, CELL_GCM_TEXTURE_D8R8G8B8, CELL_GCM_TEXTURE_Y16_X16_FLOAT, CELL_GCM_TEXTURE_COMPRESSED_B8R8_G8R8,
	CELL_GCM_TEXTURE_COMPRESSED_R8B8_R8G8
};

const char* g_TextureFiles[g_FormatCount] = {
	"/app_home/Texture-B8.raw",
	"/app_home/Texture-A1R5G5B5.raw",
	"/app_home/Texture-A4R4G4B4.raw",
	"/app_home/Texture-R5G6B5.raw",
	"/app_home/Texture-A8R8G8B8.raw",
	"/app_home/Texture-COMPRESSED_DXT1.raw",
	"/app_home/Texture-COMPRESSED_DXT23.raw",
	"/app_home/Texture-COMPRESSED_DXT45.raw",
	"/app_home/Texture-G8B8.raw",
	"/app_home/Texture-R6G5B5.raw",
	"/app_home/Texture-DEPTH24_D8.raw",
	"/app_home/Texture-DEPTH24_D8_FLOAT.raw",
	"/app_home/Texture-DEPTH16.raw",
	"/app_home/Texture-DEPTH16_FLOAT.raw",
	"/app_home/Texture-X16.raw",
	"/app_home/Texture-Y16_X16.raw",
	"/app_home/Texture-R5G5B5A1.raw",
	"/app_home/Texture-COMPRESSED_HILO8.raw",
	"/app_home/Texture-COMPRESSED_HILO_S8.raw",
	"/app_home/Texture-W16_Z16_Y16_X16_FLOAT.raw",
	"/app_home/Texture-D1R5G5B5.raw",
	"/app_home/Texture-D8R8G8B8.raw",
	"/app_home/Texture-Y16_X16_FLOAT.raw",
	"/app_home/Texture-COMPRESSED_B8R8_G8R8.raw",
	"/app_home/Texture-COMPRESSED_R8B8_R8G8.raw",
};

const uint32_t g_ColorDepths[g_FormatCount] = {
	1,  2,  2,  2,  4,  //TEXTURE_B8, TEXTURE_A1R5G5B5, TEXTURE_A4R4G4B4, TEXTURE_R5G6B5, TEXTURE_A8R8G8B8,
	2,  4,  4,  2,  2,  //TEXTURE_COMPRESSED_DXT1, TEXTURE_COMPRESSED_DXT23, TEXTURE_COMPRESSED_DXT45, TEXTURE_G8B8, TEXTURE_R6G5B5,
	4,  4,  2,  2,  2,  //TEXTURE_DEPTH24_D8, TEXTURE_DEPTH24_D8_FLOAT, TEXTURE_DEPTH16, TEXTURE_DEPTH16_FLOAT, TEXTURE_X16,
	4,  2,  2,  2,  8,  //TEXTURE_Y16_X16, TEXTURE_R5G5B5A1, TEXTURE_COMPRESSED_HILO8, TEXTURE_COMPRESSED_HILO_S8, TEXTURE_W16_Z16_Y16_X16_FLOAT
	2,  4,  4,  2,  2   //TEXTURE_D1R5G5B5, TEXTURE_D8R8G8B8, TEXTURE_Y16_X16_FLOAT, TEXTURE_COMPRESSED_B8R8_G8R8, TEXTURE_COMPRESSED_R8B8_R8G8
};

static void *       g_Textures[g_FormatCount];
static uint32_t     g_TexturesOffset[g_FormatCount];


static int32_t initDisplay(void)
{
	// Read the current video status
	CellVideoOutState videoState;
	int32_t ret = cellVideoOutGetState(CELL_VIDEO_OUT_PRIMARY, 0, &videoState);

	if (ret != CELL_OK)
	{
		printf("cellVideoOutGetState() failed: 0x%x\n", ret);
		sys_process_exit(1);
	}

	CellVideoOutResolution videoRes;
	ret = cellVideoOutGetResolution(videoState.displayMode.resolutionId, &videoRes);

	if (ret != CELL_OK)
	{
		printf("cellVideoOutGetResolution() failed: 0x%x\n", ret);
		sys_process_exit(1);
	}

	g_Width  = videoRes.width;
	g_Height = videoRes.height;

	CellVideoOutConfiguration videoCfg;
	memset(&videoCfg, 0, sizeof(CellVideoOutConfiguration));
	videoCfg.resolutionId = videoState.displayMode.resolutionId;
	videoCfg.format = CELL_VIDEO_OUT_BUFFER_COLOR_FORMAT_X8R8G8B8;
	videoCfg.pitch = cellGcmGetTiledPitchSize(cellGcmAlign(CELL_GCM_ZCULL_ALIGN_WIDTH, g_Width)*g_ColorDepth);
  
	ret = cellVideoOutConfigure(CELL_VIDEO_OUT_PRIMARY, &videoCfg, NULL, 0);

	if (ret != CELL_OK)
	{
		printf("cellVideoOutConfigure() failed: 0x%x\n", ret);
		sys_process_exit(1);
	}
  
	cellGcmSetFlipMode(CELL_GCM_DISPLAY_VSYNC);
	return 0;
}

static int32_t initBuffer(void)
{
	// Preparations for buffer allocation, etc.
	uint32_t bufferWidth = cellGcmAlign(CELL_GCM_ZCULL_ALIGN_WIDTH, g_Width);

	g_FramePitch = cellGcmGetTiledPitchSize(bufferWidth*g_ColorDepth);
	g_DepthPitch = cellGcmGetTiledPitchSize(bufferWidth*g_ZDepth);
	if (!g_FramePitch || !g_DepthPitch)
	{
		return -1;
	}

	uint32_t FrameSize  = g_FramePitch*cellGcmAlign(CELL_GCM_ZCULL_ALIGN_HEIGHT, g_Height); // Size of 1 color buffer
	uint32_t DepthSize  = g_DepthPitch*cellGcmAlign(CELL_GCM_ZCULL_ALIGN_HEIGHT, g_Height); // Size of 1 depth buffer
	uint32_t FrameLimit = cellGcmAlign(CELL_GCM_TILE_ALIGN_SIZE, COLOR_BUFFER_NUM * FrameSize); // Size of all the color buffers, to set tiled
	uint32_t DepthLimit = cellGcmAlign(CELL_GCM_TILE_ALIGN_SIZE, DepthSize);                    // Size of all the depth buffers, to set tiled

	// Get config
	CellGcmConfig gcmConfig;
	cellGcmGetConfiguration(&gcmConfig);

	// Local Memory
	cellGcmUtilInitializeLocalMemory((size_t)gcmConfig.localAddress, (size_t)gcmConfig.localSize);
	void *FrameBaseAddress= cellGcmUtilAllocateLocalMemory(FrameLimit, CELL_GCM_TILE_ALIGN_OFFSET);
	void *DepthBaseAddress= cellGcmUtilAllocateLocalMemory(DepthLimit, CELL_GCM_TILE_ALIGN_OFFSET);

	int32_t ret = cellGcmAddressToOffset(FrameBaseAddress, &g_FrameBaseOffset);

	if (ret != CELL_OK)
	{
		printf("cellGcmAddressToOffset() failed: 0x%x\n", ret);
		sys_process_exit(1);
	}

	ret = cellGcmAddressToOffset(DepthBaseAddress, &g_DepthBaseOffset);

	if (ret != CELL_OK)
	{
		printf("cellGcmAddressToOffset() failed: 0x%x\n", ret);
		sys_process_exit(1);
	}

	// Buffers
	for (uint32_t j = 0; j < 2; j++)
	{
		g_FrameOffset[j] = g_FrameBaseOffset + (j * FrameSize);
	}

	g_DepthOffset = g_DepthBaseOffset;

	// Set tile
	ret = cellGcmSetTileInfo(0, CELL_GCM_LOCATION_LOCAL, g_FrameBaseOffset, FrameLimit, g_FramePitch, CELL_GCM_COMPMODE_DISABLED, 0, 0);

	if (ret != CELL_OK)
	{
		printf("cellGcmSetTileInfo() failed: 0x%x\n", ret);
		sys_process_exit(1);
	}

	ret = cellGcmBindTile(0);

	if (ret != CELL_OK)
	{
		printf("cellGcmBindTile() failed: 0x%x\n", ret);
		sys_process_exit(1);
	}

	ret = cellGcmSetTileInfo(1, CELL_GCM_LOCATION_LOCAL, g_DepthBaseOffset, DepthLimit, g_DepthPitch, CELL_GCM_COMPMODE_Z32_SEPSTENCIL, 0, 3);

	if (ret != CELL_OK)
	{
		printf("cellGcmSetTileInfo() failed: 0x%x\n", ret);
		sys_process_exit(1);
	}

	ret = cellGcmBindTile(1);

	if (ret != CELL_OK)
	{
		printf("cellGcmBindTile() failed: 0x%x\n", ret);
		sys_process_exit(1);
	}

	// Register surface
	for (uint32_t i = 0; i < COLOR_BUFFER_NUM; i++)
	{
		ret = cellGcmSetDisplayBuffer(i, g_FrameOffset[i], g_FramePitch, g_Width, g_Height);

		if (ret != CELL_OK)
		{
			printf("cellGcmSetDisplayBuffer() failed: 0x%x\n", ret);
			sys_process_exit(1);
		}
	}

	return 0;
}

static void initShader(void)
{
	g_CGVertexProgram   = (CGprogram)(void *)&_binary_vp_shader_vpo_start;
	g_CGFragmentProgram = (CGprogram)(void *)&_binary_fp_shader_fpo_start;

	// init  
	cellGcmCgInitProgram(g_CGVertexProgram);
	cellGcmCgInitProgram(g_CGFragmentProgram);

	// allocate video memory for fragment program
	uint32_t ucodeSize;
	void *ucode;

	// get and copy 
	cellGcmCgGetUCode(g_CGFragmentProgram, &ucode, &ucodeSize);

	g_FragmentProgramUCode = cellGcmUtilAllocateLocalMemory(ucodeSize, 64);
	assert(g_FragmentProgramUCode != NULL);
	memcpy(g_FragmentProgramUCode, ucode, ucodeSize); 

	// get ucode pointer 
	cellGcmCgGetUCode(g_CGVertexProgram, &ucode, &ucodeSize);
	g_VertexProgramUCode = ucode;
}

static void SetRenderTarget(const uint32_t Index)
{
	CellGcmSurface rt;
	memset(&rt, 0, sizeof(rt));
	rt.colorFormat      = CELL_GCM_SURFACE_A8R8G8B8;
	rt.colorTarget      = CELL_GCM_SURFACE_TARGET_0;
	rt.colorLocation[0] = CELL_GCM_LOCATION_LOCAL;
	rt.colorOffset[0]   = g_FrameOffset[Index];
	rt.colorPitch[0]    = g_FramePitch;

	rt.depthFormat      = CELL_GCM_SURFACE_Z24S8;
	rt.depthLocation    = CELL_GCM_LOCATION_LOCAL;
	rt.depthOffset      = g_DepthOffset;
	rt.depthPitch       = g_DepthPitch;

	rt.colorLocation[1] = CELL_GCM_LOCATION_LOCAL;
	rt.colorLocation[2] = CELL_GCM_LOCATION_LOCAL;
	rt.colorLocation[3] = CELL_GCM_LOCATION_LOCAL;
	rt.colorOffset[1]   = 0;
	rt.colorOffset[2]   = 0;
	rt.colorOffset[3]   = 0;
	rt.colorPitch[1]    = 64;
	rt.colorPitch[2]    = 64;
	rt.colorPitch[3]    = 64;

	rt.antialias        = CELL_GCM_SURFACE_CENTER_1;
	rt.type             = CELL_GCM_SURFACE_PITCH;

	rt.x                = 0;
	rt.y                = 0;
	rt.width            = g_Width;
	rt.height           = g_Height;
	cellGcmSetSurface(&rt);
}

static int32_t initVtxTexBuffer(void)
{
	// Allocate vertex buffer
	uint32_t vsize = sizeof(Vertex_t);
	g_VertexBuffer = (Vertex_t*)cellGcmUtilAllocateLocalMemory(vsize * VERTEX_SIZE, CELL_GCM_TILE_ALIGN_OFFSET);
	assert(g_VertexBuffer != NULL);
	int32_t ret = cellGcmAddressToOffset(g_VertexBuffer, &g_VertexBufferOffset);

	if (ret != CELL_OK)
	{
		printf("cellGcmAddressToOffset() failed: 0x%x\n", ret);
		sys_process_exit(1);
	}

	// Allocate texture buffers
	for (size_t i = 0; i < g_FormatCount; i++)
	{
		g_Textures[i] = cellGcmUtilAllocateLocalMemory(g_TextureWidth * g_TextureHeight * g_ColorDepths[i], CELL_GCM_TILE_ALIGN_OFFSET);
		assert(g_Textures[i] != NULL);
		ret = cellGcmAddressToOffset(g_Textures[i], &g_TexturesOffset[i]);

		if (ret != CELL_OK)
		{
			printf("cellGcmAddressToOffset() failed: 0x%x\n", ret);
			sys_process_exit(1);
		}

		memset(g_Textures[i], 0xFF, TEX_SIZE*TEX_SIZE*g_ColorDepths[i]);
	}

	return 0;
}

static void BuildGLProjection(float *M, const float top, const float bottom, const float left, const float right, const float near, const float far)
{
	memset(M, 0, 16*sizeof(float)); 

	M[0*4+0] = (2.0f * near) / (right - left);
	M[1*4+1] = (2.0f * near) / (bottom - top);

	float A = (right + left) / (right - left);
	float B = (top + bottom) / (top - bottom);
	float C = -(far + near) / (far - near);
	float D = -(2.0f*far*near) / (far - near);

	M[0*4 + 2] = A;
	M[1*4 + 2] = B;
	M[2*4 + 2] = C;
	M[3*4 + 2] = -1.0f; 
	M[2*4 + 3] = D;
}

static uint32_t GenerateRectangle(Vertex_t *V, int i)
{
	const Vertex_t *Base = V;
	uint32_t C0 = 0xFFFFFFFF;

	const int rows = 5;
	const int cols = 5;

	for (int i=0; i<rows; i++) {
		for (int j=0; j<cols; j++) {
			V->Px =- 1.0 + (j * 2.0 / float(cols));      V->Py=-1.0+(i*2.0/float(cols));     V->Pz=-1.0;  V->RGBA=C0; V->u=0.0f; V->v=0.0f; V++;
			V->Px =- 1.0 + ((j + 1) * 2.0 / float(cols));  V->Py=-1.0+(i*2.0/float(cols));     V->Pz=-1.0;  V->RGBA=C0; V->u=1.0f; V->v=0.0f; V++;
			V->Px =- 1.0 + (j * 2.0 / float(cols));      V->Py=-1.0+((i+1)*2.0/float(cols)); V->Pz=-1.0;  V->RGBA=C0; V->u=0.0f; V->v=1.0f; V++;

			V->Px =- 1.0 + ((j + 1) * 2.0 / float(cols));  V->Py=-1.0+(i*2.0/float(cols));     V->Pz=-1.0;  V->RGBA=C0; V->u=1.0f; V->v=0.0f; V++;
			V->Px =- 1.0 + (j * 2.0 / float(cols));      V->Py=-1.0+((i+1)*2.0/float(cols)); V->Pz=-1.0;  V->RGBA=C0; V->u=0.0f; V->v=1.0f; V++;
			V->Px =- 1.0 + ((j + 1) * 2.0 / float(cols));  V->Py=-1.0+((i+1)*2.0/float(cols)); V->Pz=-1.0;  V->RGBA=C0; V->u=1.0f; V->v=1.0f; V++;
		}
	}

	return V - Base;
}

static void initModelView(void)
{
	BuildGLProjection(g_MVP, -1.0f, 1.0f, -1.0f, 1.0f, 1.0, 10000.0f); 
}

static void setViewport(void)
{
    uint16_t x, y, w,h;
    float min = 0.0f, max = 1.0f;

    x = y = 0;
    w = g_Width;
	h = g_Height;

    float scale[4] = {w * 0.5, h * -0.5f, (max-min) * 0.5f, 0.0f};
    float offset[4] = {x + scale[0], h - y + scale[1], (max+min) * 0.5f, 0.0f};

    cellGcmSetViewport(x, y, w, h, min, max, scale, offset); 
    cellGcmSetScissor(x, y, w, h);
}

static void drawRectangle(void)
{
	// Inital state
	cellGcmSetClearColor((32 << 0) | (32 << 8) | (32 << 16) | (32 << 24));
	cellGcmSetBlendEnable(CELL_GCM_FALSE);
	cellGcmSetDepthTestEnable(CELL_GCM_TRUE);
	cellGcmSetDepthFunc(CELL_GCM_LESS);
	cellGcmSetClearDepthStencil(0xffffff << 8 | 0);
	cellGcmSetClearSurface(CELL_GCM_CLEAR_Z | CELL_GCM_CLEAR_R | CELL_GCM_CLEAR_G | CELL_GCM_CLEAR_B | CELL_GCM_CLEAR_A);

	// Set uniform variables
	CGparameter modelViewProj = cellGcmCgGetNamedParameter(g_CGVertexProgram, "modelViewProj");
	CGparameter objCoord = cellGcmCgGetNamedParameter(g_CGVertexProgram, "a2v.objCoord");
	CGparameter color = cellGcmCgGetNamedParameter(g_CGVertexProgram, "a2v.color");
	CGparameter tex = cellGcmCgGetNamedParameter(g_CGVertexProgram, "a2v.tex");

	CELL_GCMUTIL_CG_PARAMETER_CHECK_ASSERT(modelViewProj);
	CELL_GCMUTIL_CG_PARAMETER_CHECK_ASSERT(objCoord);
	CELL_GCMUTIL_CG_PARAMETER_CHECK_ASSERT(color);
	CELL_GCMUTIL_CG_PARAMETER_CHECK_ASSERT(tex);

	cellGcmSetVertexProgramParameter(modelViewProj, g_MVP);

	// get Vertex Attribute index
	uint32_t ObjCoordIndex = cellGcmCgGetParameterResource(g_CGVertexProgram, objCoord) - CG_ATTR0; 
	uint32_t ColorIndex = cellGcmCgGetParameterResource(g_CGVertexProgram, color) - CG_ATTR0; 
	uint32_t TexIndex = cellGcmCgGetParameterResource(g_CGVertexProgram, tex) - CG_ATTR0; 
    
	// Textures
	CGparameter texture = cellGcmCgGetNamedParameter(g_CGFragmentProgram, "texture0");
    CELL_GCMUTIL_CG_PARAMETER_CHECK_ASSERT( texture );
    CGresource texunit = (CGresource)(cellGcmCgGetParameterResource(g_CGFragmentProgram, texture) - CG_TEXUNIT0);
      
    CellGcmTexture cubetex;
    cubetex.remap =   (CELL_GCM_TEXTURE_REMAP_REMAP << 14 |
             CELL_GCM_TEXTURE_REMAP_REMAP << 12 |
             CELL_GCM_TEXTURE_REMAP_REMAP << 10 |
             CELL_GCM_TEXTURE_REMAP_REMAP <<  8 |
             CELL_GCM_TEXTURE_REMAP_FROM_B << 6 |
             CELL_GCM_TEXTURE_REMAP_FROM_G << 4 |
             CELL_GCM_TEXTURE_REMAP_FROM_R << 2 |
             CELL_GCM_TEXTURE_REMAP_FROM_A);

    cubetex.mipmap = 1;
    cubetex.dimension = CELL_GCM_TEXTURE_DIMENSION_2;
    cubetex.cubemap = CELL_GCM_FALSE;
    cubetex.width = g_TextureWidth;
    cubetex.height = g_TextureHeight;
    cubetex.depth = 1;
    cubetex.location = CELL_GCM_LOCATION_LOCAL;
    

	// Bind the shaders
	cellGcmSetVertexProgram(g_CGVertexProgram, g_VertexProgramUCode);

	uint32_t fragment_offset;
	CELL_GCMUTIL_CHECK_ASSERT(cellGcmAddressToOffset(g_FragmentProgramUCode, &fragment_offset));
	cellGcmSetFragmentProgram(g_CGFragmentProgram, fragment_offset);

	uint32_t vertex_offset[3];

	for (size_t i=0; i<g_FormatCount; i++)
	{
		//printf("RenderTextures: Format %d / %d (%s)\n", i, g_FormatCount, g_TextureFiles[i]);
		cubetex.format = (g_ColorFormats[i] | CELL_GCM_TEXTURE_LN | CELL_GCM_TEXTURE_NR);
		cubetex.offset = g_TexturesOffset[i];
		cubetex.pitch = g_TextureWidth * g_ColorDepths[i];
		cellGcmSetTexture(texunit, &cubetex);

		// Bind texture and set filter
		cellGcmSetTextureControl(texunit, CELL_GCM_TRUE, 0, 0, 0); // MIN:0, MAX:15
      
		cellGcmSetTextureAddress(texunit,
					   CELL_GCM_TEXTURE_CLAMP,
					   CELL_GCM_TEXTURE_CLAMP,
					   CELL_GCM_TEXTURE_CLAMP,
					   CELL_GCM_TEXTURE_UNSIGNED_REMAP_NORMAL,
					   CELL_GCM_TEXTURE_ZFUNC_LESS,
					   0);
		cellGcmSetTextureFilter(texunit, 0,
					  CELL_GCM_TEXTURE_LINEAR,
					  CELL_GCM_TEXTURE_LINEAR,
					  CELL_GCM_TEXTURE_CONVOLUTION_QUINCUNX);

		//GenerateRectangle(g_VertexBuffer, 1);
		CELL_GCMUTIL_CHECK_ASSERT(cellGcmAddressToOffset(&g_VertexBuffer->Px, &vertex_offset[0]));
		CELL_GCMUTIL_CHECK_ASSERT(cellGcmAddressToOffset(&g_VertexBuffer->RGBA, &vertex_offset[1]));
		CELL_GCMUTIL_CHECK_ASSERT(cellGcmAddressToOffset(&g_VertexBuffer->u, &vertex_offset[2]));

		// Set vertex pointer
		cellGcmSetVertexDataArray(ObjCoordIndex, 0, sizeof(Vertex_t), 3, CELL_GCM_VERTEX_F, CELL_GCM_LOCATION_LOCAL, vertex_offset[0]);
		cellGcmSetVertexDataArray(ColorIndex, 0, sizeof(Vertex_t), 4, CELL_GCM_VERTEX_UB, CELL_GCM_LOCATION_LOCAL, vertex_offset[1]);
		cellGcmSetVertexDataArray(TexIndex, 0, sizeof(Vertex_t), 2, CELL_GCM_VERTEX_F, CELL_GCM_LOCATION_LOCAL, vertex_offset[2]);

		// Draw
		cellGcmSetDrawArrays(CELL_GCM_PRIMITIVE_TRIANGLES, 6*i, 6);
	}    
}

// Flip related functions
static bool sFlipped=true;
static void FlipCallback(uint32_t head)
{
    (void)head;
    sFlipped=true;
}

static void waitFlip(void)
{
    while (!sFlipped)
	{
        sys_timer_usleep(300);
    }

    sFlipped=false;
}

static void flip(void)
{
    if (cellGcmSetFlip(g_FrameIndex) != CELL_OK)
	{
		return;
	}
	
    cellGcmFlush();
    cellGcmSetWaitFlip();
}

int main(void)
{
	// Disable printf buffering
	setbuf(stdout, NULL);

	printf("TEST00008 by AlexAltea. Updated by tambre.\n");
	printf("Displaying different texture formats.\n\n");

	// Initialize
	host_addr = memalign(1024*1024, HOST_SIZE);
	int32_t ret = cellGcmInit(0x10000, HOST_SIZE, host_addr);

	if (ret != CELL_OK)
	{
		printf("cellGcmInit() failed: 0x%x\n", ret);
		sys_process_exit(1);
	}

	cellGcmSetDebugOutputLevel(CELL_GCM_DEBUG_LEVEL2);

	initDisplay();
	initBuffer();
	initVtxTexBuffer();
	initShader();
	initModelView();

	sFlipped = true;
	cellGcmUtilInitCallback(NULL);
	cellGcmSetFlipHandler(FlipCallback);

	// Load textures
	for (size_t format=0; format<g_FormatCount; format++)
	{
		FILE *file = fopen(g_TextureFiles[format], "rb");

		if (file!=NULL)
		{
			fread(g_Textures[format], sizeof(char), 256*256*g_ColorDepths[format], file);
			fclose(file);
		}
	}

	GenerateRectangle(g_VertexBuffer, 1);

	// Rendering loop
	while (cellGcmUtilCheckCallback())
	{
		// Set inital target
		g_FrameIndex ^= 0x1;
		SetRenderTarget(g_FrameIndex);

		cellGcmSetColorMask(CELL_GCM_COLOR_MASK_B | CELL_GCM_COLOR_MASK_G | CELL_GCM_COLOR_MASK_R | CELL_GCM_COLOR_MASK_A);
		cellGcmSetColorMaskMrt(0);
		setViewport();
		drawRectangle();
		cellGcmFinish(1);

		// Wait until the previous flip is done
		waitFlip();
		flip();
	}
    
	return 0;
}
