#ifndef ASYNCIMAGERESPONSE_H
#define ASYNCIMAGERESPONSE_H

#include <QFutureWatcher>
#include <QImage>
#include <QImageReader>
#include <QQuickAsyncImageProvider>
#include <QQuickImageResponse>
#include <QQuickTextureFactory>
#include <QtConcurrent>
#include <memory>
#include <atomic>

class AsyncImageResponse : public QQuickImageResponse {
    Q_OBJECT
public:
    QImage m_image;
    QString m_errorString;
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
            // Honoruje orientację EXIF
            reader.setAutoTransform(true);

            if (requestedSize.isValid() && !requestedSize.isEmpty()) {
                const QSize nativeSize = reader.size();
                if (nativeSize.isValid() && !nativeSize.isEmpty()) {
                    reader.setScaledSize(nativeSize.scaled(requestedSize, Qt::KeepAspectRatioByExpanding));
                }
            }

            if (abortFlag->load())
                return QImage();

            return reader.read();
        });

        m_watcher.setFuture(future);
    }

    ~AsyncImageResponse() override
    {
        m_watcher.disconnect();
        m_abortFlag->store(true);
    }

    // POPRAWKA 1: Natychmiastowe ubicie procesu na żądanie QML (np. przy scrollowaniu)
    void cancel() override
    {
        m_abortFlag->store(true);
    }

    QString errorString() const override
    {
        return m_errorString;
    }

    QQuickTextureFactory* textureFactory() const override
    {
        return QQuickTextureFactory::textureFactoryForImage(m_image);
    }

private slots:
    void handleFinished()
    {
        m_image = m_watcher.result();
        if (m_image.isNull())
            m_errorString = QStringLiteral("nie udało się wczytać obrazu");  // NOWE
        emit finished();
    }
};

class TaptImageProvider : public QQuickAsyncImageProvider {
public:
    QQuickImageResponse* requestImageResponse(const QString& id, const QSize& requestedSize) override
    {
        const QString path = QUrl::fromPercentEncoding(id.toUtf8());
        // POPRAWKA 2: Usunięto sztywny fallback do wymuszania 256x256,
        // pozwalamy silnikowi zadecydować lub wczytać pełny rozmiar.
        return new AsyncImageResponse(path, requestedSize);
    }
};

#endif // ASYNCIMAGERESPONSE_H
