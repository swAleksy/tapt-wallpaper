#ifndef LUTSERVICE_H
#define LUTSERVICE_H

#include <QImage>
#include <QVector>
#include <QVector3D>

struct LutData {
    int size;
    QVector<QVector3D> table;
};

class LutService {
public:
    static std::optional<LutData> load(const QString &path);
    static QImage apply(const QImage &src, const LutData &lut);
};

#endif // LUTSERVICE_H
