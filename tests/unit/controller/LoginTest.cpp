#include "LoginTest.h"

#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkRequest>
#include <QtNetwork/QNetworkReply>
#include <QtCore/QUrl>

using crrc::LoginTest;

void LoginTest::redirect()
{
  QEventLoop eventLoop;

  QNetworkAccessManager mgr;
  QObject::connect( &mgr, SIGNAL( finished( QNetworkReply* ) ), &eventLoop,
      SLOT( quit() ) );
  QNetworkRequest req( QUrl( QString( "http://localhost:3000/" ) ) );
  QNetworkReply* reply = mgr.get( req );
  std::unique_ptr<QNetworkReply> rptr{ reply };
  eventLoop.exec();

  QVERIFY2( reply->error() == QNetworkReply::NoError, "Error accessing server" );
  const auto code = reply->attribute( QNetworkRequest::HttpStatusCodeAttribute ).toInt();
  QVERIFY2( 302 == code, "Request not redirected to login page" );

  auto newUrl = reply->attribute( QNetworkRequest::RedirectionTargetAttribute ).toUrl();
  newUrl = reply->url().resolved( newUrl );
  QVERIFY2( newUrl.toString().contains( "/login" ), "Redirect URL invalid" );
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
  std::unique_ptr<QNetworkReply> rptr{ reply };
  eventLoop.exec();

  QVERIFY2( reply->error() == QNetworkReply::NoError, "Error logging into server" );
  const auto code = reply->attribute( QNetworkRequest::HttpStatusCodeAttribute ).toInt();
  QVERIFY2( 302 == code, "Request not redirected to main page" );

  auto newUrl = reply->attribute( QNetworkRequest::RedirectionTargetAttribute ).toUrl();
  newUrl = reply->url().resolved( newUrl );
  QVERIFY2( !newUrl.toString().contains( "/login" ), "Login failed.  Redirected back to login page." );
}

void LoginTest::logout()
{
  QEventLoop eventLoop;

  QNetworkAccessManager mgr;
  QObject::connect( &mgr, SIGNAL( finished( QNetworkReply* ) ), &eventLoop,
      SLOT( quit() ) );
  QNetworkRequest req( QUrl( QString( "http://localhost:3000/login" ) ) );
  req.setHeader( QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded" );
  const auto data = QString{ "username=admin&password=admin" };
  QNetworkReply* reply = mgr.post( req, data.toLocal8Bit() );
  std::unique_ptr<QNetworkReply> rptr{ reply };
  eventLoop.exec();

  QVERIFY2( reply->error() == QNetworkReply::NoError, "Error logging into server" );
  const auto code = reply->attribute( QNetworkRequest::HttpStatusCodeAttribute ).toInt();
  QVERIFY2( 302 == code, "Request not redirected to main page" );

  auto newUrl = reply->attribute( QNetworkRequest::RedirectionTargetAttribute ).toUrl();
  newUrl = reply->url().resolved( newUrl );
  QVERIFY2( !newUrl.toString().contains( "/login" ), "Login failed.  Redirected back to login page." );

  req.setUrl( QUrl( QString( "http://localhost:3000/logout" ) ) );
  QNetworkReply* temp = mgr.get( req );
  rptr.reset( temp );
  reply = rptr.get();

  eventLoop.exec();
  QVERIFY2( reply->error() == QNetworkReply::NoError, "Error logging out of server" );
  const auto code1 = reply->attribute( QNetworkRequest::HttpStatusCodeAttribute ).toInt();
  QVERIFY2( 302 == code1, "Request not redirected to login page" );

  newUrl = reply->attribute( QNetworkRequest::RedirectionTargetAttribute ).toUrl();
  newUrl = reply->url().resolved( newUrl );
  QVERIFY2( newUrl.toString().endsWith( ":3000/" ), "Logout failed.  Not redirected back to login page." );
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
  std::unique_ptr<QNetworkReply> rptr{ reply };
  eventLoop.exec();

  QVERIFY2( reply->error() == QNetworkReply::NoError, "Error logging into server" );
  const auto code = reply->attribute( QNetworkRequest::HttpStatusCodeAttribute ).toInt();
  QVERIFY2( 200 == code, "Request redirected to main page" );

  auto newUrl = reply->attribute( QNetworkRequest::RedirectionTargetAttribute ).toUrl();
  newUrl = reply->url().resolved( newUrl );
  QVERIFY2( newUrl.toString().contains( "/login" ), "Login with invalid credentials succeeded." );
}

