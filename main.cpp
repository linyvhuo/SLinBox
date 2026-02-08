#include "ui/mainwindow.h"
#include "core/configmanager.h"
#include "core/logger.h"
#include <QApplication>
#include <QDebug>
#include <QMessageBox>

int main(int argc, char *argv[])
{
    try {
        QApplication a(argc, argv);
        
        // 确保QApplication完全初始化后再初始化单例
        // 延迟初始化单例，避免在QApplication未完全初始化时访问Qt相关功能
        MainWindow w;
        w.show();
        
        // 在MainWindow构造函数中初始化单例，确保正确的初始化顺序
        return a.exec();
    } catch (const std::exception& e) {
        QMessageBox::critical(nullptr, "严重错误", 
                           QString("程序启动失败: %1").arg(e.what()));
        return -1;
    } catch (...) {
        QMessageBox::critical(nullptr, "严重错误", 
                           "程序启动失败: 未知异常");
        return -1;
    }
}
