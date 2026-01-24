#ifndef HISTORYACTIONBUTTONDELEGATE_H
#define HISTORYACTIONBUTTONDELEGATE_H

#include <QStyledItemDelegate>
#include "DatabaseEntryHistoryItem.h"

class HistoryActionButtonDelegate : public QStyledItemDelegate
{
    Q_OBJECT

public:
    explicit HistoryActionButtonDelegate(const QIcon& icon, QObject* parent = nullptr);
    void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const override;
    bool editorEvent(QEvent* event, QAbstractItemModel* model, const QStyleOptionViewItem& option, const QModelIndex& index) override;

signals:
    void clicked(const QModelIndex& index);

private:
    QIcon _icon;
};

#endif // HISTORYACTIONBUTTONDELEGATE_H
