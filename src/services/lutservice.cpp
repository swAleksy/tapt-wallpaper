#include "lutservice.h"
#include <qdir.h>

std::optional<LutData> LutService::load(const QString &path)
{
    QFile f(path);
    if (!f.open(QIODevice::ReadOnly | QIODevice::Text))
        return std::nullopt;

    LutData lut;
    QTextStream in(&f);
    while (!in.atEnd()) {
        QString line = in.readLine().trimmed();
        if (line.startsWith('#') || line.isEmpty()) continue;
        if (line.startsWith("LUT_3D_SIZE")) {
            lut.size = line.split(' ').last().toInt();
            lut.table.reserve(lut.size * lut.size * lut.size);
            continue;
        }
        QStringList parts = line.split(' ');
        if (parts.size() == 3)
            lut.table.append({ parts[0].toFloat(),
                               parts[1].toFloat(),
                               parts[2].toFloat() });
    }
    return lut;
}

QImage LutService::apply(const QImage &src, const LutData &lut)
{
    QImage dst = src.convertToFormat(QImage::Format_RGB32);
    const int N = lut.size - 1;

    for (int y = 0; y < dst.height(); ++y) {
        QRgb *row = reinterpret_cast<QRgb*>(dst.scanLine(y));
        for (int x = 0; x < dst.width(); ++x) {
            QColor c(row[x]);
            // mapuj [0..255] → indeks siatki
            float rf = c.redF()   * N;
            float gf = c.greenF() * N;
            float bf = c.blueF()  * N;
            // trilinear interpolation (prosta wersja — floor)
            int ri = qBound(0, (int)rf, N);
            int gi = qBound(0, (int)gf, N);
            int bi = qBound(0, (int)bf, N);
            // indeks w tabeli: B zmienia się najszybciej (.cube spec)
            const QVector3D &out = lut.table[ri*lut.size*lut.size + gi*lut.size + bi];
            row[x] = qRgb(out.x()*255, out.y()*255, out.z()*255);
        }
    }
    return dst;
}
