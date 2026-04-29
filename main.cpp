#include <QApplication>
#include "gamewidget.h"

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);

    app.setApplicationName("扎扎师幸存者");
    app.setApplicationVersion("1.2.1");

    GameWidget gameWidget;
    gameWidget.setWindowTitle("扎扎师幸存者");
    gameWidget.show();
    
    return app.exec();
}
