#pragma once

#include "d3dUtil.h"
#include "MathHelper.h"
#include "UploadBuffer.h"

struct ObjectConstants
{
    DirectX::XMFLOAT4X4 World = MathHelper::Identity4x4();
    DirectX::XMFLOAT4X4 TexTransform = MathHelper::Identity4x4(); // Chapter 9 texturing demo

    // Chapter 13 SobelApp demo
    //DirectX::XMFLOAT2 DisplacementMapTexelSize = { 1.0f, 1.0f };
    //float GridSpatialStep = 1.0f;
    //float Pad;

    // Chapter 14 CameraAndDynamicIndexing demo
    UINT MaterialIndex;
    UINT ObjPad0;
    UINT ObjPad1;
    UINT ObjPad2;
};

// Chapter 14 CameraAndDynamicIndexing demo
// The material structured buffer replaces our material constant buffer in the frame resources class
struct MaterialData
{
    DirectX::XMFLOAT4 DiffuseAlbedo = { 1.0f, 1.0f, 1.0f, 1.0f };
    DirectX::XMFLOAT3 FresnelR0 = { 0.01f, 0.01f, 0.01f };
    float Roughness = 0.5f;

    // Used in texture mapping.
    DirectX::XMFLOAT4X4 MatTransform = MathHelper::Identity4x4();

    UINT DiffuseMapIndex = 0;
    UINT NormalMapIndex = 0; // Chapter 19 normal mapping demo

    //UINT MaterialPad0;
    UINT MaterialPad1;
    UINT MaterialPad2;
};

struct PassConstants
{
    DirectX::XMFLOAT4X4 View = MathHelper::Identity4x4();
    DirectX::XMFLOAT4X4 InvView = MathHelper::Identity4x4();
    DirectX::XMFLOAT4X4 Proj = MathHelper::Identity4x4();
    DirectX::XMFLOAT4X4 InvProj = MathHelper::Identity4x4();
    DirectX::XMFLOAT4X4 ViewProj = MathHelper::Identity4x4();
    DirectX::XMFLOAT4X4 InvViewProj = MathHelper::Identity4x4();
    DirectX::XMFLOAT4X4 ViewProjTex = MathHelper::Identity4x4(); // Chapter 21 ambient occlusion demo
    DirectX::XMFLOAT4X4 ShadowTransform = MathHelper::Identity4x4(); // Chapter 20 shadow mapping demo
    DirectX::XMFLOAT3 EyePosW = { 0.0f, 0.0f, 0.0f };
    float cbPerObjectPad1 = 0.0f;
    DirectX::XMFLOAT2 RenderTargetSize = { 0.0f, 0.0f };
    DirectX::XMFLOAT2 InvRenderTargetSize = { 0.0f, 0.0f };
    float NearZ = 0.0f;
    float FarZ = 0.0f;
    float TotalTime = 0.0f;
    float DeltaTime = 0.0f;

    // Chapter 8 lighting demo
    DirectX::XMFLOAT4 AmbientLight = { 0.0f, 0.0f, 0.0f, 1.0f };

    // Chapter 10 blending demo
    //DirectX::XMFLOAT4 FogColor = { 0.7f, 0.7f, 0.7f, 1.0f };
    //float gFogStart = 5.0f;
    //float gFogRange = 150.0f;
    //DirectX::XMFLOAT2 cbPerObjectPad2;

    // Indices [0, NUM_DIR_LIGHTS) are directional lights;
    // indices [NUM_DIR_LIGHTS, NUM_DIR_LIGHTS+NUM_POINT_LIGHTS) are point lights;
    // indices [NUM_DIR_LIGHTS+NUM_POINT_LIGHTS, NUM_DIR_LIGHTS+NUM_POINT_LIGHT+NUM_SPOT_LIGHTS)
    // are spot lights for a maximum of MaxLights per object.
    Light Lights[MaxLights];
};

// Chpater 16 demo
// The instance buffer stores the data that varies per-instance
struct InstanceData
{
    DirectX::XMFLOAT4X4 World = MathHelper::Identity4x4();
    DirectX::XMFLOAT4X4 TexTransform = MathHelper::Identity4x4();
    UINT MaterialIndex;
    UINT InstancePad0;
    UINT InstancePad1;
    UINT InstancePad2;
};

// Chapter 21 ambient occlusion
struct SsaoConstants
{
    DirectX::XMFLOAT4X4 Proj;
    DirectX::XMFLOAT4X4 InvProj;
    DirectX::XMFLOAT4X4 ProjTex;
    DirectX::XMFLOAT4   OffsetVectors[14];

    // For SsaoBlur.hlsl
    DirectX::XMFLOAT4 BlurWeights[3];

    DirectX::XMFLOAT2 InvRenderTargetSize = { 0.0f, 0.0f };

    // Coordinates given in view space.
    float OcclusionRadius = 0.5f;
    float OcclusionFadeStart = 0.2f;
    float OcclusionFadeEnd = 2.0f;
    float SurfaceEpsilon = 0.05f;
};

// Chapter 23 character animation demo
struct SkinnedConstants
{
    DirectX::XMFLOAT4X4 BoneTransforms[96];
};

struct Vertex
{
    Vertex() = default;
    Vertex(float x, float y, float z, float nx, float ny, float nz, float u, float v) :
        Pos(x, y, z),
        Normal(nx, ny, nz),
        TexC(u, v) {}

    DirectX::XMFLOAT3 Pos;
    //DirectX::XMFLOAT4 Color; // Chapter 7 demos
    DirectX::XMFLOAT3 Normal; // Chapter 8 lighting demo
    DirectX::XMFLOAT2 TexC; // Chapter 9 texturing demo

    /* In our system, we will not store the bitangent vector B directly in memory. Instead, we will compute B = N � T when
    we need B, where N is the usual averaged vertex normal (Chapter 19 normal mapping demo) */
    DirectX::XMFLOAT3 TangentU;
};

// Chapter 23 character animation demo
struct SkinnedVertex
{
    DirectX::XMFLOAT3 Pos;
    DirectX::XMFLOAT3 Normal;
    DirectX::XMFLOAT2 TexC;
    DirectX::XMFLOAT3 TangentU;
    DirectX::XMFLOAT3 BoneWeights;
    BYTE BoneIndices[4];
};

// Stores the resources needed for the CPU to build the command lists
// for a frame.  
struct FrameResource
{
public:
    FrameResource(ID3D12Device* device, UINT passCount);
    //FrameResource(ID3D12Device* device, UINT passCount, UINT objectCount);
    //FrameResource(ID3D12Device* device, UINT passCount, UINT objectCount, UINT waveVertCount);
    FrameResource(ID3D12Device* device, UINT passCount, UINT objectCount, UINT materialCount);
    //FrameResource(ID3D12Device* device, UINT passCount, UINT maxInstanceCount, UINT materialCount); // Chapter 16 demo
    //FrameResource(ID3D12Device* device, UINT passCount, UINT objectCount, UINT materialCount, UINT waveVertCount);
    FrameResource(ID3D12Device* device, UINT passCount, UINT objectCount, UINT skinnedObjectCount, UINT materialCount);
    FrameResource(const FrameResource& rhs) = delete;
    FrameResource& operator=(const FrameResource& rhs) = delete;
    ~FrameResource();

    // We cannot reset the allocator until the GPU is done processing the commands.
    // So each frame needs their own allocator.
    Microsoft::WRL::ComPtr<ID3D12CommandAllocator> CmdListAlloc;

    // We cannot update a cbuffer until the GPU is done processing the commands
    // that reference it.  So each frame needs their own cbuffers.
    std::unique_ptr<UploadBuffer<PassConstants>> PassCB = nullptr;
    std::unique_ptr<UploadBuffer<ObjectConstants>> ObjectCB = nullptr;

    // Chapter 8 demo (lighting)
    std::unique_ptr<UploadBuffer<MaterialConstants>> MaterialCB = nullptr;

    // Chapter 14 CameraAndDynamicIndexing demo
    std::unique_ptr<UploadBuffer<MaterialData>> MaterialBuffer = nullptr;

    // We cannot update a dynamic vertex buffer until the GPU is done processing
    // the commands that reference it.  So each frame needs their own.
    std::unique_ptr<UploadBuffer<Vertex>> WavesVB = nullptr;

    // NOTE: In this demo, we instance only one render-item, so we only have
    // one structured buffer to store instancing data. To make this more
    // general (i.e., to support instancing multiple render-items), you
    // would need to have a structured buffer for each render-item, and
    // allocate each buffer with enough room for the maximum number of
    // instances you would ever draw. This sounds like a lot, but it is
    // actually no more than the amount of per-object constant data we
    // would need if we were not using instancing. For example, if we
    // were drawing 1000 objects without instancing, we would create a
    // constant buffer with enough room for a 1000 objects. With instancing,
    // we would just create a structured buffer large enough to store the
    // instance data for 1000 instances.
    std::unique_ptr<UploadBuffer<InstanceData>> InstanceBuffer = nullptr;

    std::unique_ptr<UploadBuffer<SsaoConstants>> SsaoCB = nullptr; // Chapter 21 ambient occlusion demo

    std::unique_ptr<UploadBuffer<SkinnedConstants>> SkinnedCB = nullptr; // Chapter 23 character animation demo

    // Fence value to mark commands up to this fence point.  This lets us
    // check if these frame resources are still in use by the GPU.
    UINT64 Fence = 0;
};