#include "DatabaseGroup.h"
#include <QJsonArray>

DatabaseGroup::DatabaseGroup() : _uid(QUuid::createUuid()) {}

DatabaseGroup::DatabaseGroup(const QJsonObject& jsonObj)
{
    _uid = QUuid::fromString(jsonObj["uuid"].toString());
    _parent = QUuid::fromString(jsonObj["parent"].toString());
    _title = jsonObj["title"].toString();

    if (_uid.isNull() || _title.isEmpty())
        throw std::runtime_error("Invalid or corrupted data");
}

QUuid DatabaseGroup::uid() const { return _uid; }

QUuid DatabaseGroup::parent() const { return _parent; }

void DatabaseGroup::setParent(const QUuid& parent) { _parent = parent; }

void DatabaseGroup::setParent(const DatabaseGroup& group) { _parent = group.uid(); }

QString DatabaseGroup::title() const { return _title; }

void DatabaseGroup::setTitle(const QString& title) { _title = title; }

QJsonObject DatabaseGroup::toJson() const
{
    QJsonObject obj;
    obj["uuid"] = _uid.toString(QUuid::StringFormat::WithoutBraces);
    obj["parent"] = _parent.toString(QUuid::StringFormat::WithoutBraces);
    obj["title"] = _title;
    return obj;
}
