#include "cameracapture.h"
#include "ui_cameracapture.h"

#include "settings.h"
#include "about.h"

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

CameraCapture::CameraCapture() : ui(new Ui::CameraCapture) {
    ui->setupUi(this);

    m_audioInput.reset(new QAudioInput);
    m_captureSession.setAudioInput(m_audioInput.get());

    // Camera devices:
    videoDevicesGroup = new QActionGroup(this);
    videoDevicesGroup->setExclusive(true);
    updateCameras();
    connect(&m_devices, &QMediaDevices::videoInputsChanged, this, &CameraCapture::updateCameras);

    connect(videoDevicesGroup, &QActionGroup::triggered, this, &CameraCapture::updateCameraDevice);

    connect(ui->sliderExposureCompensation, &QAbstractSlider::valueChanged, this, &CameraCapture::setExposureCompensation);

    setCamera(QMediaDevices::defaultVideoInput());
}

void CameraCapture::setCamera(const QCameraDevice &cameraDevice) {
    m_camera.reset(new QCamera(cameraDevice));
    m_captureSession.setCamera(m_camera.data());

    connect(m_camera.data(), &QCamera::activeChanged, this, &CameraCapture::updateCameraActive);
    connect(m_camera.data(), &QCamera::errorOccurred, this, &CameraCapture::displayCameraError);

    if (!m_mediaRecorder) {
        m_mediaRecorder.reset(new QMediaRecorder);
        m_captureSession.setRecorder(m_mediaRecorder.data());
        connect(m_mediaRecorder.data(), &QMediaRecorder::recorderStateChanged, this, &CameraCapture::updateRecorderState);
        connect(m_mediaRecorder.data(), &QMediaRecorder::durationChanged, this, &CameraCapture::updateRecordTime);
        connect(m_mediaRecorder.data(), &QMediaRecorder::errorChanged, this, &CameraCapture::displayRecorderError);
    }

    if (!m_imageCapture) {
        m_imageCapture.reset(new QImageCapture);
        m_captureSession.setImageCapture(m_imageCapture.get());
        connect(m_imageCapture.get(), &QImageCapture::readyForCaptureChanged, this, &CameraCapture::readyForCapture);
        connect(m_imageCapture.get(), &QImageCapture::imageCaptured, this, &CameraCapture::processCapturedImage);
        connect(m_imageCapture.get(), &QImageCapture::imageSaved, this, &CameraCapture::imageSaved);
        connect(m_imageCapture.get(), &QImageCapture::errorOccurred, this, &CameraCapture::displayCaptureError);
    }

    m_captureSession.setVideoOutput(ui->viewfinder);

    updateCameraActive(m_camera->isActive());
    updateRecorderState(m_mediaRecorder->recorderState());
    readyForCapture(m_imageCapture->isReadyForCapture());

    m_camera->start();
}

void CameraCapture::keyPressEvent(QKeyEvent *event) {
    if (event->isAutoRepeat())
        return;

    switch (event->key()) {
    case Qt::Key_CameraFocus:
        displayViewfinder();
        event->accept();
        break;
    case Qt::Key_Camera:
        if (m_doImageCapture) {
            takeImage();
        } else {
            if (m_mediaRecorder->recorderState() == QMediaRecorder::RecordingState)
                stop();
            else
                record();
        }
        event->accept();
        break;
    default:
        QMainWindow::keyPressEvent(event);
    }
}

void CameraCapture::updateRecordTime() {
    QString str = tr("Записано %1 сек").arg(m_mediaRecorder->duration() / 1000);
    ui->statusbar->showMessage(str);
}

void CameraCapture::processCapturedImage(int requestId, const QImage &img) {
    Q_UNUSED(requestId);
    QImage scaledImage = img.scaled(ui->viewfinder->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);

    ui->lastImagePreviewLabel->setPixmap(QPixmap::fromImage(scaledImage));

    displayCapturedImage();
    QTimer::singleShot(4000, this, &CameraCapture::displayViewfinder);
}

void CameraCapture::openSettings() {
    Settings settingsDialog(m_imageCapture.data(), m_mediaRecorder.data());

    if (settingsDialog.exec())
        settingsDialog.applySettings();
}

void CameraCapture::record() {
    m_mediaRecorder->record();
    updateRecordTime();
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
    m_isCapturingImage = true;
    m_imageCapture->captureToFile();
}

void CameraCapture::displayCaptureError(int id, const QImageCapture::Error error, const QString &errorString) {
    Q_UNUSED(id);
    Q_UNUSED(error);
    QMessageBox::warning(this, tr("Image Capture Error"), errorString);
    m_isCapturingImage = false;
}

void CameraCapture::startCamera() {
    m_camera->start();
}

void CameraCapture::stopCamera() {
    m_camera->stop();
}

void CameraCapture::updateCameraActive(bool active) {
    if (active) {
        ui->actionStartCamera->setEnabled(false);
        ui->actionStopCamera->setEnabled(true);
        ui->actionSettings->setEnabled(true);
    } else {
        ui->actionStartCamera->setEnabled(true);
        ui->actionStopCamera->setEnabled(false);
        ui->actionSettings->setEnabled(false);
    }
}

void CameraCapture::updateRecorderState(QMediaRecorder::RecorderState state) {
    switch (state) {
    case QMediaRecorder::StoppedState:
        ui->buttonRecord->setEnabled(true);
        ui->buttonPause->setEnabled(true);
        ui->buttonStop->setEnabled(false);
        break;
    case QMediaRecorder::PausedState:
        ui->buttonRecord->setEnabled(true);
        ui->buttonPause->setEnabled(false);
        ui->buttonStop->setEnabled(true);
        break;
    case QMediaRecorder::RecordingState:
        ui->buttonRecord->setEnabled(false);
        ui->buttonPause->setEnabled(true);
        ui->buttonStop->setEnabled(true);
        break;
    }
}

void CameraCapture::setExposureCompensation(int index) {
    m_camera->setExposureCompensation(index * 0.5);
}

void CameraCapture::displayRecorderError() {
    if (m_mediaRecorder->error() != QMediaRecorder::NoError)
        QMessageBox::warning(this, tr("Capture Error"), m_mediaRecorder->errorString());
}

void CameraCapture::displayCameraError() {
    if (m_camera->error() != QCamera::NoError)
        QMessageBox::warning(this, tr("Camera Error"), m_camera->errorString());
}

void CameraCapture::updateCameraDevice(QAction *action) {
    setCamera(qvariant_cast<QCameraDevice>(action->data()));
}

void CameraCapture::displayViewfinder() {
    ui->stackedWidget->setCurrentIndex(0);
}

void CameraCapture::displayCapturedImage() {
    ui->stackedWidget->setCurrentIndex(1);
}

void CameraCapture::readyForCapture(bool ready) {
    ui->buttonTakeImage->setEnabled(ready);
}

void CameraCapture::imageSaved(int id, const QString &fileName) {
    Q_UNUSED(id);
    ui->statusbar->showMessage(tr("Записано у \"%1\"").arg(QDir::toNativeSeparators(fileName)));

    m_isCapturingImage = false;
    if (m_applicationExiting)
        close();
}

void CameraCapture::closeEvent(QCloseEvent *event) {
    if (m_isCapturingImage) {
        setEnabled(false);
        m_applicationExiting = true;
        event->ignore();
    } else {
        event->accept();
    }
}

void CameraCapture::updateCameras() {
    ui->menuDevices->clear();
    const QList<QCameraDevice> availableCameras = QMediaDevices::videoInputs();
    for (const QCameraDevice &cameraDevice : availableCameras) {
        QAction *videoDeviceAction = new QAction(cameraDevice.description(), videoDevicesGroup);
        videoDeviceAction->setCheckable(true);
        videoDeviceAction->setData(QVariant::fromValue(cameraDevice));
        if (cameraDevice == QMediaDevices::defaultVideoInput())
            videoDeviceAction->setChecked(true);

        ui->menuDevices->addAction(videoDeviceAction);
    }
}

void CameraCapture::openAbout() {
    About* aboutDialog = new About();
    aboutDialog->open();
}

void CameraCapture::setTimerValue() {
// todo:
}


void CameraCapture::clearTimer() {

}

void CameraCapture::setGrid(int index) {

}
