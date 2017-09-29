#pragma once

#include <QtCore/QDateTime>
#include <QtCore/QCryptographicHash>
#include <QtCore/QJsonDocument>
#include <QtCore/QJsonObject>
#include <QtCore/QLocale>
#include <QtCore/QStringBuilder>

#include <Cutelyst/Controller>
#include "model/User.h"


namespace crrc
{
  namespace dao
  {
    inline QString checksum( const QByteArray& bytes )
    {
      return QString::fromLatin1(
        QCryptographicHash::hash( bytes, QCryptographicHash::Md5 ).toBase64() );
    }

    inline QString httpDate( const QDateTime& date )
    {
      return QLocale::c().toString( date,
        QLatin1String( "ddd, dd MMM yyyy hh:mm:ss 'GMT'" ) ).toLatin1();
    }

    inline uint32_t roleId( Cutelyst::Context* context )
    {
      const auto user = qvariant_cast<model::User*>( context->stash( "user" ) );
      return user->getRoleId();
    }

    inline bool isGlobalAdmin( Cutelyst::Context* context )
    {
      return ( roleId( context ) == 1u );
    }

    inline uint32_t institutionId( Cutelyst::Context* context )
    {
      const auto& institution = context->stash( "userInstitution" ).toHash();
      return institution.value( "institution_id" ).toUInt();
    }

    inline void sendJson( Cutelyst::Context* context, const QJsonObject& obj )
    {
      const auto bytes = QJsonDocument( obj ).toJson();
      context->response()->setContentType( "application/json" );
      context->response()->setContentLength( bytes.size() );
      context->response()->setBody( bytes );
    }
  }
}
