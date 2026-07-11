#include <QFutureWatcher>
#include <QImage>
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
            QImage img(id);
            if (abortFlag->load())
                return QImage();

            if (!img.isNull() && requestedSize.isValid()) {
                img = img.scaled(requestedSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
            }
            return img;
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
