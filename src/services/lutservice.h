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

    // ZMIANA: Przekazujemy LutData przez wartość (bezpieczeństwo wątkowe), dodano brakującą metodę apply
    static QImage apply(const QImage &src, const LutData &lut);
    static QImage processWallpaperOpenCV(const QImage &source, double hOffset, double bOffset, double sOffset, const LutData *lut);

    void applyChangesAsync(const QImage &source, double hOffset, double bOffset, double sOffset, const LutData lut);

    static QImage lutToTex(const LutData &lut);

signals:
    // ZMIANA: Dodano brakujące sygnały, które emitujesz w .cpp
    void lutApplied(const QImage &result);
    void lutFailed(const QString &error);

private:
    QFutureWatcher<QImage> m_watcher;
};

#endif // LUTSERVICE_H
