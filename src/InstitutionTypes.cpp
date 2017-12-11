#include "InstitutionTypes.h"
#include "dao/InstitutionTypeDAO.h"
#include "dao/functions.h"
#include "model/InstitutionType.h"

#include <QtCore/QDebug>
#include <QtCore/QJsonArray>


namespace crrc
{
  namespace util
  {
    static auto instTypeComparator = []( const QVariant& instType1, const QVariant& instType2 ) -> bool
    {
      const auto ptr1 = model::InstitutionType::from( instType1 );
      const auto ptr2 = model::InstitutionType::from( instType2 );
      return *ptr1 < *ptr2;
    };
  }
}

using crrc::InstitutionTypes;

void InstitutionTypes::index( Cutelyst::Context* c ) const
{
  auto list = dao::InstitutionTypeDAO().retrieveAll();
  qSort( list.begin(), list.end(), util::instTypeComparator );

  QJsonArray arr;
  for ( const auto& instType : list ) arr << toJson( *model::InstitutionType::from( instType ) );
  dao::sendJson( c, arr );
}

void InstitutionTypes::base( Cutelyst::Context* c ) const
{
  c->response()->setHeader( "Cache-Control", "no-cache, no-store, must-revalidate" );
}

void InstitutionTypes::object( Cutelyst::Context* c, const QString& id ) const
{
  c->setStash( "object", dao::InstitutionTypeDAO().retrieve( id.toUInt() ) );
}

void InstitutionTypes::data( Cutelyst::Context* c ) const
{
  const auto& var = c->stash( "object" );
  const auto ptr = model::InstitutionType::from( var );
  dao::sendJson( c, ptr ? toJson( *ptr ) : QJsonObject() );
}