#ifndef CLIENT_H
#define CLIENT_H

#include <QWidget>
#include <QAbstractSocket>

#include "messageheader.h"

class QTcpSocket;

namespace Ui {
class client;
}

class client : public QWidget
{
    Q_OBJECT

public:
    explicit client(QWidget *parent = 0);
    ~client();

private slots:
    void on_text_returnPressed();
    void readyRead();
    void on_connect_clicked();
    void connectedToServer();
    void on_disconnect_clicked();
    void disconnectByServer();

private:
    Ui::client *ui;
    QTcpSocket *m_socket;
    QString m_user;
    QByteArray m_receivedData;

    void updateGui(QAbstractSocket::SocketState state);
    void newMessage(const QString &message);
};

#endif // CLIENT_H
