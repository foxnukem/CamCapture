#ifndef CAMERACAPTURE_H
#define CAMERACAPTURE_H

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
    class CameraCapture;
}
class QActionGroup;
QT_END_NAMESPACE

class MetaDataDialog;

class CameraCapture : public QMainWindow {
    Q_OBJECT

public:
    CameraCapture();

private slots:
    void setCamera(const QCameraDevice &cameraDevice);

    void startCamera();
    void stopCamera();

    void takeImage();
    void record();
    void pause();
    void stop();
    void setMuted(bool);

    void openSettings();
    void openAbout();
    void setTimerValue();
    void clearTimer();
    void setGrid(int index);

    void displayCaptureError(int, QImageCapture::Error, const QString &errorString);

    void displayRecorderError();
    void displayCameraError();

    void updateCameraDevice(QAction *action);

    void updateCameraActive(bool active);
    void updateRecorderState(QMediaRecorder::RecorderState state);
    void setExposureCompensation(int index);

    void updateRecordTime();

    void processCapturedImage(int requestId, const QImage &img);

    void displayViewfinder();
    void displayCapturedImage();

    void readyForCapture(bool ready);
    void imageSaved(int id, const QString &fileName);

    void updateCameras();

protected:
    void keyPressEvent(QKeyEvent *event) override;
    void closeEvent(QCloseEvent *event) override;

private:
    Ui::CameraCapture *ui;

    QActionGroup *videoDevicesGroup = nullptr;

    QMediaDevices m_devices;
    QScopedPointer<QImageCapture> m_imageCapture;
    QMediaCaptureSession m_captureSession;
    QScopedPointer<QCamera> m_camera;
    QScopedPointer<QAudioInput> m_audioInput;
    QScopedPointer<QMediaRecorder> m_mediaRecorder;

    bool m_isCapturingImage = false;
    bool m_applicationExiting = false;
    bool m_doImageCapture = true;

    MetaDataDialog *m_metaDataDialog = nullptr;
};

#endif
