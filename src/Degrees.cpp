#include "Degrees.h"
#include "dao/DegreeDAO.h"
#include "dao/functions.h"
#include "model/Degree.h"

#include <QtCore/QDebug>

namespace crrc
{
  namespace util
  {
    static auto degreeComparator = []( const QVariant& deg1, const QVariant& deg2 ) -> bool
    {
      const auto ptr1 = model::Degree::from( deg1 );
      const auto ptr2 = model::Degree::from( deg2 );
      return *ptr1 < *ptr2;
    };
  }
}

using crrc::Degrees;

void Degrees::index( Cutelyst::Context* c ) const
{
  auto list = dao::DegreeDAO().retrieveAll();
  qSort( list.begin(), list.end(), util::degreeComparator );

  c->stash( {
    { "degrees", list },
    { "template", "degrees/index.html" }
  } );
}

void Degrees::base( Cutelyst::Context* c ) const
{
  c->response()->setHeader( "Cache-Control", "no-cache, no-store, must-revalidate" );
}


void Degrees::object( Cutelyst::Context* c, const QString& id ) const
{
  c->setStash( "object", dao::DegreeDAO().retrieve( id.toUInt() ) );
}

void Degrees::edit( Cutelyst::Context* c ) const
{
  if ( ! crrc::dao::isGlobalAdmin( c ) )
  {
    QJsonObject obj;
    obj.insert( "status", false );
    obj.insert( "message", "Unauthorized" );
    dao::sendJson( c, obj );
    return;
  }

  auto id = c->request()->param( "id", "" );
  const auto title = c->request()->param( "title", "" );
  QJsonObject obj;

  if ( title.isEmpty() || title.contains( "'" ) || title.contains( '"' ) )
  {
    obj.insert( "status", false );
    obj.insert( "id", id );
    dao::sendJson( c, obj );
    return;
  }

  dao::DegreeDAO dao;
  if ( id.isEmpty() )
  {
    qDebug() << "Inserting new degree";
    const auto cid = dao.insert( c );
    obj.insert( "id", static_cast<int>( cid ) );
    obj.insert( "status", true );
  }
  else
  {
    qDebug() << "Updating degree";
    dao.update( c );
    obj.insert( "status", true );
    obj.insert( "id", id );
  }

  dao::sendJson( c, obj );
}

void Degrees::view( Cutelyst::Context* c ) const
{
  c->setStash( "template", "degrees/view.html" );
}

void Degrees::data( Cutelyst::Context* c ) const
{
  const auto& var = c->stash( "object" );
  const auto ptr = model::Degree::from( var );
  dao::sendJson( c, toJson( *ptr ) );
}

void Degrees::search( Cutelyst::Context* c ) const
{
  const auto text = c->request()->param( "text", "" );

  if ( text.isEmpty() )
  {
    c->response()->redirect( "/degrees" );
    return;
  }

  dao::DegreeDAO dao;
  auto list = dao.search( c );
  qSort( list.begin(), list.end(), util::degreeComparator );

  c->stash( {
    { "degrees", list },
    { "searchText", text },
    { "template", "degrees/index.html" }
  } );
}

void Degrees::remove( Cutelyst::Context* c ) const
{
  auto id = c->request()->param( "id", "" );
  QJsonObject obj;

  if ( id.isEmpty() || ! dao::isGlobalAdmin( c ) )
  {
    obj.insert( "status", false );
    dao::sendJson( c, obj );
    return;
  }

  auto const result = dao::DegreeDAO().remove( id.toUInt() );
  obj.insert( "count", static_cast<int>( result ) );
  obj.insert( "id", id.toInt() );
  obj.insert( "status", true );
  dao::sendJson( c, obj );
}
