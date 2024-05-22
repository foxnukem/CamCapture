#ifndef CAMERACAPTURE_H
#define CAMERACAPTURE_H

#include "cameracapturemain.h"

#include <QAudioInput>
#include <QCamera>
#include <QImageCapture>
#include <QMediaCaptureSession>
#include <QMediaDevices>
#include <QMediaMetaData>
#include <QMediaRecorder>
#include <QScopedPointer>

class CameraCapture : public QObject
{
public:
    CameraCapture(CameraCaptureMain *mainForm);
    ~CameraCapture();
    void setCamera(const QCameraDevice &cameraDevice);
    void setAudioInput(const QAudioDevice &audioDevice);
    void startCamera();
    void stopCamera();
    void takeImage();
    void record();
    void pause();
    void stop();
    void setMuted(bool);
    void setTimerValue(int newValue);
    void clearTimer();

    QScopedPointer<QImageCapture> m_imageCapture;
    QScopedPointer<QAudioInput> m_audioInput;
    QScopedPointer<QMediaRecorder> m_mediaRecorder;
    QScopedPointer<QCamera> m_camera;
    QMediaCaptureSession m_captureSession;
private:
    CameraCaptureMain *mainForm;
    QMediaDevices m_videoDevices;
    QMediaDevices m_audioDevices;

    int m_timerValueInSeconds;
};

#endif // CAMERACAPTURE_H
