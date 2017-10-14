#include "BaseTest.h"

#include <QtTest/QTest>

using crrc::BaseTest;

void BaseTest::login( const QString& username, const QString& password,
    QNetworkAccessManager* mgr, QEventLoop* eventLoop, QNetworkRequest* req )
{
  req->setUrl( QUrl( QString( "http://localhost:3000/login" ) ) );
  req->setHeader( QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded" );
  const auto data = QString{ "username=%1&password=%2" }.arg( username ).arg( password );
  ReplyPointer reply{ mgr->post( *req, data.toLocal8Bit() ) };
  eventLoop->exec();

  QVERIFY2( reply->error() == QNetworkReply::NoError, "Error logging into server" );
  const auto code = reply->attribute( QNetworkRequest::HttpStatusCodeAttribute ).toInt();
  QVERIFY2( 302 == code, "Request not redirected to main page" );

  auto newUrl = reply->attribute( QNetworkRequest::RedirectionTargetAttribute ).toUrl();
  newUrl = reply->url().resolved( newUrl );
  QVERIFY2( !newUrl.toString().contains( "/login" ), "Login failed.  Redirected back to login page." );
}

void BaseTest::logout( QNetworkAccessManager* mgr, QEventLoop* eventLoop, QNetworkRequest* req )
{
  req->setUrl( QUrl( QString( "http://localhost:3000/logout" ) ) );
  ReplyPointer reply{ mgr->get( *req ) };
  eventLoop->exec();
  QVERIFY2( reply->error() == QNetworkReply::NoError, "Error logging out of server" );
  const auto code = reply->attribute( QNetworkRequest::HttpStatusCodeAttribute ).toInt();
  QVERIFY2( 302 == code, "Request not redirected to login page" );

  auto newUrl = reply->attribute( QNetworkRequest::RedirectionTargetAttribute ).toUrl();
  newUrl = reply->url().resolved( newUrl );
  QVERIFY2( newUrl.toString().endsWith( ":3000/" ), "Logout failed.  Not redirected back to login page." );
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
