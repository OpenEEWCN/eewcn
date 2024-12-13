#include "listhistorywidget.h"
#include "earthquakelistentryform.h"

ListHistoryWidget::ListHistoryWidget(QWidget *parent):
    QListWidget(parent)
{
}

void ListHistoryWidget::resizeEvent(QResizeEvent *event)
{
    QListWidget::resizeEvent(event);
    ResizeItems();
}

void ListHistoryWidget::ResizeItems()
{
    for(int i=0;i<count();i++){
        QListWidgetItem*wi=item(i);
        EarthquakeListEntryForm*w=(EarthquakeListEntryForm*)itemWidget(wi);
        wi->setSizeHint(QSize(viewportSizeHint().width(),1));
        if(w){//若启动时缩小窗口会发现是nullptr
            w->ExpandHeight();
            wi->setSizeHint(QSize(viewportSizeHint().width(),w->size().height()));
        }
    }
}

void ListHistoryWidget::selectionChanged(const QItemSelection &/*selected*/, const QItemSelection &/*deselected*/)
{
    QListWidgetItem*w=item(currentRow());
    if(w)
        ((EarthquakeListEntryForm*)itemWidget(w))->SetMapCenter();
}
