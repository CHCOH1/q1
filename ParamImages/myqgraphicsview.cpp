#include "myqgraphicsview.h"

MyQGraphicsView::MyQGraphicsView(QWidget *parent) :
 QGraphicsView(parent)
{
}

void MyQGraphicsView::mousePressEvent(QMouseEvent * event)
{
    if(event->button() == Qt::LeftButton)
    {
        emit clickEvent(event->x(), event->y());
    }
}
