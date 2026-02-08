#ifndef USERGUIDEDIALOG_H
#define USERGUIDEDIALOG_H

#include <QDialog>
#include <QTextEdit>

QT_BEGIN_NAMESPACE
namespace Ui { class UserGuideDialog; }
QT_END_NAMESPACE

class UserGuideDialog : public QDialog
{
    Q_OBJECT

public:
    explicit UserGuideDialog(QWidget *parent = nullptr);
    ~UserGuideDialog();

private slots:
    void onOkClicked();

private:
    void initUI();
    void loadGuideContent();

    Ui::UserGuideDialog *ui;
};

#endif // USERGUIDEDIALOG_H
