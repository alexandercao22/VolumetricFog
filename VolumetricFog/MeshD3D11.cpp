#include "MeshD3D11.h"

void MeshD3D11::Initialize(ID3D11Device* device, const MeshData& meshInfo)
{
	for (int i = 0; i < meshInfo.subMeshInfo.size(); i++)
	{
		SubMeshD3D11 subMesh;
		subMesh.Initialize(
			meshInfo.subMeshInfo[i].startIndexValue,
			meshInfo.subMeshInfo[i].nrOfIndicesInSubMesh,
			meshInfo.subMeshInfo[i].ambientTextureSRV,
			meshInfo.subMeshInfo[i].diffuseTextureSRV,
			meshInfo.subMeshInfo[i].specularTextureSRV);
		this->subMeshes.push_back(subMesh);
	}

	this->vertexBuffer.Initialize(device, meshInfo.vertexInfo.sizeOfVertex,
		meshInfo.vertexInfo.nrOfVerticesInBuffer, meshInfo.vertexInfo.vertexData);

	this->indexBuffer.Initialize(device, meshInfo.indexInfo.nrOfIndicesInBuffer, 
		meshInfo.indexInfo.indexData);
}

void MeshD3D11::BindMeshBuffers(ID3D11DeviceContext* context) const
{
	UINT stride = this->vertexBuffer.GetVertexSize();
	UINT offset = 0;
	ID3D11Buffer* buffer = this->vertexBuffer.GetBuffer();
	context->IASetVertexBuffers(0, 1, &buffer, &stride, &offset);
	context->IASetIndexBuffer(this->indexBuffer.GetBuffer(), DXGI_FORMAT_R32_UINT, 0);
}

void MeshD3D11::PerformSubMeshDrawCall(ID3D11DeviceContext* context, size_t subMeshIndex) const
{
	this->subMeshes[subMeshIndex].PerformDrawCall(context);
}

size_t MeshD3D11::GetNrOfSubMeshes() const
{
	return this->subMeshes.size();
}

ID3D11ShaderResourceView* MeshD3D11::GetAmbientSRV(size_t subMeshIndex) const
{
	return this->subMeshes[subMeshIndex].GetAmbientSRV();
}

ID3D11ShaderResourceView* MeshD3D11::GetDiffuseSRV(size_t subMeshIndex) const
{
	return this->subMeshes[subMeshIndex].GetDiffuseSRV();
}

ID3D11ShaderResourceView* MeshD3D11::GetSpecularSRV(size_t subMeshIndex) const
{
	return this->subMeshes[subMeshIndex].GetSpecularSRV();
}

void MeshD3D11::Name(std::string name)
{
	this->name = name;
}

std::string MeshD3D11::GetName() const
{
	return this->name;
}
