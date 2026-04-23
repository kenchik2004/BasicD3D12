#include "System/SystemUtils/Descriptors/View/View.h"

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
		void AllocateWithoutView(D3D12_CPU_DESCRIPTOR_HANDLE* out_cpu_desc_handle, D3D12_GPU_DESCRIPTOR_HANDLE* out_gpu_desc_handle);

		bool IsValid() const {
			return is_valid;
		}

		// ディスクリプタヒープを管理する関数をここに追加していく
		ID3D12DescriptorHeap* GetHeap() const { return descriptor_heap.Get(); }
		const D3D12_CPU_DESCRIPTOR_HANDLE& GetStartCPUHandle() const { return start_cpu; }
		const D3D12_GPU_DESCRIPTOR_HANDLE& GetStartGPUHandle() const { return start_gpu; }
		const D3D12_DESCRIPTOR_HEAP_TYPE& GetType() const { return type; }
		const unsigned int GetMaxNumDescriptors() const { return max_num_descriptors; }
		const unsigned int GetCurrentNumDescriptors() const { return current_descriptors_num; }
		const unsigned int GetIncrementSize() const { return increment_size; }
	protected:
		ComPtr<ID3D12DescriptorHeap> descriptor_heap;
		D3D12_DESCRIPTOR_HEAP_TYPE type;
		D3D12_CPU_DESCRIPTOR_HANDLE start_cpu;
		D3D12_GPU_DESCRIPTOR_HANDLE start_gpu;
		unsigned int max_num_descriptors = 0;
		unsigned int current_descriptors_num = 0;
		unsigned int increment_size = 0;
		bool is_valid = false;
	};

	class CSUHeap : public DescriptorHeap
	{
	public:
		CSUHeap(unsigned int max_count);
		std::unique_ptr<View> CreateView(const VIEW_DESC& desc, ID3D12Resource* resource) override;

	private:
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