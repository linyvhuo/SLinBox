#ifndef DELAYCALCMODULE_H
#define DELAYCALCMODULE_H

#include <QWidget>
#include "core/configmanager.h"
#include "core/logger.h"

QT_BEGIN_NAMESPACE
namespace Ui { class DelayCalcModule; }
QT_END_NAMESPACE

class DelayCalcModule : public QWidget
{
    Q_OBJECT

public:
    explicit DelayCalcModule(QWidget *parent = nullptr);
    ~DelayCalcModule();

    void loadConfig();
    void saveConfig();

private slots:
    void onCalculateForward();
    void onCalculateReverse();
    void onCalculateBaudRate();
    void onClear();
    void onCopyResult();

private:
    void initUI();

    Ui::DelayCalcModule *ui;

    ConfigManager *m_config;
    Logger *m_logger;
};

#endif // DELAYCALCMODULE_H