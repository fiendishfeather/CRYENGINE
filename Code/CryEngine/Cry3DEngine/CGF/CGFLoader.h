// Copyright 2001-2019 Crytek GmbH / Crytek Group. All rights reserved.

#pragma once

#include "../MeshCompiler/MeshCompiler.h"
#include "ChunkFile.h"
#include <Cry3DEngine/CGF/CGFContent.h>

#if defined(RESOURCE_COMPILER)
	#include "../../../Tools/CryCommonTools/Export/MeshUtils.h"
#endif

//////////////////////////////////////////////////////////////////////////
struct ILoaderCGFListener
{
	virtual ~ILoaderCGFListener(){}
	virtual void Warning(const char* format) = 0;
	virtual void Error(const char* format) = 0;
	virtual bool IsValidationEnabled() { return true; }
};

class CConverterCGF
{
private:
	struct SVF_TSpace
	{
		ushort tangent_binormal[8];
	};
	struct TSpace
	{
		Vec4 tangent;
		Vec3 bitangent;
	};
	union PosEl
	{
		uint16 uShortPos;
		int16 shortPos;
	};
	//Union vector
	class UVec4
	{
		union U
		{
			float vfloat;
			uint vuint;
			int vint;

			U& operator*(const float& other)
			{
				vfloat = vfloat*other;
				return *this;
			}
			U& operator*(const U& other)
			{
				vfloat = vfloat * other.vfloat;
				return *this;
			}
			U& operator+(const float& other)
			{
				vfloat = vfloat + other;
				return *this;
			}
			
			U() {}
			U(float f) : vfloat(f) {}
		};
	public:
		U x;
		U y;
		U z;
		U w;
	
		UVec4(float _x, float _y, float _z, float _w)
		{
			x.vfloat = _x;
			y.vfloat = _y;
			z.vfloat = _z;
			w.vfloat = _w;
		}

		float Dot(UVec4 v)
		{
			return x.vfloat * v.x.vfloat + y.vfloat * v.y.vfloat + z.vfloat * v.z.vfloat + w.vfloat * v.w.vfloat;
		}

		Vec4 ToVec4()
		{
			return Vec4(x.vfloat, y.vfloat, z.vfloat, w.vfloat);
		}

	};
public:
	CConverterCGF(IChunkFile::ChunkDesc* chunkDesc, AABB bbox);
	~CConverterCGF();

	void Process();
	void TranslateElemSize();
	inline int16_t ReadINT16(char *ByteArray, int32_t Offset);

	void ConvertP3s_c4b_t2s();
	void ConvertP3f_c4b_t2s();
	void ConvertNormals();
	void ConvertTangents();

	int GetStreamType();
	int GetCount();
	int GetElemSize();
	void* GetStreamData();

    float PackB2F(short i);
	short PackF2B(float f);

	//SC Vertex Shader - translated assembly code
	TSpace VSAssembly(Vec3 positions, uint tangentHex, uint bitangentHex, bool debug = false);
private:
	IChunkFile::ChunkDesc* pChunkDesc;

	int m_nStreamType;
	int m_nCount;
	int m_nElemSize;
	void* m_pStreamData;
	AABB m_bbox;

	int m_nElemSizeNew;
};

class CLoaderCGF
{
public:
	typedef void* (* AllocFncPtr)(size_t);
	typedef void (*  DestructFncPtr)(void*);

	CLoaderCGF(AllocFncPtr pAlloc = operator new, DestructFncPtr pDestruct = operator delete, bool bAllowStreamSharing = true);
	~CLoaderCGF();

	CContentCGF* LoadCGF(const char* filename, IChunkFile& chunkFile, ILoaderCGFListener* pListener, uint32 loadingFlags = 0);
	bool         LoadCGF(CContentCGF* pContentCGF, const char* filename, IChunkFile& chunkFile, ILoaderCGFListener* pListener, uint32 loadingFlags = 0);
	bool         LoadCGFFromMem(CContentCGF* pContentCGF, const void* pData, size_t nDataLen, IChunkFile& chunkFile, ILoaderCGFListener* pListener, uint32 loadingFlags = 0);

	const char*  GetLastError()                                  { return m_LastError; }
	CContentCGF* GetCContentCGF()                                { return m_pCompiledCGF; }

	void         SetMaxWeightsPerVertex(int maxWeightsPerVertex) { m_maxWeightsPerVertex = maxWeightsPerVertex; }

private:
	bool          LoadCGF_Int(CContentCGF* pContentCGF, const char* filename, IChunkFile& chunkFile, ILoaderCGFListener* pListener, uint32 loadingFlags);

	bool          LoadChunks(bool bJustGeometry);
	bool          LoadExportFlagsChunk(IChunkFile::ChunkDesc* pChunkDesc);
	bool          LoadNodeChunk(IChunkFile::ChunkDesc* pChunkDesc, bool bJustGeometry);

	bool          LoadHelperChunk(CNodeCGF* pNode, IChunkFile::ChunkDesc* pChunkDesc);
	bool          LoadGeomChunk(CNodeCGF* pNode, IChunkFile::ChunkDesc* pChunkDesc);
	bool          LoadCompiledMeshChunk(CNodeCGF* pNode, IChunkFile::ChunkDesc* pChunkDesc);
	bool          LoadMeshSubsetsChunk(CMesh& mesh, IChunkFile::ChunkDesc* pChunkDesc, std::vector<std::vector<uint16>>& globalBonesPerSubset);
	bool          LoadStreamDataChunk(int nChunkId, void*& pStreamData, int& nStreamType, int& nCount, int& nElemSize, bool& bSwapEndianness, AABB bbox = AABB());
	template<class T>
	bool          LoadStreamChunk(CMesh& mesh, const MESH_CHUNK_DESC_0801& chunk, ECgfStreamType Type, CMesh::EStream MStream);
	template<class TA, class TB>
	bool          LoadStreamChunk(CMesh& mesh, const MESH_CHUNK_DESC_0801& chunk, ECgfStreamType Type, CMesh::EStream MStreamA, CMesh::EStream MStreamB);
	bool          LoadBoneMappingStreamChunk(CMesh& mesh, const MESH_CHUNK_DESC_0801& chunk, const std::vector<std::vector<uint16>>& globalBonesPerSubset);
	bool          LoadIndexStreamChunk(CMesh& mesh, const MESH_CHUNK_DESC_0801& chunk);

	bool          LoadPhysicsDataChunk(CNodeCGF* pNode, int nPhysGeomType, int nChunkId);

	bool          LoadFoliageInfoChunk(IChunkFile::ChunkDesc* pChunkDesc);

	bool LoadVClothChunk(IChunkFile::ChunkDesc* pChunkDesc);

	CMaterialCGF* LoadMaterialFromChunk(int nChunkId);

	CMaterialCGF* LoadMaterialNameChunk(IChunkFile::ChunkDesc* pChunkDesc);

	void          SetupMeshSubsets(CMesh& mesh, CMaterialCGF* pMaterialCGF);

	//////////////////////////////////////////////////////////////////////////
	// loading of skinned meshes
	//////////////////////////////////////////////////////////////////////////
	bool         ProcessSkinning();
	CContentCGF* MakeCompiledSkinCGF(CContentCGF* pCGF, std::vector<int>* pVertexRemapping, std::vector<int>* pIndexRemapping);

	//old chunks
	bool   ReadBoneNameList(IChunkFile::ChunkDesc* pChunkDesc);
	bool   ReadMorphTargets(IChunkFile::ChunkDesc* pChunkDesc);
	bool   ReadBoneInitialPos(IChunkFile::ChunkDesc* pChunkDesc);
	bool   ReadBoneHierarchy(IChunkFile::ChunkDesc* pChunkDesc);
	uint32 RecursiveBoneLoader(int nBoneParentIndex, int nBoneIndex);
	bool   ReadBoneMesh(IChunkFile::ChunkDesc* pChunkDesc);

	//new chunks
	bool ReadCompiledBones(IChunkFile::ChunkDesc* pChunkDesc);
	bool ReadCompiledPhysicalBones(IChunkFile::ChunkDesc* pChunkDesc);
	bool ReadCompiledPhysicalProxies(IChunkFile::ChunkDesc* pChunkDesc);
	bool ReadCompiledMorphTargets(IChunkFile::ChunkDesc* pChunkDesc);

	bool ReadCompiledIntFaces(IChunkFile::ChunkDesc* pChunkDesc);
	bool ReadCompiledIntSkinVertice(IChunkFile::ChunkDesc* pChunkDesc);
	bool ReadCompiledExt2IntMap(IChunkFile::ChunkDesc* pChunkDesc);
	bool ReadCompiledBonesBoxes(IChunkFile::ChunkDesc* pChunkDesc);

	bool ReadCompiledBreakablePhysics(IChunkFile::ChunkDesc* pChunkDesc);

	void Warning(const char* szFormat, ...) PRINTF_PARAMS(2, 3);

private:
	uint32                              m_IsCHR;
	uint32                              m_CompiledBones;
	uint32                              m_CompiledBonesBoxes;
	uint32                              m_CompiledMesh;

	uint32                              m_numBonenameList;
	uint32                              m_numBoneInitialPos;
	uint32                              m_numMorphTargets;
	uint32                              m_numBoneHierarchy;

	std::vector<uint32>                 m_arrIndexToId;     // the mapping BoneIndex -> BoneID
	std::vector<uint32>                 m_arrIdToIndex;     // the mapping BoneID -> BineIndex
	std::vector<string>                 m_arrBoneNameTable; // names of bones
	std::vector<Matrix34>               m_arrInitPose34;
#if defined(RESOURCE_COMPILER)
	std::vector<MeshUtils::VertexLinks> m_arrLinksTmp;
	std::vector<int>                    m_vertexOldToNew; // used to re-map uncompiled Morph Target vertices right after reading them
#endif

	CContentCGF* m_pCompiledCGF;
	const void*  m_pBoneAnimRawData, * m_pBoneAnimRawDataEnd;
	uint32       m_numBones;
	int          m_nNextBone;

	//////////////////////////////////////////////////////////////////////////

	string       m_LastError;

	char         m_filename[260];

	IChunkFile*  m_pChunkFile;
	CContentCGF* m_pCGF;

	// To find really used materials
	uint16              MatIdToSubset[MAX_SUB_MATERIALS];
	int                 nMaterialsUsed;

	ILoaderCGFListener* m_pListener;

	bool                m_bUseReadOnlyMesh;
	bool                m_bAllowStreamSharing;

	int                 m_maxWeightsPerVertex;

	AllocFncPtr         m_pAllocFnc;
	DestructFncPtr      m_pDestructFnc;

	bool m_bIsConverted;
};
