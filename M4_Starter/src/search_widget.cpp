#include "aiws/gui/search_widget.hpp"
#include "aiws/gui/search_result_model.hpp"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QSpinBox>
#include <QPushButton>
#include <QTableView>
#include <QPlainTextEdit>
#include <QHeaderView>
#include <QItemSelectionModel>
#include <QAbstractItemView>
#include <utility>

namespace aiws::gui {

SearchWidget::SearchWidget(QWidget* parent) : QWidget(parent) {
    // TODO: Construct and lay out the Query, Search Results, and Selected Result
    // areas; configure the table/model; connect local events; establish initial state.
    auto* layout = new QVBoxLayout(this);
    auto* row = new QHBoxLayout;
    row->addWidget(new QLabel("Query:"));
    query_edit_ = new QLineEdit;
    query_edit_->setPlaceholderText("Search the current corpus");
    row->addWidget(query_edit_, 1);
    row->addWidget(new QLabel("Results:"));
    result_count_spin_ = new QSpinBox;
    result_count_spin_->setRange(1, 100);
    result_count_spin_->setValue(3);
    row->addWidget(result_count_spin_);
    search_button_ = new QPushButton("Search");
    row->addWidget(search_button_);
    layout->addLayout(row);
    result_model_ = new SearchResultModel(this);
    results_view_ = new QTableView;
    results_view_->setModel(result_model_);
    results_view_->setSelectionBehavior(QAbstractItemView::SelectRows);
    results_view_->setSelectionMode(QAbstractItemView::SingleSelection);
    results_view_->setEditTriggers(QAbstractItemView::NoEditTriggers);
    results_view_->horizontalHeader()->setStretchLastSection(true);
    layout->addWidget(results_view_, 1);
    auto* detail = new QHBoxLayout;
    detail->addWidget(new QLabel("Selected Result:"));
    view_source_button_ = new QPushButton("View Source");
    detail->addStretch();
    detail->addWidget(view_source_button_);
    layout->addLayout(detail);
    selected_result_text_ = new QPlainTextEdit;
    selected_result_text_->setReadOnly(true);
    layout->addWidget(selected_result_text_);
    connect(search_button_, &QPushButton::clicked, this, &SearchWidget::request_search);
    connect(query_edit_, &QLineEdit::textChanged, this, &SearchWidget::query_changed);
    connect(results_view_->selectionModel(), &QItemSelectionModel::selectionChanged, this, &SearchWidget::show_selected_result);
    connect(view_source_button_, &QPushButton::clicked, this, &SearchWidget::request_source_document);
    reset();
}

QString SearchWidget::query() const {
    // TODO
    return query_edit_->text();
}

int SearchWidget::result_count() const {
    // TODO
    return result_count_spin_->value();
}

bool SearchWidget::has_query() const {
    // TODO
    return !query().trimmed().isEmpty();
}

void SearchWidget::set_search_enabled(bool enabled) {
    // TODO
    search_button_->setEnabled(enabled);
}

void SearchWidget::set_results(std::vector<SearchResult> results) {
    // TODO
    result_model_->set_results(std::move(results));
    results_view_->clearSelection();
    selected_result_text_->clear();
    view_source_button_->setEnabled(false);
    emit selected_chunk_changed({});
}

void SearchWidget::clear_results() {
    result_model_->clear();
    results_view_->clearSelection();
    selected_result_text_->clear();
    view_source_button_->setEnabled(false);
    emit selected_chunk_changed({});
}

void SearchWidget::reset() {
    query_edit_->clear();
    result_count_spin_->setValue(3);
    clear_results();
    set_search_enabled(false);
}

void SearchWidget::request_search() {
    if (search_button_->isEnabled() && has_query())
    {
        emit search_requested(query(), result_count());
    }
}

void SearchWidget::show_selected_result() {
    const auto rows = results_view_->selectionModel()->selectedRows();
    const auto* r = rows.isEmpty() ? nullptr : result_model_->result_at(rows.first().row());
    if (!r) {
        selected_result_text_->clear();
        view_source_button_->setEnabled(false);
        emit selected_chunk_changed({});
        return;
    }
    selected_result_text_->setPlainText(QString("Document: %1\nChunk: %2\nScore: %3\nText: %4").arg(QString::fromStdString(r->document_id)).arg(r->chunk_sequence).arg(r->score).arg(QString::fromStdString(r->text)));
    view_source_button_->setEnabled(true);
    emit selected_chunk_changed(QString::fromStdString(r->chunk_id));
}

void SearchWidget::request_source_document() {
    // TODO
    const auto rows = results_view_->selectionModel()->selectedRows();
    const auto* r = rows.isEmpty() ? nullptr : result_model_->result_at(rows.first().row());
    if (r)
    {
        emit source_document_requested(QString::fromStdString(r->document_id));
    }
}

}  // namespace aiws::gui
