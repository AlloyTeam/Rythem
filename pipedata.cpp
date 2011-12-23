#include "pipedata.h"

PipeData::PipeData(){
}
void PipeData::setHeader(QString name,QString value){
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
const QString PipeData::getHeader(QString name)const{
    return allHeaders[name];
}

