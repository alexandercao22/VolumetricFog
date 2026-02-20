#include "VertexBufferD3D11.h"

VertexBufferD3D11::VertexBufferD3D11(ID3D11Device* device, UINT sizeOfVertex, UINT nrOfVerticesInBuffer, void* vertexData)
{
	Initialize(device, sizeOfVertex, nrOfVerticesInBuffer, vertexData);
}

VertexBufferD3D11::~VertexBufferD3D11()
{
	if (this->buffer != nullptr)
		this->buffer->Release();
}

void VertexBufferD3D11::Initialize(ID3D11Device* device, UINT sizeOfVertex, UINT nrOfVerticesInBuffer, void* vertexData)
{
	if (this->buffer != nullptr)
	{
		this->buffer->Release();
	}

	this->vertexSize = sizeOfVertex;
	this->nrOfVertices = nrOfVerticesInBuffer;

	D3D11_BUFFER_DESC bufferDesc;
	bufferDesc.ByteWidth = this->nrOfVertices * this->vertexSize;
	bufferDesc.Usage = D3D11_USAGE_IMMUTABLE; // Can only be read by GPU and not written
	bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bufferDesc.CPUAccessFlags = 0;
	bufferDesc.MiscFlags = 0;
	bufferDesc.StructureByteStride = 0;

	D3D11_SUBRESOURCE_DATA data;
	data.pSysMem = vertexData;
	data.SysMemPitch = 0;
	data.SysMemSlicePitch = 0;

	HRESULT hr = device->CreateBuffer(&bufferDesc, &data, &this->buffer);
	if (FAILED(hr))
	{
		std::cerr << "Failed to create vertex buffer\n";
	}
}

UINT VertexBufferD3D11::GetNrOfVertices() const
{
	return this->nrOfVertices;
}

UINT VertexBufferD3D11::GetVertexSize() const
{
	return this->vertexSize;
}

ID3D11Buffer* VertexBufferD3D11::GetBuffer() const
{
	return this->buffer;
}
