#include "aiws/gui/workspace_widget.hpp"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QListWidget>
#include <QPlainTextEdit>
#include <QPushButton>
namespace aiws::gui {

WorkspaceWidget::WorkspaceWidget(QWidget* parent) : QWidget(parent) {
    // TODO: Construct and lay out the Workspace and Selected Document areas,
    // connect local widget events, and establish initial presentation state.
    auto* outer = new QVBoxLayout(this);
    auto* row = new QHBoxLayout;
    auto* left = new QVBoxLayout;
    auto* controls = new QHBoxLayout;
    controls->addWidget(new QLabel("Workspace"));
    add_document_button_ = new QPushButton("Add Document");
    build_corpus_button_ = new QPushButton("Build Corpus");
    controls->addWidget(add_document_button_);
    controls->addWidget(build_corpus_button_);
    left->addLayout(controls);
    document_list_ = new QListWidget;
    left->addWidget(document_list_);
    corpus_status_label_ = new QLabel("Corpus: Not built");
    left->addWidget(corpus_status_label_);
    auto* right = new QVBoxLayout;
    document_title_label_ = new QLabel("Selected Document -");
    document_text_ = new QPlainTextEdit;
    document_text_->setReadOnly(true);
    right->addWidget(document_title_label_);
    right->addWidget(document_text_);
    row->addLayout(left, 1);
    row->addLayout(right, 1);
    outer->addLayout(row);
    connect(add_document_button_, &QPushButton::clicked, this, &WorkspaceWidget::add_document_requested);
    connect(build_corpus_button_, &QPushButton::clicked, this, &WorkspaceWidget::build_corpus_requested);
    connect(document_list_, &QListWidget::currentRowChanged, this, &WorkspaceWidget::document_selected);
    reset();
}

void WorkspaceWidget::add_document(const QString& title) {
    // TODO
    document_list_->addItem(title);
    document_list_->setCurrentRow(document_list_->count() - 1);
    // To suppress unused-parameter warning.
}

void WorkspaceWidget::select_document(int row) {
    // TODO
    if (row >= 0 && row < document_list_->count())
    {
        document_list_->setCurrentRow(row);
    }
}

void WorkspaceWidget::show_selected_document(const QString& title,
                                              const QString& text) {
    // TODO
    document_title_label_->setText("Selected Document - " + title);
    document_text_->setPlainText(text);
}

void WorkspaceWidget::clear_selected_document() {
    // TODO
    document_title_label_->setText("Selected Document -");
    document_text_->clear();
}

void WorkspaceWidget::set_corpus_status(const QString& text) {
    // TODO
    corpus_status_label_->setText("Corpus: " + text);
}

void WorkspaceWidget::set_build_corpus_enabled(bool enabled) {
    // TODO
    build_corpus_button_->setEnabled(enabled);
}

void WorkspaceWidget::reset() {
    // TODO
    document_list_->clear();
    clear_selected_document();
    set_corpus_status("Not built");
    set_build_corpus_enabled(false);
}

}  // namespace aiws::gui
