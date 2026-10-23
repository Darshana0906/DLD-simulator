#ifndef TRUTHTABLEDIALOG_H
#define TRUTHTABLEDIALOG_H

#include <QDialog>
#include <QLabel>
#include <QTableWidget>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QVector>
#include <QList>

class CircuitAdapter;
class Switch;
class LED;

class TruthTableDialog : public QDialog {
    Q_OBJECT
public:
    explicit TruthTableDialog(CircuitAdapter *adapter, QWidget *parent = nullptr);
    ~TruthTableDialog() override = default;

public slots:
    void refresh();
    void syncHighlightFromCircuit();

private slots:
    void onRowClicked(int row, int col);
    void copyToClipboard();
    void saveAsPng();

private:
    void setupUI();
    void setupConnections();
    void updateTableData();

    CircuitAdapter *m_adapter = nullptr;

    QLabel *m_statsLabel = nullptr;
    QLabel *m_noteLabel = nullptr;
    QLabel *m_tipLabel = nullptr;
    QTableWidget *m_tableWidget = nullptr;
    QLabel *m_activeStateLabel = nullptr;
    QPushButton *m_refreshBtn = nullptr;
    QPushButton *m_copyBtn = nullptr;
    QPushButton *m_savePngBtn = nullptr;
    QPushButton *m_closeBtn = nullptr;

    QList<Switch*> m_switches;
    QList<LED*> m_leds;
    QVector<int> m_switchNodeIds;
    QVector<int> m_ledNodeIds;

    int m_totalRows = 0;
    bool m_isSyncing = false;
};

#endif // TRUTHTABLEDIALOG_H
