#include "TruthTableDialog.h"
#include "logic/CircuitAdapter.h"
#include "logic/Node.h"
#include "elements/Switch.h"
#include "elements/LED.h"

#include <QGuiApplication>
#include <QClipboard>
#include <QHeaderView>
#include <QScrollBar>
#include <QFileDialog>
#include <QPainter>
#include <QMessageBox>
#include <QTimer>
#include <QCollator>
#include <algorithm>
#include <cmath>

TruthTableDialog::TruthTableDialog(CircuitAdapter *adapter, QWidget *parent)
    : QDialog(parent), m_adapter(adapter) {
    setWindowTitle("Truth Table");
    setMinimumSize(560, 400);
    resize(620, 480);

    setupUI();
    setupConnections();
    refresh();
}

void TruthTableDialog::setupUI() {
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(8);
    mainLayout->setContentsMargins(14, 14, 14, 14);

    // 1. Statistics Bar (clean neutral summary, replacing the green banner)
    m_statsLabel = new QLabel(this);
    m_statsLabel->setText("Number of Inputs = 0   |   Number of Outputs = 0   |   Total Rows = 0");
    m_statsLabel->setStyleSheet(
        "QLabel { "
        "  background-color: #F8FAFC; "
        "  color: #1E293B; "
        "  border: 1px solid #CBD5E1; "
        "  border-radius: 6px; "
        "  padding: 8px 14px; "
        "  font-size: 13px; "
        "  font-weight: 600; "
        "}");
    mainLayout->addWidget(m_statsLabel);

    // Subtle note label (visible only when cycle detected or inputs capped)
    m_noteLabel = new QLabel(this);
    m_noteLabel->setStyleSheet("QLabel { color: #B45309; font-size: 11px; padding-left: 2px; }");
    m_noteLabel->setVisible(false);
    mainLayout->addWidget(m_noteLabel);

    // 2. Interactive Tip Label
    m_tipLabel = new QLabel("💡 Click any row below to apply its input combination to the circuit.", this);
    m_tipLabel->setStyleSheet("QLabel { color: #475569; font-size: 11px; padding-left: 2px; }");
    mainLayout->addWidget(m_tipLabel);

    // 3. Truth Table Widget
    m_tableWidget = new QTableWidget(this);
    m_tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_tableWidget->setSelectionMode(QAbstractItemView::SingleSelection);
    m_tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_tableWidget->setAlternatingRowColors(true);
    m_tableWidget->verticalHeader()->setDefaultSectionSize(28);
    m_tableWidget->horizontalHeader()->setStretchLastSection(true);
    m_tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    m_tableWidget->setStyleSheet(
        "QTableWidget { "
        "  background-color: #FFFFFF; "
        "  alternate-background-color: #F8FAFC; "
        "  gridline-color: #E2E8F0; "
        "  border: 1px solid #CBD5E1; "
        "  border-radius: 6px; "
        "  font-family: 'Segoe UI', 'DejaVu Sans', sans-serif; "
        "  font-size: 13px; "
        "} "
        "QTableWidget::item:selected { "
        "  background-color: #3B82F6; "
        "  color: #FFFFFF; "
        "  font-weight: bold; "
        "} "
        "QHeaderView::section { "
        "  background-color: #F1F5F9; "
        "  color: #1E293B; "
        "  font-weight: 600; "
        "  font-size: 12px; "
        "  border: 1px solid #E2E8F0; "
        "  padding: 4px; "
        "}");
    mainLayout->addWidget(m_tableWidget, 1);

    // 4. Active State Display Label
    m_activeStateLabel = new QLabel("Current State: None", this);
    m_activeStateLabel->setStyleSheet(
        "QLabel { background: #F1F5F9; color: #1E293B; padding: 6px 10px; border-radius: 4px; font-weight: 600; font-size: 12px; border: 1px solid #E2E8F0; }");
    mainLayout->addWidget(m_activeStateLabel);

    // 5. Bottom Action Buttons
    QHBoxLayout *btnLayout = new QHBoxLayout();
    btnLayout->setSpacing(8);

    m_refreshBtn = new QPushButton("🔄 Refresh", this);
    m_refreshBtn->setToolTip("Re-scan circuit components and re-generate truth table");
    m_refreshBtn->setStyleSheet(
        "QPushButton { padding: 6px 14px; font-weight: 600; border-radius: 4px; border: 1px solid #CBD5E1; background: #FFFFFF; }"
        "QPushButton:hover { background: #F1F5F9; }");
    connect(m_refreshBtn, &QPushButton::clicked, this, &TruthTableDialog::refresh);
    btnLayout->addWidget(m_refreshBtn);

    m_copyBtn = new QPushButton("📋 Copy Table", this);
    m_copyBtn->setToolTip("Copy truth table to clipboard in Markdown table format");
    m_copyBtn->setStyleSheet(
        "QPushButton { padding: 6px 14px; font-weight: 600; border-radius: 4px; border: 1px solid #4338CA; background: #4F46E5; color: white; }"
        "QPushButton:hover { background: #4338CA; }"
        "QPushButton:disabled { background: #E2E8F0; color: #94A3B8; border-color: #CBD5E1; }");
    connect(m_copyBtn, &QPushButton::clicked, this, &TruthTableDialog::copyToClipboard);
    btnLayout->addWidget(m_copyBtn);

    m_savePngBtn = new QPushButton("🖼️ Save as PNG", this);
    m_savePngBtn->setToolTip("Export the full truth table as a PNG image file");
    m_savePngBtn->setStyleSheet(
        "QPushButton { padding: 6px 14px; font-weight: 600; border-radius: 4px; border: 1px solid #0284C7; background: #0EA5E9; color: white; }"
        "QPushButton:hover { background: #0284C7; }"
        "QPushButton:disabled { background: #E2E8F0; color: #94A3B8; border-color: #CBD5E1; }");
    connect(m_savePngBtn, &QPushButton::clicked, this, &TruthTableDialog::saveAsPng);
    btnLayout->addWidget(m_savePngBtn);

    btnLayout->addStretch();

    m_closeBtn = new QPushButton("Close", this);
    m_closeBtn->setStyleSheet(
        "QPushButton { padding: 6px 16px; font-weight: 600; border-radius: 4px; border: 1px solid #CBD5E1; background: #FFFFFF; }"
        "QPushButton:hover { background: #F1F5F9; }");
    connect(m_closeBtn, &QPushButton::clicked, this, &QDialog::accept);
    btnLayout->addWidget(m_closeBtn);

    mainLayout->addLayout(btnLayout);
}

void TruthTableDialog::setupConnections() {
    // Row selection and click handlers
    connect(m_tableWidget, &QTableWidget::cellClicked, this, &TruthTableDialog::onRowClicked);

    connect(m_tableWidget, &QTableWidget::itemSelectionChanged, this, [this]() {
        if (m_isSyncing) return;
        int row = m_tableWidget->currentRow();
        if (row >= 0 && row < m_totalRows) {
            onRowClicked(row, 0);
        }
    });

    if (m_adapter) {
        // Keep truth table row selection in sync when user toggles inputs or simulates on canvas
        connect(m_adapter, &CircuitAdapter::inputChanged, this, &TruthTableDialog::syncHighlightFromCircuit);
        connect(m_adapter, &CircuitAdapter::simulationStepped, this, &TruthTableDialog::syncHighlightFromCircuit);
        connect(m_adapter, &CircuitAdapter::simulationInvalidated, this, [this]() {
            if (!m_isSyncing && isVisible()) {
                syncHighlightFromCircuit();
            }
        });
    }
}

void TruthTableDialog::refresh() {
    m_copyBtn->setText("📋 Copy Table");
    m_savePngBtn->setText("🖼️ Save as PNG");

    if (!m_adapter) {
        m_statsLabel->setText("Error: Circuit adapter not connected.");
        m_statsLabel->setStyleSheet(
            "QLabel { background: #FEF2F2; color: #B91C1C; border: 1px solid #FCA5A5; padding: 8px 14px; border-radius: 6px; font-size: 13px; font-weight: 600; }");
        m_noteLabel->setVisible(false);
        m_tableWidget->clear();
        m_tableWidget->setRowCount(0);
        m_tableWidget->setColumnCount(0);
        m_activeStateLabel->setText("Current State: None");
        m_copyBtn->setEnabled(false);
        m_savePngBtn->setEnabled(false);
        return;
    }

    // Ensure adapter is fully in sync with canvas items and wire connectivity
    m_adapter->syncSceneItems();
    m_adapter->extractWires();

    m_switches.clear();
    m_leds.clear();
    m_switchNodeIds.clear();
    m_ledNodeIds.clear();

    const auto &items = m_adapter->getItems();
    for (const auto &pair : items) {
        int id = pair.first;
        QGraphicsItem *item = pair.second;
        if (!item) continue;

        if (Switch *sw = dynamic_cast<Switch*>(item)) {
            m_switches.append(sw);
        } else if (LED *led = dynamic_cast<LED*>(item)) {
            m_leds.append(led);
        }
    }

    // Check for empty inputs / outputs edge cases
    if (m_switches.isEmpty() && m_leds.isEmpty()) {
        m_statsLabel->setText("Number of Inputs = 0   |   Number of Outputs = 0   |   Total Rows = 0");
        m_statsLabel->setStyleSheet(
            "QLabel { background-color: #F8FAFC; color: #64748B; border: 1px solid #CBD5E1; border-radius: 6px; padding: 8px 14px; font-size: 13px; font-weight: 600; }");
        m_noteLabel->setText("⚠️ Canvas is empty. Place Switch inputs and LED outputs to generate a truth table.");
        m_noteLabel->setVisible(true);
        m_tableWidget->clear();
        m_tableWidget->setRowCount(0);
        m_tableWidget->setColumnCount(0);
        m_activeStateLabel->setText("Current State: None");
        m_copyBtn->setEnabled(false);
        m_savePngBtn->setEnabled(false);
        return;
    }

    if (m_switches.isEmpty()) {
        m_statsLabel->setText(QString("Number of Inputs = 0   |   Number of Outputs = %1   |   Total Rows = 0").arg(m_leds.size()));
        m_statsLabel->setStyleSheet(
            "QLabel { background-color: #F8FAFC; color: #64748B; border: 1px solid #CBD5E1; border-radius: 6px; padding: 8px 14px; font-size: 13px; font-weight: 600; }");
        m_noteLabel->setText("⚠️ No input switches found. Place at least one Switch to generate input combinations.");
        m_noteLabel->setVisible(true);
        m_tableWidget->clear();
        m_tableWidget->setRowCount(0);
        m_tableWidget->setColumnCount(0);
        m_activeStateLabel->setText("Current State: No inputs");
        m_copyBtn->setEnabled(false);
        m_savePngBtn->setEnabled(false);
        return;
    }

    if (m_leds.isEmpty()) {
        m_statsLabel->setText(QString("Number of Inputs = %1   |   Number of Outputs = 0   |   Total Rows = 0").arg(m_switches.size()));
        m_statsLabel->setStyleSheet(
            "QLabel { background-color: #F8FAFC; color: #64748B; border: 1px solid #CBD5E1; border-radius: 6px; padding: 8px 14px; font-size: 13px; font-weight: 600; }");
        m_noteLabel->setText("⚠️ No output LEDs found. Place at least one LED to observe circuit outputs.");
        m_noteLabel->setVisible(true);
        m_tableWidget->clear();
        m_tableWidget->setRowCount(0);
        m_tableWidget->setColumnCount(0);
        m_activeStateLabel->setText("Current State: No outputs");
        m_copyBtn->setEnabled(false);
        m_savePngBtn->setEnabled(false);
        return;
    }

    // Sort switches deterministically in natural numeric order (S1, S2, ..., S10)
    QCollator collator;
    collator.setNumericMode(true);

    std::sort(m_switches.begin(), m_switches.end(), [&collator, this](Switch *a, Switch *b) {
        int idA = m_adapter->getNodeId(a);
        int idB = m_adapter->getNodeId(b);
        QString tagA = m_adapter->getTag(idA);
        QString tagB = m_adapter->getTag(idB);
        if (tagA.isEmpty()) tagA = QString("S%1").arg(idA);
        if (tagB.isEmpty()) tagB = QString("S%1").arg(idB);
        return collator.compare(tagA, tagB) < 0;
    });

    // Sort LEDs deterministically in natural numeric order (Y1, Y2, ..., Y10)
    std::sort(m_leds.begin(), m_leds.end(), [&collator, this](LED *a, LED *b) {
        int idA = m_adapter->getNodeId(a);
        int idB = m_adapter->getNodeId(b);
        QString tagA = m_adapter->getTag(idA);
        QString tagB = m_adapter->getTag(idB);
        if (tagA.isEmpty()) tagA = QString("Y%1").arg(idA);
        if (tagB.isEmpty()) tagB = QString("Y%1").arg(idB);
        return collator.compare(tagA, tagB) < 0;
    });

    // Cap at 10 inputs to prevent 2^N explosion (>1024 rows) from freezing the UI
    bool inputCapped = false;
    if (m_switches.size() > 10) {
        inputCapped = true;
        m_switches = m_switches.mid(0, 10);
    }

    for (Switch *sw : m_switches) {
        m_switchNodeIds.append(m_adapter->getNodeId(sw));
    }
    for (LED *led : m_leds) {
        m_ledNodeIds.append(m_adapter->getNodeId(led));
    }

    updateTableData();

    // Contextual stats feedback replacing the green banner
    m_statsLabel->setText(QString("Number of Inputs = %1   |   Number of Outputs = %2   |   Total Rows = %3")
        .arg(m_switches.size())
        .arg(m_leds.size())
        .arg(m_totalRows));
    m_statsLabel->setStyleSheet(
        "QLabel { "
        "  background-color: #F8FAFC; "
        "  color: #1E293B; "
        "  border: 1px solid #CBD5E1; "
        "  border-radius: 6px; "
        "  padding: 8px 14px; "
        "  font-size: 13px; "
        "  font-weight: 600; "
        "}");

    if (m_adapter->hasCycle()) {
        m_noteLabel->setText("⚠️ Feedback loop detected: Evaluated using steady-state relaxation.");
        m_noteLabel->setVisible(true);
    } else if (inputCapped) {
        m_noteLabel->setText("ℹ️ Circuit has >10 inputs: Showing first 10 inputs (1,024 rows) for responsive performance.");
        m_noteLabel->setVisible(true);
    } else {
        m_noteLabel->setVisible(false);
    }

    m_copyBtn->setEnabled(m_totalRows > 0);
    m_savePngBtn->setEnabled(m_totalRows > 0);

    // Initial highlight matching current circuit state
    syncHighlightFromCircuit();
}

void TruthTableDialog::updateTableData() {
    int numInputs = m_switches.size();
    int numOutputs = m_leds.size();
    m_totalRows = 1 << numInputs;

    m_tableWidget->clear();
    m_tableWidget->setColumnCount(numInputs + numOutputs);
    m_tableWidget->setRowCount(m_totalRows);

    QStringList headers;
    for (int j = 0; j < numInputs; ++j) {
        QString tag = m_adapter->getTag(m_switchNodeIds[j]);
        headers << (tag.isEmpty() ? QString("In %1").arg(j + 1) : tag);
    }
    for (int k = 0; k < numOutputs; ++k) {
        QString tag = m_adapter->getTag(m_ledNodeIds[k]);
        headers << (tag.isEmpty() ? QString("Out %1").arg(k + 1) : tag);
    }
    m_tableWidget->setHorizontalHeaderLabels(headers);

    // Backup current switch values so truth-table generation does not alter circuit state
    QVector<bool> originalStates;
    for (Switch *sw : m_switches) {
        originalStates.append(sw->getValue());
    }

    // Evaluate each row in memory using the DAG
    for (int row = 0; row < m_totalRows; ++row) {
        // 1. Assign input states for this row
        for (int j = 0; j < numInputs; ++j) {
            bool bit = (row >> (numInputs - 1 - j)) & 1;
            m_adapter->setInputValue(m_switchNodeIds[j], bit ? LogicState::HIGH : LogicState::LOW);
        }

        // 2. Perform in-memory DAG evaluation
        m_adapter->evaluateLogicOnly();

        // 3. Populate input column cells
        for (int j = 0; j < numInputs; ++j) {
            bool bit = (row >> (numInputs - 1 - j)) & 1;
            QTableWidgetItem *item = new QTableWidgetItem(bit ? "1" : "0");
            item->setTextAlignment(Qt::AlignCenter);
            item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
            m_tableWidget->setItem(row, j, item);
        }

        // 4. Read output nodes and populate output column cells
        for (int k = 0; k < numOutputs; ++k) {
            int ledId = m_ledNodeIds[k];
            Node *node = m_adapter->getNode(ledId);
            LogicState st = node ? node->getOutput(0) : LogicState::UNDEFINED;

            QString text = (st == LogicState::HIGH) ? "1" : ((st == LogicState::LOW) ? "0" : "?");
            QTableWidgetItem *item = new QTableWidgetItem(text);
            item->setTextAlignment(Qt::AlignCenter);
            item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);

            if (st == LogicState::HIGH) {
                item->setForeground(QColor(16, 185, 129)); // Emerald Green
            } else if (st == LogicState::LOW) {
                item->setForeground(QColor(71, 85, 105));  // Slate Gray
            } else {
                item->setForeground(QColor(239, 68, 68));   // Red for Undefined
            }
            m_tableWidget->setItem(row, numInputs + k, item);
        }
    }

    // Restore original switch states and re-sync canvas visuals
    for (int j = 0; j < numInputs; ++j) {
        m_adapter->setInputValue(m_switchNodeIds[j], originalStates[j] ? LogicState::HIGH : LogicState::LOW);
    }
    m_adapter->stepSimulation();
}

void TruthTableDialog::onRowClicked(int row, int /*col*/) {
    if (row < 0 || row >= m_totalRows || m_switches.isEmpty() || m_isSyncing) return;

    m_isSyncing = true;

    int numInputs = m_switches.size();
    QStringList inputSummary;

    // Apply each row input value to the circuit switch and logic node
    for (int j = 0; j < numInputs; ++j) {
        bool bit = (row >> (numInputs - 1 - j)) & 1;
        Switch *sw = m_switches[j];
        if (sw) {
            sw->setValue(bit);
        }
        if (j < m_switchNodeIds.size()) {
            m_adapter->setInputValue(m_switchNodeIds[j], bit ? LogicState::HIGH : LogicState::LOW);
            QString tag = m_adapter->getTag(m_switchNodeIds[j]);
            inputSummary << QString("%1=%2").arg(tag.isEmpty() ? QString("S%1").arg(j + 1) : tag).arg(bit ? "1" : "0");
        }
    }

    // Trigger full circuit simulation so gate outputs, wire signals, and LEDs refresh on canvas
    m_adapter->stepSimulation();

    // Keep this row selected in the table
    m_tableWidget->selectRow(row);

    // Read resulting outputs for the active summary label
    QStringList outputSummary;
    for (int k = 0; k < m_ledNodeIds.size(); ++k) {
        int ledId = m_ledNodeIds[k];
        Node *node = m_adapter->getNode(ledId);
        LogicState st = node ? node->getOutput(0) : LogicState::UNDEFINED;
        QString tag = m_adapter->getTag(ledId);
        QString outStr = (st == LogicState::HIGH) ? "1" : ((st == LogicState::LOW) ? "0" : "?");
        outputSummary << QString("%1=%2").arg(tag.isEmpty() ? QString("Y%1").arg(k + 1) : tag).arg(outStr);
    }

    m_activeStateLabel->setText(
        QString("Applied State (Row %1): [ %2 ]  ➔  Outputs: [ %3 ]")
            .arg(row)
            .arg(inputSummary.join(", "))
            .arg(outputSummary.join(", ")));

    m_isSyncing = false;
}

void TruthTableDialog::syncHighlightFromCircuit() {
    if (m_isSyncing || m_switches.isEmpty() || m_totalRows <= 0) return;

    m_isSyncing = true;

    int numInputs = m_switches.size();
    int activeRow = 0;
    bool allValid = true;
    QStringList inputSummary;

    for (int j = 0; j < numInputs; ++j) {
        Switch *sw = m_switches[j];
        if (!sw) {
            allValid = false;
            break;
        }
        bool val = sw->getValue();
        activeRow = (activeRow << 1) | (val ? 1 : 0);

        QString tag = m_adapter->getTag(m_switchNodeIds[j]);
        inputSummary << QString("%1=%2").arg(tag.isEmpty() ? QString("S%1").arg(j + 1) : tag).arg(val ? "1" : "0");
    }

    if (allValid && activeRow >= 0 && activeRow < m_totalRows) {
        m_tableWidget->selectRow(activeRow);
        if (QTableWidgetItem *firstItem = m_tableWidget->item(activeRow, 0)) {
            m_tableWidget->scrollToItem(firstItem, QAbstractItemView::EnsureVisible);
        }

        QStringList outputSummary;
        for (int k = 0; k < m_ledNodeIds.size(); ++k) {
            int ledId = m_ledNodeIds[k];
            Node *node = m_adapter->getNode(ledId);
            LogicState st = node ? node->getOutput(0) : LogicState::UNDEFINED;
            QString tag = m_adapter->getTag(ledId);
            QString outStr = (st == LogicState::HIGH) ? "1" : ((st == LogicState::LOW) ? "0" : "?");
            outputSummary << QString("%1=%2").arg(tag.isEmpty() ? QString("Y%1").arg(k + 1) : tag).arg(outStr);
        }

        m_activeStateLabel->setText(
            QString("Current State (Row %1): [ %2 ]  ➔  Outputs: [ %3 ]")
                .arg(activeRow)
                .arg(inputSummary.join(", "))
                .arg(outputSummary.join(", ")));
    } else {
        m_tableWidget->clearSelection();
        m_activeStateLabel->setText("Current State: Not represented in table");
    }

    m_isSyncing = false;
}

void TruthTableDialog::copyToClipboard() {
    if (m_totalRows <= 0 || m_tableWidget->columnCount() == 0) return;

    int cols = m_tableWidget->columnCount();
    QStringList lines;

    // Header line
    QStringList headerList;
    QStringList dividerList;
    for (int c = 0; c < cols; ++c) {
        QString h = m_tableWidget->horizontalHeaderItem(c)->text();
        headerList << h;
        dividerList << "---";
    }
    lines << "| " + headerList.join(" | ") + " |";
    lines << "| " + dividerList.join(" | ") + " |";

    // Row lines
    for (int r = 0; r < m_totalRows; ++r) {
        QStringList rowCells;
        for (int c = 0; c < cols; ++c) {
            QTableWidgetItem *item = m_tableWidget->item(r, c);
            rowCells << (item ? item->text() : "");
        }
        lines << "| " + rowCells.join(" | ") + " |";
    }

    QGuiApplication::clipboard()->setText(lines.join("\n"));
    m_copyBtn->setText("✓ Copied!");
}

void TruthTableDialog::saveAsPng() {
    if (m_totalRows <= 0 || m_tableWidget->columnCount() == 0) {
        QMessageBox::warning(this, "Save Truth Table", "No truth table data available to save.");
        return;
    }

    QString defaultPath = "truth_table.png";
    QString filePath = QFileDialog::getSaveFileName(
        this,
        "Save Truth Table as PNG Image",
        defaultPath,
        "PNG Image (*.png);;All Files (*.*)");

    if (filePath.isEmpty()) {
        return;
    }

    if (!filePath.endsWith(".png", Qt::CaseInsensitive)) {
        filePath += ".png";
    }

    int numInputs = m_switches.size();
    int numOutputs = m_leds.size();
    int totalCols = numInputs + numOutputs;

    // Calculate column widths based on text measurements
    QFont titleFont("Segoe UI", 13, QFont::Bold);
    QFont subFont("Segoe UI", 9, QFont::Normal);
    QFont headerFont("Segoe UI", 10, QFont::Bold);
    QFont cellFont("Segoe UI", 10, QFont::Normal);
    QFont cellBoldFont("Segoe UI", 10, QFont::Bold);

    QFontMetrics fmHeader(headerFont);
    QFontMetrics fmCell(cellFont);

    int rowNumColWidth = qMax(55, fmHeader.horizontalAdvance("Row #") + 16);
    QVector<int> colWidths(totalCols);

    for (int c = 0; c < totalCols; ++c) {
        QString headerText = m_tableWidget->horizontalHeaderItem(c)
            ? m_tableWidget->horizontalHeaderItem(c)->text()
            : QString("Col %1").arg(c + 1);
        int w = fmHeader.horizontalAdvance(headerText) + 24;
        colWidths[c] = qMax(65, w);
    }

    int tableWidth = rowNumColWidth;
    for (int w : colWidths) {
        tableWidth += w;
    }

    const int padding = 20;
    const int titleHeight = 28;
    const int subtitleHeight = 20;
    const int headerRowHeight = 30;
    const int dataRowHeight = 26;

    int totalWidth = qMax(520, tableWidth + (padding * 2));
    int tableStartX = padding + (totalWidth - (padding * 2) - tableWidth) / 2;

    int totalHeight = padding + titleHeight + 6 + subtitleHeight + 14 
                    + headerRowHeight + (m_totalRows * dataRowHeight) 
                    + padding + 16;

    QImage image(totalWidth, totalHeight, QImage::Format_ARGB32);
    if (image.isNull()) {
        QMessageBox::critical(this, "Export Error", "Failed to allocate image memory (table size too large).");
        return;
    }

    image.fill(Qt::white);

    QPainter painter(&image);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setRenderHint(QPainter::TextAntialiasing, true);

    // 1. Draw Title
    painter.setFont(titleFont);
    painter.setPen(QColor("#0F172A"));
    painter.drawText(QRect(padding, padding, totalWidth - (padding * 2), titleHeight),
                     Qt::AlignLeft | Qt::AlignVCenter, "Circuit Truth Table");

    // 2. Draw Subtitle / Statistics
    painter.setFont(subFont);
    painter.setPen(QColor("#64748B"));
    QString statsText = QString("Number of Inputs = %1   |   Number of Outputs = %2   |   Total Rows = %3")
                            .arg(numInputs).arg(numOutputs).arg(m_totalRows);
    painter.drawText(QRect(padding, padding + titleHeight + 4, totalWidth - (padding * 2), subtitleHeight),
                     Qt::AlignLeft | Qt::AlignVCenter, statsText);

    int currentY = padding + titleHeight + 6 + subtitleHeight + 14;

    // 3. Draw Header Row
    // Row # header
    QRect rowHeaderRect(tableStartX, currentY, rowNumColWidth, headerRowHeight);
    painter.fillRect(rowHeaderRect, QColor("#F1F5F9"));
    painter.setPen(QColor("#CBD5E1"));
    painter.drawRect(rowHeaderRect);
    painter.setPen(QColor("#475569"));
    painter.setFont(headerFont);
    painter.drawText(rowHeaderRect, Qt::AlignCenter, "Row");

    int currentX = tableStartX + rowNumColWidth;
    for (int c = 0; c < totalCols; ++c) {
        QRect colHeaderRect(currentX, currentY, colWidths[c], headerRowHeight);
        bool isInput = (c < numInputs);
        painter.fillRect(colHeaderRect, isInput ? QColor("#EEF2F6") : QColor("#E0E7FF"));
        painter.setPen(QColor("#CBD5E1"));
        painter.drawRect(colHeaderRect);

        painter.setPen(isInput ? QColor("#1E293B") : QColor("#3730A3"));
        QString headerText = m_tableWidget->horizontalHeaderItem(c)
            ? m_tableWidget->horizontalHeaderItem(c)->text()
            : "";
        painter.drawText(colHeaderRect, Qt::AlignCenter, headerText);

        currentX += colWidths[c];
    }
    currentY += headerRowHeight;

    // 4. Draw Data Rows
    for (int r = 0; r < m_totalRows; ++r) {
        QRect rowNumRect(tableStartX, currentY, rowNumColWidth, dataRowHeight);
        QColor rowBg = (r % 2 == 0) ? QColor("#FFFFFF") : QColor("#F8FAFC");
        painter.fillRect(rowNumRect, (r % 2 == 0) ? QColor("#FAFAFA") : QColor("#F1F5F9"));
        painter.setPen(QColor("#E2E8F0"));
        painter.drawRect(rowNumRect);
        painter.setPen(QColor("#64748B"));
        painter.setFont(cellFont);
        painter.drawText(rowNumRect, Qt::AlignCenter, QString::number(r));

        currentX = tableStartX + rowNumColWidth;
        for (int c = 0; c < totalCols; ++c) {
            QRect cellRect(currentX, currentY, colWidths[c], dataRowHeight);
            painter.fillRect(cellRect, rowBg);
            painter.setPen(QColor("#E2E8F0"));
            painter.drawRect(cellRect);

            QTableWidgetItem *item = m_tableWidget->item(r, c);
            QString text = item ? item->text() : "";

            if (c < numInputs) {
                painter.setFont(cellFont);
                painter.setPen(QColor("#1E293B"));
            } else {
                painter.setFont(cellBoldFont);
                if (text == "1") {
                    painter.setPen(QColor("#059669")); // Emerald green
                } else if (text == "0") {
                    painter.setPen(QColor("#64748B")); // Slate
                } else {
                    painter.setPen(QColor("#DC2626")); // Red
                }
            }
            painter.drawText(cellRect, Qt::AlignCenter, text);

            currentX += colWidths[c];
        }
        currentY += dataRowHeight;
    }

    // 5. Draw Footer
    painter.setFont(QFont("Segoe UI", 8));
    painter.setPen(QColor("#94A3B8"));
    painter.drawText(QRect(padding, currentY + 6, totalWidth - (padding * 2), 16),
                     Qt::AlignRight | Qt::AlignVCenter, "Generated by DLD Simulator");

    painter.end();

    if (image.save(filePath, "PNG")) {
        m_savePngBtn->setText("✓ Saved PNG!");
        QTimer::singleShot(2500, this, [this]() {
            m_savePngBtn->setText("🖼️ Save as PNG");
        });
    } else {
        QMessageBox::critical(this, "Save Failed", QString("Could not write image to %1").arg(filePath));
    }
}

