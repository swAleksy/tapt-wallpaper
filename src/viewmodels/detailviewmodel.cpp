#include "detailviewmodel.h"

DetailViewModel::DetailViewModel(QObject *parent)
    : QObject(parent)
    , m_lutFiltersListModel(new LutFiltersListModel(this))
    , m_lutService(new LutService(this))
{
    connect(m_lutService, &LutService::lutApplied, this, [this](const QImage &result) {
        // zapisz m_lutProcessed, zaktualizuj imageUrl przez tymczasowy provider
        m_lutProcessed = result;
        m_busy = false;
        emit busyChanged();
        emit imageUrlChanged();
    });
    connect(m_lutService, &LutService::lutFailed, this, [this](const QString &err) {
        m_busy = false;
        emit busyChanged();
        qWarning() << "LUT failed:" << err;
    });
    m_lutFiltersListModel->loadFromDirectory(":/luts");
}

void DetailViewModel::setImage(const QString &url, const QString &name)
{
    bool urlChanged  = (m_imageUrl  != url);
    bool nameChanged = (m_imageName != name);
    if (!urlChanged && !nameChanged)
        return;

    bool wasEmpty = m_imageUrl.isEmpty();
    m_imageUrl    = url;
    m_imageName   = name;

    m_originalImage = QImage(url);

    if (urlChanged)  emit imageUrlChanged();
    if (nameChanged) emit imageNameChanged();
    if (wasEmpty != m_imageUrl.isEmpty()) emit hasImageChanged();

    emit imageLoaded();
}


void DetailViewModel::clear()
{
    setImage("", "");

    m_current = ColorState{};
    m_committed = ColorState{};

    emit hueChanged();
    emit brightnessChanged();
    emit saturationChanged();
    emit flippedChanged();
    emit activeFilterIndexChanged();
}

void DetailViewModel::applyChanges(qreal hue, qreal brightness, qreal saturation, bool flipped, int filterIndex)
{
    m_committed = m_current;   // zapisz poprzedni stan do revert

    m_current.hue = hue;
    m_current.brightness = brightness;
    m_current.saturation = saturation;
    m_current.flipped = flipped;
    m_current.activeFilterIndex = filterIndex;

    reprocessAsync();
}

void DetailViewModel::revertChanges()
{
    m_current = m_committed;

    reprocessAsync();

    emit stateReverted();
}

void DetailViewModel::reprocessAsync()
{
    // ZMIANA: przekazujemy tylko ścieżkę do .cube — samo wczytanie/parsowanie pliku
    // dzieje się teraz na wątku roboczym wewnątrz LutService::applyChangesAsync,
    // zamiast blokować wątek UI tak jak poprzednio.
    const QString lutPath = m_current.activeFilterIndex >= 0
        ? m_lutFiltersListModel->lutPath(m_current.activeFilterIndex)
        : QString();

    m_busy = true;
    emit busyChanged();

    m_lutService->applyChangesAsync(m_originalImage, m_current.hue, m_current.brightness, m_current.saturation, lutPath);

    emit hueChanged();
    emit brightnessChanged();
    emit saturationChanged();
    emit flippedChanged();
    emit activeFilterIndexChanged();
}

void DetailViewModel::setAsWallpaper()
{
    //todo
}

void DetailViewModel::addToPlaylist()
{
    //todo
}
