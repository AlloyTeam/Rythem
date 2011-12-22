#include "pipedata.h"

PipeData::PipeData(){
}
void PipeData::setHeader(QString name,QString value){
    allHeaders[name] = value;
}
const QString PipeData::getHeader(QString name)const{
    return allHeaders[name];
}
