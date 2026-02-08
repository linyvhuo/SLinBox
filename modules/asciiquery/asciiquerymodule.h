#ifndef ASCIIQUERYMODULE_H
#define ASCIIQUERYMODULE_H

#include <QWidget>
#include "core/configmanager.h"
#include "core/logger.h"

QT_BEGIN_NAMESPACE
namespace Ui { class ASCIIQueryModule; }
QT_END_NAMESPACE

class ASCIIQueryModule : public QWidget
{
    Q_OBJECT

public:
    explicit ASCIIQueryModule(QWidget *parent = nullptr);
    ~ASCIIQueryModule();

    void loadConfig();
    void saveConfig();

private slots:
    void onCharToCode();
    void onCodeToChar();
    void onClear();
    void onShowTable();

private:
    void initUI();
    void showASCIITable();

    Ui::ASCIIQueryModule *ui;

    ConfigManager *m_config;
    Logger *m_logger;
};

#endif // ASCIIQUERYMODULE_H