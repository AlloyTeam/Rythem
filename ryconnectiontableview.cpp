#include "ryconnectiontableview.h"
#include <QTemporaryFile>
#include "quazip/quazip.h"
#include "quazip/quazipfile.h"
#include "rytablemodel.h"

extern QString appPath;

RyConnectionTableView::RyConnectionTableView(QWidget *parent) :
    QTableView(parent){

    createMenu();
}

void RyConnectionTableView::contextMenuEvent(QContextMenuEvent *event){
    //QModelIndex index = this->indexAt(event->pos());
    if(this->selectionModel()->selection().length() > 0){
        _saveSelectedSessionAct->setEnabled(true);
        _removeSelectedSessionAct->setEnabled(true);
    }else{
        _saveSelectedSessionAct->setEnabled(false);
        _removeSelectedSessionAct->setEnabled(false);
    }
    _contextMenu->popup(event->globalPos());
    event->accept();
}

void RyConnectionTableView::createMenu(){
    _contextMenu = new QMenu(this);
    _saveSelectedSessionAct = _contextMenu->addAction(tr("save selected session"));

    _autoScrollAct = _contextMenu->addAction(tr("auto scroll session list"));
    _autoScrollAct->setCheckable(true);

    _saveMenu = _contextMenu->addMenu(tr("&save"));
    _saveAllSessionAct = _saveMenu->addAction(tr("&save all sessions"));
    _saveSessionsRevertAct = _saveMenu->addAction(tr("save sessions unselected"));

    _removeMenu = _contextMenu->addMenu(tr("&remove"));
    _removeSelectedSessionAct = _removeMenu->addAction(tr("remove selected session"));
    _removeAllSessionAct = _removeMenu->addAction(tr("remove all sessions"));
    _removeSessionsRevertAct = _removeMenu->addAction(tr("remove sessions unselected"));


    _saveSelectedSessionAct->connect(_saveSelectedSessionAct,SIGNAL(triggered()),this,SLOT(onAction()));

}

void RyConnectionTableView::onAction(){
    QAction *action = qobject_cast<QAction*>(sender());
    if(0 == action){
        return;
    }
    if(action==_saveSelectedSessionAct){
        QTemporaryFile file;

        if(!file.open()){
            return;
        }
        QuaZip zip(appPath+"/../../../../test.zip");
        zip.open(QuaZip::mdCreate);

        RyTableModel *model = qobject_cast<RyTableModel*>(this->model());
        QItemSelection selection = this->selectionModel()->selection();

        int index=0;
        for(int j=0;j<selection.length();++j){
            QItemSelectionRange r = selection.at(j);
            int c = r.top();
            //for(int c=r.top();c<r.bottom();++c){
                index++;
                RyPipeData_ptr item = model->getItem(c);
                QByteArray baRequest = item->requestHeaderRawData();
                baRequest.append("\r\n\r\n");
                baRequest.append(item->requestBodyRawData());
                QuaZipFile outFileReq(&zip);
                if(!outFileReq.open(QIODevice::WriteOnly,
                                 QuaZipNewInfo(
                                     QString("./raw/%1_c.txt").arg(QString::number(index)))
                                 )){
                    qDebug()<<"error on write zip:"<<QString::number(zip.getZipError());
                    return;
                }
                outFileReq.write(baRequest);
                outFileReq.close();

                QByteArray baResponse = item->responseHeaderRawData();
                baResponse.append("\r\n\r\n");
                baResponse.append(item->responseBodyRawData());
                QuaZipFile outFileRes(&zip);
                if(!outFileRes.open(QIODevice::WriteOnly,
                                 QuaZipNewInfo(
                                     QString("./raw/%1_s.txt").arg(QString::number(index)))
                                 )){
                    qDebug()<<"error on write zip:"<<QString::number(zip.getZipError());
                    return;
                }
                outFileRes.write(baResponse);
                outFileRes.close();
            //}

        }
        zip.close();
    }else{

    }

}


