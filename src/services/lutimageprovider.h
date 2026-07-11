#include <QQuickImageProvider>
#include <QUrl> // <-- Upewnij się, że masz ten nagłówek
#include "lutservice.h"

class LutImageProvider : public QQuickImageProvider
{
public:
    LutImageProvider() : QQuickImageProvider(QQuickImageProvider::Image) {}

    QImage requestImage(const QString &id, QSize *size, const QSize &requestedSize) override
    {
        // 1. Odkodowanie bezpiecznego adresu URL z powrotem na normalną ścieżkę (np. ":/luts/filtr.cube")
        const QString decodedPath = QUrl::fromPercentEncoding(id.toUtf8());
        qDebug() << QString("id (raw)    =") <<  QString(id);
        qDebug() << QString("decodedPath =") << QString(decodedPath);
        qDebug() << QString("identyczne? =") << (id == decodedPath);
        // 2. Wczytanie odkodowanej ścieżki
        auto lutData = LutService::load(decodedPath);
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
