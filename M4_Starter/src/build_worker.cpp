#include "aiws/gui/build_worker.hpp"
#include "aiws/detail/build_hooks.hpp"

#include <algorithm>
#include <exception>
#include <utility>

namespace aiws::gui {

    BuildWorker::BuildWorker(Completion completion)
        : completion_(std::move(completion)), thread_(&BuildWorker::run, this) {}

    BuildWorker::~BuildWorker() { stop(); }

    void BuildWorker::request(Workspace workspace, std::uint64_t workspace_version,
        std::uint64_t build_version, int delay_ms_per_document) {
            {
                std::lock_guard<std::mutex> lock(mutex_);
                if (stopping_) return;
                const auto generation = generation_.fetch_add(1, std::memory_order_acq_rel) + 1;
                // Copy/snapshot is produced by the caller; the worker owns its moved value.
                pending_ = Request{ std::move(workspace), workspace_version, build_version,
                                   generation, std::max(0, delay_ms_per_document) };
            }
            cv_.notify_one();
    }

    void BuildWorker::cancel() {
        {
            std::lock_guard<std::mutex> lock(mutex_);
            generation_.fetch_add(1, std::memory_order_acq_rel);
            pending_.reset();
        }
        cv_.notify_one();
    }

    void BuildWorker::stop() {
        {
            std::lock_guard<std::mutex> lock(mutex_);
            stopping_ = true;
            pending_.reset();
            generation_.fetch_add(1, std::memory_order_acq_rel);
        }
#ifndef AIWS_FAULT_SILENT_SHUTDOWN
        // An idle worker is waiting on cv_; without notification, join hangs.
        cv_.notify_one();
#endif
        if (thread_.joinable()) thread_.join();
    }

    bool BuildWorker::obsolete(std::uint64_t generation) const noexcept {
#ifdef AIWS_FAULT_NO_CANCEL
        (void)generation;
        return false; // fault: obsolete work runs to the end and delays new work
#else
        return generation_.load(std::memory_order_acquire) != generation;
#endif
    }

    void BuildWorker::run() {
        for (;;) {
            Request request;
            {
                std::unique_lock<std::mutex> lock(mutex_);
                cv_.wait(lock, [this] { return stopping_ || pending_.has_value(); });
                if (stopping_) return;
                request = std::move(*pending_);
                pending_.reset();
            }
            // A newer queued request or cancellation can invalidate this snapshot
            // before the worker starts executing it.
            if (obsolete(request.generation)) continue;

            auto result = std::make_shared<WorkerResult>();
            result->workspace_version = request.workspace_version;
            result->build_version = request.build_version;
            const detail::BuildHooks hooks{
                [this, generation = request.generation] { return obsolete(generation); },
                request.delay_ms_per_document
            };
            try {
                detail::ScopedBuildHooks scope(hooks);
                result->core.rebuild(request.workspace);
                if (obsolete(request.generation)) continue;
                result->success = true;
            }
            catch (const detail::BuildCancelled&) {
                continue; // cancellation is not a user-visible build failure
            }
            catch (const std::exception& ex) {
                if (obsolete(request.generation)) continue;
                result->error = ex.what();
            }
            catch (...) {
                if (obsolete(request.generation)) continue;
                result->error = "Unknown error building corpus";
            }
            if (!obsolete(request.generation) && completion_) completion_(std::move(result));
        }
    }

} // namespace aiws::gui
