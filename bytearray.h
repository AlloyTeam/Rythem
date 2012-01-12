#ifndef BINARYSTREAM_H
#define BINARYSTREAM_H

#include <QtCore>

class ByteArray
{
public:
	ByteArray(const QByteArray &bytes);
	~ByteArray();
	int skip(int len);
	unsigned int bytesAvailable() const;
	unsigned int position() const;

	int8_t readInt8();
	int16_t readInt16();
	int32_t readInt32();
	int64_t readInt64();
	uint8_t readUint8();
	uint16_t readUint16();
	uint32_t readUint32();
	uint64_t readUint64();

	int readBytes(char *s, int len);
	int readCString(char *s, int len);

protected:
	QByteArray *rawData;
	QDataStream *stream;
	unsigned int currentBytePosition;

};

#endif // BINARYSTREAM_H
