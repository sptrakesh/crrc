#include "InstitutionDesignationTest.h"

#include <QtCore/QJsonArray>
#include <QtCore/QJsonDocument>
#include <QtCore/QJsonObject>

namespace crrc
{
  namespace institutionDesignationtest
  {
    static QString institutionName{ "Unit Test Institution" };
    static QString institutionCity{ "Unit Test City" };
    static uint32_t institutionId = 0;

    static QString designationTitle{ "UnitTestDesignation" };
    static QString designationType{ "UT" };
    static uint32_t designationId = 0;
  }
}

using crrc::InstitutionDesignationTest;

void InstitutionDesignationTest::initTestCase()
{
  createInstitution();
  createDesignation();
}

void InstitutionDesignationTest::createInstitution()
{
  QEventLoop eventLoop;
  QNetworkAccessManager mgr;
  connect( &mgr, SIGNAL( finished( QNetworkReply* ) ), &eventLoop,
      SLOT( quit() ) );
  QNetworkRequest req;
  login( &mgr, &eventLoop, &req );

  const auto url = QString{ "http://localhost:3000/institutions/edit" };
  const auto data = QString( "name=%1&city=%2" ).
      arg( institutionDesignationtest::institutionName ).arg( institutionDesignationtest::institutionCity );
  auto reply = put( url, data, &mgr, &eventLoop, &req );

  QVERIFY2( reply->error() == QNetworkReply::NoError, "Error creating new institution" );
  const auto doc = QJsonDocument::fromJson( reply->readAll() );
  const auto obj = doc.object();
  QVERIFY2( !obj.isEmpty(), "Empty JSON response for create institution" );
  QVERIFY2( obj["id"].toInt(), "Create institution returned invalid id" );
  institutionDesignationtest::institutionId = obj["id"].toInt();

  logout( &mgr, &eventLoop, &req );
}

void InstitutionDesignationTest::createDesignation()
{
  QEventLoop eventLoop;
  QNetworkAccessManager mgr;
  connect( &mgr, SIGNAL( finished( QNetworkReply* ) ), &eventLoop,
      SLOT( quit() ) );
  QNetworkRequest req;
  login( &mgr, &eventLoop, &req );

  const auto url = QString{ "http://localhost:3000/designations/save" };
  const auto data = QString{ "title=%1&type=%2" }.
      arg( institutionDesignationtest::designationTitle ).
      arg( institutionDesignationtest::designationType );
  auto reply = post( url, data, &mgr, &eventLoop, &req );

  QVERIFY2( reply->error() == QNetworkReply::NoError, "Error creating new designation" );
  const auto doc = QJsonDocument::fromJson( reply->readAll() );
  const auto obj = doc.object();
  QVERIFY2( !obj.isEmpty(), "Empty JSON response for create designation" );
  QVERIFY2( obj["id"].toInt(), "Create designation returned invalid id" );
  institutionDesignationtest::designationId = obj["id"].toInt();

  logout( &mgr, &eventLoop, &req );
}

void InstitutionDesignationTest::create()
{
  QEventLoop eventLoop;
  QNetworkAccessManager mgr;
  connect( &mgr, SIGNAL( finished( QNetworkReply* ) ), &eventLoop,
      SLOT( quit() ) );
  QNetworkRequest req;
  login( &mgr, &eventLoop, &req );

  const auto url = QString{ "http://localhost:3000/institution/designations/save" };
  const auto data = QString{ "institutionId=%1&designationId=%2" }
      .arg( institutionDesignationtest::institutionId ).arg( institutionDesignationtest::designationId );
  auto reply = post( url, data, &mgr, &eventLoop, &req );

  QVERIFY2( reply->error() == QNetworkReply::NoError, "Error creating new InstitutionDesignation" );
  const auto doc = QJsonDocument::fromJson( reply->readAll() );
  const auto obj = doc.object();
  QVERIFY2( !obj.isEmpty(), "Empty JSON response for create InstitutionDesignation" );
  QVERIFY2( obj["status"].toBool(), "Create InstitutionDesignation returned invalid status" );

  logout( &mgr, &eventLoop, &req );
}

void InstitutionDesignationTest::retrieve()
{
  QEventLoop eventLoop;
  QNetworkAccessManager mgr;
  connect( &mgr, SIGNAL( finished( QNetworkReply* ) ), &eventLoop,
      SLOT( quit() ) );
  QNetworkRequest req;
  login( &mgr, &eventLoop, &req );

  const auto url = QString{ "http://localhost:3000/institution/designations/id/%1/view" }
      .arg( institutionDesignationtest::institutionId );
  const auto data = QString{};
  const auto reply = post( url, data, &mgr, &eventLoop, &req );

  QVERIFY2( reply->error() == QNetworkReply::NoError, "Error retrieving designation list for institution" );
  const auto doc = QJsonDocument::fromJson( reply->readAll() );
  const auto arr = doc.array();
  QVERIFY2( !arr.isEmpty(), "Empty JSON response for designation list" );

  bool found = false;
  for ( const auto& obj : arr )
  {
    const auto& designation = obj.toObject()["designation"].toObject();
    if ( institutionDesignationtest::designationId == designation["id"].toInt() )
    {
      found = true;
      break;
    }
  }

  QVERIFY2( found, "Newly created institution designation not returned in list" );

  logout( &mgr, &eventLoop, &req );
}

void InstitutionDesignationTest::update()
{
  QEventLoop eventLoop;
  QNetworkAccessManager mgr;
  connect( &mgr, SIGNAL( finished( QNetworkReply* ) ), &eventLoop,
      SLOT( quit() ) );
  QNetworkRequest req;
  login( &mgr, &eventLoop, &req );

  const auto url = QString{ "http://localhost:3000/institution/designations/save" };
  const auto data = QString{ "institutionId=%1&designationId=%2&expiration=%3" }
      .arg( institutionDesignationtest::institutionId )
      .arg( institutionDesignationtest::designationId )
      .arg( expiration );
  auto reply = post( url, data, &mgr, &eventLoop, &req );

  QVERIFY2( reply->error() == QNetworkReply::NoError, "Error updating InstitutionDesignation" );
  const auto doc = QJsonDocument::fromJson( reply->readAll() );
  const auto obj = doc.object();
  QVERIFY2( !obj.isEmpty(), "Empty JSON response for create InstitutionDesignation" );
  QVERIFY2( obj["status"].toBool(), "Create InstitutionDesignation returned invalid status" );

  logout( &mgr, &eventLoop, &req );
}

void InstitutionDesignationTest::retrieveModified()
{
  QEventLoop eventLoop;
  QNetworkAccessManager mgr;
  connect( &mgr, SIGNAL( finished( QNetworkReply* ) ), &eventLoop,
      SLOT( quit() ) );
  QNetworkRequest req;
  login( &mgr, &eventLoop, &req );

  const auto url = QString{ "http://localhost:3000/institution/designations/id/%1/view" }
      .arg( institutionDesignationtest::institutionId );
  const auto data = QString{};
  const auto reply = post( url, data, &mgr, &eventLoop, &req );

  QVERIFY2( reply->error() == QNetworkReply::NoError, "Error retrieving designation list for institution" );
  const auto doc = QJsonDocument::fromJson( reply->readAll() );
  const auto arr = doc.array();
  QVERIFY2( !arr.isEmpty(), "Empty JSON response for designation list" );

  bool found = false;
  for ( const auto& obj : arr )
  {
    if ( expiration == obj.toObject()["expiration"].toInt() )
    {
      found = true;
      break;
    }
  }

  QVERIFY2( found, "Updated institution designation not returned in list" );

  logout( &mgr, &eventLoop, &req );
}

void InstitutionDesignationTest::remove()
{
  QEventLoop eventLoop;
  QNetworkAccessManager mgr;
  connect( &mgr, SIGNAL( finished( QNetworkReply* ) ), &eventLoop,
      SLOT( quit() ) );
  QNetworkRequest req;
  login( &mgr, &eventLoop, &req );

  const auto url = QString{ "http://localhost:3000/institution/designations/remove" };
  const auto data = QString{ "institutionId=%1&designationId=%2" }
      .arg( institutionDesignationtest::institutionId ).arg( institutionDesignationtest::designationId );
  auto reply = post( url, data, &mgr, &eventLoop, &req );

  QVERIFY2( reply->error() == QNetworkReply::NoError, "Error removing InstitutionDesignation" );
  const auto doc = QJsonDocument::fromJson( reply->readAll() );
  const auto obj = doc.object();
  QVERIFY2( !obj.isEmpty(), "Empty JSON response for remove InstitutionDesignation" );
  QVERIFY2( obj["status"].toBool(), "Remove InstitutionDesignation returned invalid status" );

  logout( &mgr, &eventLoop, &req );
}

void InstitutionDesignationTest::retrieveAfterDelete()
{
  QEventLoop eventLoop;
  QNetworkAccessManager mgr;
  connect( &mgr, SIGNAL( finished( QNetworkReply* ) ), &eventLoop,
      SLOT( quit() ) );
  QNetworkRequest req;
  login( &mgr, &eventLoop, &req );

  const auto url = QString{ "http://localhost:3000/institution/designations/id/%1/view" }
      .arg( institutionDesignationtest::institutionId );
  const auto data = QString{};
  const auto reply = post( url, data, &mgr, &eventLoop, &req );

  QVERIFY2( reply->error() == QNetworkReply::NoError, "Error retrieving designation list for institution" );
  const auto doc = QJsonDocument::fromJson( reply->readAll() );
  const auto arr = doc.array();
  QVERIFY2( arr.isEmpty(), "Non-empty JSON response for designation list after delete" );

  bool found = false;
  for ( const auto& obj : arr )
  {
    if ( institutionDesignationtest::designationId == obj.toObject()["id"].toInt() )
    {
      found = true;
      break;
    }
  }

  QVERIFY2( !found, "Removed institution designation returned in list" );

  logout( &mgr, &eventLoop, &req );
}

void InstitutionDesignationTest::cleanupTestCase()
{
  removeDesignation();
  removeInstitution();
}

void InstitutionDesignationTest::removeInstitution()
{
  QEventLoop eventLoop;
  QNetworkAccessManager mgr;
  connect( &mgr, SIGNAL( finished( QNetworkReply* ) ), &eventLoop,
      SLOT( quit() ) );
  QNetworkRequest req;
  login( &mgr, &eventLoop, &req );

  const auto url = QString{ "http://localhost:3000/institutions/remove" };
  const auto data = QString( "id=%1" ).arg( institutionDesignationtest::institutionId );
  const auto reply = put( url, data, &mgr, &eventLoop, &req );

  QVERIFY2( reply->error() == QNetworkReply::NoError, "Error deleting institution" );
  const auto doc = QJsonDocument::fromJson( reply->readAll() );
  const auto obj = doc.object();
  QVERIFY2( !obj.isEmpty(), "Empty JSON response for delete institution" );
  QVERIFY2( obj["status"].toBool(), "Remove institution returned false status" );
  QVERIFY2( obj["count"].toInt(), "Remove institution returned invalid count" );

  logout( &mgr, &eventLoop, &req );
}

void InstitutionDesignationTest::removeDesignation()
{
  QEventLoop eventLoop;
  QNetworkAccessManager mgr;
  connect( &mgr, SIGNAL( finished( QNetworkReply* ) ), &eventLoop,
      SLOT( quit() ) );
  QNetworkRequest req;
  login( &mgr, &eventLoop, &req );

  const auto url = QString{ "http://localhost:3000/designations/remove" };
  const auto data = QString( "id=%1" ).arg( institutionDesignationtest::designationId );
  const auto reply = post( url, data, &mgr, &eventLoop, &req );

  QVERIFY2( reply->error() == QNetworkReply::NoError, "Error deleting designation" );
  const auto doc = QJsonDocument::fromJson( reply->readAll() );
  const auto obj = doc.object();
  QVERIFY2( !obj.isEmpty(), "Empty JSON response for delete designation" );
  QVERIFY2( obj["status"].toBool(), "Remove designation returned false status" );
  QVERIFY2( obj["count"].toInt(), "Remove designation returned invalid count" );

  logout( &mgr, &eventLoop, &req );
}
