#include "EntryActionButtonDelegate.h"
#include <QApplication>
#include <QMouseEvent>

EntryActionButtonDelegate::EntryActionButtonDelegate(const QIcon& icon, QObject *parent) : QStyledItemDelegate(parent), _icon(icon) {}

void EntryActionButtonDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    QStyleOptionButton button;
    button.rect = option.rect.adjusted(2, 2, -2, -2);
    button.icon = _icon;
    button.iconSize = QSize(20, 20);
    button.state = (QStyle::State_Enabled | QStyle::State_Raised);

    if (option.state & QStyle::State_MouseOver)
        button.state |= QStyle::State_MouseOver;

    QApplication::style()->drawControl(QStyle::CE_PushButton, &button, painter);
}

bool EntryActionButtonDelegate::editorEvent(QEvent* event, QAbstractItemModel* model, const QStyleOptionViewItem& option, const QModelIndex& index)
{
    if (event->type() == QEvent::MouseButtonRelease) {
        QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);
        if (option.rect.contains(mouseEvent->pos())) {
            emit clicked(index.data(Qt::UserRole + 1).value<const DatabaseEntry*>());
            return true;
        }
    }
    return QStyledItemDelegate::editorEvent(event, model, option, index);
}
