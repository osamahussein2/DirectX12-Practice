//***************************************************************************************
// Default.hlsl by Frank Luna (C) 2015 All Rights Reserved.
//
// Default shader, currently supports lighting.
//***************************************************************************************

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

// Chapter 14 CameraAndDynamicIndexing demo
struct MaterialData
{
    float4 DiffuseAlbedo;
    float3 FresnelR0;
    float Roughness;
    float4x4 MatTransform;
    uint DiffuseMapIndex;
    uint MatPad0;
    uint MatPad1;
    uint MatPad2;
};

//Texture2D gDiffuseMap : register(t0);
//Texture2D gDisplacementMap : register(t1); // Chapter 13 WavesCS demo

// Chapter 14 CameraAndDynamicIndexing demo
// An array of textures, which is only supported in shader model 5.1+.  Unlike Texture2DArray, the textures
// in this array can be different sizes and formats, making it more flexible than texture arrays.
Texture2D gDiffuseMap[4] : register(t0);

// Chapter 14 CameraAndDynamicIndexing demo
// Put in space1, so the texture array does not overlap with these resources.  
// The texture array will occupy registers t0, t1, ..., t3 in space0. 
StructuredBuffer<MaterialData> gMaterialData : register(t0, space1);

//SamplerState gsamLinear : register(s0); // For Crate demo (Chapter 9)

SamplerState gsamPointWrap : register(s0);
SamplerState gsamPointClamp : register(s1);
SamplerState gsamLinearWrap : register(s2);
SamplerState gsamLinearClamp : register(s3);
SamplerState gsamAnisotropicWrap : register(s4);
SamplerState gsamAnisotropicClamp : register(s5);

// Constant data that varies per frame.
cbuffer cbPerObject : register(b0)
{
    float4x4 gWorld;
    float4x4 gTexTransform; // Chapter 9 texturing demo
    
    // Chapter 13 WavesCS demo
    //float2 gDisplacementMapTexelSize;
    //float gGridSpatialStep;
    //float cbPerObjectPad1;
    
    // Chapter 14 CameraAndDynamicIndexing demo
    uint gMaterialIndex;
    uint gObjPad0;
    uint gObjPad1;
    uint gObjPad2;
};

// Constant data that varies per material.
cbuffer cbPass : register(b1)
{
    float4x4 gView;
    float4x4 gInvView;
    float4x4 gProj;
    float4x4 gInvProj;
    float4x4 gViewProj;
    float4x4 gInvViewProj;
    float3 gEyePosW;
    //float cbPerObjectPad1;
    float cbPerPassPad1; // Chapter 13 WavesCS demo
    float2 gRenderTargetSize;
    float2 gInvRenderTargetSize;
    float gNearZ;
    float gFarZ;
    float gTotalTime;
    float gDeltaTime;
    float4 gAmbientLight;

    // Allow application to change fog parameters once per frame.
    // For example, we may only use fog for certain times of day (Chapter 10 blending demo).
    float4 gFogColor;
    float gFogStart;
    float gFogRange;
    //float2 cbPerObjectPad2;
    float2 cbPerPassPad2; // Chapter 13 WavesCS demo

    // Indices [0, NUM_DIR_LIGHTS) are directional lights;
    // indices [NUM_DIR_LIGHTS, NUM_DIR_LIGHTS+NUM_POINT_LIGHTS) are point lights;
    // indices [NUM_DIR_LIGHTS+NUM_POINT_LIGHTS, NUM_DIR_LIGHTS+NUM_POINT_LIGHT+NUM_SPOT_LIGHTS)
    // are spot lights for a maximum of MaxLights per object.
    Light gLights[MaxLights];
};

cbuffer cbMaterial : register(b2)
{
    float4 gDiffuseAlbedo;
    float3 gFresnelR0;
    float gRoughness;
    float4x4 gMatTransform;
};

struct VertexIn
{
    float3 PosL : POSITION;
    float3 NormalL : NORMAL;
    float2 TexC : TEXCOORD; // Chapter 9 demos
};

struct VertexOut
{
    float4 PosH : SV_POSITION;
    float3 PosW : POSITION;
    float3 NormalW : NORMAL;
    float2 TexC : TEXCOORD; // Chapter 9 demos
};

VertexOut VS(VertexIn vin)
{
    VertexOut vout = (VertexOut) 0.0f;
    
#ifdef DISPLACEMENT_MAP
	// Sample the displacement map using non-transformed [0,1]^2 tex-coords.
	vin.PosL.y += gDisplacementMap.SampleLevel(gsamLinearWrap, vin.TexC, 1.0f).r;
	
	// Estimate normal using finite difference.
	float du = gDisplacementMapTexelSize.x;
	float dv = gDisplacementMapTexelSize.y;
	float l = gDisplacementMap.SampleLevel( gsamPointClamp, vin.TexC-float2(du, 0.0f), 0.0f ).r;
	float r = gDisplacementMap.SampleLevel( gsamPointClamp, vin.TexC+float2(du, 0.0f), 0.0f ).r;
	float t = gDisplacementMap.SampleLevel( gsamPointClamp, vin.TexC-float2(0.0f, dv), 0.0f ).r;
	float b = gDisplacementMap.SampleLevel( gsamPointClamp, vin.TexC+float2(0.0f, dv), 0.0f ).r;
	vin.NormalL = normalize( float3(-r+l, 2.0f*gGridSpatialStep, b-t) );
	
#endif
    
    // Chapter 14 CameraAndDynamicIndexing demo
    // Fetch the material data.
    MaterialData matData = gMaterialData[gMaterialIndex];
	
    // Transform to world space.
    float4 posW = mul(float4(vin.PosL, 1.0f), gWorld);
    vout.PosW = posW.xyz;

    // Assumes nonuniform scaling; otherwise, need to use inverse-transpose of world matrix.
    vout.NormalW = mul(vin.NormalL, (float3x3) gWorld);

    // Transform to homogeneous clip space.
    vout.PosH = mul(posW, gViewProj);
	
    // gTexTransform and MatTransform are used in the vertex shader to transform the input texture coordinates
	// Output vertex attributes for interpolation across triangle.
    //float4 texC = mul(float4(vin.TexC, 0.0f, 1.0f), gTexTransform);
    //vout.TexC = mul(texC, gMatTransform).xy;
    
    // Chapter 14 CameraAndDynamicIndexing demo
    // Output vertex attributes for interpolation across triangle.
    float4 texC = mul(float4(vin.TexC, 0.0f, 1.0f), gTexTransform);
    vout.TexC = mul(texC, matData.MatTransform).xy;

    return vout;
}

float4 PS(VertexOut pin) : SV_Target
{
    // Chapter 9 texturing demo
    //float4 diffuseAlbedo = gDiffuseMap.Sample(gsamAnisotropicWrap, pin.TexC) * gDiffuseAlbedo;
	
    /* Observe that we only clip if ALPHA_TEST is defined; this is because we might not want to invoke clip for some render
    items, so we need to be able to switch it on/off by having specialized shaders. Moreover, there is a cost to using alpha
    testing, so we should only use it if we need it. */
#ifdef ALPHA_TEST
	// Discard pixel if texture alpha < 0.1. We do this test as soon 
	// as possible in the shader so that we can potentially exit the
	// shader early, thereby skipping the rest of the shader code.
	clip(diffuseAlbedo.a - 0.1f);
#endif
    
    // Chapter 14 CameraAndDynamicIndexing demo
    // Fetch the material data.
    MaterialData matData = gMaterialData[gMaterialIndex];
    float4 diffuseAlbedo = matData.DiffuseAlbedo;
    float3 fresnelR0 = matData.FresnelR0;
    float roughness = matData.Roughness;
    uint diffuseTexIndex = matData.DiffuseMapIndex;

	// Dynamically look up the texture in the array.
    diffuseAlbedo *= gDiffuseMap[diffuseTexIndex].Sample(gsamLinearWrap, pin.TexC);

    // Interpolating normal can unnormalize it, so renormalize it.
    pin.NormalW = normalize(pin.NormalW);

    // Vector from point being lit to eye. 
    float3 toEyeW = gEyePosW - pin.PosW;
    float distToEye = length(toEyeW);
    toEyeW /= distToEye; // normalize

    // Indirect lighting.
    float4 ambient = gAmbientLight * diffuseAlbedo;

    const float shininess = 1.0f - roughness;
    Material mat = { diffuseAlbedo, fresnelR0, shininess };
    float3 shadowFactor = 1.0f;
    float4 directLight = ComputeLighting(gLights, mat, pin.PosW, pin.NormalW, toEyeW, shadowFactor);

    float4 litColor = ambient + directLight;

#ifdef FOG
    
	float fogAmount = saturate((distToEye - gFogStart) / gFogRange);
	litColor = lerp(litColor, gFogColor, fogAmount);
#endif

    // Common convention to take alpha from diffuse material.
    litColor.a = diffuseAlbedo.a;

    return litColor;
}