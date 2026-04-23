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
		if (type_ == D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV)
			desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE; // CBV/SRV/UAVヒープはシェーダーから見えるようにする
		// DirectX12マネージャーからデバイスを取得して、ディスクリプタヒープを作成する
		ID3D12Device* device = DirectX12Manager::Instance()->GetDevice();
		if (device) {
			device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(descriptor_heap.GetAddressOf()));
			start_cpu = descriptor_heap->GetCPUDescriptorHandleForHeapStart(); // ヒープの開始位置を取得
			increment_size = device->GetDescriptorHandleIncrementSize(type); // ディスクリプタのサイズを取得
		}
		is_valid = descriptor_heap
			&& max_num_descriptors != 0
			&& increment_size != 0;
	}
	void DescriptorHeap::AllocateWithoutView(D3D12_CPU_DESCRIPTOR_HANDLE* out_cpu_desc_handle, D3D12_GPU_DESCRIPTOR_HANDLE* out_gpu_desc_handle)
	{
		if (current_descriptors_num >= max_num_descriptors)
			return;
		if (out_cpu_desc_handle && start_cpu.ptr)
			out_cpu_desc_handle->ptr = start_cpu.ptr + increment_size * current_descriptors_num;
		if (out_gpu_desc_handle && start_gpu.ptr)
			out_gpu_desc_handle->ptr = start_gpu.ptr + increment_size * current_descriptors_num;
		current_descriptors_num++;
	}
	CSUHeap::CSUHeap(unsigned int max_count)
		:DescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, max_count)
	{

		if (descriptor_heap)
			start_gpu = descriptor_heap->GetGPUDescriptorHandleForHeapStart();

	}
	std::unique_ptr<View> CSUHeap::CreateView(const VIEW_DESC& desc, ID3D12Resource* resource)
	{
		std::unique_ptr<View> view = nullptr;
		if (!resource)
			return view;
		if (current_descriptors_num >= max_num_descriptors)
			return view;
		auto device = DirectX12Manager::Instance()->GetDevice();
		D3D12_CPU_DESCRIPTOR_HANDLE cpu_handle = { };
		D3D12_GPU_DESCRIPTOR_HANDLE gpu_handle = {};
		AllocateWithoutView(&cpu_handle, &gpu_handle);
		switch (desc.type) {

		case VIEW_DESC::SRV: {

			D3D12_SHADER_RESOURCE_VIEW_DESC srv_desc = desc.srv_desc ? *desc.srv_desc : DEFAULT_VIEW_DESC_HELPER::GetDefaultSRVDesc(resource);
			device->CreateShaderResourceView(resource, &srv_desc, cpu_handle);
			view = std::make_unique<ShaderResourceView>(srv_desc, cpu_handle, gpu_handle, current_descriptors_num-1, resource, this);
			break;
		}
		case VIEW_DESC::CBV: {
			D3D12_CONSTANT_BUFFER_VIEW_DESC cbv_desc = desc.cbv_desc ? *desc.cbv_desc : DEFAULT_VIEW_DESC_HELPER::GetDefaultCBVDesc(resource);
			device->CreateConstantBufferView(&cbv_desc, cpu_handle);
			view = std::make_unique<ConstantBufferView>(cbv_desc, cpu_handle, gpu_handle, resource, this);

			break;
		}
		case VIEW_DESC::UAV:
			//device->CreateUnorderedAccessView(resource, nullptr, desc.uav_desc, handle);
			break;
		default:
			return view;
		}
		return view;

	}

	std::unique_ptr<View> RTVHeap::CreateView(const VIEW_DESC& desc, ID3D12Resource* resource)
	{

		std::unique_ptr<View> view = nullptr;
		if (!resource)
			return view;
		if (current_descriptors_num >= max_num_descriptors)
			return view;
		auto device = DirectX12Manager::Instance()->GetDevice();
		D3D12_CPU_DESCRIPTOR_HANDLE handle = {};
		AllocateWithoutView(&handle, nullptr);
		device->CreateRenderTargetView(resource, desc.rtv_desc, handle);
		D3D12_RENDER_TARGET_VIEW_DESC rtv_desc = desc.rtv_desc ? *desc.rtv_desc : DEFAULT_VIEW_DESC_HELPER::GetDefaultRTVDesc(resource);
		view = std::make_unique<RenderTargetView>(rtv_desc, handle, resource,this);
		current_descriptors_num++;
		return view;

	}
	std::unique_ptr<View> DSVHeap::CreateView(const VIEW_DESC& desc, ID3D12Resource* resource)
	{

		std::unique_ptr<View> view = nullptr;
		if (!resource)
			return view;
		if (current_descriptors_num >= max_num_descriptors)
			return view;
		auto device = DirectX12Manager::Instance()->GetDevice();
		D3D12_CPU_DESCRIPTOR_HANDLE handle = {};
		AllocateWithoutView(&handle, nullptr);
		device->CreateDepthStencilView(resource, desc.dsv_desc, handle);
		D3D12_DEPTH_STENCIL_VIEW_DESC dsv_desc = desc.dsv_desc ? *desc.dsv_desc : DEFAULT_VIEW_DESC_HELPER::GetDefaultDSVDesc(resource);
		view = std::make_unique<DepthStencilView>(dsv_desc, handle, resource, this);
		current_descriptors_num++;
		return view;
	}
}