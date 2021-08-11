#include "client.h"
#include "ui_client.h"

#include <QtWidgets>
#include <QtNetwork>

client::client(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::client)
  , m_socket(new QTcpSocket(this))
{
    ui->setupUi(this);
    ui->address->setText("127.0.0.1");
    ui->port->setValue(52693);

    ui->text->setFocus();

    connect(m_socket, &QTcpSocket::readyRead,
            this, &client::readyRead);
    connect(m_socket, &QTcpSocket::connected,
            this, &client::connectedToServer);
    connect(m_socket, &QTcpSocket::disconnected,
            this, &client::disconnectByServer);
}

client::~client()
{
    delete ui;
}

void client::on_text_returnPressed()
{
    const QString text = ui->text->text().simplified();
    if (text.isEmpty() || m_socket->state() != QAbstractSocket::ConnectedState) {
        return;
    }

    QString message = QStringLiteral("%1: %2")
            .arg(m_user).arg(ui->text->text());

    QByteArray messageArray = message.toUtf8();
    messageArray.append(23);
    m_socket->write(messageArray);
    ui->text->clear();
}

void client::readyRead()
{
    if (m_socket->state() != QAbstractSocket::ConnectedState) {
        return;
    }

    m_receivedData.append(m_socket->readAll());
    while(true) {
        int endIndex = m_receivedData.indexOf(23);
        if (endIndex < 0) {
            break;
        }
        QString message = QString::fromUtf8(m_receivedData.left(endIndex));
        m_receivedData.remove(0, endIndex + 1);
        newMessage(message);
    }
}

void client::on_connect_clicked()
{
    const QString user = ui->user->text().simplified();
    if (user.isEmpty()) {
        ui->chat->appendPlainText(tr("== Unable to connect to server. "
                                     "You must define an user name."));
        return;
    }

    m_user = user;

    if (m_socket->state() != QAbstractSocket::ConnectedState) {
        ui->chat->appendPlainText(tr("== Connecting..."));
        m_socket->connectToHost(ui->address->text(), ui->port->value());
        updateGui(QAbstractSocket::ConnectingState);
    }
}

void client::connectedToServer()
{
    ui->chat->appendPlainText(tr("== Connected to server."));
    updateGui(QAbstractSocket::ConnectedState);
}

void client::on_disconnect_clicked()
{
    if (m_socket->state() != QAbstractSocket::ConnectingState) {
        ui->chat->appendPlainText(tr("== Abort connecting."));
    }
    m_socket->abort();
    updateGui(QAbstractSocket::UnconnectedState);
}

void client::disconnectByServer()
{
    ui->chat->appendPlainText(tr("== Disconnected by server."));
    updateGui(QAbstractSocket::UnconnectedState);
}

void client::updateGui(QAbstractSocket::SocketState state)
{
    const bool connected = (state == QAbstractSocket::ConnectedState);
    const bool unconnected = (state == QAbstractSocket::UnconnectedState);
    ui->connect->setEnabled(unconnected);
    ui->address->setEnabled(unconnected);
    ui->port->setEnabled(unconnected);
    ui->user->setEnabled(unconnected);

    ui->disconnect->setEnabled(!unconnected);
    ui->chat->setEnabled(connected);
    ui->text->setEnabled(connected);
}

void client::newMessage(const QString &message)
{
    ui->chat->appendPlainText(message);
}

