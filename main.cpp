#include "mainwindow.h"
#include <QApplication>
#include <QFont>

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    app.setApplicationName("AIChatbot");
    app.setOrganizationName("OOPProject");

    QFont font("Segoe UI", 10);
    app.setFont(font);

    MainWindow w;
    w.show();

    return app.exec();
}