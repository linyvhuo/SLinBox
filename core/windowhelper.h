#ifndef WINDOWHELPER_H
#define WINDOWHELPER_H

#include <windows.h>
#include <QObject>
#include <QString>
#include <QRect>
#include <QPoint>

class WindowHelper : public QObject
{
    Q_OBJECT

public:
    explicit WindowHelper(QObject *parent = nullptr);
    ~WindowHelper();

    static WindowHelper* instance();

    bool activateWindow(const QString &windowTitle);
    bool activateWindowByProcess(const QString &processName);
    bool isWindowExists(const QString &windowTitle);
    bool isProcessRunning(const QString &processName);
    bool setWindowTopMost(const QString &windowTitle, bool topMost);
    bool setWindowPos(const QString &windowTitle, int x, int y, int width, int height);
    QRect getWindowRect(const QString &windowTitle);
    QPoint getWindowCenter(const QString &windowTitle);
    bool minimizeWindow(const QString &windowTitle);
    bool maximizeWindow(const QString &windowTitle);
    bool restoreWindow(const QString &windowTitle);
    bool closeWindow(const QString &windowTitle);
    bool startProcess(const QString &processPath, const QString &arguments = "");
    bool killProcess(const QString &processName);
    QString getActiveWindowTitle();
    QString getActiveWindowProcessName();
    bool clickWindow(const QString &windowTitle, int x, int y);
    bool clickAt(int x, int y);
    bool sendKeys(const QString &windowTitle, const QString &keys, std::function<bool()> stopCheck = nullptr);
    bool pasteText(const QString &windowTitle, const QString &text, std::function<bool()> stopCheck = nullptr);
    bool setWindowTransparent(const QString &windowTitle, int alpha);

private:
    HWND findWindowByTitle(const QString &title);
    HWND findWindowByProcess(const QString &processName);
    QString getWindowTitle(HWND hwnd);
    QString getProcessName(HWND hwnd);

    static WindowHelper *s_instance;
};

#endif // WINDOWHELPER_H