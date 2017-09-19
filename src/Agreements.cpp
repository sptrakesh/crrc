#include "Agreements.h"
#include "AttachmentController.h"
#include "dao/AgreementDAO.h"
#include "dao/InstitutionDAO.h"
#include "dao/InstitutionAgreementDAO.h"

#include <QtCore/QStringBuilder>
#include <QtCore/QtDebug>
#include <QtCore/QJsonDocument>
#include <QtCore/QJsonObject>

using crrc::Agreements;

void Agreements::index( Cutelyst::Context* c ) const
{
  dao::AgreementDAO dao;
  c->stash( {
    { "agreements", dao.retrieveAll() },
    { "template", "agreements/index.html" }
  } );
}

void Agreements::base( Cutelyst::Context* c ) const
{
  c->response()->setHeader( "Cache-Control", "no-cache, no-store, must-revalidate" );
}


void Agreements::object( Cutelyst::Context* c, const QString& id ) const
{
  AttachmentController<dao::AgreementDAO>().object( c, id );
}

void Agreements::create( Cutelyst::Context* c ) const
{
  c->stash( {
    { "institutions", dao::InstitutionDAO().retrieveAll( dao::InstitutionDAO::Mode::Partial ) },
    { "template", "agreements/form.html" }
  } );
}

void Agreements::edit( Cutelyst::Context* c ) const
{
  const auto& obj = AttachmentController<dao::AgreementDAO>().edit( c );
  QJsonObject json;
  json.insert( "id", obj.value( "id" ).toInt() );
  const auto& bytes = QJsonDocument( json ).toJson();

  c->response()->setContentType( "application/json" );
  c->response()->setContentLength( bytes.size() );
  c->response()->setBody( bytes );
}

void Agreements::institutions( Cutelyst::Context* c ) const
{
  const auto dao = dao::AgreementDAO();
  const auto& result = dao.saveProgram( c );
  if ( !result.isEmpty() && result != "0" ) c->setStash( "error", result );

  c->stash( {
    { "template", "agreements/view.html" },
    { "object", dao.retrieve( c->request()->param( "institution_id" ) ) }
  } );
}

void Agreements::view( Cutelyst::Context* c ) const
{
  const auto& object = c->stash( "object" ).toHash();

  const auto& list = dao::InstitutionAgreementDAO().retrieve( object.value( "id" ).toString() );
  c->stash( {
    { "template", "agreements/view.html" },
    { "relations", list }
  } );
}

void Agreements::display( Cutelyst::Context* c ) const
{
  AttachmentController<dao::AgreementDAO>().display( c );
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
  AttachmentController<dao::AgreementDAO>().remove( c, "/agreements" );
}