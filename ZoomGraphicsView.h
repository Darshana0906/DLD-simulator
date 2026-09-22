#ifndef ZOOMGRAPHICSVIEW_H
#define ZOOMGRAPHICSVIEW_H

#include <QGraphicsView>

class ZoomGraphicsView : public QGraphicsView
{
public:
    explicit ZoomGraphicsView(QWidget *parent = nullptr);

protected:
    void wheelEvent(QWheelEvent *event) override;
};

#endif