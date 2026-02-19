#pragma once

//------------------------------------------------------------
// @brief アプリケーションマネージャー
// @brief アプリケーション全体の管理を行うクラス
// 
//------------------------------------------------------------

class ApplicationManager : public Singleton<ApplicationManager>
{
private:
	friend class Singleton<ApplicationManager>;
	ApplicationManager();

	// アプリケーション全体の管理を行うクラス
	//DirectXに関する管理はDirectXManagerクラスに任せる
	//以下のマネージャーを追加するとよい
	//・DirectXManager：DirectXの管理
	//・InputManager：入力デバイスの管理
	//・SceneManager：シーンの管理
	//・ResourceManager：リソースの管理
	//・AudioManager：音声の管理
	//・PhysicsManager：物理演算の管理
	//・TimeManager：時間の管理
	//・ThreadManager：スレッドの管理(今回はマルチスレッドは扱わない予定)


public:
	void Initialize() {}

	void Finalize() {}

};