#include "IndexBufferD3D11.h"

IndexBufferD3D11::IndexBufferD3D11(ID3D11Device* device, size_t nrOfIndicesInBuffer, uint32_t* indexData)
{
	Initialize(device, nrOfIndicesInBuffer, indexData);
}

IndexBufferD3D11::~IndexBufferD3D11()
{
	if (this->buffer != nullptr)
		this->buffer->Release();
}

void IndexBufferD3D11::Initialize(ID3D11Device* device, size_t nrOfIndicesInBuffer, uint32_t* indexData)
{
	if (this->buffer != nullptr)
	{
		this->buffer->Release();
	}

	this->nrOfIndices = nrOfIndicesInBuffer;

	D3D11_BUFFER_DESC indexDesc;
	indexDesc.ByteWidth = this->nrOfIndices * sizeof(uint32_t);
	indexDesc.Usage = D3D11_USAGE_IMMUTABLE; // Can only be read by GPU and not written
	indexDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	indexDesc.CPUAccessFlags = 0;
	indexDesc.MiscFlags = 0;
	indexDesc.StructureByteStride = sizeof(uint32_t);

	D3D11_SUBRESOURCE_DATA data;
	data.pSysMem = indexData;
	data.SysMemPitch = 0;
	data.SysMemSlicePitch = 0;

	HRESULT hr = device->CreateBuffer(&indexDesc, &data, &this->buffer);
	if (FAILED(hr))
	{
		std::cerr << "Failed to create index buffer";
	}
}

void IndexBufferD3D11::Initialize(ID3D11Device* device, size_t nrOfIndicesInBuffer, std::vector<uint32_t>* indexData)
{
	this->nrOfIndices = nrOfIndicesInBuffer;

	D3D11_BUFFER_DESC indexDesc;
	indexDesc.ByteWidth = nrOfIndices * sizeof(uint32_t);
	indexDesc.Usage = D3D11_USAGE_IMMUTABLE; // Can only be read by GPU and not written
	indexDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	indexDesc.CPUAccessFlags = 0;
	indexDesc.MiscFlags = 0;
	indexDesc.StructureByteStride = sizeof(uint32_t);
	size_t as;

	D3D11_SUBRESOURCE_DATA data;
	data.pSysMem = indexData;
	data.SysMemPitch = 0;
	data.SysMemSlicePitch = 0;

	HRESULT hr = device->CreateBuffer(&indexDesc, &data, &this->buffer);
}

size_t IndexBufferD3D11::GetNrOfIndices() const
{
	return this->nrOfIndices;
}

ID3D11Buffer* IndexBufferD3D11::GetBuffer() const
{
	return this->buffer;
}
