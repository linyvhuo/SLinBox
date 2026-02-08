#ifndef SCREENHELPER_H
#define SCREENHELPER_H

#include <QObject>
#include <QPixmap>
#include <QRect>
#include <QPoint>
#include <QScreen>

class ScreenHelper : public QObject
{
    Q_OBJECT

public:
    explicit ScreenHelper(QObject *parent = nullptr);
    ~ScreenHelper();

    static ScreenHelper* instance();

    QList<QScreen*> getScreens();
    QScreen* getPrimaryScreen();
    QScreen* getScreen(int index);
    int getScreenCount();
    int getPrimaryScreenIndex();

    QRect getScreenGeometry(int index = -1);
    QRect getAvailableGeometry(int index = -1);
    QSize getScreenSize(int index = -1);
    QPoint getScreenCenter(int index = -1);

    QPixmap captureScreen(int index = -1);
    QPixmap captureRect(const QRect &rect);
    QPixmap captureWindow(WId windowId);

    bool saveScreenshot(const QPixmap &pixmap, const QString &filePath, const QString &format = "PNG");

    QPoint globalToScreen(const QPoint &globalPos, int screenIndex);
    QPoint screenToGlobal(const QPoint &screenPos, int screenIndex);

    QRect getVirtualScreenGeometry();
    QPoint getVirtualScreenCenter();

    bool isPointInScreen(const QPoint &point);
    int getScreenIndexAtPoint(const QPoint &point);

    void setCursorPos(const QPoint &pos);
    QPoint getCursorPos();

    void hideCursor();
    void showCursor();

private:
    void initScreens();
    void onScreenAdded(QScreen *screen);
    void onScreenRemoved(QScreen *screen);

    QList<QScreen*> m_screens;
    static ScreenHelper *s_instance;
};

#endif // SCREENHELPER_H