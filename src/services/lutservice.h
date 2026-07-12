#ifndef LUTSERVICE_H
#define LUTSERVICE_H

#include <QHash>
#include <QImage>
#include <QMutex>
#include <QVector3D>
#include <QVector>
#include <QString>
#include <optional>

struct LutData {
    int size = 0;
    QVector<QVector3D> table;
};

class LutService {
public:
    // Blokujemy możliwość tworzenia instancji tej klasy (opcjonalne, ale to dobra praktyka)
    LutService() = delete;

    static std::optional<LutData> loadCached(const QString& path);
    static std::optional<LutData> load(const QString& path);
    static QImage lutToTex(const LutData& lut);

private:
    static QMutex s_lutCacheMutex;
    static QHash<QString, LutData> s_lutCache;
};

#endif // LUTSERVICE_H
