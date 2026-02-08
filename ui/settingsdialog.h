#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QDialog>
#include <QKeySequenceEdit>
#include "core/configmanager.h"
#include "core/logger.h"

QT_BEGIN_NAMESPACE
namespace Ui { class SettingsDialog; }
QT_END_NAMESPACE

class SettingsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SettingsDialog(QWidget *parent = nullptr);
    ~SettingsDialog();

    QString getTheme() const;
    QString getLanguage() const;
    QString getCloseAction() const;
    bool getStartupShow() const;
    bool getAutoReply() const;
    QString getScreenshotHotkey() const;
    QString getStopHotkey() const;
    QString getOcrHotkey() const;
    QString getClipboardOcrHotkey() const;
    QString getOCREngine() const;
    QString getOCRLanguage() const;
    QString getOCRPath() const;
    QString getLogLevel() const;
    QString getLogPath() const;
    int getMaxLogSize() const;
    QString getDataPath() const;

private slots:
    void onAccepted();
    void onBrowseLogPath();
    void onBrowseDataPath();
    void onBrowseOCRPath();
    void onClearData();
    void onSoftwareInfoClicked();
    void onUserGuideClicked();
    void onUserAgreementClicked();
    void onHomePageClicked();
    void onUpdateClicked();
    
signals:
    void settingsApplied();
    
private:
    void initUI();
    void loadSettings();
    void saveSettings();

    Ui::SettingsDialog *ui;

    ConfigManager *m_config;
    Logger *m_logger;
};

#endif // SETTINGSDIALOG_H