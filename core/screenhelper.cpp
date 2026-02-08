#include "screenhelper.h"
#include <QGuiApplication>
#include <QCursor>
#include <QPixmap>
#include <QDebug>

ScreenHelper* ScreenHelper::s_instance = nullptr;

ScreenHelper::ScreenHelper(QObject *parent)
    : QObject(parent)
{
    // 延迟初始化屏幕列表，避免在QApplication未完全初始化时访问
    // initScreens() 将在首次调用时执行
    
    QGuiApplication *app = qobject_cast<QGuiApplication*>(QCoreApplication::instance());
    if (app) {
        connect(app, &QGuiApplication::screenAdded, this, &ScreenHelper::onScreenAdded);
        connect(app, &QGuiApplication::screenRemoved, this, &ScreenHelper::onScreenRemoved);
    }
}

ScreenHelper::~ScreenHelper()
{
}

ScreenHelper* ScreenHelper::instance()
{
    if (!s_instance) {
        s_instance = new ScreenHelper();
        // 在QApplication完全初始化后再初始化屏幕列表
        s_instance->initScreens();
    }
    return s_instance;
}

QList<QScreen*> ScreenHelper::getScreens()
{
    return m_screens;
}

QScreen* ScreenHelper::getPrimaryScreen()
{
    return QGuiApplication::primaryScreen();
}

QScreen* ScreenHelper::getScreen(int index)
{
    if (index < 0 || index >= m_screens.size()) {
        return getPrimaryScreen();
    }
    return m_screens[index];
}

int ScreenHelper::getScreenCount()
{
    return m_screens.size();
}

int ScreenHelper::getPrimaryScreenIndex()
{
    QScreen *primary = getPrimaryScreen();
    for (int i = 0; i < m_screens.size(); ++i) {
        if (m_screens[i] == primary) {
            return i;
        }
    }
    return 0;
}

QRect ScreenHelper::getScreenGeometry(int index)
{
    QScreen *screen = getScreen(index);
    if (screen) {
        return screen->geometry();
    }
    return QRect();
}

QRect ScreenHelper::getAvailableGeometry(int index)
{
    QScreen *screen = getScreen(index);
    if (screen) {
        return screen->availableGeometry();
    }
    return QRect();
}

QSize ScreenHelper::getScreenSize(int index)
{
    return getScreenGeometry(index).size();
}

QPoint ScreenHelper::getScreenCenter(int index)
{
    return getScreenGeometry(index).center();
}

QPixmap ScreenHelper::captureScreen(int index)
{
    QScreen *screen = getScreen(index);
    if (screen) {
        return screen->grabWindow(0);
    }
    return QPixmap();
}

QPixmap ScreenHelper::captureRect(const QRect &rect)
{
    QScreen *screen = QGuiApplication::screenAt(rect.center());
    if (screen) {
        return screen->grabWindow(0, rect.x(), rect.y(), rect.width(), rect.height());
    }
    return QPixmap();
}

QPixmap ScreenHelper::captureWindow(WId windowId)
{
    QScreen *screen = QGuiApplication::primaryScreen();
    if (screen) {
        return screen->grabWindow(windowId);
    }
    return QPixmap();
}

bool ScreenHelper::saveScreenshot(const QPixmap &pixmap, const QString &filePath, const QString &format)
{
    if (pixmap.isNull()) {
        return false;
    }
    return pixmap.save(filePath, format.toUtf8().constData());
}

QPoint ScreenHelper::globalToScreen(const QPoint &globalPos, int screenIndex)
{
    QScreen *screen = getScreen(screenIndex);
    if (screen) {
        return globalPos - screen->geometry().topLeft();
    }
    return globalPos;
}

QPoint ScreenHelper::screenToGlobal(const QPoint &screenPos, int screenIndex)
{
    QScreen *screen = getScreen(screenIndex);
    if (screen) {
        return screen->geometry().topLeft() + screenPos;
    }
    return screenPos;
}

QRect ScreenHelper::getVirtualScreenGeometry()
{
    QRect virtualRect;
    for (QScreen *screen : m_screens) {
        virtualRect = virtualRect.united(screen->geometry());
    }
    return virtualRect;
}

QPoint ScreenHelper::getVirtualScreenCenter()
{
    return getVirtualScreenGeometry().center();
}

bool ScreenHelper::isPointInScreen(const QPoint &point)
{
    for (QScreen *screen : m_screens) {
        if (screen->geometry().contains(point)) {
            return true;
        }
    }
    return false;
}

int ScreenHelper::getScreenIndexAtPoint(const QPoint &point)
{
    for (int i = 0; i < m_screens.size(); ++i) {
        if (m_screens[i]->geometry().contains(point)) {
            return i;
        }
    }
    return -1;
}

void ScreenHelper::setCursorPos(const QPoint &pos)
{
    QCursor::setPos(pos);
}

QPoint ScreenHelper::getCursorPos()
{
    return QCursor::pos();
}

void ScreenHelper::hideCursor()
{
    QGuiApplication::setOverrideCursor(Qt::BlankCursor);
}

void ScreenHelper::showCursor()
{
    QGuiApplication::restoreOverrideCursor();
}

void ScreenHelper::initScreens()
{
    m_screens = QGuiApplication::screens();
}

void ScreenHelper::onScreenAdded(QScreen *screen)
{
    if (!m_screens.contains(screen)) {
        m_screens.append(screen);
    }
}

void ScreenHelper::onScreenRemoved(QScreen *screen)
{
    m_screens.removeAll(screen);
}