#include <string>
#include "websocketclient.h"
#include "bytearray.h"

#define MAGIC_TOKEN "258EAFA5-E914-47DA-95CA-C5AB0DC85B11"

enum{
	DATAFRAME_OPCODE_CONTINUE = 0x0,
	DATAFRAME_OPCODE_TEXT = 0x1,
	DATAFRAME_OPCODE_BINARY = 0x2,
	DATAFRAME_OPCODE_CONNECTION_LOST = 0x8,
	DATAFRAME_OPCODE_PING = 0x9,
	DATAFRAME_OPCODE_PONG = 0xA
};

WebSocketClient::WebSocketClient(QObject *parent) :
	QTcpSocket(parent)
{
	handshaked = false;
	buffer = new QByteArray();
	connect(this, SIGNAL(disconnected()), this, SLOT(onDisconnected()));
	connect(this, SIGNAL(aboutToClose()), this, SLOT(onDisconnected()));
	connect(this, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(onError(QAbstractSocket::SocketError)));
	connect(this, SIGNAL(readyRead()), this, SLOT(onReadyRead()));
	qDebug() << "[WebSocketClient] connected";
}

WebSocketClient::~WebSocketClient(){
	delete buffer;
}

//send message to client
void WebSocketClient::sendMessage(const char *message){
	this->write(generateMessagePackage(message));
	this->flush();
}
void WebSocketClient::sendMessage(const QByteArray &message){
	this->write(generateMessagePackage(message.data()));
	this->flush();
}

//emit finished signal when websocket is disconnected
void WebSocketClient::onDisconnected(){
	qDebug() << "disconnected";
	emit finished();
}

//websocket error handler
void WebSocketClient::onError(QAbstractSocket::SocketError socketError){
	qWarning() << "error" << socketError;
}

//read all received data and process them
void WebSocketClient::onReadyRead(){
	QByteArray request = this->readAll();
	buffer->append(request);
	processReceivedData();
}

//process received data, handshake with client or decode data frame
void WebSocketClient::processReceivedData(){
    /*
    http://blog.csdn.net/fenglibing/article/details/6699154
    判断新旧版本可以通过是否包含“Sec-WebSocket-Key1”和“Sec-WebSocket-Key2”
    */
	if(handshaked){
		//process data frame
		//TODO 這裡要根據不同websocket協議版本,用不同方式讀取內容
		if(buffer->size() >= 2){
			//TODO what if FIN(the first bit) is 1? we need to merge multiple package!
			//get opcode, mask, and payload length
			uint8_t opcode = buffer->at(0) & 0x0F; //interpretation of the payload data
			switch(opcode){
			case DATAFRAME_OPCODE_PING:
				//TODO response a PONG frame
				buffer->clear();
				return;

			case DATAFRAME_OPCODE_PONG:
				//no action is required when received a pong frame
				buffer->clear();
				return;

			case DATAFRAME_OPCODE_CONNECTION_LOST:
				//close the underlying socket when received a close frame
				this->abort();
				return;

			default:
				//decode data frame and emit a message signal when a complete data frame is received
				bool mask = buffer->at(1) >> 7;
				 //0-125:payload length
				//126:read the next 2bytes as payload length
				//127:read the next 8bytes as payload length
				uint64_t payloadLen = buffer->at(1) & 0x7F;
				ByteArray stream(*buffer);
				stream.skip(2);

				//get extended payload length
				if(payloadLen == 126){
					if(buffer->size() >= 4){
						payloadLen = stream.readInt16();
					}
					else return;//keep waiting for more data
				}
				else if(payloadLen == 127){
					if(buffer->size() >= 10){
						payloadLen = stream.readInt64();
					}
					else return;//keep waiting for more data
				}

				//get masking key
				char maskingKey[4];
				if(mask){
					if(stream.bytesAvailable() >= 4) stream.readBytes(maskingKey, 4);
					else return;
				}

				//get payload content
				char payload[payloadLen + 1]; //+1 for the ending \0
				if(stream.bytesAvailable() >= payloadLen) stream.readCString(payload, payloadLen);
				else return;

				//unmask the payload if masking key exist
				if(mask){
					for(unsigned int i=0; i<payloadLen; i++){
						payload[i] = payload[i] ^ (maskingKey[i % 4]);
					}
				}
				qDebug() << "payload:" << payload;
				emit message(QByteArray(payload));
				buffer->clear();
			}
		}
	}
	else{
		//process handshake header

		// 舊版websocket協議
        QString randomStr;

		if(buffer->indexOf("\r\n\r\n") != -1){
			qDebug() << "[WebSocketClient] handshaking ...";
			//format header fields
            QString handshakeRequest(*buffer);
            qDebug()<<*buffer;
            QStringList components = handshakeRequest.split("\r\n");
			QMap<QString, QString> header;
			for(int i=1; i<components.length(); i++){
				if(components.at(i).length()){
					QStringList headerKV = components.at(i).split(": ");
                    if(headerKV.size()<2){
                        randomStr = headerKV.at(0);
                        continue;
                    }
                    qDebug()<<headerKV.at(0);
					header.insert(headerKV.at(0), headerKV.at(1));
				}
			}
			//close the underlying socket if this is not a valid websocket handshake request
            if(!header.contains("Upgrade") || header.value("Upgrade").toLower() != "websocket"){
				qWarning() << "[WebSocketClient] INVALID HANDSHAKE REQUEST";
				this->abort();
			}
			//otherwise generate an accept hash base on the Sec-WebSocket-Key and send
			//the handshake response back to client to finish the handshake process
            else{
                QByteArray hash;
                if(!randomStr.isEmpty()){
                    QString str1 = header.value("Sec-WebSocket-Key1");
                    QString str2 = header.value("Sec-WebSocket-Key2");
                    hash = generateAcceptHash(str1,str2,randomStr);
                    sendHandshakeResponse(hash,true);
                }else{
                    hash = generateAcceptHash(header.value("Sec-WebSocket-Key"));
                    sendHandshakeResponse(hash);
                }
				handshaked = true;
				qDebug() << "[WebSocketClient] handshake complete";
			}
			buffer->clear();
		}
	}
}

//accept hash = base64(sha1(key + MAGIC_TOKEN))
QByteArray WebSocketClient::generateAcceptHash(const QString &key) const{
	QByteArray input;
	input.append(key);
	input.append(MAGIC_TOKEN);
	QByteArray sha1 = QCryptographicHash::hash(input, QCryptographicHash::Sha1);
	return sha1.toBase64();
}
QByteArray WebSocketClient::generateAcceptHash(QString key1,QString key2,QString str){
    QByteArray input;

    long long kDNum1 = key1.count(' ');
    QString str2 = key1.remove(QRegExp("[^\\d]*"));
    qDebug()<<"str1="<<str2;
    long long kNum1 = str2.toLongLong();
    qDebug()<<kNum1<<kDNum1;

    long long kDNum2 = key2.count(' ');
    str2 = key2.remove(QRegExp("[^\\d]*"));
    qDebug()<<"str2="<<str2;
    long long kNum2 = str2.toLongLong();
    qDebug()<<kNum2<<kDNum2;
    QString str3("%1%2%3");
    str3 = str3.arg(kNum1/kDNum1).arg(kNum2/kDNum2).arg(str);

    input.append(str3);

    qDebug()<<input;

    QByteArray md5 = QCryptographicHash::hash(input,QCryptographicHash::Md5);
    qDebug()<<md5;
    return md5;
}

//create a data frame with the specify message content
QByteArray WebSocketClient::generateMessagePackage(const char *message, bool){
	QByteArray bytes;
	QDataStream stream(&bytes, QIODevice::WriteOnly);

	//FIN=1, RSV1/2/3=0, opcode=1
	stream << (uint8_t)(DATAFRAME_OPCODE_TEXT | 0x80);

	//MASK=0, payloadLen
	uint64_t payloadLen = strlen(message);
	if(payloadLen < 126){
		stream << (uint8_t)payloadLen;
	}
	else if(payloadLen == 126){
		stream << (uint8_t)126 << (uint16_t)payloadLen;
	}
	else if(payloadLen == 127){
		stream << (uint8_t)127 << payloadLen;
	}

	//payload
	stream.writeRawData(message, strlen(message));
	return bytes;
}

void WebSocketClient::sendHandshakeResponse(const QByteArray &acceptHash,bool isNewProtocol){
/* new protocol should return something like this
HTTP/1.1 101 WebSocket Protocol Handshake
Upgrade: WebSocket
Connection: Upgrade
Sec-WebSocket-Origin: http://example.com
Sec-WebSocket-Location: ws://example.com/demo
Sec-WebSocket-Protocol: sample
8jKS’y:G*Co,Wxa-
*/
    if(isNewProtocol){
        this->write("HTTP/1.1 101 Switching Protocols\r\n");
        this->write("Upgrade: WebSocket\r\n");
        this->write("Connection: Upgrade\r\n");
        this->write(acceptHash);
        this->write("\r\n\r\n");
        this->flush();
    }else{
        this->write("HTTP/1.1 101 Switching Protocols\r\n");
        this->write("Upgrade: websocket\r\n");
        this->write("Connection: Upgrade\r\n");
        this->write("Sec-WebSocket-Accept: ");
        this->write(acceptHash);
        this->write("\r\n\r\n");
        this->flush();
    }
}
