#include "InstitutionTest.h"

#include <QtCore/QJsonArray>
#include <QtCore/QJsonDocument>
#include <QtCore/QJsonObject>

namespace crrc
{
  namespace institutiontest
  {
    static uint32_t id = 0;
  }
}

using crrc::InstitutionTest;

void InstitutionTest::create()
{
  QEventLoop eventLoop;
  QNetworkAccessManager mgr;
  connect( &mgr, SIGNAL( finished( QNetworkReply* ) ), &eventLoop,
      SLOT( quit() ) );
  QNetworkRequest req;
  login( &mgr, &eventLoop, &req );

  const auto url = QString{ "http://localhost:3000/institutions/edit" };
  const auto data = QString{ "name=%1&city=%2" }.arg( name ).arg( city );
  auto reply = put( url, data, &mgr, &eventLoop, &req );

  QVERIFY2( reply->error() == QNetworkReply::NoError, "Error creating new institution" );
  const auto doc = QJsonDocument::fromJson( reply->readAll() );
  const auto obj = doc.object();
  QVERIFY2( !obj.isEmpty(), "Empty JSON response for create institution" );
  QVERIFY2( obj["id"].toInt(), "Create institution returned invalid id" );
  institutiontest::id = obj["id"].toInt();

  logout( &mgr, &eventLoop, &req );
}

void InstitutionTest::retrieve()
{
  QEventLoop eventLoop;
  QNetworkAccessManager mgr;
  connect( &mgr, SIGNAL( finished( QNetworkReply* ) ), &eventLoop,
      SLOT( quit() ) );
  QNetworkRequest req;
  login( &mgr, &eventLoop, &req );

  const auto url = QString{ "http://localhost:3000/institutions/id/%1/data" }.arg( institutiontest::id );
  const auto reply = get( url, &mgr, &eventLoop, &req );

  QVERIFY2( reply->error() == QNetworkReply::NoError, "Error retrieving institution" );
  const auto doc = QJsonDocument::fromJson( reply->readAll() );
  const auto obj = doc.object();
  QVERIFY2( !obj.isEmpty(), "Empty JSON response for institution id" );
  QVERIFY2( institutiontest::id == obj["id"].toInt(), "Json response returned invalid id" );
  QVERIFY2( name == obj["name"].toString(), "Json response returned invalid name" );
  QVERIFY2( city == obj["city"].toString(), "Json response returned invalid city" );

  logout( &mgr, &eventLoop, &req );
}

void InstitutionTest::invalid()
{
  QEventLoop eventLoop;
  QNetworkAccessManager mgr;
  connect( &mgr, SIGNAL( finished( QNetworkReply* ) ), &eventLoop,
      SLOT( quit() ) );
  QNetworkRequest req;
  login( &mgr, &eventLoop, &req );

  const auto url = QString{ "http://localhost:3000/institutions/id/0/data" };
  const auto reply = get( url, &mgr, &eventLoop, &req );

  QVERIFY2( reply->error() == QNetworkReply::NoError, "Error retrieving invalid institution" );
  const auto doc = QJsonDocument::fromJson( reply->readAll() );
  const auto obj = doc.object();
  QVERIFY2( obj.isEmpty(), "Non-Empty JSON response for invalid institution id" );

  logout( &mgr, &eventLoop, &req );
}

void InstitutionTest::checkUnique()
{
  QEventLoop eventLoop;
  QNetworkAccessManager mgr;
  connect( &mgr, SIGNAL( finished( QNetworkReply* ) ), &eventLoop,
      SLOT( quit() ) );
  QNetworkRequest req;
  login( &mgr, &eventLoop, &req );

  const auto url = QString{ "http://localhost:3000/institutions/checkUnique/?name=%1&city=%2" }
      .arg( name ).arg( city );
  const auto reply = get( url, &mgr, &eventLoop, &req );

  QVERIFY2( reply->error() == QNetworkReply::NoError, "Error retrieving institution" );
  const QString text = reply->readAll();
  QVERIFY2( !text.isEmpty(), "Empty response for name/city check" );
  QVERIFY2( "false" == text, "Json response returned true for existing institution" );

  logout( &mgr, &eventLoop, &req );
}

void InstitutionTest::checkInvalidUnique()
{
  QEventLoop eventLoop;
  QNetworkAccessManager mgr;
  connect( &mgr, SIGNAL( finished( QNetworkReply* ) ), &eventLoop,
      SLOT( quit() ) );
  QNetworkRequest req;
  login( &mgr, &eventLoop, &req );

  const auto url = QString{ "http://localhost:3000/institutions/checkUnique/?name=_blah_&city=_blah_" };
  const auto reply = get( url, &mgr, &eventLoop, &req );

  QVERIFY2( reply->error() == QNetworkReply::NoError, "Error retrieving institution" );
  const QString text = reply->readAll();
  QVERIFY2( !text.isEmpty(), "Empty response for name/city check" );
  QVERIFY2( "true" == text, "Json response returned false for non-existing institution" );

  logout( &mgr, &eventLoop, &req );
}

void InstitutionTest::index()
{
  QEventLoop eventLoop;
  QNetworkAccessManager mgr;
  connect( &mgr, SIGNAL( finished( QNetworkReply* ) ), &eventLoop,
      SLOT( quit() ) );
  QNetworkRequest req;
  login( &mgr, &eventLoop, &req );

  const auto url = QString{ "http://localhost:3000/institutions" };
  const auto data = QString{};
  const auto reply = post( url, data, &mgr, &eventLoop, &req );

  QVERIFY2( reply->error() == QNetworkReply::NoError, "Error retrieving institution list" );
  const auto doc = QJsonDocument::fromJson( reply->readAll() );
  const auto arr = doc.array();
  QVERIFY2( !arr.isEmpty(), "Empty JSON response for institution list" );

  bool found = false;
  for ( const auto& obj : arr )
  {
    if ( institutiontest::id == obj.toObject()["id"].toInt() )
    {
      found = true;
      break;
    }
  }

  QVERIFY2( found, "Newly created institution not returned in list" );

  logout( &mgr, &eventLoop, &req );
}

void InstitutionTest::update()
{
  QEventLoop eventLoop;
  QNetworkAccessManager mgr;
  connect( &mgr, SIGNAL( finished( QNetworkReply* ) ), &eventLoop,
      SLOT( quit() ) );
  QNetworkRequest req;
  login( &mgr, &eventLoop, &req );

  const auto url = QString{ "http://localhost:3000/institutions/edit" };
  const auto address = QString{ "1 Main St" };
  const auto data = QString{ "id=%1&name=%2&city=%3&address=%4&institutionTypeId=%5" }
      .arg( institutiontest::id ).arg( name ).arg( city ).arg( address ).arg( institutionTypeId );
  const auto reply = put( url, data, &mgr, &eventLoop, &req );

  QVERIFY2( reply->error() == QNetworkReply::NoError, "Error updating institution" );
  const auto doc = QJsonDocument::fromJson( reply->readAll() );
  const auto obj = doc.object();
  QVERIFY2( !obj.isEmpty(), "Empty JSON response for create institution" );
  QVERIFY2( institutiontest::id == obj["id"].toInt(), "Json response returned invalid id" );
  QVERIFY2( address == obj["address"].toString(), "Json response returned invalid address" );
  const auto instType = obj["institutionType"].toObject();
  QVERIFY2( static_cast<int>( institutionTypeId ) == instType["id"].toInt(), "Json response returned invalid institution type" );

  logout( &mgr, &eventLoop, &req );
}

void InstitutionTest::remove()
{
  QEventLoop eventLoop;
  QNetworkAccessManager mgr;
  connect( &mgr, SIGNAL( finished( QNetworkReply* ) ), &eventLoop,
      SLOT( quit() ) );
  QNetworkRequest req;
  login( &mgr, &eventLoop, &req );

  const auto url = QString{ "http://localhost:3000/institutions/remove" };
  const auto data = QString( "id=%1" ).arg( institutiontest::id );
  const auto reply = put( url, data, &mgr, &eventLoop, &req );

  QVERIFY2( reply->error() == QNetworkReply::NoError, "Error deleting institution" );
  const auto doc = QJsonDocument::fromJson( reply->readAll() );
  const auto obj = doc.object();
  QVERIFY2( !obj.isEmpty(), "Empty JSON response for delete institution" );
  QVERIFY2( obj["status"].toBool(), "Remove institution returned false status" );
  QVERIFY2( obj["count"].toInt(), "Remove institution returned invalid count" );

  logout( &mgr, &eventLoop, &req );
}
