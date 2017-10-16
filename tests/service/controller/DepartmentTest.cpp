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
    static QString institutionName{ "Unit Test Institution" };
    static QString institutionCity{ "Unit Test City" };
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
  qDebug() << obj;
  QVERIFY2( !obj.isEmpty(), "Empty JSON response for delete institution" );
  QVERIFY2( obj["status"].toBool(), "Remove institution returned false status" );
  QVERIFY2( obj["count"].toInt(), "Remove institution returned invalid count" );

  logout( &mgr, &eventLoop, &req );
}
