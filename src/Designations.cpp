#include "Designations.h"
#include "dao/DesignationDAO.h"
#include <dao/functions.h>
#include "model/Designation.h"

#include <QtCore/QJsonArray>

namespace crrc
{
  namespace util
  {
    static auto designationComparator = []( const QVariant& deg1, const QVariant& deg2 ) -> bool
    {
      const auto ptr1 = model::Designation::from( deg1 );
      const auto ptr2 = model::Designation::from( deg2 );
      return *ptr1 < *ptr2;
    };
  }
}

using crrc::Designations;

void Designations::index( Cutelyst::Context* c )
{
  auto list = dao::DesignationDAO().retrieveAll();
  qSort( list.begin(), list.end(), util::designationComparator );

  QJsonArray arr;
  for ( const auto& designation : list ) arr << toJson( *model::Designation::from( designation ) );
  dao::sendJson( c, arr );
}

void Designations::base( Cutelyst::Context* c ) const
{
  c->response()->setHeader( "Cache-Control", "no-cache, no-store, must-revalidate" );
}

void Designations::object( Cutelyst::Context* c, const QString& id ) const
{
  c->setStash( "object", dao::DesignationDAO().retrieve( id.toUInt() ) );
}

void Designations::data( Cutelyst::Context* c ) const
{
  const auto& object = c->stash( "object" );
  const auto ptr = model::Designation::from( object );
  dao::sendJson( c, ptr ? toJson( *ptr ) : QJsonObject() );
}

void Designations::save( Cutelyst::Context* c ) const
{
  QJsonObject json;
  const auto dao = dao::DesignationDAO{};

  if ( ! dao::isGlobalAdmin( c ) )
  {
    json.insert( "message", "Unauthorised to edit designations" );
    json.insert( "status", false );
    dao::sendJson( c, json );
    return;
  }

  const auto id = c->request()->param( "id", "" );
  uint32_t did = id.toUInt();

  if ( id.isEmpty() )
  {
    did = dao.insert( c );
    json.insert( "id", static_cast<int>( did ) );
    json.insert( "count", 1 );
  }
  else
  {
    const auto count = dao.update( c );
    json.insert( "id", static_cast<int>( did ) );
    json.insert( "count", static_cast<int>( count ) );
    json.insert( "status", count > 0 );
  }

  dao::sendJson( c, json );
}

void Designations::remove( Cutelyst::Context* c ) const
{
  auto id = c->request()->param( "id", "" );
  QJsonObject obj;

  if ( id.isEmpty() || ! dao::isGlobalAdmin( c ) )
  {
    obj.insert( "status", false );
    dao::sendJson( c, obj );
    return;
  }

  auto const result = dao::DesignationDAO().remove( id.toUInt() );
  obj.insert( "count", static_cast<int>( result ) );
  obj.insert( "id", id.toInt() );
  obj.insert( "status", true );
  dao::sendJson( c, obj );
}
