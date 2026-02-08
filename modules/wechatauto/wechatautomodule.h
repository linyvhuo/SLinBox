#ifndef WECHATAUTOMODULE_H
#define WECHATAUTOMODULE_H

#include <QWidget>
#include <QThread>
#include <QTimer>
#include <QMutex>
#include <QFile>
#include <QByteArray>
#ifdef USE_OPENCV
#include <opencv2/opencv.hpp>
#endif
#include "core/configmanager.h"
#include "core/logger.h"
#include "core/windowhelper.h"
#include "core/screenhelper.h"
#include "ui/floatwindow.h"

QT_BEGIN_NAMESPACE
namespace Ui { class WeChatAutoModule; }
QT_END_NAMESPACE

class WeChatAutoWorker : public QObject
{
    Q_OBJECT

public:
    explicit WeChatAutoWorker(QObject *parent = nullptr);
    ~WeChatAutoWorker();

    void setConfig(const QMap<QString, QVariant> &config);
    void setQuestions(const QStringList &questions);
    void stop();
    bool shouldStop();

signals:
    void progressChanged(int current, int total);
    void statusChanged(const QString &status);
    void errorOccurred(const QString &error);
    void finished();

public slots:
    void startAutomation();

private:
    bool startWeChat();
    bool activateWeChatWindow();
    bool recognizeIcon(const QString &iconName, QPoint &position);
    bool clickIcon(const QPoint &position);
    bool recognizeAndOpenIcon(const QString &iconName);
    bool inputQuestion(const QString &question);
    bool clickSendButton();
    bool waitForAnswer(int timeout);
    bool executeQuestionLoop();

    QString getTemplatePath(const QString &iconName);
#ifdef USE_OPENCV
    cv::Mat loadTemplate(const QString &path);
    bool matchTemplate(const cv::Mat &screen, const cv::Mat &templ, cv::Point &matchLoc, double threshold);
    double matchTemplateNCC(const cv::Mat &screen, const cv::Mat &templ, int x, int y);
#endif

    QMap<QString, QVariant> m_config;
    QStringList m_questions;
    bool m_running;
    QMutex m_mutex;
    
    ConfigManager *m_configManager;
};

class WeChatAutoModule : public QWidget
{
    Q_OBJECT

public:
    explicit WeChatAutoModule(QWidget *parent = nullptr);
    ~WeChatAutoModule();

    void loadConfig();
    void saveConfig();

private slots:
    void onBrowseWeChatPath();
    void onBrowseQuestionLibrary();
    void onStartAutomation();
    void onStopAutomation();
    void onProgressChanged(int current, int total);
    void onStatusChanged(const QString &status);
    void onErrorOccurred(const QString &error);
    void onWorkerFinished();
    void onQuestionModeChanged(int index);
    void onBrowseInputBoxIcon();
    void onBrowseHistoryChatIcon();
    void onBrowseMindsparkTabIcon();
    void onBrowseMindsparkAppIcon();
    void onBrowseWorkbenchIcon();
    void onBrowseSendButtonIcon();
    void onPreviewQuestions();
    void onFloatWindowStopClicked();
    void onFloatWindowEnableChanged(int state);
    void onStopHotkeyTriggered();

private:
    void initUI();
    void loadQuestionsFromFile(const QString &filePath);
    void generateQuestionsFromKeywords();
    QString getQuestionModeString() const;
    void showQuestionsPreview();
    void showIconPreview(const QString &iconPath);
    void setupStopHotkey(const QString &hotkeyStr);

    Ui::WeChatAutoModule *ui;

    QThread *m_workerThread;
    WeChatAutoWorker *m_worker;

    ConfigManager *m_config;
    Logger *m_logger;

    QStringList m_questions;
    
    FloatWindow *m_floatWindow;
    bool m_stopHotkeyRegistered;
};

#endif // WECHATAUTOMODULE_H