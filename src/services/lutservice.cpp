#include "services/lutservice.h"
#include <QFile>
#include <QTextStream>

QMutex LutService::s_lutCacheMutex;
QHash<QString, LutData> LutService::s_lutCache;

std::optional<LutData> LutService::load(const QString& path)
{
    QFile f(path);
    if (!f.open(QIODevice::ReadOnly | QIODevice::Text))
        return std::nullopt;

    LutData lut;
    QTextStream in(&f);
    while (!in.atEnd()) {
        QString line = in.readLine().trimmed();
        if (line.startsWith('#') || line.isEmpty())
            continue;
        if (line.startsWith("LUT_3D_SIZE")) {
            lut.size = line.split(' ').last().toInt();
            lut.table.reserve(lut.size * lut.size * lut.size);
            continue;
        }
        const auto parts = QStringView { line }.split(' ', Qt::SkipEmptyParts);
        if (parts.size() == 3)
            lut.table.append({ parts[0].toFloat(), parts[1].toFloat(), parts[2].toFloat() });
    }
    return lut;
}

std::optional<LutData> LutService::loadCached(const QString& path)
{
    {
        QMutexLocker locker(&s_lutCacheMutex);
        auto it = s_lutCache.constFind(path);
        if (it != s_lutCache.constEnd())
            return *it;
    }
    auto lut = load(path);
    if (lut) {
        QMutexLocker locker(&s_lutCacheMutex);
        s_lutCache.insert(path, *lut);
    }
    return lut;
}

QImage LutService::lutToTex(const LutData& lut)
{
    int dim = lut.size;
    QImage lutImage(dim * dim, dim, QImage::Format_RGB32);

    int index = 0;
    for (int z = 0; z < dim; ++z) {
        for (int y = 0; y < dim; ++y) {
            for (int x = 0; x < dim; ++x) {
                const QVector3D& color = lut.table[index++];

                int r = static_cast<int>(qBound(0.0f, color.x(), 1.0f) * 255.0f);
                int g = static_cast<int>(qBound(0.0f, color.y(), 1.0f) * 255.0f);
                int b = static_cast<int>(qBound(0.0f, color.z(), 1.0f) * 255.0f);

                int pixelX = x + (z * dim);
                int pixelY = y;

                lutImage.setPixel(pixelX, pixelY, qRgb(r, g, b));
            }
        }
    }
    return lutImage;
}
