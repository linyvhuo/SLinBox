#ifndef BASECONVERTMODULE_H
#define BASECONVERTMODULE_H

#include <QWidget>
#include <QString>
#include "core/configmanager.h"
#include "core/logger.h"

QT_BEGIN_NAMESPACE
namespace Ui { class BaseConvertModule; }
QT_END_NAMESPACE

class BaseConvertModule : public QWidget
{
    Q_OBJECT

public:
    explicit BaseConvertModule(QWidget *parent = nullptr);
    ~BaseConvertModule();

    void loadConfig();
    void saveConfig();

private slots:
    void onConvert();
    void onClear();
    void onCopyResult();
    void onSourceBaseChanged(int index);
    void onTargetBaseChanged(int index);
    void onInputChanged(const QString &text);

private:
    void initUI();
    QString convertBase(const QString &input, int fromBase, int toBase);
    bool isValidNumber(const QString &input, int base);

    Ui::BaseConvertModule *ui;

    ConfigManager *m_config;
    Logger *m_logger;
};

#endif // BASECONVERTMODULE_H