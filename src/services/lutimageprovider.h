#include <QQuickImageProvider>
#include "lutservice.h"

class LutImageProvider : public QQuickImageProvider
{
public:
    LutImageProvider() : QQuickImageProvider(QQuickImageProvider::Image) {}

    QImage requestImage(const QString &id, QSize *size, const QSize &requestedSize) override
    {
        // 'id' to będzie ścieżka do pliku .cube przekazana z QML
        // Np. jeśli w QML wpiszemy "image://lut/C:/filtry/ColdChrome.cube"

        auto lutData = LutService::load(id);
        if (!lutData) {
            return QImage(); // Jeśli plik nie istnieje, zwróć pusty obraz
        }

        QImage lutTex = LutService::lutToTex(*lutData);

        if (size) {
            *size = lutTex.size();
        }
        return lutTex;
    }
};
