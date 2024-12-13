#ifndef MAPVIEWFRAME_H
#define MAPVIEWFRAME_H

#include <QFrame>


class MapViewFrame : public QFrame
{
public:
    MapViewFrame(QWidget*parent=nullptr);
protected:
    void resizeEvent(QResizeEvent*event)override;
};

#endif // MAPVIEWFRAME_H
