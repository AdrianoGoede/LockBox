#include "DatabaseGroup.h"

DatabaseGroup::DatabaseGroup() : _uid(QUuid::createUuid()) {}

DatabaseGroup::DatabaseGroup(const DatabaseGroupDto& groupDto) : _uid(QUuid::createUuid()), _title(groupDto.title), _parent(groupDto.parent) {}

DatabaseGroup::DatabaseGroup(QDataStream& in) { in >> _uid >> _parent >> _title; }

QUuid DatabaseGroup::uid() const { return _uid; }

QUuid DatabaseGroup::parent() const { return _parent; }

void DatabaseGroup::setParent(const QUuid& parent) { _parent = parent; }

QString DatabaseGroup::title() const { return _title; }

void DatabaseGroup::setTitle(const QString& title) { _title = title; }

void DatabaseGroup::toBinary(QDataStream& out) const { out << _uid << _parent << _title; }
