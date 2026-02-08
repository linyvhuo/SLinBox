#ifndef USERAGREEMENTDIALOG_H
#define USERAGREEMENTDIALOG_H

#include <QDialog>
#include <QCheckBox>
#include <QPushButton>
#include <QLabel>
#include <QVBoxLayout>
#include <QTextEdit>
#include "core/configmanager.h"
#include "core/logger.h"

QT_BEGIN_NAMESPACE
namespace Ui { class UserAgreementDialog; }
QT_END_NAMESPACE

class UserAgreementDialog : public QDialog
{
    Q_OBJECT

public:
    explicit UserAgreementDialog(QWidget *parent = nullptr);
    ~UserAgreementDialog();

    bool isAgreed() const;

private slots:
    void onAgreeClicked();
    void onDisagreeClicked();

private:
    void initUI();
    void loadConfig();
    void saveConfig();
    
    Ui::UserAgreementDialog *ui;
    ConfigManager *m_config;
    Logger *m_logger;
};

#endif // USERAGREEMENTDIALOG_H
