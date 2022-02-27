#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "proxyinput.h"
#include <QtNetwork/QtNetwork>
#include <QFileDialog>
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->progressBar->setVisible(false);
    ui->MainTable->verticalHeader()->setVisible(false);
    ui->MainTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    //this->setStyleSheet("QWidget { background-color:white; } QPushButton {border-color: black; }");
}

MainWindow::~MainWindow()
{
    delete ui;
}

bool sendRequest(QString URL, QString proxyHost, QString proxyPort, QString proxyUsername, QString proxyPassword){
    //need to add extra generic headers

    qDebug() << "Attempting Request";

    QTimer timer;
    timer.setSingleShot(true);

    // create custom temporary event loop on stack
    QEventLoop eventLoop;

    // "quit()" the event-loop, when the network request "finished()"
    QNetworkAccessManager mgr;
    QNetworkProxy proxy;
    proxy.setType(QNetworkProxy::HttpProxy);
    //proxy.setType(QNetworkProxy::Socks5Proxy); //will require extra for implementation
    proxy.setHostName(proxyHost);
    proxy.setPort(proxyPort.toInt());
    proxy.setUser(proxyUsername);
    proxy.setPassword(proxyPassword);
    QString auth = proxyUsername + ":" + proxyPassword;
    QString encoded_auth = "Basic " + auth.toUtf8().toBase64();
    proxy.setRawHeader("Proxy-Authorization", encoded_auth.toUtf8());
    mgr.setProxy(proxy);
    QObject::connect(&mgr, SIGNAL(finished(QNetworkReply*)), &eventLoop, SLOT(quit()));

    // the HTTP request
    QNetworkRequest req((QUrl(URL)));
    if (!URL.contains("https")){
        req.setRawHeader("Proxy-Authorization", encoded_auth.toUtf8());
    }

    req.setRawHeader("User-Agent", "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/98.0.4758.82 Safari/537.36");
    req.setRawHeader("cache-control", "no-cache");
    req.setRawHeader("pragma", "no-cache");
    req.setRawHeader("accept", "text/html,application/xhtml+xml,application/xml;q=0.9,image/avif,image/webp,image/apng,*/*;q=0.8,application/signed-exchange;v=b3;q=0.9");
    req.setRawHeader("accept-encoding", "gzip, deflate, br");
    req.setRawHeader("accept-language", "en-US,en;q=0.9");
    req.setRawHeader("sec-ch-ua", "");
    req.setRawHeader("", "\" Not A;Brand\";v=\"99\", \"Chromium\";v=\"98\", \"Google Chrome\";v=\"98\"");
    req.setRawHeader("sec-ch-ua-mobile", "?0");
    req.setRawHeader("sec-ch-ua-platform", "\"Windows\"");
    req.setRawHeader("sec-fetch-dest", "document");
    req.setRawHeader("sec-fetch-mode", "navigate");
    req.setRawHeader("sec-fetch-site", "same-origin");
    req.setRawHeader("upgrade-insecure-requests", "1");

    QNetworkReply *reply = mgr.get(req);
    QObject::connect(&timer, SIGNAL(timeout()), &eventLoop, SLOT(quit()));
    QObject::connect(reply, SIGNAL(finished()), &eventLoop, SLOT(quit()));
    timer.start(10000);
    eventLoop.exec(); // blocks stack until "finished()" has been called

    if (reply->error() == QNetworkReply::NoError) {
        if(timer.isActive()) {
            timer.stop();
        }
        QString statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toString();
        if (statusCode == "") {
            qDebug() << "Failure" << "Timeout";
            return false;
        }
        else if (statusCode[0] != QString("4") && statusCode[0] != QString("5")) {
            qDebug() << "Success" << reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toString();
            return true;
        }
        else {
            qDebug() << "Failure" << "Other";
            return false;
        }

        delete reply;
    }
    else {
        if(timer.isActive()) {
            timer.stop();
        }
        qDebug() << "Failure" << reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toString();
        return false;
        delete reply;
    }
}

void MainWindow::on_TestBTN_clicked()
{
    if (ui->MainTable->rowCount() > 0){

        ui->MainTable->setSortingEnabled(false);
        ui->ImportBTN->setEnabled(false);
        ui->ClearBTN->setEnabled(false);
        ui->ExportBTN->setEnabled(false);
        ui->TestBTN->setEnabled(false);

        ui->progressBar->setVisible(true);
        QString URL = ui->URLBox->text();
        for (int i = 0; i < ui->MainTable->rowCount(); i++) {\
            ui->MainTable->item(i,4)->setText("");
            ui->MainTable->item(i,5)->setText("");
        }
        for (int i = 0; i < ui->MainTable->rowCount(); i++) {
            QString host = ui->MainTable->item(i, 0)->text();
            QString port = ui->MainTable->item(i, 1)->text();
            QString username = ui->MainTable->item(i, 2)->text();
            QString password = ui->MainTable->item(i, 3)->text();
            QTime myTimer;
            int start = myTimer.currentTime().msecsSinceStartOfDay();
            bool success = sendRequest(URL, host, port, username, password);
            int end = myTimer.currentTime().msecsSinceStartOfDay();
            qDebug() << (end - start) << "ms";
            double percentComplete = (double(i + 1) / (ui->MainTable->rowCount())) * 100;
            if (success) {
                ui->MainTable->item(i,4)->setText("Success");
                ui->MainTable->item(i,4)->setForeground(QBrush(QColor(74, 160, 44)));
                ui->MainTable->item(i,5)->setText(QString::number(end - start) + "ms");
            }
            else {
                ui->MainTable->item(i,4)->setText("Failure");
                ui->MainTable->item(i,4)->setForeground(QBrush(QColor(255, 0, 0)));
                ui->MainTable->item(i,5)->setText("N/A");
            }
            ui->progressBar->setValue(percentComplete);
            ui->MainTable->scrollToItem(ui->MainTable->item(i,0));
        }
        ui->MainTable->setSortingEnabled(true);
        ui->ImportBTN->setEnabled(true);
        ui->ClearBTN->setEnabled(true);
        ui->ExportBTN->setEnabled(true);
        ui->TestBTN->setEnabled(true);
    }
}


void MainWindow::on_ImportBTN_clicked()
{
    ProxyInput proxyInput;
    proxyInput.setModal(true);
    proxyInput.exec();
    if (proxyInput.proxies.size() > 0) {
        ui->MainTable->clearContents();
        ui->MainTable->setRowCount(0);
        ui->progressBar->setVisible(false);
        ui->progressBar->setValue(0);
        QStringList proxyData = proxyInput.proxies.split('\n');
        for (int j = 0; j < proxyData.count(); j++) {
            if (proxyData[j] != ""){
                ui->MainTable->setRowCount(ui->MainTable->rowCount() + 1);
                QTableWidgetItem *item;
                QStringList proxyInfo = proxyData[j].split(":");
                for (int i = 0; i < ui->MainTable->columnCount(); i++) {
                    item = new QTableWidgetItem;
                    switch(i) {
                    case(0):
                        item->setText(proxyInfo[0]);
                        break;
                    case(1):
                        item->setText(proxyInfo[1]);
                        break;
                    case(2):
                        item->setText(proxyInfo[2]);
                        break;
                    case(3):
                        item->setText(proxyInfo[3]);
                        break;
                    case(4):
                        item->setText("");
                        break;
                    case(5):
                        item->setText("");
                        break;
                    }
                    item->setTextAlignment(Qt::AlignCenter);
                    ui->MainTable->setItem(ui->MainTable->rowCount() - 1, i, item);
                }

            }
        }
    }
}


void MainWindow::on_ClearBTN_clicked()
{
    ui->MainTable->clearContents();
    ui->MainTable->setRowCount(0);
    ui->progressBar->setVisible(false);
    ui->progressBar->setValue(0);
}


void MainWindow::on_ExportBTN_clicked()
{
    QString proxyString = "";
    for (int i = 0; i < ui->MainTable->rowCount(); i++) {
        if (ui->MainTable->item(i,4)->text() == "Success") {
            proxyString += ui->MainTable->item(i,0)->text() + ":" + ui->MainTable->item(i,1)->text() + ":" + ui->MainTable->item(i,2)->text() + ":" + ui->MainTable->item(i,3)->text() + '\n';
        }
    }
    const QString downloadsFolder = QStandardPaths::writableLocation(QStandardPaths::DownloadLocation);
    QString fileName = QFileDialog::getSaveFileName(this,
        tr("Save Working Proxies"),
        downloadsFolder + "/Proxies.txt",
        tr("Text Files (*.txt);;All Files (*)")
    );
    if (fileName.isEmpty())
            return;
    else {
        QFile file(fileName);
        if (file.open(QIODevice::ReadWrite)) {
            QTextStream out(&file);
            out << proxyString;
            file.close();
        }
        else {
            QMessageBox::information(this, tr("Unable to save to file"),
                file.errorString());
            return;
        }
    }
}

