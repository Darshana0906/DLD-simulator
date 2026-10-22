#ifndef EQUATIONDIALOG_H
#define EQUATIONDIALOG_H

#include <QDialog>
#include <QLabel>
#include <QTextEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include "logic/EquationGenerator.h"

class CircuitAdapter;

class EquationDialog : public QDialog {
    Q_OBJECT
public:
    explicit EquationDialog(CircuitAdapter *adapter, QWidget *parent = nullptr);

public slots:
    void refresh();

private:
    CircuitAdapter *m_adapter = nullptr;

    QLabel *m_statusBanner = nullptr;
    QTextEdit *m_contentArea = nullptr;
    QPushButton *m_copyBtn = nullptr;
    QPushButton *m_refreshBtn = nullptr;
    QPushButton *m_closeBtn = nullptr;

    QString m_lastCopiedText;

    void updateUI(const EquationReport &report);
};

#endif // EQUATIONDIALOG_H

