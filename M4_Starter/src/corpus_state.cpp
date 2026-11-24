#include "aiws/gui/corpus_state.hpp"
namespace aiws::gui {
	std::unique_ptr<CorpusState> NotBuiltState::build() const { return std::make_unique<BuildingState>(); }
	std::unique_ptr<CorpusState> NotBuiltState::invalidate() const { return std::make_unique<NotBuiltState>(); }
	std::unique_ptr<CorpusState> NotBuiltState::succeeded() const { return std::make_unique<NotBuiltState>(); }
	std::unique_ptr<CorpusState> NotBuiltState::failed() const { return std::make_unique<NotBuiltState>(); }
	std::unique_ptr<CorpusState> BuildingState::build() const { return std::make_unique<BuildingState>(); }
	std::unique_ptr<CorpusState> BuildingState::invalidate() const { return std::make_unique<NotBuiltState>(); }
	std::unique_ptr<CorpusState> BuildingState::succeeded() const { return std::make_unique<CurrentState>(); }
	std::unique_ptr<CorpusState> BuildingState::failed() const { return std::make_unique<FailedState>(); }
	std::unique_ptr<CorpusState> CurrentState::build() const { return std::make_unique<BuildingState>(); }
	std::unique_ptr<CorpusState> CurrentState::invalidate() const { return std::make_unique<NotBuiltState>(); }
	std::unique_ptr<CorpusState> CurrentState::succeeded() const { return std::make_unique<CurrentState>(); }
	std::unique_ptr<CorpusState> CurrentState::failed() const { return std::make_unique<FailedState>(); }
	std::unique_ptr<CorpusState> FailedState::build() const { return std::make_unique<BuildingState>(); }
	std::unique_ptr<CorpusState> FailedState::invalidate() const { return std::make_unique<NotBuiltState>(); }
	std::unique_ptr<CorpusState> FailedState::succeeded() const { return std::make_unique<FailedState>(); }
	std::unique_ptr<CorpusState> FailedState::failed() const { return std::make_unique<FailedState>(); }
} // namespace aiws::gui
