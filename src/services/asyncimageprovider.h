#ifndef ASYNCIMAGERESPONSE_H
#define ASYNCIMAGERESPONSE_H

#include <QFutureWatcher>
#include <QImage>
#include <QImageReader>
#include <QQuickAsyncImageProvider>
#include <QQuickImageResponse>
#include <QQuickTextureFactory>
#include <QtConcurrent>

class AsyncImageResponse : public QQuickImageResponse {
    Q_OBJECT
public:
    QImage m_image;
    QFutureWatcher<QImage> m_watcher;
    std::shared_ptr<std::atomic<bool>> m_abortFlag;

    AsyncImageResponse(const QString& id, const QSize& requestedSize)
    {
        m_abortFlag = std::make_shared<std::atomic<bool>>(false);
        auto abortFlag = m_abortFlag;

        connect(&m_watcher, &QFutureWatcher<QImage>::finished, this, &AsyncImageResponse::handleFinished);

        QFuture<QImage> future = QtConcurrent::run([id, requestedSize, abortFlag]() {
            if (abortFlag->load())
                return QImage();

            QImageReader reader(id);
            // Honoruje orientację EXIF przy okazji — poprzednia wersja (QImage(id))
            // robiła to automatycznie tylko dla formatów obsługujących auto-transform
            // domyślnie; tu ustawiamy to jawnie.
            reader.setAutoTransform(true);

            if (requestedSize.isValid() && !requestedSize.isEmpty()) {
                const QSize nativeSize = reader.size(); // czyta nagłówek, nie dekoduje pikseli
                if (nativeSize.isValid() && !nativeSize.isEmpty()) {
                    // KeepAspectRatio ręcznie — setScaledSize() samo nie zachowuje proporcji
                    reader.setScaledSize(nativeSize.scaled(requestedSize, Qt::KeepAspectRatio));
                }
                // jeśli nativeSize nieznany (np. reader nie umie odczytać nagłówka bez
                // pełnego dekodu dla danego formatu) — celowo NIE ustawiamy scaledSize
                // i lecimy pełnym odczytem niżej; unikamy zgadywania rozmiaru.
            }

            if (abortFlag->load())
                return QImage();

            return reader.read(); // QImage() jeśli się nie uda — obsłużone niżej jak wcześniej
        });

        m_watcher.setFuture(future);
    }

    ~AsyncImageResponse() override
    {
        m_watcher.disconnect();
        m_abortFlag->store(true);
    }

    QQuickTextureFactory* textureFactory() const override { return QQuickTextureFactory::textureFactoryForImage(m_image); }

private slots:
    void handleFinished()
    {
        m_image = m_watcher.result();
        emit finished();
    }
};

class TaptImageProvider : public QQuickAsyncImageProvider {
public:
    QQuickImageResponse* requestImageResponse(const QString& id, const QSize& requestedSize) override
    {
        const QString path = QUrl::fromPercentEncoding(id.toUtf8());
        return new AsyncImageResponse(path, requestedSize.isValid() ? requestedSize : QSize(256, 256));
    }
};


#endif // ASYNCIMAGERESPONSE_H
