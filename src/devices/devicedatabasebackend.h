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

#ifndef DEVICEDATABASEBACKEND_H
#define DEVICEDATABASEBACKEND_H

#include <QObject>

#include <boost/shared_ptr.hpp>

class Database;

class DeviceDatabaseBackend : public QObject {
  Q_OBJECT

public:
  Q_INVOKABLE DeviceDatabaseBackend(QObject* parent = 0);

  struct Device {
    Device() : id_(-1) {}

    int id_;
    QString unique_id_;
    QString friendly_name_;
    quint64 size_;
    QString icon_name_;
  };
  typedef QList<Device> DeviceList;

  static const int kDeviceSchemaVersion;

  void Init(boost::shared_ptr<Database> db);
  boost::shared_ptr<Database> db() const { return db_; }

  DeviceList GetAllDevices();
  int AddDevice(const Device& device);
  void RemoveDevice(int id);

private:
  boost::shared_ptr<Database> db_;

};

#endif // DEVICEDATABASEBACKEND_H
