#include "DirectX12Manager.h" 
#include "System/SystemUtils/DeviceContext/ID3D12DeviceContext.h"
#include "System/SystemUtils/DescriptorHeaps/DescriptorHeap/DescriptorHeap.h"
#include "System/Managers/WindowManager/WindowManager.h"
#ifdef _DEBUG
#include <dxgidebug.h>
#pragma comment(lib, "dxguid.lib")
#endif // _DEBUG


namespace System {



	void DirectX12Manager::SetMinimumFeatureLevel(D3D_FEATURE_LEVEL level)
	{
		// すでにデバイスが作成されている場合は、機能レベルを変更できないため、何もせずに関数を終了する
		if (device) return;

		if (level < D3D_FEATURE_LEVEL_11_0) {
			// DirectX12の最低限必要な機能レベルはD3D_FEATURE_LEVEL_11_0なので、
			// 引数がそれより小さい場合は、最低限必要な機能レベルをD3D_FEATURE_LEVEL_11_0に設定する
			level = D3D_FEATURE_LEVEL_11_0;
		}

		minimum_feature_level = level;
	}



	int DirectX12Manager::WaitForFence(ComPtr<ID3D12Fence> fence_, const size_t& value)
	{
		while (fence_->GetCompletedValue() < value)
		{
			// フェンスの完了値が、引数で指定された値に達するまで待機する
		}
		return 0;
	}

	int DirectX12Manager::DrawStart()
	{
		ID3D12CommandList* command_lists[] = { draw_context->GetCommandList() };
		draw_queue->ExecuteCommandLists(1, command_lists);
		WindowManager::Instance()->ScreenFlip();
		draw_queue->Signal(draw_fence.Get(), ++draw_fence_value);
		WaitForFence(draw_fence, draw_fence_value);
		return 0;
	}

	std::unique_ptr<RenderTargetView> DirectX12Manager::CreateRenderTargetView(ID3D12Resource* resource,D3D12_RENDER_TARGET_VIEW_DESC* desc) {
		VIEW_DESC view_desc = {};
		view_desc.type = VIEW_DESC::VIEW_TYPE::RTV;
		view_desc.rtv_desc = desc;
		View* view = rtv_heap->CreateView(view_desc, resource).release();
		auto cast_view = static_cast<RenderTargetView*>(view);
		return std::unique_ptr<RenderTargetView>(cast_view);

	}

	std::unique_ptr<ShaderResourceView> DirectX12Manager::CreateShaderResourceView(ID3D12Resource* resource, D3D12_SHADER_RESOURCE_VIEW_DESC* desc)
	{
		VIEW_DESC view_desc = {};
		view_desc.type = VIEW_DESC::VIEW_TYPE::SRV;
		view_desc.srv_desc = desc;
		View* view = cbv_srv_uav_heap->CreateView(view_desc, resource).release();
		auto cast_view = static_cast<ShaderResourceView*>(view);
		return std::unique_ptr<ShaderResourceView>(cast_view);
	}

	std::unique_ptr<DepthStencilView> DirectX12Manager::CreateDepthStencilView(ID3D12Resource* resource, D3D12_DEPTH_STENCIL_VIEW_DESC* desc)
	{
		VIEW_DESC view_desc = {};
		view_desc.type = VIEW_DESC::VIEW_TYPE::DSV;
		view_desc.dsv_desc = desc;
		View* view = dsv_heap->CreateView(view_desc, resource).release();
		auto cast_view = static_cast<DepthStencilView*>(view);
		return std::unique_ptr<DepthStencilView>(cast_view);
	}

	int DirectX12Manager::Initialize()
	{
		ComPtr<IDXGIAdapter> dxgi_adapter = nullptr;
		if (CreateFactory(dxgi_adapter) < 0) {
			return -1;
		}
		if (CreteDevice(dxgi_adapter) < 0) {
			return -1;
		}
		if (CreateCommandQueues() < 0) {
			return -1;
		}
		if (CreateContexts() != 0) {
			return -1;
		}
		if (CreateDescriptorHeaps() != 0) {
			return -1;
		}
		return 0;
	}
	int DirectX12Manager::Finalize()
	{
		WaitForFence(draw_fence, draw_fence_value);
		draw_context.reset();
		draw_queue.Reset();
		draw_fence.Reset();
		//copy_queue.Reset();
		rtv_heap.reset();
		dsv_heap.reset();
		cbv_srv_uav_heap.reset();
#ifdef _DEBUG
		{

			ComPtr<IDXGIDebug1> debug;
			if (SUCCEEDED(DXGIGetDebugInterface1(0, IID_PPV_ARGS(&debug)))) {
				factory.Reset();
				OutputDebugString(L"DXGIFactoryを解放します。残っているオブジェクト一覧\n");
				debug->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_DETAIL);
			}
			ComPtr<ID3D12DebugDevice> debug_device;
			if (SUCCEEDED(device.As(&debug_device))) {
				device.Reset();
				OutputDebugString(L"ID3D12Deviceを解放します。残っているオブジェクト一覧\n");
				debug_device->ReportLiveDeviceObjects(D3D12_RLDO_IGNORE_INTERNAL);
				int ref = device.Reset();
				if (ref == 0)
				{
					OutputDebugString(L"ID3D12Deviceを正常に解放しました。\n");
				}

			}
		}
#else
		factory.Reset();
		int ref = device.Reset();
		if (ref > 0) {
			OutputDebugString(L"ID3D12Deviceを解放します。残っているオブジェクトがある可能性があります。\n");
		}
		else
		{
			OutputDebugString(L"ID3D12Deviceを正常に解放しました。\n");
		}

#endif

		return 0;
	}
	DirectX12Manager* DirectX12Manager::Instance()
	{
		static DirectX12Manager manager;
		return &manager;
	}
	int DirectX12Manager::CreteDevice(ComPtr<IDXGIAdapter>& dxgi_adapter)
	{

		// DirectX12のデバイスを作成するために、引数で指定されたアダプタを使用して、機能レベルを順番に試していく
		//この順番は、高い順に試すのがよい。
		static constexpr std::array<D3D_FEATURE_LEVEL, 4> feature_levels = {
			D3D_FEATURE_LEVEL_12_1,
			D3D_FEATURE_LEVEL_12_0,
			D3D_FEATURE_LEVEL_11_1,
			D3D_FEATURE_LEVEL_11_0
		};


		for (size_t i = 0; i < feature_levels.size(); ++i) {

			// 引数で指定された最低限必要な機能レベルより小さい機能レベルは、デバイスの作成に使用しない
			if (feature_levels[i] < minimum_feature_level) continue;

			HRESULT hr = D3D12CreateDevice(
				dxgi_adapter.Get(),
				feature_levels[i],
				IID_PPV_ARGS(device.GetAddressOf())
			);

			if (hr == S_OK) {
				// デバイスの作成に成功した場合は、成功を示す 0 を返す
				return 0;
			}
		}
		// デバイスの作成に失敗した場合は、失敗を示す -1 を返す
		return -1;

	}

	int DirectX12Manager::CreateSingleCommandQueue(D3D12_COMMAND_LIST_TYPE context_type, ComPtr<ID3D12CommandQueue>& command_queue, ComPtr<ID3D12Fence>& fence_)
	{

		D3D12_COMMAND_QUEUE_DESC desc = {};

		//コマンドリストにも種類がある。代表的なのは、
		// D3D12_COMMAND_LIST_TYPE_DIRECT：グラフィックスコマンドリスト。描画やコンピュートのコマンドを記録するためのコマンドリスト
		// D3D12_COMMAND_LIST_TYPE_COPY：コピーコマンドリスト。リソースのコピーや転送のコマンドを記録するためのコマンドリスト
		// D3D12_COMMAND_LIST_TYPE_COMPUTE：コンピュートコマンドリスト。コンピュートシェーダーのコマンドを記録するためのコマンドリスト
		// コマンドキューも、コマンドリストの種類に合わせて、同じ種類のコマンドキューを作成する必要がある


		desc.Type = context_type;	//指定されたコマンドリストの種類に合わせて、コマンドキューの種類を設定
		desc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;	// コマンドキューの優先度を「通常」に設定
		desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;	// コマンドキューのフラグは特に設定しない
		desc.NodeMask = 0;							// ノードマスクは特に設定しない
		HRESULT hr = device->CreateCommandQueue(&desc, IID_PPV_ARGS(command_queue.GetAddressOf()));

		if (hr != S_OK) {
			// コマンドキューの作成に失敗した場合は、失敗を示す -1 を返す
			return -1;
		}
		hr = device->CreateFence(draw_fence_value, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(fence_.GetAddressOf()));

		if (hr != S_OK) {
			// フェンスの作成に失敗した場合は、失敗を示す -1 を返す
			return -1;
		}

		return 0;
	}

	int DirectX12Manager::CreateCommandQueues()
		// コマンドキューの作成
	{
		if (CreateSingleCommandQueue(D3D12_COMMAND_LIST_TYPE_DIRECT, draw_queue, draw_fence) != 0) {
			return -1;
		}
		//if (CreateSingleCommandQueue(D3D12_COMMAND_LIST_TYPE_COPY, copy_queue) != 0) {
		//	return -1;
		//}
		return 0;
	}

	int DirectX12Manager::CreateSingleContext(D3D12_COMMAND_LIST_TYPE context_type, std::unique_ptr<ID3D12DeviceContext>& context_)
	{
		context_ = std::make_unique<ID3D12DeviceContext>(GetDevice(), D3D12_COMMAND_LIST_TYPE_DIRECT);
		// コンテキストの作成に失敗した場合は、失敗を示す -1 を返す
		if (!context_ || !context_->IsValid()) {
			return -1;
		}
		return 0;
	}

	int DirectX12Manager::CreateContexts()
	{
		if (CreateSingleContext(D3D12_COMMAND_LIST_TYPE_DIRECT, draw_context) != 0) {
			return -1;
		}
		// 将来的に、描画用のコンテキスト以外にも、コピー用のコンテキストやコンピュート用のコンテキストなどを
		// 作成することも考えられるため、複数のコンテキストを管理できるようにしておく

		return 0;
	}

	int DirectX12Manager::CreateFactory(ComPtr<IDXGIAdapter>& dxgi_adapter)
	{
		unsigned int dxgi_factory_flags = 0;
#ifdef _DEBUG
		{
			ComPtr<ID3D12Debug> debug_controller;
			if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(debug_controller.GetAddressOf())))) {
				debug_controller->EnableDebugLayer();	// デバッグレイヤーを有効にする
				dxgi_factory_flags |= DXGI_CREATE_FACTORY_DEBUG;	// デバッグレイヤーを有効にするフラグを設定
			}


		}
#endif

		//DXGIFactoryの作成
		{
			HRESULT hr = CreateDXGIFactory2(dxgi_factory_flags, IID_PPV_ARGS(factory.GetAddressOf()));
			if (hr != S_OK) {
				// DXGIFactoryの作成に失敗した場合は、失敗を示す -1 を返す
				return -1;
			}
			//
			std::vector<ComPtr<IDXGIAdapter>> adapters;
			ComPtr<IDXGIAdapter> tmp_adapter = nullptr;
			for (unsigned int i = 0;
				factory->EnumAdapters(i, tmp_adapter.GetAddressOf()) != DXGI_ERROR_NOT_FOUND;
				++i) {
				adapters.push_back(tmp_adapter);
				tmp_adapter = nullptr;
			}

			//一旦使用アダプタをnullにしておく
			tmp_adapter = nullptr;

			// 使用可能なアダプタの中から、NVIDIA製やAMD製のGPUを優先的に選択する
			for (auto& adapter : adapters) {
				DXGI_ADAPTER_DESC desc = {};
				adapter->GetDesc(&desc);
				std::wstring string_desc = desc.Description;

				// NVIDIA製やAMD製のGPUを優先的に選択するため、GPUの説明に「NVIDIA」や「AMD」が含まれているか確認する
				if (string_desc.find(L"NVIDIA") != string_desc.npos) {
					tmp_adapter = adapter;
					break;
				}
				if (string_desc.find(L"AMD") != string_desc.npos) {
					tmp_adapter = adapter;
					break;
				}
			}
			dxgi_adapter = tmp_adapter;
		}
		if (dxgi_adapter == nullptr) {
			// 使用可能なアダプタが見つからなかった場合は、失敗を示す -1 を返す
			return -1;
		}

		return 0;
	}
	int DirectX12Manager::CreateDescriptorHeaps()
	{
		rtv_heap = std::make_unique<RTVHeap>(100);
		dsv_heap = std::make_unique<DSVHeap>(100);
		cbv_srv_uav_heap = std::make_unique<CSUHeap>(100);

		return 0;
	}
}
