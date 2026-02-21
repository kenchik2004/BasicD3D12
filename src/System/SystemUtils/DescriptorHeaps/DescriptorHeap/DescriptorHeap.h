#include "System/SystemUtils/Descriptors/BaseView/View.h"

namespace System {




	//-------------------------------------------------------------
	// @brief ディスクリプタヒープ
	// @brief D3D12のディスクリプタヒープを管理するクラス
	//-------------------------------------------------------------


	class DescriptorHeap
	{
	public:
		DescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE type_, unsigned int max_count);
		virtual ~DescriptorHeap() { descriptor_heap.Reset(); }
		virtual std::unique_ptr<View> CreateView(const VIEW_DESC& desc, ID3D12Resource* resource) = 0;

		bool IsValid()const {
			return descriptor_heap
				&& max_num_descriptors != 0
				&& increment_size != 0;
		}
		// ディスクリプタヒープを管理する関数をここに追加していく
	protected:
		ComPtr<ID3D12DescriptorHeap> descriptor_heap;
		D3D12_DESCRIPTOR_HEAP_TYPE type;
		D3D12_CPU_DESCRIPTOR_HANDLE start;
		unsigned int max_num_descriptors = 0;
		unsigned int current_descriptors_num = 0;
		unsigned int increment_size = 0;
	};

	class CSUHeap : public DescriptorHeap
	{
	public:
		CSUHeap(unsigned int max_count) :DescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, max_count) {}
		std::unique_ptr<View> CreateView(const VIEW_DESC& desc, ID3D12Resource* resource) override;

	};

	class RTVHeap : public DescriptorHeap
	{
	public:
		RTVHeap(unsigned int max_count) : DescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_RTV, max_count) {}
		std::unique_ptr<View> CreateView(const VIEW_DESC& desc, ID3D12Resource* resource) override;
	};

	class DSVHeap : public DescriptorHeap
	{
	public:
		DSVHeap(unsigned int max_count) : DescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_DSV, max_count) {}
		std::unique_ptr<View> CreateView(const VIEW_DESC& desc, ID3D12Resource* resource) override;
	};
}

