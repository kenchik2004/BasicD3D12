#pragma once
#include "System/SystemUtils/D3DBuffer/D3DBuffer/D3DBuffer.h"

namespace System {

	class RenderTargetView;
	class DepthStencilView;
	class ShaderResourceView;

	class Texture final :public D3DBuffer
	{
		friend class TextureLoader;
	private:
		struct TextureIndexBuffer {
			unsigned int my_index;
		};
		std::unique_ptr<RenderTargetView> rtv;
		std::unique_ptr<DepthStencilView> dsv;
		std::unique_ptr<ShaderResourceView> srv;
	public:
		Texture(ComPtr<ID3D12Resource>& resource, std::unique_ptr<ShaderResourceView> srv_, std::unique_ptr<RenderTargetView> rtv_, std::unique_ptr<DepthStencilView> dsv_);
		RenderTargetView* Rtv() const { return rtv.get(); }
		DepthStencilView* Dsv() const { return dsv.get(); }
		ShaderResourceView* Srv() const { return srv.get(); }
		class TextureLoader final
		{
		private:
			static HRESULT CreateEmptyTexture(unsigned int width, unsigned int height, unsigned short array_size, unsigned short mip_levels, DXGI_FORMAT format, D3D12_RESOURCE_FLAGS flags, ComPtr<ID3D12Resource>& texture_resource);
			static HRESULT CreateUploadBuffer(size_t size, ComPtr<ID3D12Resource>& upload_buffer);
			static HRESULT UploadTextureData(const void* data, size_t size, ID3D12Resource* texture_resource);
			static HRESULT CopyUploadBufferToTexture(ID3D12Resource* upload_buffer, ID3D12Resource* texture_resource);
			static HRESULT CreateViewsForTexture(ID3D12Resource* texture_resource, DXGI_FORMAT format, D3D12_RESOURCE_FLAGS flags, std::unique_ptr<ShaderResourceView>& out_srv, std::unique_ptr<RenderTargetView>& out_rtv, std::unique_ptr<DepthStencilView>& out_dsv);
		public:
			static std::unique_ptr<Texture> LoadFromFile(const std::wstring& path, D3D12_RESOURCE_FLAGS flags = D3D12_RESOURCE_FLAG_NONE);
			static std::unique_ptr<Texture> CreateEmpty(unsigned int width, unsigned int height, DXGI_FORMAT format, D3D12_RESOURCE_FLAGS flags = D3D12_RESOURCE_FLAG_NONE);
		};

	};
};

