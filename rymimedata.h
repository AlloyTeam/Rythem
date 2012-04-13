#ifndef RYMIMEDATA_H
#define RYMIMEDATA_H
#include <QtCore>
#include <proxy/rypipedata.h>
#include <QMimeData>

class RyMimeData:public QMimeData{
public:
    RyMimeData(RyPipeData_ptr data):QMimeData(){
        _data = data;
    }
    RyPipeData_ptr pipeData()const{
        return _data;
    }
private:
    RyPipeData_ptr _data;
};

#endif // RYMIMEDATA_H
