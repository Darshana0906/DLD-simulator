#include "ReverseEquationDialog.h"
#include "CircuitScene.h"
#include "logic/EquationParser.h"
#include "logic/CircuitBuilder.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QMessageBox>

QPushButton* ReverseEquationDialog::createKeypadButton(const QString &text, const QString &tooltip, const QString &colorStyle) {
    QPushButton *btn = new QPushButton(text, this);
    btn->setToolTip(tooltip);
    btn->setCursor(Qt::PointingHandCursor);
    btn->setMinimumHeight(34);
    btn->setStyleSheet(colorStyle);
    return btn;
}

ReverseEquationDialog::ReverseEquationDialog(CircuitScene *scene, QWidget *parent)
    : QDialog(parent), m_scene(scene) {
    setWindowTitle("Equation → Circuit");
    setMinimumSize(620, 560);
    resize(660, 600);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(10);
    mainLayout->setContentsMargins(16, 16, 16, 16);

    // Title & Instructions
    QLabel *titleLabel = new QLabel("Construct Boolean Equation:", this);
    titleLabel->setStyleSheet("QLabel { font-size: 15px; font-weight: bold; color: #1E1B4B; }");
    mainLayout->addWidget(titleLabel);

    // =========================================================================
    // Keypad Toolbar / Builder Buttons
    // =========================================================================
    QGroupBox *keypadGroup = new QGroupBox("Quick-Build Buttons", this);
    keypadGroup->setStyleSheet(
        "QGroupBox { font-size: 12px; font-weight: bold; color: #475569; "
        "  border: 1px solid #CBD5E1; border-radius: 8px; margin-top: 6px; padding-top: 10px; background: #F8FAFC; }"
        "QGroupBox::title { subcontrol-origin: margin; left: 10px; padding: 0 4px; }");

    QVBoxLayout *keypadLayout = new QVBoxLayout(keypadGroup);
    keypadLayout->setSpacing(8);

    // --- Row 1: Variables (Y, =, S, Reset) ---
    QHBoxLayout *row1 = new QHBoxLayout();
    row1->setSpacing(6);

    const QString keypadStyle =
        "QPushButton { background: #FFFFFF; color: #1E293B; border: 1px solid #CBD5E1; "
        "  border-radius: 6px; font-weight: bold; font-size: 13px; padding: 4px 12px; }"
        "QPushButton:hover { background: #EEF2FF; border-color: #818CF8; color: #4338CA; }"
        "QPushButton:pressed { background: #E0E7FF; border-color: #4F46E5; color: #3730A3; }";

    const QString utilityStyle =
        "QPushButton { background: #F8FAFC; color: #64748B; border: 1px solid #CBD5E1; "
        "  border-radius: 6px; font-size: 11px; font-weight: 600; padding: 4px 10px; }"
        "QPushButton:hover { background: #F1F5F9; color: #1E293B; border-color: #94A3B8; }"
        "QPushButton:pressed { background: #E2E8F0; }";

    const QString clearStyle =
        "QPushButton { background: #F8FAFC; color: #64748B; border: 1px solid #CBD5E1; "
        "  border-radius: 6px; font-size: 11px; font-weight: 600; padding: 4px 10px; }"
        "QPushButton:hover { background: #FEE2E2; color: #DC2626; border-color: #FCA5A5; }"
        "QPushButton:pressed { background: #FECDD3; }";

    QLabel *varLabel = new QLabel("Variables:", this);
    varLabel->setStyleSheet("font-weight: 600; font-size: 11px; color: #64748B; min-width: 65px;");
    row1->addWidget(varLabel);

    // Output Button (Y1 -> Y2 -> ...)
    m_outputBtn = createKeypadButton(
        "Y1",
        "Click to insert output variable. Automatically increments to Y2, Y3...",
        keypadStyle);
    connect(m_outputBtn, &QPushButton::clicked, this, &ReverseEquationDialog::onOutputVarClicked);
    row1->addWidget(m_outputBtn);

    // Equals Button
    QPushButton *eqBtn = createKeypadButton(
        " = ",
        "Insert equals sign (=)",
        keypadStyle);
    connect(eqBtn, &QPushButton::clicked, this, [this]() { insertToken(" = "); });
    row1->addWidget(eqBtn);

    // Input Switch Button (S1 -> S2 -> ...)
    m_inputBtn = createKeypadButton(
        "S1",
        "Click to insert input switch. Automatically increments to S2, S3...",
        keypadStyle);
    connect(m_inputBtn, &QPushButton::clicked, this, &ReverseEquationDialog::onInputVarClicked);
    row1->addWidget(m_inputBtn);

    // Reset S/Y Counters Button
    QPushButton *resetCountBtn = createKeypadButton(
        "↺ Reset S/Y",
        "Reset S counter to S1 and Y counter to Y1",
        utilityStyle);
    connect(resetCountBtn, &QPushButton::clicked, this, &ReverseEquationDialog::onResetCountersClicked);
    row1->addWidget(resetCountBtn);

    row1->addStretch();
    keypadLayout->addLayout(row1);

    // --- Row 2: Logic Gates (AND, OR, NOT, XOR, XNOR) ---
    QHBoxLayout *row2 = new QHBoxLayout();
    row2->setSpacing(6);

    QLabel *gateLabel = new QLabel("Gates:", this);
    gateLabel->setStyleSheet("font-weight: 600; font-size: 11px; color: #64748B; min-width: 65px;");
    row2->addWidget(gateLabel);

    QPushButton *andBtn = createKeypadButton("AND  ·", "Insert AND operator (·)", keypadStyle);
    connect(andBtn, &QPushButton::clicked, this, [this]() { insertToken(" · "); });
    row2->addWidget(andBtn);

    QPushButton *orBtn = createKeypadButton("OR  +", "Insert OR operator (+)", keypadStyle);
    connect(orBtn, &QPushButton::clicked, this, [this]() { insertToken(" + "); });
    row2->addWidget(orBtn);

    QPushButton *notBtn = createKeypadButton("NOT  '", "Insert NOT complement (')", keypadStyle);
    connect(notBtn, &QPushButton::clicked, this, [this]() { insertToken("'"); });
    row2->addWidget(notBtn);

    QPushButton *xorBtn = createKeypadButton("XOR  ⊕", "Insert XOR operator (⊕)", keypadStyle);
    connect(xorBtn, &QPushButton::clicked, this, [this]() { insertToken(" ⊕ "); });
    row2->addWidget(xorBtn);

    QPushButton *xnorBtn = createKeypadButton("XNOR  ⊙", "Insert XNOR operator (⊙)", keypadStyle);
    connect(xnorBtn, &QPushButton::clicked, this, [this]() { insertToken(" ⊙ "); });
    row2->addWidget(xnorBtn);

    row2->addStretch();
    keypadLayout->addLayout(row2);

    // --- Row 3: Grouping & Clear ---
    QHBoxLayout *row3 = new QHBoxLayout();
    row3->setSpacing(6);

    QLabel *groupLabel = new QLabel("Grouping:", this);
    groupLabel->setStyleSheet("font-weight: 600; font-size: 11px; color: #64748B; min-width: 65px;");
    row3->addWidget(groupLabel);

    QPushButton *lParenBtn = createKeypadButton(" ( ", "Insert left parenthesis", keypadStyle);
    connect(lParenBtn, &QPushButton::clicked, this, [this]() { insertToken("("); });
    row3->addWidget(lParenBtn);

    QPushButton *rParenBtn = createKeypadButton(" ) ", "Insert right parenthesis", keypadStyle);
    connect(rParenBtn, &QPushButton::clicked, this, [this]() { insertToken(")"); });
    row3->addWidget(rParenBtn);

    QPushButton *clearAllBtn = createKeypadButton(
        "🗑 Clear", "Clear entire equation editor and reset counters",
        clearStyle);
    connect(clearAllBtn, &QPushButton::clicked, this, &ReverseEquationDialog::onClearAllClicked);
    row3->addWidget(clearAllBtn);

    row3->addStretch();
    keypadLayout->addLayout(row3);

    mainLayout->addWidget(keypadGroup);

    // =========================================================================
    // Equation Text Input Area
    // =========================================================================
    QLabel *editorLabel = new QLabel("Equation Editor (click buttons above or type directly):", this);
    editorLabel->setStyleSheet("font-size: 12px; font-weight: 600; color: #334155;");
    mainLayout->addWidget(editorLabel);

    m_inputArea = new QTextEdit(this);
    m_inputArea->setPlaceholderText(
        "Construct equation using the buttons above or type directly:\n"
        "Y1 = S1 · S2\n"
        "Y2 = (S1 + S2)'\n\n"
        "Tip: Use (S1 · S2)' for NAND, (S1 + S2)' for NOR");
    m_inputArea->setStyleSheet(
        "QTextEdit { "
        "  background-color: #FFFFFF; "
        "  border: 2px solid #CBD5E1; "
        "  border-radius: 6px; "
        "  padding: 10px; "
        "  font-family: 'Courier New', 'DejaVu Sans Mono', monospace; "
        "  font-size: 15px; "
        "  font-weight: 600; "
        "  color: #1E293B; "
        "}");
    m_inputArea->setAcceptRichText(false);
    mainLayout->addWidget(m_inputArea, 1);

    // Bottom Action Buttons
    QHBoxLayout *btnLayout = new QHBoxLayout();
    btnLayout->setSpacing(8);

    m_generateBtn = new QPushButton("▶ Generate Circuit on Canvas", this);
    m_generateBtn->setToolTip("Parse equation and place circuit components on canvas");
    m_generateBtn->setStyleSheet(
        "QPushButton { padding: 9px 22px; font-weight: 700; font-size: 13px; "
        "  border-radius: 6px; border: 1px solid #4338CA; "
        "  background: #4F46E5; color: white; }"
        "QPushButton:hover { background: #4338CA; }"
        "QPushButton:pressed { background: #3730A3; }");
    connect(m_generateBtn, &QPushButton::clicked, this, &ReverseEquationDialog::onGenerateClicked);
    btnLayout->addWidget(m_generateBtn);

    btnLayout->addStretch();

    m_closeBtn = new QPushButton("Close", this);
    m_closeBtn->setStyleSheet(
        "QPushButton { padding: 8px 18px; font-weight: 600; font-size: 13px; "
        "  border-radius: 6px; border: 1px solid #CBD5E1; "
        "  background: #FFFFFF; color: #334155; }"
        "QPushButton:hover { background: #F1F5F9; border-color: #94A3B8; }");
    connect(m_closeBtn, &QPushButton::clicked, this, &QDialog::accept);
    btnLayout->addWidget(m_closeBtn);

    mainLayout->addLayout(btnLayout);
}

void ReverseEquationDialog::insertToken(const QString &text) {
    m_inputArea->insertPlainText(text);
    m_inputArea->setFocus();
}

void ReverseEquationDialog::onInputVarClicked() {
    QString varName = QString("S%1").arg(m_sCounter++);
    insertToken(varName);
    m_inputBtn->setText(QString("S%1").arg(m_sCounter));
    m_inputBtn->setToolTip(QString("Click to insert S%1 (auto-increments)").arg(m_sCounter));
}

void ReverseEquationDialog::onOutputVarClicked() {
    QString outName = QString("Y%1").arg(m_yCounter++);
    insertToken(outName);
    m_outputBtn->setText(QString("Y%1").arg(m_yCounter));
    m_outputBtn->setToolTip(QString("Click to insert Y%1 (auto-increments)").arg(m_yCounter));
}

void ReverseEquationDialog::onResetCountersClicked() {
    m_sCounter = 1;
    m_inputBtn->setText("S1");
    m_inputBtn->setToolTip("Click to insert S1 (auto-increments)");

    m_yCounter = 1;
    m_outputBtn->setText("Y1");
    m_outputBtn->setToolTip("Click to insert Y1 (auto-increments)");
}

void ReverseEquationDialog::onClearAllClicked() {
    m_inputArea->clear();
    onResetCountersClicked();
    m_inputArea->setFocus();
}

void ReverseEquationDialog::onGenerateClicked() {
    QString input = m_inputArea->toPlainText().trimmed();
    if (input.isEmpty()) {
        QMessageBox::warning(this, "Empty Equation", "Please construct or type a Boolean equation.");
        return;
    }

    // Step 1: Parse the equation(s)
    ParseResult parseResult = EquationParser::parse(input);
    if (!parseResult.success) {
        QMessageBox::warning(this, "Parse Error", parseResult.errorMessage);
        return;
    }

    // Step 2: Clear existing canvas before generating by default
    if (m_scene) {
        m_scene->clearCircuit();
    }

    // Step 3: Build the circuit
    CircuitBuilder builder(m_scene);
    BuildResult buildResult = builder.build(parseResult.equations);

    if (!buildResult.success) {
        QMessageBox::warning(this, "Build Error", buildResult.errorMessage);
        return;
    }

    // Emit bounds so MainWindow can center the view directly on the generated circuit
    emit circuitGenerated(buildResult.bounds);

    // If parent exists, move dialog slightly to the side so canvas is not obscured
    if (parentWidget()) {
        int targetX = parentWidget()->geometry().right() - width() - 20;
        int targetY = parentWidget()->geometry().top() + 60;
        if (targetX > 0 && targetY > 0) {
            move(targetX, targetY);
        }
    }
}
