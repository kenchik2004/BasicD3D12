
#include "System/Managers/ApplicationManager/ApplicationManager.h" 
#include "System/Managers/WindowManager/WindowManager.h"







int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{

	std::wstring class_name = L"BasicD3D12Window";		// ウィンドウクラス名
	std::wstring window_name = L"そんなウィンドウ名で大丈夫か?";	// ウィンドウタイトル
	unsigned int window_width = 1280;		// ウィンドウの幅
	unsigned int window_height = 720;		// ウィンドウの高さ

	// ウィンドウの情報を設定
	System::WindowManager::Instance()->SetWindowInfo(class_name, window_name, window_width, window_height);

	// アプリケーションを実行
	//この中に前回用意したウィンドウ作成やDirectX12の初期化、メインループなどのコードが入っている
	//更に、メインループ終了後のファイナライズ(終了処理)もこの中に入っている
	System::ApplicationManager::Instance()->RunApplication();

	return 0;

}
