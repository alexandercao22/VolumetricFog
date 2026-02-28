#include "SceneHelper.h"

void CreateMainCamera(ID3D11DeviceContext* context, ID3D11Device*& device, MainCamera* mainCamera, HWND* window)
{
	RECT rect;
	if (GetWindowRect(*window, &rect))
	{
		float width = (float)rect.right;
		float height = (float)rect.bottom;

		MatrixInfo matrixInfo;
		matrixInfo.position = DirectX::XMFLOAT3(0.0f, 0.0f, -4.0f);
		matrixInfo.forward = DirectX::XMFLOAT3(0.0f, 0.0f, 1.0f);
		matrixInfo.right = DirectX::XMFLOAT3(1.0f, 0.0f, 0.0f);
		matrixInfo.up = DirectX::XMFLOAT3(0.0f, 1.0f, 0.0f);
		matrixInfo.fovAngleY = DirectX::XM_PIDIV2;
		matrixInfo.aspectRatio = width / height;
		matrixInfo.nearZ = 0.1f;
		matrixInfo.farZ = 100.0f;

		mainCamera->Initialize(device, matrixInfo);
		ID3D11Buffer* buffer = mainCamera->GetConstantBuffer();
		context->VSSetConstantBuffers(0, 1, &buffer);
		buffer->Release();
	}
}

void CreateInputLayout(ID3D11Device*& device, InputLayoutD3D11& inputLayout,
	InputLayoutStruct* semanticNames, size_t totalLayouts, ShaderD3D11* deferredVS)
{
	for (int i = 0; i < totalLayouts; i++)
	{
		inputLayout.AddInputElement(semanticNames[i].semanticName, semanticNames[i].format);
	}
	inputLayout.FinalizeInputLayout(device, deferredVS->GetShaderByteData(), deferredVS->GetShaderByteSize());
}

void MainCameraMovement(ID3D11DeviceContext* context, MainCamera* mainCamera, float deltaTime, HWND* window)
{
	static DirectX::XMFLOAT3 startPos = mainCamera->GetPosition();
	static DirectX::XMFLOAT3 forward = mainCamera->GetForward();
	static DirectX::XMFLOAT3 right = mainCamera->GetRight();
	static DirectX::XMFLOAT3 up = mainCamera->GetUp();

	if (GetForegroundWindow() != *window) // Check if the active window is the project window
	{
		return;
	}

	// Camera rotation
	RECT rect;
	if (GetWindowRect(*window, &rect)) // Get the dimensions of the window
	{
		int centerX = rect.right / 2;
		int centerY = rect.bottom / 2;

		POINT point;
		if (GetCursorPos(&point))
		{
			// Determines how fast the camera should rotate.
			// The faster the mouse is moving, the higher the values sensX and sensY
			// is going to be depending on which axis the mouse moved.
			int sensX = point.x - centerX;
			int sensY = point.y - centerY;

			if (point.x <= centerX - 1) // Look Left
			{
				mainCamera->RotateUp(deltaTime * sensX);
			}
			if (point.x >= centerX + 1) // Look Right
			{
				mainCamera->RotateUp(deltaTime * sensX);
			}
			if (point.y <= centerY - 1) // Look Up
			{
				if (mainCamera->GetForward().y <= 0.99f)
					mainCamera->RotateRight(deltaTime * sensY);
			}
			if (point.y >= centerY + 1) // Look Down
			{
				if (mainCamera->GetForward().y >= -0.99f)
					mainCamera->RotateRight(deltaTime * sensY);
			}
		}
		SetCursorPos(centerX, centerY);
	}

	// Camera movement
	float movementSpeed = 1.0f * deltaTime;
	if (GetKeyState(VK_CONTROL) & 0x8000 || GetKeyState(VK_XBUTTON2) & 0x8000) // Speed up
	{
		movementSpeed *= 3.0f;
	}
	if (GetKeyState('W') & 0x8000) // Move Forwards
	{
		mainCamera->MoveForward(movementSpeed);
	}
	if (GetKeyState('S') & 0x8000) // Move Backwards
	{
		mainCamera->MoveForward(-movementSpeed);
	}
	if (GetKeyState('A') & 0x8000) // Move Left
	{
		mainCamera->MoveRight(-movementSpeed);
	}
	if (GetKeyState('D') & 0x8000) // Move Right
	{
		mainCamera->MoveRight(movementSpeed);
	}
	if (GetKeyState(VK_SPACE) & 0x8000) // Move Up
	{
		mainCamera->MoveUp(movementSpeed);
	}
	if (GetKeyState(VK_LSHIFT) & 0x8000) // Move Down
	{
		mainCamera->MoveUp(-movementSpeed);
	}
	if (GetKeyState(VK_CONTROL) & 0x8000 && GetKeyState(VK_MENU) & 0x8000) // Reset Position and Rotation
	{
		mainCamera->SetPosition(startPos);
		mainCamera->SetRotation(forward, right, up);
	}

	//if (GetKeyState('O') & 0x8000)
	//{
	//	// Put a debug point on this row
	//	std::cerr << "--------------------Manual Debug--------------------\n";
	//}

	mainCamera->UpdateInternalConstantBuffer(context); // Updates the new values of the camera
	mainCamera->RotateUp(0.0f);
	mainCamera->RotateRight(0.0f);
}

void DynamicCubeMapping(ID3D11DeviceContext* context, ID3D11Device*& device, CameraD3D11* cubeCams, ID3D11Buffer** cubeCamBuffer,
	D3D11_VIEWPORT& cubeView, float cubeTextureSize, RenderTargetD3D11* cubeMappingRTV, ID3D11ShaderResourceView*& cubeMappingSRV)
{
	// Cameras
	ProjectionInfo projInfo;
	projInfo.fovAngleY = DirectX::XM_PIDIV2;
	projInfo.aspectRatio = 1.0f;
	projInfo.nearZ = 0.1f;
	projInfo.farZ = 100.0f;

	//						  {    +x,    -x,    +y,    -y,    +z,    -z }
	float upRotations[6] =    {  PID2, -PID2,  0.0f,  0.0f,  0.0f,    PI }; // Rotations around local up vector
	float rightRotations[6] = {  0.0f,  0.0f, -PID2,  PID2,  0.0f,  0.0f }; // Rotations around local right vector

	for (int i = 0; i < 6; i++)
	{
		cubeCams[i].Initialize(device, projInfo, DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f));
		cubeCams[i].RotateUp(upRotations[i]);
		cubeCams[i].RotateRight(rightRotations[i]);
		cubeCams[i].UpdateInternalConstantBuffer(context);
		cubeCamBuffer[i] = cubeCams[i].GetConstantBuffer();
	}

	// Viewport
	cubeView.TopLeftX = 0.0f;
	cubeView.TopLeftY = 0.0f;
	cubeView.Width = cubeTextureSize;
	cubeView.Height = cubeTextureSize;
	cubeView.MinDepth = 0.0f;
	cubeView.MaxDepth = 1.0f;

	// RTV & SRV
	cubeMappingRTV->Initialize(device, (UINT)cubeTextureSize, (UINT)cubeTextureSize, DXGI_FORMAT_R8G8B8A8_UNORM, true);
	cubeMappingSRV = cubeMappingRTV->GetSRV();
}

void ObjToMeshes(ID3D11Device*& device, MeshD3D11* mesh, ObjInfo* objInfo, size_t totalMeshes,
	std::vector<MeshBoundPositions>& meshBoundPositions)
{
	for (int i = 0; i < totalMeshes; i++) // For every mesh
	{
		objl::Loader obj;
		obj.LoadFile(objInfo[i].address);

		MeshData meshData;
		meshData.vertexInfo.sizeOfVertex = sizeof(objl::Vertex);
		meshData.vertexInfo.nrOfVerticesInBuffer = obj.LoadedVertices.size();

		// Store vertices and indices
		MeshBoundPositions meshBound;
		std::vector<objl::Vertex> vertices;
		for (int j = 0; j < meshData.vertexInfo.nrOfVerticesInBuffer; j++) // For every vertex in a mesh
		{
			DirectX::XMVECTOR pos = { obj.LoadedVertices[j].Position.X, obj.LoadedVertices[j].Position.Y, obj.LoadedVertices[j].Position.Z };
			DirectX::XMVECTOR nor = { obj.LoadedVertices[j].Normal.X, obj.LoadedVertices[j].Normal.Y, obj.LoadedVertices[j].Normal.Z };
			nor = DirectX::XMVector3Transform(nor, objInfo[i].rotation);
			pos = DirectX::XMVectorScale(pos, objInfo[i].scale);
			if (objInfo[i].translateFirst)
			{
				pos = DirectX::XMVector3Transform(pos, objInfo[i].translation);
				pos = DirectX::XMVector3Transform(pos, objInfo[i].rotation);
			}
			else
			{
				pos = DirectX::XMVector3Transform(pos, objInfo[i].rotation);
				pos = DirectX::XMVector3Transform(pos, objInfo[i].translation);
			}
			DirectX::XMFLOAT3 posFloat;
			DirectX::XMFLOAT3 norFloat;
			DirectX::XMStoreFloat3(&posFloat, pos);
			DirectX::XMStoreFloat3(&norFloat, nor);
			objl::Vertex newVertex = {
				{ posFloat.x, posFloat.y, posFloat.z },
				{ norFloat.x, norFloat.y, norFloat.z },
				obj.LoadedVertices[j].TextureCoordinate
			};
			vertices.push_back(newVertex);

			// Minimum positions
			if (posFloat.x < meshBound.min.x)
			{
				meshBound.min.x = posFloat.x;
			}
			if (posFloat.y < meshBound.min.y)
			{
				meshBound.min.y = posFloat.y;
			}
			if (posFloat.z < meshBound.min.z)
			{
				meshBound.min.z = posFloat.z;
			}

			// Maximum positions
			if (posFloat.x > meshBound.max.x)
			{
				meshBound.max.x = posFloat.x;
			}
			if (posFloat.y > meshBound.max.y)
			{
				meshBound.max.y = posFloat.y;
			}
			if (posFloat.z > meshBound.max.z)
			{
				meshBound.max.z = posFloat.z;
			}
		}
		meshBoundPositions.push_back(meshBound);

		meshData.vertexInfo.vertexData = vertices.data();

		meshData.indexInfo.nrOfIndicesInBuffer = obj.LoadedIndices.size();
		meshData.indexInfo.indexData = obj.LoadedIndices.data();

		// Store submeshes and textures
		int startIndex = 0;
		for (int j = 0; j < obj.LoadedMeshes.size(); j++) // For every texture in a mesh
		{
			MeshData::SubMeshInfo subMeshInfo;
			subMeshInfo.startIndexValue = startIndex;
			subMeshInfo.nrOfIndicesInSubMesh = obj.LoadedMeshes[j].Indices.size();

			// If the mesh doesn't have a material, default colour
			if (obj.LoadedMaterials.size() == 0)
			{
				char defaultColour[] = { 128, 128, 128, 255 };
				ShaderResourceTextureD3D11 defaultSRV(device, 1, 1, defaultColour);
				subMeshInfo.ambientTextureSRV = defaultSRV.GetSRV();
				subMeshInfo.diffuseTextureSRV = subMeshInfo.ambientTextureSRV;
				subMeshInfo.specularTextureSRV = subMeshInfo.ambientTextureSRV;

				subMeshInfo.ambientTextureSRV->AddRef();
			}
			else
			{
				// Store texture maps
				ShaderResourceTextureD3D11 textureAmbient(device, obj.LoadedMeshes[j].MeshMaterial.map_Ka.c_str());
				ShaderResourceTextureD3D11 textureDiffuse(device, obj.LoadedMeshes[j].MeshMaterial.map_Kd.c_str());
				ShaderResourceTextureD3D11 textureSpecular(device, obj.LoadedMeshes[j].MeshMaterial.map_Ks.c_str());
				subMeshInfo.ambientTextureSRV = textureAmbient.GetSRV();
				subMeshInfo.diffuseTextureSRV = textureDiffuse.GetSRV();
				subMeshInfo.specularTextureSRV = textureSpecular.GetSRV();

				// If the mesh doesn't have a texture map but have texture colour
				if (subMeshInfo.ambientTextureSRV == nullptr)
				{
					objl::Vector3 ambientColour = obj.LoadedMeshes[j].MeshMaterial.Ka;
					char colour[] = { ambientColour.X * 255, ambientColour.Y * 255, ambientColour.Z * 255, 255 };
					ShaderResourceTextureD3D11 defaultDiffuse(device, 1, 1, colour);
					subMeshInfo.ambientTextureSRV = defaultDiffuse.GetSRV();
					subMeshInfo.ambientTextureSRV->AddRef();
				}
				else
				{
					subMeshInfo.ambientTextureSRV->AddRef();
				}
				if (subMeshInfo.diffuseTextureSRV == nullptr)
				{
					objl::Vector3 diffuseColour = obj.LoadedMeshes[j].MeshMaterial.Kd;
					char colour[] = { diffuseColour.X * 255, diffuseColour.Y * 255, diffuseColour.Z * 255, 255 };
					ShaderResourceTextureD3D11 defaultDiffuse(device, 1, 1, colour);
					subMeshInfo.diffuseTextureSRV = defaultDiffuse.GetSRV();
					subMeshInfo.diffuseTextureSRV->AddRef();
				}
				else
				{
					subMeshInfo.diffuseTextureSRV->AddRef();
				}
				if (subMeshInfo.specularTextureSRV == nullptr)
				{
					objl::Vector3 specularColour = obj.LoadedMeshes[j].MeshMaterial.Ks;
					char colour[] = { specularColour.X * 255, specularColour.Y * 255, specularColour.Z * 255, 255 };
					ShaderResourceTextureD3D11 defaultDiffuse(device, 1, 1, colour);
					subMeshInfo.specularTextureSRV = defaultDiffuse.GetSRV();
					subMeshInfo.specularTextureSRV->AddRef();
				}
				else
				{
					subMeshInfo.specularTextureSRV->AddRef();
				}
			}

			meshData.subMeshInfo.push_back(subMeshInfo);

			startIndex += subMeshInfo.nrOfIndicesInSubMesh;
		}
		mesh[i].Initialize(device, meshData);
		mesh[i].Name(objInfo[i].address);
	}
}

void DeferredRenderingSetup(ID3D11DeviceContext* context, ID3D11Device*& device, UINT width, UINT height, const UINT NR_OF_GBUFFERS,
	ID3D11Texture2D** DRtexture, ID3D11ShaderResourceView** DRsrv, ID3D11RenderTargetView** DRrtv,
	ID3D11UnorderedAccessView*& DRuav, IDXGISwapChain*& swapChain)
{
	// get the address of the back buffer
	ID3D11Texture2D* backBuffer = nullptr;
	if (FAILED(swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&backBuffer))))
	{
		std::cerr << "Failed to get back buffer!" << std::endl;
		return;
	}

	// use the back buffer address to create the UAV
	// null as description to base it on the backbuffers values
	HRESULT hr = device->CreateUnorderedAccessView(backBuffer, nullptr, &DRuav);
	backBuffer->Release();

	D3D11_TEXTURE2D_DESC texture2dDesc;
	texture2dDesc.Width = width;
	texture2dDesc.Height = height;
	texture2dDesc.MipLevels = 1;
	texture2dDesc.ArraySize = 1;
	texture2dDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	texture2dDesc.SampleDesc.Count = 1;
	texture2dDesc.SampleDesc.Quality = 0;
	texture2dDesc.Usage = D3D11_USAGE_DEFAULT; // Read and write access by the GPU
	texture2dDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
	texture2dDesc.CPUAccessFlags = 0;
	texture2dDesc.MiscFlags = 0;

	for (int i = 0; i < NR_OF_GBUFFERS; i++)
	{
		hr = device->CreateTexture2D(&texture2dDesc, nullptr, &DRtexture[i]);
		if (FAILED(hr))
		{
			std::cerr << "Could not create G-buffer texture\n";
			return;
		}

		hr = device->CreateShaderResourceView(DRtexture[i], nullptr, &DRsrv[i]);
		if (FAILED(hr))
		{
			std::cerr << "Could not create G-buffer srv\n";
			return;
		}

		hr = device->CreateRenderTargetView(DRtexture[i], nullptr, &DRrtv[i]);
		if (FAILED(hr))
		{
			std::cerr << "Could not create G-buffer rtv\n";
			return;
		}
	}
}

void SetupLights(ID3D11DeviceContext* context, ID3D11Device*& device, SpotLightCollectionD3D11* spotLights, 
	MainCamera* mainCamera, ConstantBufferD3D11* cBufferCS, UINT* totalSpotLights, DirectionalLight* directionLight)
{
	// Initialize direction light
	DirectX::XMFLOAT3 colour = { 1.0f, 1.0f, 1.0f };
	DirectX::XMFLOAT3 direction = { 1.0f, -2.0f, 2.0f };
	directionLight->Initialize(device, colour, direction);

	// Initialize spot lights
	SpotLightData spotLightData;
	spotLightData.shadowMapInfo.textureDimension = 2;

	SpotLightData::PerLightInfo perLightInfo;

	perLightInfo.colour = { 1.0f, 0.0f, 0.0f };
	perLightInfo.rotationX = PID4;
	perLightInfo.rotationY = PI + PID4 / 2;
	perLightInfo.outAngle = PID4 / 2;
	perLightInfo.inAngle = PID4 / 4;
	perLightInfo.projectionNearZ = 0.1f;
	perLightInfo.projectionFarZ = 100.0f;
	perLightInfo.initialPosition = { 0.0f, 10.0f, 10.0f };
	spotLightData.perLightInfo.push_back(perLightInfo);

	perLightInfo.colour = { 0.0f, 1.0f, 0.0f };
	perLightInfo.rotationX = PID4;
	perLightInfo.rotationY = PI;
	perLightInfo.outAngle = PID4 / 2;
	perLightInfo.inAngle = PID4 / 4;
	perLightInfo.projectionNearZ = 0.1f;
	perLightInfo.projectionFarZ = 100.0f;
	perLightInfo.initialPosition = { 0.0f, 10.0f, 10.0f };
	spotLightData.perLightInfo.push_back(perLightInfo);

	perLightInfo.colour = { 0.0f, 0.0f, 1.0f };
	perLightInfo.rotationX = PID4;
	perLightInfo.rotationY = PI - PID4 / 2;
	perLightInfo.outAngle = PID4 / 2;
	perLightInfo.inAngle = PID4 / 4;
	perLightInfo.projectionNearZ = 0.1f;
	perLightInfo.projectionFarZ = 100.0f;
	perLightInfo.initialPosition = { 0.0f, 10.0f, 10.0f };
	spotLightData.perLightInfo.push_back(perLightInfo);
	
	spotLights->Initialize(device, spotLightData);
	
	// Set spot lights structured buffer
	ID3D11ShaderResourceView* spotLightSRV = spotLights->GetLightBufferSRV();
	context->CSSetShaderResources(0, 1, &spotLightSRV);

	// Set directional light structured buffer
	ID3D11ShaderResourceView* dirLightSRV = directionLight->GetLightBufferSRV();
	context->CSSetShaderResources(7, 1, &dirLightSRV);

	// Constant buffer with necessary data
	DirectX::XMFLOAT3 camPosFloat3 = mainCamera->GetPosition();
	CbufferCS cbuffer;
	cbuffer.camPos = DirectX::XMFLOAT4(camPosFloat3.x, camPosFloat3.y, camPosFloat3.z, 1.0f);
	cbuffer.totalSpotLights = spotLights->GetNrOfLights();
	cbuffer.fullLight = 0.0f;
	cbuffer.shadows = 1.0f;
	*totalSpotLights = cbuffer.totalSpotLights;
	cBufferCS->Initialize(device, sizeof(cbuffer), &cbuffer);
	ID3D11Buffer* camPosBuffer = cBufferCS->GetBuffer();
	context->CSSetConstantBuffers(1, 1, &camPosBuffer);
	camPosBuffer->Release();
}

void UpdatePerFrame(ID3D11DeviceContext* context, ID3D11Device*& device, UINT totalSpotLights, MainCamera* mainCamera,
	ConstantBufferD3D11* cBufferCS, ID3D11Buffer*& camPosBuffer, ConstantBufferD3D11* camPosConstBuffer,
	ConstantBufferD3D11* particleConstantBuffer, float particleSize, ConstantBufferD3D11* tessellationPositions,
	DirectX::XMFLOAT4 moveObj, MeshD3D11* frustumMesh, ConstantBufferD3D11* frustumCbuffer, DirectX::BoundingFrustum* cameraFrustum,
	ConstantBufferD3D11 *rayConstBuffer, ConstantBufferD3D11 *rayConstData, float time, float deltaTime)
{
	static float lights = 1.0f;
	if (GetKeyState('L')) // Turn on/off lights
	{
		lights = 0.0f;
	}
	else
	{
		lights = 1.0f;
	}

	static float fullLight = 0.0f;
	if (GetKeyState('K')) // Turn on/off full ambient
	{
		fullLight = 1.0f;
	}
	else
	{
		fullLight = 0.0f;
	}

	static float shadows = 1.0f;
	if (GetKeyState('J')) // Turn on/off shadows
	{
		shadows = 0.0f;
	}
	else
	{
		shadows = 1.0f;
	}

	// Update the compute shader constant buffer
	const DirectX::XMFLOAT3 camPos = mainCamera->GetPosition();
	CbufferCS cbuffer;
	cbuffer.camPos = DirectX::XMFLOAT4(camPos.x, camPos.y, camPos.z, lights);
	cbuffer.totalSpotLights = totalSpotLights;
	cbuffer.fullLight = fullLight;
	cbuffer.shadows = shadows;
	cBufferCS->UpdateBuffer(context, &cbuffer);

	camPosConstBuffer->UpdateBuffer(context, &cbuffer.camPos);
	camPosBuffer = camPosConstBuffer->GetBuffer();

	// Update particle Constant Buffer
	ParticleConstantBufferData particleConstantBufferData;
	particleConstantBufferData.worldMatrix = mainCamera->GetWorldMatrix();
	particleConstantBufferData.vpMatrix = mainCamera->GetViewProjectionMatrix();
	particleConstantBufferData.position = camPos;
	particleConstantBufferData.position.x = particleConstantBufferData.position.x;
	particleConstantBufferData.position.y = particleConstantBufferData.position.y;
	particleConstantBufferData.position.z = particleConstantBufferData.position.z;
	particleConstantBufferData.particleSize = particleSize;
	particleConstantBuffer->UpdateBuffer(context, &particleConstantBufferData);

	Positions positions;
	positions.camPosition = { camPos.x, camPos.y, camPos.z, 1.0f };
	positions.objPosition = moveObj;
	tessellationPositions->UpdateBuffer(context, &positions);

	// Update camera frustum
	*cameraFrustum = mainCamera->GetFrustum();
	DirectX::XMFLOAT3 frustumCorners[8];
	DirectX::XMVECTOR quaternion = DirectX::XMQuaternionNormalize(DirectX::XMLoadFloat4(&cameraFrustum->Orientation));
	DirectX::XMStoreFloat4(&cameraFrustum->Orientation, quaternion);
	cameraFrustum->GetCorners(frustumCorners);

	DirectX::XMFLOAT3 colour = { 1.0f, 0.0f, 0.0f };
	BoundingBoxVertex frustumVertices[8];
	for (int i = 0; i < 8; i++)
	{
		frustumVertices[i].position = frustumCorners[i];
		frustumVertices[i].colour = colour;
	}

	uint32_t frustumIndices[] =
	{
		0,1, 1,2, 2,3, 3,0,
		0,4, 1,5, 2,6, 3,7,
		4,5, 5,6, 6,7, 7,4
	};

	MeshData frustumMeshData;
	frustumMeshData.vertexInfo.sizeOfVertex = sizeof(BoundingBoxVertex);
	frustumMeshData.vertexInfo.nrOfVerticesInBuffer = 8;
	frustumMeshData.vertexInfo.vertexData = frustumVertices;
	frustumMeshData.indexInfo.nrOfIndicesInBuffer = 24;
	frustumMeshData.indexInfo.indexData = frustumIndices;
	MeshData::SubMeshInfo frustumSubMeshData;
	frustumSubMeshData.startIndexValue = 0;
	frustumSubMeshData.nrOfIndicesInSubMesh = 24;
	frustumSubMeshData.ambientTextureSRV = nullptr;
	frustumSubMeshData.diffuseTextureSRV = nullptr;
	frustumSubMeshData.specularTextureSRV = nullptr;
	frustumMeshData.subMeshInfo.push_back(frustumSubMeshData);

	frustumMesh->Initialize(device, frustumMeshData);

	DirectX::XMFLOAT4X4 worldMatrix = mainCamera->GetWorldMatrix();
	DirectX::XMFLOAT4X4 viewProjMatrix = mainCamera->GetViewProjectionMatrix();
	DirectX::XMFLOAT4X4 matrices[2] = { worldMatrix, viewProjMatrix };
	frustumCbuffer->UpdateBuffer(context, &matrices);

	CameraData camData;
	camData.camPos = DirectX::XMFLOAT4(camPos.x, camPos.y, camPos.z, 1.0f);
	camData.viewProj = mainCamera->GetViewProjectionMatrix();

	DirectX::XMMATRIX invMat = DirectX::XMMatrixInverse(nullptr, DirectX::XMLoadFloat4x4(&camData.viewProj));
	DirectX::XMStoreFloat4x4(&camData.viewProj, invMat);

	rayConstBuffer->UpdateBuffer(context, &camData);

	RayData rayData;
	rayData.time = time;
	rayData.deltaTime = deltaTime;
	rayData.totalSpotLights = totalSpotLights;
	rayConstData->UpdateBuffer(context, &rayData);
}

void SetupReflection(ID3D11Device*& device, const UINT NR_OF_GBUFFERS, ID3D11Texture2D** cubeTextureG, 
	ID3D11ShaderResourceView** cubeSRVg, ID3D11RenderTargetView** cubeRTVg, ID3D11UnorderedAccessView** cubeUAVs,
	ID3D11Texture2D*& cubeTextures, ID3D11ShaderResourceView*& cubeTextureSRV, UINT size)
{
	// G-buffer textures
	D3D11_TEXTURE2D_DESC texture2dDesc;
	texture2dDesc.Width = size;
	texture2dDesc.Height = size;
	texture2dDesc.MipLevels = 1;
	texture2dDesc.ArraySize = 1;
	texture2dDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	texture2dDesc.SampleDesc.Count = 1;
	texture2dDesc.SampleDesc.Quality = 0;
	texture2dDesc.Usage = D3D11_USAGE_DEFAULT; // Read and write access by the GPU
	texture2dDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
	texture2dDesc.CPUAccessFlags = 0;
	texture2dDesc.MiscFlags = 0;

	HRESULT hr;
	for (int i = 0; i < NR_OF_GBUFFERS; i++)
	{
		hr = device->CreateTexture2D(&texture2dDesc, nullptr, &cubeTextureG[i]);
		if (FAILED(hr))
		{
			std::cerr << "Could not create reflection G-buffer texture\n";
			return;
		}

		hr = device->CreateShaderResourceView(cubeTextureG[i], nullptr, &cubeSRVg[i]);
		if (FAILED(hr))
		{
			std::cerr << "Could not create reflection G-buffer srv\n";
			return;
		}

		hr = device->CreateRenderTargetView(cubeTextureG[i], nullptr, &cubeRTVg[i]);
		if (FAILED(hr))
		{
			std::cerr << "Could not create reflection G-buffer rtv\n";
			return;
		}
	}

	// UAV texture
	D3D11_TEXTURE2D_DESC desc;
	desc.Width = size;
	desc.Height = size;
	desc.MipLevels = 1;
	desc.ArraySize = 6;
	desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	desc.SampleDesc.Count = 1;
	desc.SampleDesc.Quality = 0;
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.BindFlags = D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE;
	desc.CPUAccessFlags = 0;
	desc.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE;

	hr = device->CreateTexture2D(&desc, nullptr, &cubeTextures);
	if (FAILED(hr))
	{
		std::cerr << "Could not create texture cube\n";
		return;
	}

	hr = device->CreateShaderResourceView(cubeTextures, nullptr, &cubeTextureSRV);
	if (FAILED(hr))
	{
		std::cerr << "Could not create texture cube SRV\n";
		return;
	}

	D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc;
	uavDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	uavDesc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2DARRAY;
	uavDesc.Texture2DArray.ArraySize = 1;
	uavDesc.Texture2DArray.MipSlice = 0;

	for (int i = 0; i < 6; i++)
	{
		uavDesc.Texture2DArray.FirstArraySlice = i;
		hr = device->CreateUnorderedAccessView(cubeTextures, &uavDesc, &cubeUAVs[i]);
		if (FAILED(hr))
		{
			std::cerr << "Could not create cube UAV\n";
			return;
		}
	}
}

void SetupParticles(ID3D11Device*& device, StructuredBufferD3D11* particleBuffer, MainCamera* mainCamera,
	ConstantBufferD3D11* particleConstantBuffer, int nrOfParticles, float particleSize)
{
	// Particle positions
	struct Particle
	{
		DirectX::XMFLOAT4 pos;

		Particle(float x, float y, float z, float w)
		{
			this->pos.x = x;
			this->pos.y = y;
			this->pos.z = z;
			this->pos.w = w;
		}
	};

	std::vector<Particle> particles;

	std::random_device RandomDevice;
	std::uniform_real_distribution<> RandomXZ(-20.0f, 20.0f);
	std::uniform_real_distribution<> RandomY(-10.0f, 20.0f);
	std::uniform_real_distribution<> RandomPi(-DirectX::XM_PI, DirectX::XM_PI);
	for (int i = 0; i < nrOfParticles; i++)
	{
		float randomX = (float)RandomXZ(RandomDevice);
		float randomY = (float)RandomY(RandomDevice);
		float randomZ = (float)RandomXZ(RandomDevice);
		float randomW = (float)RandomPi(RandomDevice);

		Particle particle(randomX, randomY, randomZ, randomW);
		particles.push_back(particle);
	}

	particleBuffer->Initialize(device, sizeof(Particle), particles.size(), particles.data(), true, true, true);

	// Particle Constant Buffer
	ParticleConstantBufferData particleConstantBufferData;
	particleConstantBufferData.worldMatrix = mainCamera->GetWorldMatrix();
	particleConstantBufferData.vpMatrix = mainCamera->GetViewProjectionMatrix();
	particleConstantBufferData.position = mainCamera->GetPosition();
	particleConstantBufferData.particleSize = particleSize;

	particleConstantBuffer->Initialize(device, sizeof(ParticleConstantBufferData), &particleConstantBufferData);
}

void SetupTessellation(ID3D11Device*& device, ConstantBufferD3D11* tessellationPositions, MainCamera* mainCamera,
	DirectX::XMFLOAT4 moveObj)
{
	struct Positions positions;

	DirectX::XMFLOAT3 camPosition = mainCamera->GetPosition();
	positions.camPosition = { camPosition.x, camPosition.y, camPosition.z, 1.0f };
	positions.objPosition = moveObj;

	tessellationPositions->Initialize(device, sizeof(Positions), &positions);
}

void SetupCulling(ID3D11Device*& device, MainCamera* mainCamera, QuadTree<MeshD3D11>* quadTree, MeshD3D11* meshes, size_t totalMeshes,
	std::vector<MeshBoundPositions>& meshBoundPositions, MeshD3D11* frustumMesh, DirectX::BoundingFrustum* cameraFrustum,
	ConstantBufferD3D11* frustumCbuffer, MeshD3D11* meshBoundingBoxLines)
{
	// Create the main volume bounding box
	DirectX::XMFLOAT3 center = { 0.0f, 0.0f, 0.0f };
	DirectX::XMFLOAT3 extents = { 25.0f, 25.0f, 25.0f };
	quadTree->Initialize(center, extents);

	// Create the bounding boxes for each object
	std::vector<DirectX::BoundingBox> meshBoundingBoxes;
	for (int i = 0; i < meshBoundPositions.size(); i++)
	{
		DirectX::BoundingBox meshBoundingBox;
		DirectX::XMVECTOR min = DirectX::XMLoadFloat3(&meshBoundPositions[i].min);
		DirectX::XMVECTOR max = DirectX::XMLoadFloat3(&meshBoundPositions[i].max);
		DirectX::BoundingBox::CreateFromPoints(meshBoundingBox, min, max);
		meshBoundingBoxes.push_back(meshBoundingBox);
	}

	// Add the bounding boxes in a quad tree
	for (int i = 0; i < totalMeshes; i++)
	{
		quadTree->AddElement(&meshes[i], meshBoundingBoxes[i]);
	}

	//std::cerr << quadTree->ToGraphviz() << std::endl << std::endl;
	//std::cerr << quadTree->ToGraphvizTree() << std::endl << std::endl;

	// Create bounding frustum
	DirectX::XMFLOAT3 frustumCorners[8];
	cameraFrustum->GetCorners(frustumCorners);

	DirectX::XMFLOAT3 colour = { 1.0f, 0.0f, 0.0f };
	BoundingBoxVertex boundingBoxVertices[8];
	for (int i = 0; i < 8; i++)
	{
		boundingBoxVertices[i].position = frustumCorners[i];
		boundingBoxVertices[i].colour = colour;
	}

	uint32_t indices[] =
	{
		0,1, 1,2, 2,3, 3,0,
		0,4, 1,5, 2,6, 3,7,
		4,5, 5,6, 6,7, 7,4
	};

	MeshData boundingBoxMeshData;
	boundingBoxMeshData.vertexInfo.sizeOfVertex = sizeof(BoundingBoxVertex);
	boundingBoxMeshData.vertexInfo.nrOfVerticesInBuffer = 8;
	boundingBoxMeshData.vertexInfo.vertexData = boundingBoxVertices;
	boundingBoxMeshData.indexInfo.nrOfIndicesInBuffer = 24;
	boundingBoxMeshData.indexInfo.indexData = indices;
	MeshData::SubMeshInfo boundingBoxSubMeshData;
	boundingBoxSubMeshData.startIndexValue = 0;
	boundingBoxSubMeshData.nrOfIndicesInSubMesh = 24;
	boundingBoxSubMeshData.ambientTextureSRV = nullptr;
	boundingBoxSubMeshData.diffuseTextureSRV = nullptr;
	boundingBoxSubMeshData.specularTextureSRV = nullptr;
	boundingBoxMeshData.subMeshInfo.push_back(boundingBoxSubMeshData);

	frustumMesh->Initialize(device, boundingBoxMeshData);

	DirectX::XMFLOAT4X4 worldMatrix = mainCamera->GetWorldMatrix();
	DirectX::XMFLOAT4X4 viewProjMatrix = mainCamera->GetViewProjectionMatrix();
	DirectX::XMFLOAT4X4 matrices[2] = { worldMatrix, viewProjMatrix };
	frustumCbuffer->Initialize(device, sizeof(matrices), &matrices);

	// Create mesh bounding boxes
	colour = { 0.0f, 1.0f, 0.0f };
	for (int i = 0; i < totalMeshes + 2; i++)
	{
		DirectX::XMFLOAT3 corners[8];
		meshBoundingBoxes[i].GetCorners(corners);

		for (int j = 0; j < 8; j++)
		{
			boundingBoxVertices[j].position = corners[j];
			boundingBoxVertices[j].colour = colour;
		}

		meshBoundingBoxLines[i].Initialize(device, boundingBoxMeshData);
	}
}

void SetupRayMarchingVolFog(ID3D11Device *&device, ConstantBufferD3D11 *rayConstBuffer, MainCamera *mainCamera, 
	ConstantBufferD3D11 *rayConstData, UINT totalSpotLights)
{
	CameraData camData;
	DirectX::XMFLOAT3 camPos = mainCamera->GetPosition();
	camData.camPos = DirectX::XMFLOAT4(camPos.x, camPos.y, camPos.z, 1.0f);
	camData.viewProj = mainCamera->GetViewProjectionMatrix();

	DirectX::XMMATRIX invMat = DirectX::XMMatrixInverse(nullptr, DirectX::XMLoadFloat4x4(&camData.viewProj));
	DirectX::XMStoreFloat4x4(&camData.viewProj, invMat);

	rayConstBuffer->Initialize(device, sizeof(CameraData), &camData);

	RayData rayData;
	rayData.time = 0.0f;
	rayData.deltaTime = 0.0f;
	rayData.totalSpotLights = totalSpotLights;
	rayConstData->Initialize(device, sizeof(RayData), &rayData);
}
