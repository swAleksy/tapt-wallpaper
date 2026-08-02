#ifndef QUEUEMODEL_H
#define QUEUEMODEL_H

#include "models/queueitem.h"
#include <QAbstractListModel>
#include <QObject>

class QueueModel : public QAbstractListModel {
    Q_OBJECT
public:
    enum Roles {
        IdRole = Qt::UserRole + 1,
        SourcePathRole,
        NameRole,
        HueRole,
        BrightnessRole,
        SaturationRole,
        FlippedRole,
        LutPathRole
    };
    // rowCount/data/roleNames analogicznie do ImagesModel

    explicit QueueModel(QObject* parent = nullptr);

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    void addOrUpdate(const QueueItem& item); // po id
    void remove(const QString& id);
    void resetToDefaults(const QString& id); // edit = EditState::identity()

private:
    QList<QueueItem> m_items;

};

#endif // QUEUEMODEL_H
