#include "pipedata.h"

PipeData::PipeData(int socketDescriptor):socketId(socketDescriptor){
}



void PipeData::setHeader(QByteArray name,QByteArray value){
    if(name == "Host"){
        int d = value.indexOf(":");
        if(d!=-1){
            allHeaders["Host"] = value.left(d);
            allHeaders["Port"] = value.mid(d+1);
        }else{
            allHeaders["Host"] = value;
            allHeaders["Port"] = "80";
        }
    }else{
        allHeaders[name] = value;
    }
}
const QByteArray PipeData::getHeader(QByteArray name)const{
    return allHeaders[name];
}
const QString PipeData::getBodyDecoded(){
    return "--";
}

