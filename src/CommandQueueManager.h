#pragma once

// コマンドキュー、フェンス、フェンス値を所有・管理するクラス
class CommandQueueManager {
public:
    // コンストラクタ: コマンドキューとフェンスを作成・初期化する
    CommandQueueManager(ID3D12Device* device);
    ~CommandQueueManager();

    // 渡されたコマンドリストをキューに積み、実行する
    void ExecuteCommandLists(UINT num_lists, ID3D12CommandList* const* lists);

    // キューの実行完了を待つ
    void WaitForExecution();

    ID3D12CommandQueue* GetCommandQueue() const { return command_queue_.Get(); }

private:
    ComPtr<ID3D12CommandQueue> command_queue_;
    ComPtr<ID3D12Fence>        fence_;
    UINT64                     fence_value_ = 0;
    HANDLE                     fence_event_ = nullptr;
};
