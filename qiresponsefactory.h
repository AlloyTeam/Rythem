#ifndef QIRESPONSEFACTORY_H
#define QIRESPONSEFACTORY_H

#include <QString>
#include "iqiresponseworker.h"

class QiResponseFactory
{
public:
	static IQiResponseWorker getResponseWorker(QString &fullPath);
};

#endif // QIRESPONSEFACTORY_H
