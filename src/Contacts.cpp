#include "Contacts.h"
#include "dao/ContactDAO.h"
#include "dao/InstitutionDAO.h"
#include "dao/UserDAO.h"
#include "dao/RoleDAO.h"
#include "dao/functions.h"
#include "model/Contact.h"

#include <QtCore/QDebug>

using crrc::Contacts;

void Contacts::index( Cutelyst::Context* c ) const
{
  dao::ContactDAO dao;
  const auto& list = dao::isGlobalAdmin( c ) ? dao.retrieveAll() :
    dao.retrieveByInstitution( dao::institutionId( c ) );

  c->stash( {
    { "contacts", list },
    { "template", "contacts/index.html" }
  } );
}

void Contacts::base( Cutelyst::Context* c ) const
{
  c->response()->setHeader( "Cache-Control", "no-cache, no-store, must-revalidate" );
}


void Contacts::object( Cutelyst::Context* c, const QString& id ) const
{
  dao::ContactDAO dao;
  const auto obj = dao.retrieve( id.toUInt() );
  const auto* cptr = qvariant_cast<model::Contact*>( obj );

  switch ( dao::roleId( c ) )
  {
  case 1:
    c->setStash( "object", obj );
    break;
  default:
    auto iid = cptr->getInstitutionId();
    if ( !iid ) iid = -1;
    c->setStash( "object",
      ( iid == dao::institutionId( c ) ) ? obj : QVariantHash() );
  }
}

void Contacts::create( Cutelyst::Context* c ) const
{
  if ( dao::isGlobalAdmin( c ) )
  {
    c->setStash( "institutions", dao::InstitutionDAO().retrieveAll() );
  }
  else
  {
    QVariantList list;
    list << c->stash( "userInstitution" ).toHash();
    c->setStash( "institutions", list );
  }

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

  const dao::ContactDAO dao;

  if ( ! dao::isGlobalAdmin( c ) )
  {
    auto iid = c->request()->param( "institution" ).toInt();
    if ( !iid ) iid = -1;
    if ( static_cast<int32_t>( iid ) != dao::institutionId( c ) )
    {
      c->stash()["error_msg"] = "Institution does not match current user!";
      return;
    }
  }

  if ( id.isEmpty() )
  {
    const auto cid = dao.insert( c );
    id = QString::number( cid );
  }
  else dao.update( c );

  c->stash( {
    { "template", "contacts/view.html" },
    { "object", dao.retrieve( id.toUInt() ) }
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
  if ( ! dao::isGlobalAdmin( c ) )
  {
    c->stash()["status_msg"] = "Unauthorized to edit contacts";
    c->response()->redirect( "/contacts" );
    return;
  }

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
  const auto status = dao::isGlobalAdmin( c ) ?
    dao::UserDAO().isUsernameAvailable( c->request()->param( "username" ) ) :
    false;
  const QString string = status ? "true" : "false";
  c->response()->setContentType( "text/plain" );
  c->response()->setContentLength( string.size() );
  c->response()->setBody( string );
}
