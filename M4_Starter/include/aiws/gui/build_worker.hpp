#pragma once

#include "aiws/processing_core.hpp"
#include "aiws/workspace.hpp"

#include <atomic>
#include <condition_variable>
#include <cstdint>
#include <functional>
#include <memory>
#include <mutex>
#include <optional>
#include <string>
#include <thread>

namespace aiws::gui {

    // The worker owns the core until a successful result is transferred to the GUI.
    // Neither this object nor the worker thread owns or accesses any Qt widget.
    struct WorkerResult {
        ProcessingCore core;
        std::uint64_t workspace_version{ 0 };
        std::uint64_t build_version{ 0 };
        bool success{ false };
        std::string error;
    };

    class BuildWorker {
    public:
        using Completion = std::function<void(std::shared_ptr<WorkerResult>)>;

        explicit BuildWorker(Completion completion);
        ~BuildWorker();

        BuildWorker(const BuildWorker&) = delete;
        BuildWorker& operator=(const BuildWorker&) = delete;

        // GUI thread: supersedes a queued/running build; never joins/waits for work.
        void request(Workspace workspace, std::uint64_t workspace_version,
            std::uint64_t build_version, int delay_ms_per_document);

        // GUI thread: invalidates the active request and drops any queued request.
        void cancel();

        // Called by the GUI owner at shutdown. Idempotent; joins the sole worker.
        void stop();

    private:
        struct Request {
            Workspace workspace;
            std::uint64_t workspace_version{ 0 };
            std::uint64_t build_version{ 0 };
            std::uint64_t generation{ 0 };
            int delay_ms_per_document{ 0 };
        };

        void run();
        bool obsolete(std::uint64_t generation) const noexcept;

        Completion completion_;
        std::mutex mutex_;
        std::condition_variable cv_;
        std::optional<Request> pending_;  // one-slot, latest-request-wins
        bool stopping_{ false };           // under mutex_
        std::atomic<std::uint64_t> generation_{ 0 };
        std::thread thread_;             // constructed only once
    };

} // namespace aiws::gui
