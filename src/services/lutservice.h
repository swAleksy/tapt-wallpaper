#ifndef LUTSERVICE_H
#define LUTSERVICE_H

#include <QImage>
#include <QVector>
#include <QVector3D>
#include <QFutureWatcher>
#include <optional>

struct LutData {
    int size;
    QVector<QVector3D> table;
};

class LutService : public QObject
{
    Q_OBJECT
public:
    explicit LutService(QObject *parent = nullptr);
    static std::optional<LutData> load(const QString &path);

    static QImage apply(const QImage &src, const LutData &lut);
    static QImage processWallpaperOpenCV(const QImage &source, double hOffset, double bOffset, double sOffset, const LutData *lut);

    // ZMIANA: przyjmuje teraz ścieżkę do pliku .cube zamiast gotowego LutData —
    // wczytanie i sparsowanie LUT-a też jest blokującym I/O, więc ma się dziać
    // na wątku roboczym, a nie na wątku UI (patrz DetailViewModel::reprocessAsync)
    void applyChangesAsync(const QImage &source, double hOffset, double bOffset, double sOffset, const QString &lutPath);

    static QImage lutToTex(const LutData &lut);

signals:
    void lutApplied(const QImage &result);
    void lutFailed(const QString &error);

private:
    QFutureWatcher<QImage> m_watcher;
};

#endif // LUTSERVICE_H
