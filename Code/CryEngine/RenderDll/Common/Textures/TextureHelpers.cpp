// Copyright 2001-2018 Crytek GmbH / Crytek Group. All rights reserved.

#include "StdAfx.h"
#include "TextureHelpers.h"
#include <CryString/StringUtils.h>

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
	int8           priority;
	CTexture**     def;
	CTexture**     neutral;
	const char*    suffix;
	const char*    semantic;
	const char*    semanticOld;
}
s_TexSlotSemantics[] =
{
	// NOTE: must be in order with filled holes to allow direct lookup
	{ EFTT_DIFFUSE,          4, &CRendererResources::s_ptexNoTexture, &CRendererResources::s_ptexWhite,    "_diff",   "TM_Diffuse",         "$TEX_Diffuse"         },
	{ EFTT_NORMALS,          2, &CRendererResources::s_ptexFlatBump,  &CRendererResources::s_ptexFlatBump, "_ddn",    "TM_Normals",         "$TEX_Normals"         },
	{ EFTT_SPECULAR,         1, &CRendererResources::s_ptexWhite,     &CRendererResources::s_ptexWhite,    "_spec",   "TM_Specular",        "$TEX_Specular"        },
	{ EFTT_ENV,              0, &CRendererResources::s_ptexWhite,     &CRendererResources::s_ptexWhite,    "_cm",     "TM_Env",             "$TEX_EnvCM"           },
	{ EFTT_DETAIL_OVERLAY,   3, &CRendererResources::s_ptexGray,      &CRendererResources::s_ptexWhite,    "_detail", "TM_Detail",          "$TEX_Detail"          },
	{ EFTT_SMOOTHNESS,       2, &CRendererResources::s_ptexWhite,     &CRendererResources::s_ptexWhite,    "_ddna",   "TM_Smoothness",      "$TEX_Smoothness"      },
	{ EFTT_HEIGHT,           2, &CRendererResources::s_ptexWhite,     &CRendererResources::s_ptexWhite,    "_displ",  "TM_Height",          "$TEX_Height"          },
	{ EFTT_DECAL_OVERLAY,    3, &CRendererResources::s_ptexGray,      &CRendererResources::s_ptexWhite,    "",        "TM_DecalOverlay",    "$TEX_DecalOverlay"    },
	{ EFTT_SUBSURFACE,       3, &CRendererResources::s_ptexWhite,     &CRendererResources::s_ptexWhite,    "_sss",    "TM_SubSurface",      "$TEX_SubSurface"      },
	{ EFTT_CUSTOM,           4, &CRendererResources::s_ptexWhite,     &CRendererResources::s_ptexWhite,    "",        "TM_Custom",          "$TEX_Custom"          },
	{ EFTT_CUSTOM_SECONDARY, 2, &CRendererResources::s_ptexFlatBump,  &CRendererResources::s_ptexFlatBump, "",        "TM_CustomSecondary", "$TEX_CustomSecondary" },
	{ EFTT_OPACITY,          4, &CRendererResources::s_ptexWhite,     &CRendererResources::s_ptexWhite,    "",        "TM_Opacity",         "$TEX_Opacity"         },
	{ EFTT_TRANSLUCENCY,     2, &CRendererResources::s_ptexWhite,     &CRendererResources::s_ptexWhite,    "_trans",  "TM_Translucency",    "$TEX_Translucency"    },
	{ EFTT_EMITTANCE,        1, &CRendererResources::s_ptexWhite,     &CRendererResources::s_ptexWhite,    "_em",     "TM_Emittance",       "$TEX_Emittance"       },

	{ EFTT_LAYER1_DIFFUSE,   2, &CRendererResources::s_ptexWhite,     &CRendererResources::s_ptexWhite,    "",        "TM_Layer1_Diffuse",  "$TEX_Layer1_Diffuse"  },
	{ EFTT_LAYER1_NORMALS,   2, &CRendererResources::s_ptexWhite,     &CRendererResources::s_ptexWhite,    "",        "TM_Layer1_Normals",  "$TEX_Layer1_Normals"  },
	{ EFTT_LAYER2_DIFFUSE,   2, &CRendererResources::s_ptexWhite,     &CRendererResources::s_ptexWhite,    "",        "TM_Layer2_Diffuse",  "$TEX_Layer2_Diffuse" },
	{ EFTT_LAYER2_NORMALS,   2, &CRendererResources::s_ptexWhite,     &CRendererResources::s_ptexWhite,    "",        "TM_Layer2_Normals",  "$TEX_Layer2_Normals" },
	{ EFTT_LAYER3_DIFFUSE,   2, &CRendererResources::s_ptexWhite,     &CRendererResources::s_ptexWhite,    "",        "TM_Layer3_Diffuse",  "$TEX_Layer3_Diffuse" },
	{ EFTT_LAYER3_NORMALS,   2, &CRendererResources::s_ptexWhite,     &CRendererResources::s_ptexWhite,    "",        "TM_Layer3_Normals",  "$TEX_Layer3_Normals" },
	{ EFTT_LAYER4_DIFFUSE,   2, &CRendererResources::s_ptexWhite,     &CRendererResources::s_ptexWhite,    "",        "TM_Layer4_Diffuse",  "$TEX_Layer4_Diffuse" },
	{ EFTT_LAYER4_NORMALS,   2, &CRendererResources::s_ptexWhite,     &CRendererResources::s_ptexWhite,    "",        "TM_Layer4_Normals",  "$TEX_Layer4_Normals" },

	{ EFTT_LAYER1_WT_DIFFUSE,   2, &CRendererResources::s_ptexWhite,     &CRendererResources::s_ptexWhite,    "",        "TM_Layer1_WT_Diffuse",  "$TEX_Layer1_WT_Diffuse" },
	{ EFTT_LAYER1_WT_NORMALS,   2, &CRendererResources::s_ptexWhite,     &CRendererResources::s_ptexWhite,    "",        "TM_Layer1_WT_Normals",  "$TEX_Layer1_WT_Normals" },
	{ EFTT_LAYER2_WT_DIFFUSE,   2, &CRendererResources::s_ptexWhite,     &CRendererResources::s_ptexWhite,    "",        "TM_Layer2_WT_Diffuse",  "$TEX_Layer2_WT_Diffuse" },
	{ EFTT_LAYER2_WT_NORMALS,   2, &CRendererResources::s_ptexWhite,     &CRendererResources::s_ptexWhite,    "",        "TM_Layer2_WT_Normals",  "$TEX_Layer2_WT_Normals" },
	{ EFTT_LAYER3_WT_DIFFUSE,   2, &CRendererResources::s_ptexWhite,     &CRendererResources::s_ptexWhite,    "",        "TM_Layer3_WT_Diffuse",  "$TEX_Layer3_WT_Diffuse" },
	{ EFTT_LAYER3_WT_NORMALS,   2, &CRendererResources::s_ptexWhite,     &CRendererResources::s_ptexWhite,    "",        "TM_Layer3_WT_Normals",  "$TEX_Layer3_WT_Normals" },
	{ EFTT_LAYER4_WT_DIFFUSE,   2, &CRendererResources::s_ptexWhite,     &CRendererResources::s_ptexWhite,    "",        "TM_Layer4_WT_Diffuse",  "$TEX_Layer4_WT_Diffuse" },
	{ EFTT_LAYER4_WT_NORMALS,   2, &CRendererResources::s_ptexWhite,     &CRendererResources::s_ptexWhite,    "",        "TM_Layer4_WT_Normals",  "$TEX_Layer4_WT_Normals" },

	{ EFTT_LAYER1_SMOOTHNESS,   2, &CRendererResources::s_ptexWhite,     &CRendererResources::s_ptexWhite,    "",        "TM_Layer1_Smoothness",  "$TEX_Layer1_Smoothness" },
	{ EFTT_LAYER2_SMOOTHNESS,   2, &CRendererResources::s_ptexWhite,     &CRendererResources::s_ptexWhite,    "",        "TM_Layer2_Smoothness",  "$TEX_Layer2_Smoothness" },
	{ EFTT_LAYER3_SMOOTHNESS,   2, &CRendererResources::s_ptexWhite,     &CRendererResources::s_ptexWhite,    "",        "TM_Layer3_Smoothness",  "$TEX_Layer3_Smoothness" },
	{ EFTT_LAYER4_SMOOTHNESS,   2, &CRendererResources::s_ptexWhite,     &CRendererResources::s_ptexWhite,    "",        "TM_Layer4_Smoothness",  "$TEX_Layer4_Smoothness" },

	{ EFTT_LAYER1_WT_SMOOTHNESS,   2, &CRendererResources::s_ptexWhite,     &CRendererResources::s_ptexWhite,    "",        "TM_Layer1_WT_Smoothness",  "$TEX_Layer1_WT_Smoothness" },
	{ EFTT_LAYER2_WT_SMOOTHNESS,   2, &CRendererResources::s_ptexWhite,     &CRendererResources::s_ptexWhite,    "",        "TM_Layer2_WT_Smoothness",  "$TEX_Layer2_WT_Smoothness" },
	{ EFTT_LAYER3_WT_SMOOTHNESS,   2, &CRendererResources::s_ptexWhite,     &CRendererResources::s_ptexWhite,    "",        "TM_Layer3_WT_Smoothness",  "$TEX_Layer3_WT_Smoothness" },
	{ EFTT_LAYER4_WT_SMOOTHNESS,   2, &CRendererResources::s_ptexWhite,     &CRendererResources::s_ptexWhite,    "",        "TM_Layer4_WT_Smoothness",  "$TEX_Layer4_WT_Smoothness" },

	// This is the terminator for the name-search
	{ EFTT_UNKNOWN,          0, &CRendererResources::s_pTexNULL,      &CRendererResources::s_pTexNULL,     "",        nullptr,              nullptr                },
};

#if 0
static class Verify
{
public:
	Verify()
	{
		for (int i = 0; s_TexSlotSemantics[i].def; i++)
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

namespace  TextureHelpers
{
EEfResTextures FindTexSlot(const char* texSemantic)
{
	if (texSemantic)
	{
		for (int i = 0; s_TexSlotSemantics[i].semantic; i++)
		{
			if ((stricmp(s_TexSlotSemantics[i].semantic, texSemantic) == 0) ||
			    (stricmp(s_TexSlotSemantics[i].semanticOld, texSemantic) == 0))
			{
				return s_TexSlotSemantics[i].slot;
			}
		}
	}

	return EFTT_UNKNOWN;
}

bool VerifyTexSuffix(EEfResTextures texSlot, const char* texPath)
{
	assert((texSlot >= 0) && (texSlot < EFTT_MAX));
	return (strlen(texPath) > strlen(s_TexSlotSemantics[texSlot].suffix) && (CryStringUtils::stristr(texPath, s_TexSlotSemantics[texSlot].suffix)));
}

bool VerifyTexSuffix(EEfResTextures texSlot, const string& texPath)
{
	assert((texSlot >= 0) && (texSlot < EFTT_MAX));
	return (texPath.size() > strlen(s_TexSlotSemantics[texSlot].suffix) && (CryStringUtils::stristr(texPath, s_TexSlotSemantics[texSlot].suffix)));
}

const char* LookupTexSuffix(EEfResTextures texSlot)
{
	assert((texSlot >= 0) && (texSlot < EFTT_MAX));
	return s_TexSlotSemantics[texSlot].suffix;
}

int8 LookupTexPriority(EEfResTextures texSlot)
{
	assert((texSlot >= 0) && (texSlot < EFTT_MAX));
	return s_TexSlotSemantics[texSlot].priority;
}

CTexture* LookupTexDefault(EEfResTextures texSlot)
{
	assert((texSlot >= 0) && (texSlot < EFTT_MAX));
	return *s_TexSlotSemantics[texSlot].def;
}

CTexture* LookupTexNeutral(EEfResTextures texSlot)
{
	assert((texSlot >= 0) && (texSlot < EFTT_MAX));
	return *s_TexSlotSemantics[texSlot].neutral;
}

}
