#include "Agreements.h"
#include "AttachmentController.h"
#include "dao/AgreementDAO.h"
#include "dao/InstitutionDAO.h"
#include "dao/InstitutionAgreementDAO.h"
#include "dao/ProgramDAO.h"
#include "dao/functions.h"
#include "model/Agreement.h"
#include "model/Program.h"

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
  const auto ptr = model::Agreement::from( obj );
  QJsonObject json;
  if ( ptr ) json.insert( "id", static_cast<int>( ptr->getId() ) );
  else json.insert( "id", 0 );
  dao::sendJson( c, json );
}

void Agreements::institutions( Cutelyst::Context* c ) const
{
  const auto dao = dao::AgreementDAO();
  const auto result = dao.saveProgram( c );
  if ( !result ) c->setStash( "error", result );

  const auto& object = dao.retrieve( c->request()->param( "id" ).toUInt() );
  if ( ! object.isNull() )
  {
    c->setStash( "object", object );
    view( c );
  }
  else c->response()->redirect( "/agreements" );
}

void Agreements::view( Cutelyst::Context* c ) const
{
  const auto& object = c->stash( "object" );
  const auto ptr = model::Agreement::from( object );

  const auto& relations = dao::InstitutionAgreementDAO().retrieve( c, QString::number( ptr->getId() ) );

  dao::ProgramDAO dao;

  const auto func = [](const QVariant& var, QJsonArray& array)
  {
    const auto ptr = model::Program::from( var );
    QJsonObject json;
    json.insert( "id", static_cast<int>( ptr->getId() ) );
    json.insert( "title", ptr->getTitle() );
    array << json;
  };

  auto iid = ptr->getTransferInstitutionId();
  const auto& trp = iid ?  dao.retrieveByInstitution( iid ) : QVariantList();
  QJsonArray transferPrograms;
  for ( const auto& value : trp ) func( value, transferPrograms );

  iid = ptr->getTransfereeInstitutionId();
  const auto& treep = iid ?  dao.retrieveByInstitution( iid ) : QVariantList();
  QJsonArray transfereePrograms;
  for ( const auto& value : treep ) func( value, transfereePrograms );

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