#ifndef LUTSERVICE_H
#define LUTSERVICE_H

#include <QFutureWatcher>
#include <QHash>
#include <QImage>
#include <QMutex>
#include <QVector3D>
#include <QVector>
#include <optional>
#include <qhashfunctions.h>

struct LutData {
    int size = 0;
    QVector<QVector3D> table;
};

class LutService : public QObject {
    Q_OBJECT
public:
    explicit LutService(QObject* parent = nullptr);

    static std::optional<LutData> loadCached(const QString& path);

    static std::optional<LutData> load(const QString& path);

    static QImage apply(const QImage& src, const LutData& lut);
    static QImage processWallpaperOpenCV(const QImage& source, double hOffset, double bOffset, double sOffset, bool flipped, const LutData* lut);

    // ZMIANA: przyjmuje teraz ścieżkę do pliku .cube zamiast gotowego LutData —
    // wczytanie i sparsowanie LUT-a też jest blokującym I/O, więc ma się dziać
    // na wątku roboczym, a nie na wątku UI (patrz DetailViewModel::reprocessAsync)
    void applyChangesAsync(const QString path, double hOffset, double bOffset, double sOffset, bool flipped, const QString& lutPath);

    void cancel();

    static QImage lutToTex(const LutData& lut);

signals:
    void lutApplied(const QImage& result);
    void lutFailed(const QString& error);

private:
    QFutureWatcher<QImage> m_watcher;

    static QMutex s_lutCacheMutex;
    static QHash<QString, LutData> s_lutCache;
};

#endif // LUTSERVICE_H
