#ifndef ZOOMGRAPHICSVIEW_H
#define ZOOMGRAPHICSVIEW_H

#include <QGraphicsView>
#include <QEnterEvent>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QKeyEvent>
#include <QGestureEvent>

class CircuitScene;

class ZoomGraphicsView : public QGraphicsView
{
    Q_OBJECT
public:
    explicit ZoomGraphicsView(QWidget *parent = nullptr);

    // Call this after setting the scene so the view can enforce cursor
    void setCircuitScene(CircuitScene *scene);

    // Apply the correct cursor right now based on state.
    void applyCursor();

protected:
    bool viewportEvent(QEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void enterEvent(QEnterEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void keyReleaseEvent(QKeyEvent *event) override;

private:
    CircuitScene *m_circuitScene = nullptr;
    double m_zoomLevel = 1.0;   // Tracks absolute zoom level

    bool m_isPanning = false;
    bool m_spacePressed = false;
    QPoint m_panStartPos;

    static constexpr double kZoomMin = 0.1;   // 10%
    static constexpr double kZoomMax = 5.0;   // 500%
};

#endif // ZOOMGRAPHICSVIEW_H