#include "IndexBuffer.h"

#include "System/Managers/DirectX12Manager/DirectX12Manager.h"
#include "System/SystemUtils/DeviceContext/ID3D12DeviceContext.h"
#include "System/SystemUtils/CommandQueue/CommandQueue.h"
namespace System {

	IndexBuffer::IndexBuffer(std::vector<unsigned int>& indices, D3D12_HEAP_TYPE heap_type)
	{
		index_data = indices;
		heap_properties.Type = D3D12_HEAP_TYPE_UPLOAD;
		heap_properties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
		heap_properties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
		heap_properties.CreationNodeMask = 0;
		heap_properties.VisibleNodeMask = 0;

		resource_desc.Width = sizeof(unsigned int) * index_data.size();

		resource_desc.Alignment = 0;
		resource_desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
		resource_desc.Height = 1;
		resource_desc.DepthOrArraySize = 1;
		resource_desc.MipLevels = 1;
		resource_desc.Format = DXGI_FORMAT_UNKNOWN;
		resource_desc.SampleDesc.Count = 1;
		resource_desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
		resource_desc.Flags = D3D12_RESOURCE_FLAG_NONE;

		HRESULT hr = DirectX12Manager::Instance()->GetDevice()->CreateCommittedResource(&heap_properties, D3D12_HEAP_FLAG_NONE, &resource_desc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(d3d_resource.GetAddressOf()));
		if (FAILED(hr)) {
			return;
		}
		ib_view.BufferLocation = d3d_resource->GetGPUVirtualAddress();
		ib_view.Format = DXGI_FORMAT_R32_UINT;
		ib_view.SizeInBytes = static_cast<unsigned int>(index_data.size() * sizeof(unsigned int));
		//インデックスバッファにデータを転送する
		{
			void* mapped_data = nullptr;
			HRESULT hr = d3d_resource->Map(0, nullptr, &mapped_data);
			if (FAILED(hr)) {
				return;
			}
			std::copy(index_data.begin(), index_data.end(), reinterpret_cast<unsigned int*>(mapped_data));
			d3d_resource->Unmap(0, nullptr);
		}

		if (heap_type == D3D12_HEAP_TYPE_DEFAULT) {
			//デフォルトヒープを使用する場合は、今作成したバッファをGPU専用バッファにコピーする
			//GPU専用バッファを作成する
			//ヒーププロパティ以外は同じものを流用できる
			ComPtr<ID3D12Resource> gpu_buffer;
			D3D12_HEAP_PROPERTIES gpu_heap_properties = heap_properties;
			gpu_heap_properties.Type = D3D12_HEAP_TYPE_DEFAULT;
			gpu_heap_properties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
			gpu_heap_properties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
			gpu_heap_properties.CreationNodeMask = 0;
			gpu_heap_properties.VisibleNodeMask = 0;

			HRESULT hr = DirectX12Manager::Instance()->GetDevice()->CreateCommittedResource(&gpu_heap_properties, D3D12_HEAP_FLAG_NONE, &resource_desc, D3D12_RESOURCE_STATE_COMMON, nullptr, IID_PPV_ARGS(gpu_buffer.GetAddressOf()));
			if (FAILED(hr)) {
				return;
			}

			ID3D12DeviceContext* context = DirectX12Manager::Instance()->GetCopyContext();
			if (!context || !context->IsValid())
			{
				return;
			}
			ID3D12GraphicsCommandList* cmd_list = context->GetCommandList();
			std::vector<ID3D12DeviceContext*> contexts = { context };

			CommandQueue* copy_queue = DirectX12Manager::Instance()->GetCopyQueue();

			context->ResetCommandList();
			cmd_list->CopyResource(gpu_buffer.Get(), d3d_resource.Get());

			context->CloseCommandList();
			copy_queue->Execute(contexts);
			//実行を待たなければ、コピー中にアップロードバッファが破棄されてしまう可能性があるため、コピーキューの完了を待つ
			copy_queue->WaitForCompletion(context);
			//コピーが完了したら、GPU専用バッファをこのクラスのリソースとして使用する
			d3d_resource.Swap(gpu_buffer);

			//ビューの更新
			ib_view.BufferLocation = d3d_resource->GetGPUVirtualAddress();
			ib_view.Format = DXGI_FORMAT_R32_UINT;
			ib_view.SizeInBytes = static_cast<unsigned int>(index_data.size() * sizeof(unsigned int));

		}
		is_valid = true;

	}

}