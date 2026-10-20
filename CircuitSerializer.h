#ifndef CIRCUITSERIALIZER_H
#define CIRCUITSERIALIZER_H

#include <QString>
#include <QByteArray>

class CircuitScene;

class CircuitSerializer {
public:
    // Save circuit to file (.dld, .json, or Smart .svg)
    static bool saveToFile(const QString &filePath, CircuitScene *scene, QString *errorMessage = nullptr);

    // Load circuit from file (.dld, .json, or Smart .svg with metadata)
    static bool loadFromFile(const QString &filePath, CircuitScene *scene, QString *errorMessage = nullptr);

    // Export circuit as standalone or Smart vector SVG
    static bool exportToSvg(const QString &filePath, CircuitScene *scene, bool embedMetadata = true, QString *errorMessage = nullptr);

    // Generate JSON representation of the circuit
    static QByteArray serializeToJson(CircuitScene *scene);

    // Restore circuit from JSON byte array
    static bool deserializeFromJson(const QByteArray &jsonData, CircuitScene *scene, QString *errorMessage = nullptr);

    // Generate SVG string representing the circuit
    static QString generateSvgString(CircuitScene *scene, bool embedMetadata = true);
};

#endif // CIRCUITSERIALIZER_H
