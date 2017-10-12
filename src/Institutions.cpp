#include "Institutions.h"
#include "dao/InstitutionDAO.h"
#include "dao/DesignationDAO.h"
#include "dao/InstitutionDesignationDAO.h"
#include "dao/functions.h"
#include "model/Institution.h"
#include "model/InstitutionDesignation.h"

#include <QtCore/QJsonArray>
#include "model/Designation.h"

using crrc::Institutions;

void Institutions::index( Cutelyst::Context* c ) const
{
  dao::InstitutionDAO dao;
  c->stash( {
    { "institutions", dao.retrieveAll() },
    { "template", "institutions/index.html" }
  } );
}

void Institutions::base( Cutelyst::Context* c ) const
{
  c->response()->setHeader( "Cache-Control", "no-cache, no-store, must-revalidate" );
}


void Institutions::object( Cutelyst::Context* c, const QString& id ) const
{
  c->setStash( "object", dao::InstitutionDAO().retrieve( id.toUInt() ) );
}

void Institutions::create( Cutelyst::Context* c ) const
{
  const auto id = c->request()->param( "id", "" );
  if ( !id.isEmpty() ) object( c, id );

  c->stash( {
    { "template", "institutions/form.html" },
    { "designations", dao::DesignationDAO().retrieveByType( "CAE" ) }
  } );
}

void Institutions::edit( Cutelyst::Context* c ) const
{
  auto id = c->request()->param( "id", "" );
  const auto name = c->request()->param( "name", "" );
  if ( name.isEmpty() )
  {
    c->stash()["error_msg"] = "Institution name required!";
    return;
  }

  dao::InstitutionDAO dao;
  if ( id.isEmpty() )
  {
    const auto cid = dao.insert( c );
    id = QString::number( cid );
  }
  else dao.update( c );

  c->stash( {
    { "template", "institutions/view.html" },
    { "object", dao.retrieve( id.toUInt() ) }
  } );
}

void Institutions::view( Cutelyst::Context* c ) const
{
  c->setStash( "template", "institutions/view.html" );
}

void Institutions::data( Cutelyst::Context* c ) const
{
  const auto& var = c->stash( "object" );
  const auto ptr = model::Institution::from( var );
  auto json = toJson( *ptr );

  const auto& list = dao::InstitutionDesignationDAO().retrieve( c, ptr->getId() );
  QJsonArray arr;
  for ( const auto& id : list )
  {
    QJsonObject obj;
    const auto idptr = model::InstitutionDesignation::from( id );
    const auto dptr = model::Designation::from( idptr->getDesignation() );
    obj.insert( "designation", toJson( *dptr ) );
    obj.insert( "expiration", static_cast<int>( idptr->getExpiration() ) );
    arr << obj;
  }

  json.insert( "designations", arr );
  dao::sendJson( c, json );
}

void Institutions::search( Cutelyst::Context* c ) const
{
  const auto text = c->request()->param( "text", "" );
  if ( text.isEmpty() )
  {
    c->response()->redirect( "/institutions" );
    return;
  }

  dao::InstitutionDAO dao;
  c->stash( {
    { "institutions", dao.search( c ) },
    { "searchText", text },
    { "template", "institutions/index.html" }
  } );
}

void Institutions::remove( Cutelyst::Context* c )
{
  auto id = c->request()->param( "id", "" );
  QString statusMsg;

  if ( ! id.isEmpty() )
  {
    dao::InstitutionDAO dao;
    statusMsg = dao.remove( id.toUInt() );
  }
  else statusMsg = "No institution identifier specified!";

  c->stash()["status_msg"] = statusMsg;
  c->response()->redirect( "/institutions" );
}

void Institutions::checkUnique( Cutelyst::Context* c )
{
  const auto name = c->request()->param( "name" );
  const auto city = c->request()->param( "city" );
  const auto status = dao::InstitutionDAO().isUnique( name, city );
  const QString string = status ? "true" : "false";

  c->response()->setContentType( "text/plain" );
  c->response()->setContentLength( string.size() );
  c->response()->setBody( string );
}
