#include "UndoManager.h"
#include "CircuitScene.h"
#include "CircuitSerializer.h"

UndoManager::UndoManager(CircuitScene *scene, QObject *parent)
    : QObject(parent), m_scene(scene) {
    if (m_scene) {
        resetInitialState();
    }
}

void UndoManager::resetInitialState() {
    m_undoStack.clear();
    m_redoStack.clear();
    if (m_scene) {
        QByteArray initial = CircuitSerializer::serializeToJson(m_scene);
        m_undoStack.append(initial);
    }
    emit canUndoChanged(false);
    emit canRedoChanged(false);
}

void UndoManager::clear() {
    resetInitialState();
}

bool UndoManager::canUndo() const {
    return m_undoStack.size() > 1;
}

bool UndoManager::canRedo() const {
    return !m_redoStack.isEmpty();
}

void UndoManager::pushState(const QString &actionName) {
    Q_UNUSED(actionName);
    if (!m_scene || m_isPerformingUndoRedo)
        return;

    QByteArray state = CircuitSerializer::serializeToJson(m_scene);
    if (!m_undoStack.isEmpty() && m_undoStack.last() == state) {
        // No meaningful difference from previous state
        return;
    }

    m_undoStack.append(state);
    if (m_undoStack.size() > kMaxHistory) {
        m_undoStack.removeFirst();
    }
    m_redoStack.clear();

    emit canUndoChanged(canUndo());
    emit canRedoChanged(canRedo());
}

void UndoManager::undo() {
    if (!canUndo() || !m_scene || m_isPerformingUndoRedo)
        return;

    m_isPerformingUndoRedo = true;

    QByteArray currentState = m_undoStack.takeLast();
    m_redoStack.append(currentState);

    QByteArray prevState = m_undoStack.last();
    CircuitSerializer::deserializeFromJson(prevState, m_scene);

    m_isPerformingUndoRedo = false;

    emit canUndoChanged(canUndo());
    emit canRedoChanged(canRedo());
    emit stateRestored();
}

void UndoManager::redo() {
    if (!canRedo() || !m_scene || m_isPerformingUndoRedo)
        return;

    m_isPerformingUndoRedo = true;

    QByteArray nextState = m_redoStack.takeLast();
    m_undoStack.append(nextState);

    CircuitSerializer::deserializeFromJson(nextState, m_scene);

    m_isPerformingUndoRedo = false;

    emit canUndoChanged(canUndo());
    emit canRedoChanged(canRedo());
    emit stateRestored();
}

