#ifndef MYQGRAPHICSVIEW_H
#define MYQGRAPHICSVIEW_H

#include <QGraphicsView>
#include <QMouseEvent>

class MyQGraphicsView : public QGraphicsView
{
   Q_OBJECT
public:
   explicit MyQGraphicsView(QWidget *parent = 0);

signals:
   void clickEvent(int x, int y);

public slots:
   void mousePressEvent(QMouseEvent * e);
private:
};

#endif // MYQGRAPHICSVIEW_H
