#include "mainwindow.h"

#include <QApplication>

class HideFromTaskbar : public QWidget
{
public:
    HideFromTaskbar(QWidget *parent)
        : QWidget(parent, Qt::Dialog)
    {

    }
};

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;

    QLockFile lockFile(QDir::temp().absoluteFilePath("ARM.lock"));

    if (!lockFile.tryLock(50))
    {
        QMessageBox msgBox;
        msgBox.setIcon(QMessageBox::Warning);
        msgBox.setText("Application alredy running");
        msgBox.exec();
        return 1;
    }

    // Make application a widget from a widget
    // Hides it from taskbar, and you don't need a tray entry
    HideFromTaskbar widget(&w);

    w.show();
    return a.exec();
}
