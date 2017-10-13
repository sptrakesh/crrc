#pragma once

#include <QtCore/QObject>
#include <QtCore/QEventLoop>
#include <QtCore/QUrl>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkRequest>


namespace crrc
{
  class BaseTest : public QObject
  {
    Q_OBJECT
  protected:
    BaseTest( QObject* parent = nullptr ) : QObject( parent ){}

    void login( QNetworkAccessManager* mgr, QEventLoop* eventLoop,
      QNetworkRequest* req );

    void logout( QNetworkAccessManager* mgr, QEventLoop* eventLoop,
      QNetworkRequest* req );
  };
}
