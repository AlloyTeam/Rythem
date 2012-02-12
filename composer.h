#ifndef COMPOSER_H
#define COMPOSER_H

#include <QWidget>
#include <QTcpSocket>

namespace Ui {
    class Composer;
}

class Composer : public QWidget
{
        Q_OBJECT

    public:
        explicit Composer(QWidget *parent = 0);
        ~Composer();
        void setupProxy(QString host,qint16 port);

    private:
        Ui::Composer *ui;
        QTcpSocket *socket;

        QString _proxyHost;
        quint16 _proxyPort;

        QList<QStringList> _requests;

    private slots:
        void onConnected();
        void onData();
        void onError(QAbstractSocket::SocketError);
        void onClose();

        void onConnectBtnClick();
        void onDisConnectBtnClick();
        void onSendBtnClick();
        void onRequestChanged(int);

        void connectToHost();
        void sendData(const QByteArray &ba);
};

#endif // COMPOSER_H
