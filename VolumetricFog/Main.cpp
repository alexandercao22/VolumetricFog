#include <Windows.h>
#include <iostream>
#include <d3d11.h>
#include <chrono>

#include "WindowHelper.h"
#include "D3D11Helper.h"
#include "PipelineHelper.h"
#include "SceneHelper.h"
#include "RenderHelper.h"

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPWSTR    lpCmdLine,
	_In_ int       nCmdShow)
{
	std::chrono::steady_clock::time_point time = std::chrono::steady_clock::now(); // Time since start
	const char* fileName = "error_log.txt";
	std::ofstream fileStream(fileName);
	std::cerr.rdbuf(fileStream.rdbuf());

	float ratio = 9.0f / 16.0f;
	const UINT WIDTH = 1920;// 1920
	const UINT HEIGHT = WIDTH * ratio;// 1080
	HWND window;

	if (!SetupWindow(hInstance, WIDTH, HEIGHT, nCmdShow, window))
	{
		std::cerr << "Failed to setup window!" << std::endl;
		return -1;
	}

	ID3D11Device* device;
	ID3D11DeviceContext* immediateContext;
	IDXGISwapChain* swapChain;
	ID3D11RenderTargetView* rtv; // Main camera RTV
	D3D11_VIEWPORT viewport; // Main camera viewport

	if (!SetupD3D11(WIDTH, HEIGHT, window, device, immediateContext, swapChain, rtv, viewport))
	{
		std::cerr << "Failed to setup d3d11!" << std::endl;
		return -1;
	}

	// DepthStencilView
	DepthBufferD3D11 depthStencil(device, WIDTH, HEIGHT, true);

	// Main Camera
	MainCamera mainCamera;
	CreateMainCamera(immediateContext, device, &mainCamera, &window);

	std::vector<MeshBoundPositions> meshBoundPositions; // Save min and max positions of all meshes for frustum culling

	// OBJ Meshes
	ObjInfo objInfo[] = {
		//  { "OBJ adress", { translation }, { rotation }, translateFirst, scale }
			{ "sphere.obj", { 0.0f, -5.0f, 0.0f } },
			{ "cubeWithTexture.obj", { -5.0f, 5.0f, 0.0f }, { 0.0f, DirectX::XM_PIDIV4, 0.0f }, true },
			{ "teapot.obj" },
			{ "sphere.obj", { 0.0f, 0.0f, 10.0f }, { 0.0f, 0.0f, 0.0f }, true, 0.7f },
			{ "error_text.obj", { -7.0f, 0.0f, 0.0f }, { 0.0f, -DirectX::XM_PIDIV2, 0.0f }, false, 5 },
			{ "room1.obj", { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f }, false, 2 },
			{ "Duck.obj", { 7.0f, 0.0f, 0.0f }, { 0.0f, DirectX::XM_PI, 0.0f }, false },
	};
	const size_t totalMeshes = sizeof(objInfo) / sizeof(ObjInfo);
	MeshD3D11 meshes[totalMeshes];
	ObjToMeshes(device, meshes, objInfo, totalMeshes, meshBoundPositions);
	SamplerD3D11 meshSampler(device, D3D11_TEXTURE_ADDRESS_WRAP);
	ID3D11SamplerState* meshSamplerState = meshSampler.GetSamplerState();

	// Deferred Rendering
	ShaderD3D11 deferredVS(device, ShaderType::VERTEX_SHADER, L"DeferredVS.cso");
	ShaderD3D11 deferredPS(device, ShaderType::PIXEL_SHADER, L"DeferredPS.cso");
	ShaderD3D11 deferredCS(device, ShaderType::COMPUTE_SHADER, L"DeferredCS.cso");
	const UINT NR_OF_GBUFFERS = 5;
	ID3D11Texture2D* DRtexture[NR_OF_GBUFFERS] = { nullptr };
	ID3D11ShaderResourceView* DRsrv[NR_OF_GBUFFERS] = { nullptr };
	ID3D11RenderTargetView* DRrtv[NR_OF_GBUFFERS] = { nullptr };
	ID3D11UnorderedAccessView* DRuav = nullptr;
	DeferredRenderingSetup(immediateContext, device, WIDTH, HEIGHT, NR_OF_GBUFFERS, DRtexture, DRsrv, DRrtv, DRuav, swapChain);

	// Input Layout
	InputLayoutD3D11 inputLayout;
	InputLayoutStruct semanticNames[] =
	{
		{ "POSITION", DXGI_FORMAT_R32G32B32_FLOAT },
		{ "NORMAL", DXGI_FORMAT_R32G32B32_FLOAT },
		{ "UV", DXGI_FORMAT_R32G32_FLOAT }
	};
	CreateInputLayout(device, inputLayout, semanticNames, sizeof(semanticNames) / sizeof(InputLayoutStruct), &deferredVS);

	// Lights
	SpotLightCollectionD3D11 spotLights;
	DirectionalLight directionLight;
	ConstantBufferD3D11 cBufferCS; // C-buffer for miscellaneous light stuff
	UINT totalSpotLights;
	std::array<float, 4> borderColour = { 1.0f, 1.0f, 1.0f, 1.0f };
	SamplerD3D11 shadowSampler(device, D3D11_TEXTURE_ADDRESS_BORDER, borderColour);
	ShaderD3D11 shadowVS(device, ShaderType::VERTEX_SHADER, L"ShadowVS.cso");
	SetupLights(immediateContext, device, &spotLights, &mainCamera, &cBufferCS, &totalSpotLights, &directionLight);  // ID3D11Buffer Memory Leak 4

	// Dynamic Cubic Environment Mapping
	CameraD3D11 cubeCams[6];
	ID3D11Buffer* cubeCamBuffer[6]; // ViewProj matrices
	ID3D11Buffer* camPosBuffer = nullptr; // Main camera position
	float cubeTextureSize = 1024.0f;
	D3D11_VIEWPORT cubeView;
	ShaderD3D11 reflectiveCubeVS(device, ShaderType::VERTEX_SHADER, L"ReflectiveCubeVS.cso");
	ShaderD3D11 reflectiveCubePS(device, ShaderType::PIXEL_SHADER, L"ReflectiveCubePS.cso");
	DepthBufferD3D11 cubeDepth(device, (UINT)cubeTextureSize, (UINT)cubeTextureSize, false);
	SamplerD3D11 cubeSampler(device, D3D11_TEXTURE_ADDRESS_WRAP);
	ID3D11SamplerState* cubeSamplerState = cubeSampler.GetSamplerState();
	RenderTargetD3D11 cubeMappingRTV; // Contains the texture cube
	ID3D11ShaderResourceView* cubeMappingSRV; // Texture cube as SRV
	DynamicCubeMapping(immediateContext, device, cubeCams, cubeCamBuffer, cubeView, cubeTextureSize, &cubeMappingRTV, cubeMappingSRV);

	// Reflective mesh
	ObjInfo reflectiveObj = { "sphere.obj" };
	MeshD3D11 reflectiveMesh;
	ObjToMeshes(device, &reflectiveMesh, &reflectiveObj, 1, meshBoundPositions);

	// Deferred rendering for reflection
	ShaderD3D11 reflectiveCubeCS(device, ShaderType::COMPUTE_SHADER, L"ReflectiveCubeCS.cso");
	ID3D11Texture2D* cubeTextureG[NR_OF_GBUFFERS] = { nullptr };
	ID3D11ShaderResourceView* cubeSRVg[NR_OF_GBUFFERS] = { nullptr };
	ID3D11RenderTargetView* cubeRTVg[NR_OF_GBUFFERS] = { nullptr };
	ID3D11Texture2D* cubeTextures = nullptr;
	ID3D11ShaderResourceView* cubeTextureSRV = nullptr;
	ID3D11UnorderedAccessView* cubeUAVs[6] = { nullptr };
	SetupReflection(device, NR_OF_GBUFFERS, cubeTextureG, cubeSRVg, cubeRTVg, cubeUAVs, cubeTextures, cubeTextureSRV, cubeTextureSize);

	// Billboarded Particles
	int nrOfParticles = 1000;
	float particleSize = 0.05f;
	StructuredBufferD3D11 particleBuffer;
	ShaderD3D11 particleCS(device, ShaderType::COMPUTE_SHADER, L"ParticleCS.cso");
	ShaderD3D11 particleVS(device, ShaderType::VERTEX_SHADER, L"ParticleVS.cso");
	ShaderD3D11 particleGS(device, ShaderType::GEOMETRY_SHADER, L"ParticleGS.cso");
	ShaderD3D11 particlePS(device, ShaderType::PIXEL_SHADER, L"ParticlePS.cso");
	ConstantBufferD3D11 particleConstantBuffer;
	SetupParticles(device, &particleBuffer, &mainCamera, &particleConstantBuffer, nrOfParticles, particleSize);

	// LOD tessellation
	ShaderD3D11 tessellationHS(device, ShaderType::HULL_SHADER, L"TessellationHS.cso");
	ShaderD3D11 tessellationDS(device, ShaderType::DOMAIN_SHADER, L"TessellationDS.cso");
	ConstantBufferD3D11 tessellationPositions;
	DirectX::XMFLOAT4 moveObj = { 0.0f, 5.0f, 10.0f, 1.0f };
	ObjInfo tessellationObj = { "LowIcoSphere.obj", { moveObj.x, moveObj.y, moveObj.z } };
	MeshD3D11 tessellationMesh;
	ObjToMeshes(device, &tessellationMesh, &tessellationObj, 1, meshBoundPositions);
	SetupTessellation(device, &tessellationPositions, &mainCamera, moveObj);

	// Frustum culling
	MeshD3D11 frustumMesh;
	frustumMesh.Name("Frustum");
	ConstantBufferD3D11 frustumCbuffer;
	QuadTree<MeshD3D11> quadTree;
	DirectX::BoundingFrustum cameraFrustum = mainCamera.GetFrustum();
	MeshD3D11 meshBoundingBoxLines[totalMeshes + 2];
	SetupCulling(device, &mainCamera, &quadTree, meshes, totalMeshes, meshBoundPositions, &frustumMesh, &cameraFrustum,
		&frustumCbuffer, meshBoundingBoxLines);

	// Bounding boxes
	InputLayoutD3D11 cullingInputLayout;
	InputLayoutStruct cullingSemanticNames[] =
	{
		{ "POSITION", DXGI_FORMAT_R32G32B32_FLOAT },
		{ "COLOUR", DXGI_FORMAT_R32G32B32_FLOAT }
	};
	ShaderD3D11 cullingVS(device, ShaderType::VERTEX_SHADER, L"CullingVS.cso");
	ShaderD3D11 cullingPS(device, ShaderType::PIXEL_SHADER, L"CullingPS.cso");
	CreateInputLayout(device, cullingInputLayout, cullingSemanticNames, std::size(cullingSemanticNames), &cullingVS);

	// Volumetric fog ray-marching compute shader
	ShaderD3D11 volFogRayCS(device, ShaderType::COMPUTE_SHADER, L"VolumetricFogRayCS.cso");
	ConstantBufferD3D11 rayConstBuffer;
	ConstantBufferD3D11 rayConstData;
	SetupRayMarchingVolFog(device, &rayConstBuffer, &mainCamera, &rayConstData, WIDTH, HEIGHT);

	MSG msg = { };
	ShowCursor(FALSE); // Hide cursor
	SetCursorPos(WIDTH / 2, HEIGHT / 2); // Center cursor
	ConstantBufferD3D11 camPosConstBuffer;
	DirectX::XMFLOAT3 tempPos0 = mainCamera.GetPosition();
	camPosConstBuffer.Initialize(device, sizeof(DirectX::XMFLOAT4), &tempPos0);
	float deltaTime = 0.0f;
	while (!(GetKeyState(VK_ESCAPE) & 0x8000) && msg.message != WM_QUIT)
	{
		std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();
		time = start;

		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		float t = std::chrono::duration<float>(time.time_since_epoch()).count();
		UpdatePerFrame(immediateContext, device, totalSpotLights, &mainCamera, &cBufferCS, camPosBuffer,
			&camPosConstBuffer, &particleConstantBuffer, particleSize, &tessellationPositions, moveObj, &frustumMesh,
			&frustumCbuffer, &cameraFrustum, &rayConstBuffer, &rayConstData, t, deltaTime);

		RenderShadowMaps(immediateContext, inputLayout.GetInputLayout(), &spotLights, &shadowVS, &cubeView, meshes, totalMeshes,
			&reflectiveMesh, &directionLight, &tessellationMesh);

		RenderReflection(immediateContext, &reflectiveCubeVS, cubeCamBuffer, &cubeView, cubeDepth, &meshSamplerState, meshes, totalMeshes,
			cubeSRVg, cubeRTVg, NR_OF_GBUFFERS, cubeUAVs, &reflectiveCubeCS, &deferredPS, &spotLights, &shadowSampler, &directionLight);

		DeferredRendering(immediateContext, &depthStencil, &deferredCS, DRsrv, DRrtv, NR_OF_GBUFFERS, DRuav, viewport,
			&deferredVS, &deferredPS, inputLayout.GetInputLayout(), &meshSamplerState, meshes, totalMeshes, &reflectiveMesh,
			&spotLights, &shadowSampler, &directionLight, rtv, &reflectiveCubePS, camPosBuffer, cubeTextureSRV, cubeSamplerState,
			&particleBuffer, &particleCS, &particleVS, &particleGS, &particlePS, &particleConstantBuffer,
			&tessellationHS, &tessellationDS, &tessellationMesh, &tessellationPositions,
			cullingInputLayout.GetInputLayout(), &cullingVS, &cullingPS, &frustumMesh, &frustumCbuffer,
			&quadTree, &cameraFrustum, meshBoundingBoxLines, &volFogRayCS, &rayConstBuffer, &rayConstData);

		std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
		std::chrono::duration<float> runtime = end - start;
		deltaTime = runtime.count();

		MainCameraMovement(immediateContext, &mainCamera, deltaTime, &window);
		swapChain->Present(0, 0);
	}

	// Release COM-objects
	ID3D11Buffer* testBuffer = particleConstantBuffer.GetBuffer();
	testBuffer->Release();
	testBuffer = tessellationPositions.GetBuffer();
	testBuffer->Release();
	testBuffer = frustumCbuffer.GetBuffer();
	testBuffer->Release();
	for (int i = 0; i < totalMeshes; i++)
	{
		for (int j = 0; j < meshes[i].GetNrOfSubMeshes(); j++)
		{
			ID3D11ShaderResourceView* amb = meshes[i].GetAmbientSRV(j);
			ID3D11ShaderResourceView* dif = meshes[i].GetDiffuseSRV(j);
			ID3D11ShaderResourceView* spe = meshes[i].GetSpecularSRV(j);
			if (amb != nullptr)
				amb->Release();
			if (dif != nullptr)
				dif->Release();
			if (spe != nullptr)
				spe->Release();
		}
	}
	for (int i = 0; i < reflectiveMesh.GetNrOfSubMeshes(); i++)
	{
		ID3D11ShaderResourceView* amb = reflectiveMesh.GetAmbientSRV(i);
		ID3D11ShaderResourceView* dif = reflectiveMesh.GetDiffuseSRV(i);
		ID3D11ShaderResourceView* spe = reflectiveMesh.GetSpecularSRV(i);
		if (amb != nullptr)
			amb->Release();
		if (dif != nullptr)
			dif->Release();
		if (spe != nullptr)
			spe->Release();
	}
	for (int i = 0; i < tessellationMesh.GetNrOfSubMeshes(); i++)
	{
		ID3D11ShaderResourceView* amb = tessellationMesh.GetAmbientSRV(i);
		ID3D11ShaderResourceView* dif = tessellationMesh.GetDiffuseSRV(i);
		ID3D11ShaderResourceView* spe = tessellationMesh.GetSpecularSRV(i);
		if (amb != nullptr)
			amb->Release();
		if (dif != nullptr)
			dif->Release();
		if (spe != nullptr)
			spe->Release();
	}
	for (int i = 0; i < 6; i++)
	{
		cubeCamBuffer[i]->Release();
		cubeUAVs[i]->Release();
	}
	for (int i = 0; i < NR_OF_GBUFFERS; i++)
	{
		DRtexture[i]->Release();
		DRsrv[i]->Release();
		DRrtv[i]->Release();
		cubeTextureG[i]->Release();
		cubeSRVg[i]->Release();
		cubeRTVg[i]->Release();
	}
	cubeTextures->Release();
	cubeTextureSRV->Release();
	DRuav->Release();
	cubeMappingSRV->Release();
	camPosBuffer->Release();
	rtv->Release();
	swapChain->Release();
	immediateContext->Release();
	device->Release();

	return 0;
}

/*
* Keybinds
* w - Move forward
* a - Move left
* s - Move backward
* d - Move right
* lshift - Descend
* space - Ascend
* j - Toggle shadows
* k - Toggle full ambient
* l - Toggle lights
* v - Detach/attach frustum
* b - Toggle bounding boxes
*/
