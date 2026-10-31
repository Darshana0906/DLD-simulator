#ifndef REVERSEEQUATIONDIALOG_H
#define REVERSEEQUATIONDIALOG_H

#include <QDialog>
#include <QLabel>
#include <QTextEdit>
#include <QPushButton>
#include <QCheckBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QRectF>

class CircuitScene;

class ReverseEquationDialog : public QDialog {
    Q_OBJECT
public:
    explicit ReverseEquationDialog(CircuitScene *scene, QWidget *parent = nullptr);

signals:
    void circuitGenerated(const QRectF &bounds);

private slots:
    void onGenerateClicked();
    void onInputVarClicked();
    void onOutputVarClicked();
    void onResetCountersClicked();
    void onClearAllClicked();

private:
    void insertToken(const QString &text);
    QPushButton* createKeypadButton(const QString &text, const QString &tooltip, const QString &colorStyle);

    CircuitScene *m_scene = nullptr;

    QTextEdit *m_inputArea = nullptr;
    QPushButton *m_generateBtn = nullptr;
    QPushButton *m_closeBtn = nullptr;
    QCheckBox *m_clearCanvasCheck = nullptr;

    // Keypad auto-increment buttons
    QPushButton *m_inputBtn = nullptr;   // Starts "S1", increments to "S2", "S3"...
    QPushButton *m_outputBtn = nullptr;  // Starts "Y1", increments to "Y2", "Y3"...
    int m_sCounter = 1;
    int m_yCounter = 1;
};

#endif // REVERSEEQUATIONDIALOG_H
