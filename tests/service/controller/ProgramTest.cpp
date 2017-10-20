#include "ProgramTest.h"

#include <QtCore/QJsonArray>
#include <QtCore/QJsonDocument>
#include <QtCore/QJsonObject>

namespace crrc
{
  namespace programtest
  {
    static uint32_t id = 0;
    static QString institutionName{ "Unit Test Institution" };
    static QString institutionCity{ "Unit Test City" };
    static uint32_t institutionId = 0;
  }
}

using crrc::ProgramTest;

void ProgramTest::initTestCase()
{
  QEventLoop eventLoop;
  QNetworkAccessManager mgr;
  connect( &mgr, SIGNAL( finished( QNetworkReply* ) ), &eventLoop,
      SLOT( quit() ) );
  QNetworkRequest req;
  login( &mgr, &eventLoop, &req );

  const auto url = QString{ "http://localhost:3000/institutions/edit" };
  const auto data = QString( "name=%1&city=%2" ).
      arg( programtest::institutionName ).arg( programtest::institutionCity );
  auto reply = put( url, data, &mgr, &eventLoop, &req );

  QVERIFY2( reply->error() == QNetworkReply::NoError, "Error creating new institution" );
  const auto doc = QJsonDocument::fromJson( reply->readAll() );
  const auto obj = doc.object();
  QVERIFY2( !obj.isEmpty(), "Empty JSON response for create institution" );
  QVERIFY2( obj["id"].toInt(), "Create institution returned invalid id" );
  programtest::institutionId = obj["id"].toInt();

  logout( &mgr, &eventLoop, &req );
}

void ProgramTest::create()
{
  QEventLoop eventLoop;
  QNetworkAccessManager mgr;
  connect( &mgr, SIGNAL( finished( QNetworkReply* ) ), &eventLoop,
      SLOT( quit() ) );
  QNetworkRequest req;
  login( &mgr, &eventLoop, &req );

  const auto url = QString{ "http://localhost:3000/programs/edit" };
  const auto data = QString{ "title=%1&institution=%2" }.
      arg( title ).arg( programtest::institutionId );
  auto reply = put( url, data, &mgr, &eventLoop, &req );

  QVERIFY2( reply->error() == QNetworkReply::NoError, "Error creating new program" );
  const auto doc = QJsonDocument::fromJson( reply->readAll() );
  const auto obj = doc.object();
  QVERIFY2( !obj.isEmpty(), "Empty JSON response for create program" );
  QVERIFY2( obj["id"].toInt(), "Create program returned invalid id" );
  programtest::id = obj["id"].toInt();

  logout( &mgr, &eventLoop, &req );
}

void ProgramTest::retrieve()
{
  QEventLoop eventLoop;
  QNetworkAccessManager mgr;
  connect( &mgr, SIGNAL( finished( QNetworkReply* ) ), &eventLoop,
      SLOT( quit() ) );
  QNetworkRequest req;
  login( &mgr, &eventLoop, &req );

  const auto url = QString( "http://localhost:3000/programs/id/%1/data" ).arg( programtest::id );
  const auto reply = get( url, &mgr, &eventLoop, &req );

  QVERIFY2( reply->error() == QNetworkReply::NoError, "Error retrieving program" );
  const auto doc = QJsonDocument::fromJson( reply->readAll() );
  const auto obj = doc.object();
  qDebug() << obj;
  QVERIFY2( !obj.isEmpty(), "Empty JSON response for program id" );
  QCOMPARE( static_cast<int>( programtest::id ), obj["id"].toInt() );
  QCOMPARE( static_cast<int>( programtest::institutionId ), obj["institution"].toObject()["id"].toInt() );
  QCOMPARE( title, obj["title"].toString() );
  QCOMPARE( QString{ "Unspecified" }, obj["credits"].toString() );

  logout( &mgr, &eventLoop, &req );
}

void ProgramTest::invalid()
{
  QEventLoop eventLoop;
  QNetworkAccessManager mgr;
  connect( &mgr, SIGNAL( finished( QNetworkReply* ) ), &eventLoop,
      SLOT( quit() ) );
  QNetworkRequest req;
  login( &mgr, &eventLoop, &req );

  const auto url = QString{ "http://localhost:3000/programs/id/0/data" };
  const auto reply = get( url, &mgr, &eventLoop, &req );

  QVERIFY2( reply->error() == QNetworkReply::NoError, "Error retrieving invalid program" );
  const auto doc = QJsonDocument::fromJson( reply->readAll() );
  const auto obj = doc.object();
  QVERIFY2( obj.isEmpty(), "Non-Empty JSON response for invalid program id" );

  logout( &mgr, &eventLoop, &req );
}

void ProgramTest::index()
{
  QEventLoop eventLoop;
  QNetworkAccessManager mgr;
  connect( &mgr, SIGNAL( finished( QNetworkReply* ) ), &eventLoop,
      SLOT( quit() ) );
  QNetworkRequest req;
  login( &mgr, &eventLoop, &req );

  const auto url = QString{ "http://localhost:3000/programs" };
  const auto data = QString{};
  const auto reply = post( url, data, &mgr, &eventLoop, &req );

  QVERIFY2( reply->error() == QNetworkReply::NoError, "Error retrieving program list" );
  const auto doc = QJsonDocument::fromJson( reply->readAll() );
  const auto arr = doc.array();
  QVERIFY2( !arr.isEmpty(), "Empty JSON response for program list" );

  bool found = false;
  for ( const auto& obj : arr )
  {
    if ( programtest::id == obj.toObject()["id"].toInt() )
    {
      found = true;
      break;
    }
  }

  QVERIFY2( found, "Newly created program not returned in list" );

  logout( &mgr, &eventLoop, &req );
}

void ProgramTest::update()
{
  QEventLoop eventLoop;
  QNetworkAccessManager mgr;
  connect( &mgr, SIGNAL( finished( QNetworkReply* ) ), &eventLoop,
      SLOT( quit() ) );
  QNetworkRequest req;
  login( &mgr, &eventLoop, &req );

  const auto url = QString{ "http://localhost:3000/programs/edit" };
  const auto data = QString{ "id=%1&title=%2&credits=%3&institution=%4" }.
      arg( programtest::id ).arg( title ).arg( credits ).arg( programtest::institutionId );
  const auto reply = put( url, data, &mgr, &eventLoop, &req );

  QVERIFY2( reply->error() == QNetworkReply::NoError, "Error updating program" );
  const auto doc = QJsonDocument::fromJson( reply->readAll() );
  const auto obj = doc.object();
  QVERIFY2( !obj.isEmpty(), "Empty JSON response for create program" );
  QVERIFY2( programtest::id == obj["id"].toInt(), "Json response returned invalid id" );
  QVERIFY2( obj["status"].toBool(), "Json response returned invalid status" );

  logout( &mgr, &eventLoop, &req );
}

void ProgramTest::retrieveModified()
{
  QEventLoop eventLoop;
  QNetworkAccessManager mgr;
  connect( &mgr, SIGNAL( finished( QNetworkReply* ) ), &eventLoop,
      SLOT( quit() ) );
  QNetworkRequest req;
  login( &mgr, &eventLoop, &req );

  const auto url = QString( "http://localhost:3000/programs/id/%1/data" ).arg( programtest::id );
  const auto reply = get( url, &mgr, &eventLoop, &req );

  QVERIFY2( reply->error() == QNetworkReply::NoError, "Error retrieving program" );
  const auto doc = QJsonDocument::fromJson( reply->readAll() );
  const auto obj = doc.object();
  QVERIFY2( !obj.isEmpty(), "Empty JSON response for program id" );
  QVERIFY2( programtest::id == obj["id"].toInt(), "Json response returned invalid id" );
  QVERIFY2( programtest::institutionId == obj["institution"].toObject()["id"].toInt(), "Json response returned invalid institution" );
  QVERIFY2( title == obj["title"].toString(), "Json response returned invalid title" );
  QVERIFY2( credits == obj["credits"].toString(), "Json response returned invalid credits" );

  logout( &mgr, &eventLoop, &req );
}

void ProgramTest::remove()
{
  QEventLoop eventLoop;
  QNetworkAccessManager mgr;
  connect( &mgr, SIGNAL( finished( QNetworkReply* ) ), &eventLoop,
      SLOT( quit() ) );
  QNetworkRequest req;
  login( &mgr, &eventLoop, &req );

  const auto url = QString{ "http://localhost:3000/programs/remove" };
  const auto data = QString( "id=%1" ).arg( programtest::id );
  const auto reply = put( url, data, &mgr, &eventLoop, &req );

  QVERIFY2( reply->error() == QNetworkReply::NoError, "Error deleting program" );
  const auto doc = QJsonDocument::fromJson( reply->readAll() );
  const auto obj = doc.object();
  qDebug() << obj;
  QVERIFY2( !obj.isEmpty(), "Empty JSON response for delete program" );
  QVERIFY2( obj["status"].toBool(), "Remove program returned false status" );
  QVERIFY2( obj["count"].toInt(), "Remove program returned invalid count" );

  logout( &mgr, &eventLoop, &req );
}

void ProgramTest::retrieveAfterDelete()
{
  QEventLoop eventLoop;
  QNetworkAccessManager mgr;
  connect( &mgr, SIGNAL( finished( QNetworkReply* ) ), &eventLoop,
      SLOT( quit() ) );
  QNetworkRequest req;
  login( &mgr, &eventLoop, &req );

  const auto url = QString{ "http://localhost:3000/programs/id/%1/data" }.arg( programtest::id );
  const auto reply = get( url, &mgr, &eventLoop, &req );

  QVERIFY2( reply->error() == QNetworkReply::NoError, "Error retrieving deleted program" );
  const auto doc = QJsonDocument::fromJson( reply->readAll() );
  const auto obj = doc.object();
  QVERIFY2( obj.isEmpty(), "Non-Empty JSON response for deleted program id" );

  logout( &mgr, &eventLoop, &req );
}

void ProgramTest::cleanupTestCase()
{
  QEventLoop eventLoop;
  QNetworkAccessManager mgr;
  connect( &mgr, SIGNAL( finished( QNetworkReply* ) ), &eventLoop,
      SLOT( quit() ) );
  QNetworkRequest req;
  login( &mgr, &eventLoop, &req );

  const auto url = QString{ "http://localhost:3000/institutions/remove" };
  const auto data = QString( "id=%1" ).arg( programtest::institutionId );
  const auto reply = put( url, data, &mgr, &eventLoop, &req );

  QVERIFY2( reply->error() == QNetworkReply::NoError, "Error deleting institution" );
  const auto doc = QJsonDocument::fromJson( reply->readAll() );
  const auto obj = doc.object();
  QVERIFY2( !obj.isEmpty(), "Empty JSON response for delete institution" );
  QVERIFY2( obj["status"].toBool(), "Remove institution returned false status" );
  QVERIFY2( obj["count"].toInt(), "Remove institution returned invalid count" );

  logout( &mgr, &eventLoop, &req );
}
