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
	connect(this, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(onError(QAbstractSocket::SocketError)));
	connect(this, SIGNAL(readyRead()), this, SLOT(onReadyRead()));
	qDebug() << "[WebSocketClient] connected";
}

WebSocketClient::~WebSocketClient(){
	delete buffer;
}

void WebSocketClient::sendMessage(const char *message){
	this->write(generateMessagePackage(message));
	this->flush();
}
void WebSocketClient::sendMessage(const QByteArray &message){
	this->write(generateMessagePackage(message.data()));
	this->flush();
}

void WebSocketClient::onDisconnected(){
	qDebug() << "disconnected";
	emit finished();
}

void WebSocketClient::onError(QAbstractSocket::SocketError socketError){
	qWarning() << "error" << socketError;
}

void WebSocketClient::onReadyRead(){
	QByteArray request = this->readAll();
	buffer->append(request);
	processReceivedData();
}

void WebSocketClient::processReceivedData(){
	if(handshaked){
		//process data frame
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
				this->close();
				return;

			default:
				bool mask = buffer->at(1) >> 7; //whether payload data is masked
				uint64_t payloadLen = buffer->at(1) & 0x7F; //0-125:payload length, 126:read the next 2bytes as payload length, 127:read the next 8bytes as payload length
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
		if(buffer->indexOf("\r\n\r\n") != -1){
			//handshake request received, process the header
			qDebug() << "[WebSocketClient] handshaking ...";
			QString handshakeRequest(*buffer);
			QStringList components = handshakeRequest.split("\r\n");
			QMap<QString, QString> header;
			for(int i=1; i<components.length(); i++){
				if(components.at(i).length()){
					QStringList headerKV = components.at(i).split(": ");
					header.insert(headerKV.at(0), headerKV.at(1));
				}
			}
			if(!header.contains("Upgrade") || header.value("Upgrade") != "websocket"){
				qWarning() << "[WebSocketClient] INVAILD HANDSHAKE REQUEST";
				this->close();
			}
			else{
				QByteArray hash = generateAcceptHash(header.value("Sec-WebSocket-Key"));
				sendHandshakeResponse(hash);
				handshaked = true;
				qDebug() << "[WebSocketClient] handshake complete";
			}
			buffer->clear();
		}
	}
}

QByteArray WebSocketClient::generateAcceptHash(const QString &key) const{
	QByteArray input;
	input.append(key);
	input.append(MAGIC_TOKEN);
	QByteArray sha1 = QCryptographicHash::hash(input, QCryptographicHash::Sha1);
	return sha1.toBase64();
}

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

void WebSocketClient::sendHandshakeResponse(const QByteArray &acceptHash){
	this->write("HTTP/1.1 101 Switching Protocols\r\n");
	this->write("Upgrade: websocket\r\n");
	this->write("Connection: Upgrade\r\n");
	this->write("Sec-WebSocket-Accept: ");
	this->write(acceptHash);
	this->write("\r\n\r\n");
	this->flush();
}
