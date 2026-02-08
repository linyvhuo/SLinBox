#include "globalhotkeymanager.h"
#include <QApplication>
#include <QWidget>
#include <QDebug>

GlobalHotkeyManager::GlobalHotkeyManager(QObject *parent)
    : QObject(parent)
    , m_nextId(1)
    , m_windowHandle(nullptr)
{
    qApp->installNativeEventFilter(this);
}

GlobalHotkeyManager::~GlobalHotkeyManager()
{
    unregisterAllHotkeys();
    qApp->removeNativeEventFilter(this);
}

GlobalHotkeyManager* GlobalHotkeyManager::instance()
{
    static GlobalHotkeyManager instance;
    return &instance;
}

void GlobalHotkeyManager::setWindowHandle(QWidget *window)
{
#ifdef Q_OS_WIN
    if (window) {
        m_windowHandle = reinterpret_cast<HWND>(window->winId());
    }
#else
    Q_UNUSED(window);
#endif
}

bool GlobalHotkeyManager::registerHotkey(const QKeySequence &keySequence, QObject *receiver, const char *slot)
{
    if (keySequence.isEmpty() || !receiver || !slot) {
        return false;
    }
    
#ifdef Q_OS_WIN
    int modifiers = 0;
    int key = 0;
    
    for (int i = 0; i < keySequence.count(); ++i) {
        int keyInt = keySequence[i];
        
        Qt::KeyboardModifiers mod = Qt::KeyboardModifiers(keyInt & Qt::KeyboardModifierMask);
        Qt::Key k = Qt::Key(keyInt & ~Qt::KeyboardModifierMask);
        
        modifiers |= convertToNativeModifiers(mod);
        key = convertToNativeKey(k);
    }
    
    if (key == 0) {
        return false;
    }
    
    int hotkeyId = m_nextId++;
    
    HWND hwnd = m_windowHandle;
    if (!hwnd) {
        QWidget *activeWindow = QApplication::activeWindow();
        if (!activeWindow) {
            QWidgetList topLevelWidgets = QApplication::topLevelWidgets();
            for (QWidget *widget : topLevelWidgets) {
                if (!widget->isWindow() || widget->windowType() == Qt::Desktop || widget->windowType() == Qt::SubWindow) {
                    continue;
                }
                activeWindow = widget;
                break;
            }
        }
        
        if (!activeWindow) {
            qWarning() << "Failed to register hotkey: No active window found";
            return false;
        }
        
        hwnd = reinterpret_cast<HWND>(activeWindow->winId());
    }
    
    if (!RegisterHotKey(hwnd, hotkeyId, modifiers, key)) {
        qWarning() << "Failed to register hotkey:" << keySequence.toString();
        return false;
    }
    
    m_hotkeyMap[hotkeyId] = qMakePair(receiver, slot);
    m_idToKeySequence[hotkeyId] = keySequence;
    
    return true;
#else
    Q_UNUSED(keySequence);
    Q_UNUSED(receiver);
    Q_UNUSED(slot);
    qWarning() << "Global hotkeys are not supported on this platform";
    return false;
#endif
}

bool GlobalHotkeyManager::unregisterHotkey(const QKeySequence &keySequence)
{
    if (keySequence.isEmpty()) {
        return false;
    }
    
#ifdef Q_OS_WIN
    HWND hwnd = m_windowHandle;
    if (!hwnd) {
        QWidget *activeWindow = QApplication::activeWindow();
        if (!activeWindow) {
            QWidgetList topLevelWidgets = QApplication::topLevelWidgets();
            for (QWidget *widget : topLevelWidgets) {
                if (!widget->isWindow() || widget->windowType() == Qt::Desktop || widget->windowType() == Qt::SubWindow) {
                    continue;
                }
                activeWindow = widget;
                break;
            }
        }
        
        if (!activeWindow) {
            qWarning() << "Failed to unregister hotkey: No active window found";
            return false;
        }
        
        hwnd = reinterpret_cast<HWND>(activeWindow->winId());
    }
    
    for (auto it = m_idToKeySequence.begin(); it != m_idToKeySequence.end(); ) {
        if (it.value() == keySequence) {
            int hotkeyId = it.key();
            
            if (UnregisterHotKey(hwnd, hotkeyId)) {
                m_hotkeyMap.remove(hotkeyId);
                it = m_idToKeySequence.erase(it);
                return true;
            }
        } else {
            ++it;
        }
    }
    return false;
#else
    Q_UNUSED(keySequence);
    return false;
#endif
}

void GlobalHotkeyManager::unregisterAllHotkeys()
{
#ifdef Q_OS_WIN
    HWND hwnd = m_windowHandle;
    if (!hwnd) {
        QWidget *activeWindow = QApplication::activeWindow();
        if (!activeWindow) {
            QWidgetList topLevelWidgets = QApplication::topLevelWidgets();
            for (QWidget *widget : topLevelWidgets) {
                if (!widget->isWindow() || widget->windowType() == Qt::Desktop || widget->windowType() == Qt::SubWindow) {
                    continue;
                }
                activeWindow = widget;
                break;
            }
        }
        
        if (activeWindow) {
            hwnd = reinterpret_cast<HWND>(activeWindow->winId());
        }
    }
    
    for (auto it = m_idToKeySequence.begin(); it != m_idToKeySequence.end(); ) {
        int hotkeyId = it.key();
        if (hwnd) {
            UnregisterHotKey(hwnd, hotkeyId);
        }
        m_hotkeyMap.remove(hotkeyId);
        it = m_idToKeySequence.erase(it);
    }
#endif
}

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
bool GlobalHotkeyManager::nativeEventFilter(const QByteArray &eventType, void *message, qintptr *result)
#else
bool GlobalHotkeyManager::nativeEventFilter(const QByteArray &eventType, void *message, long *result)
#endif
{
    Q_UNUSED(result);
    
#ifdef Q_OS_WIN
    if (eventType == "windows_generic_MSG") {
        MSG *msg = static_cast<MSG*>(message);
        
        if (msg->message == WM_HOTKEY) {
            int hotkeyId = static_cast<int>(msg->wParam);
            
            if (m_hotkeyMap.contains(hotkeyId)) {
                auto pair = m_hotkeyMap[hotkeyId];
                
                QString slotName = QString::fromLatin1(pair.second);
                if (slotName.startsWith("1")) {
                    slotName = slotName.mid(1);
                }
                int parenIndex = slotName.indexOf('(');
                if (parenIndex != -1) {
                    slotName = slotName.left(parenIndex);
                }
                
                QMetaObject::invokeMethod(pair.first, slotName.toLatin1().constData());
                return true;
            }
        }
    }
#else
    Q_UNUSED(eventType);
    Q_UNUSED(message);
#endif
    
    return false;
}

int GlobalHotkeyManager::convertToNativeModifiers(Qt::KeyboardModifiers modifiers)
{
    int nativeMods = 0;
    
    if (modifiers & Qt::ControlModifier) {
        nativeMods |= MOD_CONTROL;
    }
    if (modifiers & Qt::AltModifier) {
        nativeMods |= MOD_ALT;
    }
    if (modifiers & Qt::ShiftModifier) {
        nativeMods |= MOD_SHIFT;
    }
    if (modifiers & Qt::MetaModifier) {
        nativeMods |= MOD_WIN;
    }
    
    return nativeMods;
}

int GlobalHotkeyManager::convertToNativeKey(int key)
{
    switch (key) {
        case Qt::Key_A: return 'A';
        case Qt::Key_B: return 'B';
        case Qt::Key_C: return 'C';
        case Qt::Key_D: return 'D';
        case Qt::Key_E: return 'E';
        case Qt::Key_F: return 'F';
        case Qt::Key_G: return 'G';
        case Qt::Key_H: return 'H';
        case Qt::Key_I: return 'I';
        case Qt::Key_J: return 'J';
        case Qt::Key_K: return 'K';
        case Qt::Key_L: return 'L';
        case Qt::Key_M: return 'M';
        case Qt::Key_N: return 'N';
        case Qt::Key_O: return 'O';
        case Qt::Key_P: return 'P';
        case Qt::Key_Q: return 'Q';
        case Qt::Key_R: return 'R';
        case Qt::Key_S: return 'S';
        case Qt::Key_T: return 'T';
        case Qt::Key_U: return 'U';
        case Qt::Key_V: return 'V';
        case Qt::Key_W: return 'W';
        case Qt::Key_X: return 'X';
        case Qt::Key_Y: return 'Y';
        case Qt::Key_Z: return 'Z';
        case Qt::Key_0: return '0';
        case Qt::Key_1: return '1';
        case Qt::Key_2: return '2';
        case Qt::Key_3: return '3';
        case Qt::Key_4: return '4';
        case Qt::Key_5: return '5';
        case Qt::Key_6: return '6';
        case Qt::Key_7: return '7';
        case Qt::Key_8: return '8';
        case Qt::Key_9: return '9';
        case Qt::Key_F1: return VK_F1;
        case Qt::Key_F2: return VK_F2;
        case Qt::Key_F3: return VK_F3;
        case Qt::Key_F4: return VK_F4;
        case Qt::Key_F5: return VK_F5;
        case Qt::Key_F6: return VK_F6;
        case Qt::Key_F7: return VK_F7;
        case Qt::Key_F8: return VK_F8;
        case Qt::Key_F9: return VK_F9;
        case Qt::Key_F10: return VK_F10;
        case Qt::Key_F11: return VK_F11;
        case Qt::Key_F12: return VK_F12;
        case Qt::Key_Space: return VK_SPACE;
        case Qt::Key_Return: return VK_RETURN;
        case Qt::Key_Enter: return VK_RETURN;
        case Qt::Key_Escape: return VK_ESCAPE;
        case Qt::Key_Tab: return VK_TAB;
        case Qt::Key_Backspace: return VK_BACK;
        case Qt::Key_Delete: return VK_DELETE;
        case Qt::Key_Insert: return VK_INSERT;
        case Qt::Key_Home: return VK_HOME;
        case Qt::Key_End: return VK_END;
        case Qt::Key_PageUp: return VK_PRIOR;
        case Qt::Key_PageDown: return VK_NEXT;
        case Qt::Key_Left: return VK_LEFT;
        case Qt::Key_Up: return VK_UP;
        case Qt::Key_Right: return VK_RIGHT;
        case Qt::Key_Down: return VK_DOWN;
        default: return 0;
    }
}
