#ifndef CAMERACAPTUREMAIN_H
#define CAMERACAPTUREMAIN_H

#include <QAudioInput>
#include <QCamera>
#include <QImageCapture>
#include <QMediaCaptureSession>
#include <QMediaDevices>
#include <QMediaMetaData>
#include <QMediaRecorder>
#include <QScopedPointer>

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui {
    class CameraCaptureMain;
}
class QActionGroup;
QT_END_NAMESPACE

class MetaDataDialog;

class CameraCaptureMain : public QMainWindow {
    Q_OBJECT

public:
    Ui::CameraCaptureMain *ui;
    CameraCaptureMain();

public slots:
    void setCamera(const QCameraDevice &cameraDevice);
    void setAudioInput(const QAudioDevice &audioDevice);

    void startCamera();
    void stopCamera();

    void takeImage();
    void record();
    void pause();
    void stop();
    void setMuted(bool);

    void openSettings();
    void openAbout();
    void openHelp();

    void setTimerValue();
    void clearTimer();

    void setGrid(int index);

    void updateCameraDevice(QAction *action);
    void updateCameraActive(bool active);
    void updateAudioInputDevice(QAction *action);

    void updateRecorderState(QMediaRecorder::RecorderState state);

    void updateRecordTime(QMediaRecorder *mediaRecorder);

    void processCapturedImage(const QImage &img);

    void displayViewfinder();
    void displayCapturedImage();

    void readyForCapture(bool ready);
    void imageSaved(const QString &fileName);

    void updateCameras();
    void updateAudioDevices();

    void displayCaptureError(const QString &errorString);
    void displayRecorderError();
    void displayCameraError();

protected:
    void keyPressEvent(QKeyEvent *event) override;
    void closeEvent(QCloseEvent *event) override;

private:
    QObject *cameraCapture;
    QActionGroup *videoDevicesGroup = nullptr;
    QActionGroup *audioDevicesGroup = nullptr;

    QMediaDevices m_videoDevices;
    QMediaDevices m_audioDevices;

    bool m_isCapturingImage = false;
    bool m_applicationExiting = false;
    bool m_doImageCapture = true;
};

#endif
