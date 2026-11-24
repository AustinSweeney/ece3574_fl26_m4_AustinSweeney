#pragma once
#include <memory>
namespace aiws::gui {
    class CorpusState {
    public:
        virtual ~CorpusState() = default;
        virtual std::unique_ptr<CorpusState> build() const = 0;
        virtual std::unique_ptr<CorpusState> invalidate() const = 0;
        virtual std::unique_ptr<CorpusState> succeeded() const = 0;
        virtual std::unique_ptr<CorpusState> failed() const = 0;
        virtual bool usable() const { return false; }
        virtual bool building() const { return false; }
    };
    class NotBuiltState final : public CorpusState {
    public:
        std::unique_ptr<CorpusState> build() const override;
        std::unique_ptr<CorpusState> invalidate() const override;
        std::unique_ptr<CorpusState> succeeded() const override;
        std::unique_ptr<CorpusState> failed() const override;
    };
    class BuildingState final : public CorpusState {
    public:
        std::unique_ptr<CorpusState> build() const override;
        std::unique_ptr<CorpusState> invalidate() const override;
        std::unique_ptr<CorpusState> succeeded() const override;
        std::unique_ptr<CorpusState> failed() const override;
        bool building() const override { return true; }
    };
    class CurrentState final : public CorpusState {
    public:
        std::unique_ptr<CorpusState> build() const override;
        std::unique_ptr<CorpusState> invalidate() const override;
        std::unique_ptr<CorpusState> succeeded() const override;
        std::unique_ptr<CorpusState> failed() const override;
        bool usable() const override { return true; }
    };
    class FailedState final : public CorpusState {
    public:
        std::unique_ptr<CorpusState> build() const override;
        std::unique_ptr<CorpusState> invalidate() const override;
        std::unique_ptr<CorpusState> succeeded() const override;
        std::unique_ptr<CorpusState> failed() const override;
    };
} // namespace aiws::gui
