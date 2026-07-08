#ifndef LUTSERVICE_H
#define LUTSERVICE_H

#include <QImage>
#include <QVector>
#include <QVector3D>
#include <QFutureWatcher>
#include <optional>
#include <QHash>
#include <QMutex>

struct LutData {
    int size = 0;
    QVector<QVector3D> table;
};

class LutService : public QObject
{
    Q_OBJECT
public:
    explicit LutService(QObject *parent = nullptr);
    static std::optional<LutData> load(const QString &path);

    static QImage apply(const QImage &src, const LutData &lut);
    static QImage processWallpaperOpenCV(const QImage &source, double hOffset, double bOffset, double sOffset, bool flipped, const LutData *lut);

    // ZMIANA: przyjmuje teraz ścieżkę do pliku .cube zamiast gotowego LutData —
    // wczytanie i sparsowanie LUT-a też jest blokującym I/O, więc ma się dziać
    // na wątku roboczym, a nie na wątku UI (patrz DetailViewModel::reprocessAsync)
    void applyChangesAsync(const QImage &source, double hOffset, double bOffset, double sOffset, bool flipped, const QString &lutPath);

    void cancel();

    static QImage lutToTex(const LutData &lut);

signals:
    void lutApplied(const QImage &result);
    void lutFailed(const QString &error);

private:
    QFutureWatcher<QImage> m_watcher;

    QMutex m_lutCacheMutex;
    QHash<QString, LutData> m_lutCache;
};

#endif // LUTSERVICE_H
