#include "SubMeshD3D11.h"

void SubMeshD3D11::Initialize(size_t startIndexValue, size_t nrOfIndicesInSubMesh,
	ID3D11ShaderResourceView* ambientTextureSRV, ID3D11ShaderResourceView* diffuseTextureSRV,
	ID3D11ShaderResourceView* specularTextureSRV)
{
	this->startIndex = startIndexValue;
	this->nrOfIndices = nrOfIndicesInSubMesh;
	this->ambientTexture = ambientTextureSRV;
	this->diffuseTexture = diffuseTextureSRV;
	this->specularTexture = specularTextureSRV;
}

void SubMeshD3D11::PerformDrawCall(ID3D11DeviceContext* context) const
{
	context->DrawIndexed(this->nrOfIndices, this->startIndex, 0);
}

ID3D11ShaderResourceView* SubMeshD3D11::GetAmbientSRV() const
{
	return this->ambientTexture;
}

ID3D11ShaderResourceView* SubMeshD3D11::GetDiffuseSRV() const
{
	return this->diffuseTexture;
}

ID3D11ShaderResourceView* SubMeshD3D11::GetSpecularSRV() const
{
	return this->specularTexture;
}
