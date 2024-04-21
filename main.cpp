#include "cameracapture.h"

#include <QApplication>

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    CameraCapture camera;
    camera.show();

    return app.exec();
};
