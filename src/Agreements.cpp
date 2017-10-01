#include "Agreements.h"
#include "AttachmentController.h"
#include "dao/AgreementDAO.h"
#include "dao/InstitutionDAO.h"
#include "dao/InstitutionAgreementDAO.h"
#include "dao/ProgramDAO.h"
#include "dao/functions.h"

#include <QtCore/QtDebug>
#include <QtCore/QJsonArray>
#include <QtCore/QJsonObject>

using crrc::Agreements;

void Agreements::index( Cutelyst::Context* c ) const
{
  const auto& list = dao::isGlobalAdmin( c ) ?
    dao::AgreementDAO().retrieveAll() :
    dao::AgreementDAO().retrieveByInstitution( dao::institutionId( c ) );
  c->stash( {
    { "agreements", list },
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
    { "institutions", dao::InstitutionDAO().retrieveAll() },
    { "template", "agreements/form.html" }
  } );
}

void Agreements::edit( Cutelyst::Context* c ) const
{
  const auto& obj = AttachmentController<dao::AgreementDAO>().edit( c );
  QJsonObject json;
  json.insert( "id", obj.value( "id" ).toInt() );
  const auto& bytes = QJsonDocument( json ).toJson();
  dao::sendJson( c, json );
}

void Agreements::institutions( Cutelyst::Context* c ) const
{
  const auto dao = dao::AgreementDAO();
  const auto& result = dao.saveProgram( c );
  if ( !result.isEmpty() && result != "0" ) c->setStash( "error", result );

  c->setStash( "object", dao.retrieve( c->request()->param( "agreement_id" ) ) );
  view( c );
}

void Agreements::view( Cutelyst::Context* c ) const
{
  const auto& object = c->stash( "object" ).toHash();
  const auto& id = object.value( "id" );

  const auto& relations = dao::InstitutionAgreementDAO().retrieve( id.toString() );

  dao::ProgramDAO dao;
  const auto& transferInstitution = object.value( "transferInstitution" );
  const auto& transfereeInstitution = object.value( "transfereeInstitution" );

  const auto func = [](const QVariantHash& hash, QJsonArray& array)
  {
    QJsonObject json;
    json.insert( "program_id", hash.value( "program_id" ).toInt() );
    json.insert( "title", hash.value( "title" ).toString() );
    array << json;
  };

  const auto& trp = transferInstitution.isNull() ?  QVariantList() :
    dao.retrieveByInstitution( transferInstitution.toHash().value( "institution_id" ).toUInt(), dao::ProgramDAO::Mode::Partial );
  QJsonArray transferPrograms;
  for ( const auto& value : trp ) func( value.toHash(), transferPrograms );

  const auto& treep = transfereeInstitution.isNull() ?  QVariantList() :
    dao.retrieveByInstitution( transfereeInstitution.toHash().value( "institution_id" ).toUInt(), dao::ProgramDAO::Mode::Partial );
  QJsonArray transfereePrograms;
  for ( const auto& value : treep ) func( value.toHash(), transfereePrograms );

  c->stash( {
    { "template", "agreements/view.html" },
    { "transferPrograms", QJsonDocument( transferPrograms ).toJson() },
    { "transfereePrograms", QJsonDocument( transfereePrograms ).toJson() },
    { "relations", relations }
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
  const auto& response = c->request()->param( "response" );
  if ( response == "html" )
  {
    AttachmentController<dao::AgreementDAO>().remove( c, "/agreements" );
    return;
  }

  AttachmentController<dao::AgreementDAO>().remove( c, "" );
  const auto& msg = c->stash( "status_msg" ).toString();

  QJsonObject json;
  json.insert( "id", c->request()->param( "id" ) );
  json.insert( "status", ( msg == "1" ) );
  json.insert( "message", msg );
  dao::sendJson( c, json );
}