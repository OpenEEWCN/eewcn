#include "mapviewframe.h"
#include <QQuickWidget>
#include <QQuickItem>

MapViewFrame::MapViewFrame(QWidget*parent):QFrame(parent)
{
}

void MapViewFrame::resizeEvent(QResizeEvent*event)
{
    QFrame::resizeEvent(event);
    QQuickWidget*w=findChild<QQuickWidget*>("quickMapView");
    w->setGeometry(0,0,width(),height());
    QMetaObject::invokeMethod(w->rootObject(),"refreshMarks");
}
