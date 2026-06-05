#include "detailviewmodel.h"
 
DetailViewModel::DetailViewModel(QObject *parent) : QObject(parent){}
 
void DetailViewModel::setImage(const QString &url, const QString &name)
{
    if (m_imageUrl == url && m_imageName == name)
        return;
 
    m_imageUrl  = url;
    m_imageName = name;
    emit imageChanged();  
}
 
void DetailViewModel::clear()
{
    setImage("", "");
}
