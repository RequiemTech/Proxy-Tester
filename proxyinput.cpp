#include "proxyinput.h"
#include "ui_proxyinput.h"

ProxyInput::ProxyInput(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ProxyInput)
{
    ui->setupUi(this);
}

ProxyInput::~ProxyInput()
{
    delete ui;
}

void ProxyInput::on_buttonBox_accepted()
{
    this->proxies = ui->MainField->toPlainText();
}

