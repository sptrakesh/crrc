#include "Programs.h"
#include "dao/ProgramDAO.h"
#include "dao/InstitutionDAO.h"
#include "dao/DegreeDAO.h"
#include "dao/DesignationDAO.h"
#include "dao/functions.h"

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
  const auto& obj = dao::ProgramDAO().retrieve( id );

  int32_t iid = dao::institutionId( c );
  if ( !iid ) iid = -1;

  const auto& inst = obj.value( "institution" );
  const auto iptr = qvariant_cast<model::Institution*>( inst );
  const auto piid = iptr ? iptr->getInstitutionId() : 0;

  if ( dao::isGlobalAdmin( c ) || iid == piid )
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
  if ( id.isEmpty() )
  {
    const auto cid = dao.insert( c );
    id = QString::number( cid );
  }
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
  auto id = c->request()->param( "program_id", "" );
  QJsonObject json;
  json.insert( "id", id );

  if ( ! id.isEmpty() )
  {
    json.insert( "message", dao::ProgramDAO().remove( id.toUInt() ) );
    json.insert( "status", true );
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
