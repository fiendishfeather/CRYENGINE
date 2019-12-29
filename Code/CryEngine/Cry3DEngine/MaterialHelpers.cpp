// Copyright 2001-2019 Crytek GmbH / Crytek Group. All rights reserved.

#include "StdAfx.h"
#include "MaterialHelpers.h"
#include <CryString/CryPath.h>

/* -----------------------------------------------------------------------
 * These functions are used in Cry3DEngine, CrySystem, CryRenderD3D11,
 * Editor, ResourceCompilerMaterial and more
 */

//////////////////////////////////////////////////////////////////////////
namespace
{

static struct
{
	EEfResTextures slot;
	const char*    ename;
	bool           adjustable;
	const char*    name;
	const char*    suffix;
}
s_TexSlotSemantics[] =
{
	// NOTE: must be in order with filled holes to allow direct lookup
	{ EFTT_DIFFUSE,          "EFTT_DIFFUSE",          true,  "Diffuse",      "_diff"   },
	{ EFTT_NORMALS,          "EFTT_NORMALS",          true,  "Bumpmap",      "_ddn"    },        // Ideally "Normal" but need to keep backwards-compatibility
	{ EFTT_SPECULAR,         "EFTT_SPECULAR",         true,  "Specular",     "_spec"   },
	{ EFTT_ENV,              "EFTT_ENV",              true,  "Environment",  "_cm"     },
	{ EFTT_DETAIL_OVERLAY,   "EFTT_DETAIL_OVERLAY",   true,  "Detail",       "_detail" },
	{ EFTT_SMOOTHNESS,       "EFTT_SMOOTHNESS",       false, "Smoothness",   "_ddna"   },
	{ EFTT_HEIGHT,           "EFTT_HEIGHT",           true,  "Heightmap",    "_displ"  },
	{ EFTT_DECAL_OVERLAY,    "EFTT_DECAL_OVERLAY",    true,  "Decal",        ""        },         // called "DecalOverlay" in the shaders
	{ EFTT_SUBSURFACE,       "EFTT_SUBSURFACE",       true,  "SubSurface",   "_sss"    },         // called "Subsurface" in the shaders
	{ EFTT_CUSTOM,           "EFTT_CUSTOM",           true,  "Custom",       ""        },         // called "CustomMap" in the shaders
	{ EFTT_CUSTOM_SECONDARY, "EFTT_CUSTOM_SECONDARY", true,  "[1] Custom",   ""        },
	{ EFTT_OPACITY,          "EFTT_OPACITY",          true,  "Opacity",      ""        },
	{ EFTT_TRANSLUCENCY,     "EFTT_TRANSLUCENCY",     false, "Translucency", "_trans"  },
	{ EFTT_EMITTANCE,        "EFTT_EMITTANCE",        true,  "Emittance",    "_em"     },

	{ EFTT_LAYER1_DIFFUSE,   "EFTT_LAYER1_DIFFUSE",   true,  "Layer1 Diffuse",    "" },
	{ EFTT_LAYER1_NORMALS,   "EFTT_LAYER1_NORMALS",   true,  "Layer1 Bumpmap",    "" },
	{ EFTT_LAYER2_DIFFUSE,   "EFTT_LAYER2_DIFFUSE",   true,  "Layer2 Diffuse",    "" },
	{ EFTT_LAYER2_NORMALS,   "EFTT_LAYER2_NORMALS",   true,  "Layer2 Bumpmap",    "" },
	{ EFTT_LAYER3_DIFFUSE,   "EFTT_LAYER3_DIFFUSE",   true,  "Layer3 Diffuse",    "" },
	{ EFTT_LAYER3_NORMALS,   "EFTT_LAYER3_NORMALS",   true,  "Layer3 Bumpmap",    "" },
	{ EFTT_LAYER4_DIFFUSE,   "EFTT_LAYER4_DIFFUSE",   true,  "Layer4 Diffuse",    "" },
	{ EFTT_LAYER4_NORMALS,   "EFTT_LAYER4_NORMALS",   true,  "Layer4 Bumpmap",    "" },

	{ EFTT_LAYER1_WT_DIFFUSE,	"EFTT_LAYER1_WT_DIFFUSE",	true,   "Layer1 WT Diffuse",    "" },
	{ EFTT_LAYER1_WT_NORMALS,	"EFTT_LAYER1_WT_NORMALS",	true,   "Layer1 WT Bumpmap",    "" },
	{ EFTT_LAYER2_WT_DIFFUSE,	"EFTT_LAYER2_WT_DIFFUSE",	true,   "Layer2 WT Diffuse",    "" },
	{ EFTT_LAYER2_WT_NORMALS,	"EFTT_LAYER2_WT_NORMALS",	true,   "Layer2 WT Bumpmap",    "" },
	{ EFTT_LAYER3_WT_DIFFUSE,	"EFTT_LAYER3_WT_DIFFUSE",	true,   "Layer3 WT Diffuse",    "" },
	{ EFTT_LAYER3_WT_NORMALS,	"EFTT_LAYER3_WT_NORMALS",	true,   "Layer3 WT Bumpmap",    "" },
	{ EFTT_LAYER4_WT_DIFFUSE,	"EFTT_LAYER4_WT_DIFFUSE",	true,   "Layer4 WT Diffuse",    "" },
	{ EFTT_LAYER4_WT_NORMALS,   "EFTT_LAYER4_WT_NORMALS",   true,   "Layer4 WT Bumpmap",    "" },

	{ EFTT_LAYER1_SMOOTHNESS,   "EFTT_LAYER1_SMOOTHNESS",   false,  "Layer1 Smoothness",    "" },
	{ EFTT_LAYER2_SMOOTHNESS,   "EFTT_LAYER2_SMOOTHNESS",   false,  "Layer2 Smoothness",    "" },
	{ EFTT_LAYER3_SMOOTHNESS,   "EFTT_LAYER3_SMOOTHNESS",   false,  "Layer3 Smoothness",    "" },
	{ EFTT_LAYER4_SMOOTHNESS,   "EFTT_LAYER4_SMOOTHNESS",   false,  "Layer4 Smoothness",    "" },

	{ EFTT_LAYER1_WT_SMOOTHNESS,   "EFTT_LAYER1_WT_SMOOTHNESS",   false,  "Layer1 WT Smoothness",    "" },
	{ EFTT_LAYER2_WT_SMOOTHNESS,   "EFTT_LAYER2_WT_SMOOTHNESS",   false,  "Layer2 WT Smoothness",    "" },
	{ EFTT_LAYER3_WT_SMOOTHNESS,   "EFTT_LAYER3_WT_SMOOTHNESS",   false,  "Layer3 WT Smoothness",    "" },
	{ EFTT_LAYER4_WT_SMOOTHNESS,   "EFTT_LAYER4_WT_SMOOTHNESS",   false,  "Layer4 WT Smoothness",    "" },

	// Backwards compatible names are found here and mapped to the updated enum
	{ EFTT_NORMALS,          "EFTT_BUMP",             false, "Normal",       ""        },         // called "Bump" in the shaders
	{ EFTT_SMOOTHNESS,       "EFTT_GLOSS_NORMAL_A",   false, "GlossNormalA", ""        },
	{ EFTT_HEIGHT,           "EFTT_BUMPHEIGHT",       false, "Height",       ""        },         // called "BumpHeight" in the shaders
	

	// This is the terminator for the name-search
	{ EFTT_UNKNOWN,          "EFTT_UNKNOWN",          false, nullptr,        ""        },
};

static struct
{
	EEfResTextures nativeSlot;
	const char*    scSlotName;
}
s_SCTexSlotPairs[] =
{
	{ EFTT_DIFFUSE,				"TexSlot1"},
	{ EFTT_NORMALS,				"TexSlot2"},
	{ EFTT_CUSTOM_SECONDARY,	"TexSlot3"},
	{ EFTT_SPECULAR,			"TexSlot4"},
	{ EFTT_ENV,					"TexSlot5"},
	{ EFTT_DETAIL_OVERLAY,		"TexSlot6"},
	//{ EFTT_NORMALS,	"TexSlot7"},
	//{ EFTT_NORMALS,	"TexSlot8"},
	{ EFTT_CUSTOM,				"TexSlot9"},
	//{ EFTT_NORMALS,	"TexSlot10"},
	//{ EFTT_NORMALS,	"TexSlot11"},
	{ EFTT_OPACITY,				"TexSlot12"},
	{ EFTT_LAYER1_NORMALS,		"TexSlot13"},

	// This is the terminator for the slotname-search
	{ EFTT_UNKNOWN,				 nullptr}
};

static struct
{
	EEfResTextures nativeSlot;
	const char*    scSlotName;
}
s_SCTexSlotPairsLB[] =
{
	{ EFTT_NORMALS,				"Custom"},
	{ EFTT_DIFFUSE,				"[1] Custom"},
	{ EFTT_EMITTANCE,			"Opacity"},
	{ EFTT_DECAL_OVERLAY,			"Decal"},

	// This is the terminator for the slotname-search
	{ EFTT_UNKNOWN,				 nullptr}
};

static struct
{
	int matRefSlot;
	EEfResTextures layerTexSlotDiffuse;
	EEfResTextures layerTexSlotBumpmap;
}
s_SCTexSlotMatReferencesPairs[] =
{
	{ 0,	EFTT_LAYER1_DIFFUSE,	EFTT_LAYER1_NORMALS},
	{ 1,	EFTT_LAYER2_DIFFUSE,	EFTT_LAYER2_NORMALS},
	{ 2,	EFTT_LAYER3_DIFFUSE,	EFTT_LAYER3_NORMALS},
	{ 3,	EFTT_LAYER4_DIFFUSE,	EFTT_LAYER4_NORMALS},
	{ 4,	EFTT_LAYER1_WT_DIFFUSE,	EFTT_LAYER1_WT_NORMALS},
	{ 5,	EFTT_LAYER2_WT_DIFFUSE,	EFTT_LAYER2_WT_NORMALS},
	{ 6,	EFTT_LAYER3_WT_DIFFUSE,	EFTT_LAYER3_WT_NORMALS},
	{ 7,	EFTT_LAYER4_WT_DIFFUSE,	EFTT_LAYER4_WT_NORMALS},

	// This is the terminator for the slotname-search
	//{ EFTT_UNKNOWN,				 nullptr, nullptr}
};

#if 0
static class Verify
{
public:
	Verify()
	{
		for (int i = 0; s_TexSlotSemantics[i].name; i++)
		{
			if (s_TexSlotSemantics[i].slot != i)
			{
				throw std::runtime_error("Invalid texture slot lookup array.");
			}
		}
	}
}
s_VerifyTexSlotSemantics;
#endif

}

EEfResTextures MaterialHelpers::FindTexSlot(const char* texName) const
{
	if (texName)
	{
		for (int i = 0; s_TexSlotSemantics[i].name; i++)
		{
			if (stricmp(s_TexSlotSemantics[i].name, texName) == 0)
			{
				return s_TexSlotSemantics[i].slot;
			}
		}
	}

	return EFTT_UNKNOWN;
}

const char* MaterialHelpers::FindTexName(EEfResTextures texSlot) const
{
	for (int i = 0; s_TexSlotSemantics[i].name; i++)
	{
		if (s_TexSlotSemantics[i].slot == texSlot)
		{
			return s_TexSlotSemantics[i].name;
		}
	}

	return NULL;
}

const char* MaterialHelpers::LookupTexName(EEfResTextures texSlot) const
{
	assert((texSlot >= 0) && (texSlot < EFTT_MAX));
	return s_TexSlotSemantics[texSlot].name;
}

const char* MaterialHelpers::LookupTexEnum(EEfResTextures texSlot) const
{
	assert((texSlot >= 0) && (texSlot < EFTT_MAX));
	return s_TexSlotSemantics[texSlot].ename;
}

const char* MaterialHelpers::LookupTexSuffix(EEfResTextures texSlot) const
{
	assert((texSlot >= 0) && (texSlot < EFTT_MAX));
	return s_TexSlotSemantics[texSlot].suffix;
}

bool MaterialHelpers::IsAdjustableTexSlot(EEfResTextures texSlot) const
{
	assert((texSlot >= 0) && (texSlot < EFTT_MAX));
	return s_TexSlotSemantics[texSlot].adjustable;
}

//////////////////////////////////////////////////////////////////////////
bool MaterialHelpers::SetGetMaterialParamFloat(IRenderShaderResources& pShaderResources, const char* sParamName, float& v, bool bGet) const
{
	EEfResTextures texSlot = EFTT_UNKNOWN;

	if (!stricmp("emissive_intensity", sParamName))
		texSlot = EFTT_EMITTANCE;
	else if (!stricmp("shininess", sParamName))
		texSlot = EFTT_SMOOTHNESS;
	else if (!stricmp("opacity", sParamName))
		texSlot = EFTT_OPACITY;

	if (!stricmp("alpha", sParamName))
	{
		if (bGet)
			v = pShaderResources.GetAlphaRef();
		else
			pShaderResources.SetAlphaRef(v);

		return true;
	}
	else if (texSlot != EFTT_UNKNOWN)
	{
		if (bGet)
			v = pShaderResources.GetStrengthValue(texSlot);
		else
			pShaderResources.SetStrengthValue(texSlot, v);

		return true;
	}

	return false;
}

//////////////////////////////////////////////////////////////////////////
bool MaterialHelpers::SetGetMaterialParamVec3(IRenderShaderResources& pShaderResources, const char* sParamName, Vec3& v, bool bGet) const
{
	EEfResTextures texSlot = EFTT_UNKNOWN;

	if (!stricmp("diffuse", sParamName))
		texSlot = EFTT_DIFFUSE;
	else if (!stricmp("specular", sParamName))
		texSlot = EFTT_SPECULAR;
	else if (!stricmp("emissive_color", sParamName))
		texSlot = EFTT_EMITTANCE;

	if (texSlot != EFTT_UNKNOWN)
	{
		if (bGet)
			v = pShaderResources.GetColorValue(texSlot).toVec3();
		else
			pShaderResources.SetColorValue(texSlot, ColorF(v, 1.0f));

		return true;
	}

	return false;
}

//////////////////////////////////////////////////////////////////////////
void MaterialHelpers::SetTexModFromXml(SEfTexModificator& pTextureModifier, const XmlNodeRef& node) const
{
	XmlNodeRef modNode = node->findChild("TexMod");
	if (modNode)
	{
		// Modificators
		float f;
		uint8 c;

		modNode->getAttr("TexMod_RotateType", pTextureModifier.m_eRotType);
		modNode->getAttr("TexMod_TexGenType", pTextureModifier.m_eTGType);
		modNode->getAttr("TexMod_bTexGenProjected", pTextureModifier.m_bTexGenProjected);

		for (int baseu = 'U', u = baseu; u <= 'W'; u++)
		{
			char RT[] = "Rotate?";
			RT[6] = u;

			if (modNode->getAttr(RT, f)) pTextureModifier.m_Rot[u - baseu] = Degr2Word(f);

			char RR[] = "TexMod_?RotateRate";
			RR[7] = u;
			char RP[] = "TexMod_?RotatePhase";
			RP[7] = u;
			char RA[] = "TexMod_?RotateAmplitude";
			RA[7] = u;
			char RC[] = "TexMod_?RotateCenter";
			RC[7] = u;

			if (modNode->getAttr(RR, f)) pTextureModifier.m_RotOscRate[u - baseu] = Degr2Word(f);
			if (modNode->getAttr(RP, f)) pTextureModifier.m_RotOscPhase[u - baseu] = Degr2Word(f);
			if (modNode->getAttr(RA, f)) pTextureModifier.m_RotOscAmplitude[u - baseu] = Degr2Word(f);
			if (modNode->getAttr(RC, f)) pTextureModifier.m_RotOscCenter[u - baseu] = f;

			if (u > 'V')
				continue;

			char TL[] = "Tile?";
			TL[4] = u;
			char OF[] = "Offset?";
			OF[6] = u;

			if (modNode->getAttr(TL, f)) pTextureModifier.m_Tiling[u - baseu] = f;
			if (modNode->getAttr(OF, f)) pTextureModifier.m_Offs[u - baseu] = f;

			char OT[] = "TexMod_?OscillatorType";
			OT[7] = u;
			char OR[] = "TexMod_?OscillatorRate";
			OR[7] = u;
			char OP[] = "TexMod_?OscillatorPhase";
			OP[7] = u;
			char OA[] = "TexMod_?OscillatorAmplitude";
			OA[7] = u;

			if (modNode->getAttr(OT, c)) pTextureModifier.m_eMoveType[u - baseu] = c;
			if (modNode->getAttr(OR, f)) pTextureModifier.m_OscRate[u - baseu] = f;
			if (modNode->getAttr(OP, f)) pTextureModifier.m_OscPhase[u - baseu] = f;
			if (modNode->getAttr(OA, f)) pTextureModifier.m_OscAmplitude[u - baseu] = f;
		}
	}
}

//////////////////////////////////////////////////////////////////////////
static SEfTexModificator defaultTexMod;
static bool defaultTexMod_Initialized = false;

void MaterialHelpers::SetXmlFromTexMod(const SEfTexModificator& pTextureModifier, XmlNodeRef& node) const
{
	if (!defaultTexMod_Initialized)
	{
		ZeroStruct(defaultTexMod);
		defaultTexMod.m_Tiling[0] = 1;
		defaultTexMod.m_Tiling[1] = 1;
		defaultTexMod_Initialized = true;
	}

	if (memcmp(&pTextureModifier, &defaultTexMod, sizeof(pTextureModifier)) == 0)
	{
		return;
	}

	XmlNodeRef modNode = node->newChild("TexMod");
	if (modNode)
	{
		// Modificators
		float f;
		uint16 s;
		uint8 c;

		modNode->setAttr("TexMod_RotateType", pTextureModifier.m_eRotType);
		modNode->setAttr("TexMod_TexGenType", pTextureModifier.m_eTGType);
		modNode->setAttr("TexMod_bTexGenProjected", pTextureModifier.m_bTexGenProjected);

		for (int baseu = 'U', u = baseu; u <= 'W'; u++)
		{
			char RT[] = "Rotate?";
			RT[6] = u;

			if ((s = pTextureModifier.m_Rot[u - baseu]) != defaultTexMod.m_Rot[u - baseu]) modNode->setAttr(RT, Word2Degr(s));

			char RR[] = "TexMod_?RotateRate";
			RR[7] = u;
			char RP[] = "TexMod_?RotatePhase";
			RP[7] = u;
			char RA[] = "TexMod_?RotateAmplitude";
			RA[7] = u;
			char RC[] = "TexMod_?RotateCenter";
			RC[7] = u;

			if ((s = pTextureModifier.m_RotOscRate[u - baseu]) != defaultTexMod.m_RotOscRate[u - baseu]) modNode->setAttr(RR, Word2Degr(s));
			if ((s = pTextureModifier.m_RotOscPhase[u - baseu]) != defaultTexMod.m_RotOscPhase[u - baseu]) modNode->setAttr(RP, Word2Degr(s));
			if ((s = pTextureModifier.m_RotOscAmplitude[u - baseu]) != defaultTexMod.m_RotOscAmplitude[u - baseu]) modNode->setAttr(RA, Word2Degr(s));
			if ((f = pTextureModifier.m_RotOscCenter[u - baseu]) != defaultTexMod.m_RotOscCenter[u - baseu]) modNode->setAttr(RC, f);

			if (u > 'V')
				continue;

			char TL[] = "Tile?";
			TL[4] = u;
			char OF[] = "Offset?";
			OF[6] = u;

			if ((f = pTextureModifier.m_Tiling[u - baseu]) != defaultTexMod.m_Tiling[u - baseu]) modNode->setAttr(TL, f);
			if ((f = pTextureModifier.m_Offs[u - baseu]) != defaultTexMod.m_Offs[u - baseu]) modNode->setAttr(OF, f);

			char OT[] = "TexMod_?OscillatorType";
			OT[7] = u;
			char OR[] = "TexMod_?OscillatorRate";
			OR[7] = u;
			char OP[] = "TexMod_?OscillatorPhase";
			OP[7] = u;
			char OA[] = "TexMod_?OscillatorAmplitude";
			OA[7] = u;

			if ((c = pTextureModifier.m_eMoveType[u - baseu]) != defaultTexMod.m_eMoveType[u - baseu]) modNode->setAttr(OT, c);
			if ((f = pTextureModifier.m_OscRate[u - baseu]) != defaultTexMod.m_OscRate[u - baseu]) modNode->setAttr(OR, f);
			if ((f = pTextureModifier.m_OscPhase[u - baseu]) != defaultTexMod.m_OscPhase[u - baseu]) modNode->setAttr(OP, f);
			if ((f = pTextureModifier.m_OscAmplitude[u - baseu]) != defaultTexMod.m_OscAmplitude[u - baseu]) modNode->setAttr(OA, f);
		}
	}
}

//////////////////////////////////////////////////////////////////////////
EEfResTextures MaterialHelpers::FindSCTexSlotPair(const char* slotName, const char* shaderName) const
{
	if (slotName)
	{
		if (!strcmp(shaderName,"LayerBlend"))
		{
			for (int i = 0; s_SCTexSlotPairsLB[i].scSlotName; i++)
			{
				if (stricmp(s_SCTexSlotPairsLB[i].scSlotName, slotName) == 0)
				{
					return s_SCTexSlotPairsLB[i].nativeSlot;
				}
			}
			return EFTT_UNKNOWN;
		}
		for (int i = 0; s_SCTexSlotPairs[i].scSlotName; i++)
		{
			if (stricmp(s_SCTexSlotPairs[i].scSlotName, slotName) == 0)
			{
				return s_SCTexSlotPairs[i].nativeSlot;
			}
		}
	}
	return EFTT_UNKNOWN;
}

//////////////////////////////////////////////////////////////////////////
EEfResTextures MaterialHelpers::FindSCTexSlotMatRefPairDiffuse(int matRefSlot) const
{
	for (int i = 0; i < 8; i++)
	{
		if (s_SCTexSlotMatReferencesPairs[i].matRefSlot == matRefSlot)
		{
			return s_SCTexSlotMatReferencesPairs[i].layerTexSlotDiffuse;
		}
	}
	return EFTT_UNKNOWN;
}

//////////////////////////////////////////////////////////////////////////
EEfResTextures MaterialHelpers::FindSCTexSlotMatRefPairBumpmap(int matRefSlot) const
{
	for (int i = 0; i < 8; i++)
	{
		if (s_SCTexSlotMatReferencesPairs[i].matRefSlot == matRefSlot)
		{
			return s_SCTexSlotMatReferencesPairs[i].layerTexSlotBumpmap;
		}
	}
	return EFTT_UNKNOWN;
}

//////////////////////////////////////////////////////////////////////////
const char*    MaterialHelpers::SCGetMtlDiff(const XmlNodeRef& node) const
{
	return node->getAttr("Diffuse");
}

//////////////////////////////////////////////////////////////////////////
const char*    MaterialHelpers::SCGetMtlSpec(const XmlNodeRef& node) const
{
	return node->getAttr("Specular");
}

//////////////////////////////////////////////////////////////////////////
string  MaterialHelpers::SCGetMtlSmooth(const XmlNodeRef& node) const
{
	//return ((float.Parse(val)) / 255).ToString();
	string shininess(node->getAttr("Shininess"));
	float newValue = (float)(atof(shininess) / 255.0f);
	string  shininessNew(std::to_string(newValue).c_str());
	return shininessNew;
}

//////////////////////////////////////////////////////////////////////////
XmlNodeRef     MaterialHelpers::SCSetPublicParams(const XmlNodeRef& node) const
{
	XmlNodeRef newPublicParamsNode = GetISystem()->CreateXmlNode("PublicParams");
	if (node->findChild("PublicParams")==0)
	{
		return newPublicParamsNode;
	}

	//clone first
	const char* key(NULL);
	const char* value(NULL);
	int index=0;
	while (node->findChild("PublicParams")->getAttributeByIndex(index,&key,&value))
	{
		newPublicParamsNode->setAttr(key, value);
		index++;
	}

	//now add SC custom nodes
	XmlNodeRef matReferencesNode = node->findChild("MatReferences");
	if (!matReferencesNode)
	{
		return newPublicParamsNode;
	}

	IXmlParser* pXmlParser = GetISystem()->GetXmlUtils()->CreateXmlParser();

	for (int c = 0; c < matReferencesNode->getChildCount(); c++)
	{
		XmlNodeRef matRefNode = matReferencesNode->getChild(c);
		const char* layerFilePath = matRefNode->getAttr("File");
		int layerSlot = atoi(matRefNode->getAttr("Slot"));
		if (!layerFilePath)
		{
			continue;
		}

		XmlNodeRef mtlLayerNode = pXmlParser->ParseFile(layerFilePath, false);
		if (!mtlLayerNode)
		{
			continue;
		}
		if (layerSlot == 0)newPublicParamsNode->setAttr("Diff1",   SCGetMtlDiff(mtlLayerNode));
		if (layerSlot == 0)newPublicParamsNode->setAttr("Spec1",   SCGetMtlSpec(mtlLayerNode));
		if (layerSlot == 0)newPublicParamsNode->setAttr("Smooth1", SCGetMtlSmooth(mtlLayerNode));

		if (layerSlot == 1)newPublicParamsNode->setAttr("Diff2", SCGetMtlDiff(mtlLayerNode));
		if (layerSlot == 1)newPublicParamsNode->setAttr("Spec2", SCGetMtlSpec(mtlLayerNode));
		if (layerSlot == 1)newPublicParamsNode->setAttr("Smooth2", SCGetMtlSmooth(mtlLayerNode));

		if (layerSlot == 2)newPublicParamsNode->setAttr("Diff3", SCGetMtlDiff(mtlLayerNode));
		if (layerSlot == 2)newPublicParamsNode->setAttr("Spec3", SCGetMtlSpec(mtlLayerNode));
		if (layerSlot == 2)newPublicParamsNode->setAttr("Smooth3", SCGetMtlSmooth(mtlLayerNode));

		if (layerSlot == 3)newPublicParamsNode->setAttr("Diff4", SCGetMtlDiff(mtlLayerNode));
		if (layerSlot == 3)newPublicParamsNode->setAttr("Spec4", SCGetMtlSpec(mtlLayerNode));
		if (layerSlot == 3)newPublicParamsNode->setAttr("Smooth4", SCGetMtlSmooth(mtlLayerNode));

		// wear/tear layers
		if (layerSlot == 4)newPublicParamsNode->setAttr("WearDiff1", SCGetMtlDiff(mtlLayerNode));
		if (layerSlot == 4)newPublicParamsNode->setAttr("WearSpec1", SCGetMtlSpec(mtlLayerNode));

		if (layerSlot == 5)newPublicParamsNode->setAttr("WearDiff2", SCGetMtlDiff(mtlLayerNode));
		if (layerSlot == 5)newPublicParamsNode->setAttr("WearSpec2", SCGetMtlSpec(mtlLayerNode));

		if (layerSlot == 6)newPublicParamsNode->setAttr("WearDiff3", SCGetMtlDiff(mtlLayerNode));
		if (layerSlot == 6)newPublicParamsNode->setAttr("WearSpec3", SCGetMtlSpec(mtlLayerNode));

		if (layerSlot == 7)newPublicParamsNode->setAttr("WearDiff4", SCGetMtlDiff(mtlLayerNode));
		if (layerSlot == 7)newPublicParamsNode->setAttr("WearSpec4", SCGetMtlSpec(mtlLayerNode));
	}

	return newPublicParamsNode;
}

//////////////////////////////////////////////////////////////////////////
void MaterialHelpers::SetTexturesFromXml(SInputShaderResources& pShaderResources, const XmlNodeRef& node, const char* szBaseFileName) const
{
	XmlNodeRef texturesNode = node->findChild("Textures");
	if (!texturesNode)
	{
		return;
	}

	//SC Tex slots layout(Illum):
	//TexSlot1->Diffuse|EFTT_DIFFUSE
	//TexSlot2->Bumpmap|EFTT_NORMALS
	//TexSlot3->[1] Custom|EFTT_CUSTOM_SECONDARY
	//TexSlot4->Specular|EFTT_SPECULAR
	//TexSlot5->Environment|EFTT_ENV
	//TexSlot6->Detail|EFTT_DETAIL_OVERLAY
	//TexSlot7->
	//TexSlot8->
	//TexSlot9->Custom|EFTT_CUSTOM
	//TexSlot10->
	//TexSlot11->
	//TexSlot12->Opacity|EFTT_OPACITY
	//TexSlot13->Layer1 Bumpmap|EFTT_LAYER1_NORMALS(2nd detailmap)
	///////////////-
	
	for (int c = 0; c < texturesNode->getChildCount(); c++)
	{
		XmlNodeRef texNode = texturesNode->getChild(c);
		EEfResTextures texId;

		//const char* const szTexmapTemp = texNode->getAttr("Map");
		const char* const szTexmap = texNode->getAttr("Map");
		//const char* szTexmap;
		//SC data converting
		if (string(szTexmap).substr(0, 7) == "TexSlot")
		{ 
			texId = MaterialHelpers::FindSCTexSlotPair(szTexmap,"");
			if (texId == EFTT_UNKNOWN)
			{
				CryLogAlways("MaterialHelpers::SetTexturesFromXml() | Tex pair for name %s not found. | %s -> %s", szTexmap, szBaseFileName, node->getAttr("Name"));
			}
		}
		else if ((!strcmp(node->getAttr("Shader"),"LayerBlend"))&&(node->findChild("MatReferences")))
		{ 
			texId = MaterialHelpers::FindSCTexSlotPair(szTexmap, "LayerBlend");
			if (texId == EFTT_UNKNOWN)
			{
				CryLogAlways("MaterialHelpers::SetTexturesFromXml() | Tex pair for name %s not found. | %s -> %s", szTexmap, szBaseFileName, node->getAttr("Name"));
			}
		}
		else
		{
			texId = MaterialHelpers::FindTexSlot(szTexmap);
		}
		//////////////////-

		if (texId == EFTT_UNKNOWN)
		{
			continue;
		}

		// Correct texid found.

		const char* const szFile = texNode->getAttr("File");

		if (szFile[0] == '.' && (szFile[1] == '/' || szFile[1] == '\\') &&
		    szBaseFileName && szBaseFileName[0])
		{
			// Texture file location is relative to material file folder
			pShaderResources.m_Textures[texId].m_Name = PathUtil::Make(PathUtil::GetPathWithoutFilename(szBaseFileName), &szFile[2]);
		}
		else
		{
			pShaderResources.m_Textures[texId].m_Name = szFile;
		}

		texNode->getAttr("IsTileU", pShaderResources.m_Textures[texId].m_bUTile);
		texNode->getAttr("IsTileV", pShaderResources.m_Textures[texId].m_bVTile);

		{
			// Try to parse texture type from name or index (in this order).
			XmlString texTypeName;
			if (texNode->getAttr("TexType", texTypeName))
			{
				ETEX_Type texType = (ETEX_Type)GetTextureTypeFromName(texTypeName);
				if (texType == eTT_MaxTexType || !IsTextureTypeExposed(texType))
				{
					texType = (ETEX_Type)strtoul(texTypeName, NULL, 10);
					if (texType == eTT_1D)texType = eTT_2D; //special handling for sc - different ETEX_Type layout? | fix for invalid textype console spam
				}

				if (texType > eTT_MaxTexType || !IsTextureTypeExposed(texType))
				{
					CryLog("%s: Ignoring bad value '%s' for texture type.\n", __FUNCTION__, texTypeName.c_str());
				}
				else
				{
					pShaderResources.m_Textures[texId].m_Sampler.m_eTexType = texType;
				}
			}
		}

		int filter = pShaderResources.m_Textures[texId].m_Filter;
		if (texNode->getAttr("Filter", filter))
		{
			pShaderResources.m_Textures[texId].m_Filter = filter;
		}

		SetTexModFromXml(*pShaderResources.m_Textures[texId].AddModificator(), texNode);
	}

	//sc data
	//load MatReferences as layer blend tex slots
	XmlNodeRef matReferencesNode = node->findChild("MatReferences");
	if (!matReferencesNode)
	{
		return;
	}

	//inefficient to request xmlparser for each material, good enough for now until I implement proper layers system.
	IXmlParser* pXmlParser = GetISystem()->GetXmlUtils()->CreateXmlParser();

	for (int c = 0; c < matReferencesNode->getChildCount(); c++)
	{
		XmlNodeRef matRefNode = matReferencesNode->getChild(c);
		const char* layerFilePath = matRefNode->getAttr("File");
		int layerSlot = atoi(matRefNode->getAttr("Slot"));
		if (!layerFilePath)
		{
			continue;
		}

		XmlNodeRef mtlLayerNode = pXmlParser->ParseFile(layerFilePath, false);
		if (!mtlLayerNode)
		{
			continue;
		}

		//IMaterial *layerMtl = gEnv->p3DEngine->GetMaterialManager()->LoadMaterialFromXml(layerFilePath, mtlLayerNode);

		int offsetToLayersSlots = EFTT_LAYER1_DIFFUSE;

		XmlNodeRef layerMtlTexturesNode = mtlLayerNode->findChild("Textures");

		//EEfResTextures texIdDiffuse = EFTT_UNKNOWN;
		//EEfResTextures texIdBumpmap = EFTT_UNKNOWN;

		for (int i=0;i< layerMtlTexturesNode->getChildCount();i++)
		{
			XmlNodeRef texNode = layerMtlTexturesNode->getChild(i);
			const char* const szTexmap = texNode->getAttr("Map");
			const char* const szFile = texNode->getAttr("File");

			if (szTexmap)
			{
				EEfResTextures texId = EFTT_UNKNOWN;
				if (!strcmp(szTexmap, "Diffuse"))
				{
					texId = MaterialHelpers::FindSCTexSlotMatRefPairDiffuse(layerSlot);
				}
				if (!strcmp(szTexmap, "Bumpmap"))
				{
					texId = MaterialHelpers::FindSCTexSlotMatRefPairBumpmap(layerSlot);
				}
				if (texId == EFTT_UNKNOWN)
				{
					continue;
				}

				if (layerSlot == 0)SCGetMtlDiff(mtlLayerNode);

				pShaderResources.m_Textures[texId].m_Sampler.m_eTexType = eTT_2D;

				if (szFile[0] == '.' && (szFile[1] == '/' || szFile[1] == '\\') &&
					szBaseFileName && szBaseFileName[0])
				{
					// Texture file location is relative to material file folder
					pShaderResources.m_Textures[texId].m_Name = PathUtil::Make(PathUtil::GetPathWithoutFilename(szBaseFileName), &szFile[2]);
				}
				else
				{
					pShaderResources.m_Textures[texId].m_Name = szFile;
				}

				int filter = pShaderResources.m_Textures[texId].m_Filter;
				if (texNode->getAttr("Filter", filter))
				{
					pShaderResources.m_Textures[texId].m_Filter = filter;
				}

				SetTexModFromXml(*pShaderResources.m_Textures[texId].AddModificator(), texNode);
			}
		}
	}
}

//////////////////////////////////////////////////////////////////////////
static SInputShaderResourcesPtr s_defaultShaderResource;

void MaterialHelpers::SetXmlFromTextures(const SInputShaderResources& pShaderResources, XmlNodeRef& node, const char* szBaseFileName) const
{
	if (!s_defaultShaderResource)
	{
		s_defaultShaderResource = gEnv->pRenderer->EF_CreateInputShaderResource();
	}

	// Save texturing data.

	XmlNodeRef texturesNode = node->newChild("Textures");

	string basePath;
	if (szBaseFileName && szBaseFileName[0])
	{
		basePath = PathUtil::GetPathWithoutFilename(szBaseFileName);
	}

	for (EEfResTextures texId = EEfResTextures(0); texId < EFTT_MAX; texId = EEfResTextures(texId + 1))
	{
		if (!pShaderResources.m_Textures[texId].m_Name.empty())
		{
			XmlNodeRef texNode = texturesNode->newChild("Texture");

			//    texNode->setAttr("TexID",texId);
			texNode->setAttr("Map", MaterialHelpers::LookupTexName(texId));

			const string& filename = pShaderResources.m_Textures[texId].m_Name;

			if (!basePath.empty() &&
			    filename.length() > basePath.length() &&
			    filename[0] != '/' && filename[0] != '\\' &&
			    memcmp(basePath.data(), filename.data(), basePath.length()) == 0)
			{
				// Make texture file location relative to material file folder
				string localFilename;
				localFilename.reserve(2 + filename.length() - basePath.length());
				localFilename = "./";
				localFilename += filename.c_str() + basePath.length();
				texNode->setAttr("File", localFilename.c_str());
			}
			else
			{
				texNode->setAttr("File", filename.c_str());
			}

			if (pShaderResources.m_Textures[texId].m_Filter != s_defaultShaderResource->m_Textures[texId].m_Filter)
			{
				texNode->setAttr("Filter", pShaderResources.m_Textures[texId].m_Filter);
			}
			if (pShaderResources.m_Textures[texId].m_bUTile != s_defaultShaderResource->m_Textures[texId].m_bUTile)
			{
				texNode->setAttr("IsTileU", pShaderResources.m_Textures[texId].m_bUTile);
			}
			if (pShaderResources.m_Textures[texId].m_bVTile != s_defaultShaderResource->m_Textures[texId].m_bVTile)
			{
				texNode->setAttr("IsTileV", pShaderResources.m_Textures[texId].m_bVTile);
			}
			if (pShaderResources.m_Textures[texId].m_Sampler.m_eTexType != s_defaultShaderResource->m_Textures[texId].m_Sampler.m_eTexType)
			{
				CRY_ASSERT(IsTextureTypeExposed(pShaderResources.m_Textures[texId].m_Sampler.m_eTexType));
				texNode->setAttr("TexType", GetNameFromTextureType(pShaderResources.m_Textures[texId].m_Sampler.m_eTexType));
			}

			//////////////////////////////////////////////////////////////////////////
			// Save texture modificators Modificators
			//////////////////////////////////////////////////////////////////////////
			SetXmlFromTexMod(*pShaderResources.m_Textures[texId].GetModificator(), texNode);
		}
	}
}

//////////////////////////////////////////////////////////////////////////
void MaterialHelpers::SetVertexDeformFromXml(SInputShaderResources& pShaderResources, const XmlNodeRef& node) const
{
	if (!s_defaultShaderResource)
		s_defaultShaderResource = gEnv->pRenderer->EF_CreateInputShaderResource();

	if (s_defaultShaderResource->m_DeformInfo.m_eType != pShaderResources.m_DeformInfo.m_eType)
	{
		node->setAttr("vertModifType", pShaderResources.m_DeformInfo.m_eType);
	}

	XmlNodeRef deformNode = node->findChild("VertexDeform");
	if (deformNode)
	{
		int deform_type = eDT_Unknown;
		deformNode->getAttr("Type", deform_type);
		pShaderResources.m_DeformInfo.m_eType = (EDeformType)deform_type;
		deformNode->getAttr("DividerX", pShaderResources.m_DeformInfo.m_fDividerX);
		deformNode->getAttr("DividerY", pShaderResources.m_DeformInfo.m_fDividerY);
		deformNode->getAttr("DividerZ", pShaderResources.m_DeformInfo.m_fDividerZ);
		deformNode->getAttr("DividerW", pShaderResources.m_DeformInfo.m_fDividerW);
		deformNode->getAttr("NoiseScale", pShaderResources.m_DeformInfo.m_vNoiseScale);

		XmlNodeRef waveX = deformNode->findChild("WaveX");
		if (waveX)
		{
			int type = eWF_None;
			waveX->getAttr("Type", type);
			pShaderResources.m_DeformInfo.m_WaveX.m_eWFType = (EWaveForm)type;
			waveX->getAttr("Amp", pShaderResources.m_DeformInfo.m_WaveX.m_Amp);
			waveX->getAttr("Level", pShaderResources.m_DeformInfo.m_WaveX.m_Level);
			waveX->getAttr("Phase", pShaderResources.m_DeformInfo.m_WaveX.m_Phase);
			waveX->getAttr("Freq", pShaderResources.m_DeformInfo.m_WaveX.m_Freq);
		}

		XmlNodeRef waveY = deformNode->findChild("WaveY");
		if (waveY)
		{
			int type = eWF_None;
			waveY->getAttr("Type", type);
			pShaderResources.m_DeformInfo.m_WaveY.m_eWFType = (EWaveForm)type;
			waveY->getAttr("Amp", pShaderResources.m_DeformInfo.m_WaveY.m_Amp);
			waveY->getAttr("Level", pShaderResources.m_DeformInfo.m_WaveY.m_Level);
			waveY->getAttr("Phase", pShaderResources.m_DeformInfo.m_WaveY.m_Phase);
			waveY->getAttr("Freq", pShaderResources.m_DeformInfo.m_WaveY.m_Freq);
		}

		XmlNodeRef waveZ = deformNode->findChild("WaveZ");
		if (waveZ)
		{
			int type = eWF_None;
			waveZ->getAttr("Type", type);
			pShaderResources.m_DeformInfo.m_WaveZ.m_eWFType = (EWaveForm)type;
			waveZ->getAttr("Amp", pShaderResources.m_DeformInfo.m_WaveZ.m_Amp);
			waveZ->getAttr("Level", pShaderResources.m_DeformInfo.m_WaveZ.m_Level);
			waveZ->getAttr("Phase", pShaderResources.m_DeformInfo.m_WaveZ.m_Phase);
			waveZ->getAttr("Freq", pShaderResources.m_DeformInfo.m_WaveZ.m_Freq);
		}

		XmlNodeRef waveW = deformNode->findChild("WaveW");
		if (waveW)
		{
			int type = eWF_None;
			waveW->getAttr("Type", type);
			pShaderResources.m_DeformInfo.m_WaveW.m_eWFType = (EWaveForm)type;
			waveW->getAttr("Amp", pShaderResources.m_DeformInfo.m_WaveW.m_Amp);
			waveW->getAttr("Level", pShaderResources.m_DeformInfo.m_WaveW.m_Level);
			waveW->getAttr("Phase", pShaderResources.m_DeformInfo.m_WaveW.m_Phase);
			waveW->getAttr("Freq", pShaderResources.m_DeformInfo.m_WaveW.m_Freq);
		}
	}
}

//////////////////////////////////////////////////////////////////////////
void MaterialHelpers::SetXmlFromVertexDeform(const SInputShaderResources& pShaderResources, XmlNodeRef& node) const
{
	int vertModif = pShaderResources.m_DeformInfo.m_eType;
	node->setAttr("vertModifType", vertModif);

	if (pShaderResources.m_DeformInfo.m_eType != eDT_Unknown)
	{
		XmlNodeRef deformNode = node->newChild("VertexDeform");

		deformNode->setAttr("Type", pShaderResources.m_DeformInfo.m_eType);
		deformNode->setAttr("DividerX", pShaderResources.m_DeformInfo.m_fDividerX);
		deformNode->setAttr("DividerY", pShaderResources.m_DeformInfo.m_fDividerY);
		deformNode->setAttr("NoiseScale", pShaderResources.m_DeformInfo.m_vNoiseScale);

		if (pShaderResources.m_DeformInfo.m_WaveX.m_eWFType != eWF_None)
		{
			XmlNodeRef waveX = deformNode->newChild("WaveX");
			waveX->setAttr("Type", pShaderResources.m_DeformInfo.m_WaveX.m_eWFType);
			waveX->setAttr("Amp", pShaderResources.m_DeformInfo.m_WaveX.m_Amp);
			waveX->setAttr("Level", pShaderResources.m_DeformInfo.m_WaveX.m_Level);
			waveX->setAttr("Phase", pShaderResources.m_DeformInfo.m_WaveX.m_Phase);
			waveX->setAttr("Freq", pShaderResources.m_DeformInfo.m_WaveX.m_Freq);
		}

		if (pShaderResources.m_DeformInfo.m_WaveY.m_eWFType != eWF_None)
		{
			XmlNodeRef waveY = deformNode->newChild("WaveY");
			waveY->setAttr("Type", pShaderResources.m_DeformInfo.m_WaveY.m_eWFType);
			waveY->setAttr("Amp", pShaderResources.m_DeformInfo.m_WaveY.m_Amp);
			waveY->setAttr("Level", pShaderResources.m_DeformInfo.m_WaveY.m_Level);
			waveY->setAttr("Phase", pShaderResources.m_DeformInfo.m_WaveY.m_Phase);
			waveY->setAttr("Freq", pShaderResources.m_DeformInfo.m_WaveY.m_Freq);
		}
	}
}

//////////////////////////////////////////////////////////////////////////
void MaterialHelpers::SetDetailDecalFromXml(SInputShaderResources& pShaderResources, const XmlNodeRef& node) const
{
	const XmlNodeRef detailDecalNode = node->findChild("DetailDecal");
	SDetailDecalInfo* pDetailDecalInfo = &pShaderResources.m_DetailDecalInfo;

	pShaderResources.m_DetailDecalInfo.Reset();
	if (detailDecalNode)
	{
		detailDecalNode->getAttr("Opacity", pDetailDecalInfo->nBlending);
		detailDecalNode->getAttr("SSAOAmount", pDetailDecalInfo->nSSAOAmount);
		detailDecalNode->getAttr("topTileU", pDetailDecalInfo->vTileOffs[0].x);
		detailDecalNode->getAttr("topTileV", pDetailDecalInfo->vTileOffs[0].y);
		detailDecalNode->getAttr("topOffsV", pDetailDecalInfo->vTileOffs[0].z);
		detailDecalNode->getAttr("topOffsU", pDetailDecalInfo->vTileOffs[0].w);
		detailDecalNode->getAttr("topRotation", pDetailDecalInfo->nRotation[0]);
		detailDecalNode->getAttr("topDeformation", pDetailDecalInfo->nDeformation[0]);
		detailDecalNode->getAttr("topSortOffset", pDetailDecalInfo->nThreshold[0]);
		detailDecalNode->getAttr("bottomTileU", pDetailDecalInfo->vTileOffs[1].x);
		detailDecalNode->getAttr("bottomTileV", pDetailDecalInfo->vTileOffs[1].y);
		detailDecalNode->getAttr("bottomOffsV", pDetailDecalInfo->vTileOffs[1].z);
		detailDecalNode->getAttr("bottomOffsU", pDetailDecalInfo->vTileOffs[1].w);
		detailDecalNode->getAttr("bottomRotation", pDetailDecalInfo->nRotation[1]);
		detailDecalNode->getAttr("bottomDeformation", pDetailDecalInfo->nDeformation[1]);
		detailDecalNode->getAttr("bottomSortOffset", pDetailDecalInfo->nThreshold[1]);
	}
}

void MaterialHelpers::SetXmlFromDetailDecal(const SInputShaderResources& pShaderResources, XmlNodeRef& node) const
{
	XmlNodeRef detailDecalNode = node->newChild("DetailDecal");
	const SDetailDecalInfo* pDetailDecalInfo = &pShaderResources.m_DetailDecalInfo;

	detailDecalNode->setAttr("Opacity", pDetailDecalInfo->nBlending);
	detailDecalNode->setAttr("SSAOAmount", pDetailDecalInfo->nSSAOAmount);
	detailDecalNode->setAttr("topTileU", pDetailDecalInfo->vTileOffs[0].x);
	detailDecalNode->setAttr("topTileV", pDetailDecalInfo->vTileOffs[0].y);
	detailDecalNode->setAttr("topOffsV", pDetailDecalInfo->vTileOffs[0].z);
	detailDecalNode->setAttr("topOffsU", pDetailDecalInfo->vTileOffs[0].w);
	detailDecalNode->setAttr("topRotation", pDetailDecalInfo->nRotation[0]);
	detailDecalNode->setAttr("topDeformation", pDetailDecalInfo->nDeformation[0]);
	detailDecalNode->setAttr("topSortOffset", pDetailDecalInfo->nThreshold[0]);
	detailDecalNode->setAttr("bottomTileU", pDetailDecalInfo->vTileOffs[1].x);
	detailDecalNode->setAttr("bottomTileV", pDetailDecalInfo->vTileOffs[1].y);
	detailDecalNode->setAttr("bottomOffsV", pDetailDecalInfo->vTileOffs[1].z);
	detailDecalNode->setAttr("bottomOffsU", pDetailDecalInfo->vTileOffs[1].w);
	detailDecalNode->setAttr("bottomRotation", pDetailDecalInfo->nRotation[1]);
	detailDecalNode->setAttr("bottomDeformation", pDetailDecalInfo->nDeformation[1]);
	detailDecalNode->setAttr("bottomSortOffset", pDetailDecalInfo->nThreshold[1]);
}

//////////////////////////////////////////////////////////////////////////
void MaterialHelpers::SetLightingFromXml(SInputShaderResources& pShaderResources, const XmlNodeRef& node) const
{
	// Load lighting data.
	Vec3 vColor;
	Vec4 vColor4;
	if (node->getAttr("Diffuse", vColor))
	{
		pShaderResources.m_LMaterial.m_Diffuse = ColorF(vColor);
	}
	if (node->getAttr("Specular", vColor))
	{
		pShaderResources.m_LMaterial.m_Specular = ColorF(vColor);
	}
	if (node->getAttr("Emittance", vColor4))
	{
		pShaderResources.m_LMaterial.m_Emittance = ColorF(vColor4.x, vColor4.y, vColor4.z, vColor4.w);
	}

	node->getAttr("Shininess", pShaderResources.m_LMaterial.m_Smoothness);
	node->getAttr("Opacity", pShaderResources.m_LMaterial.m_Opacity);
	node->getAttr("AlphaTest", pShaderResources.m_AlphaRef);
	node->getAttr("FurAmount", pShaderResources.m_FurAmount);
	node->getAttr("VoxelCoverage", pShaderResources.m_VoxelCoverage);
	node->getAttr("CloakAmount", pShaderResources.m_CloakAmount);
	node->getAttr("HeatAmountScaled", pShaderResources.m_HeatAmount);
}

//////////////////////////////////////////////////////////////////////////
void MaterialHelpers::SetXmlFromLighting(const SInputShaderResources& pShaderResources, XmlNodeRef& node) const
{
	if (!s_defaultShaderResource)
		s_defaultShaderResource = gEnv->pRenderer->EF_CreateInputShaderResource();

	// Save ligthing data.
	if (s_defaultShaderResource->m_LMaterial.m_Diffuse != pShaderResources.m_LMaterial.m_Diffuse)
		node->setAttr("Diffuse", pShaderResources.m_LMaterial.m_Diffuse.toVec3());
	if (s_defaultShaderResource->m_LMaterial.m_Specular != pShaderResources.m_LMaterial.m_Specular)
		node->setAttr("Specular", pShaderResources.m_LMaterial.m_Specular.toVec3());
	if (s_defaultShaderResource->m_LMaterial.m_Emittance != pShaderResources.m_LMaterial.m_Emittance)
		node->setAttr("Emittance", pShaderResources.m_LMaterial.m_Emittance.toVec4());

	if (s_defaultShaderResource->m_LMaterial.m_Opacity != pShaderResources.m_LMaterial.m_Opacity)
		node->setAttr("Opacity", pShaderResources.m_LMaterial.m_Opacity);
	if (s_defaultShaderResource->m_LMaterial.m_Smoothness != pShaderResources.m_LMaterial.m_Smoothness)
		node->setAttr("Shininess", pShaderResources.m_LMaterial.m_Smoothness);

	if (s_defaultShaderResource->m_AlphaRef != pShaderResources.m_AlphaRef)
		node->setAttr("AlphaTest", pShaderResources.m_AlphaRef);
	if (s_defaultShaderResource->m_FurAmount != pShaderResources.m_FurAmount)
		node->setAttr("FurAmount", pShaderResources.m_FurAmount);
	if (s_defaultShaderResource->m_VoxelCoverage != pShaderResources.m_VoxelCoverage)
		node->setAttr("VoxelCoverage", pShaderResources.m_VoxelCoverage);
	if (s_defaultShaderResource->m_CloakAmount != pShaderResources.m_CloakAmount)
		node->setAttr("CloakAmount", pShaderResources.m_CloakAmount);
	if (s_defaultShaderResource->m_HeatAmount != pShaderResources.m_HeatAmount)
		node->setAttr("HeatAmountScaled", pShaderResources.m_HeatAmount);
}

//////////////////////////////////////////////////////////////////////////
void MaterialHelpers::SetShaderParamsFromXml(SInputShaderResources& pShaderResources, const XmlNodeRef& node) const
{
	int nA = node->getNumAttributes();
	if (!nA)
		return;

	for (int i = 0; i < nA; i++)
	{
		const char* key = NULL, * val = NULL;
		node->getAttributeByIndex(i, &key, &val);

		// try to set existing param first
		bool bFound = false;

		for (int i = 0; i < pShaderResources.m_ShaderParams.size(); i++)
		{
			SShaderParam* pParam = &pShaderResources.m_ShaderParams[i];

			if (strcmp(pParam->m_Name, key) == 0)
			{
				bFound = true;

				switch (pParam->m_Type)
				{
				case eType_BYTE:
					node->getAttr(key, pParam->m_Value.m_Byte);
					break;
				case eType_SHORT:
					node->getAttr(key, pParam->m_Value.m_Short);
					break;
				case eType_INT:
					node->getAttr(key, pParam->m_Value.m_Int);
					break;
				case eType_FLOAT:
					node->getAttr(key, pParam->m_Value.m_Float);
					break;
				case eType_FCOLOR:
					{
						Vec3 vValue;
						node->getAttr(key, vValue);

						pParam->m_Value.m_Color[0] = vValue.x;
						pParam->m_Value.m_Color[1] = vValue.y;
						pParam->m_Value.m_Color[2] = vValue.z;
					}
					break;
				case eType_VECTOR:
					{
						Vec3 vValue;
						node->getAttr(key, vValue);

						pParam->m_Value.m_Vector[0] = vValue.x;
						pParam->m_Value.m_Vector[1] = vValue.y;
						pParam->m_Value.m_Vector[2] = vValue.z;
					}
					break;
				default:
					break;
				}
			}
		}

		if (!bFound)
		{
			assert(val && key);

			SShaderParam Param;
			cry_strcpy(Param.m_Name, key);
			Param.m_Value.m_Color[0] = Param.m_Value.m_Color[1] = Param.m_Value.m_Color[2] = Param.m_Value.m_Color[3] = 0;
#if defined(_DEBUG) && !defined(IGNORE_ASSERTS) // this is used in SkyLightNishita.cpp to redefine asserts
			int res = sscanf(val, "%f,%f,%f,%f", &Param.m_Value.m_Color[0], &Param.m_Value.m_Color[1], &Param.m_Value.m_Color[2], &Param.m_Value.m_Color[3]);
			assert(res);
#else
			sscanf(val, "%f,%f,%f,%f", &Param.m_Value.m_Color[0], &Param.m_Value.m_Color[1], &Param.m_Value.m_Color[2], &Param.m_Value.m_Color[3]);
#endif

			pShaderResources.m_ShaderParams.push_back(Param);
		}
	}
}

//////////////////////////////////////////////////////////////////////////
void MaterialHelpers::SetXmlFromShaderParams(const SInputShaderResources& pShaderResources, XmlNodeRef& node) const
{
	for (int i = 0; i < pShaderResources.m_ShaderParams.size(); i++)
	{
		const SShaderParam* pParam = &pShaderResources.m_ShaderParams[i];
		switch (pParam->m_Type)
		{
		case eType_BYTE:
			node->setAttr(pParam->m_Name, (int)pParam->m_Value.m_Byte);
			break;
		case eType_SHORT:
			node->setAttr(pParam->m_Name, (int)pParam->m_Value.m_Short);
			break;
		case eType_INT:
			node->setAttr(pParam->m_Name, (int)pParam->m_Value.m_Int);
			break;
		case eType_FLOAT:
			node->setAttr(pParam->m_Name, (float)pParam->m_Value.m_Float);
			break;
		case eType_FCOLOR:
			node->setAttr(pParam->m_Name, Vec3(pParam->m_Value.m_Color[0], pParam->m_Value.m_Color[1], pParam->m_Value.m_Color[2]));
			break;
		case eType_VECTOR:
			node->setAttr(pParam->m_Name, Vec3(pParam->m_Value.m_Vector[0], pParam->m_Value.m_Vector[1], pParam->m_Value.m_Vector[2]));
			break;
		default:
			break;
		}
	}
}

//////////////////////////////////////////////////////////////////////////
void MaterialHelpers::MigrateXmlLegacyData(SInputShaderResources& pShaderResources, const XmlNodeRef& node) const
{
	float glowAmount;

	// Migrate glow from 3.8.3 to emittance
	if (node->getAttr("GlowAmount", glowAmount) && glowAmount > 0)
	{
		if (pShaderResources.m_Textures[EFTT_DIFFUSE].m_Sampler.m_eTexType == eTT_2D)
			pShaderResources.m_Textures[EFTT_EMITTANCE].m_Name = pShaderResources.m_Textures[EFTT_DIFFUSE].m_Name;

		const float legacyHDRDynMult = 2.0f;
		const float legacyIntensityScale = 10.0f;  // Legacy scale factor 10000 divided by 1000 for kilonits
		pShaderResources.m_LMaterial.m_Emittance.a = powf(glowAmount * legacyHDRDynMult, legacyHDRDynMult) * legacyIntensityScale;
	}
}

//////////////////////////////////////////////////////////////////////////
void MaterialHelpers::ConvertSCData(SInputShaderResources& pShaderResources, const XmlNodeRef& node) const
{
	XmlNodeRef texturesNode = node->findChild("Textures");
	if (!texturesNode)
	{
		return;
	}

	//Convert TexSlotXX to native names.
	for (int c = 0; c < texturesNode->getChildCount(); c++)
	{
		XmlNodeRef texNode = texturesNode->getChild(c);

		const char* const szTexmap = texNode->getAttr("Map");
		if (string(szTexmap).substr(0,7) != "TexSlot")
		{
			//its not sc asset
			break;
		}

		int texNumber;

		//extract tex number from map attr, TexSlotXX
		texNumber = atoi(string(szTexmap).substr(7).c_str());
		texNumber--;//in xml TexSlots start at 1; 1=diffuse slot in sc xml

		const char* nativeTexName = FindTexName((EEfResTextures)texNumber);
		if (nativeTexName == NULL)
		{
			CryLogAlways("MaterialHelpers::ConvertSCData() | Tex name on slot %d not found.", texNumber);
			continue;
		}

		texNode->setAttr("Map", nativeTexName);
	}
}

//////////////////////////////////////////////////////////////////////////

const char* MaterialHelpers::GetNameFromTextureType(uint8 texType) const
{
	CRY_ASSERT(texType < eTT_MaxTexType);
	switch (texType)
	{
	case eTT_1D:
		return "1D";
	case eTT_2D:
		return "2D";
	case eTT_3D:
		return "3D";
	case eTT_Cube:
		return "Cube-Map";
	case eTT_CubeArray:
		return "Cube-Array";
	case eTT_Dyn2D:
		return "Dynamic 2D-Map";
	case eTT_User:
		return "From User Params";
	case eTT_NearestCube:
		return "Nearest Cube-Map probe for alpha blended";
	case eTT_2DArray:
		return "2D-Array";
	case eTT_2DMS:
		return "2D-MS";
	case eTT_Auto2D:
		return "Auto 2D-Map";
	default:
		CRY_ASSERT(0 && "unknown texture type");
		return nullptr;
	}
}

uint8 MaterialHelpers::GetTextureTypeFromName(const char* szName) const
{
	CRY_ASSERT(szName);
	for (int i = 0; i < eTT_MaxTexType; ++i)
	{
		if (!strcmp(szName, GetNameFromTextureType(i)))
		{
			return i;
		}
	}
	return eTT_MaxTexType;
}

bool MaterialHelpers::IsTextureTypeExposed(uint8 texType) const
{
	CRY_ASSERT(texType < eTT_MaxTexType);
	switch (texType)
	{
	case eTT_2D:
	case eTT_Cube:
	case eTT_NearestCube:
	case eTT_Auto2D:
	case eTT_Dyn2D:
	case eTT_User:
		return true;
	default:
		return false;
	}
}
