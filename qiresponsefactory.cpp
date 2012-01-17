#include "qiresponsefactory.h"
#include "qiremoteresponseworker.h"

IQiResponseWorker *QiResponseFactory::getResponseWorker(const int descriptor, const QString &url, const int port){
	//TODO get destination from rule manager and return a worker to fetch the content
	//1. ask rule manager which rule fits argument url and port;
	//2. if a rule is returned, create a response worker according to that rule type;
	//3. set the worker's destination and return it;
	//4. if no rule is found, then we create a remote response worker to fetch the remote content
	IQiResponseWorker *worker = new QiRemoteResponseWorker();
	//we also have a QiLocalFileResponseWorker for reading local file content
	worker->setDestination(url, port);
	return worker;

	//TODO we need to reuse socket for the same socket descriptor!!
}
