//****************************************************************************
// Common.hlsl by Frank Luna (C) 2015 All Rights Reserved.
//****************************************************************************

// Defaults for number of lights.
#ifndef NUM_DIR_LIGHTS
#define NUM_DIR_LIGHTS 3
#endif

#ifndef NUM_POINT_LIGHTS
#define NUM_POINT_LIGHTS 0
#endif

#ifndef NUM_SPOT_LIGHTS
#define NUM_SPOT_LIGHTS 0
#endif

// Include structures and functions for lighting.
#include "LightingUtils.hlsl"

struct MaterialData
{
    float4 DiffuseAlbedo;
    float3 FresnelR0;
    float Roughness;
    float4x4 MatTransform;
    uint DiffuseMapIndex;
    uint NormalMapIndex; // Chapter 19 normal mapping demo
    //uint MatPad0;
    uint MatPad1;
    uint MatPad2;
};

TextureCube gCubeMap : register(t0);

// Chapter 20 shadow mapping demo
Texture2D gShadowMap : register(t1);

// Chapter 21 ambient occlusion demo
Texture2D gSsaoMap : register(t2);

// An array of textures, which is only supported in shader model 5.1+. Unlike
// Texture2DArray, the textures in this array can be different sizes and
// formats, making it more flexible than texture arrays.
//Texture2D gDiffuseMap[4] : register(t1);

// Chapter 19 normal mapping demo
//Texture2D gTextureMaps[10] : register(t2);

// Chapter 21 ambient occlusion demo
//Texture2D gTextureMaps[10] : register(t3);

// Chapter 23 character animations demo
Texture2D gTextureMaps[48] : register(t3);

// Put in space1, so the texture array does not overlap with these resources.
// The texture array will occupy registers t0, t1, ..., t3 in space0.
StructuredBuffer<MaterialData> gMaterialData : register(t0, space1);

SamplerState gsamPointWrap : register(s0);
SamplerState gsamPointClamp : register(s1);
SamplerState gsamLinearWrap : register(s2);
SamplerState gsamLinearClamp : register(s3);
SamplerState gsamAnisotropicWrap : register(s4);
SamplerState gsamAnisotropicClamp : register(s5);
SamplerComparisonState gsamShadow : register(s6);

// Constant data that varies per frame.
cbuffer cbPerObject : register(b0)
{
    float4x4 gWorld;
    float4x4 gTexTransform;
    uint gMaterialIndex;
    uint gObjPad0;
    uint gObjPad1;
    uint gObjPad2;
};

// Chapter 23 character animation demo
/* The final bone transforms are stored in a constant buffer where they are accessed in the vertex shader to do the
animation transformations */
cbuffer cbSkinned : register(b1)
{
    float4x4 gBoneTransforms[96];
};

// Constant data that varies per material.
cbuffer cbPass : register(b2) // used to be b1, only for Chapter 23 demo
{
    float4x4 gView;
    float4x4 gInvView;
    float4x4 gProj;
    float4x4 gInvProj;
    float4x4 gViewProj;
    float4x4 gInvViewProj;
    float4x4 gViewProjTex; // Chapter 21 ambient occlusion demo
    float4x4 gShadowTransform; // Chapter 20 shadow mapping demo
    float3 gEyePosW;
    float cbPerObjectPad1;
    float2 gRenderTargetSize;
    float2 gInvRenderTargetSize;
    float gNearZ;
    float gFarZ;
    float gTotalTime;
    float gDeltaTime;
    float4 gAmbientLight;
    
    // Indices [0, NUM_DIR_LIGHTS) are directional lights;
    // indices [NUM_DIR_LIGHTS, NUM_DIR_LIGHTS+NUM_POINT_LIGHTS) are point lights;
    // indices [NUM_DIR_LIGHTS+NUM_POINT_LIGHTS,
    // NUM_DIR_LIGHTS+NUM_POINT_LIGHT+NUM_SPOT_LIGHTS)
    // are spot lights for a maximum of MaxLights per object.
    Light gLights[MaxLights];
};

// Chapter 19 normal mapping demo

//--------------------------------------------------------------------
// Transforms a normal map sample to world space.
//--------------------------------------------------------------------
float3 NormalSampleToWorldSpace(float3 normalMapSample, float3 unitNormalW, float3 tangentW)
{
    // Uncompress each component from [0,1] to [-1,1].
    float3 normalT = 2.0f * normalMapSample - 1.0f;
    
    /* After the interpolation, the tangent vector and normal vector may not be orthonormal. This code makes sure T is
    orthonormal to N by subtracting off any component of T along the direction N. Note that there is the assumption that
    unitNormalW is normalized */
    
    // Build orthonormal basis.
    float3 N = unitNormalW;
    float3 T = normalize(tangentW - dot(tangentW, N) * N);
    float3 B = cross(N, T);
    
    float3x3 TBN = float3x3(T, B, N);
    
    // Transform from tangent space to world space.
    float3 bumpedNormalW = mul(normalT, TBN);
    
    return bumpedNormalW;
}

// Chapter 20 shadow mapping demo
//---------------------------------------------------------------------------------------
// PCF for shadow mapping.
//---------------------------------------------------------------------------------------
float CalcShadowFactor(float4 shadowPosH)
{
    // Complete projection by doing division by w.
    shadowPosH.xyz /= shadowPosH.w;
    
    // Depth in NDC space.
    float depth = shadowPosH.z;
    uint width, height, numMips;
    gShadowMap.GetDimensions(0, width, height, numMips);
    
    // Texel size.
    float dx = 1.0f / (float) width;
    float percentLit = 0.0f;
    
    const float2 offsets[9] =
    {
        float2(-dx, -dx), float2(0.0f, -dx), float2(dx, -dx),
        float2(-dx, 0.0f), float2(0.0f, 0.0f), float2(dx, 0.0f),
        float2(-dx, +dx), float2(0.0f, +dx), float2(dx, +dx)
    };
    
    // 3x3 box filter pattern. Each sample does a 4-tap PCF.
 [unroll]
    for (int i = 0; i < 9; ++i)
    {
        // When we build our PCF kernel, we offset our texture coordinates to sample neighboring values in the shadow map
        percentLit += gShadowMap.SampleCmpLevelZero(gsamShadow, shadowPosH.xy + offsets[i], depth).r;
    }

    return percentLit / 9.0f;
}