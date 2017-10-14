#include "DegreeTest.h"

#include <QtCore/QJsonDocument>
#include <QtCore/QJsonObject>
#include <QtCore/QUrl>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkRequest>
#include <QtNetwork/QNetworkReply>

namespace crrc
{
  namespace degreetest
  {
    static uint32_t id = 0;
  }
}

using crrc::DegreeTest;

void DegreeTest::create()
{
  QEventLoop eventLoop;
  QNetworkAccessManager mgr;
  connect( &mgr, SIGNAL( finished( QNetworkReply* ) ), &eventLoop,
      SLOT( quit() ) );
  QNetworkRequest req;
  login( &mgr, &eventLoop, &req );

  const auto url = QString{ "http://localhost:3000/degrees/create" };
  const auto data = QString( "title=%1&duration=%2" ).arg( title ).arg( duration );
  auto reply = post( url, data, &mgr, &eventLoop, &req );

  if ( reply->error() == QNetworkReply::NoError )
  {
    const auto doc = QJsonDocument::fromJson( reply->readAll() );
    const auto obj = doc.object();
    QVERIFY2( !obj.isEmpty(), "Empty JSON response for create degree" );
    QVERIFY2( obj["status"].toBool(), "Create degree returned false status" );
    QVERIFY2( obj["id"].toInt(), "Create degree returned invalid id" );
    degreetest::id = obj["id"].toInt();
  }
  else
  {
    QFAIL( "Error creating new degree" );
  }

  logout( &mgr, &eventLoop, &req );
}

void DegreeTest::retrieve()
{
  QEventLoop eventLoop;
  QNetworkAccessManager mgr;
  connect( &mgr, SIGNAL( finished( QNetworkReply* ) ), &eventLoop,
      SLOT( quit() ) );
  QNetworkRequest req;
  login( &mgr, &eventLoop, &req );

  const auto url = QString( "http://localhost:3000/degrees/id/%1/data" ).arg( degreetest::id );
  const auto reply = get( url, &mgr, &eventLoop, &req );

  if ( reply->error() == QNetworkReply::NoError )
  {
    const auto doc = QJsonDocument::fromJson( reply->readAll() );
    const auto obj = doc.object();
    QVERIFY2( !obj.isEmpty(), "Empty JSON response for degree JSON" );
    QVERIFY2( degreetest::id == obj["id"].toInt(), "Json response returned invalid id" );
    QVERIFY2( title == obj["title"].toString(), "Json response returned invalid title" );
    QVERIFY2( duration == obj["duration"].toString(), "Json response returned invalid title" );
  }
  else
  {
    QFAIL( "Error creating new degree" );
  }

  logout( &mgr, &eventLoop, &req );
}

void DegreeTest::update()
{
  QEventLoop eventLoop;
  QNetworkAccessManager mgr;
  connect( &mgr, SIGNAL( finished( QNetworkReply* ) ), &eventLoop,
      SLOT( quit() ) );
  QNetworkRequest req;
  login( &mgr, &eventLoop, &req );

  const auto url = QString( "http://localhost:3000/degrees/id/%1/update" ).arg( degreetest::id );
  const auto modified = "durationModified";
  const auto data = QString( "id=%1&title=%2&duration=%3" ).arg( degreetest::id ).arg( title ).arg( modified );
  const auto reply = post( url, data, &mgr, &eventLoop, &req );

  if ( reply->error() == QNetworkReply::NoError )
  {
    const auto doc = QJsonDocument::fromJson( reply->readAll() );
    const auto obj = doc.object();
    QVERIFY2( !obj.isEmpty(), "Empty JSON response for create degree" );
    QVERIFY2( degreetest::id == obj["id"].toInt(), "Json response returned invalid id" );
  }
  else
  {
    QFAIL( "Error updating degree" );
  }

  logout( &mgr, &eventLoop, &req );
}

void DegreeTest::remove()
{
  QEventLoop eventLoop;
  QNetworkAccessManager mgr;
  connect( &mgr, SIGNAL( finished( QNetworkReply* ) ), &eventLoop,
      SLOT( quit() ) );
  QNetworkRequest req;
  login( &mgr, &eventLoop, &req );

  const auto url = QString{ "http://localhost:3000/degrees/remove" };
  const auto data = QString( "id=%1" ).arg( degreetest::id );
  const auto reply = post( url, data, &mgr, &eventLoop, &req );

  if ( reply->error() == QNetworkReply::NoError )
  {
    const auto doc = QJsonDocument::fromJson( reply->readAll() );
    const auto obj = doc.object();
    QVERIFY2( !obj.isEmpty(), "Empty JSON response for create degree" );
    QVERIFY2( obj["status"].toBool(), "Remove degree returned false status" );
    QVERIFY2( obj["count"].toInt(), "Remove degree returned invalid count" );
  }
  else
  {
    QFAIL( "Error creating new degree" );
  }

  logout( &mgr, &eventLoop, &req );
}
