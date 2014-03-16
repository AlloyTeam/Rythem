#ifndef RYPIPEDATA_H
#define RYPIPEDATA_H

#include <QObject>
#include <QSharedPointer>
#include <QVariant>
#include <QDebug>


class RyPipeData : public QObject
{
        Q_OBJECT
    public:
        explicit RyPipeData(int socketHandle,quint64 socketConnectionId,QObject *parent = 0);

        bool operator <(RyPipeData &pipeData);

        // return byte remain (if the request/response has content-length but
        //   not complete yet
        int parseRequest(QByteArray* request,bool* isRequestOk=0);
        int parseResponse(QByteArray* response,bool* isResposeOk=0);

        bool parseRequestHeader(const QByteArray&);
        bool parseResponseHeader(const QByteArray&);

        const QByteArray dataToSend(bool sendToProxy = false)const;
        bool appendRequestBody(QByteArray* newData);

        int readChunk(int chunkSize,QByteArray* chunkData,QByteArray* unChunkDes);
        int getChunkSize(QByteArray* chunkedData);

        bool appendResponseBody(QByteArray* newData);
        /*
        //does not need this setters&getters just use public variable
        QString& host()const;
        quint16 port()const;
        QString& method()const;
        QString& path()const;
        QString& httpVersion()const;
        */
        int socketHandle;       // socket really handle
        quint64 socketConnectionId;//connection id in rythem
        QString host;
        QString virtualHost; // for header
        QString replacedHost;
        quint16 port;
        QString method;
        QString path;
        QString httpVersion;
        QString fullUrl;
        QString serverIp;
        quint64 id;
        quint64 number;// for view only

        QString responseStatus;
        QString responseResone;


        //for performance
        class PerformanceDateTime{
            public:
                PerformanceDateTime(){
                    reset();
                }
                PerformanceDateTime(const PerformanceDateTime& other){
                    setToOther(other);
                }
                PerformanceDateTime& operator =(const PerformanceDateTime& other){
                    if(&other == this){
                        return *this;
                    }
                    setToOther(other);
                    return *this;
                }

                void reset(){
                    clientConnected = -1;
                    requestBegin = -1;
                    requestDone = -1;
                    responseConnected = -1;
                    responseBegin = -1;
                    responseDone = -1;
                }

                qint64 clientConnected;
                qint64 requestBegin;
                qint64 requestDone;
                qint64 responseConnected;
                qint64 responseBegin;
                qint64 responseDone;
            private:
                void setToOther(const PerformanceDateTime& other){
                    if(other.clientConnected !=-1){
                        clientConnected = other.clientConnected;
                    }
                    if(other.requestBegin!=-1){
                        requestBegin = other.requestBegin;
                    }
                    if(other.requestDone!=-1){
                        requestDone = other.requestDone;
                    }
                    if(other.responseConnected!=-1){
                        responseConnected = other.responseConnected;
                    }
                    if(other.responseBegin!=-1){
                        responseBegin = other.responseBegin;
                    }
                    if(other.responseDone!=-1){
                        responseDone = other.responseDone;
                    }
                }

        };
        PerformanceDateTime performances;

        QString getRequestHeader(const QString& name)const;
        QString getResponseHeader(const QString& name)const;

        bool isMatchingRule;
        bool isContentReplaced;
        bool isImported;
        bool isConnectTunnel;
        int ruleType;

        QByteArray requestHeaderRawData()const{
            return _requestHeaderRawData;
        }
        QByteArray requestBodyRawData()const{
            return _requestBody;
        }

        QByteArray responseBodyRawData()const{
            return _responseBody;
        }
        QByteArray responseHeaderRawData()const{
            return _responseHeaderRawData;
        }
        QByteArray responseBodyRawDataUnChunked()const{
            return _responseUnChunked;
        }
        bool isContentLenthUnLimit()const{
            return _isContentLenthUnLimit;
        }
        bool isResponseChunked()const{
            return _isResponseChunked;
        }
        const QMap<QString,QString> requestHeaders()const{
            return _requestHeaders;
        }
        const QMap<QString,QString> responseHeaders()const{
            return _responseHeaders;
        }


    signals:

    public slots:

    private:
        QString _method;
        QString _httpVersion;
        QString _path;

        QByteArray _dataToSend;
        QByteArray _sigToSend;
        QByteArray _sigToSendForProxy;

        QMap<QString,QString> _requestHeaders;
        QByteArray _requestHeaderRawData;
        QByteArray _requestBody;
        int _requestBodyRemain;//request body remain bytes to receive

        QMap<QString,QString> _responseHeaders;
        QByteArray _responseHeaderRawData;
        QByteArray _responseBody;
        QByteArray _responseBodyUnCompressed;
        bool _isResponseChunked;
        bool _isContentLenthUnLimit;
        QByteArray _responseUnChunked;
        int _lastChunkSize;
        QByteArray _lastChunkSizeByteArray; //当数据未返回完整chunksize字段时，此处缓存前段

        int _responseBodyRemain;//response body remain bytes to receive

        void parseHeaders(const QList<QByteArray>& headers,QMap<QString,QString>*);



};

#endif // RYPIPEDATA_H

typedef QSharedPointer<RyPipeData> RyPipeData_ptr;
typedef QSharedPointer<RyPipeData::PerformanceDateTime> PerformanceDateTime_ptr;
