#ifndef UNDOMANAGER_H
#define UNDOMANAGER_H

#include <QObject>
#include <QList>
#include <QByteArray>
#include <QString>

class CircuitScene;

/**
 * UndoManager
 *
 * Manages full undo/redo state history for the circuit simulator canvas.
 * - Snapshots the circuit state (components, pins, wires, input count, switch states)
 * - Restores state accurately via CircuitSerializer
 * - Emits signals when canUndo / canRedo status changes to update UI action buttons
 */
class UndoManager : public QObject {
    Q_OBJECT
public:
    explicit UndoManager(CircuitScene *scene, QObject *parent = nullptr);

    bool canUndo() const;
    bool canRedo() const;

    void resetInitialState();
    void clear();

    bool isPerformingUndoRedo() const { return m_isPerformingUndoRedo; }

public slots:
    void pushState(const QString &actionName = QString());
    void undo();
    void redo();

signals:
    void canUndoChanged(bool canUndo);
    void canRedoChanged(bool canRedo);
    void stateRestored();

private:
    CircuitScene *m_scene;
    QList<QByteArray> m_undoStack;
    QList<QByteArray> m_redoStack;
    bool m_isPerformingUndoRedo = false;
    static constexpr int kMaxHistory = 60;
};

#endif // UNDOMANAGER_H

