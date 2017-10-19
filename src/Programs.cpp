#include "Programs.h"
#include "dao/ProgramDAO.h"
#include "dao/InstitutionDAO.h"
#include "dao/DegreeDAO.h"
#include "dao/DesignationDAO.h"
#include "dao/functions.h"
#include "model/Program.h"

#include <QtCore/QDebug>

namespace crrc
{
  namespace util
  {
    static auto programComparator = []( const QVariant& prog1, const QVariant& prog2 ) -> bool
    {
      const auto ptr1 = model::Program::from( prog1 );
      const auto ptr2 = model::Program::from( prog2 );
      return *ptr1 < *ptr2;
    };
  }
}

using crrc::Programs;

void Programs::index( Cutelyst::Context* c ) const
{
  auto list = dao::isGlobalAdmin( c ) ?
    dao::ProgramDAO().retrieveAll() :
    dao::ProgramDAO().retrieveByInstitution( dao::institutionId( c ) );
  qSort( list.begin(), list.end(), util::programComparator );

  if ( "POST" == c->request()->method() )
  {
    const auto& json = toArray( list );
    dao::sendJson( c, json );
    return;
  }

  c->stash( {
    { "programs", list },
    { "template", "programs/index.html" }
  } );
}

void Programs::base( Cutelyst::Context* c ) const
{
  c->response()->setHeader( "Cache-Control", "no-cache, no-store, must-revalidate" );
}


void Programs::object( Cutelyst::Context* c, const QString& id ) const
{
  const auto& obj = dao::ProgramDAO().retrieve( id.toUInt() );
  const auto ptr = model::Program::from( obj );

  if ( !ptr )
  {
    c->setStash( "object", obj );
    return;
  }

  int32_t iid = dao::institutionId( c );
  if ( !iid ) iid = -1;

  if ( dao::isGlobalAdmin( c ) || iid == ptr->getInstitutionId() )
  {
    c->setStash( "object", obj );
  }
}

void Programs::create( Cutelyst::Context* c ) const
{
  const auto& list = dao::isGlobalAdmin( c ) ?
    dao::InstitutionDAO().retrieveAll() :
    QVariantList( { dao::InstitutionDAO().retrieve( dao::institutionId( c ) ) } );

  c->stash( {
    { "template", "programs/form.html" },
    { "institutions", list },
    { "degrees", dao::DegreeDAO().retrieveAll() },
    { "designations", dao::DesignationDAO().retrieveAll() }
  } );
}

void Programs::data( Cutelyst::Context* c ) const
{
  const auto& var = c->stash( "object" );
  const auto ptr = model::Program::from( var );
  dao::sendJson( c, ptr ? toJson( *ptr ) : QJsonObject() );
}

void Programs::edit( Cutelyst::Context* c ) const
{
  auto id = c->request()->param( "id", "" );
  const auto title = c->request()->param( "title", "" );

  QJsonObject json;

  if ( title.isEmpty() )
  {
    if ( "PUT" == c->request()->method() )
    {
      json.insert( "status", false );
      json.insert( "message", "Program title required!" );
      dao::sendJson( c, json );
      return;
    }

    c->stash()["error_msg"] = "Program title required!";
    return;
  }

  if ( !checkInstitution( c ) )
  {
    if ( "PUT" == c->request()->method() )
    {
      json.insert( "status", false );
      json.insert( "message", "Not authorized to edit this institution" );
      dao::sendJson( c, json );
      return;
    }

    return;
  }

  dao::ProgramDAO dao;
  auto pid = id.toUInt();
  auto count = 0;

  if ( id.isEmpty() ) pid = dao.insert( c );
  else count = dao.update( c );

  if ( "PUT" == c->request()->method() )
  {
    json.insert( "id", static_cast<int>( pid ) );
    json.insert( "count", count );
    json.insert( "status", id > 0 && count > 0 );
    return;
  }

  c->response()->redirect( "/programs" );
}

void Programs::search( Cutelyst::Context* c ) const
{
  const auto text = c->request()->param( "text", "" );

  if ( text.isEmpty() )
  {
    c->response()->redirect( "/programs" );
    return;
  }

  auto list = dao::ProgramDAO().search( c );
  qSort( list.begin(), list.end(), util::programComparator );

  if ( "PUT" == c->request()->method() )
  {
    const auto& json = toArray( list );
    dao::sendJson( c, json );
    return;
  }

  c->stash( {
    { "programs", list },
    { "searchText", text },
    { "template", "programs/index.html" }
  } );
}

void Programs::remove( Cutelyst::Context* c )
{
  auto id = c->request()->param( "id", "" );
  QJsonObject json;
  json.insert( "id", id );

  if ( ! id.isEmpty() )
  {
    const auto count = dao::ProgramDAO().remove( id.toUInt() );
    json.insert( "count", static_cast<int>( count ) );
    json.insert( "status", count ? true : false );
  }
  else
  {
    json.insert( "message", "No program identifier specified!" );
    json.insert( "status", false );
  }

  dao::sendJson( c, json );
}

bool Programs::checkInstitution( Cutelyst::Context* context ) const
{
  if ( dao::isGlobalAdmin( context ) ) return true;

  auto iid = context->request()->param( "institution" ).toInt();
  if ( !iid ) iid = -1;
  if ( static_cast<int32_t>( iid ) != dao::institutionId( context ) )
  {
    context->stash()["error_msg"] = "Institution does not match current user!";
    return false;
  }

  return true;
}

QJsonArray Programs::toArray( const QVariantList& list ) const
{
  auto arr = QJsonArray{};
  for ( const auto& item : list ) arr << toJson( *model::Program::from( item ) );
  return arr;
}
