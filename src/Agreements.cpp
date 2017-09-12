#include "Agreements.h"
#include "dao/AgreementDAO.h"
#include "dao/InstitutionAgreementDAO.h"
#include <QtCore/QStringBuilder>
#include <QtCore/QtDebug>
#include <Cutelyst/Plugins/Authentication/authentication.h>
#include <Cutelyst/Plugins/Authentication/authenticationuser.h>

using crrc::Agreements;

void Agreements::index( Cutelyst::Context* c ) const
{
  using Cutelyst::Authentication;
  using Cutelyst::AuthenticationUser;

  const AuthenticationUser& user = Authentication::user( c );
  qDebug() << "Authenticated user: " << user << ", id: " << user.id();

  dao::AgreementDAO dao;
  c->stash( {
    { "agreements", dao.retrieveAll() },
    { "template", "agreements/index.html" }
  } );
}

void Agreements::base( Cutelyst::Context* c ) const
{
  c->response()->setHeader( "Cache-Control", "no-cache" );
}


void Agreements::object( Cutelyst::Context* c, const QString& id ) const
{
  dao::AgreementDAO dao;
  c->setStash( "object", dao.retrieve( id ) );
}

void Agreements::create( Cutelyst::Context* c ) const
{
  c->setStash( "template", "agreements/form.html" );
}

void Agreements::edit( Cutelyst::Context* c ) const
{
  auto id = c->request()->param( "id", "" );

  dao::AgreementDAO dao;
  if ( id.isEmpty() )
  {
    auto cid = dao.insert( c );
    id = QString::number( cid );
  }
  else dao.update( c );

  c->stash( {
    { "template", "agreements/view.html" },
    { "object", dao.retrieve( id ) }
  } );
}

void Agreements::view( Cutelyst::Context* c ) const
{
  const auto& object = c->stash( "object" ).toHash();

  dao::InstitutionAgreementDAO dao;
  const auto& list = dao.retrieve( object.value( "agreement_id" ).toString() );
  c->stash( {
    { "template", "agreements/view.html" },
    { "relations", list }
  } );
}

void Agreements::display( Cutelyst::Context* c ) const
{
  const auto& object = c->stash( "object" ).toHash();
  dao::AgreementDAO dao;
  const auto bytes = dao.contents( c, object.value( "agreement_id" ).toString() );
  c->response()->setContentType( object.value( "mimetype" ).toString() );
  c->response()->setContentLength( object.value( "filesize" ).toUInt() );
  QString disposition = "inline; filename=\"" % object.value( "filename" ).toString() % "\"";
  c->response()->setHeader( "Content-Disposition", disposition );
  c->response()->setBody( bytes );
}


void Agreements::search( Cutelyst::Context* c ) const
{
  const auto text = c->request()->param( "text", "" );

  if ( text.isEmpty() )
  {
    c->response()->redirect( "/agreements" );
    return;
  }

  dao::AgreementDAO dao;
  c->stash( {
    { "agreements", dao.search( c ) },
    { "searchText", text },
    { "template", "agreements/index.html" }
  } );
}

void Agreements::remove( Cutelyst::Context* c )
{
  auto id = c->request()->param( "id", "" );
  QString statusMsg;

  if ( ! id.isEmpty() )
  {
    dao::AgreementDAO dao;
    statusMsg = dao.remove( id.toUInt() );
  }
  else statusMsg = "No agreement identifier specified!";

  c->stash()["status_msg"] = statusMsg;
  c->response()->redirect( "/agreements" );
}