#ifndef GLOBALHOTKEYMANAGER_H
#define GLOBALHOTKEYMANAGER_H

#include <QObject>
#include <QMap>
#include <QKeySequence>
#include <QAbstractNativeEventFilter>
#include <QWidget>
#include <QString>

#ifdef Q_OS_WIN
#include <windows.h>
#endif

class GlobalHotkeyManager : public QObject, public QAbstractNativeEventFilter
{
    Q_OBJECT

public:
    static GlobalHotkeyManager* instance();
    
    void setWindowHandle(QWidget *window);
    bool registerHotkey(const QKeySequence &keySequence, QObject *receiver, const char *slot);
    bool unregisterHotkey(const QKeySequence &keySequence);
    void unregisterAllHotkeys();

protected:
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    bool nativeEventFilter(const QByteArray &eventType, void *message, qintptr *result) override;
#else
    bool nativeEventFilter(const QByteArray &eventType, void *message, long *result) override;
#endif

private:
    explicit GlobalHotkeyManager(QObject *parent = nullptr);
    ~GlobalHotkeyManager();
    
    int convertToNativeModifiers(Qt::KeyboardModifiers modifiers);
    int convertToNativeKey(int key);

    QMap<int, QPair<QObject*, const char*>> m_hotkeyMap;
    QMap<int, QKeySequence> m_idToKeySequence;
    int m_nextId;
#ifdef Q_OS_WIN
    HWND m_windowHandle;
#endif
};

#endif // GLOBALHOTKEYMANAGER_H
