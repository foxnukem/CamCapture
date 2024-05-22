#include "cameracapture.h"
#include "cameracapturemain.h"
#include "ui_cameracapturemain.h"

#include <QAudioDevice>
#include <QAudioInput>
#include <QCameraDevice>
#include <QMediaDevices>
#include <QMediaFormat>
#include <QMediaMetaData>
#include <QMediaRecorder>
#include <QVideoWidget>

#include <QLineEdit>
#include <QMessageBox>

#include <QAction>
#include <QActionGroup>
#include <QImage>
#include <QKeyEvent>
#include <QPalette>

#include <QDir>
#include <QTimer>

CameraCapture::CameraCapture(CameraCaptureMain *mainForm) {
    this->mainForm = mainForm;
    this->m_timerValueInSeconds = 0;
    this->setCamera(QMediaDevices::defaultVideoInput());
    this->setAudioInput(QMediaDevices::defaultAudioInput());
}

CameraCapture::~CameraCapture() {
    delete mainForm;
}

void CameraCapture::setCamera(const QCameraDevice &cameraDevice) {
    m_camera.reset(new QCamera(cameraDevice));
    m_captureSession.setCamera(m_camera.data());

    connect(m_camera.data(), &QCamera::activeChanged, this, [=](bool active) { mainForm->updateCameraActive(active); });
    connect(m_camera.data(), &QCamera::errorOccurred, this, [=]() { mainForm->displayCameraError(); });

    if (!m_mediaRecorder) {
        m_mediaRecorder.reset(new QMediaRecorder);
        m_captureSession.setRecorder(m_mediaRecorder.data());
        connect(m_mediaRecorder.data(), &QMediaRecorder::recorderStateChanged, this, [=](auto state) { mainForm->updateRecorderState(state); });
        connect(m_mediaRecorder.data(), &QMediaRecorder::durationChanged, this, [=]() { mainForm->updateRecordTime(this->m_mediaRecorder.get()); });
        connect(m_mediaRecorder.data(), &QMediaRecorder::errorChanged, this, [=]() { mainForm->displayRecorderError(); });
    }

    if (!m_imageCapture) {
        m_imageCapture.reset(new QImageCapture);
        m_captureSession.setImageCapture(m_imageCapture.get());
        connect(m_imageCapture.get(), &QImageCapture::readyForCaptureChanged, this, [=](bool ready) { mainForm->readyForCapture(ready); });
        connect(m_imageCapture.get(), &QImageCapture::imageCaptured, this, [=](int, const QImage &img) { mainForm->processCapturedImage(img); });
        connect(m_imageCapture.get(), &QImageCapture::imageSaved, this, [=](int, const QString &fileName) { mainForm->imageSaved(fileName); });
        connect(m_imageCapture.get(), &QImageCapture::errorOccurred, this, [=](int, auto, auto &errorString) { mainForm->displayCaptureError(errorString); });
    }

    m_captureSession.setVideoOutput(mainForm->ui->viewfinder);

    mainForm->updateCameraActive(m_camera->isActive());
    mainForm->updateRecorderState(m_mediaRecorder->recorderState());
    mainForm->readyForCapture(m_imageCapture->isReadyForCapture());

    m_camera->start();
}

void CameraCapture::setAudioInput(const QAudioDevice &audioDevice) {
    m_audioInput.reset(new QAudioInput(audioDevice));
    m_captureSession.setAudioInput(m_audioInput.data());
}

void CameraCapture::record() {
    if (m_timerValueInSeconds > 0) {
        QTimer::singleShot(1000 * m_timerValueInSeconds, this, [=](){
            m_mediaRecorder->record();
            mainForm->updateRecordTime(m_mediaRecorder.data());
        });
        return;
    }
    m_mediaRecorder->record();
    mainForm->updateRecordTime(m_mediaRecorder.data());
}

void CameraCapture::pause() {
    m_mediaRecorder->pause();
}

void CameraCapture::stop() {
    m_mediaRecorder->stop();
}

void CameraCapture::setMuted(bool muted) {
    m_captureSession.audioInput()->setMuted(muted);
}

void CameraCapture::takeImage() {
    if (m_timerValueInSeconds > 0) {
        QTimer::singleShot(1000 * m_timerValueInSeconds, this, [=](){ this->m_imageCapture->captureToFile(); });
        return;
    }
    m_imageCapture->captureToFile();
}

void CameraCapture::setTimerValue(int newValue) {
    m_timerValueInSeconds = newValue;
}

void CameraCapture::clearTimer() {
    m_timerValueInSeconds = 0;
}

void CameraCapture::startCamera() {
    m_camera->start();
}

void CameraCapture::stopCamera() {
    m_camera->stop();
}

