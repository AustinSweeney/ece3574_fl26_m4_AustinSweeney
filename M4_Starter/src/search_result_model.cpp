#include "aiws/gui/search_result_model.hpp"
#include <QString>
#include <utility>
namespace aiws::gui {

SearchResultModel::SearchResultModel(QObject* parent)
    : QAbstractTableModel(parent) {
    // TODO
    
}

int SearchResultModel::rowCount(const QModelIndex& parent) const {
    // TODO
    return parent.isValid() ? 0 : static_cast<int>(results_.size());
}

int SearchResultModel::columnCount(const QModelIndex& parent) const {
    // TODO
    return parent.isValid() ? 0 : 5;
}

QVariant SearchResultModel::data(const QModelIndex& index, int role) const {
    // TODO
    if (role != Qt::DisplayRole || !index.isValid() || index.row() < 0 || index.row() >= rowCount() || index.column() < 0 || index.column() >= 5)
    {
        return {};
    }
    
    const auto& r = results_[index.row()];
    switch (index.column()) {
    case 0:
        return index.row() + 1;
    case 1:
        return QString::fromStdString(r.document_id);
    case 2:
        return static_cast<qulonglong>(r.chunk_sequence);
    case 3:
        return r.score;
    case 4:
        return QString::fromStdString(r.text);
    default:
        return  {};
    }
}

QVariant SearchResultModel::headerData(int section,
                                       Qt::Orientation orientation,
                                       int role) const {
    // TODO
    if (role != Qt::DisplayRole)
    {
        return {};
    }
    if (orientation == Qt::Vertical)
    {
        return section + 1;
    }
    static const char* labels[] = {
        "Rank","Document","Chunk","Score","Text"
    };
    return section >= 0 && section < 5 
        ? QVariant(QString::fromUtf8(labels[section]))
        : QVariant{};
}

void SearchResultModel::set_results(std::vector<SearchResult> results) {
    // TODO
    beginResetModel();
    results_ = std::move(results);
    endResetModel();// To suppress unused-parameter warning.
}

void SearchResultModel::clear() {
    // TODO
    set_results({});
}

const SearchResult* SearchResultModel::result_at(int row) const noexcept {
    // TODO
    return row >= 0 && static_cast<std::size_t>(row) < results_.size() ? &results_[row] : nullptr;
}

}  // namespace aiws::gui
