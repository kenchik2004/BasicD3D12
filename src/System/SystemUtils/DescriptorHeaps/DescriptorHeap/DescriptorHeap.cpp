#include "System/SystemUtils/DescriptorHeaps/DescriptorHeap/DescriptorHeap.h"
#include "System/Managers/DirectX12Manager/DirectX12Manager.h"
namespace System {

	DescriptorHeap::DescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE type_, unsigned int max_count)
		:type(type_), max_num_descriptors(max_count), current_descriptors_num(0)
	{
		// ディスクリプタヒープの説明構造体を作成
		D3D12_DESCRIPTOR_HEAP_DESC desc = {};
		desc.Type = type;
		desc.NumDescriptors = max_num_descriptors;
		desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE; // デフォルトのフラグを使用
		// DirectX12マネージャーからデバイスを取得して、ディスクリプタヒープを作成する
		ID3D12Device* device = DirectX12Manager::Instance()->GetDevice();
		if (device) {
			device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(descriptor_heap.GetAddressOf()));
			start = descriptor_heap->GetCPUDescriptorHandleForHeapStart(); // ヒープの開始位置を取得
			increment_size = device->GetDescriptorHandleIncrementSize(type); // ディスクリプタのサイズを取得
		}
	}
	std::unique_ptr<View> CSUHeap::CreateView(const VIEW_DESC& desc, ID3D12Resource* resource)
	{
		std::unique_ptr<View> view = nullptr;
		if (!resource)
			return view;
		auto device = DirectX12Manager::Instance()->GetDevice();
		D3D12_CPU_DESCRIPTOR_HANDLE handle = { start.ptr + increment_size * current_descriptors_num };
		switch (desc.type) {

		case VIEW_DESC::SRV: {

			device->CreateShaderResourceView(resource, desc.srv_desc, handle);
			D3D12_SHADER_RESOURCE_VIEW_DESC srv_desc = desc.srv_desc ? *desc.srv_desc : DEFAULT_VIEW_DESC_HELPER::GetDefaultSRVDesc(resource);
			view = std::make_unique<ShaderResourceView>(srv_desc, handle, resource);
			break;
		}
		case VIEW_DESC::CBV:

			device->CreateConstantBufferView(desc.cbv_desc, handle);

			break;
		case VIEW_DESC::UAV:
			device->CreateUnorderedAccessView(resource, nullptr, desc.uav_desc, handle);
			break;
		default:
			return view;
		}
		current_descriptors_num++;
		return view;

	}
	std::unique_ptr<View> RTVHeap::CreateView(const VIEW_DESC& desc, ID3D12Resource* resource)
	{

		std::unique_ptr<View> view = nullptr;
		if (!resource)
			return view;
		auto device = DirectX12Manager::Instance()->GetDevice();
		D3D12_CPU_DESCRIPTOR_HANDLE handle = { start.ptr + increment_size * current_descriptors_num };
		device->CreateRenderTargetView(resource, desc.rtv_desc, handle);
		D3D12_RENDER_TARGET_VIEW_DESC rtv_desc = desc.rtv_desc ? *desc.rtv_desc : DEFAULT_VIEW_DESC_HELPER::GetDefaultRTVDesc(resource);
		view = std::make_unique<RenderTargetView>(rtv_desc, handle, resource);
		current_descriptors_num++;
		return view;

	}
	std::unique_ptr<View> DSVHeap::CreateView(const VIEW_DESC& desc, ID3D12Resource* resource)
	{

		std::unique_ptr<View> view = nullptr;
		if (!resource)
			return view;
		auto device = DirectX12Manager::Instance()->GetDevice();
		D3D12_CPU_DESCRIPTOR_HANDLE handle = { start.ptr + increment_size * current_descriptors_num };
		device->CreateDepthStencilView(resource, desc.dsv_desc, handle);
		D3D12_DEPTH_STENCIL_VIEW_DESC dsv_desc = desc.dsv_desc ? *desc.dsv_desc : DEFAULT_VIEW_DESC_HELPER::GetDefaultDSVDesc(resource);
		view = std::make_unique<DepthStencilView>(dsv_desc, handle, resource);
		current_descriptors_num++;
		return view;
	}
}