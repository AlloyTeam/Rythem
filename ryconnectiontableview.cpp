#include "ryconnectiontableview.h"
#include <QTemporaryFile>
#include "quazip/quazip.h"
#include "quazip/quazipfile.h"
#include "rytablemodel.h"

#include "rymimedata.h"

extern QString appPath;
extern QByteArray gzipDecompress(QByteArray data);

RyConnectionTableView::RyConnectionTableView(QWidget *parent) :
    QTableView(parent){
    _isMouseDown = false;
    this->setDragEnabled(true);
    createMenu();
}

void RyConnectionTableView::contextMenuEvent(QContextMenuEvent *event){
    //QModelIndex index = this->indexAt(event->pos());
    if(this->selectionModel()->selection().length() > 0){
        _saveSelectedSessionAct->setEnabled(true);
        _removeSelectedSessionAct->setEnabled(true);
        _saveSessionRespnoseBodyAct->setEnabled(true);
    }else{
        _saveSelectedSessionAct->setEnabled(false);
        _removeSelectedSessionAct->setEnabled(false);
        _saveSessionRespnoseBodyAct->setEnabled(false);
    }
    _contextMenu->popup(event->globalPos());
    event->accept();
}

void RyConnectionTableView::createMenu(){
    _contextMenu = new QMenu(this);
    _saveSelectedSessionAct = _contextMenu->addAction(tr("save selected session..."));
    _saveSessionRespnoseBodyAct = _contextMenu->addAction(tr("&save response body..."));

    _autoScrollAct = _contextMenu->addAction(tr("auto scroll session list"));
    _autoScrollAct->setCheckable(true);

    _saveMenu = _contextMenu->addMenu(tr("&save"));
    _saveAllSessionAct = _saveMenu->addAction(tr("&save all sessions"));
    _saveSessionsRevertAct = _saveMenu->addAction(tr("save sessions unselected"));

    _removeMenu = _contextMenu->addMenu(tr("&remove"));
    _removeSelectedSessionAct = _removeMenu->addAction(tr("remove selected session"));
    _removeAllSessionAct = _removeMenu->addAction(tr("remove all sessions"));
    _removeSessionsRevertAct = _removeMenu->addAction(tr("remove sessions unselected"));



    _contextMenu->connect(_contextMenu,SIGNAL(triggered(QAction*)),this,SLOT(onAction(QAction*)));

}

void RyConnectionTableView::mousePressEvent(QMouseEvent *event){
    _isMouseDown = true;
    QTableView::mousePressEvent(event);
}
void RyConnectionTableView::mouseReleaseEvent(QMouseEvent *event){
    _isMouseDown = false;
    QTableView::mouseReleaseEvent(event);
}

void RyConnectionTableView::mouseMoveEvent(QMouseEvent *event){
    if(!_isMouseDown){
        QTableView::mouseMoveEvent(event);
        return;
    }
    QModelIndex index = this->indexAt(event->pos());
    if (!index.isValid()){
        return;
    }
    QPoint hotSpot = event->pos() - this->pos();
    RyTableModel* model = qobject_cast<RyTableModel*>(this->model());
    RyPipeData_ptr data = model->getItem(index.row());
    RyMimeData *mimeData = new RyMimeData(data);
    mimeData->setText(data->getRequestHeader("Host"));
    QDrag *drag = new QDrag(this);
    drag->setMimeData(mimeData);
    //drag->setPixmap(pixmap);
    drag->setHotSpot(hotSpot);
    drag->exec(Qt::CopyAction | Qt::MoveAction, Qt::CopyAction);
}


void RyConnectionTableView::onAction(QAction *action){
    if(0 == action){
        return;
    }
    if(action==_saveSelectedSessionAct){

        QFileDialog dialog;
        QString saveFileName = dialog.getSaveFileName(this);

        QuaZip zip(saveFileName);
        zip.open(QuaZip::mdCreate);

        RyTableModel *model = qobject_cast<RyTableModel*>(this->model());
        QItemSelection selection = this->selectionModel()->selection();

        int index=0;
        for(int j=0;j<selection.length();++j){
            QItemSelectionRange r = selection.at(j);
            for(int c=r.top();c<=r.bottom();++c){
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
            }

        }
        zip.close();
    }else if(action == _saveSessionRespnoseBodyAct){

        QFileDialog dialog;
        QString saveDir = dialog.getExistingDirectory(this,tr("save session response body to..."));
        if(saveDir.isEmpty()){
            return;
        }
        QDir dir(saveDir);

        RyTableModel *model = qobject_cast<RyTableModel*>(this->model());
        QItemSelection selection = this->selectionModel()->selection();
        int index=0;
        for(int j=0;j<selection.length();++j){
            QItemSelectionRange r = selection.at(j);
            for(int c=r.top();c<=r.bottom();++c){
                index++;
                RyPipeData_ptr item = model->getItem(c);
                QByteArray baResponse;
                if(item->isResponseChunked()){
                    baResponse = item->responseBodyRawDataUnChunked();
                }else{
                    baResponse = item->responseBodyRawData();
                }
                bool isEncrypted = !item->getResponseHeader("Content-Encoding").isEmpty();
                if(isEncrypted){
                    baResponse = gzipDecompress(baResponse);
                }

                QString fileName = "/"+item->host+item->path;

                if(fileName.endsWith("/")){
                    fileName.append("index.html");
                }
                fileName.prepend(saveDir);
                QFile saveFile(fileName);
                QFileInfo fi(saveFile);
                QString dirPath = fi.absolutePath();
                dir.mkpath(dirPath);
                if(saveFile.exists()){
                    //todo
                    qDebug()<<"exits"<<fileName;
                }
                //qDebug()<<fileName;
                saveFile.open(QIODevice::WriteOnly);
                saveFile.write(baResponse);
                saveFile.close();
            }
        }
    }

}


