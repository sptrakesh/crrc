#include "BaseTest.h"

#include <QtCore/QJsonDocument>
#include <QtCore/QJsonObject>
#include <QtTest/QTest>

using crrc::BaseTest;

void BaseTest::login( const QString& username, const QString& password,
    QNetworkAccessManager* mgr, QEventLoop* eventLoop, QNetworkRequest* req )
{
  req->setUrl( QUrl( QString( "http://localhost:3000/login" ) ) );
  req->setHeader( QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded" );
  const auto data = QString{ "username=%1&password=%2" }.arg( username ).arg( password );
  ReplyPointer reply{ mgr->put( *req, data.toLocal8Bit() ) };
  eventLoop->exec();

  QVERIFY2( reply->error() == QNetworkReply::NoError, "Error logging into server" );
  const auto code = reply->attribute( QNetworkRequest::HttpStatusCodeAttribute ).toInt();
  QVERIFY2( 200 == code, "PUT request redirected" );

  const auto doc = QJsonDocument::fromJson( reply->readAll() );
  const auto obj = doc.object();
  QVERIFY2( obj["status"].toBool(), "Login returned incorrect status" );
}

void BaseTest::logout( QNetworkAccessManager* mgr, QEventLoop* eventLoop, QNetworkRequest* req )
{
  req->setUrl( QUrl( QString( "http://localhost:3000/logout" ) ) );
  ReplyPointer reply{ mgr->post( *req, QByteArray{} ) };
  eventLoop->exec();
  QVERIFY2( reply->error() == QNetworkReply::NoError, "Error logging out of server" );
  const auto code = reply->attribute( QNetworkRequest::HttpStatusCodeAttribute ).toInt();
  QVERIFY2( 200 == code, "POST request redirected to login page" );

  const auto doc = QJsonDocument::fromJson( reply->readAll() );
  const auto obj = doc.object();
  QVERIFY2( obj["status"].toBool(), "Logout returned incorrect status" );
}

BaseTest::ReplyPointer
BaseTest::get( const QString& url, QNetworkAccessManager* mgr,
    QEventLoop* eventLoop, QNetworkRequest* req )
{
  req->setUrl( QUrl( url ) );
  ReplyPointer rptr{ mgr->get( *req ) };
  eventLoop->exec();
  return std::move( rptr );
}

BaseTest::ReplyPointer
BaseTest::post( const QString& url, const QString& data,
    QNetworkAccessManager* mgr, QEventLoop* eventLoop, QNetworkRequest* req )
{
  req->setUrl( QUrl( url ) );
  req->setHeader( QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded" );
  ReplyPointer rptr{ mgr->post( *req, data.toLocal8Bit() ) };
  eventLoop->exec();
  return std::move( rptr );
}

BaseTest::ReplyPointer BaseTest::put( const QString& url, const QString& data,
    QNetworkAccessManager* mgr, QEventLoop* eventLoop, QNetworkRequest* req )
{
  req->setUrl( QUrl( url ) );
  req->setHeader( QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded" );
  ReplyPointer rptr{ mgr->put( *req, data.toLocal8Bit() ) };
  eventLoop->exec();
  return std::move( rptr );
}
