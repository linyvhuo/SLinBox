#include "windowhelper.h"
#include <windows.h>
#include <tlhelp32.h>
#include <psapi.h>
#include <string>
#include <QApplication>
#include <QClipboard>

WindowHelper* WindowHelper::s_instance = nullptr;

WindowHelper::WindowHelper(QObject *parent)
    : QObject(parent)
{
}

WindowHelper::~WindowHelper()
{
}

WindowHelper* WindowHelper::instance()
{
    if (!s_instance) {
        s_instance = new WindowHelper();
    }
    return s_instance;
}

bool WindowHelper::activateWindow(const QString &windowTitle)
{
    HWND hwnd = findWindowByTitle(windowTitle);
    if (!hwnd) {
        return false;
    }

    if (IsIconic(hwnd)) {
        ShowWindow(hwnd, SW_RESTORE);
    }

    return SetForegroundWindow(hwnd) != FALSE;
}

bool WindowHelper::activateWindowByProcess(const QString &processName)
{
    HWND hwnd = findWindowByProcess(processName);
    if (!hwnd) {
        return false;
    }

    if (IsIconic(hwnd)) {
        ShowWindow(hwnd, SW_RESTORE);
    }

    return SetForegroundWindow(hwnd) != FALSE;
}

bool WindowHelper::isWindowExists(const QString &windowTitle)
{
    return findWindowByTitle(windowTitle) != nullptr;
}

bool WindowHelper::isProcessRunning(const QString &processName)
{
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnapshot == INVALID_HANDLE_VALUE) {
        return false;
    }

    PROCESSENTRY32 pe32;
    pe32.dwSize = sizeof(PROCESSENTRY32);

    if (!Process32First(hSnapshot, &pe32)) {
        CloseHandle(hSnapshot);
        return false;
    }

    bool found = false;
    do {
        QString currentProcess = QString::fromWCharArray(pe32.szExeFile);
        if (currentProcess.compare(processName, Qt::CaseInsensitive) == 0) {
            found = true;
            break;
        }
    } while (Process32Next(hSnapshot, &pe32));

    CloseHandle(hSnapshot);
    return found;
}

bool WindowHelper::setWindowTopMost(const QString &windowTitle, bool topMost)
{
    HWND hwnd = findWindowByTitle(windowTitle);
    if (!hwnd) {
        return false;
    }

    return SetWindowPos(hwnd, topMost ? HWND_TOPMOST : HWND_NOTOPMOST,
                       0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE) != FALSE;
}

bool WindowHelper::setWindowPos(const QString &windowTitle, int x, int y, int width, int height)
{
    HWND hwnd = findWindowByTitle(windowTitle);
    if (!hwnd) {
        return false;
    }

    return SetWindowPos(hwnd, nullptr, x, y, width, height,
                       SWP_NOZORDER) != FALSE;
}

QRect WindowHelper::getWindowRect(const QString &windowTitle)
{
    HWND hwnd = findWindowByTitle(windowTitle);
    if (!hwnd) {
        return QRect();
    }

    RECT rect;
    if (GetWindowRect(hwnd, &rect)) {
        return QRect(rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top);
    }

    return QRect();
}

QPoint WindowHelper::getWindowCenter(const QString &windowTitle)
{
    QRect rect = getWindowRect(windowTitle);
    if (rect.isEmpty()) {
        return QPoint();
    }

    return rect.center();
}

bool WindowHelper::minimizeWindow(const QString &windowTitle)
{
    HWND hwnd = findWindowByTitle(windowTitle);
    if (!hwnd) {
        return false;
    }

    return ShowWindow(hwnd, SW_MINIMIZE) != FALSE;
}

bool WindowHelper::maximizeWindow(const QString &windowTitle)
{
    HWND hwnd = findWindowByTitle(windowTitle);
    if (!hwnd) {
        return false;
    }

    return ShowWindow(hwnd, SW_MAXIMIZE) != FALSE;
}

bool WindowHelper::restoreWindow(const QString &windowTitle)
{
    HWND hwnd = findWindowByTitle(windowTitle);
    if (!hwnd) {
        return false;
    }

    return ShowWindow(hwnd, SW_RESTORE) != FALSE;
}

bool WindowHelper::closeWindow(const QString &windowTitle)
{
    HWND hwnd = findWindowByTitle(windowTitle);
    if (!hwnd) {
        return false;
    }

    return PostMessage(hwnd, WM_CLOSE, 0, 0) != FALSE;
}

bool WindowHelper::startProcess(const QString &processPath, const QString &arguments)
{
    STARTUPINFO si;
    PROCESS_INFORMATION pi;

    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    ZeroMemory(&pi, sizeof(pi));

    QString command = "\"" + processPath + "\"";
    if (!arguments.isEmpty()) {
        command += " " + arguments;
    }

    std::wstring cmd = command.toStdWString();
    return CreateProcess(nullptr, const_cast<LPWSTR>(cmd.c_str()), nullptr, nullptr,
                        FALSE, 0, nullptr, nullptr, &si, &pi) != FALSE;
}

bool WindowHelper::killProcess(const QString &processName)
{
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnapshot == INVALID_HANDLE_VALUE) {
        return false;
    }

    PROCESSENTRY32 pe32;
    pe32.dwSize = sizeof(PROCESSENTRY32);

    if (!Process32First(hSnapshot, &pe32)) {
        CloseHandle(hSnapshot);
        return false;
    }

    bool killed = false;
    do {
        QString currentProcess = QString::fromWCharArray(pe32.szExeFile);
        if (currentProcess.compare(processName, Qt::CaseInsensitive) == 0) {
            HANDLE hProcess = OpenProcess(PROCESS_TERMINATE, FALSE, pe32.th32ProcessID);
            if (hProcess) {
                TerminateProcess(hProcess, 0);
                CloseHandle(hProcess);
                killed = true;
            }
        }
    } while (Process32Next(hSnapshot, &pe32));

    CloseHandle(hSnapshot);
    return killed;
}

QString WindowHelper::getActiveWindowTitle()
{
    HWND hwnd = GetForegroundWindow();
    if (!hwnd) {
        return QString();
    }

    return getWindowTitle(hwnd);
}

QString WindowHelper::getActiveWindowProcessName()
{
    HWND hwnd = GetForegroundWindow();
    if (!hwnd) {
        return QString();
    }

    return getProcessName(hwnd);
}

bool WindowHelper::clickWindow(const QString &windowTitle, int x, int y)
{
    HWND hwnd = findWindowByTitle(windowTitle);
    if (!hwnd) {
        return false;
    }

    RECT rect;
    if (!GetWindowRect(hwnd, &rect)) {
        return false;
    }

    int absoluteX = rect.left + x;
    int absoluteY = rect.top + y;

    SetCursorPos(absoluteX, absoluteY);
    Sleep(50);

    mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, 0);
    Sleep(50);
    mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);

    return true;
}

bool WindowHelper::clickAt(int x, int y)
{
    SetCursorPos(x, y);
    Sleep(50);

    mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, 0);
    Sleep(50);
    mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);

    return true;
}

bool WindowHelper::sendKeys(const QString &windowTitle, const QString &keys, std::function<bool()> stopCheck)
{
    Q_UNUSED(windowTitle);
    
    for (int i = 0; i < keys.length(); ++i) {
        if (stopCheck && stopCheck()) {
            return false;
        }
        
        QChar ch = keys[i];
        wchar_t wc = ch.unicode();
        
        INPUT input[2];
        
        ZeroMemory(&input[0], sizeof(INPUT));
        input[0].type = INPUT_KEYBOARD;
        input[0].ki.wVk = 0;
        input[0].ki.wScan = wc;
        input[0].ki.dwFlags = KEYEVENTF_UNICODE;
        
        ZeroMemory(&input[1], sizeof(INPUT));
        input[1].type = INPUT_KEYBOARD;
        input[1].ki.wVk = 0;
        input[1].ki.wScan = wc;
        input[1].ki.dwFlags = KEYEVENTF_UNICODE | KEYEVENTF_KEYUP;
        
        UINT result = SendInput(2, input, sizeof(INPUT));
        if (result != 2) {
            DWORD error = GetLastError();
        }
        
        Sleep(100);
    }

    Sleep(500);

    return true;
}

bool WindowHelper::pasteText(const QString &windowTitle, const QString &text, std::function<bool()> stopCheck)
{
    Q_UNUSED(windowTitle);
    
     QClipboard *qtClipboard = QApplication::clipboard();
    if (!qtClipboard) {
        return false;
    }

    QString originalContent = qtClipboard->text();

    if (stopCheck && stopCheck()) {
        return false;
    }

    int textLength = text.length();
    HGLOBAL hGlobal = GlobalAlloc(GMEM_MOVEABLE, (textLength + 1) * sizeof(wchar_t));
    if (!hGlobal) {
        return false;
    }

    wchar_t* pGlobal = (wchar_t*)GlobalLock(hGlobal);
    if (!pGlobal) {
        GlobalFree(hGlobal);
        return false;
    }

    memcpy(pGlobal, text.utf16(), (textLength + 1) * sizeof(wchar_t));
    GlobalUnlock(hGlobal);

    if (!OpenClipboard(nullptr)) {
        GlobalFree(hGlobal);
        return false;
    }

    EmptyClipboard();

    HANDLE hData = SetClipboardData(CF_UNICODETEXT, hGlobal);
    if (!hData) {
        CloseClipboard();
        GlobalFree(hGlobal);
        return false;
    }

    CloseClipboard();
    Sleep(300);

    QString clipboardContent = qtClipboard->text();
    if (clipboardContent != text) {
    }

    if (stopCheck && stopCheck()) {
        qtClipboard->setText(originalContent);
        return false;
    }

    keybd_event(VK_CONTROL, 0, 0, 0);
    Sleep(50);
    keybd_event(0x56, 0, 0, 0);
    Sleep(50);
    keybd_event(0x56, 0, KEYEVENTF_KEYUP, 0);
    Sleep(50);
    keybd_event(VK_CONTROL, 0, KEYEVENTF_KEYUP, 0);
    Sleep(500);

    if (stopCheck && stopCheck()) {
        qtClipboard->setText(originalContent);
        return false;
    }

    Sleep(800);

    qtClipboard->setText(originalContent);
    Sleep(100);

    return true;
}

bool WindowHelper::setWindowTransparent(const QString &windowTitle, int alpha)
{
    HWND hwnd = findWindowByTitle(windowTitle);
    if (!hwnd) {
        return false;
    }

    if (alpha < 0) alpha = 0;
    if (alpha > 255) alpha = 255;

    LONG style = GetWindowLong(hwnd, GWL_EXSTYLE);
    SetWindowLong(hwnd, GWL_EXSTYLE, style | WS_EX_LAYERED);

    return SetLayeredWindowAttributes(hwnd, 0, alpha, LWA_ALPHA) != FALSE;
}

HWND WindowHelper::findWindowByTitle(const QString &title)
{
    HWND hwnd = nullptr;
    hwnd = FindWindow(nullptr, title.toStdWString().c_str());
    return hwnd;
}

HWND WindowHelper::findWindowByProcess(const QString &processName)
{
    HWND hwnd = nullptr;
    
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnapshot == INVALID_HANDLE_VALUE) {
        return nullptr;
    }

    PROCESSENTRY32 pe32;
    pe32.dwSize = sizeof(PROCESSENTRY32);

    if (!Process32First(hSnapshot, &pe32)) {
        CloseHandle(hSnapshot);
        return nullptr;
    }

    DWORD processId = 0;
    do {
        QString currentProcess = QString::fromWCharArray(pe32.szExeFile);
        if (currentProcess.compare(processName, Qt::CaseInsensitive) == 0) {
            processId = pe32.th32ProcessID;
            break;
        }
    } while (Process32Next(hSnapshot, &pe32));

    CloseHandle(hSnapshot);

    if (processId == 0) {
        return nullptr;
    }

    struct EnumWindowsData {
        DWORD processId;
        HWND hwnd;
    } data = { processId, nullptr };

    EnumWindows([](HWND hWnd, LPARAM lParam) -> BOOL {
        EnumWindowsData* data = reinterpret_cast<EnumWindowsData*>(lParam);
        DWORD windowProcessId;
        GetWindowThreadProcessId(hWnd, &windowProcessId);
        
        if (windowProcessId == data->processId && IsWindowVisible(hWnd)) {
            data->hwnd = hWnd;
            return FALSE;
        }
        return TRUE;
    }, reinterpret_cast<LPARAM>(&data));

    return data.hwnd;
}

QString WindowHelper::getWindowTitle(HWND hwnd)
{
    if (!hwnd) {
        return QString();
    }

    int length = GetWindowTextLength(hwnd);
    if (length == 0) {
        return QString();
    }

    wchar_t* buffer = new wchar_t[length + 1];
    GetWindowText(hwnd, buffer, length + 1);
    
    QString title = QString::fromWCharArray(buffer);
    delete[] buffer;

    return title;
}

QString WindowHelper::getProcessName(HWND hwnd)
{
    if (!hwnd) {
        return QString();
    }

    DWORD processId;
    GetWindowThreadProcessId(hwnd, &processId);

    HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, processId);
    if (!hProcess) {
        return QString();
    }

    wchar_t buffer[MAX_PATH];
    DWORD size = MAX_PATH;
    
    if (!QueryFullProcessImageName(hProcess, 0, buffer, &size)) {
        CloseHandle(hProcess);
        return QString();
    }

    CloseHandle(hProcess);

    QString fullPath = QString::fromWCharArray(buffer);
    return fullPath.mid(fullPath.lastIndexOf('\\') + 1);
}
