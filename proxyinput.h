#ifndef PROXYINPUT_H
#define PROXYINPUT_H

#include <QDialog>
namespace Ui {
class ProxyInput;
}

class ProxyInput : public QDialog
{
    Q_OBJECT

public:
    explicit ProxyInput(QWidget *parent = nullptr);
    ~ProxyInput();
    QString proxies;

private slots:
    void on_buttonBox_accepted();

private:
    Ui::ProxyInput *ui;
};

#endif // PROXYINPUT_H
