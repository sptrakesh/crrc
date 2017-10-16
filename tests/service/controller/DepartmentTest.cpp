#include "DepartmentTest.h"

#include <QtCore/QJsonArray>
#include <QtCore/QJsonObject>
#include <QtCore/QJsonDocument>

namespace crrc
{
  namespace departmenttest
  {
    static int32_t institutionId = 0;
    static int32_t id = 0;
    static const QString institutionName{ "Unit Test Institution" };
    static const QString institutionCity{ "Unit Test City" };
    static const QString modified = "ddd";
  }
}

using crrc::DepartmentTest;

void DepartmentTest::initTestCase()
{
  QEventLoop eventLoop;
  QNetworkAccessManager mgr;
  connect( &mgr, SIGNAL( finished( QNetworkReply* ) ), &eventLoop,
      SLOT( quit() ) );
  QNetworkRequest req;
  login( &mgr, &eventLoop, &req );

  const auto url = QString{ "http://localhost:3000/institutions/edit" };
  const auto data = QString( "name=%1&city=%2" ).
      arg( departmenttest::institutionName ).arg( departmenttest::institutionCity );
  auto reply = put( url, data, &mgr, &eventLoop, &req );

  QVERIFY2( reply->error() == QNetworkReply::NoError, "Error creating new institution" );
  const auto doc = QJsonDocument::fromJson( reply->readAll() );
  const auto obj = doc.object();
  qDebug() << obj;
  QVERIFY2( !obj.isEmpty(), "Empty JSON response for create institution" );
  QVERIFY2( obj["id"].toInt(), "Create institution returned invalid id" );
  departmenttest::institutionId = obj["id"].toInt();

  logout( &mgr, &eventLoop, &req );
}

void DepartmentTest::create()
{
  QEventLoop eventLoop;
  QNetworkAccessManager mgr;
  connect( &mgr, SIGNAL( finished( QNetworkReply* ) ), &eventLoop,
      SLOT( quit() ) );
  QNetworkRequest req;
  login( &mgr, &eventLoop, &req );

  const auto url = QString{ "http://localhost:3000/institution/departments/save" };
  const auto data = QString( "name=%1&prefix=%2&institutionId=%3" ).
      arg( name ).arg( prefix ).arg( departmenttest::institutionId );
  auto reply = post( url, data, &mgr, &eventLoop, &req );

  QVERIFY2( reply->error() == QNetworkReply::NoError, "Error creating new department" );
  const auto doc = QJsonDocument::fromJson( reply->readAll() );
  const auto obj = doc.object();
  QVERIFY2( !obj.isEmpty(), "Empty JSON response for create department" );
  QVERIFY2( obj["id"].toInt(), "Create department returned invalid id" );
  departmenttest::id = obj["id"].toInt();

  logout( &mgr, &eventLoop, &req );
}

void DepartmentTest::retrieve()
{
  QEventLoop eventLoop;
  QNetworkAccessManager mgr;
  connect( &mgr, SIGNAL( finished( QNetworkReply* ) ), &eventLoop,
      SLOT( quit() ) );
  QNetworkRequest req;
  login( &mgr, &eventLoop, &req );

  const auto url = QString( "http://localhost:3000/departments/id/%1/data" ).arg( departmenttest::id );
  const auto reply = get( url, &mgr, &eventLoop, &req );

  QVERIFY2( reply->error() == QNetworkReply::NoError, "Error retrieving department" );
  const auto doc = QJsonDocument::fromJson( reply->readAll() );
  const auto obj = doc.object();
  QVERIFY2( !obj.isEmpty(), "Empty JSON response for department id" );
  QVERIFY2( departmenttest::id == obj["id"].toInt(), "Json response returned invalid id" );
  QVERIFY2( name == obj["name"].toString(), "Json response returned invalid name" );
  QVERIFY2( prefix == obj["prefix"].toString(), "Json response returned invalid prefix" );

  logout( &mgr, &eventLoop, &req );
}

void DepartmentTest::invalid()
{
  QEventLoop eventLoop;
  QNetworkAccessManager mgr;
  connect( &mgr, SIGNAL( finished( QNetworkReply* ) ), &eventLoop,
      SLOT( quit() ) );
  QNetworkRequest req;
  login( &mgr, &eventLoop, &req );

  const auto url = QString{ "http://localhost:3000/departments/id/0/data" };
  const auto reply = get( url, &mgr, &eventLoop, &req );

  QVERIFY2( reply->error() == QNetworkReply::NoError, "Error retrieving invalid department" );
  const auto doc = QJsonDocument::fromJson( reply->readAll() );
  const auto obj = doc.object();
  QVERIFY2( obj.isEmpty(), "Non-Empty JSON response for invalid department id" );

  logout( &mgr, &eventLoop, &req );
}

void DepartmentTest::index()
{
  QEventLoop eventLoop;
  QNetworkAccessManager mgr;
  connect( &mgr, SIGNAL( finished( QNetworkReply* ) ), &eventLoop,
      SLOT( quit() ) );
  QNetworkRequest req;
  login( &mgr, &eventLoop, &req );

  const auto url = QString{ "http://localhost:3000/departments" };
  const auto reply = get( url, &mgr, &eventLoop, &req );

  QVERIFY2( reply->error() == QNetworkReply::NoError, "Error retrieving department list" );
  const auto doc = QJsonDocument::fromJson( reply->readAll() );
  const auto arr = doc.array();
  QVERIFY2( !arr.isEmpty(), "Empty JSON response for department list" );

  bool found = false;
  for ( const auto& obj : arr )
  {
    if ( departmenttest::id == obj.toObject()["id"].toInt() )
    {
      found = true;
      break;
    }
  }

  QVERIFY2( found, "Newly created department not returned in list" );

  logout( &mgr, &eventLoop, &req );
}

void DepartmentTest::update()
{
  QEventLoop eventLoop;
  QNetworkAccessManager mgr;
  connect( &mgr, SIGNAL( finished( QNetworkReply* ) ), &eventLoop,
      SLOT( quit() ) );
  QNetworkRequest req;
  login( &mgr, &eventLoop, &req );

  const auto url = QString{ "http://localhost:3000/institution/departments/save" };
  const auto data = QString( "id=%1&name=%2&prefix=%3&institutionId=%4" )
      .arg( departmenttest::id ).arg( name ).arg( departmenttest::modified ).arg( departmenttest::institutionId );
  const auto reply = post( url, data, &mgr, &eventLoop, &req );

  QVERIFY2( reply->error() == QNetworkReply::NoError, "Error updating department" );
  const auto doc = QJsonDocument::fromJson( reply->readAll() );
  const auto obj = doc.object();
  QVERIFY2( !obj.isEmpty(), "Empty JSON response for create department" );
  QVERIFY2( departmenttest::id == obj["id"].toInt(), "Json response returned invalid id" );

  logout( &mgr, &eventLoop, &req );
}

void DepartmentTest::retrieveModified()
{
  QEventLoop eventLoop;
  QNetworkAccessManager mgr;
  connect( &mgr, SIGNAL( finished( QNetworkReply* ) ), &eventLoop,
      SLOT( quit() ) );
  QNetworkRequest req;
  login( &mgr, &eventLoop, &req );

  const auto url = QString( "http://localhost:3000/departments/id/%1/data" ).arg( departmenttest::id );
  const auto reply = get( url, &mgr, &eventLoop, &req );

  QVERIFY2( reply->error() == QNetworkReply::NoError, "Error retrieving department" );
  const auto doc = QJsonDocument::fromJson( reply->readAll() );
  const auto obj = doc.object();
  QVERIFY2( !obj.isEmpty(), "Empty JSON response for department id" );
  QVERIFY2( departmenttest::id == obj["id"].toInt(), "Json response returned invalid id" );
  QVERIFY2( name == obj["name"].toString(), "Json response returned invalid name" );
  QVERIFY2( departmenttest::modified == obj["prefix"].toString(), "Json response returned invalid prefix" );

  logout( &mgr, &eventLoop, &req );
}

void DepartmentTest::remove()
{
  QEventLoop eventLoop;
  QNetworkAccessManager mgr;
  connect( &mgr, SIGNAL( finished( QNetworkReply* ) ), &eventLoop,
      SLOT( quit() ) );
  QNetworkRequest req;
  login( &mgr, &eventLoop, &req );

  const auto url = QString{ "http://localhost:3000/institution/departments/remove" };
  const auto data = QString( "id=%1&institutionId=%2" ).
      arg( departmenttest::id ).arg( departmenttest::institutionId );
  const auto reply = post( url, data, &mgr, &eventLoop, &req );

  QVERIFY2( reply->error() == QNetworkReply::NoError, "Error deleting department" );
  const auto doc = QJsonDocument::fromJson( reply->readAll() );
  const auto obj = doc.object();
  qDebug() << obj;
  QVERIFY2( !obj.isEmpty(), "Empty JSON response for delete department" );
  QVERIFY2( obj["status"].toBool(), "Remove department returned false status" );
  QVERIFY2( obj["count"].toInt(), "Remove department returned invalid count" );

  logout( &mgr, &eventLoop, &req );
}

void DepartmentTest::readDeleted()
{
  QEventLoop eventLoop;
  QNetworkAccessManager mgr;
  connect( &mgr, SIGNAL( finished( QNetworkReply* ) ), &eventLoop,
      SLOT( quit() ) );
  QNetworkRequest req;
  login( &mgr, &eventLoop, &req );

  const auto url = QString{ "http://localhost:3000/departments/id/%1/data" }.arg( departmenttest::id );
  const auto reply = get( url, &mgr, &eventLoop, &req );

  QVERIFY2( reply->error() == QNetworkReply::NoError, "Error retrieving deleted department" );
  const auto doc = QJsonDocument::fromJson( reply->readAll() );
  const auto obj = doc.object();
  QVERIFY2( obj.isEmpty(), "Non-Empty JSON response for deleted department id" );

  logout( &mgr, &eventLoop, &req );
}

void DepartmentTest::cleanupTestCase()
{
  QEventLoop eventLoop;
  QNetworkAccessManager mgr;
  connect( &mgr, SIGNAL( finished( QNetworkReply* ) ), &eventLoop,
      SLOT( quit() ) );
  QNetworkRequest req;
  login( &mgr, &eventLoop, &req );

  const auto url = QString{ "http://localhost:3000/institutions/remove" };
  const auto data = QString( "id=%1" ).arg( departmenttest::institutionId );
  const auto reply = put( url, data, &mgr, &eventLoop, &req );

  QVERIFY2( reply->error() == QNetworkReply::NoError, "Error deleting institution" );
  const auto doc = QJsonDocument::fromJson( reply->readAll() );
  const auto obj = doc.object();
  QVERIFY2( !obj.isEmpty(), "Empty JSON response for delete institution" );
  QVERIFY2( obj["status"].toBool(), "Remove institution returned false status" );
  QVERIFY2( obj["count"].toInt(), "Remove institution returned invalid count" );

  logout( &mgr, &eventLoop, &req );
}

