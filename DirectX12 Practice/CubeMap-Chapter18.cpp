////***************************************************************************************
//// CubeMapApp.cpp by Frank Luna (C) 2015 All Rights Reserved.
////***************************************************************************************
//
//#include "d3dApp.h"
//#include "MathHelper.h"
//#include "UploadBuffer.h"
//#include "GeometryGenerator.h"
//#include "Camera.h"
//#include "FrameResource.h"
//
//using Microsoft::WRL::ComPtr;
//using namespace DirectX;
//using namespace DirectX::PackedVector;
//
//#pragma comment(lib, "d3dcompiler.lib")
//#pragma comment(lib, "D3D12.lib")
//
//const int gNumFrameResources = 3;
//
//// Lightweight structure stores parameters to draw a shape.  This will
//// vary from app-to-app.
//struct RenderItem
//{
//	RenderItem() = default;
//	RenderItem(const RenderItem& rhs) = delete;
//
//	// World matrix of the shape that describes the object's local space
//	// relative to the world space, which defines the position, orientation,
//	// and scale of the object in the world.
//	XMFLOAT4X4 World = MathHelper::Identity4x4();
//
//	XMFLOAT4X4 TexTransform = MathHelper::Identity4x4();
//
//	// Dirty flag indicating the object data has changed and we need to update the constant buffer.
//	// Because we have an object cbuffer for each FrameResource, we have to apply the
//	// update to each FrameResource.  Thus, when we modify obect data we should set 
//	// NumFramesDirty = gNumFrameResources so that each frame resource gets the update.
//	int NumFramesDirty = gNumFrameResources;
//
//	// Index into GPU constant buffer corresponding to the ObjectCB for this render item.
//	UINT ObjCBIndex = -1;
//
//	Material* Mat = nullptr;
//	MeshGeometry* Geo = nullptr;
//
//	// Primitive topology.
//	D3D12_PRIMITIVE_TOPOLOGY PrimitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
//
//	// DrawIndexedInstanced parameters.
//	UINT IndexCount = 0;
//	UINT StartIndexLocation = 0;
//	int BaseVertexLocation = 0;
//};
//
//enum class RenderLayer : int
//{
//	Opaque = 0,
//	Sky,
//	Count
//};
//
//class CubeMapApp : public D3DApp
//{
//public:
//	CubeMapApp(HINSTANCE hInstance);
//	CubeMapApp(const CubeMapApp& rhs) = delete;
//	CubeMapApp& operator=(const CubeMapApp& rhs) = delete;
//	~CubeMapApp();
//
//	virtual bool Initialize()override;
//
//private:
//	virtual void OnResize()override;
//	virtual void Update(const GameTimer& gt)override;
//	virtual void Draw(const GameTimer& gt)override;
//
//	virtual void OnMouseDown(WPARAM btnState, int x, int y)override;
//	virtual void OnMouseUp(WPARAM btnState, int x, int y)override;
//	virtual void OnMouseMove(WPARAM btnState, int x, int y)override;
//
//	void OnKeyboardInput(const GameTimer& gt);
//	void AnimateMaterials(const GameTimer& gt);
//	void UpdateObjectCBs(const GameTimer& gt);
//	void UpdateMaterialBuffer(const GameTimer& gt);
//	void UpdateMainPassCB(const GameTimer& gt);
//
//	void LoadTextures();
//	void BuildRootSignature();
//	void BuildDescriptorHeaps();
//	void BuildShadersAndInputLayout();
//	void BuildShapeGeometry();
//	void BuildSkullGeometry();
//	void BuildPSOs();
//	void BuildFrameResources();
//	void BuildMaterials();
//	void BuildRenderItems();
//	void DrawRenderItems(ID3D12GraphicsCommandList* cmdList, const std::vector<RenderItem*>& ritems);
//
//	std::array<const CD3DX12_STATIC_SAMPLER_DESC, 6> GetStaticSamplers();
//
//private:
//
//	std::vector<std::unique_ptr<FrameResource>> mFrameResources;
//	FrameResource* mCurrFrameResource = nullptr;
//	int mCurrFrameResourceIndex = 0;
//
//	UINT mCbvSrvDescriptorSize = 0;
//
//	ComPtr<ID3D12RootSignature> mRootSignature = nullptr;
//
//	ComPtr<ID3D12DescriptorHeap> mSrvDescriptorHeap = nullptr;
//
//	std::unordered_map<std::string, std::unique_ptr<MeshGeometry>> mGeometries;
//	std::unordered_map<std::string, std::unique_ptr<Material>> mMaterials;
//	std::unordered_map<std::string, std::unique_ptr<Texture>> mTextures;
//	std::unordered_map<std::string, ComPtr<ID3DBlob>> mShaders;
//	std::unordered_map<std::string, ComPtr<ID3D12PipelineState>> mPSOs;
//
//	std::vector<D3D12_INPUT_ELEMENT_DESC> mInputLayout;
//
//	// List of all the render items.
//	std::vector<std::unique_ptr<RenderItem>> mAllRitems;
//
//	// Render items divided by PSO.
//	std::vector<RenderItem*> mRitemLayer[(int)RenderLayer::Count];
//
//	UINT mSkyTexHeapIndex = 0;
//
//	PassConstants mMainPassCB;
//
//	Camera mCamera;
//
//	POINT mLastMousePos;
//};
//
//int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE prevInstance,
//	PSTR cmdLine, int showCmd)
//{
//	// Enable run-time memory check for debug builds.
//#if defined(DEBUG) | defined(_DEBUG)
//	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
//#endif
//
//	try
//	{
//		CubeMapApp theApp(hInstance);
//		if (!theApp.Initialize())
//			return 0;
//
//		return theApp.Run();
//	}
//	catch (DxException& e)
//	{
//		MessageBox(nullptr, e.ToString().c_str(), L"HR Failed", MB_OK);
//		return 0;
//	}
//}
//
//CubeMapApp::CubeMapApp(HINSTANCE hInstance)
//	: D3DApp(hInstance)
//{
//}
//
//CubeMapApp::~CubeMapApp()
//{
//}
//
//bool CubeMapApp::Initialize()
//{
//	if (!D3DApp::Initialize())
//		return false;
//
//	// Reset the command list to prep for initialization commands.
//	ThrowIfFailed(mCommandList->Reset(mDirectCmdListAlloc.Get(), nullptr));
//
//	// Get the increment size of a descriptor in this heap type.  This is hardware specific, 
//	// so we have to query this information.
//	mCbvSrvDescriptorSize = md3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
//
//	mCamera.SetPosition(0.0f, 2.0f, -15.0f);
//
//	LoadTextures();
//	BuildRootSignature();
//	BuildDescriptorHeaps();
//	BuildShadersAndInputLayout();
//	BuildShapeGeometry();
//	BuildSkullGeometry();
//	BuildMaterials();
//	BuildRenderItems();
//	BuildFrameResources();
//	BuildPSOs();
//
//	// Execute the initialization commands.
//	ThrowIfFailed(mCommandList->Close());
//	ID3D12CommandList* cmdsLists[] = { mCommandList.Get() };
//	mCommandQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);
//
//	// Wait until initialization is complete.
//	FlushCommandQueue();
//
//	return true;
//}
//
//void CubeMapApp::OnResize()
//{
//	D3DApp::OnResize();
//
//	mCamera.SetLens(0.25f * MathHelper::Pi, AspectRatio(), 1.0f, 1000.0f);
//}
//
//void CubeMapApp::Update(const GameTimer& gt)
//{
//	OnKeyboardInput(gt);
//
//	// Cycle through the circular frame resource array.
//	mCurrFrameResourceIndex = (mCurrFrameResourceIndex + 1) % gNumFrameResources;
//	mCurrFrameResource = mFrameResources[mCurrFrameResourceIndex].get();
//
//	// Has the GPU finished processing the commands of the current frame resource?
//	// If not, wait until the GPU has completed commands up to this fence point.
//	if (mCurrFrameResource->Fence != 0 && mFence->GetCompletedValue() < mCurrFrameResource->Fence)
//	{
//		HANDLE eventHandle = CreateEventEx(nullptr, false, false, EVENT_ALL_ACCESS);
//		ThrowIfFailed(mFence->SetEventOnCompletion(mCurrFrameResource->Fence, eventHandle));
//		WaitForSingleObject(eventHandle, INFINITE);
//		CloseHandle(eventHandle);
//	}
//
//	AnimateMaterials(gt);
//	UpdateObjectCBs(gt);
//	UpdateMaterialBuffer(gt);
//	UpdateMainPassCB(gt);
//}
//
//void CubeMapApp::Draw(const GameTimer& gt)
//{
//	auto cmdListAlloc = mCurrFrameResource->CmdListAlloc;
//
//	// Reuse the memory associated with command recording.
//	// We can only reset when the associated command lists have finished execution on the GPU.
//	ThrowIfFailed(cmdListAlloc->Reset());
//
//	// A command list can be reset after it has been added to the command queue via ExecuteCommandList.
//	// Reusing the command list reuses memory.
//	ThrowIfFailed(mCommandList->Reset(cmdListAlloc.Get(), mPSOs["opaque"].Get()));
//
//	mCommandList->RSSetViewports(1, &mScreenViewport);
//	mCommandList->RSSetScissorRects(1, &mScissorRect);
//
//	// Indicate a state transition on the resource usage.
//	mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(CurrentBackBuffer(),
//		D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));
//
//	// Clear the back buffer and depth buffer.
//	mCommandList->ClearRenderTargetView(CurrentBackBufferView(), Colors::LightSteelBlue, 0, nullptr);
//	mCommandList->ClearDepthStencilView(DepthStencilView(), D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);
//
//	// Specify the buffers we are going to render to.
//	mCommandList->OMSetRenderTargets(1, &CurrentBackBufferView(), true, &DepthStencilView());
//
//	ID3D12DescriptorHeap* descriptorHeaps[] = { mSrvDescriptorHeap.Get() };
//	mCommandList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);
//
//	mCommandList->SetGraphicsRootSignature(mRootSignature.Get());
//
//	auto passCB = mCurrFrameResource->PassCB->Resource();
//	mCommandList->SetGraphicsRootConstantBufferView(1, passCB->GetGPUVirtualAddress());
//
//	// Bind all the materials used in this scene.  For structured buffers, we can bypass the heap and 
//	// set as a root descriptor.
//	auto matBuffer = mCurrFrameResource->MaterialBuffer->Resource();
//	mCommandList->SetGraphicsRootShaderResourceView(2, matBuffer->GetGPUVirtualAddress());
//
//	// Bind the sky cube map.  For our demos, we just use one "world" cube map representing the environment
//	// from far away, so all objects will use the same cube map and we only need to set it once per-frame.  
//	// If we wanted to use "local" cube maps, we would have to change them per-object, or dynamically
//	// index into an array of cube maps.
//
//	CD3DX12_GPU_DESCRIPTOR_HANDLE skyTexDescriptor(mSrvDescriptorHeap->GetGPUDescriptorHandleForHeapStart());
//	skyTexDescriptor.Offset(mSkyTexHeapIndex, mCbvSrvDescriptorSize);
//	mCommandList->SetGraphicsRootDescriptorTable(3, skyTexDescriptor);
//
//	// Bind all the textures used in this scene.  Observe
//	// that we only have to specify the first descriptor in the table.  
//	// The root signature knows how many descriptors are expected in the table.
//	mCommandList->SetGraphicsRootDescriptorTable(4, mSrvDescriptorHeap->GetGPUDescriptorHandleForHeapStart());
//
//	DrawRenderItems(mCommandList.Get(), mRitemLayer[(int)RenderLayer::Opaque]);
//
//	mCommandList->SetPipelineState(mPSOs["sky"].Get());
//	DrawRenderItems(mCommandList.Get(), mRitemLayer[(int)RenderLayer::Sky]);
//
//	// Indicate a state transition on the resource usage.
//	mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(CurrentBackBuffer(),
//		D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));
//
//	// Done recording commands.
//	ThrowIfFailed(mCommandList->Close());
//
//	// Add the command list to the queue for execution.
//	ID3D12CommandList* cmdsLists[] = { mCommandList.Get() };
//	mCommandQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);
//
//	// Swap the back and front buffers
//	ThrowIfFailed(mSwapChain->Present(0, 0));
//	mCurrBackBuffer = (mCurrBackBuffer + 1) % SwapChainBufferCount;
//
//	// Advance the fence value to mark commands up to this fence point.
//	mCurrFrameResource->Fence = ++mCurrentFence;
//
//	// Add an instruction to the command queue to set a new fence point. 
//	// Because we are on the GPU timeline, the new fence point won't be 
//	// set until the GPU finishes processing all the commands prior to this Signal().
//	mCommandQueue->Signal(mFence.Get(), mCurrentFence);
//}
//
//void CubeMapApp::OnMouseDown(WPARAM btnState, int x, int y)
//{
//	mLastMousePos.x = x;
//	mLastMousePos.y = y;
//
//	SetCapture(mhMainWnd);
//}
//
//void CubeMapApp::OnMouseUp(WPARAM btnState, int x, int y)
//{
//	ReleaseCapture();
//}
//
//void CubeMapApp::OnMouseMove(WPARAM btnState, int x, int y)
//{
//	if ((btnState & MK_LBUTTON) != 0)
//	{
//		// Make each pixel correspond to a quarter of a degree.
//		float dx = XMConvertToRadians(0.25f * static_cast<float>(x - mLastMousePos.x));
//		float dy = XMConvertToRadians(0.25f * static_cast<float>(y - mLastMousePos.y));
//
//		mCamera.Pitch(dy);
//		mCamera.RotateY(dx);
//	}
//
//	mLastMousePos.x = x;
//	mLastMousePos.y = y;
//}
//
//void CubeMapApp::OnKeyboardInput(const GameTimer& gt)
//{
//	const float dt = gt.DeltaTime();
//
//	if (GetAsyncKeyState('W') & 0x8000)
//		mCamera.Walk(10.0f * dt);
//
//	if (GetAsyncKeyState('S') & 0x8000)
//		mCamera.Walk(-10.0f * dt);
//
//	if (GetAsyncKeyState('A') & 0x8000)
//		mCamera.Strafe(-10.0f * dt);
//
//	if (GetAsyncKeyState('D') & 0x8000)
//		mCamera.Strafe(10.0f * dt);
//
//	mCamera.UpdateViewMatrix();
//}
//
//void CubeMapApp::AnimateMaterials(const GameTimer& gt)
//{
//
//}
//
//void CubeMapApp::UpdateObjectCBs(const GameTimer& gt)
//{
//	auto currObjectCB = mCurrFrameResource->ObjectCB.get();
//	for (auto& e : mAllRitems)
//	{
//		// Only update the cbuffer data if the constants have changed.  
//		// This needs to be tracked per frame resource.
//		if (e->NumFramesDirty > 0)
//		{
//			XMMATRIX world = XMLoadFloat4x4(&e->World);
//			XMMATRIX texTransform = XMLoadFloat4x4(&e->TexTransform);
//
//			ObjectConstants objConstants;
//			XMStoreFloat4x4(&objConstants.World, XMMatrixTranspose(world));
//			XMStoreFloat4x4(&objConstants.TexTransform, XMMatrixTranspose(texTransform));
//			objConstants.MaterialIndex = e->Mat->MatCBIndex;
//
//			currObjectCB->CopyData(e->ObjCBIndex, objConstants);
//
//			// Next FrameResource need to be updated too.
//			e->NumFramesDirty--;
//		}
//	}
//}
//
//void CubeMapApp::UpdateMaterialBuffer(const GameTimer& gt)
//{
//	auto currMaterialBuffer = mCurrFrameResource->MaterialBuffer.get();
//	for (auto& e : mMaterials)
//	{
//		// Only update the cbuffer data if the constants have changed.  If the cbuffer
//		// data changes, it needs to be updated for each FrameResource.
//		Material* mat = e.second.get();
//		if (mat->NumFramesDirty > 0)
//		{
//			XMMATRIX matTransform = XMLoadFloat4x4(&mat->MatTransform);
//
//			MaterialData matData;
//			matData.DiffuseAlbedo = mat->DiffuseAlbedo;
//			matData.FresnelR0 = mat->FresnelR0;
//			matData.Roughness = mat->Roughness;
//			XMStoreFloat4x4(&matData.MatTransform, XMMatrixTranspose(matTransform));
//			matData.DiffuseMapIndex = mat->DiffuseSrvHeapIndex;
//
//			currMaterialBuffer->CopyData(mat->MatCBIndex, matData);
//
//			// Next FrameResource need to be updated too.
//			mat->NumFramesDirty--;
//		}
//	}
//}
//
//void CubeMapApp::UpdateMainPassCB(const GameTimer& gt)
//{
//	XMMATRIX view = mCamera.GetView();
//	XMMATRIX proj = mCamera.GetProj();
//
//	XMMATRIX viewProj = XMMatrixMultiply(view, proj);
//	XMMATRIX invView = XMMatrixInverse(&XMMatrixDeterminant(view), view);
//	XMMATRIX invProj = XMMatrixInverse(&XMMatrixDeterminant(proj), proj);
//	XMMATRIX invViewProj = XMMatrixInverse(&XMMatrixDeterminant(viewProj), viewProj);
//
//	XMStoreFloat4x4(&mMainPassCB.View, XMMatrixTranspose(view));
//	XMStoreFloat4x4(&mMainPassCB.InvView, XMMatrixTranspose(invView));
//	XMStoreFloat4x4(&mMainPassCB.Proj, XMMatrixTranspose(proj));
//	XMStoreFloat4x4(&mMainPassCB.InvProj, XMMatrixTranspose(invProj));
//	XMStoreFloat4x4(&mMainPassCB.ViewProj, XMMatrixTranspose(viewProj));
//	XMStoreFloat4x4(&mMainPassCB.InvViewProj, XMMatrixTranspose(invViewProj));
//	mMainPassCB.EyePosW = mCamera.GetPosition3f();
//	mMainPassCB.RenderTargetSize = XMFLOAT2((float)mClientWidth, (float)mClientHeight);
//	mMainPassCB.InvRenderTargetSize = XMFLOAT2(1.0f / mClientWidth, 1.0f / mClientHeight);
//	mMainPassCB.NearZ = 1.0f;
//	mMainPassCB.FarZ = 1000.0f;
//	mMainPassCB.TotalTime = gt.TotalTime();
//	mMainPassCB.DeltaTime = gt.DeltaTime();
//	mMainPassCB.AmbientLight = { 0.25f, 0.25f, 0.35f, 1.0f };
//	mMainPassCB.Lights[0].Direction = { 0.57735f, -0.57735f, 0.57735f };
//	mMainPassCB.Lights[0].Strength = { 0.6f, 0.6f, 0.6f };
//	mMainPassCB.Lights[1].Direction = { -0.57735f, -0.57735f, 0.57735f };
//	mMainPassCB.Lights[1].Strength = { 0.3f, 0.3f, 0.3f };
//	mMainPassCB.Lights[2].Direction = { 0.0f, -0.707f, -0.707f };
//	mMainPassCB.Lights[2].Strength = { 0.15f, 0.15f, 0.15f };
//
//	auto currPassCB = mCurrFrameResource->PassCB.get();
//	currPassCB->CopyData(0, mMainPassCB);
//}
//
//void CubeMapApp::LoadTextures()
//{
//	std::vector<std::string> texNames =
//	{
//		"bricksDiffuseMap",
//		"tileDiffuseMap",
//		"defaultDiffuseMap",
//		"skyCubeMap"
//	};
//
//	std::vector<std::wstring> texFilenames =
//	{
//		L"Textures/bricks2.dds",
//		L"Textures/tile.dds",
//		L"Textures/white1x1.dds",
//		L"Textures/grasscube1024.dds"
//	};
//
//	// Detect that the DDS file contains a cube map, and create a texture array and load the face data into each element
//	for (int i = 0; i < (int)texNames.size(); ++i)
//	{
//		auto texMap = std::make_unique<Texture>();
//		texMap->Name = texNames[i];
//		texMap->Filename = texFilenames[i];
//		ThrowIfFailed(DirectX::CreateDDSTextureFromFile12(md3dDevice.Get(),
//			mCommandList.Get(), texMap->Filename.c_str(),
//			texMap->Resource, texMap->UploadHeap));
//
//		mTextures[texMap->Name] = std::move(texMap);
//	}
//}
//
//void CubeMapApp::BuildRootSignature()
//{
//	CD3DX12_DESCRIPTOR_RANGE texTable0;
//	texTable0.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 0);
//
//	CD3DX12_DESCRIPTOR_RANGE texTable1;
//	texTable1.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 5, 1, 0);
//
//	// Root parameter can be a table, root descriptor or root constants.
//	CD3DX12_ROOT_PARAMETER slotRootParameter[5];
//
//	// Perfomance TIP: Order from most frequent to least frequent.
//	slotRootParameter[0].InitAsConstantBufferView(0);
//	slotRootParameter[1].InitAsConstantBufferView(1);
//	slotRootParameter[2].InitAsShaderResourceView(0, 1);
//	slotRootParameter[3].InitAsDescriptorTable(1, &texTable0, D3D12_SHADER_VISIBILITY_PIXEL);
//	slotRootParameter[4].InitAsDescriptorTable(1, &texTable1, D3D12_SHADER_VISIBILITY_PIXEL);
//
//
//	auto staticSamplers = GetStaticSamplers();
//
//	// A root signature is an array of root parameters.
//	CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc(5, slotRootParameter,
//		(UINT)staticSamplers.size(), staticSamplers.data(),
//		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);
//
//	// create a root signature with a single slot which points to a descriptor range consisting of a single constant buffer
//	ComPtr<ID3DBlob> serializedRootSig = nullptr;
//	ComPtr<ID3DBlob> errorBlob = nullptr;
//	HRESULT hr = D3D12SerializeRootSignature(&rootSigDesc, D3D_ROOT_SIGNATURE_VERSION_1,
//		serializedRootSig.GetAddressOf(), errorBlob.GetAddressOf());
//
//	if (errorBlob != nullptr)
//	{
//		::OutputDebugStringA((char*)errorBlob->GetBufferPointer());
//	}
//	ThrowIfFailed(hr);
//
//	ThrowIfFailed(md3dDevice->CreateRootSignature(
//		0,
//		serializedRootSig->GetBufferPointer(),
//		serializedRootSig->GetBufferSize(),
//		IID_PPV_ARGS(mRootSignature.GetAddressOf())));
//}
//
//void CubeMapApp::BuildDescriptorHeaps()
//{
//	//
//	// Create the SRV heap.
//	//
//	D3D12_DESCRIPTOR_HEAP_DESC srvHeapDesc = {};
//	srvHeapDesc.NumDescriptors = 5;
//	srvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
//	srvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
//	ThrowIfFailed(md3dDevice->CreateDescriptorHeap(&srvHeapDesc, IID_PPV_ARGS(&mSrvDescriptorHeap)));
//
//	//
//	// Fill out the heap with actual descriptors.
//	//
//	CD3DX12_CPU_DESCRIPTOR_HANDLE hDescriptor(mSrvDescriptorHeap->GetCPUDescriptorHandleForHeapStart());
//
//	auto bricksTex = mTextures["bricksDiffuseMap"]->Resource;
//	auto tileTex = mTextures["tileDiffuseMap"]->Resource;
//	auto whiteTex = mTextures["defaultDiffuseMap"]->Resource;
//	auto skyTex = mTextures["skyCubeMap"]->Resource;
//
//	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
//	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
//	srvDesc.Format = bricksTex->GetDesc().Format;
//	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
//	srvDesc.Texture2D.MostDetailedMip = 0;
//	srvDesc.Texture2D.MipLevels = bricksTex->GetDesc().MipLevels;
//	srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;
//	md3dDevice->CreateShaderResourceView(bricksTex.Get(), &srvDesc, hDescriptor);
//
//	// next descriptor
//	hDescriptor.Offset(1, mCbvSrvDescriptorSize);
//
//	srvDesc.Format = tileTex->GetDesc().Format;
//	srvDesc.Texture2D.MipLevels = tileTex->GetDesc().MipLevels;
//	md3dDevice->CreateShaderResourceView(tileTex.Get(), &srvDesc, hDescriptor);
//
//	// next descriptor
//	hDescriptor.Offset(1, mCbvSrvDescriptorSize);
//
//	srvDesc.Format = whiteTex->GetDesc().Format;
//	srvDesc.Texture2D.MipLevels = whiteTex->GetDesc().MipLevels;
//	md3dDevice->CreateShaderResourceView(whiteTex.Get(), &srvDesc, hDescriptor);
//
//	// next descriptor
//	hDescriptor.Offset(1, mCbvSrvDescriptorSize);
//
//	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
//	srvDesc.TextureCube.MostDetailedMip = 0;
//	srvDesc.TextureCube.MipLevels = skyTex->GetDesc().MipLevels;
//	srvDesc.TextureCube.ResourceMinLODClamp = 0.0f;
//	srvDesc.Format = skyTex->GetDesc().Format;
//	md3dDevice->CreateShaderResourceView(skyTex.Get(), &srvDesc, hDescriptor);
//
//	mSkyTexHeapIndex = 3;
//}
//
//void CubeMapApp::BuildShadersAndInputLayout()
//{
//	const D3D_SHADER_MACRO alphaTestDefines[] =
//	{
//		"ALPHA_TEST", "1",
//		NULL, NULL
//	};
//
//	mShaders["standardVS"] = d3dUtil::CompileShader(L"Shaders\\Default.hlsl", nullptr, "VS", "vs_5_1");
//	mShaders["opaquePS"] = d3dUtil::CompileShader(L"Shaders\\Default.hlsl", nullptr, "PS", "ps_5_1");
//
//	mShaders["skyVS"] = d3dUtil::CompileShader(L"Shaders\\Sky.hlsl", nullptr, "VS", "vs_5_1");
//	mShaders["skyPS"] = d3dUtil::CompileShader(L"Shaders\\Sky.hlsl", nullptr, "PS", "ps_5_1");
//
//	mInputLayout =
//	{
//		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
//		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
//		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
//	};
//}
//
//void CubeMapApp::BuildShapeGeometry()
//{
//	GeometryGenerator geoGen;
//	GeometryGenerator::MeshData box = geoGen.CreateBox(1.0f, 1.0f, 1.0f, 3);
//	GeometryGenerator::MeshData grid = geoGen.CreateGrid(20.0f, 30.0f, 60, 40);
//	GeometryGenerator::MeshData sphere = geoGen.CreateSphere(0.5f, 20, 20);
//	GeometryGenerator::MeshData cylinder = geoGen.CreateCylinder(0.5f, 0.3f, 3.0f, 20, 20);
//
//	//
//	// We are concatenating all the geometry into one big vertex/index buffer.  So
//	// define the regions in the buffer each submesh covers.
//	//
//
//	// Cache the vertex offsets to each object in the concatenated vertex buffer.
//	UINT boxVertexOffset = 0;
//	UINT gridVertexOffset = (UINT)box.Vertices.size();
//	UINT sphereVertexOffset = gridVertexOffset + (UINT)grid.Vertices.size();
//	UINT cylinderVertexOffset = sphereVertexOffset + (UINT)sphere.Vertices.size();
//
//	// Cache the starting index for each object in the concatenated index buffer.
//	UINT boxIndexOffset = 0;
//	UINT gridIndexOffset = (UINT)box.Indices32.size();
//	UINT sphereIndexOffset = gridIndexOffset + (UINT)grid.Indices32.size();
//	UINT cylinderIndexOffset = sphereIndexOffset + (UINT)sphere.Indices32.size();
//
//	SubmeshGeometry boxSubmesh;
//	boxSubmesh.IndexCount = (UINT)box.Indices32.size();
//	boxSubmesh.StartIndexLocation = boxIndexOffset;
//	boxSubmesh.BaseVertexLocation = boxVertexOffset;
//
//	SubmeshGeometry gridSubmesh;
//	gridSubmesh.IndexCount = (UINT)grid.Indices32.size();
//	gridSubmesh.StartIndexLocation = gridIndexOffset;
//	gridSubmesh.BaseVertexLocation = gridVertexOffset;
//
//	SubmeshGeometry sphereSubmesh;
//	sphereSubmesh.IndexCount = (UINT)sphere.Indices32.size();
//	sphereSubmesh.StartIndexLocation = sphereIndexOffset;
//	sphereSubmesh.BaseVertexLocation = sphereVertexOffset;
//
//	SubmeshGeometry cylinderSubmesh;
//	cylinderSubmesh.IndexCount = (UINT)cylinder.Indices32.size();
//	cylinderSubmesh.StartIndexLocation = cylinderIndexOffset;
//	cylinderSubmesh.BaseVertexLocation = cylinderVertexOffset;
//
//	//
//	// Extract the vertex elements we are interested in and pack the
//	// vertices of all the meshes into one vertex buffer.
//	//
//
//	auto totalVertexCount =
//		box.Vertices.size() +
//		grid.Vertices.size() +
//		sphere.Vertices.size() +
//		cylinder.Vertices.size();
//
//	std::vector<Vertex> vertices(totalVertexCount);
//
//	UINT k = 0;
//	for (size_t i = 0; i < box.Vertices.size(); ++i, ++k)
//	{
//		vertices[k].Pos = box.Vertices[i].Position;
//		vertices[k].Normal = box.Vertices[i].Normal;
//		vertices[k].TexC = box.Vertices[i].TexC;
//	}
//
//	for (size_t i = 0; i < grid.Vertices.size(); ++i, ++k)
//	{
//		vertices[k].Pos = grid.Vertices[i].Position;
//		vertices[k].Normal = grid.Vertices[i].Normal;
//		vertices[k].TexC = grid.Vertices[i].TexC;
//	}
//
//	for (size_t i = 0; i < sphere.Vertices.size(); ++i, ++k)
//	{
//		vertices[k].Pos = sphere.Vertices[i].Position;
//		vertices[k].Normal = sphere.Vertices[i].Normal;
//		vertices[k].TexC = sphere.Vertices[i].TexC;
//	}
//
//	for (size_t i = 0; i < cylinder.Vertices.size(); ++i, ++k)
//	{
//		vertices[k].Pos = cylinder.Vertices[i].Position;
//		vertices[k].Normal = cylinder.Vertices[i].Normal;
//		vertices[k].TexC = cylinder.Vertices[i].TexC;
//	}
//
//	std::vector<std::uint16_t> indices;
//	indices.insert(indices.end(), std::begin(box.GetIndices16()), std::end(box.GetIndices16()));
//	indices.insert(indices.end(), std::begin(grid.GetIndices16()), std::end(grid.GetIndices16()));
//	indices.insert(indices.end(), std::begin(sphere.GetIndices16()), std::end(sphere.GetIndices16()));
//	indices.insert(indices.end(), std::begin(cylinder.GetIndices16()), std::end(cylinder.GetIndices16()));
//
//	const UINT vbByteSize = (UINT)vertices.size() * sizeof(Vertex);
//	const UINT ibByteSize = (UINT)indices.size() * sizeof(std::uint16_t);
//
//	auto geo = std::make_unique<MeshGeometry>();
//	geo->Name = "shapeGeo";
//
//	ThrowIfFailed(D3DCreateBlob(vbByteSize, &geo->VertexBufferCPU));
//	CopyMemory(geo->VertexBufferCPU->GetBufferPointer(), vertices.data(), vbByteSize);
//
//	ThrowIfFailed(D3DCreateBlob(ibByteSize, &geo->IndexBufferCPU));
//	CopyMemory(geo->IndexBufferCPU->GetBufferPointer(), indices.data(), ibByteSize);
//
//	geo->VertexBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(),
//		mCommandList.Get(), vertices.data(), vbByteSize, geo->VertexBufferUploader);
//
//	geo->IndexBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(),
//		mCommandList.Get(), indices.data(), ibByteSize, geo->IndexBufferUploader);
//
//	geo->VertexByteStride = sizeof(Vertex);
//	geo->VertexBufferByteSize = vbByteSize;
//	geo->IndexFormat = DXGI_FORMAT_R16_UINT;
//	geo->IndexBufferByteSize = ibByteSize;
//
//	geo->DrawArgs["box"] = boxSubmesh;
//	geo->DrawArgs["grid"] = gridSubmesh;
//	geo->DrawArgs["sphere"] = sphereSubmesh;
//	geo->DrawArgs["cylinder"] = cylinderSubmesh;
//
//	mGeometries[geo->Name] = std::move(geo);
//}
//
//void CubeMapApp::BuildSkullGeometry()
//{
//	std::ifstream fin("Models/skull.txt");
//
//	if (!fin)
//	{
//		MessageBox(0, L"Models/skull.txt not found.", 0, 0);
//		return;
//	}
//
//	UINT vcount = 0;
//	UINT tcount = 0;
//	std::string ignore;
//
//	fin >> ignore >> vcount;
//	fin >> ignore >> tcount;
//	fin >> ignore >> ignore >> ignore >> ignore;
//
//	XMFLOAT3 vMinf3(+MathHelper::Infinity, +MathHelper::Infinity, +MathHelper::Infinity);
//	XMFLOAT3 vMaxf3(-MathHelper::Infinity, -MathHelper::Infinity, -MathHelper::Infinity);
//
//	XMVECTOR vMin = XMLoadFloat3(&vMinf3);
//	XMVECTOR vMax = XMLoadFloat3(&vMaxf3);
//
//	std::vector<Vertex> vertices(vcount);
//	for (UINT i = 0; i < vcount; ++i)
//	{
//		fin >> vertices[i].Pos.x >> vertices[i].Pos.y >> vertices[i].Pos.z;
//		fin >> vertices[i].Normal.x >> vertices[i].Normal.y >> vertices[i].Normal.z;
//
//		vertices[i].TexC = { 0.0f, 0.0f };
//
//		XMVECTOR P = XMLoadFloat3(&vertices[i].Pos);
//
//		vMin = XMVectorMin(vMin, P);
//		vMax = XMVectorMax(vMax, P);
//	}
//
//	BoundingBox bounds;
//	XMStoreFloat3(&bounds.Center, 0.5f * (vMin + vMax));
//	XMStoreFloat3(&bounds.Extents, 0.5f * (vMax - vMin));
//
//	fin >> ignore;
//	fin >> ignore;
//	fin >> ignore;
//
//	std::vector<std::int32_t> indices(3 * tcount);
//	for (UINT i = 0; i < tcount; ++i)
//	{
//		fin >> indices[i * 3 + 0] >> indices[i * 3 + 1] >> indices[i * 3 + 2];
//	}
//
//	fin.close();
//
//	//
//	// Pack the indices of all the meshes into one index buffer.
//	//
//
//	const UINT vbByteSize = (UINT)vertices.size() * sizeof(Vertex);
//
//	const UINT ibByteSize = (UINT)indices.size() * sizeof(std::int32_t);
//
//	auto geo = std::make_unique<MeshGeometry>();
//	geo->Name = "skullGeo";
//
//	ThrowIfFailed(D3DCreateBlob(vbByteSize, &geo->VertexBufferCPU));
//	CopyMemory(geo->VertexBufferCPU->GetBufferPointer(), vertices.data(), vbByteSize);
//
//	ThrowIfFailed(D3DCreateBlob(ibByteSize, &geo->IndexBufferCPU));
//	CopyMemory(geo->IndexBufferCPU->GetBufferPointer(), indices.data(), ibByteSize);
//
//	geo->VertexBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(),
//		mCommandList.Get(), vertices.data(), vbByteSize, geo->VertexBufferUploader);
//
//	geo->IndexBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(),
//		mCommandList.Get(), indices.data(), ibByteSize, geo->IndexBufferUploader);
//
//	geo->VertexByteStride = sizeof(Vertex);
//	geo->VertexBufferByteSize = vbByteSize;
//	geo->IndexFormat = DXGI_FORMAT_R32_UINT;
//	geo->IndexBufferByteSize = ibByteSize;
//
//	SubmeshGeometry submesh;
//	submesh.IndexCount = (UINT)indices.size();
//	submesh.StartIndexLocation = 0;
//	submesh.BaseVertexLocation = 0;
//	submesh.Bounds = bounds;
//
//	geo->DrawArgs["skull"] = submesh;
//
//	mGeometries[geo->Name] = std::move(geo);
//}
//
//void CubeMapApp::BuildPSOs()
//{
//	D3D12_GRAPHICS_PIPELINE_STATE_DESC opaquePsoDesc;
//
//	//
//	// PSO for opaque objects.
//	//
//	ZeroMemory(&opaquePsoDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
//	opaquePsoDesc.InputLayout = { mInputLayout.data(), (UINT)mInputLayout.size() };
//	opaquePsoDesc.pRootSignature = mRootSignature.Get();
//	opaquePsoDesc.VS =
//	{
//		reinterpret_cast<BYTE*>(mShaders["standardVS"]->GetBufferPointer()),
//		mShaders["standardVS"]->GetBufferSize()
//	};
//	opaquePsoDesc.PS =
//	{
//		reinterpret_cast<BYTE*>(mShaders["opaquePS"]->GetBufferPointer()),
//		mShaders["opaquePS"]->GetBufferSize()
//	};
//	opaquePsoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
//	opaquePsoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
//	opaquePsoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
//	opaquePsoDesc.SampleMask = UINT_MAX;
//	opaquePsoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
//	opaquePsoDesc.NumRenderTargets = 1;
//	opaquePsoDesc.RTVFormats[0] = mBackBufferFormat;
//	opaquePsoDesc.SampleDesc.Count = m4xMsaaState ? 4 : 1;
//	opaquePsoDesc.SampleDesc.Quality = m4xMsaaState ? (m4xMsaaQuality - 1) : 0;
//	opaquePsoDesc.DSVFormat = mDepthStencilFormat;
//	ThrowIfFailed(md3dDevice->CreateGraphicsPipelineState(&opaquePsoDesc, IID_PPV_ARGS(&mPSOs["opaque"])));
//
//	//
//	// PSO for sky.
//	//
//
//	/* In particular, because the camera lies inside the sphere, we need to disable back face culling (or making
//	counterclockwise triangles front facing would also work), and we need to change the depth comparison function to
//	LESS_EQUAL so that the sky will pass the depth test */
//	D3D12_GRAPHICS_PIPELINE_STATE_DESC skyPsoDesc = opaquePsoDesc;
//
//	// The camera is inside the sky sphere, so just turn off culling.
//	skyPsoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
//
//	// Make sure the depth function is LESS_EQUAL and not just LESS.  
//	// Otherwise, the normalized depth values at z = 1 (NDC) will 
//	// fail the depth test if the depth buffer was cleared to 1.
//	skyPsoDesc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
//	skyPsoDesc.pRootSignature = mRootSignature.Get();
//	skyPsoDesc.VS =
//	{
//		reinterpret_cast<BYTE*>(mShaders["skyVS"]->GetBufferPointer()),
//		mShaders["skyVS"]->GetBufferSize()
//	};
//	skyPsoDesc.PS =
//	{
//		reinterpret_cast<BYTE*>(mShaders["skyPS"]->GetBufferPointer()),
//		mShaders["skyPS"]->GetBufferSize()
//	};
//	ThrowIfFailed(md3dDevice->CreateGraphicsPipelineState(&skyPsoDesc, IID_PPV_ARGS(&mPSOs["sky"])));
//
//}
//
//void CubeMapApp::BuildFrameResources()
//{
//	for (int i = 0; i < gNumFrameResources; ++i)
//	{
//		mFrameResources.push_back(std::make_unique<FrameResource>(md3dDevice.Get(),
//			1, (UINT)mAllRitems.size(), (UINT)mMaterials.size()));
//	}
//}
//
//void CubeMapApp::BuildMaterials()
//{
//	auto bricks0 = std::make_unique<Material>();
//	bricks0->Name = "bricks0";
//	bricks0->MatCBIndex = 0;
//	bricks0->DiffuseSrvHeapIndex = 0;
//	bricks0->DiffuseAlbedo = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
//	bricks0->FresnelR0 = XMFLOAT3(0.1f, 0.1f, 0.1f);
//	bricks0->Roughness = 0.3f;
//
//	auto tile0 = std::make_unique<Material>();
//	tile0->Name = "tile0";
//	tile0->MatCBIndex = 1;
//	tile0->DiffuseSrvHeapIndex = 1;
//	tile0->DiffuseAlbedo = XMFLOAT4(0.9f, 0.9f, 0.9f, 1.0f);
//	tile0->FresnelR0 = XMFLOAT3(0.2f, 0.2f, 0.2f);
//	tile0->Roughness = 0.1f;
//
//	auto mirror0 = std::make_unique<Material>();
//	mirror0->Name = "mirror0";
//	mirror0->MatCBIndex = 2;
//	mirror0->DiffuseSrvHeapIndex = 2;
//	mirror0->DiffuseAlbedo = XMFLOAT4(0.0f, 0.0f, 0.1f, 1.0f);
//	mirror0->FresnelR0 = XMFLOAT3(0.98f, 0.97f, 0.95f);
//	mirror0->Roughness = 0.1f;
//
//	auto skullMat = std::make_unique<Material>();
//	skullMat->Name = "skullMat";
//	skullMat->MatCBIndex = 3;
//	skullMat->DiffuseSrvHeapIndex = 2;
//	skullMat->DiffuseAlbedo = XMFLOAT4(0.8f, 0.8f, 0.8f, 1.0f);
//	skullMat->FresnelR0 = XMFLOAT3(0.2f, 0.2f, 0.2f);
//	skullMat->Roughness = 0.2f;
//
//	auto sky = std::make_unique<Material>();
//	sky->Name = "sky";
//	sky->MatCBIndex = 4;
//	sky->DiffuseSrvHeapIndex = 3;
//	sky->DiffuseAlbedo = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
//	sky->FresnelR0 = XMFLOAT3(0.1f, 0.1f, 0.1f);
//	sky->Roughness = 1.0f;
//
//	mMaterials["bricks0"] = std::move(bricks0);
//	mMaterials["tile0"] = std::move(tile0);
//	mMaterials["mirror0"] = std::move(mirror0);
//	mMaterials["skullMat"] = std::move(skullMat);
//	mMaterials["sky"] = std::move(sky);
//}
//
//void CubeMapApp::BuildRenderItems()
//{
//	auto skyRitem = std::make_unique<RenderItem>();
//	XMStoreFloat4x4(&skyRitem->World, XMMatrixScaling(5000.0f, 5000.0f, 5000.0f));
//	skyRitem->TexTransform = MathHelper::Identity4x4();
//	skyRitem->ObjCBIndex = 0;
//	skyRitem->Mat = mMaterials["sky"].get();
//	skyRitem->Geo = mGeometries["shapeGeo"].get();
//	skyRitem->PrimitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
//	skyRitem->IndexCount = skyRitem->Geo->DrawArgs["sphere"].IndexCount;
//	skyRitem->StartIndexLocation = skyRitem->Geo->DrawArgs["sphere"].StartIndexLocation;
//	skyRitem->BaseVertexLocation = skyRitem->Geo->DrawArgs["sphere"].BaseVertexLocation;
//
//	mRitemLayer[(int)RenderLayer::Sky].push_back(skyRitem.get());
//	mAllRitems.push_back(std::move(skyRitem));
//
//	auto boxRitem = std::make_unique<RenderItem>();
//	XMStoreFloat4x4(&boxRitem->World, XMMatrixScaling(2.0f, 1.0f, 2.0f) * XMMatrixTranslation(0.0f, 0.5f, 0.0f));
//	XMStoreFloat4x4(&boxRitem->TexTransform, XMMatrixScaling(1.0f, 1.0f, 1.0f));
//	boxRitem->ObjCBIndex = 1;
//	boxRitem->Mat = mMaterials["bricks0"].get();
//	boxRitem->Geo = mGeometries["shapeGeo"].get();
//	boxRitem->PrimitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
//	boxRitem->IndexCount = boxRitem->Geo->DrawArgs["box"].IndexCount;
//	boxRitem->StartIndexLocation = boxRitem->Geo->DrawArgs["box"].StartIndexLocation;
//	boxRitem->BaseVertexLocation = boxRitem->Geo->DrawArgs["box"].BaseVertexLocation;
//
//	mRitemLayer[(int)RenderLayer::Opaque].push_back(boxRitem.get());
//	mAllRitems.push_back(std::move(boxRitem));
//
//	auto skullRitem = std::make_unique<RenderItem>();
//	XMStoreFloat4x4(&skullRitem->World, XMMatrixScaling(0.4f, 0.4f, 0.4f) * XMMatrixTranslation(0.0f, 1.0f, 0.0f));
//	skullRitem->TexTransform = MathHelper::Identity4x4();
//	skullRitem->ObjCBIndex = 2;
//	skullRitem->Mat = mMaterials["skullMat"].get();
//	skullRitem->Geo = mGeometries["skullGeo"].get();
//	skullRitem->PrimitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
//	skullRitem->IndexCount = skullRitem->Geo->DrawArgs["skull"].IndexCount;
//	skullRitem->StartIndexLocation = skullRitem->Geo->DrawArgs["skull"].StartIndexLocation;
//	skullRitem->BaseVertexLocation = skullRitem->Geo->DrawArgs["skull"].BaseVertexLocation;
//
//	mRitemLayer[(int)RenderLayer::Opaque].push_back(skullRitem.get());
//	mAllRitems.push_back(std::move(skullRitem));
//
//	auto gridRitem = std::make_unique<RenderItem>();
//	gridRitem->World = MathHelper::Identity4x4();
//	XMStoreFloat4x4(&gridRitem->TexTransform, XMMatrixScaling(8.0f, 8.0f, 1.0f));
//	gridRitem->ObjCBIndex = 3;
//	gridRitem->Mat = mMaterials["tile0"].get();
//	gridRitem->Geo = mGeometries["shapeGeo"].get();
//	gridRitem->PrimitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
//	gridRitem->IndexCount = gridRitem->Geo->DrawArgs["grid"].IndexCount;
//	gridRitem->StartIndexLocation = gridRitem->Geo->DrawArgs["grid"].StartIndexLocation;
//	gridRitem->BaseVertexLocation = gridRitem->Geo->DrawArgs["grid"].BaseVertexLocation;
//
//	mRitemLayer[(int)RenderLayer::Opaque].push_back(gridRitem.get());
//	mAllRitems.push_back(std::move(gridRitem));
//
//	XMMATRIX brickTexTransform = XMMatrixScaling(1.5f, 2.0f, 1.0f);
//	UINT objCBIndex = 4;
//	for (int i = 0; i < 5; ++i)
//	{
//		auto leftCylRitem = std::make_unique<RenderItem>();
//		auto rightCylRitem = std::make_unique<RenderItem>();
//		auto leftSphereRitem = std::make_unique<RenderItem>();
//		auto rightSphereRitem = std::make_unique<RenderItem>();
//
//		XMMATRIX leftCylWorld = XMMatrixTranslation(-5.0f, 1.5f, -10.0f + i * 5.0f);
//		XMMATRIX rightCylWorld = XMMatrixTranslation(+5.0f, 1.5f, -10.0f + i * 5.0f);
//
//		XMMATRIX leftSphereWorld = XMMatrixTranslation(-5.0f, 3.5f, -10.0f + i * 5.0f);
//		XMMATRIX rightSphereWorld = XMMatrixTranslation(+5.0f, 3.5f, -10.0f + i * 5.0f);
//
//		XMStoreFloat4x4(&leftCylRitem->World, rightCylWorld);
//		XMStoreFloat4x4(&leftCylRitem->TexTransform, brickTexTransform);
//		leftCylRitem->ObjCBIndex = objCBIndex++;
//		leftCylRitem->Mat = mMaterials["bricks0"].get();
//		leftCylRitem->Geo = mGeometries["shapeGeo"].get();
//		leftCylRitem->PrimitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
//		leftCylRitem->IndexCount = leftCylRitem->Geo->DrawArgs["cylinder"].IndexCount;
//		leftCylRitem->StartIndexLocation = leftCylRitem->Geo->DrawArgs["cylinder"].StartIndexLocation;
//		leftCylRitem->BaseVertexLocation = leftCylRitem->Geo->DrawArgs["cylinder"].BaseVertexLocation;
//
//		XMStoreFloat4x4(&rightCylRitem->World, leftCylWorld);
//		XMStoreFloat4x4(&rightCylRitem->TexTransform, brickTexTransform);
//		rightCylRitem->ObjCBIndex = objCBIndex++;
//		rightCylRitem->Mat = mMaterials["bricks0"].get();
//		rightCylRitem->Geo = mGeometries["shapeGeo"].get();
//		rightCylRitem->PrimitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
//		rightCylRitem->IndexCount = rightCylRitem->Geo->DrawArgs["cylinder"].IndexCount;
//		rightCylRitem->StartIndexLocation = rightCylRitem->Geo->DrawArgs["cylinder"].StartIndexLocation;
//		rightCylRitem->BaseVertexLocation = rightCylRitem->Geo->DrawArgs["cylinder"].BaseVertexLocation;
//
//		XMStoreFloat4x4(&leftSphereRitem->World, leftSphereWorld);
//		leftSphereRitem->TexTransform = MathHelper::Identity4x4();
//		leftSphereRitem->ObjCBIndex = objCBIndex++;
//		leftSphereRitem->Mat = mMaterials["mirror0"].get();
//		leftSphereRitem->Geo = mGeometries["shapeGeo"].get();
//		leftSphereRitem->PrimitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
//		leftSphereRitem->IndexCount = leftSphereRitem->Geo->DrawArgs["sphere"].IndexCount;
//		leftSphereRitem->StartIndexLocation = leftSphereRitem->Geo->DrawArgs["sphere"].StartIndexLocation;
//		leftSphereRitem->BaseVertexLocation = leftSphereRitem->Geo->DrawArgs["sphere"].BaseVertexLocation;
//
//		XMStoreFloat4x4(&rightSphereRitem->World, rightSphereWorld);
//		rightSphereRitem->TexTransform = MathHelper::Identity4x4();
//		rightSphereRitem->ObjCBIndex = objCBIndex++;
//		rightSphereRitem->Mat = mMaterials["mirror0"].get();
//		rightSphereRitem->Geo = mGeometries["shapeGeo"].get();
//		rightSphereRitem->PrimitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
//		rightSphereRitem->IndexCount = rightSphereRitem->Geo->DrawArgs["sphere"].IndexCount;
//		rightSphereRitem->StartIndexLocation = rightSphereRitem->Geo->DrawArgs["sphere"].StartIndexLocation;
//		rightSphereRitem->BaseVertexLocation = rightSphereRitem->Geo->DrawArgs["sphere"].BaseVertexLocation;
//
//		mRitemLayer[(int)RenderLayer::Opaque].push_back(leftCylRitem.get());
//		mRitemLayer[(int)RenderLayer::Opaque].push_back(rightCylRitem.get());
//		mRitemLayer[(int)RenderLayer::Opaque].push_back(leftSphereRitem.get());
//		mRitemLayer[(int)RenderLayer::Opaque].push_back(rightSphereRitem.get());
//
//		mAllRitems.push_back(std::move(leftCylRitem));
//		mAllRitems.push_back(std::move(rightCylRitem));
//		mAllRitems.push_back(std::move(leftSphereRitem));
//		mAllRitems.push_back(std::move(rightSphereRitem));
//	}
//}
//
//void CubeMapApp::DrawRenderItems(ID3D12GraphicsCommandList* cmdList, const std::vector<RenderItem*>& ritems)
//{
//	UINT objCBByteSize = d3dUtil::CalcConstantBufferByteSize(sizeof(ObjectConstants));
//
//	auto objectCB = mCurrFrameResource->ObjectCB->Resource();
//
//	// For each render item...
//	for (size_t i = 0; i < ritems.size(); ++i)
//	{
//		auto ri = ritems[i];
//
//		cmdList->IASetVertexBuffers(0, 1, &ri->Geo->VertexBufferView());
//		cmdList->IASetIndexBuffer(&ri->Geo->IndexBufferView());
//		cmdList->IASetPrimitiveTopology(ri->PrimitiveType);
//
//		D3D12_GPU_VIRTUAL_ADDRESS objCBAddress = objectCB->GetGPUVirtualAddress() + ri->ObjCBIndex * objCBByteSize;
//
//		cmdList->SetGraphicsRootConstantBufferView(0, objCBAddress);
//
//		cmdList->DrawIndexedInstanced(ri->IndexCount, 1, ri->StartIndexLocation, ri->BaseVertexLocation, 0);
//	}
//}
//
//std::array<const CD3DX12_STATIC_SAMPLER_DESC, 6> CubeMapApp::GetStaticSamplers()
//{
//	// Applications usually only need a handful of samplers.  So just define them all up front
//	// and keep them available as part of the root signature.  
//
//	const CD3DX12_STATIC_SAMPLER_DESC pointWrap(
//		0, // shaderRegister
//		D3D12_FILTER_MIN_MAG_MIP_POINT, // filter
//		D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressU
//		D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressV
//		D3D12_TEXTURE_ADDRESS_MODE_WRAP); // addressW
//
//	const CD3DX12_STATIC_SAMPLER_DESC pointClamp(
//		1, // shaderRegister
//		D3D12_FILTER_MIN_MAG_MIP_POINT, // filter
//		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressU
//		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressV
//		D3D12_TEXTURE_ADDRESS_MODE_CLAMP); // addressW
//
//	const CD3DX12_STATIC_SAMPLER_DESC linearWrap(
//		2, // shaderRegister
//		D3D12_FILTER_MIN_MAG_MIP_LINEAR, // filter
//		D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressU
//		D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressV
//		D3D12_TEXTURE_ADDRESS_MODE_WRAP); // addressW
//
//	const CD3DX12_STATIC_SAMPLER_DESC linearClamp(
//		3, // shaderRegister
//		D3D12_FILTER_MIN_MAG_MIP_LINEAR, // filter
//		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressU
//		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressV
//		D3D12_TEXTURE_ADDRESS_MODE_CLAMP); // addressW
//
//	const CD3DX12_STATIC_SAMPLER_DESC anisotropicWrap(
//		4, // shaderRegister
//		D3D12_FILTER_ANISOTROPIC, // filter
//		D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressU
//		D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressV
//		D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressW
//		0.0f,                             // mipLODBias
//		8);                               // maxAnisotropy
//
//	const CD3DX12_STATIC_SAMPLER_DESC anisotropicClamp(
//		5, // shaderRegister
//		D3D12_FILTER_ANISOTROPIC, // filter
//		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressU
//		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressV
//		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressW
//		0.0f,                              // mipLODBias
//		8);                                // maxAnisotropy
//
//	return {
//		pointWrap, pointClamp,
//		linearWrap, linearClamp,
//		anisotropicWrap, anisotropicClamp };
//}