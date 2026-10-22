#include "EquationDialog.h"
#include "logic/CircuitAdapter.h"
#include <QHBoxLayout>
#include <QGuiApplication>
#include <QClipboard>
#include <QIcon>

EquationDialog::EquationDialog(CircuitAdapter *adapter, QWidget *parent)
    : QDialog(parent), m_adapter(adapter) {
    setWindowTitle("Logical Equations");
    setMinimumSize(420, 240);
    resize(460, 280);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(10);
    mainLayout->setContentsMargins(14, 14, 14, 14);

    // Status Banner
    m_statusBanner = new QLabel(this);
    m_statusBanner->setWordWrap(true);
    m_statusBanner->setStyleSheet(
        "QLabel { padding: 8px 12px; border-radius: 6px; font-size: 12px; font-weight: bold; }");
    mainLayout->addWidget(m_statusBanner);

    // Content Display Area (Rich text read-only)
    m_contentArea = new QTextEdit(this);
    m_contentArea->setReadOnly(true);
    m_contentArea->setStyleSheet(
        "QTextEdit { "
        "  background-color: #F8FAFC; "
        "  border: 1px solid #CBD5E1; "
        "  border-radius: 6px; "
        "  padding: 10px; "
        "  font-family: 'Segoe UI', 'DejaVu Sans', sans-serif; "
        "  font-size: 13px; "
        "}");
    mainLayout->addWidget(m_contentArea, 1);

    // Bottom Action Buttons
    QHBoxLayout *btnLayout = new QHBoxLayout();
    btnLayout->setSpacing(8);

    m_refreshBtn = new QPushButton("🔄 Refresh", this);
    m_refreshBtn->setToolTip("Re-check circuit readiness and re-calculate equations");
    m_refreshBtn->setStyleSheet(
        "QPushButton { padding: 6px 14px; font-weight: 600; border-radius: 4px; border: 1px solid #CBD5E1; background: #FFFFFF; }"
        "QPushButton:hover { background: #F1F5F9; }");
    connect(m_refreshBtn, &QPushButton::clicked, this, &EquationDialog::refresh);
    btnLayout->addWidget(m_refreshBtn);

    m_copyBtn = new QPushButton("📋 Copy Equation", this);
    m_copyBtn->setToolTip("Copy generated Boolean equation to clipboard");
    m_copyBtn->setStyleSheet(
        "QPushButton { padding: 6px 14px; font-weight: 600; border-radius: 4px; border: 1px solid #4338CA; background: #4F46E5; color: white; }"
        "QPushButton:hover { background: #4338CA; }"
        "QPushButton:disabled { background: #E2E8F0; color: #94A3B8; border-color: #CBD5E1; }");
    connect(m_copyBtn, &QPushButton::clicked, this, [this]() {
        if (!m_lastCopiedText.isEmpty()) {
            QGuiApplication::clipboard()->setText(m_lastCopiedText);
            m_copyBtn->setText("✓ Copied!");
        }
    });
    btnLayout->addWidget(m_copyBtn);

    btnLayout->addStretch();

    m_closeBtn = new QPushButton("Close", this);
    m_closeBtn->setStyleSheet(
        "QPushButton { padding: 6px 16px; font-weight: 600; border-radius: 4px; border: 1px solid #CBD5E1; background: #FFFFFF; }"
        "QPushButton:hover { background: #F1F5F9; }");
    connect(m_closeBtn, &QPushButton::clicked, this, &QDialog::accept);
    btnLayout->addWidget(m_closeBtn);

    mainLayout->addLayout(btnLayout);

    refresh();
}

void EquationDialog::refresh() {
    m_copyBtn->setText("📋 Copy Equation");
    if (!m_adapter) {
        m_statusBanner->setText("⚠️ Error: Circuit adapter not connected.");
        m_statusBanner->setStyleSheet(
            "QLabel { background: #FEF2F2; color: #B91C1C; border: 1px solid #FCA5A5; padding: 10px; border-radius: 6px; }");
        m_contentArea->setHtml("<p style='color:#64748B;'>No circuit data available.</p>");
        m_copyBtn->setEnabled(false);
        return;
    }

    EquationReport report = EquationGenerator::generate(m_adapter);
    updateUI(report);
}

void EquationDialog::updateUI(const EquationReport &report) {
    if (!report.readiness.isReady) {
        // Case 1: Feedback loop detected
        if (report.readiness.hasFeedbackLoop) {
            m_statusBanner->setText("⚠️ Feedback loop detected");
            m_statusBanner->setStyleSheet(
                "QLabel { background: #FEF2F2; color: #B91C1C; border: 1px solid #FCA5A5; padding: 8px 12px; border-radius: 6px; font-weight:bold; }");

            QString html = "<p style='color:#334155; margin:8px 0;'>Feedback loop detected in the circuit. Logical equations cannot be generated for sequential feedback circuits.</p>";
            m_contentArea->setHtml(html);
            m_copyBtn->setEnabled(false);
            m_lastCopiedText.clear();
            return;
        }

        // Case 2: Open ends detected
        m_statusBanner->setText("⚠️ Open ends detected");
        m_statusBanner->setStyleSheet(
            "QLabel { background: #FFFBEB; color: #B45309; border: 1px solid #FCD34D; padding: 8px 12px; border-radius: 6px; font-weight:bold; }");

        QString html = "<p style='color:#334155; margin:4px 0 8px 0;'>The following open ends must be connected:</p>";
        html += "<ul style='color:#DC2626; margin:0; padding-left:20px; line-height: 1.5;'>";
        for (const QString &issue : report.readiness.issues) {
            html += QString("<li>%1</li>").arg(issue);
        }
        html += "</ul>";

        m_contentArea->setHtml(html);
        m_copyBtn->setEnabled(false);
        m_lastCopiedText.clear();
        return;
    }

    // Case 3: Circuit Ready - show output equations only for LEDs
    m_statusBanner->setText("✓ Circuit Ready");
    m_statusBanner->setStyleSheet(
        "QLabel { background: #F0FDF4; color: #15803D; border: 1px solid #86EFAC; padding: 8px 12px; border-radius: 6px; font-weight:bold; }");

    QString html;
    QString plainTextCopy;

    for (const OutputEquation &eq : report.equations) {
        html += "<div style='background:#FFFFFF; border:1px solid #CBD5E1; border-left:4px solid #4F46E5; border-radius:4px; padding:10px 14px; margin-bottom:8px;'>";
        html += QString("<div style='font-size:15px; font-weight:bold; color:#1E1B4B; font-family:Courier, monospace;'>%1</div>")
                    .arg(eq.fullEquation);
        html += "</div>";

        if (!plainTextCopy.isEmpty()) plainTextCopy += "\n";
        plainTextCopy += eq.fullEquation;
    }

    m_contentArea->setHtml(html);
    m_copyBtn->setEnabled(!plainTextCopy.isEmpty());
    m_lastCopiedText = plainTextCopy;
}

