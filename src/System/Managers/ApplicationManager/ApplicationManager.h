#pragma once


namespace System {
	//------------------------------------------------------------
	// @brief アプリケーションマネージャー
	// @brief アプリケーション全体の管理を行うクラス
	// 
	//------------------------------------------------------------
	class ApplicationManager : public Singleton<ApplicationManager>
	{
	private:
		friend class Singleton<ApplicationManager>;
		ApplicationManager() = default;

		// アプリケーション全体の管理を行うクラス
		//以下のマネージャーを追加するとよい
		//・DirectXManager：DirectXデバイスや描画などのコマンド管理
		//・WindowManager：ウィンドウとスワップチェインの管理
		//・ResourceManager：リソースの管理(テクスチャやモデルなど)
		//・MaterialManager：マテリアルの管理(シェーダーやパイプラインステートなど)
		//・InputManager：入力デバイスの管理
		//・SceneManager：シーンの管理
		//・AudioManager：音声の管理
		//・PhysicsManager：物理演算の管理
		//・TimeManager：時間の管理
		//・ThreadManager：スレッドの管理(今回はマルチスレッドは扱わない予定)


	public:
		int RunApplication();
		int Initialize();
		int MainLoop();
		int Finalize();

	};
}