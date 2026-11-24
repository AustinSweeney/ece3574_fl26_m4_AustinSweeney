#pragma once

#include <exception>
#include <functional>

namespace aiws::detail {

    // Internal, thread-local hook: leaves the public M1/M2 ProcessingCore API intact.
    // It is set only on the build worker thread while ProcessingCore::rebuild runs.
    struct BuildHooks {
        std::function<bool()> cancelled;
        int delay_per_document_ms{ 0 };
    };

    extern thread_local const BuildHooks* active_build_hooks;

    class BuildCancelled final : public std::exception {
    public:
        const char* what() const noexcept override { return "build cancelled"; }
    };

    class ScopedBuildHooks final {
    public:
        explicit ScopedBuildHooks(const BuildHooks& hooks) noexcept
            : previous_(active_build_hooks) {
            active_build_hooks = &hooks;
        }
        ~ScopedBuildHooks() { active_build_hooks = previous_; }
        ScopedBuildHooks(const ScopedBuildHooks&) = delete;
        ScopedBuildHooks& operator=(const ScopedBuildHooks&) = delete;
    private:
        const BuildHooks* previous_;
    };

} // namespace aiws::detail
