#include "Contacts.h"
#include "dao/ContactDAO.h"
#include "dao/UserDAO.h"
#include "dao/RoleDAO.h"

using crrc::Contacts;

void Contacts::index( Cutelyst::Context* c ) const
{
  dao::ContactDAO dao;
  c->setStash( "contacts", dao.retrieveAll() );
  c->setStash( "template", "contacts/index.html" );
}

void Contacts::base( Cutelyst::Context* c ) const
{
  c->response()->setHeader( "Cache-Control", "no-cache" );
}


void Contacts::object( Cutelyst::Context* c, const QString& id ) const
{
  dao::ContactDAO dao;
  const auto obj = dao.retrieve( id );
  c->setStash( "object", obj );
}

void Contacts::create( Cutelyst::Context* c ) const
{
  c->stash( {
    { "template", "contacts/form.html" },
    { "roles", dao::RoleDAO().retrieveAll() }
  } );
}

void Contacts::edit( Cutelyst::Context* c ) const
{
  auto id = c->request()->param( "id", "" );
  const auto name = c->request()->param( "name", "" );
  if ( name.isEmpty() )
  {
    c->stash()["error_msg"] = "Contact name required!";
    return;
  }

  dao::ContactDAO dao;
  if ( id.isEmpty() )
  {
    auto cid = dao.insert( c );
    id = QString::number( cid );
  }
  else dao.update( c );

  c->stash( {
    { "template", "contacts/view.html" },
    { "object", dao.retrieve( id ) }
  } );
}

void Contacts::view( Cutelyst::Context* c ) const
{
  c->setStash( "template", "contacts/view.html" );
}

void Contacts::search( Cutelyst::Context* c ) const
{
  const auto text = c->request()->param( "text", "" );

  if ( text.isEmpty() )
  {
    c->response()->redirect( "/contacts" );
    return;
  }

  dao::ContactDAO dao;
  c->stash( {
    { "contacts", dao.search( c ) },
    { "searchText", text },
    { "template", "contacts/index.html" }
  } );
}

void Contacts::remove( Cutelyst::Context* c )
{
  auto id = c->request()->param( "id", "" );
  QString statusMsg;

  if ( ! id.isEmpty() )
  {
    dao::ContactDAO dao;
    statusMsg = dao.remove( id.toUInt() );
  }
  else statusMsg = "No contact identifier specified!";

  c->stash()["status_msg"] = statusMsg;
  c->response()->redirect( "/contacts" );
}

void Contacts::isUsernameAvailable( Cutelyst::Context* c )
{
  const auto status = dao::UserDAO().isUsernameAvailable( c->request()->param( "username" ) );
  const QString string = status ? "true" : "false";
  c->response()->setContentType( "text/plain" );
  c->response()->setContentLength( string.size() );
  c->response()->setBody( string );
}
