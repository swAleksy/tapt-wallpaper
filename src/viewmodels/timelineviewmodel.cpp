#include "viewmodels/timelineviewmodel.h"
#include "models/editstate.h"
#include <QUuid>
#include <qcoreapplication.h>

QString TimelineViewModel::addItem(
	const QString &sourcePath,
	const QString &name,
	qreal hue,
	qreal brightness,
	qreal saturation,
	bool flipped,
	const QString &lutPath)
{
   	EditState state {
		hue,
		brightness,
		saturation,
		flipped,
		lutPath
	};

    QString id = QUuid::createUuid().toString(QUuid::WithoutBraces);
    QueueItem newItem {id, sourcePath, name, state};

    m_model -> addOrUpdate(newItem);
    return id;

}
