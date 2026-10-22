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

    QLabel *varLabel = new QLabel("Variables:", this);
    varLabel->setStyleSheet("font-weight: 600; font-size: 11px; color: #64748B; min-width: 65px;");
    row1->addWidget(varLabel);

    // Output Button (Y1 -> Y2 -> ...)
    m_outputBtn = createKeypadButton(
        "Y1",
        "Click to insert output variable. Automatically increments to Y2, Y3...",
        "QPushButton { background: #EDE9FE; color: #4338CA; border: 1px solid #A5B4FC; "
        "  border-radius: 6px; font-weight: bold; font-size: 13px; padding: 4px 14px; }"
        "QPushButton:hover { background: #DDD6FE; }"
        "QPushButton:pressed { background: #C4B5FD; }");
    connect(m_outputBtn, &QPushButton::clicked, this, &ReverseEquationDialog::onOutputVarClicked);
    row1->addWidget(m_outputBtn);

    // Equals Button
    QPushButton *eqBtn = createKeypadButton(
        " = ",
        "Insert equals sign (=)",
        "QPushButton { background: #F1F5F9; color: #1E293B; border: 1px solid #CBD5E1; "
        "  border-radius: 6px; font-weight: bold; font-size: 14px; padding: 4px 14px; }"
        "QPushButton:hover { background: #E2E8F0; }"
        "QPushButton:pressed { background: #CBD5E1; }");
    connect(eqBtn, &QPushButton::clicked, this, [this]() { insertToken(" = "); });
    row1->addWidget(eqBtn);

    // Input Switch Button (S1 -> S2 -> ...)
    m_inputBtn = createKeypadButton(
        "S1",
        "Click to insert input switch. Automatically increments to S2, S3...",
        "QPushButton { background: #ECFDF5; color: #047857; border: 1px solid #6EE7B7; "
        "  border-radius: 6px; font-weight: bold; font-size: 13px; padding: 4px 14px; }"
        "QPushButton:hover { background: #D1FAE5; }"
        "QPushButton:pressed { background: #A7F3D0; }");
    connect(m_inputBtn, &QPushButton::clicked, this, &ReverseEquationDialog::onInputVarClicked);
    row1->addWidget(m_inputBtn);

    // Reset S/Y Counters Button
    QPushButton *resetCountBtn = createKeypadButton(
        "↺ Reset S/Y",
        "Reset S counter to S1 and Y counter to Y1",
        "QPushButton { background: #FFFFFF; color: #64748B; border: 1px dashed #CBD5E1; "
        "  border-radius: 6px; font-size: 11px; font-weight: 600; padding: 4px 10px; }"
        "QPushButton:hover { background: #F1F5F9; color: #334155; }");
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

    QString gateStyle =
        "QPushButton { background: #EFF6FF; color: #1D4ED8; border: 1px solid #93C5FD; "
        "  border-radius: 6px; font-weight: bold; font-size: 12px; padding: 4px 12px; }"
        "QPushButton:hover { background: #DBEAFE; }"
        "QPushButton:pressed { background: #BFDBFE; }";

    QPushButton *andBtn = createKeypadButton("AND  ·", "Insert AND operator (·)", gateStyle);
    connect(andBtn, &QPushButton::clicked, this, [this]() { insertToken(" · "); });
    row2->addWidget(andBtn);

    QPushButton *orBtn = createKeypadButton("OR  +", "Insert OR operator (+)", gateStyle);
    connect(orBtn, &QPushButton::clicked, this, [this]() { insertToken(" + "); });
    row2->addWidget(orBtn);

    QPushButton *notBtn = createKeypadButton("NOT  '", "Insert NOT complement (')",
        "QPushButton { background: #FFF1F2; color: #BE123C; border: 1px solid #FDA4AF; "
        "  border-radius: 6px; font-weight: bold; font-size: 13px; padding: 4px 12px; }"
        "QPushButton:hover { background: #FFE4E6; }"
        "QPushButton:pressed { background: #FECDD3; }");
    connect(notBtn, &QPushButton::clicked, this, [this]() { insertToken("'"); });
    row2->addWidget(notBtn);

    QPushButton *xorBtn = createKeypadButton("XOR  ⊕", "Insert XOR operator (⊕)", gateStyle);
    connect(xorBtn, &QPushButton::clicked, this, [this]() { insertToken(" ⊕ "); });
    row2->addWidget(xorBtn);

    QPushButton *xnorBtn = createKeypadButton("XNOR  ⊙", "Insert XNOR operator (⊙)", gateStyle);
    connect(xnorBtn, &QPushButton::clicked, this, [this]() { insertToken(" ⊙ "); });
    row2->addWidget(xnorBtn);

    row2->addStretch();
    keypadLayout->addLayout(row2);

    // --- Row 3: Grouping & Editing ---
    QHBoxLayout *row3 = new QHBoxLayout();
    row3->setSpacing(6);

    QLabel *groupLabel = new QLabel("Grouping:", this);
    groupLabel->setStyleSheet("font-weight: 600; font-size: 11px; color: #64748B; min-width: 65px;");
    row3->addWidget(groupLabel);

    QString parenStyle =
        "QPushButton { background: #F8FAFC; color: #334155; border: 1px solid #CBD5E1; "
        "  border-radius: 6px; font-weight: bold; font-size: 13px; padding: 4px 10px; }"
        "QPushButton:hover { background: #E2E8F0; }";

    QPushButton *lParenBtn = createKeypadButton(" ( ", "Left parenthesis", parenStyle);
    connect(lParenBtn, &QPushButton::clicked, this, [this]() { insertToken("("); });
    row3->addWidget(lParenBtn);

    QPushButton *rParenBtn = createKeypadButton(" ) ", "Right parenthesis", parenStyle);
    connect(rParenBtn, &QPushButton::clicked, this, [this]() { insertToken(")"); });
    row3->addWidget(rParenBtn);

    QPushButton *nandTipBtn = createKeypadButton(
        "( · )'", "NAND template: (A · B)' produces a NAND gate",
        "QPushButton { background: #FEF3C7; color: #B45309; border: 1px solid #FCD34D; "
        "  border-radius: 6px; font-weight: bold; font-size: 11px; padding: 4px 8px; }"
        "QPushButton:hover { background: #FDE68A; }");
    connect(nandTipBtn, &QPushButton::clicked, this, [this]() { insertToken("()' "); });
    row3->addWidget(nandTipBtn);

    QPushButton *spaceBtn = createKeypadButton("␣ Space", "Insert space", parenStyle);
    connect(spaceBtn, &QPushButton::clicked, this, [this]() { insertToken(" "); });
    row3->addWidget(spaceBtn);

    QPushButton *newLineBtn = createKeypadButton("↵ New Line", "New line for another output (e.g. Y2)", parenStyle);
    connect(newLineBtn, &QPushButton::clicked, this, [this]() { insertToken("\n"); });
    row3->addWidget(newLineBtn);

    QPushButton *backspaceBtn = createKeypadButton(
        "⌫ Backspace", "Delete character before cursor",
        "QPushButton { background: #FEE2E2; color: #DC2626; border: 1px solid #FCA5A5; "
        "  border-radius: 6px; font-weight: bold; font-size: 11px; padding: 4px 8px; }"
        "QPushButton:hover { background: #FECACA; }");
    connect(backspaceBtn, &QPushButton::clicked, this, &ReverseEquationDialog::onBackspaceClicked);
    row3->addWidget(backspaceBtn);

    QPushButton *clearAllBtn = createKeypadButton(
        "🗑 Clear", "Clear entire equation editor and reset counters",
        "QPushButton { background: #FFFFFF; color: #EF4444; border: 1px solid #FCA5A5; "
        "  border-radius: 6px; font-weight: bold; font-size: 11px; padding: 4px 8px; }"
        "QPushButton:hover { background: #FEF2F2; }");
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

    // Clear canvas checkbox
    m_clearCanvasCheck = new QCheckBox("Clear existing canvas before generating", this);
    m_clearCanvasCheck->setChecked(true);
    m_clearCanvasCheck->setStyleSheet("QCheckBox { font-size: 12px; color: #475569; padding: 2px 0; }");
    mainLayout->addWidget(m_clearCanvasCheck);

    // Bottom Action Buttons
    QHBoxLayout *btnLayout = new QHBoxLayout();
    btnLayout->setSpacing(8);

    m_generateBtn = new QPushButton("▶ Generate Circuit on Canvas", this);
    m_generateBtn->setToolTip("Parse equation and place circuit components on canvas");
    m_generateBtn->setStyleSheet(
        "QPushButton { padding: 9px 22px; font-weight: 700; font-size: 14px; "
        "  border-radius: 6px; border: 1px solid #15803D; "
        "  background: #16A34A; color: white; }"
        "QPushButton:hover { background: #15803D; }"
        "QPushButton:pressed { background: #166534; }");
    connect(m_generateBtn, &QPushButton::clicked, this, &ReverseEquationDialog::onGenerateClicked);
    btnLayout->addWidget(m_generateBtn);

    btnLayout->addStretch();

    m_closeBtn = new QPushButton("Close", this);
    m_closeBtn->setStyleSheet(
        "QPushButton { padding: 8px 18px; font-weight: 600; font-size: 13px; "
        "  border-radius: 6px; border: 1px solid #CBD5E1; "
        "  background: #FFFFFF; color: #334155; }"
        "QPushButton:hover { background: #F1F5F9; }");
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

void ReverseEquationDialog::onBackspaceClicked() {
    m_inputArea->textCursor().deletePreviousChar();
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

    // Step 2: Optionally clear the canvas
    if (m_clearCanvasCheck->isChecked()) {
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
