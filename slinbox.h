#ifndef SLINBOX_H
#define SLINBOX_H

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui { class SLinBox; }
QT_END_NAMESPACE

class SLinBox : public QMainWindow
{
    Q_OBJECT

public:
    SLinBox(QWidget *parent = nullptr);
    ~SLinBox();

private:
    Ui::SLinBox *ui;
};
#endif // SLINBOX_H
