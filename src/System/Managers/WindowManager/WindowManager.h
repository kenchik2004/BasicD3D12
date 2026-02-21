#pragma once
#include "System/SystemUtils/Descriptors/BaseView/View.h"
namespace System {
	//-------------------------------------------------------------
	// @brief ウィンドウマネージャー
	// @brief ウィンドウとスワップチェインの管理を行うクラス
	//-------------------------------------------------------------
	class WindowManager : public Singleton<WindowManager>
	{
	private:
		friend class Singleton<WindowManager>;
		WindowManager() = default;

		HWND window_handle;
		std::wstring window_class_name;
		std::wstring window_name;

		unsigned int window_width;
		unsigned int window_height;

		ComPtr<IDXGISwapChain4> swap_chain;
		int CreateMainWindow();
		// バックバッファのリソースとRTVを管理するための配列。スワップチェインのバッファ数は2に固定しているため、要素数も2にしている。
		struct Texture {

		private:
			ComPtr<ID3D12Resource> d3d_resource;
			std::unique_ptr<RenderTargetView> rtv;
			std::unique_ptr<DepthStencilView> dsv;
			std::unique_ptr<ShaderResourceView> srv;
		public:
			Texture(ComPtr<ID3D12Resource> d3d_resource_,
				std::unique_ptr<RenderTargetView> rtv_,
				std::unique_ptr<DepthStencilView> dsv_,
				std::unique_ptr<ShaderResourceView> srv_) {
				d3d_resource = d3d_resource_;
				rtv = std::move(rtv_);
				dsv = std::move(dsv_);
				srv = std::move(srv_);

			}
			ID3D12Resource* Resource() { return d3d_resource.Get(); }
			RenderTargetView* Rtv() { return rtv.get(); }
			DepthStencilView* Dsv() { return dsv.get(); }
			ShaderResourceView* Srv() { return srv.get(); }
		};
		std::array<std::unique_ptr<Texture>, 2> back_buffers;

		static LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	public:
		//-------------------------------------------------------------
		// @brief ウィンドウの情報を設定する関数
		//
		// @param [in] class_name ウィンドウクラス名
		// @param [in] window_name ウィンドウタイトルバーに表示される名前
		// @param [in] width ウィンドウの幅（ピクセル単位）
		// @param [in] height ウィンドウの高さ（ピクセル単位）
		// @warning ウィンドウの情報は、ウィンドウの作成前に設定する必要がある。ウィンドウの作成後にこの関数を呼び出しても、ウィンドウの情報は変更されない。
		//-------------------------------------------------------------
		void SetWindowInfo(const std::wstring& class_name, const std::wstring& window_name, unsigned int width, unsigned int height) {
			if (window_handle)
				return;		// ウィンドウがすでに作成されている場合は、ウィンドウの情報を変更しない
			this->window_class_name = class_name;
			this->window_name = window_name;
			this->window_width = width;
			this->window_height = height;
		}

		IDXGISwapChain4* GetSwapChain() const { return swap_chain.Get(); }
		Texture* GetCurrentBackBuffer() const {
			UINT back_buffer_index = swap_chain->GetCurrentBackBufferIndex();		// 現在のバックバッファのインデックスを取得
			return back_buffers[back_buffer_index].get();
		}

		//--------------------------------------------------------------
		// @brief スワップチェインを作成する関数
		//--------------------------------------------------------------
		int CreateSwapChain();
		int CreateBackBuffers();
		int ReleaseSwapChain() {
			swap_chain.Reset();
			return 0;
		}

		int Initialize();
		int Finalize();

		int ScreenFlip() {
			swap_chain->Present(1, 0);		// 垂直同期ありで画面を更新する
			return 0;
		}
	};
}

