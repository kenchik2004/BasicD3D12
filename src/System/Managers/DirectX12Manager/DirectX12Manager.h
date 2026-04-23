#pragma once

//D3D12には、デバイスコンテキストが存在しないため、DirectX11のようなデバイスコンテキストを表すクラスは必要ない。
//ただ、コマンドキューやコマンドリストを管理する必要はあるため、自作でD3D12用のコンテキストを作ることにする。
#include "System/SystemUtils/DeviceContext/ID3D12DeviceContext.h"
#include "System/SystemUtils/CommandQueue/CommandQueue.h"
namespace System {
	class DescriptorHeap;
	class RTVHeap;
	class DSVHeap;
	class CSUHeap;
	class View;
	class RenderTargetView;
	class ShaderResourceView;
	class ConstantBufferView;
	class DepthStencilView;
	//-------------------------------------------------------------
	// @brief DirectX12マネージャー
	// @brief DirectX12のデバイスの管理を行うクラス
	//-------------------------------------------------------------
	class DirectX12Manager
	{
	public:
		static constexpr size_t DRAW_CONTEXT_FRAME_COUNT = 3;	// 描画コンテキストの数。将来的に、複数の描画コンテキストを使用することも考えられるため、定数として定義しておく

	private:
		DirectX12Manager() = default;

		ComPtr<IDXGIFactory6> factory;
		ComPtr<IDXGIAdapter> dxgi_adapter;
		ComPtr<ID3D12Device> device;
		D3D_FEATURE_LEVEL minimum_feature_level = D3D_FEATURE_LEVEL_11_0;

		//他のマネージャーと違い、Singletonを継承させない
		// D3D11では1対1の関係だった。しかしD3D12では、
		// 複数のコマンドリストを作成することができる
		// (というか、そうしないとD3D11よりパフォーマンスが下がる)
		//将来的に増やすが、現在はとりあえず1つだけ作成しておく
		std::array<std::unique_ptr<ID3D12DeviceContext>, DRAW_CONTEXT_FRAME_COUNT> draw_context = {};
		unsigned int current_draw_context_index = 0;	// 現在使用している描画コンテキストのインデックス。これを切り替えることで、複数の描画コンテキストを使用することができるようになる。

		std::unique_ptr<ID3D12DeviceContext> copy_context = nullptr;

		std::unique_ptr<CommandQueue> draw_command_queue = nullptr;

		std::unique_ptr<CommandQueue> copy_command_queue = nullptr;
		// 将来的に、描画用のコマンドキュー以外にも、コピー用のコマンドキューやコンピュート用のコマンドキューなど
		// を作成することも考えられるため、複数のコマンドキューを管理できるようにしておく
		// コンピュートキューまで管理すると滅茶苦茶に散らかるので、今は描画とコピーだけやっておこう
		//std::unique_ptr<CommandQueue> compute_command_queue = nullptr;

		std::unique_ptr<CSUHeap> cbv_srv_uav_heap = nullptr;
		std::unique_ptr<RTVHeap> rtv_heap = nullptr;
		std::unique_ptr<DSVHeap> dsv_heap = nullptr;

		int CreteDevice(ComPtr<IDXGIAdapter>& dxgi_adapter);
		int CreateCommandQueues();
		int CreateSingleContext(D3D12_COMMAND_LIST_TYPE context_type, std::unique_ptr<ID3D12DeviceContext>& context);
		int CreateContexts();
		int CreateFactory(ComPtr<IDXGIAdapter>& dxgi_adapter);

		int CreateDescriptorHeaps();

	public:
		IDXGIFactory6* GetFactory() const { return factory.Get(); }
		ID3D12Device* GetDevice() const { return device.Get(); }

		CommandQueue* GetDrawQueue() const { return draw_command_queue.get(); }
		CommandQueue* GetCopyQueue() const { return copy_command_queue.get(); }

		HRESULT ResetDrawContext();
		HRESULT ResetAllDrawContexts();
		HRESULT CloseDrawContext();
		ID3D12DeviceContext* GetDrawContext() const { return draw_context[current_draw_context_index].get(); }
		ID3D12DeviceContext* GetCopyContext() const { return copy_context.get(); }
		CSUHeap* GetCBVSRVUAVHeap() const { return cbv_srv_uav_heap.get(); }
		RTVHeap* GetRTVHeap() const { return rtv_heap.get(); }
		DSVHeap* GetDSVHeap() const { return dsv_heap.get(); }
		const unsigned int GetFrameIndex() const { return current_draw_context_index; }	// 現在のフレームインデックスを取得する関数。これを使用して、描画コンテキストの切り替えを行うことができるようになる。


		//---------------------------------------------
		//初期化前にのみ呼ばれるべき関数群

		// @brief DirectX12の最低限必要な機能レベルを設定する関数
		void SetMinimumFeatureLevel(D3D_FEATURE_LEVEL level);


		//---------------------------------------------

		int DrawBegin();
		int DrawEnd();

		std::unique_ptr<RenderTargetView> CreateRenderTargetView(ID3D12Resource* resource, D3D12_RENDER_TARGET_VIEW_DESC* desc);
		std::unique_ptr<ShaderResourceView> CreateShaderResourceView(ID3D12Resource* resource, D3D12_SHADER_RESOURCE_VIEW_DESC* desc);
		std::unique_ptr<ConstantBufferView> CreateConstantBufferView(ID3D12Resource* resource, D3D12_CONSTANT_BUFFER_VIEW_DESC* desc);
		std::unique_ptr<DepthStencilView> CreateDepthStencilView(ID3D12Resource* resource, D3D12_DEPTH_STENCIL_VIEW_DESC* desc);


		int Initialize();
		int Finalize();
		static DirectX12Manager* Instance();
	};
}