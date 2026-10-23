#include "CircuitSerializer.h"
#include "CircuitScene.h"
#include "logic/CircuitAdapter.h"
#include "elements/Point.h"
#include "elements/Wire.h"
#include "elements/Input.h"
#include "elements/Switch.h"
#include "elements/GND.h"
#include "elements/Const1.h"
#include "elements/LED.h"
#include "gates/Gate.h"
#include "gates/AndGate.h"
#include "gates/OrGate.h"
#include "gates/NotGate.h"
#include "gates/NandGate.h"
#include "gates/NorGate.h"
#include "gates/XorGate.h"
#include "gates/XnorGate.h"

#include <QFile>
#include <QFileInfo>
#include <QTextStream>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QPainterPath>
#include <QRectF>
#include <algorithm>

namespace {

QString getComponentType(QGraphicsItem *item) {
    if (dynamic_cast<AndGate*>(item)) return "AndGate";
    if (dynamic_cast<OrGate*>(item)) return "OrGate";
    if (dynamic_cast<NotGate*>(item)) return "NotGate";
    if (dynamic_cast<NandGate*>(item)) return "NandGate";
    if (dynamic_cast<NorGate*>(item)) return "NorGate";
    if (dynamic_cast<XorGate*>(item)) return "XorGate";
    if (dynamic_cast<XnorGate*>(item)) return "XnorGate";
    if (dynamic_cast<Switch*>(item)) return "Switch";
    if (dynamic_cast<Const1*>(item)) return "Const1";
    if (dynamic_cast<GND*>(item)) return "GND";
    if (dynamic_cast<LED*>(item)) return "LED";
    return QString();
}

QGraphicsItem* createComponent(const QString &type) {
    if (type == "AndGate") return new AndGate();
    if (type == "OrGate") return new OrGate();
    if (type == "NotGate") return new NotGate();
    if (type == "NandGate") return new NandGate();
    if (type == "NorGate") return new NorGate();
    if (type == "XorGate") return new XorGate();
    if (type == "XnorGate") return new XnorGate();
    if (type == "Switch") return new Switch();
    if (type == "Const1") return new Const1();
    if (type == "GND") return new GND();
    if (type == "LED") return new LED();
    return nullptr;
}

QString pathToSvgD(const QPainterPath &path, const QPointF &offset = QPointF(0, 0)) {
    QString d;
    for (int i = 0; i < path.elementCount(); ++i) {
        const QPainterPath::Element &elem = path.elementAt(i);
        qreal x = elem.x + offset.x();
        qreal y = elem.y + offset.y();
        if (elem.isMoveTo()) {
            d += QString("M %1 %2 ").arg(x, 0, 'f', 1).arg(y, 0, 'f', 1);
        } else if (elem.isLineTo()) {
            d += QString("L %1 %2 ").arg(x, 0, 'f', 1).arg(y, 0, 'f', 1);
        } else if (elem.isCurveTo()) {
            if (i + 2 < path.elementCount()) {
                const QPainterPath::Element &cp2 = path.elementAt(i + 1);
                const QPainterPath::Element &end = path.elementAt(i + 2);
                d += QString("C %1 %2, %3 %4, %5 %6 ")
                     .arg(x, 0, 'f', 1).arg(y, 0, 'f', 1)
                     .arg(cp2.x + offset.x(), 0, 'f', 1).arg(cp2.y + offset.y(), 0, 'f', 1)
                     .arg(end.x + offset.x(), 0, 'f', 1).arg(end.y + offset.y(), 0, 'f', 1);
                i += 2;
            }
        }
    }
    return d.trimmed();
}

} // namespace

QByteArray CircuitSerializer::serializeToJson(CircuitScene *scene) {
    if (!scene) return QByteArray();

    CircuitAdapter *adapter = scene->getAdapter();
    if (adapter) {
        adapter->syncSceneItems();
    }

    QJsonObject rootObj;
    rootObj["format"] = "DLDSimulator";
    rootObj["version"] = "1.0";

    QJsonObject sceneObj;
    sceneObj["width"] = scene->sceneRect().width();
    sceneObj["height"] = scene->sceneRect().height();
    rootObj["scene"] = sceneObj;

    QJsonArray componentsArray;
    QJsonArray wiresArray;

    for (QGraphicsItem *item : scene->items()) {
        QString compType = getComponentType(item);
        if (compType.isEmpty()) continue;

        QJsonObject compObj;
        int id = adapter ? adapter->getNodeId(item) : -1;
        compObj["id"] = id;
        compObj["type"] = compType;
        compObj["x"] = item->pos().x();
        compObj["y"] = item->pos().y();
        if (item->scale() != 1.0) {
            compObj["scale"] = item->scale();
        }

        if (Gate *g = dynamic_cast<Gate*>(item)) {
            compObj["inputCount"] = g->getInputCount();
        } else if (Switch *sw = dynamic_cast<Switch*>(item)) {
            compObj["state"] = sw->getValue();
        } else if (LED *led = dynamic_cast<LED*>(item)) {
            compObj["state"] = led->getState();
        }

        componentsArray.append(compObj);
    }
    rootObj["components"] = componentsArray;

    if (adapter) {
        for (QGraphicsItem *item : scene->items()) {
            Wire *wire = dynamic_cast<Wire*>(item);
            if (!wire || !wire->getStartPoint() || !wire->getEndPoint()) continue;

            int startNodeId = -1, endNodeId = -1;
            int startPinIdx = 0, endPinIdx = 0;
            bool startIsOutput = false, endIsOutput = false;

            if (!adapter->getPinInfo(wire->getStartPoint(), startNodeId, startIsOutput, startPinIdx))
                continue;
            if (!adapter->getPinInfo(wire->getEndPoint(), endNodeId, endIsOutput, endPinIdx))
                continue;

            QJsonObject wireObj;
            wireObj["startNodeId"] = startNodeId;
            wireObj["startIsOutput"] = startIsOutput;
            wireObj["startPinIdx"] = startPinIdx;
            wireObj["endNodeId"] = endNodeId;
            wireObj["endIsOutput"] = endIsOutput;
            wireObj["endPinIdx"] = endPinIdx;
            wireObj["logicValue"] = wire->getLogicValue();

            wiresArray.append(wireObj);
        }
    }
    rootObj["wires"] = wiresArray;

    QJsonDocument doc(rootObj);
    return doc.toJson(QJsonDocument::Indented);
}

bool CircuitSerializer::deserializeFromJson(const QByteArray &jsonData, CircuitScene *scene, QString *errorMessage) {
    if (!scene) {
        if (errorMessage) *errorMessage = "Circuit scene is null.";
        return false;
    }

    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(jsonData, &parseError);
    if (doc.isNull() || !doc.isObject()) {
        if (errorMessage) {
            *errorMessage = QString("JSON Parse Error: %1 (offset %2)")
                                .arg(parseError.errorString())
                                .arg(parseError.offset);
        }
        return false;
    }

    QJsonObject root = doc.object();
    if (root.value("format").toString() != "DLDSimulator") {
        if (errorMessage) {
            *errorMessage = "File is not a valid DLDSimulator circuit project.";
        }
        return false;
    }

    // Step 1: Safely clear previous scene and adapter state
    scene->clearCircuit();
    CircuitAdapter *adapter = scene->getAdapter();

    // Step 2: Instantiate components
    QJsonArray components = root.value("components").toArray();
    for (const QJsonValue &val : components) {
        QJsonObject compObj = val.toObject();
        QString type = compObj.value("type").toString();
        int id = compObj.value("id").toInt(-1);
        qreal x = compObj.value("x").toDouble();
        qreal y = compObj.value("y").toDouble();

        QGraphicsItem *item = createComponent(type);
        if (!item) continue;

        item->setPos(x, y);

        if (Gate *g = dynamic_cast<Gate*>(item)) {
            if (compObj.contains("inputCount")) {
                g->setInputCount(compObj.value("inputCount").toInt());
            }
        } else if (Switch *sw = dynamic_cast<Switch*>(item)) {
            if (compObj.contains("state")) {
                sw->setValue(compObj.value("state").toBool());
            }
        } else if (LED *led = dynamic_cast<LED*>(item)) {
            if (compObj.contains("state")) {
                led->setState(compObj.value("state").toBool());
            }
        }

        if (compObj.contains("scale")) {
            item->setScale(compObj.value("scale").toDouble());
        }

        scene->addItem(item);
        if (adapter) {
            adapter->registerItem(item, id);
        }
    }

    // Step 3: Reconstruct wires
    QJsonArray wires = root.value("wires").toArray();
    if (adapter) {
        for (const QJsonValue &val : wires) {
            QJsonObject wireObj = val.toObject();
            int startNodeId = wireObj.value("startNodeId").toInt();
            bool startIsOutput = wireObj.value("startIsOutput").toBool();
            int startPinIdx = wireObj.value("startPinIdx").toInt();

            int endNodeId = wireObj.value("endNodeId").toInt();
            bool endIsOutput = wireObj.value("endIsOutput").toBool();
            int endPinIdx = wireObj.value("endPinIdx").toInt();

            Point *startPin = adapter->getPin(startNodeId, startIsOutput, startPinIdx);
            Point *endPin = adapter->getPin(endNodeId, endIsOutput, endPinIdx);

            if (startPin && endPin) {
                Wire *wire = new Wire(startPin, endPin);
                if (wireObj.contains("logicValue")) {
                    wire->setLogicValue(wireObj.value("logicValue").toInt());
                }
                scene->addItem(wire);
                adapter->onWireConnected(wire);
            }
        }

        // Finalize DAG connections
        adapter->extractWires();
    }

    return true;
}

QString CircuitSerializer::generateSvgString(CircuitScene *scene, bool embedMetadata) {
    if (!scene) return QString();

    // Determine bounding rectangle of all circuit elements
    QRectF bounds;
    for (QGraphicsItem *item : scene->items()) {
        if (dynamic_cast<Gate*>(item) || dynamic_cast<Input*>(item) ||
            dynamic_cast<LED*>(item) || dynamic_cast<Wire*>(item)) {
            bounds = bounds.united(item->sceneBoundingRect());
        }
    }

    if (bounds.isEmpty()) {
        bounds = QRectF(0, 0, 800, 600);
    } else {
        bounds.adjust(-40, -40, 40, 40);
    }

    QString svg;
    QTextStream out(&svg);

    out << "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\"?>\n";
    out << QString("<svg width=\"%1\" height=\"%2\" viewBox=\"%3 %4 %5 %6\" "
                   "xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">\n")
               .arg(bounds.width(), 0, 'f', 1)
               .arg(bounds.height(), 0, 'f', 1)
               .arg(bounds.x(), 0, 'f', 1)
               .arg(bounds.y(), 0, 'f', 1)
               .arg(bounds.width(), 0, 'f', 1)
               .arg(bounds.height(), 0, 'f', 1);

    out << "  <title>DLD Circuit Diagram</title>\n";
    out << "  <desc>Generated by DLD Simulator</desc>\n";

    // Background
    out << QString("  <rect x=\"%1\" y=\"%2\" width=\"%3\" height=\"%4\" fill=\"#f8f9fa\" />\n")
               .arg(bounds.x(), 0, 'f', 1)
               .arg(bounds.y(), 0, 'f', 1)
               .arg(bounds.width(), 0, 'f', 1)
               .arg(bounds.height(), 0, 'f', 1);

    // Group: Wires
    out << "  <g id=\"dld-wires\">\n";
    for (QGraphicsItem *item : scene->items()) {
        Wire *wire = dynamic_cast<Wire*>(item);
        if (!wire) continue;

        const auto &segments = wire->getSegments();
        if (segments.size() < 2) continue;

        QString pathD = QString("M %1 %2").arg(segments.first().x(), 0, 'f', 1).arg(segments.first().y(), 0, 'f', 1);
        for (int i = 1; i < segments.size(); ++i) {
            pathD += QString(" L %1 %2").arg(segments[i].x(), 0, 'f', 1).arg(segments[i].y(), 0, 'f', 1);
        }

        QString strokeColor = (wire->getLogicValue() == 1) ? "#00c853" : "#263238";
        qreal strokeWidth = (wire->getLogicValue() == 1) ? 2.5 : 2.0;

        out << QString("    <path d=\"%1\" stroke=\"%2\" stroke-width=\"%3\" fill=\"none\" "
                       "stroke-linecap=\"round\" stroke-linejoin=\"round\" />\n")
                   .arg(pathD)
                   .arg(strokeColor)
                   .arg(strokeWidth);
    }
    out << "  </g>\n";

    // Group: Components
    out << "  <g id=\"dld-components\">\n";
    for (QGraphicsItem *item : scene->items()) {
        if (Gate *gate = dynamic_cast<Gate*>(item)) {
            // Draw input pin lead lines (drawn before gate body so body fill cleanly masks the junction)
            for (Point *pin : gate->getInputPins()) {
                QPointF p = pin->getCenterScenePos();
                qreal endX = 25.0;
                if (dynamic_cast<NotGate*>(gate)) {
                    endX = 5.0;
                } else if (dynamic_cast<XorGate*>(gate) || dynamic_cast<XnorGate*>(gate)) {
                    qreal dy = (p.y() - (gate->pos().y() + 30.0)) / 30.0;
                    endX = 7.0 - 15.0 * (dy * dy);
                }
                out << QString("    <line x1=\"%1\" y1=\"%2\" x2=\"%3\" y2=\"%4\" stroke=\"#000000\" stroke-width=\"2\" stroke-linecap=\"round\" />\n")
                           .arg(p.x(), 0, 'f', 1).arg(p.y(), 0, 'f', 1)
                           .arg(gate->pos().x() + endX, 0, 'f', 1).arg(p.y(), 0, 'f', 1);
            }

            QString pathD = pathToSvgD(gate->getPath(), gate->pos());
            out << QString("    <path d=\"%1\" fill=\"#ffffff\" stroke=\"#000000\" stroke-width=\"2\" />\n")
                       .arg(pathD);

            // Draw output pin lead line
            if (gate->getOutputPin()) {
                QPointF p = gate->getOutputPin()->getCenterScenePos();
                qreal outStartX = gate->pos().x() + 100;
                if (dynamic_cast<NotGate*>(gate)) {
                    outStartX = gate->pos().x() + 90;
                } else if (dynamic_cast<NandGate*>(gate) || dynamic_cast<NorGate*>(gate) || dynamic_cast<XnorGate*>(gate)) {
                    outStartX = gate->pos().x() + 110;
                }
                out << QString("    <line x1=\"%1\" y1=\"%2\" x2=\"%3\" y2=\"%4\" stroke=\"#000000\" stroke-width=\"2\" stroke-linecap=\"round\" />\n")
                           .arg(outStartX, 0, 'f', 1).arg(p.y(), 0, 'f', 1)
                           .arg(p.x(), 0, 'f', 1).arg(p.y(), 0, 'f', 1);
            }

            // Gate pin terminals (dots)
            for (Point *pin : gate->getInputPins()) {
                QPointF p = pin->getCenterScenePos();
                out << QString("    <circle cx=\"%1\" cy=\"%2\" r=\"3.5\" fill=\"#263238\" stroke=\"#000000\" stroke-width=\"1\" />\n")
                           .arg(p.x(), 0, 'f', 1).arg(p.y(), 0, 'f', 1);
            }
            if (gate->getOutputPin()) {
                QPointF p = gate->getOutputPin()->getCenterScenePos();
                out << QString("    <circle cx=\"%1\" cy=\"%2\" r=\"3.5\" fill=\"#263238\" stroke=\"#000000\" stroke-width=\"1\" />\n")
                           .arg(p.x(), 0, 'f', 1).arg(p.y(), 0, 'f', 1);
            }
        } else if (Switch *sw = dynamic_cast<Switch*>(item)) {
            QPointF p = sw->pos();
            QString fill = sw->getValue() ? "#00c853" : "#ffffff";
            // Terminal lead line connecting switch circle to output pin
            out << QString("    <line x1=\"%1\" y1=\"%2\" x2=\"%3\" y2=\"%4\" stroke=\"#000000\" stroke-width=\"2\" stroke-linecap=\"round\" />\n")
                       .arg(p.x() + 40, 0, 'f', 1).arg(p.y() + 25, 0, 'f', 1)
                       .arg(p.x() + 55, 0, 'f', 1).arg(p.y() + 25, 0, 'f', 1);
            out << QString("    <circle cx=\"%1\" cy=\"%2\" r=\"15\" fill=\"%3\" stroke=\"#000000\" stroke-width=\"2\" />\n")
                       .arg(p.x() + 25, 0, 'f', 1).arg(p.y() + 25, 0, 'f', 1).arg(fill);
            out << QString("    <text x=\"%1\" y=\"%2\" font-family=\"Ubuntu, sans-serif\" font-size=\"11\" "
                           "font-weight=\"bold\" text-anchor=\"middle\" dominant-baseline=\"central\">0/1</text>\n")
                       .arg(p.x() + 25, 0, 'f', 1).arg(p.y() + 25, 0, 'f', 1);
            if (sw->getOutputPoint()) {
                QPointF pinPos = sw->getOutputPoint()->getCenterScenePos();
                out << QString("    <circle cx=\"%1\" cy=\"%2\" r=\"3.5\" fill=\"#263238\" stroke=\"#000000\" stroke-width=\"1\" />\n")
                           .arg(pinPos.x(), 0, 'f', 1).arg(pinPos.y(), 0, 'f', 1);
            }
        } else if (dynamic_cast<Const1*>(item)) {
            QPointF p = item->pos();
            // Terminal lead line connecting const1 rect to output pin
            out << QString("    <line x1=\"%1\" y1=\"%2\" x2=\"%3\" y2=\"%4\" stroke=\"#000000\" stroke-width=\"2\" stroke-linecap=\"round\" />\n")
                       .arg(p.x() + 40, 0, 'f', 1).arg(p.y() + 25, 0, 'f', 1)
                       .arg(p.x() + 55, 0, 'f', 1).arg(p.y() + 25, 0, 'f', 1);
            out << QString("    <rect x=\"%1\" y=\"%2\" width=\"30\" height=\"30\" fill=\"#ffffff\" stroke=\"#000000\" stroke-width=\"2\" rx=\"4\" />\n")
                       .arg(p.x() + 10, 0, 'f', 1).arg(p.y() + 10, 0, 'f', 1);
            out << QString("    <text x=\"%1\" y=\"%2\" font-family=\"Ubuntu, sans-serif\" font-size=\"14\" "
                           "font-weight=\"bold\" text-anchor=\"middle\" dominant-baseline=\"central\">1</text>\n")
                       .arg(p.x() + 25, 0, 'f', 1).arg(p.y() + 25, 0, 'f', 1);
            if (Input *inp = dynamic_cast<Input*>(item)) {
                if (inp->getOutputPoint()) {
                    QPointF pinPos = inp->getOutputPoint()->getCenterScenePos();
                    out << QString("    <circle cx=\"%1\" cy=\"%2\" r=\"3.5\" fill=\"#263238\" stroke=\"#000000\" stroke-width=\"1\" />\n")
                               .arg(pinPos.x(), 0, 'f', 1).arg(pinPos.y(), 0, 'f', 1);
                }
            }
        } else if (dynamic_cast<GND*>(item)) {
            QPointF p = item->pos();
            // Terminal lead line to pin
            out << QString("    <line x1=\"%1\" y1=\"%2\" x2=\"%3\" y2=\"%4\" stroke=\"#000000\" stroke-width=\"2\" stroke-linecap=\"round\" />\n")
                       .arg(p.x() + 20, 0, 'f', 1).arg(p.y() + 15, 0, 'f', 1)
                       .arg(p.x() + 55, 0, 'f', 1).arg(p.y() + 15, 0, 'f', 1);
            // Stem down to ground plates
            out << QString("    <line x1=\"%1\" y1=\"%2\" x2=\"%3\" y2=\"%4\" stroke=\"#000000\" stroke-width=\"2\" stroke-linecap=\"round\" />\n")
                       .arg(p.x() + 20, 0, 'f', 1).arg(p.y() + 15, 0, 'f', 1)
                       .arg(p.x() + 20, 0, 'f', 1).arg(p.y() + 27, 0, 'f', 1);
            out << QString("    <line x1=\"%1\" y1=\"%2\" x2=\"%3\" y2=\"%4\" stroke=\"#000000\" stroke-width=\"2\" stroke-linecap=\"round\" />\n")
                       .arg(p.x() + 4, 0, 'f', 1).arg(p.y() + 27, 0, 'f', 1)
                       .arg(p.x() + 36, 0, 'f', 1).arg(p.y() + 27, 0, 'f', 1);
            out << QString("    <line x1=\"%1\" y1=\"%2\" x2=\"%3\" y2=\"%4\" stroke=\"#000000\" stroke-width=\"2\" stroke-linecap=\"round\" />\n")
                       .arg(p.x() + 8, 0, 'f', 1).arg(p.y() + 33, 0, 'f', 1)
                       .arg(p.x() + 32, 0, 'f', 1).arg(p.y() + 33, 0, 'f', 1);
            out << QString("    <line x1=\"%1\" y1=\"%2\" x2=\"%3\" y2=\"%4\" stroke=\"#000000\" stroke-width=\"2\" stroke-linecap=\"round\" />\n")
                       .arg(p.x() + 12, 0, 'f', 1).arg(p.y() + 39, 0, 'f', 1)
                       .arg(p.x() + 28, 0, 'f', 1).arg(p.y() + 39, 0, 'f', 1);
            out << QString("    <line x1=\"%1\" y1=\"%2\" x2=\"%3\" y2=\"%4\" stroke=\"#000000\" stroke-width=\"2\" stroke-linecap=\"round\" />\n")
                       .arg(p.x() + 16, 0, 'f', 1).arg(p.y() + 45, 0, 'f', 1)
                       .arg(p.x() + 24, 0, 'f', 1).arg(p.y() + 45, 0, 'f', 1);
            if (Input *inp = dynamic_cast<Input*>(item)) {
                if (inp->getOutputPoint()) {
                    QPointF pinPos = inp->getOutputPoint()->getCenterScenePos();
                    out << QString("    <circle cx=\"%1\" cy=\"%2\" r=\"3.5\" fill=\"#263238\" stroke=\"#000000\" stroke-width=\"1\" />\n")
                               .arg(pinPos.x(), 0, 'f', 1).arg(pinPos.y(), 0, 'f', 1);
                }
            }
        } else if (LED *led = dynamic_cast<LED*>(item)) {
            QPointF p = led->pos();
            QString bulbFill = led->getState() ? "#ffeb3b" : "#ffffff";
            QString collarFill = led->getState() ? "#fff596" : "#ffffff";

            // Terminal lead wire from input pin to bulb body
            out << QString("    <line x1=\"%1\" y1=\"%2\" x2=\"%3\" y2=\"%4\" stroke=\"#000000\" stroke-width=\"2\" stroke-linecap=\"round\" />\n")
                       .arg(p.x() - 5, 0, 'f', 1).arg(p.y() + 25, 0, 'f', 1)
                       .arg(p.x() + 17.5, 0, 'f', 1).arg(p.y() + 25, 0, 'f', 1);

            // Glass bulb body (dome shape)
            out << QString("    <path d=\"M %1 %2 C %3 %4, %5 %6, %7 %8 C %9 %10, %11 %12, %13 %14 C %15 %16, %17 %18, %19 %20 C %21 %22, %23 %24, %25 %26 Z\" "
                           "fill=\"%27\" stroke=\"#000000\" stroke-width=\"2\" stroke-linejoin=\"round\" />\n")
                       .arg(p.x() + 20.5, 0, 'f', 1).arg(p.y() + 33.5, 0, 'f', 1)   // moveTo
                       .arg(p.x() + 20.0, 0, 'f', 1).arg(p.y() + 27.0, 0, 'f', 1)   // cubicTo cp1
                       .arg(p.x() + 14.0, 0, 'f', 1).arg(p.y() + 23.0, 0, 'f', 1)   // cubicTo cp2
                       .arg(p.x() + 14.0, 0, 'f', 1).arg(p.y() + 17.0, 0, 'f', 1)   // cubicTo end
                       .arg(p.x() + 14.0, 0, 'f', 1).arg(p.y() + 9.3, 0, 'f', 1)    // cubicTo cp1
                       .arg(p.x() + 20.3, 0, 'f', 1).arg(p.y() + 3.0, 0, 'f', 1)    // cubicTo cp2
                       .arg(p.x() + 28.0, 0, 'f', 1).arg(p.y() + 3.0, 0, 'f', 1)    // cubicTo end
                       .arg(p.x() + 35.7, 0, 'f', 1).arg(p.y() + 3.0, 0, 'f', 1)    // cubicTo cp1
                       .arg(p.x() + 42.0, 0, 'f', 1).arg(p.y() + 9.3, 0, 'f', 1)    // cubicTo cp2
                       .arg(p.x() + 42.0, 0, 'f', 1).arg(p.y() + 17.0, 0, 'f', 1)   // cubicTo end
                       .arg(p.x() + 42.0, 0, 'f', 1).arg(p.y() + 23.0, 0, 'f', 1)   // cubicTo cp1
                       .arg(p.x() + 36.0, 0, 'f', 1).arg(p.y() + 27.0, 0, 'f', 1)   // cubicTo cp2
                       .arg(p.x() + 35.5, 0, 'f', 1).arg(p.y() + 33.5, 0, 'f', 1)   // cubicTo end
                       .arg(bulbFill);

            // Socket collar inside bulb
            out << QString("    <rect x=\"%1\" y=\"%2\" width=\"5\" height=\"4\" fill=\"%3\" stroke=\"#000000\" stroke-width=\"1.5\" />\n")
                       .arg(p.x() + 25.5, 0, 'f', 1).arg(p.y() + 29.5, 0, 'f', 1).arg(collarFill);

            // Filament supports
            out << QString("    <line x1=\"%1\" y1=\"%2\" x2=\"%3\" y2=\"%4\" stroke=\"#000000\" stroke-width=\"1.5\" stroke-linecap=\"round\" />\n")
                       .arg(p.x() + 26.5, 0, 'f', 1).arg(p.y() + 29.5, 0, 'f', 1)
                       .arg(p.x() + 23.0, 0, 'f', 1).arg(p.y() + 20.0, 0, 'f', 1);
            out << QString("    <line x1=\"%1\" y1=\"%2\" x2=\"%3\" y2=\"%4\" stroke=\"#000000\" stroke-width=\"1.5\" stroke-linecap=\"round\" />\n")
                       .arg(p.x() + 29.5, 0, 'f', 1).arg(p.y() + 29.5, 0, 'f', 1)
                       .arg(p.x() + 33.0, 0, 'f', 1).arg(p.y() + 20.0, 0, 'f', 1);

            // Zigzag filament
            out << QString("    <path d=\"M %1 %2 L %3 %4 L %5 %6 L %7 %8 L %9 %10 L %11 %12 L %13 %14\" "
                           "fill=\"none\" stroke=\"#000000\" stroke-width=\"1.5\" stroke-linecap=\"round\" stroke-linejoin=\"round\" />\n")
                       .arg(p.x() + 23.0, 0, 'f', 1).arg(p.y() + 20.0, 0, 'f', 1)
                       .arg(p.x() + 24.7, 0, 'f', 1).arg(p.y() + 18.2, 0, 'f', 1)
                       .arg(p.x() + 26.3, 0, 'f', 1).arg(p.y() + 20.0, 0, 'f', 1)
                       .arg(p.x() + 28.0, 0, 'f', 1).arg(p.y() + 18.2, 0, 'f', 1)
                       .arg(p.x() + 29.7, 0, 'f', 1).arg(p.y() + 20.0, 0, 'f', 1)
                       .arg(p.x() + 31.3, 0, 'f', 1).arg(p.y() + 18.2, 0, 'f', 1)
                       .arg(p.x() + 33.0, 0, 'f', 1).arg(p.y() + 20.0, 0, 'f', 1);

            // Screw base threads (3 rounded ribs)
            out << QString("    <rect x=\"%1\" y=\"%2\" width=\"12.4\" height=\"3\" rx=\"1.5\" ry=\"1.5\" fill=\"#ffffff\" stroke=\"#000000\" stroke-width=\"1.5\" />\n")
                       .arg(p.x() + 21.8, 0, 'f', 1).arg(p.y() + 34.0, 0, 'f', 1);
            out << QString("    <rect x=\"%1\" y=\"%2\" width=\"12\" height=\"3\" rx=\"1.5\" ry=\"1.5\" fill=\"#ffffff\" stroke=\"#000000\" stroke-width=\"1.5\" />\n")
                       .arg(p.x() + 22.0, 0, 'f', 1).arg(p.y() + 37.2, 0, 'f', 1);
            out << QString("    <rect x=\"%1\" y=\"%2\" width=\"11\" height=\"3\" rx=\"1.5\" ry=\"1.5\" fill=\"#ffffff\" stroke=\"#000000\" stroke-width=\"1.5\" />\n")
                       .arg(p.x() + 22.5, 0, 'f', 1).arg(p.y() + 40.4, 0, 'f', 1);

            // Bottom base contact terminal
            out << QString("    <path d=\"M %1 %2 Q %3 %4, %5 %6 Z\" fill=\"#323232\" stroke=\"#000000\" stroke-width=\"1.5\" stroke-linejoin=\"round\" />\n")
                       .arg(p.x() + 24.0, 0, 'f', 1).arg(p.y() + 43.4, 0, 'f', 1)
                       .arg(p.x() + 28.0, 0, 'f', 1).arg(p.y() + 47.0, 0, 'f', 1)
                       .arg(p.x() + 32.0, 0, 'f', 1).arg(p.y() + 43.4, 0, 'f', 1);

            // Input pin dot
            if (led->getInputPoint()) {
                QPointF pinPos = led->getInputPoint()->getCenterScenePos();
                out << QString("    <circle cx=\"%1\" cy=\"%2\" r=\"3.5\" fill=\"#263238\" stroke=\"#000000\" stroke-width=\"1\" />\n")
                           .arg(pinPos.x(), 0, 'f', 1).arg(pinPos.y(), 0, 'f', 1);
            }
        }
    }
    out << "  </g>\n";

    // Group: Embedded Circuit Metadata for Smart SVG
    if (embedMetadata) {
        QByteArray jsonNetlist = serializeToJson(scene);
        out << "\n  <!-- Embedded DLDSimulator Circuit Metadata for Interactive Re-import -->\n";
        out << "  <metadata id=\"dld-circuit-state\">\n";
        out << "  <![CDATA[\n";
        out << QString::fromUtf8(jsonNetlist);
        out << "\n  ]]>\n";
        out << "  </metadata>\n";
    }

    out << "</svg>\n";
    return svg;
}

bool CircuitSerializer::saveToFile(const QString &filePath, CircuitScene *scene, QString *errorMessage) {
    if (filePath.endsWith(".dld", Qt::CaseInsensitive) || filePath.endsWith(".json", Qt::CaseInsensitive)) {
        QFile file(filePath);
        if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
            if (errorMessage) {
                *errorMessage = QString("Failed to open file for writing: %1").arg(file.errorString());
            }
            return false;
        }

        QByteArray jsonData = serializeToJson(scene);
        qint64 bytesWritten = file.write(jsonData);
        file.close();

        if (bytesWritten != jsonData.size()) {
            if (errorMessage) {
                *errorMessage = "Failed to write complete circuit data to file.";
            }
            return false;
        }
        return true;
    }

    // Default to Smart SVG with embedded metadata
    return exportToSvg(filePath, scene, true, errorMessage);
}

bool CircuitSerializer::loadFromFile(const QString &filePath, CircuitScene *scene, QString *errorMessage) {
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        if (errorMessage) {
            *errorMessage = QString("Failed to open file for reading: %1").arg(file.errorString());
        }
        return false;
    }

    QByteArray fileData = file.readAll();
    file.close();

    // Check if the file is a Smart SVG containing embedded metadata
    QString content = QString::fromUtf8(fileData);
    int metaStart = content.indexOf("<metadata id=\"dld-circuit-state\">");
    if (metaStart == -1) {
        metaStart = content.indexOf("<metadata>");
    }

    if (metaStart != -1) {
        int tagEnd = content.indexOf(">", metaStart);
        int metaEnd = content.indexOf("</metadata>", tagEnd);
        if (tagEnd != -1 && metaEnd != -1) {
            QString inner = content.mid(tagEnd + 1, metaEnd - (tagEnd + 1)).trimmed();
            // Remove CDATA if present
            if (inner.startsWith("<![CDATA[")) {
                inner = inner.mid(9);
            }
            if (inner.endsWith("]]>")) {
                inner.chop(3);
            }
            return deserializeFromJson(inner.trimmed().toUtf8(), scene, errorMessage);
        }
    }

    // Otherwise, treat directly as JSON (.dld or .json)
    return deserializeFromJson(fileData, scene, errorMessage);
}

bool CircuitSerializer::exportToSvg(const QString &filePath, CircuitScene *scene, bool embedMetadata, QString *errorMessage) {
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)) {
        if (errorMessage) {
            *errorMessage = QString("Failed to open SVG file for writing: %1").arg(file.errorString());
        }
        return false;
    }

    QString svgContent = generateSvgString(scene, embedMetadata);
    QTextStream out(&file);
    out << svgContent;
    file.close();

    return true;
}
