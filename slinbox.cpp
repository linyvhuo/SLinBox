#include "slinbox.h"
#include "ui_slinbox.h"

SLinBox::SLinBox(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::SLinBox)
{
    ui->setupUi(this);
}

SLinBox::~SLinBox()
{
    delete ui;
}

