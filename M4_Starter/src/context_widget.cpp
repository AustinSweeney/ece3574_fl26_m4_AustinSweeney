#include "aiws/gui/context_widget.hpp"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QSlider>
#include <QPushButton>
#include <QPlainTextEdit>

namespace aiws::gui {

ContextWidget::ContextWidget(QWidget* parent) : QWidget(parent) {
    // TODO: Construct and lay out the Context area, configure the token-budget
    // control, connect local events, and establish initial state.

    auto* layout = new QVBoxLayout(this);
    layout->addWidget(new QLabel("Context: "));
    auto* row = new QHBoxLayout;
    row->addWidget(new QLabel("Token budget:"));
    context_budget_slider_= new QSlider(Qt::Horizontal);
    context_budget_slider_->setRange(50, 1000);
    context_budget_slider_->setSingleStep(50);
    context_budget_slider_->setPageStep(50);
    context_budget_slider_->setTickInterval(50);
    row->addWidget(context_budget_slider_, 1);
    budget_value_label_ = new QLabel;
    row->addWidget(budget_value_label_);
    build_context_button_ = new QPushButton("Build Context");
    row->addWidget(build_context_button_);
    layout->addLayout(row);
    context_text_ = new QPlainTextEdit;
    context_text_->setReadOnly(true);
    layout->addWidget(context_text_);
    connect(context_budget_slider_, &QSlider::valueChanged, this, &ContextWidget::budget_changed);
    connect(build_context_button_, &QPushButton::clicked, this, &ContextWidget::request_context);
    reset();
}

int ContextWidget::token_budget() const {
    // TODO
    return context_budget_slider_->value();
}

void ContextWidget::set_build_context_enabled(bool enabled) {
    // TODO
    build_context_button_->setEnabled(enabled);  // To suppress unused-parameter warning.
}

void ContextWidget::set_context(const std::vector<ContextItem>& items) {
    // TODO
    QString text;
    for (const auto& item : items)
    {
        text += QString("Document: %1\nChunk: %2\nTokens: %3\nScore: %4\nTruncated: %5\nText: %6\n\n").arg(QString::fromStdString(item.document_id)).arg(item.chunk_sequence).arg(item.token_count).arg(item.score).arg(item.truncated ? "Yes" : "No").arg(QString::fromStdString(item.text));// To suppress unused-parameter warning.
    }
    context_text_->setPlainText(text);
}

void ContextWidget::clear_context() {
    // TODO
    context_text_->clear();
}

void ContextWidget::reset() {
    // TODO
    context_budget_slider_->setValue(300);
    budget_changed(300);
    clear_context();
    set_build_context_enabled(false);
}

void ContextWidget::request_context() {
    // TODO
    if (build_context_button_->isEnabled())
    {
        emit build_context_requested(token_budget());
    }
}

void ContextWidget::budget_changed(int value) {
    // TODO
    budget_value_label_->setText(QString::number(value));  // To suppress unused-parameter warning.
}

}  // namespace aiws::gui
