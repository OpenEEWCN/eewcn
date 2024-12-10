#ifndef LISTHISTORYWIDGET_H
#define LISTHISTORYWIDGET_H

#include <QListWidget>


class ListHistoryWidget : public QListWidget
{
    Q_OBJECT
public:
    ListHistoryWidget(QWidget *parent = nullptr);
    void ResizeItems();
protected:
    void resizeEvent(QResizeEvent*event)override;
    void selectionChanged(const QItemSelection&/*selected*/,const QItemSelection&/*deselected*/)override;
};

#endif // LISTHISTORYWIDGET_H
