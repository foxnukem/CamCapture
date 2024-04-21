#include "settings.h"
#include "ui_settings.h"

#include <QAudioDevice>
#include <QAudioInput>
#include <QCamera>
#include <QImageCapture>
#include <QCameraDevice>
#include <QMediaCaptureSession>
#include <QMediaFormat>
#include <QMediaRecorder>

#include <QComboBox>
#include <QSpinBox>

#include <QDebug>
#include <QTextStream>

static QString toFormattedString(const QCameraFormat &cameraFormat) {
    QString string;
    QTextStream str(&string);
    str << QVideoFrameFormat::pixelFormatToString(cameraFormat.pixelFormat())
        << ' ' << cameraFormat.resolution().width()
        << 'x' << cameraFormat.resolution().height()
        << ' ' << cameraFormat.minFrameRate()
        << '-' << cameraFormat.maxFrameRate() << "кадрів на секунду";
    return string;
}

Settings::Settings(QImageCapture *imageCapture, QMediaRecorder *mediaRecorder, QWidget *parent) : QDialog(parent), ui(new Ui::SettingsUi), imageCapture(imageCapture), mediaRecorder(mediaRecorder) {
    ui->setupUi(this);

    // image codecs
    ui->comboBoxImageCodec->addItem(tr("Формат за замовчуванням"), QVariant(QString()));
    const auto supportedImageFormats = QImageCapture::supportedFormats();
    for (const auto &f : supportedImageFormats) {
        QString description = QImageCapture::fileFormatDescription(f);
        ui->comboBoxImageCodec->addItem(QImageCapture::fileFormatName(f) + ": " + description, QVariant::fromValue(f));
    }

    ui->sliderImageQuality->setRange(0, int(QImageCapture::VeryHighQuality));

    ui->comboBoxImageResolution->addItem(tr("Роздільна здатність за замовчуванням"));
    const QList<QSize> supportedResolutions =
        imageCapture->captureSession()->camera()->cameraDevice().photoResolutions();
    for (const QSize &resolution : supportedResolutions) {
        ui->comboBoxImageResolution->addItem(
            QString("%1x%2").arg(resolution.width()).arg(resolution.height()),
            QVariant(resolution));
    }

    selectComboBoxItem(ui->comboBoxImageCodec, QVariant::fromValue(imageCapture->fileFormat()));
    selectComboBoxItem(ui->comboBoxImageResolution, QVariant(imageCapture->resolution()));
    ui->sliderImageQuality->setValue(imageCapture->quality());

    // sample rate:
    auto audioDevice = mediaRecorder->captureSession()->audioInput()->device();
    ui->spinBoxAudioSampleRate->setRange(audioDevice.minimumSampleRate(), audioDevice.maximumSampleRate());

    // camera format
    ui->comboBoxVideoResolution->addItem(tr("Формат за замовчуванням"));

    auto *camera = mediaRecorder->captureSession()->camera();
    const QList<QCameraFormat> videoFormats = camera->cameraDevice().videoFormats();

    for (const QCameraFormat &format : videoFormats)
        ui->comboBoxVideoResolution->addItem(toFormattedString(format), QVariant::fromValue(format));

    connect(ui->comboBoxVideoResolution, &QComboBox::currentIndexChanged, [this](int /*index*/) {
        this->setFpsRange(boxValue(ui->comboBoxVideoResolution).value<QCameraFormat>());
    });

    setFpsRange(camera->cameraFormat());

    connect(ui->sliderVideoFps, &QSlider::valueChanged, ui->spinBoxVIdeoFps, &QSpinBox::setValue);
    connect(ui->spinBoxVIdeoFps, &QSpinBox::valueChanged, ui->sliderVideoFps, &QSlider::setValue);

    updateFormatsAndCodecs();
    connect(ui->comboBoxAudioCodec, &QComboBox::currentIndexChanged, this, &Settings::updateFormatsAndCodecs);
    connect(ui->comboBoxVideoCodec, &QComboBox::currentIndexChanged, this, &Settings::updateFormatsAndCodecs);
    connect(ui->comboBoxAudioFormat, &QComboBox::currentIndexChanged, this, &Settings::updateFormatsAndCodecs);

    ui->sliderAudioQuality->setRange(0, int(QMediaRecorder::VeryHighQuality));

    QMediaFormat format = mediaRecorder->mediaFormat();
    selectComboBoxItem(ui->comboBoxAudioFormat, QVariant::fromValue(format.fileFormat()));
    selectComboBoxItem(ui->comboBoxAudioCodec, QVariant::fromValue(format.audioCodec()));
    selectComboBoxItem(ui->comboBoxVideoCodec, QVariant::fromValue(format.videoCodec()));

    ui->sliderAudioQuality->setValue(mediaRecorder->quality());
    ui->spinBoxAudioSampleRate->setValue(mediaRecorder->audioSampleRate());
    selectComboBoxItem(ui->comboBoxVideoResolution, QVariant::fromValue(camera->cameraFormat()));

    ui->sliderVideoFps->setValue(mediaRecorder->videoFrameRate());
    ui->spinBoxVIdeoFps->setValue(mediaRecorder->videoFrameRate());
}

Settings::~Settings() {
    delete ui;
}

void Settings::changeEvent(QEvent *e) {
    QDialog::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        break;
    default:
        break;
    }
}

void Settings::setFpsRange(const QCameraFormat &format) {
    ui->sliderVideoFps->setRange(format.minFrameRate(), format.maxFrameRate());
    ui->spinBoxVIdeoFps->setRange(format.minFrameRate(), format.maxFrameRate());
}

void Settings::applySettings() {
    QMediaFormat format;
    format.setFileFormat(boxValue(ui->comboBoxAudioFormat).value<QMediaFormat::FileFormat>());
    format.setAudioCodec(boxValue(ui->comboBoxAudioCodec).value<QMediaFormat::AudioCodec>());
    format.setVideoCodec(boxValue(ui->comboBoxVideoCodec).value<QMediaFormat::VideoCodec>());

    imageCapture->setFileFormat(boxValue(ui->comboBoxImageCodec).value<QImageCapture::FileFormat>());
    imageCapture->setQuality(QImageCapture::Quality(ui->sliderImageQuality->value()));
    imageCapture->setResolution(boxValue(ui->comboBoxImageResolution).toSize());

    mediaRecorder->setMediaFormat(format);
    mediaRecorder->setQuality(QMediaRecorder::Quality(ui->sliderAudioQuality->value()));
    mediaRecorder->setAudioSampleRate(ui->spinBoxAudioSampleRate->value());

    const auto &cameraFormat = boxValue(ui->comboBoxVideoResolution).value<QCameraFormat>();
    mediaRecorder->setVideoResolution(cameraFormat.resolution());
    mediaRecorder->setVideoFrameRate(ui->sliderVideoFps->value());

    mediaRecorder->captureSession()->camera()->setCameraFormat(cameraFormat);
}

void Settings::updateFormatsAndCodecs() {
    if (m_updatingFormats)
        return;
    m_updatingFormats = true;

    QMediaFormat format;
    if (ui->comboBoxAudioFormat->count())
        format.setFileFormat(boxValue(ui->comboBoxAudioFormat).value<QMediaFormat::FileFormat>());
    if (ui->comboBoxAudioCodec->count())
        format.setAudioCodec(boxValue(ui->comboBoxAudioCodec).value<QMediaFormat::AudioCodec>());
    if (ui->comboBoxVideoCodec->count())
        format.setVideoCodec(boxValue(ui->comboBoxVideoCodec).value<QMediaFormat::VideoCodec>());

    int currentIndex = 0;
    ui->comboBoxAudioCodec->clear();
    ui->comboBoxAudioCodec->addItem(tr("Аудіокодек за замовчуванням"), QVariant::fromValue(QMediaFormat::AudioCodec::Unspecified));
    for (auto codec : format.supportedAudioCodecs(QMediaFormat::Encode)) {
        if (codec == format.audioCodec())
            currentIndex = ui->comboBoxAudioCodec->count();
        ui->comboBoxAudioCodec->addItem(QMediaFormat::audioCodecDescription(codec),
                                   QVariant::fromValue(codec));
    }
    ui->comboBoxAudioCodec->setCurrentIndex(currentIndex);

    currentIndex = 0;
    ui->comboBoxVideoCodec->clear();
    ui->comboBoxVideoCodec->addItem(tr("Відеокодек за замовчуванням"), QVariant::fromValue(QMediaFormat::VideoCodec::Unspecified));
    for (auto codec : format.supportedVideoCodecs(QMediaFormat::Encode)) {
        if (codec == format.videoCodec())
            currentIndex = ui->comboBoxVideoCodec->count();
        ui->comboBoxVideoCodec->addItem(QMediaFormat::videoCodecDescription(codec),
                                   QVariant::fromValue(codec));
    }
    ui->comboBoxVideoCodec->setCurrentIndex(currentIndex);

    currentIndex = 0;
    ui->comboBoxAudioFormat->clear();
    ui->comboBoxAudioFormat->addItem(tr("Формат файлу за замовчуванням"),
                                    QVariant::fromValue(QMediaFormat::UnspecifiedFormat));
    for (auto container : format.supportedFileFormats(QMediaFormat::Encode)) {
        if (container == format.fileFormat())
            currentIndex = ui->comboBoxAudioFormat->count();
        ui->comboBoxAudioFormat->addItem(QMediaFormat::fileFormatDescription(container),
                                        QVariant::fromValue(container));
    }
    ui->comboBoxAudioFormat->setCurrentIndex(currentIndex);

    m_updatingFormats = false;
}

QVariant Settings::boxValue(const QComboBox *box) const {
    const int idx = box->currentIndex();
    return idx != -1 ? box->itemData(idx) : QVariant{};
}

void Settings::selectComboBoxItem(QComboBox *box, const QVariant &value) {
    const int idx = box->findData(value);
    if (idx != -1)
        box->setCurrentIndex(idx);
}
