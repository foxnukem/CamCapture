#ifndef SETTINGS_H
#define SETTINGS_H

#include <QDialog>

QT_BEGIN_NAMESPACE
class QCameraFormat;
class QComboBox;
class QImageCapture;
class QMediaRecorder;
namespace Ui {
    class SettingsUi;
}
QT_END_NAMESPACE

class Settings : public QDialog {
    Q_OBJECT

public:
    explicit Settings(QImageCapture *imageCapture, QMediaRecorder *mediaRecorder, QWidget *parent = nullptr);
    ~Settings();

    void applySettings();
    void updateFormatsAndCodecs();

    QString format() const;
    void setFormat(const QString &format);

protected:
    void changeEvent(QEvent *e) override;

private:
    void setFpsRange(const QCameraFormat &format);
    QVariant boxValue(const QComboBox *) const;
    void selectComboBoxItem(QComboBox *box, const QVariant &value);

    Ui::SettingsUi *ui;
    QImageCapture *imageCapture;
    QMediaRecorder *mediaRecorder;
    bool m_updatingFormats = false;
};

#endif // SETTINGS_H
