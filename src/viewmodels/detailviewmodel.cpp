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
    m_current = { hue, brightness, saturation, flipped, filterIndex };

    emit hueChanged();
    emit brightnessChanged();
    emit saturationChanged();
    emit flippedChanged();
    emit activeFilterIndexChanged();
}

void DetailViewModel::revertChanges()
{

    emit stateReverted();
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
	// todo
}
