#include "viewmodels/detailviewmodel.h"
#include <QImageReader>

DetailViewModel::DetailViewModel(QObject* parent)
    : QObject(parent)
    , m_lutFiltersListModel(new LutFiltersListModel(this))
{
    m_lutFiltersListModel->loadFromDirectory(":/luts");
}

void DetailViewModel::setImage(const QString& url, const QString& name)
{
    bool urlChanged = (m_imageUrl != url);
    bool nameChanged = (m_imageName != name);
    if (!urlChanged && !nameChanged)
        return;

    // setImage() to zawsze "nowy" obraz (np. wybrany z pickera), nie
    // edycja istniejącego elementu kolejki — inaczej niż loadForEditing().
    m_editingPlaylistItemId.clear();

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


void DetailViewModel::applyChanges(qreal hue, qreal brightness, qreal saturation, bool flipped, int filterIndex)
{
    // Jasność ograniczona do -50%…+50%, nasycenie do -90%…+90% — niezależnie
    // od tego, skąd trafiła tu wartość (suwak w DetailView.qml jest już
    // ograniczony do tego zakresu, ale clamp tutaj jest ostatnią linią
    // obrony, np. gdy dane pochodzą ze starszego elementu kolejki).
    brightness = clampBrightness(brightness);
    saturation = clampSaturation(saturation);

    m_current = { hue, brightness, saturation, flipped, filterIndex };

    emit hueChanged();
    emit brightnessChanged();
    emit saturationChanged();
    emit flippedChanged();
    emit activeFilterIndexChanged();

    // Jeśli edytujemy element już obecny w kolejce, odeślij zmiany do
    // TimelineViewModel::updateItem() (podłącz w warstwie wyżej), żeby
    // QueueModel wyemitował dataChanged i PreviewImage w TimelinePanel
    // dostał nowe hue/brightness/saturation/lutPath.
    if (!m_editingPlaylistItemId.isEmpty()) {
        QString lutPath;
        if (filterIndex >= 0 && m_lutFiltersListModel)
            lutPath = m_lutFiltersListModel->lutPath(filterIndex);

        emit itemEditApplied(m_editingPlaylistItemId, hue, brightness, saturation, flipped, lutPath);
    }
}

void DetailViewModel::revertChanges()
{
    // "Przywróć" czyści korekty i filtr do stanu domyślnego (zero), a NIE
    // cofa do ostatnio zatwierdzonych wartości (applyChanges()).
    m_current = ColorState {};

    emit hueChanged();
    emit brightnessChanged();
    emit saturationChanged();
    emit flippedChanged();
    emit activeFilterIndexChanged();

    emit stateReverted();

    // Tak samo jak w applyChanges(): jeśli edytujemy element już obecny
    // w kolejce, odeślij wyzerowany stan dalej, żeby podgląd na osi czasu
    // też wyczyścił się na żywo, a nie tylko lokalny podgląd w DetailView.
    if (!m_editingPlaylistItemId.isEmpty()) {
        emit itemEditApplied(m_editingPlaylistItemId,
                              m_current.hue,
                              m_current.brightness,
                              m_current.saturation,
                              m_current.flipped,
                              QString());
    }
}

void DetailViewModel::setAsWallpaper()
{
    // todo
}

void DetailViewModel::addToPlaylist()
{
    QString lutPath = "";
    if (m_current.activeFilterIndex >= 0 && m_lutFiltersListModel) {
        lutPath = m_lutFiltersListModel->lutPath(m_current.activeFilterIndex);
    }

    emit imageAdded(
        m_originalImagePath,
        m_imageName,
        m_current.hue,
        m_current.brightness,
        m_current.saturation,
        m_current.flipped,
        lutPath
    );
}

void DetailViewModel::loadForEditing(
	const QString &playlistItemId,
	const QString &sourcePath,
	const QString &name,
	qreal hue,
	qreal brightness,
	qreal saturation,
	bool flipped,
	const QString &lutPath)
{
    m_editingPlaylistItemId = playlistItemId;

    const bool urlChanged = (m_imageUrl != sourcePath);
    const bool nameChanged = (m_imageName != name);
    const bool hadImage = hasImage();

    // QueueItem::sourcePath to zawsze surowa ścieżka systemowa (patrz
    // queueitem.h), więc — w odróżnieniu od setImage() — nie trzeba tu
    // zdejmować prefiksu "image://taptimage/". PreviewImage.resolvedSource
    // sam dokleja ten prefiks, gdy source nie zawiera "://".
    m_imageUrl = sourcePath;
    m_imageName = name;
    m_originalImagePath = sourcePath;
    m_originalImageValid = QImageReader(sourcePath).canRead();
    if (!m_originalImageValid)
        qWarning() << "DetailViewModel::loadForEditing: nie da się wczytać" << sourcePath;

    // lutPath -> indeks w liście filtrów (odwrotność tego, co addToPlaylist()
    // robi w drugą stronę). Brak dopasowania = -1, tak samo jak "brak filtra".
    int filterIndex = -1;
    if (!lutPath.isEmpty() && m_lutFiltersListModel) {
        const int count = m_lutFiltersListModel->rowCount();
        for (int i = 0; i < count; ++i) {
            if (m_lutFiltersListModel->lutPath(i) == lutPath) {
                filterIndex = i;
                break;
            }
        }
    }

    m_current = ColorState { hue, clampBrightness(brightness), clampSaturation(saturation), flipped, filterIndex };

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

void DetailViewModel::setEditingItemId(const QString &id)
{
    m_editingPlaylistItemId = id;
}
