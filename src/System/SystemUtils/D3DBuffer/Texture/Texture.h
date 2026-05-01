#pragma once
#include "System/SystemUtils/D3DBuffer/D3DBuffer/D3DBuffer.h"

namespace System {


	class RenderTargetView;
	class DepthStencilView;
	class ShaderResourceView;

	class Texture final :public D3DBuffer
	{
	private:
		friend class TextureLoader;
		std::unique_ptr<RenderTargetView> rtv;
		std::unique_ptr<DepthStencilView> dsv;
		std::unique_ptr<ShaderResourceView> srv;
		unsigned int width_32 = 0;
		bool is_loaded = false;

		HRESULT Initialize(ComPtr<ID3D12Resource>& resource, std::unique_ptr<ShaderResourceView> srv_, std::unique_ptr<RenderTargetView> rtv_, std::unique_ptr<DepthStencilView> dsv_);
	public:
		Texture(ComPtr<ID3D12Resource>& resource, std::unique_ptr<ShaderResourceView> srv_, std::unique_ptr<RenderTargetView> rtv_, std::unique_ptr<DepthStencilView> dsv_);
		RenderTargetView* Rtv() const { return rtv.get(); }
		DepthStencilView* Dsv() const { return dsv.get(); }
		ShaderResourceView* Srv() const { return srv.get(); }
		const unsigned int Width() const { return width_32; }
		const unsigned int Height() const { return resource_desc.Height; }
		const unsigned short ArraySize() const { return resource_desc.DepthOrArraySize; }
		const size_t& Width64()const { return resource_desc.Width; }// 64ビット版の幅。幅が4GBを超える可能性がある場合はこちらを使用する必要がある
		bool IsLoaded() const { return is_loaded; }

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
