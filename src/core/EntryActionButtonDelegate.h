#ifndef ENTRYACTIONBUTTONDELEGATE_H
#define ENTRYACTIONBUTTONDELEGATE_H

#include <QObject>
#include <QStyledItemDelegate>
#include "DatabaseEntry.h"

class EntryActionButtonDelegate : public QStyledItemDelegate
{
    Q_OBJECT

public:
    explicit EntryActionButtonDelegate(const QIcon& icon, QObject* parent = nullptr);
    void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const override;
    bool editorEvent(QEvent* event, QAbstractItemModel* model, const QStyleOptionViewItem& option, const QModelIndex& index) override;

signals:
    void clicked(const DatabaseEntry* entry);

private:
    QIcon _icon;
};

#endif // ENTRYACTIONBUTTONDELEGATE_H
