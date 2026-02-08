#ifndef DATASHEETMODULE_H
#define DATASHEETMODULE_H

#include <QWidget>
#include "core/configmanager.h"
#include "core/logger.h"

QT_BEGIN_NAMESPACE
namespace Ui { class DataSheetModule; }
QT_END_NAMESPACE

class DataSheetModule : public QWidget
{
    Q_OBJECT

public:
    explicit DataSheetModule(QWidget *parent = nullptr);
    ~DataSheetModule();

    void loadConfig();
    void saveConfig();

private slots:
    void onOpenDataSheet();

private:
    void initUI();

    Ui::DataSheetModule *ui;

    ConfigManager *m_config;
    Logger *m_logger;
};

#endif // DATASHEETMODULE_H