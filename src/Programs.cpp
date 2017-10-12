#include "Programs.h"
#include "dao/ProgramDAO.h"
#include "dao/InstitutionDAO.h"
#include "dao/DegreeDAO.h"
#include "dao/DesignationDAO.h"
#include "dao/functions.h"
#include "model/Program.h"

#include <QtCore/QDebug>

using crrc::Programs;

void Programs::index( Cutelyst::Context* c ) const
{
  const auto& list = dao::isGlobalAdmin( c ) ? 
    dao::ProgramDAO().retrieveAll() :
    dao::ProgramDAO().retrieveByInstitution( dao::institutionId( c ) );
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
  dao::sendJson( c, toJson( *ptr ) );
}

void Programs::edit( Cutelyst::Context* c ) const
{
  auto id = c->request()->param( "id", "" );
  const auto title = c->request()->param( "title", "" );
  if ( title.isEmpty() )
  {
    c->stash()["error_msg"] = "Program title required!";
    return;
  }

  if ( !checkInstitution( c ) ) return;

  dao::ProgramDAO dao;
  if ( id.isEmpty() ) dao.insert( c );
  else dao.update( c );

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

  dao::ProgramDAO dao;
  c->stash( {
    { "programs", dao.search( c ) },
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
