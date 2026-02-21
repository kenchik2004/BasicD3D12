#pragma once

//D3D12には、デバイスコンテキストが存在しないため、DirectX11のようなデバイスコンテキストを表すクラスは必要ない。
//ただ、コマンドキューやコマンドリストを管理する必要はあるため、自作でD3D12用のコンテキストを作ることにする。
#include "System/SystemUtils/DeviceContext/ID3D12DeviceContext.h"
#include "System/SystemUtils/Descriptors/BaseView/View.h"
#include "System/SystemUtils/DescriptorHeaps/DescriptorHeap/DescriptorHeap.h"
namespace System {
	class DescriptorHeap;
	//-------------------------------------------------------------
	// @brief DirectX12マネージャー
	// @brief DirectX12のデバイスやコマンドキューなどの管理を行うクラス
	//-------------------------------------------------------------
	class DirectX12Manager : public Singleton<DirectX12Manager>
	{
	private:
		friend class Singleton<DirectX12Manager>;
		DirectX12Manager() = default;

		ComPtr<IDXGIFactory6> factory;
		ComPtr<ID3D12Device> device;
		D3D_FEATURE_LEVEL minimum_feature_level = D3D_FEATURE_LEVEL_11_0;

		//他のマネージャーと違い、Singletonを継承させない
		// D3D11では1対1の関係だった。しかしD3D12では、
		// 複数のコマンドリストを作成することができる
		// (というか、そうしないとD3D11よりパフォーマンスが下がる)
		//将来的に増やすが、現在はとりあえず1つだけ作成しておく
		std::unique_ptr<ID3D12DeviceContext> draw_context = nullptr;


		ComPtr<ID3D12CommandQueue> draw_queue;
		ComPtr<ID3D12Fence> draw_fence;
		size_t draw_fence_value = 0;

		//ComPtr<ID3D12CommandQueue> copy_queue;
		// 将来的に、描画用のコマンドキュー以外にも、コピー用のコマンドキューやコンピュート用のコマンドキューなど
		// を作成することも考えられるため、複数のコマンドキューを管理できるようにしておく
		// コンピュートキューまで管理すると滅茶苦茶に散らかるので、今は描画とコピーだけやっておこう
		//ComPtr<ID3D12CommandQueue> compute_queue;

		std::unique_ptr<DescriptorHeap> cbv_srv_uav_heap = nullptr;
		std::unique_ptr<DescriptorHeap> rtv_heap = nullptr;
		std::unique_ptr<DescriptorHeap> dsv_heap = nullptr;

		int CreteDevice(ComPtr<IDXGIAdapter>& dxgi_adapter);
		int CreateSingleCommandQueue(D3D12_COMMAND_LIST_TYPE context_type, ComPtr<ID3D12CommandQueue>& command_queue, ComPtr<ID3D12Fence>& fence_);
		int CreateCommandQueues();
		int CreateSingleContext(D3D12_COMMAND_LIST_TYPE context_type, std::unique_ptr<ID3D12DeviceContext>& context);
		int CreateContexts();
		int CreateFactory(ComPtr<IDXGIAdapter>& dxgi_adapter);

		int CreateDescriptorHeaps();

	public:
		IDXGIFactory6* GetFactory() const { return factory.Get(); }
		ID3D12Device* GetDevice() const { return device.Get(); }

		ID3D12CommandQueue* GetDrawQueue() const { return draw_queue.Get(); }
		//ID3D12CommandQueue* GetCopyQueue() const { return copy_queue.Get(); }

		ID3D12DeviceContext* GetDrawContext() const { return draw_context.get(); }


		//---------------------------------------------
		//初期化前にのみ呼ばれるべき関数群

		// @brief DirectX12の最低限必要な機能レベルを設定する関数
		void SetMinimumFeatureLevel(D3D_FEATURE_LEVEL level);


		//---------------------------------------------

		int WaitForFence(ComPtr<ID3D12Fence> fence_, const size_t& value);
		int DrawStart();

		std::unique_ptr<RenderTargetView> CreateRenderTargetView(ID3D12Resource* resource, const D3D12_RENDER_TARGET_VIEW_DESC& desc);
		std::unique_ptr<ShaderResourceView> CreateShaderResourceView(ID3D12Resource* resource, const D3D12_SHADER_RESOURCE_VIEW_DESC& desc);
		std::unique_ptr<DepthStencilView> CreateDepthStencilView(ID3D12Resource* resource, const D3D12_DEPTH_STENCIL_VIEW_DESC& desc);


		int Initialize();
		int Finalize();
	};
}