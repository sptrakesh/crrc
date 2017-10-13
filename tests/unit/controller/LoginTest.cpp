#include "LoginTest.h"

#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkRequest>
#include <QtNetwork/QNetworkReply>
#include <QtCore/QUrl>
#include <QtCore/QUrlQuery>

using crrc::LoginTest;

void LoginTest::redirect()
{
  QEventLoop eventLoop;

  QNetworkAccessManager mgr;
  QObject::connect( &mgr, SIGNAL( finished( QNetworkReply* ) ), &eventLoop,
      SLOT( quit() ) );
  QNetworkRequest req( QUrl( QString( "http://localhost:3000/" ) ) );
  QNetworkReply* reply = mgr.get( req );
  eventLoop.exec();

  if ( reply->error() == QNetworkReply::NoError )
  {
    const auto code = reply->attribute( QNetworkRequest::HttpStatusCodeAttribute ).toInt();
    QVERIFY2( 302 == code, "Request not redirected to login page" );

    auto newUrl = reply->attribute( QNetworkRequest::RedirectionTargetAttribute ).toUrl();
    newUrl = reply->url().resolved( newUrl );
    QVERIFY2( newUrl.toString().contains( "/login" ), "Redirect URL invalid" );
    delete reply;
  }
  else
  {
    delete reply;
    QFAIL( "Error accessing server" );
  }
}

void LoginTest::valid()
{
  QEventLoop eventLoop;

  QNetworkAccessManager mgr;
  QObject::connect( &mgr, SIGNAL( finished( QNetworkReply* ) ), &eventLoop,
      SLOT( quit() ) );
  QNetworkRequest req( QUrl( QString( "http://localhost:3000/login" ) ) );
  req.setHeader( QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded" );
  const auto data = QString{ "username=admin&password=admin" };
  QNetworkReply* reply = mgr.post( req, data.toLocal8Bit() );
  eventLoop.exec();

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

void LoginTest::invalid()
{
  QEventLoop eventLoop;

  QNetworkAccessManager mgr;
  QObject::connect( &mgr, SIGNAL( finished( QNetworkReply* ) ), &eventLoop,
      SLOT( quit() ) );
  QNetworkRequest req( QUrl( QString( "http://localhost:3000/login" ) ) );
  req.setHeader( QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded" );
  const auto data = QString{ "username=_blah&password=_blah" };
  QNetworkReply* reply = mgr.post( req, data.toLocal8Bit() );
  eventLoop.exec();

  if ( reply->error() == QNetworkReply::NoError )
  {
    const auto code = reply->attribute( QNetworkRequest::HttpStatusCodeAttribute ).toInt();
    QVERIFY2( 200 == code, "Request redirected to main page" );

    auto newUrl = reply->attribute( QNetworkRequest::RedirectionTargetAttribute ).toUrl();
    newUrl = reply->url().resolved( newUrl );
    QVERIFY2( newUrl.toString().contains( "/login" ), "Login with invalid credentials succeeded." );
    delete reply;
  }
  else
  {
    delete reply;
    QFAIL( "Error logging into server" );
  }
}

