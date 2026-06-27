#include "detailviewmodel.h"

DetailViewModel::DetailViewModel(QObject *parent)
    : QObject(parent)
    , m_lutFiltersListModel(new LutFiltersListModel(this))
{
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

    if (urlChanged)  emit imageUrlChanged();
    if (nameChanged) emit imageNameChanged();
    if (wasEmpty != m_imageUrl.isEmpty()) emit hasImageChanged();

    emit imageLoaded();
}


void DetailViewModel::clear()
{
    setImage("", "");
}

void DetailViewModel::applyChanges(qreal hue, qreal brightness, qreal saturation, bool flipped, int filterIndex)
{
    // todo
}

void DetailViewModel::revertChanges()
{
    //todo
}

void DetailViewModel::setAsWallpaper()
{
    //todo
}

void DetailViewModel::addToPlaylist()
{
    //todo
}
