#include "detailviewmodel.h"
#include <QImageReader>

DetailViewModel::DetailViewModel(QObject* parent)
    : QObject(parent)
    , m_lutFiltersListModel(new LutFiltersListModel(this))
    , m_lutService(new LutService(this))
{
    connect(m_lutService, &LutService::lutApplied, this, [this](const QImage& result) {
        m_busy = false;
        emit busyChanged();
    });
    connect(m_lutService, &LutService::lutFailed, this, [this](const QString& err) {
        m_busy = false;
        emit busyChanged();
        qWarning() << "LUT failed:" << err;
    });
    m_lutFiltersListModel->loadFromDirectory(":/luts");
}

void DetailViewModel::setImage(const QString& url, const QString& name)
{
    bool urlChanged = (m_imageUrl != url);
    bool nameChanged = (m_imageName != name);
    if (!urlChanged && !nameChanged)
        return;

    m_lutService->cancel();

    const bool hadImage = hasImage();
    m_imageUrl = url;
    m_imageName = name;

    QString localPath = url;
    const QString prefix = QStringLiteral("image://taptimage/");

    if (localPath.startsWith(prefix)) {
        localPath.remove(0, prefix.size());
        localPath = QUrl::fromPercentEncoding(localPath.toUtf8());
    } else if (localPath.startsWith("file://"))
        localPath = QUrl(localPath).toLocalFile();

    m_originalImagePath = localPath;
    m_originalImageValid = QImageReader(localPath).canRead();
    if (!m_originalImageValid)
        qWarning() << "DetailViewModel: nie da się wczytać" << localPath;

    m_current = ColorState {};
    // m_committed    = ColorState{};
    // m_lutProcessed = QImage();

    if (urlChanged)
        emit imageUrlChanged();
    if (nameChanged)
        emit imageNameChanged();
    if (hadImage != hasImage())
        emit hasImageChanged();

    emit hueChanged();
    emit brightnessChanged();
    emit saturationChanged();
    emit flippedChanged();
    emit activeFilterIndexChanged();

    emit imageLoaded();
}

// void DetailViewModel::clear()
// {
//     setImage("", "");

//     m_current = ColorState{};
//     m_committed = ColorState{};

//     emit hueChanged();
//     emit brightnessChanged();
//     emit saturationChanged();
//     emit flippedChanged();
//     emit activeFilterIndexChanged();
// }

void DetailViewModel::applyChanges(qreal hue, qreal brightness, qreal saturation, bool flipped, int filterIndex)
{
    // m_committed = m_current;
    m_current = { hue, brightness, saturation, flipped, filterIndex };

    emit hueChanged();
    emit brightnessChanged();
    emit saturationChanged();
    emit flippedChanged();
    emit activeFilterIndexChanged();

    // reprocessAsync(); podgląd tylko przez qml/gpu
}

void DetailViewModel::revertChanges()
{

    // reprocessAsync();
    emit stateReverted();
}

void DetailViewModel::reprocessAsync()
{
    const QString path = m_originalImagePath;
    // ZMIANA: przekazujemy tylko ścieżkę do .cube — samo wczytanie/parsowanie pliku
    // dzieje się teraz na wątku roboczym wewnątrz LutService::applyChangesAsync,
    // zamiast blokować wątek UI tak jak poprzednio.
    const QString lutPath = m_current.activeFilterIndex >= 0 ? m_lutFiltersListModel->lutPath(m_current.activeFilterIndex) : QString();

    m_busy = true;
    emit busyChanged();

    m_lutService->applyChangesAsync(path, m_current.hue, m_current.brightness, m_current.saturation, m_current.flipped, lutPath);

    emit hueChanged();
    emit brightnessChanged();
    emit saturationChanged();
    emit flippedChanged();
    emit activeFilterIndexChanged();
}

void DetailViewModel::setAsWallpaper()
{
    // todo
}

void DetailViewModel::addToPlaylist()
{
    // todo
}
