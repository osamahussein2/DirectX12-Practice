//****************************************************************************
// TreeSprite.hlsl by Frank Luna (C) 2015 All Rights Reserved.
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

Texture2DArray gTreeMapArray : register(t0);

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
    float4x4 gTexTransform;
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
    float cbPerPassPad1;
    float2 gRenderTargetSize;
    float2 gInvRenderTargetSize;
    float gNearZ;
    float gFarZ;
    float gTotalTime;
    float gDeltaTime;
    float4 gAmbientLight;
    
    float4 gFogColor;
    float gFogStart;
    float gFogRange;
    float2 cbPerPassPad2;
    
    // Indices [0, NUM_DIR_LIGHTS) are directional lights;
    // indices [NUM_DIR_LIGHTS, NUM_DIR_LIGHTS+NUM_POINT_LIGHTS) are point
    // lights;
    // indices [NUM_DIR_LIGHTS+NUM_POINT_LIGHTS,
    // NUM_DIR_LIGHTS+NUM_POINT_LIGHT+NUM_SPOT_LIGHTS)
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
    float3 PosW : POSITION;
    float2 SizeW : SIZE;
};

struct VertexOut
{
    float4 CenterH : SV_POSITION;
    float3 CenterW : POSITION;
    float2 SizeW : SIZE;
};

struct GeoOut
{
    float4 PosH : SV_POSITION;
    float3 PosW : POSITION;
    float3 NormalW : NORMAL;
    float2 TexC : TEXCOORD;
    uint PrimID : SV_PrimitiveID;
};

VertexOut VS(VertexIn vin)
{
    VertexOut vout;
    
    // Just pass data over to geometry shader.
    vout.CenterH = float4(vin.PosW, 0.0f);
    vout.CenterW = vin.PosW;
    vout.SizeW = vin.SizeW;
    
    return vout;
}

 // We expand each point into a quad (4 vertices), so the maximum number of vertices
 // we output per geometry shader invocation is 4.

/* When this semantic (uint primID : SV_PrimitiveID) is specified, it tells the input assembler stage to automatically
generate a primitive ID for each primitive. When a draw call is executed to draw n primitives, the first primitive is labeled
0; the second primitive is labeled 1; and so on, until the last primitive in the draw call is labeled n-1. The primitive IDs
are only unique for a single draw call */
[maxvertexcount(4)]
void GS(point VertexOut gin[1], uint primID : SV_PrimitiveID, inout TriangleStream<GeoOut> triStream)
{
    //
    // Compute the local coordinate system of the sprite relative to the world
    // space such that the billboard is aligned with the y-axis and faces the eye.
    //
    float3 up = float3(0.0f, 1.0f, 0.0f);
    float3 look = gEyePosW - gin[0].CenterW;
    look.y = 0.0f; // y-axis aligned, so project to xz-plane
    look = normalize(look);
    float3 right = cross(up, look);
    
    //
    // Compute triangle strip vertices (quad) in world space.
    //
    float halfWidth = 0.5f * gin[0].SizeW.x;
    float halfHeight = 0.5f * gin[0].SizeW.y;
    
    float4 v[4];
    v[0] = float4(gin[0].CenterW + halfWidth * right - halfHeight * up, 1.0f);
    v[1] = float4(gin[0].CenterW + halfWidth * right + halfHeight * up, 1.0f);
    v[2] = float4(gin[0].CenterW - halfWidth * right - halfHeight * up, 1.0f);
    v[3] = float4(gin[0].CenterW - halfWidth * right + halfHeight * up, 1.0f);
    
    //
    // Transform quad vertices to world space and output
    // them as a triangle strip.
    //
    float2 texC[4] =
    {
        float2(0.0f, 1.0f),
        float2(0.0f, 0.0f),
        float2(1.0f, 1.0f),
        float2(1.0f, 0.0f)
    };
    
    GeoOut gout;
    [unroll]
        
    for (int i = 0; i < 4; ++i)
    {
        gout.PosH = mul(v[i], gViewProj);
        gout.PosW = v[i].xyz;
        gout.NormalW = look;
        gout.TexC = texC[i];
        gout.PrimID = primID;

        triStream.Append(gout);
    }
}

float4 PS(GeoOut pin) : SV_Target
{
    /* When using a texture array, three texture coordinates are required. The first two texture coordinates are the usual 2D
    texture coordinates; the third texture coordinate is an index into the texture array. For example, 0 is the index to the
    first texture in the array, 1 is the index to the second texture in the array, 2 is the index to the third texture in the
    array, and so on. */
    
    // Sample a texture array
    float3 uvw = float3(pin.TexC, pin.PrimID % 3);
    float4 diffuseAlbedo = gTreeMapArray.Sample(gsamAnisotropicWrap, uvw) * gDiffuseAlbedo;
    
#ifdef ALPHA_TEST
    // Discard pixel if texture alpha < 0.1. We do this test as soon
    // as possible in the shader so that we can potentially exit the
    // shader early, thereby skipping the rest of the shader code.
    clip(diffuseAlbedo.a - 0.1f);
#endif
    
    // Interpolating normal can unnormalize it, so renormalize it.
    pin.NormalW = normalize(pin.NormalW);
    
    // Vector from point being lit to eye.
    float3 toEyeW = gEyePosW - pin.PosW;
    float distToEye = length(toEyeW);
    toEyeW /= distToEye; // normalize
    
    // Light terms.
    float4 ambient = gAmbientLight * diffuseAlbedo;
    
    const float shininess = 1.0f - gRoughness;
    Material mat = { diffuseAlbedo, gFresnelR0, shininess };
    float3 shadowFactor = 1.0f;
    float4 directLight = ComputeLighting(gLights, mat, pin.PosW, pin.NormalW, toEyeW, shadowFactor);
    
    float4 litColor = ambient + directLight;
#ifdef FOG
    float fogAmount = saturate((distToEye - gFogStart) / gFogRange);
    litColor = lerp(litColor, gFogColor, fogAmount);
#endif
    
    // Common convention to take alpha from diffuse albedo.
    litColor.a = diffuseAlbedo.a;
    
    return litColor;

}