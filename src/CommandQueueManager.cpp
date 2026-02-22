#include "precompile.h"
#include "CommandQueueManager.h"

#include <stdexcept>

CommandQueueManager::CommandQueueManager(ID3D12Device* device)
    : fence_value_(0), fence_event_(nullptr)
{
    // コマンドキューを作成
    D3D12_COMMAND_QUEUE_DESC queue_desc = {};
    queue_desc.Type  = D3D12_COMMAND_LIST_TYPE_DIRECT;
    queue_desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
    HRESULT hr = device->CreateCommandQueue(&queue_desc, IID_PPV_ARGS(&command_queue_));
    if (FAILED(hr)) {
        throw std::runtime_error("Failed to create command queue");
    }

    // フェンスを作成（初期値 0）
    hr = device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence_));
    if (FAILED(hr)) {
        throw std::runtime_error("Failed to create fence");
    }

    // CPU-GPU 同期用イベントを作成
    fence_event_ = CreateEvent(nullptr, FALSE, FALSE, nullptr);
    if (fence_event_ == nullptr) {
        throw std::runtime_error("Failed to create fence event");
    }
}

CommandQueueManager::~CommandQueueManager()
{
    if (fence_event_) {
        CloseHandle(fence_event_);
        fence_event_ = nullptr;
    }
}

// 渡されたコマンドリストをキューに積み、実行する
void CommandQueueManager::ExecuteCommandLists(UINT num_lists, ID3D12CommandList* const* lists)
{
    command_queue_->ExecuteCommandLists(num_lists, lists);
}

// キューの実行完了を待つ
void CommandQueueManager::WaitForExecution()
{
    ++fence_value_;
    command_queue_->Signal(fence_.Get(), fence_value_);

    if (fence_->GetCompletedValue() < fence_value_) {
        fence_->SetEventOnCompletion(fence_value_, fence_event_);
        WaitForSingleObject(fence_event_, INFINITE);
    }
}
