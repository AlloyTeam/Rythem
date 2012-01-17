#ifndef QIRESPONSEFACTORY_H
#define QIRESPONSEFACTORY_H

#include <QString>
#include "iqiresponseworker.h"

class QiResponseFactory
{
public:
	static IQiResponseWorker *getResponseWorker(const int descriptor, const QString &url, const int port = 80);
};

#endif // QIRESPONSEFACTORY_H
