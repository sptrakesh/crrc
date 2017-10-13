#include "BaseTest.h"

#include <QtTest/QTest>

using crrc::BaseTest;

void BaseTest::login( QNetworkAccessManager* mgr, QEventLoop* eventLoop, QNetworkRequest* req )
{
  req->setUrl( QUrl( QString( "http://localhost:3000/login" ) ) );
  req->setHeader( QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded" );
  const auto data = QString{ "username=admin&password=admin" };
  QNetworkReply* reply = mgr->post( *req, data.toLocal8Bit() );
  eventLoop->exec();

  if ( reply->error() == QNetworkReply::NoError )
  {
    const auto code = reply->attribute( QNetworkRequest::HttpStatusCodeAttribute ).toInt();
    QVERIFY2( 302 == code, "Request not redirected to main page" );

    auto newUrl = reply->attribute( QNetworkRequest::RedirectionTargetAttribute ).toUrl();
    newUrl = reply->url().resolved( newUrl );
    QVERIFY2( !newUrl.toString().contains( "/login" ), "Login failed.  Redirected back to login page." );

    delete reply;
  }
  else
  {
    delete reply;
    QFAIL( "Error logging into server" );
  }
}

void BaseTest::logout( QNetworkAccessManager* mgr, QEventLoop* eventLoop, QNetworkRequest* req )
{
  req->setUrl( QUrl( QString( "http://localhost:3000/logout" ) ) );
  QNetworkReply* reply = mgr->get( *req );
  eventLoop->exec();
  if ( reply->error() == QNetworkReply::NoError )
  {
    const auto code = reply->attribute( QNetworkRequest::HttpStatusCodeAttribute ).toInt();
    QVERIFY2( 302 == code, "Request not redirected to login page" );

    auto newUrl = reply->attribute( QNetworkRequest::RedirectionTargetAttribute ).toUrl();
    newUrl = reply->url().resolved( newUrl );
    QVERIFY2( newUrl.toString().endsWith( ":3000/" ), "Logout failed.  Not redirected back to login page." );

    delete reply;
  }
  else
  {
    delete reply;
    QFAIL( "Error logging out of server" );
  }
}
