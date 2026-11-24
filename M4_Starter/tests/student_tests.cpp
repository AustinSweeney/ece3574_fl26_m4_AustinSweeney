#include "aiws/gui/main_window.hpp"
#include <QApplication>
#include <QElapsedTimer>
#include <QFile>
#include <QLabel>
#include <QMetaObject>
#include <QTemporaryDir>
#include <QTest>

using aiws::gui::MainWindow;
class StudentTests : public QObject {
    Q_OBJECT
private:
    static QString make_file(const QString& dir, const QString& name) {
        const QString path = dir + "/" + name;
        QFile file(path);
        if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) return {};
        file.write("sample words for the corpus\n");
        return path;
    }
    static bool built(MainWindow& window) {
        for (auto* label : window.findChildren<QLabel*>()) {
            if (label->text().contains("Corpus Built")) return true;
        }
        return false;
    }
private slots:
    void workerBuildAndReset() {
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        MainWindow window;
        window.set_build_delay_ms(220);
        for (int i = 0; i < 8; ++i) {
            QVERIFY(window.add_document_file(make_file(dir.path(), QString::number(i) + ".txt")));
        }
        QVERIFY(QMetaObject::invokeMethod(&window, "build_corpus", Qt::DirectConnection));
        QTest::qWait(250); // first document has begun
        QVERIFY(QMetaObject::invokeMethod(&window, "reset_application", Qt::DirectConnection));
        QVERIFY(window.add_document_file(make_file(dir.path(), "new.txt")));
        QElapsedTimer elapsed;
        elapsed.start();
        QVERIFY(QMetaObject::invokeMethod(&window, "build_corpus", Qt::DirectConnection));
        QTRY_VERIFY_WITH_TIMEOUT(built(window), 1350);
        QVERIFY2(elapsed.elapsed() < 1350, "obsolete build blocked the new request");
    }
    void idleShutdown() {
        MainWindow window;
        QTest::qWait(150); // let worker reach its condition_variable wait
        QElapsedTimer elapsed;
        elapsed.start();
        window.request_shutdown();
        QVERIFY(elapsed.elapsed() < 1000);
    }
};
QTEST_MAIN(StudentTests)
#include "student_tests.moc"
