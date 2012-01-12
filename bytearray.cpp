#include "bytearray.h"

ByteArray::ByteArray(const QByteArray &bytes)
{
	rawData = new QByteArray(bytes);
	stream = new QDataStream(bytes);
	currentBytePosition = 0;
}

ByteArray::~ByteArray(){
	delete rawData;
	delete stream;
}

int ByteArray::skip(int len){
	int rs = stream->skipRawData(len);
	if(rs != -1) currentBytePosition += rs;
	return rs;
}

unsigned int ByteArray::bytesAvailable() const{
	return rawData->size() - currentBytePosition;
}

unsigned int ByteArray::position() const{
	return currentBytePosition;
}

int8_t ByteArray::readInt8(){
	int8_t rs;
	*stream >> rs;
	currentBytePosition += 1;
	return rs;
}
int16_t ByteArray::readInt16(){
	int16_t rs;
	*stream >> rs;
	currentBytePosition += 2;
	return rs;
}
int32_t ByteArray::readInt32(){
	int32_t rs;
	*stream >> rs;
	currentBytePosition += 4;
	return rs;
}
int64_t ByteArray::readInt64(){
	int64_t rs;
	*stream >> rs;
	currentBytePosition += 8;
	return rs;
}

uint8_t ByteArray::readUint8(){
	uint8_t rs;
	*stream >> rs;
	currentBytePosition += 1;
	return rs;
}
uint16_t ByteArray::readUint16(){
	uint16_t rs;
	*stream >> rs;
	currentBytePosition += 2;
	return rs;
}
uint32_t ByteArray::readUint32(){
	uint32_t rs;
	*stream >> rs;
	currentBytePosition += 4;
	return rs;
}
uint64_t ByteArray::readUint64(){
	uint64_t rs;
	*stream >> rs;
	currentBytePosition += 8;
	return rs;
}

int ByteArray::readBytes(char *s, int len){
	int rs = stream->readRawData(s, len);
	if(rs != -1){
		currentBytePosition += rs;
	}
	return rs;
}

int ByteArray::readCString(char *s, int len){
	int rs = stream->readRawData(s, len);
	if(rs != -1){
		s[len] = '\0';
		currentBytePosition += rs;
	}
	return rs;
}
