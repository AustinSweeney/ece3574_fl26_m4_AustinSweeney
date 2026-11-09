#include "aiws/gui/term_inspector_widget.hpp"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>

namespace aiws::gui {

TermInspectorWidget::TermInspectorWidget(QWidget* parent) : QWidget(parent) {
    // TODO: Construct and lay out the Term Inspector, connect local events,
    // and establish initial state.
    auto* layout = new QVBoxLayout(this);
    layout->addWidget(new QLabel("Term Inspector:"));
    auto* row = new QHBoxLayout;
    row->addWidget(new QLabel("Term:"));
    term_edit_ = new QLineEdit;
    term_edit_->setPlaceholderText("Enter a term");
    row->addWidget(term_edit_);
    inspect_button_ = new QPushButton("Inspect");
    row->addWidget(inspect_button_);
    layout->addLayout(row);
    auto* stats = new QHBoxLayout;
    stats->addWidget(new QLabel("Document Frequency:"));
    document_frequency_value_ = new QLabel("—");
    stats->addWidget(document_frequency_value_);
    stats->addWidget(new QLabel("Selected Chunk TF:"));
    chunk_frequency_value_ = new QLabel("—");
    stats->addWidget(chunk_frequency_value_);
    layout->addLayout(stats);
    layout->addStretch();
    connect(inspect_button_, &QPushButton::clicked, this, &TermInspectorWidget::request_inspection);
    reset();
}

void TermInspectorWidget::set_inspection_enabled(bool enabled) {
    // TODO
    inspect_button_->setEnabled(enabled);
}

void TermInspectorWidget::set_selected_chunk(const QString& chunk_id) {
    // TODO
    selected_chunk_id_ = chunk_id;
    clear_statistics();// To suppress unused-parameter warning.
}

void TermInspectorWidget::set_statistics(std::size_t document_frequency,
                                         bool has_chunk_frequency,
                                         std::size_t chunk_frequency) {
    // TODO
    document_frequency_value_->setText(QString::number(document_frequency));
    chunk_frequency_value_->setText(has_chunk_frequency ? QString::number(chunk_frequency) : QString("—"));// To suppress unused-parameter warning.
}

void TermInspectorWidget::clear_statistics() {
    // TODO
    document_frequency_value_->setText("—");
    chunk_frequency_value_->setText("—");
}

void TermInspectorWidget::reset() {
    // TODO
    term_edit_->clear();
    selected_chunk_id_.clear();
    clear_statistics();
    set_inspection_enabled(false);
}

void TermInspectorWidget::request_inspection() {
    // TODO
    if (inspect_button_->isEnabled() && !term_edit_->text().trimmed().isEmpty())emit term_inspection_requested(term_edit_->text(), selected_chunk_id_);
}

}  // namespace aiws::gui
