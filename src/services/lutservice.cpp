#include "lutservice.h"
#include <QFile>
#include <QPromise>
#include <QTextStream>
#include <QtConcurrent>
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <qhashfunctions.h>
#include <qimage.h>

QMutex LutService::s_lutCacheMutex;
QHash<QString, LutData> LutService::s_lutCache;

LutService::LutService(QObject* parent)
    : QObject(parent)
{
    connect(&m_watcher, &QFutureWatcher<QImage>::finished, this, [this]() {
        // ZMIANA: zadanie unieważnione przez nowsze żądanie (patrz applyChangesAsync)
        // — nie jest to błąd, po prostu wynik nikogo już nie interesuje.
        if (m_watcher.isCanceled())
            return;

        QImage result = m_watcher.result();
        if (!result.isNull()) {
            emit lutApplied(result);
        } else {
            emit lutFailed("LUT processing failed (empty image)");
        }
    });
}

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

QImage LutService::processWallpaperOpenCV(const QImage& source, double hOffset, double bOffset, double sOffset, bool flipped, const LutData* lut)
{
    QImage rgbImg = source.convertToFormat(QImage::Format_RGB888).copy();
    cv::Mat mat(rgbImg.height(), rgbImg.width(), CV_8UC3, (void*)rgbImg.bits(), rgbImg.bytesPerLine());
    cv::Mat cvtMat;
    cv::cvtColor(mat, cvtMat, cv::COLOR_RGB2BGR);

    cv::cvtColor(cvtMat, cvtMat, cv::COLOR_BGR2HSV);

    std::vector<cv::Mat> channels;
    cv::split(cvtMat, channels);

    int cvHueOffset = static_cast<int>(hOffset * 180.0);
    int cvSatOffset = static_cast<int>(sOffset * 255.0);
    int cvValOffset = static_cast<int>(bOffset * 255.0);

    if (cvHueOffset != 0) {

        channels[0].forEach<uchar>([cvHueOffset](uchar& pixel, const int* /*position*/) -> void {
            int newHue = (static_cast<int>(pixel) + cvHueOffset + 180) % 180;
            pixel = static_cast<uchar>(newHue);
        });
    }

    if (cvSatOffset != 0) {
        cv::add(channels[1], cvSatOffset, channels[1]);
    }
    if (cvValOffset != 0) {
        cv::add(channels[2], cvValOffset, channels[2]);
    }

    cv::merge(channels, cvtMat);
    cv::cvtColor(cvtMat, cvtMat, cv::COLOR_HSV2RGB);

    QImage result((const uchar*)cvtMat.data, cvtMat.cols, cvtMat.rows, cvtMat.step, QImage::Format_RGB888);

    if (flipped) {
        result = result.flipped(Qt::Horizontal);
    }

    result = result.copy();

    if (lut && lut->size > 0 && !lut->table.isEmpty()) {
        result = LutService::apply(result, *lut);
    }

    return result;
}

void LutService::applyChangesAsync(const QString path, double hOffset, double bOffset, double sOffset, bool flipped, const QString& lutPath)
{
    // NOWE — "flaga abort": unieważnij poprzednie zadanie, zanim wystartuje nowe.
    // Jeśli poprzednie jeszcze siedziało w kolejce QThreadPool i się nie zaczęło,
    // zostanie w ogóle pominięte. Jeśli już się liczy, i tak nie da się przerwać
    // pojedynczego wywołania OpenCV w środku, ale sprawdzenia isCanceled() poniżej
    // pozwalają zrezygnować z reszty pracy (parsowanie LUT-a, emit sygnału).
    m_watcher.future().cancel();

    auto future = QtConcurrent::run([this, path, hOffset, bOffset, sOffset, flipped, lutPath](QPromise<QImage>& promise) {
        if (promise.isCanceled())
            return;

        QImage sourceImage(path);

        // ZMIANA: wczytanie/parsowanie pliku .cube przeniesione tutaj — to blokujące I/O
        // (dawniej robione synchronicznie w DetailViewModel::applyChanges na wątku UI).
        LutData lut;
        if (!lutPath.isEmpty()) {
            if (auto lutOpt = LutService::loadCached(lutPath)) {
                lut = *lutOpt;
            }
        }

        if (promise.isCanceled())
            return;

        QImage result = processWallpaperOpenCV(sourceImage, hOffset, bOffset, sOffset, flipped, &lut);

        if (promise.isCanceled())
            return;

        promise.addResult(result);
    });

    m_watcher.setFuture(future);
}

// Twoja metoda aplikująca LUT na obiekt QImage (zostawiamy ją, bo jest wywoływana w kroku 5)
QImage LutService::apply(const QImage& src, const LutData& lut)
{
    QImage dst = src.convertToFormat(QImage::Format_RGB32);
    const int N = lut.size - 1;

    for (int y = 0; y < dst.height(); ++y) {
        QRgb* row = reinterpret_cast<QRgb*>(dst.scanLine(y));
        for (int x = 0; x < dst.width(); ++x) {
            QRgb p = row[x];
            float rf = qRed(p) / 255.0f * N;
            float gf = qGreen(p) / 255.0f * N;
            float bf = qBlue(p) / 255.0f * N;

            int ri = qBound(0, (int)rf, N);
            int gi = qBound(0, (int)gf, N);
            int bi = qBound(0, (int)bf, N);

            const QVector3D& out = lut.table[bi * lut.size * lut.size + gi * lut.size + ri];
            row[x] = qRgb(out.x() * 255, out.y() * 255, out.z() * 255);
        }
    }
    return dst;
}

void LutService::cancel() { m_watcher.future().cancel(); }

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
