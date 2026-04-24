#pragma once
#include "System/SystemUtils/D3DBuffer/D3DBuffer/D3DBuffer.h"

namespace System {


	class RenderTargetView;
	class DepthStencilView;
	class ShaderResourceView;

	class Texture final :public D3DBuffer
	{
	private:
		std::unique_ptr<RenderTargetView> rtv;
		std::unique_ptr<DepthStencilView> dsv;
		std::unique_ptr<ShaderResourceView> srv;
		unsigned int width_32 = 0;
	public:
		Texture(ComPtr<ID3D12Resource>& resource, std::unique_ptr<ShaderResourceView> srv_, std::unique_ptr<RenderTargetView> rtv_, std::unique_ptr<DepthStencilView> dsv_);
		RenderTargetView* Rtv() const { return rtv.get(); }
		DepthStencilView* Dsv() const { return dsv.get(); }
		ShaderResourceView* Srv() const { return srv.get(); }
		const unsigned int Width() const { return width_32; }
		const unsigned int Height() const { return resource_desc.Height; }
		const unsigned short ArraySize() const { return resource_desc.DepthOrArraySize; }
		const size_t& Width64()const { return resource_desc.Width; }// 64ビット版の幅。幅が4GBを超える可能性がある場合はこちらを使用する必要がある

		class Loader final
		{
		private:
			static HRESULT CreateEmptyTexture(const D3D12_RESOURCE_DESC& desc, ComPtr<ID3D12Resource>& texture_resource, D3D12_CLEAR_VALUE* p_clear_value = nullptr);
			static HRESULT CreateUploadBuffer(size_t size, ComPtr<ID3D12Resource>& upload_buffer);
			static HRESULT UploadTextureData(ID3D12Resource* upload_buffer, void* data, unsigned int row_pitch, unsigned int height, unsigned short depth = 1U);
			static HRESULT CopyUploadBufferToTexture(ID3D12Resource* upload_buffer, ID3D12Resource* texture_resource);
			static HRESULT CreateViewsForTexture(ID3D12Resource* texture_resource, DXGI_FORMAT format, D3D12_RESOURCE_FLAGS flags, std::unique_ptr<ShaderResourceView>& out_srv, std::unique_ptr<RenderTargetView>& out_rtv, std::unique_ptr<DepthStencilView>& out_dsv);

		public:
			static std::unique_ptr<Texture> LoadFromFile(const std::wstring& path, D3D12_RESOURCE_FLAGS flags = D3D12_RESOURCE_FLAG_NONE);
			static std::unique_ptr<Texture> CreateEmpty(const D3D12_RESOURCE_DESC& desc, D3D12_CLEAR_VALUE* p_clear_value = nullptr);
			static int SaveToFile(Texture* texture, const std::wstring& path,D3D12_RESOURCE_STATES cur_state= D3D12_RESOURCE_STATE_COMMON);
		};

	};
}

//------------------------------------------------------------------------------
//Texture::Loader::CreateEmptyに渡す用のdescを簡単に作るためのマクロを定義しておく
#define TEX1D_DESC(width, format, flags) \
		 { \
			.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE1D, \
			.Alignment = 0, \
			.Width = width, \
			.Height = 1, \
			.DepthOrArraySize = 1, \
			.MipLevels = 1, \
			.Format = format, \
			.SampleDesc = {1, 0}, \
			.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN, \
			.Flags = flags \
		}
#define TEX2D_DESC(width, height, format, flags) \
		 { \
			.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D, \
			.Alignment = 0, \
			.Width = width, \
			.Height = height, \
			.DepthOrArraySize = 1, \
			.MipLevels = 1, \
			.Format = format, \
			.SampleDesc = {1, 0}, \
			.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN, \
			.Flags = flags \
		}
#define TEX2DARRAY_DESC(width, height, array_size, format, flags) \
		 { \
			.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D, \
			.Alignment = 0, \
			.Width = width, \
			.Height = height, \
			.DepthOrArraySize = array_size, \
			.MipLevels = 1, \
			.Format = format, \
			.SampleDesc = {1, 0}, \
			.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN, \
			.Flags = flags \
		}


#define TEX3D_DESC(width, height, depth, format, flags) \
		 { \
			.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE3D, \
			.Alignment = 0, \
			.Width = width, \
			.Height = height, \
			.DepthOrArraySize = depth, \
			.MipLevels = 1, \
			.Format = format, \
			.SampleDesc = {1, 0}, \
			.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN, \
			.Flags = flags \
		}
