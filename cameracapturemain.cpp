#include "cameracapturemain.h"
#include "ui_cameracapturemain.h"

#include "settings.h"
#include "about.h"
#include "cameracapture.h"

#include <QAudioDevice>
#include <QAudioInput>
#include <QCameraDevice>
#include <QMediaDevices>
#include <QMediaFormat>
#include <QMediaMetaData>
#include <QMediaRecorder>
#include <QVideoWidget>
#include <QDesktopServices>

#include <QLineEdit>
#include <QMessageBox>

#include <QAction>
#include <QActionGroup>
#include <QImage>
#include <QKeyEvent>
#include <QPalette>
#include <QStackedLayout>

#include <QDir>
#include <QTimer>

CameraCaptureMain::CameraCaptureMain() : ui(new Ui::CameraCaptureMain) {
    ui->setupUi(this);
    ui->buttonSetTimerValue->setDisabled(false);
    ui->spinBoxTimerValue->setDisabled(false);
    ui->buttonClearTimer->setDisabled(true);

    // Camera devices:
    videoDevicesGroup = new QActionGroup(this);
    videoDevicesGroup->setExclusive(true);
    updateCameras();
    connect(&m_videoDevices, &QMediaDevices::videoInputsChanged, this, &CameraCaptureMain::updateCameras);
    connect(videoDevicesGroup, &QActionGroup::triggered, this, &CameraCaptureMain::updateCameraDevice);

    // Audio inputs:
    audioDevicesGroup = new QActionGroup(this);
    audioDevicesGroup->setExclusive(true);
    updateAudioDevices();
    connect(&m_audioDevices, &QMediaDevices::audioInputsChanged, this, &CameraCaptureMain::updateAudioDevices);
    connect(audioDevicesGroup, &QActionGroup::triggered, this, &CameraCaptureMain::updateAudioInputDevice);

    this->cameraCapture = new CameraCapture(this);
}

void CameraCaptureMain::setCamera(const QCameraDevice &cameraDevice) {
    dynamic_cast<CameraCapture*>(cameraCapture)->setCamera(cameraDevice);
}

void CameraCaptureMain::setAudioInput(const QAudioDevice &audioDevice) {
    dynamic_cast<CameraCapture*>(cameraCapture)->setAudioInput(audioDevice);
}

void CameraCaptureMain::keyPressEvent(QKeyEvent *event) {
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
                if (dynamic_cast<CameraCapture*>(cameraCapture)->m_mediaRecorder->recorderState() == QMediaRecorder::RecordingState)
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

void CameraCaptureMain::updateRecordTime(QMediaRecorder *mediaRecorder) {
    QString str = tr("Записано %1 сек").arg(mediaRecorder->duration() / 1000);
    ui->statusbar->showMessage(str);
}

void CameraCaptureMain::processCapturedImage(const QImage &img) {
    QImage scaledImage = img.scaled(ui->viewfinder->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);

    ui->lastImagePreviewLabel->setPixmap(QPixmap::fromImage(scaledImage));

    displayCapturedImage();
    QTimer::singleShot(4000, this, &CameraCaptureMain::displayViewfinder);
}

void CameraCaptureMain::openSettings() {
    Settings settingsDialog(dynamic_cast<CameraCapture*>(cameraCapture)->m_imageCapture.data(), dynamic_cast<CameraCapture*>(cameraCapture)->m_mediaRecorder.data());

    if (settingsDialog.exec())
        settingsDialog.applySettings();
}

void CameraCaptureMain::record() {
    dynamic_cast<CameraCapture*>(cameraCapture)->record();
}

void CameraCaptureMain::pause() {
    dynamic_cast<CameraCapture*>(cameraCapture)->pause();
}

void CameraCaptureMain::stop() {
    dynamic_cast<CameraCapture*>(cameraCapture)->stop();
}

void CameraCaptureMain::setMuted(bool muted) {
    dynamic_cast<CameraCapture*>(cameraCapture)->setMuted(muted);
}

void CameraCaptureMain::takeImage() {
    m_isCapturingImage = true;
    dynamic_cast<CameraCapture*>(cameraCapture)->takeImage();
}

void CameraCaptureMain::displayCaptureError(const QString &errorString) {
    QMessageBox::warning(this, tr("Помилка запису зображення"), errorString);
    m_isCapturingImage = false;
}

void CameraCaptureMain::startCamera() {
    dynamic_cast<CameraCapture*>(cameraCapture)->startCamera();
}

void CameraCaptureMain::stopCamera() {
    dynamic_cast<CameraCapture*>(cameraCapture)->stopCamera();
}

void CameraCaptureMain::updateCameraActive(bool active) {
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

void CameraCaptureMain::updateRecorderState(QMediaRecorder::RecorderState state) {
    switch (state) {
        case QMediaRecorder::StoppedState:
            ui->buttonRecord->setEnabled(true);
            ui->buttonPause->setEnabled(true);
            ui->buttonStop->setEnabled(false);
            ui->statusbar->showMessage("Файл із відео успішно збережено", 500);
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

void CameraCaptureMain::displayRecorderError() {
    if (dynamic_cast<CameraCapture*>(cameraCapture)->m_mediaRecorder->error() != QMediaRecorder::NoError)
        QMessageBox::warning(this, tr("Помилка запису"), dynamic_cast<CameraCapture*>(cameraCapture)->m_mediaRecorder->errorString());
}

void CameraCaptureMain::displayCameraError() {
    if (dynamic_cast<CameraCapture*>(cameraCapture)->m_camera->error() != QCamera::NoError)
        QMessageBox::warning(this, tr("Помилка камери"), dynamic_cast<CameraCapture*>(cameraCapture)->m_camera->errorString());
}

void CameraCaptureMain::updateCameraDevice(QAction *action) {
    dynamic_cast<CameraCapture*>(cameraCapture)->setCamera(qvariant_cast<QCameraDevice>(action->data()));
}

void CameraCaptureMain::updateAudioInputDevice(QAction *action) {
    dynamic_cast<CameraCapture*>(cameraCapture)->setAudioInput(qvariant_cast<QAudioDevice>(action->data()));
}

void CameraCaptureMain::displayViewfinder() {
    ui->preview->setCurrentIndex(0);
}

void CameraCaptureMain::displayCapturedImage() {
    ui->preview->setCurrentIndex(1);
}

void CameraCaptureMain::readyForCapture(bool ready) {
    ui->buttonTakeImage->setEnabled(ready);
}

void CameraCaptureMain::imageSaved(const QString &fileName) {
    ui->statusbar->showMessage(tr("Записано у \"%1\"").arg(QDir::toNativeSeparators(fileName)));

    m_isCapturingImage = false;
    if (m_applicationExiting)
        close();
}

void CameraCaptureMain::closeEvent(QCloseEvent *event) {
    if (m_isCapturingImage) {
        setEnabled(false);
        m_applicationExiting = true;
        event->ignore();
    } else {
        event->accept();
    }
}

void CameraCaptureMain::updateCameras() {
    ui->menuVideoDevices->clear();
    const QList<QCameraDevice> availableCameras = QMediaDevices::videoInputs();
    for (const QCameraDevice &cameraDevice : availableCameras) {
        QAction *videoDeviceAction = new QAction(cameraDevice.description(), videoDevicesGroup);
        videoDeviceAction->setCheckable(true);
        videoDeviceAction->setData(QVariant::fromValue(cameraDevice));
        if (cameraDevice == QMediaDevices::defaultVideoInput())
            videoDeviceAction->setChecked(true);

        ui->menuVideoDevices->addAction(videoDeviceAction);
    }
}

void CameraCaptureMain::updateAudioDevices() {
    ui->menuAudioDevices->clear();
    const QList<QAudioDevice> avaliableAudioDevices = QMediaDevices::audioInputs();
    for (const QAudioDevice &audioInputDevice : avaliableAudioDevices) {
        QAction *audioInputDeviceAction = new QAction(audioInputDevice.description(), videoDevicesGroup);
        audioInputDeviceAction->setCheckable(true);
        audioInputDeviceAction->setData(QVariant::fromValue(audioInputDevice));
        if (audioInputDevice == QMediaDevices::defaultAudioInput())
            audioInputDeviceAction->setChecked(true);

        ui->menuAudioDevices->addAction(audioInputDeviceAction);
    }
}

void CameraCaptureMain::openAbout() {
    About* aboutDialog = new About();
    aboutDialog->open();
}

void CameraCaptureMain::openHelp() {
    QDesktopServices::openUrl(QUrl("file:///" + QCoreApplication::applicationDirPath() + "/help.html"));
}

void CameraCaptureMain::setTimerValue() {
    dynamic_cast<CameraCapture*>(cameraCapture)->setTimerValue(ui->spinBoxTimerValue->value());
    ui->buttonSetTimerValue->setDisabled(true);
    ui->spinBoxTimerValue->setDisabled(true);
    ui->buttonClearTimer->setDisabled(false);
}

void CameraCaptureMain::clearTimer() {
    dynamic_cast<CameraCapture*>(cameraCapture)->clearTimer();
    ui->buttonSetTimerValue->setDisabled(false);
    ui->spinBoxTimerValue->setDisabled(false);
    ui->buttonClearTimer->setDisabled(true);
}

