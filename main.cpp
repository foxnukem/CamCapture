#include "cameracapturemain.h"

#include <QApplication>

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    CameraCaptureMain camera;
    camera.show();

    return app.exec();
};
