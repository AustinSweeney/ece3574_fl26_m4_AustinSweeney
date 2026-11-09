# M4 Decision Record

See specification Section 8.



Architecture and Threading



The application uses separate threads for the GUI, corpus building, and IPC server. The GUI thread handles user interactions and updates the interface, while a std::thread performs corpus building in the background. This prevents the GUI from freezing during processing.



When a build begins, the program copies the workspace and records its version. A QTimer checks whether the worker has finished. Once complete, the GUI thread retrieves the result and updates the corpus.



Corpus States and Thread Safety



The corpus has four states: Not Built, Building, Current, and Failed. Adding a document or resetting the workspace invalidates the corpus. Each workspace modification increments workspace\_version\_.



If the workspace changes during a build, the completed result is rejected because its version no longer matches. The user must rebuild the corpus.



The worker processes a separate workspace copy and stores its result independently. An atomic completion flag and thread joining ensure the GUI only accesses the completed result safely.



IPC Implementation



The application uses a Unix-domain socket to communicate with the aiws\_send command-line client. The IPC server runs on a separate thread and uses poll() for communication.



The client sends a four-byte file path length followed by the path. The server checks whether the file exists and is readable, then returns a status byte and an optional error message.



Valid file paths are forwarded to the GUI through QMetaObject::invokeMethod() with Qt::QueuedConnection, ensuring GUI modifications occur on the correct thread.



Shutdown



When the user requests an exit, the application stops the IPC server and prevents new operations. If a corpus build is running, shutdown waits for the worker to finish before closing. The worker thread is joined, and the Unix socket file is removed during cleanup.

Partner: Daniel Lomus

Session 1: 11/8/26, 18:30–19:34, on Daniel's code, worked on CR-1.

Roles: Daniel drove first.

Contribution Daniel made as navigator: Walked through understanding of Linux socket, and work systems.

Contribution Austin made: Assist with debuging my finished implementation of CR-1.

Interaction that worked best for us: Overview of the code with explanation.



Partner: Daniel Lomus

Session 2: 11/8/26, 19:34–20:25, on Austin's code, worked on CR-1.

Roles: Austin drove next.

Contribution Austin made as navigator: Explanation of try and catch functions, exception handling within the IPC server.

Contribution my partner made: Noted area's of improvements to user's IPC Server section of code, specifically the design of the threaded socket connection handling.

Interaction that worked best for us: Overview of the code with explanation.

