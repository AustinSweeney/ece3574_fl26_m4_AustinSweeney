M5 Design Record
Architecture and Threading
The application uses separate threads for the GUI, corpus building, and IPC server. The GUI thread handles user input and updates the widgets. M5 replaces the M4 thread created for each build with one BuildWorker thread that starts when MainWindow is constructed and stops when the application exits.

When Build Corpus is clicked, MainWindow copies the workspace and sends it to the worker with workspace_version_, build_version_, and build_delay_ms_. The worker uses std::mutex and std::condition_variable to wait for requests without continuously polling. It keeps only the newest pending request.
Cancellation and Shared Data
BuildWorker uses an atomic generation_ value to identify outdated builds. A new request, Reset, or document addition invalidates the previous generation. build_hooks.hpp provides cancellation checks used by processing_core.cpp between documents. This allows an outdated build to stop without finishing the entire corpus.

The worker builds its own workspace copy and returns a WorkerResult. The result is passed to the GUI using QMetaObject::invokeMethod() with Qt::QueuedConnection. The GUI checks both version numbers before accepting it, so an old build cannot replace a newer corpus. Widgets are only modified on the GUI thread.
Corpus States
M5 uses the State pattern through CorpusState and four classes: NotBuiltState, BuildingState, CurrentState, and FailedState. Each state handles build(), invalidate(), succeeded(), and failed() and returns the next state. MainWindow calls these methods instead of using a chain of conditions to choose transitions. Search, context building, and term inspection require a usable corpus.

For example, if a document is added during a build, the workspace version increases, the worker is canceled, and the state returns to Not Built. A later completion from the old build is rejected.
Shutdown and IPC
Exit stops the IPC server, prevents new GUI operations, cancels the current build, and joins the worker thread. The worker is notified through the condition variable so shutdown also works when no build is running.

The M4 Unix-domain socket implementation remains in place. Remote document requests are forwarded to the GUI thread before the workspace is changed.
Fault Switches and Testing
AIWS_FAULT_NO_CANCEL is located in build_worker.cpp and disables normal generation-based cancellation. AIWS_FAULT_SILENT_SHUTDOWN is also in build_worker.cpp and skips the worker notification during shutdown. These switches are intended to make the corresponding cancellation and idle-shutdown tests fail.

Testing should include normal builds, replacing a running build, Reset during a build, remote document additions, and Exit while the worker is idle or active.

AI Assistance
I used AI to redo my CMAKE text file, verify the design of my state pattern would work, debugging and assistance in deisgning the system. Details logged in the AI_LOG.md file.