#include "Texture.h"
#include "System/Managers/DirectX12Manager/DirectX12Manager.h"
#include "System/SystemUtils/DeviceContext/ID3D12DeviceContext.h"
#include "System/SystemUtils/CommandQueue/CommandQueue.h"
#include "System/SystemUtils/Descriptors/View/View.h"

namespace System {
	HRESULT Texture::Initialize(ComPtr<ID3D12Resource>& resource, std::unique_ptr<ShaderResourceView> srv_, std::unique_ptr<RenderTargetView> rtv_, std::unique_ptr<DepthStencilView> dsv_)
	{
		if (!resource) {
			return E_FAIL;
		}
		if (!rtv_ && !dsv_ && !srv_) {
			return E_FAIL;
		}
		d3d_resource.Swap(resource);
		rtv = std::move(rtv_);
		dsv = std::move(dsv_);
		srv = std::move(srv_);
		resource_desc = d3d_resource->GetDesc();
		width_32 = static_cast<unsigned int>(resource_desc.Width);
		is_valid = true;
	}
	Texture::Texture(ComPtr<ID3D12Resource>& resource, std::unique_ptr<ShaderResourceView> srv_, std::unique_ptr<RenderTargetView> rtv_, std::unique_ptr<DepthStencilView> dsv_)
	{
		Initialize(resource, std::move(srv_), std::move(rtv_), std::move(dsv_));
	}


}