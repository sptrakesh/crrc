#include "Contacts.h"
#include "dao/ContactDAO.h"
#include "dao/InstitutionDAO.h"
#include "dao/UserDAO.h"
#include "dao/RoleDAO.h"
#include "dao/functions.h"
#include "model/Contact.h"

#include <QtCore/QDebug>
#include <QtCore/QJsonArray>
#include <QtCore/QJsonDocument>

namespace crrc
{
  namespace util
  {
    static auto contactComparator = []( const QVariant& deg1, const QVariant& deg2 ) -> bool
    {
      const auto ptr1 = model::Contact::from( deg1 );
      const auto ptr2 = model::Contact::from( deg2 );
      return *ptr1 < *ptr2;
    };

    QJsonArray contactsToArray( const QVariantList& list )
    {
      auto arr = QJsonArray();
      for ( const auto cvar : list )
      {
        const auto ptr = model::Contact::from( cvar );
        arr << toJson( *ptr );
      }

      return arr;
    }
  }
}

using crrc::Contacts;

void Contacts::index( Cutelyst::Context* c ) const
{
  dao::ContactDAO dao;
  auto list = dao::isGlobalAdmin( c ) ? dao.retrieveAll() :
    dao.retrieveByInstitution( dao::institutionId( c ) );
  qSort( list.begin(), list.end(), util::contactComparator );


  const auto& arr = util::contactsToArray( list );
  if ( "POST" == c->request()->method() )
  {
    dao::sendJson( c, arr );
    return;
  }

  c->stash( {
    { "contacts", QJsonDocument( arr ).toJson() },
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
  const auto* cptr = model::Contact::from( obj );

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

  const dao::ContactDAO dao{};

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

  if ( "PUT" == c->request()->method() )
  {
    const auto contact = dao.retrieve( id.toUInt() );
    dao::sendJson( c, toJson( *model::Contact::from( contact ) ) );
    return;
  }

  c->stash( {
    { "template", "contacts/view.html" },
    { "object", dao.retrieve( id.toUInt() ) }
  } );
}

void Contacts::view( Cutelyst::Context* c ) const
{
  c->setStash( "template", "contacts/view.html" );
}

void Contacts::data( Cutelyst::Context* c ) const
{
  const auto& var = c->stash( "object" );
  const auto ptr = model::Contact::from( var );
  dao::sendJson( c, ptr ? toJson( *ptr ) : QJsonObject() );
}

void Contacts::search( Cutelyst::Context* c ) const
{
  const auto text = c->request()->param( "text", "" );

  if ( text.isEmpty() )
  {
    c->response()->redirect( "/contacts" );
    return;
  }

  const dao::ContactDAO dao{};
  const auto& list = dao.search( c );

  if ( "PUT" == c->request()->method() )
  {
    const auto& arr = util::contactsToArray( list );
    dao::sendJson( c, arr );
    return;
  }

  c->stash( {
    { "contacts", list },
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
  uint32_t count = 0;

  if ( ! id.isEmpty() )
  {
    const dao::ContactDAO dao{};
    count = dao.remove( id.toUInt() );
  }

  if ( "PUT" == c->request()->method() )
  {
    QJsonObject obj;
    obj.insert( "status", count > 0 );
    obj.insert( "count", static_cast<int>( count ) );
    obj.insert( "id", id.toInt() );
    dao::sendJson( c, obj );
    return;
  }

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
