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

  req.setUrl( QUrl( QString( "http://localhost:3000/degrees/create" ) ) );
  req.setHeader( QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded" );
  const auto data = QString( "title=%1&duration=%2" ).arg( title ).arg( duration );
  QNetworkReply* reply = mgr.post( req, data.toLocal8Bit() );
  eventLoop.exec();

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
    delete reply;
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

  req.setUrl( QUrl( QString( "http://localhost:3000/degrees/id/%1/data" ).arg( degreetest::id ) ) );
  QNetworkReply* reply = mgr.get( req );
  eventLoop.exec();

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
    delete reply;
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

  req.setUrl( QUrl( QString( "http://localhost:3000/degrees/id/%1/update" ).arg( degreetest::id ) ) );
  req.setHeader( QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded" );
  const auto modified = "durationModified";
  const auto data = QString( "id=%1&title=%2&duration=%3" ).arg( degreetest::id ).arg( title ).arg( modified );
  QNetworkReply* reply = mgr.post( req, data.toLocal8Bit() );
  eventLoop.exec();

  if ( reply->error() == QNetworkReply::NoError )
  {
    const auto doc = QJsonDocument::fromJson( reply->readAll() );
    const auto obj = doc.object();
    QVERIFY2( !obj.isEmpty(), "Empty JSON response for create degree" );
    QVERIFY2( degreetest::id == obj["id"].toInt(), "Json response returned invalid id" );
    QVERIFY2( title == obj["title"].toString(), "Json response returned invalid title" );
    QVERIFY2( modified == obj["duration"].toString(), "Json response returned invalid title" );
  }
  else
  {
    delete reply;
    QFAIL( "Error creating new degree" );
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

  req.setUrl( QUrl( QString( "http://localhost:3000/degrees/remove" ) ) );
  req.setHeader( QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded" );
  const auto data = QString( "id=%1" ).arg( degreetest::id );
  QNetworkReply* reply = mgr.post( req, data.toLocal8Bit() );
  eventLoop.exec();

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
    delete reply;
    QFAIL( "Error creating new degree" );
  }

  logout( &mgr, &eventLoop, &req );
}
