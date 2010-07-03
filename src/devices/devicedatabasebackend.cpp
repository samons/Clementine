/* This file is part of Clementine.

   Clementine is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   Clementine is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with Clementine.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "devicedatabasebackend.h"
#include "core/database.h"
#include "core/scopedtransaction.h"

#include <QFile>
#include <QSqlQuery>
#include <QVariant>

const int DeviceDatabaseBackend::kDeviceSchemaVersion = 0;

DeviceDatabaseBackend::DeviceDatabaseBackend(QObject *parent)
  : QObject(parent)
{
}

void DeviceDatabaseBackend::Init(boost::shared_ptr<Database> db) {
  db_ = db;
}

DeviceDatabaseBackend::DeviceList DeviceDatabaseBackend::GetAllDevices() {
  QMutexLocker l(db_->Mutex());
  QSqlDatabase db(db_->Connect());

  DeviceList ret;

  QSqlQuery q("SELECT ROWID, unique_id, friendly_name, size, icon"
              " FROM devices", db);
  q.exec();
  if (db_->CheckErrors(q.lastError())) return ret;

  while (q.next()) {
    Device dev;
    dev.id_ = q.value(0).toInt();
    dev.unique_id_ = q.value(1).toString();
    dev.friendly_name_ = q.value(2).toString();
    dev.size_ = q.value(3).toLongLong();
    dev.icon_name_ = q.value(4).toString();
    ret << dev;
  }
  return ret;
}

int DeviceDatabaseBackend::AddDevice(const Device &device) {
  QMutexLocker l(db_->Mutex());
  QSqlDatabase db(db_->Connect());

  ScopedTransaction t(&db);

  // Insert the device into the devices table
  QSqlQuery q("INSERT INTO devices (unique_id, friendly_name, size, icon)"
              " VALUES (:unique_id, :friendly_name, :size, :icon)", db);
  q.bindValue(":unique_id", device.unique_id_);
  q.bindValue(":friendly_name", device.friendly_name_);
  q.bindValue(":size", device.size_);
  q.bindValue(":icon", device.icon_name_);
  q.exec();
  if (db_->CheckErrors(q.lastError())) return -1;
  int id = q.lastInsertId().toInt();

  // Create the songs tables for the device
  QString filename(":device-schema.sql");
  QFile schema_file(filename);
  if (!schema_file.open(QIODevice::ReadOnly))
    qFatal("Couldn't open schema file %s", filename.toUtf8().constData());
  QString schema = QString::fromUtf8(schema_file.readAll());
  schema.replace("%deviceid", QString::number(id));

  db_->ExecCommands(schema, db);

  t.Commit();
  return id;
}

void DeviceDatabaseBackend::RemoveDevice(int id) {
  QMutexLocker l(db_->Mutex());
  QSqlDatabase db(db_->Connect());

  ScopedTransaction t(&db);

  // Remove the device from the devices table
  QSqlQuery q("DELETE FROM devices WHERE ROWID=:id", db);
  q.bindValue(":id", id);
  q.exec();
  if (db_->CheckErrors(q.lastError())) return;

  // Remove the songs tables for the device
  db.exec(QString("DROP TABLE device_%1").arg(id));
  db.exec(QString("DROP TABLE device_%1_fts").arg(id));

  t.Commit();
}
