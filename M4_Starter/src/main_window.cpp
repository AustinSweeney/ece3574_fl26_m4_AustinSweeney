#include "aiws/gui/main_window.hpp"
#include "aiws/gui/workspace_widget.hpp"
#include "aiws/gui/search_widget.hpp"
#include "aiws/gui/term_inspector_widget.hpp"
#include "aiws/gui/context_widget.hpp"
#include "aiws/gui/resettable.hpp"
#include "aiws/ipc_server.hpp"
#include <QApplication>
#include <QCloseEvent>
#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QFileDialog>
#include <QFileInfo>
#include <QFile>
#include <QString>
#include <QByteArray>
#include <QTimer>
#include <QMetaObject>
#include <QPointer>
#include <QString>
#include <chrono>
#include <utility>
#include <exception>
#include <algorithm>

namespace aiws::gui {

    MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent) {
        // TODO: Construct the M3 application window, compose the GUI components,
        // establish layouts/presentation, connect events, and establish initial state.
        setWindowTitle("AI Workspace");
        resize(900, 900);
        auto* central = new QWidget;
        setCentralWidget(central);
        auto* layout = new QVBoxLayout(central);
        auto* header = new QLabel("AI Workspace | Document Search and Context Explorer");
        header->setStyleSheet("background:#722332;color:white;padding:14px;font-size:20px;font-weight:bold;");
        layout->addWidget(header);
        workspace_widget_ = new WorkspaceWidget;
        search_widget_ = new SearchWidget;
        term_inspector_widget_ = new TermInspectorWidget;
        context_widget_ = new ContextWidget;
        layout->addWidget(workspace_widget_, 3);
        layout->addWidget(search_widget_, 4);
        auto* bottom = new QHBoxLayout;
        bottom->addWidget(term_inspector_widget_, 1);
        bottom->addWidget(context_widget_, 1);
        layout->addLayout(bottom, 2);
        auto* footer = new QHBoxLayout;
        status_label_ = new QLabel("Current Status: Ready");
        footer->addWidget(status_label_, 1);
        reset_button_ = new QPushButton("Reset");
        reset_button_->setStyleSheet("background:#e4b04d;color:#202020;padding:6px;");
        exit_button_ = new QPushButton("Exit");
        exit_button_->setStyleSheet("background:#3267a3;color:white;padding:6px;");
        footer->addWidget(reset_button_);
        footer->addWidget(exit_button_);
        layout->addLayout(footer);
        connect(workspace_widget_, &WorkspaceWidget::add_document_requested, this, &MainWindow::add_document);
        connect(workspace_widget_, &WorkspaceWidget::build_corpus_requested, this, &MainWindow::build_corpus);
        connect(workspace_widget_, &WorkspaceWidget::document_selected, this, &MainWindow::document_selected);
        connect(search_widget_, &SearchWidget::search_requested, this, &MainWindow::search_requested);
        connect(search_widget_, &SearchWidget::query_changed, this, &MainWindow::query_changed);
        connect(search_widget_, &SearchWidget::source_document_requested, this, &MainWindow::source_document_requested);
        connect(search_widget_, &SearchWidget::selected_chunk_changed, term_inspector_widget_, &TermInspectorWidget::set_selected_chunk);
        connect(term_inspector_widget_, &TermInspectorWidget::term_inspection_requested, this, &MainWindow::term_inspection_requested);
        connect(context_widget_, &ContextWidget::build_context_requested, this, &MainWindow::build_context_requested);
        connect(reset_button_, &QPushButton::clicked, this, &MainWindow::reset_application);
        connect(exit_button_, &QPushButton::clicked, this, &MainWindow::exit_application);

        QPointer<MainWindow> safe_window(this);
        build_worker_ = std::make_unique<BuildWorker>([safe_window](std::shared_ptr<WorkerResult> result) {
            if (!safe_window) return;
            QMetaObject::invokeMethod(safe_window.data(), [safe_window, result = std::move(result)]() mutable {
                if (safe_window) safe_window->complete_build_(std::move(result));
                }, Qt::QueuedConnection);
            });

        update_action_states();
    }

    MainWindow::~MainWindow()
    {
        ipc_server_.stop();

        if (build_worker_) build_worker_->stop();
    }

    void MainWindow::set_exit_callback(ExitCallback callback) noexcept {
        // TODO
        exit_callback_ = callback;  // To suppress unused-parameter warning.
    }

    void MainWindow::add_document() {
        // TODO
        if (shutdown_pending_)
        {
            return;
        }
        const QString path = QFileDialog::getOpenFileName(this, "Add Document", QString(), "Text files (*.txt);; ALL files (*)");
        if (path.isEmpty())
        {
            return;
        }
        if (!add_document_file(path))
        {
            status_label_->setText("Current Status: Unable to Open Document.");
        }
    }

    void MainWindow::build_corpus() {
        if (workspace_.documents().empty() || shutdown_pending_) return;
        ++build_version_;
        corpus_state_ = corpus_state_->build();
        clear_processing_outputs();
        workspace_widget_->set_corpus_status("Building");
        status_label_->setText("Current Status: Building Corpus");
        update_action_states();
        build_worker_->request(workspace_, workspace_version_, build_version_, build_delay_ms_);
    }

    void MainWindow::complete_build_(std::shared_ptr<WorkerResult> result) {
        // Versions reject both obsolete workspace snapshots and superseded builds.
        // State objects alone decide how a valid completion transitions the corpus.
        if (shutdown_pending_ || result->workspace_version != workspace_version_ ||
            result->build_version != build_version_) return;
        if (result->success) {
            processing_core_ = std::move(result->core);
            corpus_state_ = corpus_state_->succeeded();
            workspace_widget_->set_corpus_status(QString::number(processing_core_.chunk_count()) + " Chunks");
            status_label_->setText("Current Status: Corpus Built");
        }
        else {
            corpus_state_ = corpus_state_->failed();
            workspace_widget_->set_corpus_status("Failed");
            status_label_->setText("Corpus Status: Build failed: " + QString::fromStdString(result->error));
        }
        update_action_states();
    }

    void MainWindow::document_selected(int row) {
        // TODO
        const auto& docs = workspace_.documents();
        if (row < 0 || static_cast<std::size_t>(row) >= docs.size())
        {
            workspace_widget_->clear_selected_document();
            return;
        }
        const auto& doc = docs[row];
        workspace_widget_->show_selected_document(QString::fromStdString(doc.title()), QString::fromStdString(doc.text()));

    }

    void MainWindow::search_requested(const QString& query, int k) {
        // TODO
        if (!corpus_state_->usable() || query.trimmed().isEmpty())
        {
            return;
        }
        try
        {
            search_widget_->set_results(processing_core_.search(query.toStdString(), k));
            context_widget_->clear_context();
            term_inspector_widget_->clear_statistics();
            status_label_->setText("Current status: Search Complete");

        }
        catch (const std::exception& e)
        {
            status_label_->setText("Current status: " + QString::fromUtf8(e.what()));
        }
    }

    void MainWindow::query_changed(const QString& query) {
        (void)query;
        update_action_states();
    }

    void MainWindow::build_context_requested(int token_budget) {
        // TODO
        if (!corpus_state_->usable() || !search_widget_->has_query())
        {
            return;
        }
        try
        {
            context_widget_->set_context(processing_core_.build_context(search_widget_->query().toStdString(), search_widget_->result_count(), static_cast<std::size_t>(token_budget)));
            status_label_->setText("Current status: Context Built");
        }
        catch (const std::exception& e)
        {
            status_label_->setText("Current Status: " + QString::fromUtf8(e.what()));
        }
    }

    void MainWindow::source_document_requested(const QString& document_id) {
        // TODO
        const auto& docs = workspace_.documents();
        for (std::size_t i = 0; i < docs.size(); ++i)
        {
            if (QString::fromStdString(docs[i].id()) == document_id)
            {
                workspace_widget_->select_document(static_cast<int>(i));
                document_selected(static_cast<int>(i));
                return;
            }
        }
    }

    void MainWindow::term_inspection_requested(const QString& term,
        const QString& chunk_id) {
        // TODO
        if (!corpus_state_->usable() || term.trimmed().isEmpty())
        {
            return;
        }
        try
        {
            const auto df = processing_core_.document_frequency(term.toStdString());
            const bool has = !chunk_id.isEmpty();
            const auto tf = has ? processing_core_.term_frequency(term.toStdString(), chunk_id.toStdString()) : 0;
            term_inspector_widget_->set_statistics(df, has, tf);
            status_label_->setText("Current status: Term inspected");

        }
        catch (const std::exception& e)
        {
            status_label_->setText("Current_status: " + QString::fromUtf8(e.what()));
        }
    }

    void MainWindow::reset_application() {
        // TODO
        ++workspace_version_;
        build_worker_->cancel();
        workspace_.clear();
        corpus_state_ = corpus_state_->invalidate();
        workspace_widget_->set_corpus_status("Not built");
        Resettable* components[] = { workspace_widget_,search_widget_,term_inspector_widget_,context_widget_ };
        for (auto* component : components)
        {
            component->reset();


        }
        update_action_states();
        workspace_widget_->set_corpus_status("Not built");
        status_label_->setText("Current Status: Application reset");
    }

    void MainWindow::exit_application() {
        if (exit_callback_)
        {
            exit_callback_(this);
        }
    }

    void MainWindow::update_action_states() {
        // TODO
        workspace_widget_->set_build_corpus_enabled(!workspace_.documents().empty() && !shutdown_pending_);
        const bool query_ok = corpus_state_->usable() && search_widget_->has_query() && !shutdown_pending_;
        search_widget_->set_search_enabled(query_ok);
        context_widget_->set_build_context_enabled(query_ok);
        term_inspector_widget_->set_inspection_enabled(corpus_state_->usable() && !shutdown_pending_);

        reset_button_->setEnabled(!shutdown_pending_);
        exit_button_->setEnabled(!shutdown_pending_);

    }

    void MainWindow::invalidate_corpus() {
        // TODO
        build_worker_->cancel();
        corpus_state_ = corpus_state_->invalidate();
        workspace_widget_->set_corpus_status("Not built");
        clear_processing_outputs();
        update_action_states();
    }

    void MainWindow::clear_processing_outputs() {
        // TODO
        search_widget_->clear_results();
        term_inspector_widget_->clear_statistics();
        context_widget_->clear_context();
    }

    bool MainWindow::add_document_file(const QString& path)
    {
        if (shutdown_pending_)
        {
            return false;
        }
        QFile file(path);

        if (!file.open(QIODevice::ReadOnly))
        {
            return false;
        }

        const QString title = QFileInfo(path).fileName();
        const QString absolute = QFileInfo(path).absoluteFilePath();

        const QString content = QString::fromUtf8(file.readAll());

        workspace_.add_document(Document(absolute.toStdString(), title.toStdString(), content.toStdString()));

        ++workspace_version_;
        invalidate_corpus();

        workspace_widget_->add_document(title);
        update_action_states();

        status_label_->setText("Current Status: Document added");

        return true;
    }
    void MainWindow::set_build_delay_ms(int ms_per_document)
    {
        build_delay_ms_ = std::max(0, ms_per_document);
    }

    void MainWindow::request_shutdown() {
        if (shutdown_pending_) return;
        shutdown_pending_ = true;
        ipc_server_.stop();
        update_action_states();
        finish_shutdown_();
    }

    void MainWindow::finish_shutdown_()
    {
        if (build_worker_)
        {
            build_worker_->stop();
        }

        QApplication::quit();
        close();
    }
    void MainWindow::closeEvent(QCloseEvent* event)
    {
        if (shutdown_pending_)
        {
            event->accept();
            return;
        }

        event->ignore();
        exit_application();
    }
    bool MainWindow::start_ipc_server(const QString& path)
    {
        QPointer<MainWindow> window(this);

        return ipc_server_.start(
            path.toStdString(),
            [window](const std::string& file_path)
            {
                if (!window)
                {
                    return;
                }

                QMetaObject::invokeMethod(
                    window.data(),
                    [window, file_path]()
                    {
                        if (!window)
                        {
                            return;
                        }

                        window->add_document_file(
                            QString::fromStdString(file_path)
                        );
                    },
                    Qt::QueuedConnection
                );
            }
        );
    }
}  // namespace aiws::gui
