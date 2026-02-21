#include "ApplicationManager.h"
#include "System/Managers/WindowManager/WindowManager.h"
#include "System/Managers/DirectX12Manager/DirectX12Manager.h"

namespace System {

	int ApplicationManager::RunApplication()
	{

		//アプリケーションの初期化
		if (Initialize() != 0)
		{
			//初期化に失敗した場合は、アプリを終了する
			Finalize();
			return -1;
		}

		MainLoop();		// メインループ


		return Finalize();		// アプリケーションの終了処理

	}
	int ApplicationManager::Initialize()
	{
		//ウィンドウマネージャーの初期化
		if (WindowManager::Instance()->Initialize() != 0) return -1;
		if (DirectX12Manager::Instance()->Initialize() != 0) return -1;
		if (WindowManager::Instance()->CreateSwapChain() != 0) return -1;



		return 0;
	}
	int ApplicationManager::MainLoop()
	{
		MSG msg;		// Windowsのメッセージを格納する構造体

		while (true) {

			// メッセージが来ているか確認。来ていればmsgに格納して、キューから削除する
			// 英語でPeek = 「覗く」という意味。PeekMessageは「メッセージを覗いてみる」という意味で、メッセージが来ているか確認する関数
			if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
				// 来ていた場合、TranslateMessageでキーボード入力などのメッセージを翻訳して、DispatchMessageでウィンドウプロシージャに送る
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
			// WM_QUIT(「アプリ終わるでー」のメッセージ)が来ていたら、ループを抜けてアプリを終了する
			if (msg.message == WM_QUIT) {
				break;
			}

			if constexpr (true) {
				// ここにゲームの更新や描画のコードを入れることになる

				auto back_buffer = WindowManager::Instance()->GetCurrentBackBuffer();
				auto handle = back_buffer->Rtv()->GetCPUHandle();
				auto cmd_list = DirectX12Manager::Instance()->GetDrawContext()->GetCommandList();
				auto cmd_allocator = DirectX12Manager::Instance()->GetDrawContext()->GetCommandAllocator();

				D3D12_RESOURCE_BARRIER begin_barrier = {};
				begin_barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
				begin_barrier.Transition.pResource = back_buffer->Resource();
				begin_barrier.Transition.Subresource = 0;
				begin_barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
				begin_barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
				cmd_list->ResourceBarrier(1, &begin_barrier);

				cmd_list->OMSetRenderTargets(1, &handle, FALSE, nullptr);
				float clear_color[4] = { 0.0f, 0.0f, 1.0f, 1.0f };
				cmd_list->ClearRenderTargetView(handle, clear_color, 0, nullptr);

				D3D12_RESOURCE_BARRIER end_barrier = {};
				end_barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
				end_barrier.Transition.pResource = back_buffer->Resource();
				end_barrier.Transition.Subresource = 0;
				end_barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
				end_barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
				cmd_list->ResourceBarrier(1, &end_barrier);

				cmd_list->Close();
				DirectX12Manager::Instance()->DrawStart();
				cmd_allocator->Reset();
				cmd_list->Reset(cmd_allocator, nullptr);
			}


		}
		return 0;

	}
	int ApplicationManager::Finalize()
	{
		WindowManager::Instance()->ReleaseSwapChain();
		DirectX12Manager::Instance()->Finalize();
		WindowManager::Instance()->Finalize();

		return 0;
	}
}
