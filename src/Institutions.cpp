#include "Institutions.h"
#include "dao/ContactDAO.h"
#include "dao/InstitutionDAO.h"
  
using crrc::Institutions;

void Institutions::index( Cutelyst::Context* c ) const
{
  dao::InstitutionDAO dao;
  c->setStash( "institutions", dao.retrieveAll() );
  c->setStash( "template", "institutions/index.html" );
}

void Institutions::base( Cutelyst::Context* c ) const
{
  c->response()->setHeader( "Cache-Control", "no-cache" );
}


void Institutions::object( Cutelyst::Context* c, const QString& id ) const
{
  dao::InstitutionDAO dao;
  c->setStash( "object", dao.retrieve( id ) );
}

void Institutions::create( Cutelyst::Context* c ) const
{
  const auto id = c->request()->param( "id", "" );
  if ( !id.isEmpty() ) object( c, id );
  c->setStash( "contacts", dao::ContactDAO().retrieveAll( dao::ContactDAO::Mode::Partial ) );
  c->setStash( "template", "institutions/form.html" );
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
    auto cid = dao.insert( c );
    id = QString::number( cid );
  }
  else dao.update( c );

  c->stash( {
    { "template", "institutions/view.html" },
    { "object", dao.retrieve( id ) }
  } );
}

void Institutions::view( Cutelyst::Context* c ) const
{
  c->setStash( "template", "institutions/view.html" );
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