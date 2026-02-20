#include "ShaderResourceTextureD3D11.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

ShaderResourceTextureD3D11::ShaderResourceTextureD3D11(ID3D11Device* device, UINT width, UINT height, void* textureData)
{
	Initialize(device, width, height, textureData);
}

ShaderResourceTextureD3D11::ShaderResourceTextureD3D11(ID3D11Device* device, const char* pathToTextureFile)
{
	Initialize(device, pathToTextureFile);
}

ShaderResourceTextureD3D11::~ShaderResourceTextureD3D11()
{
	if (this->texture != nullptr)
		this->texture->Release();
	if (this->srv != nullptr)
		this->srv->Release();
}

void ShaderResourceTextureD3D11::Initialize(ID3D11Device* device, UINT width, UINT height, void* textureData)
{
	D3D11_TEXTURE2D_DESC textureDesc;
	textureDesc.Width = width;
	textureDesc.Height = height;
	textureDesc.MipLevels = 1;
	textureDesc.ArraySize = 1;
	textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.SampleDesc.Quality = 0;
	textureDesc.Usage = D3D11_USAGE_IMMUTABLE; // Can only be read by GPU and not written
	textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	textureDesc.CPUAccessFlags = 0;
	textureDesc.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA data;
	data.pSysMem = textureData;
	data.SysMemPitch = width * 4; // Distance (in bytes) from beginning of one line of a texture to the next line.
	data.SysMemSlicePitch = 0;

	HRESULT hr = device->CreateTexture2D(&textureDesc, &data, &this->texture);
	if (SUCCEEDED(hr))
		hr = device->CreateShaderResourceView(this->texture, nullptr, &this->srv);
}

void ShaderResourceTextureD3D11::Initialize(ID3D11Device* device, const char* pathToTextureFile)
{
	int width, height, channels;
	unsigned char* imageData = stbi_load(pathToTextureFile, &width, &height, &channels, 4); // Put the image in /RasterizerDemo/x64/Debug
	if (imageData == NULL)
	{
		stbi_image_free(imageData);
		return;
	}

	D3D11_TEXTURE2D_DESC textureDesc;
	textureDesc.Width = width;
	textureDesc.Height = height;
	textureDesc.MipLevels = 1;
	textureDesc.ArraySize = 1;
	textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.SampleDesc.Quality = 0;
	textureDesc.Usage = D3D11_USAGE_IMMUTABLE; // Can only be read by GPU and not written
	textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	textureDesc.CPUAccessFlags = 0;
	textureDesc.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA data;
	data.pSysMem = imageData;
	data.SysMemPitch = width * 4; // Distance (in bytes) from beginning of one line of a texture to the next line.
	data.SysMemSlicePitch = 0;

	HRESULT hr = device->CreateTexture2D(&textureDesc, &data, &this->texture);
	stbi_image_free(imageData);
	if (SUCCEEDED(hr))
		hr = device->CreateShaderResourceView(this->texture, nullptr, &this->srv);
}

ID3D11ShaderResourceView* ShaderResourceTextureD3D11::GetSRV() const
{
	return this->srv;
}
