#include "HistoryActionButtonDelegate.h"
#include <QApplication>
#include <QMouseEvent>

HistoryActionButtonDelegate::HistoryActionButtonDelegate(const QIcon& icon, QObject* parent) : QStyledItemDelegate(parent), _icon(icon) {}

void HistoryActionButtonDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
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

bool HistoryActionButtonDelegate::editorEvent(QEvent* event, QAbstractItemModel* model, const QStyleOptionViewItem& option, const QModelIndex& index)
{
    if (event->type() == QEvent::MouseButtonRelease) {
        QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);
        if (option.rect.contains(mouseEvent->pos())) {
            emit clicked(index);
            return true;
        }
    }
    return QStyledItemDelegate::editorEvent(event, model, option, index);
}
