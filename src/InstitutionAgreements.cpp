#include "InstitutionAgreements.h"
#include "dao/AgreementDAO.h"
#include "dao/InstitutionDAO.h"
#include "dao/ProgramDAO.h"
#include "dao/InstitutionAgreementDAO.h"

#include <QtCore/QStringBuilder>
#include <QtCore/QDebug>
#include <QtCore/QJsonArray>
#include <QtCore/QJsonDocument>
#include <QtCore/QJsonObject>

using crrc::InstitutionAgreements;

void InstitutionAgreements::index( Cutelyst::Context* c ) const
{
  dao::InstitutionAgreementDAO dao;
  c->setStash( "template", "institutionAgreements/index.html" );
}

void InstitutionAgreements::base( Cutelyst::Context* c ) const
{
  c->response()->setHeader( "Cache-Control", "no-cache" );
}


void InstitutionAgreements::object( Cutelyst::Context* c, const QString& id ) const
{
  dao::InstitutionAgreementDAO dao;
  auto list = dao.retrieve( id );
  c->setStash( "object", list.first() );
}

void InstitutionAgreements::create( Cutelyst::Context* c ) const
{
  const auto& institutions = dao::InstitutionDAO().retrieveAll(
    dao::InstitutionDAO::Mode::Partial );

  dao::ProgramDAO pdao;
  QJsonObject json;
  foreach ( auto& institution, institutions )
  {
    const auto& inst = institution.toHash();
    const auto id = inst.value( "institution_id" ).toUInt();
    const auto& p = pdao.retrieveByInstitution( id, dao::ProgramDAO::Mode::Partial );
    json.insert( QString::number( id ), QJsonArray::fromVariantList( p ) );
  }

  c->stash( {
    { "agreements", dao::AgreementDAO().retrieveAll( dao::AgreementDAO::Mode::Partial ) },
    { "institutions", institutions },
    { "programs", QJsonDocument( json ).toJson( QJsonDocument::Compact ) },
    { "template", "institutionAgreements/form.html" }
  } );
}

void InstitutionAgreements::edit( Cutelyst::Context* c ) const
{
  if ( !validate( c ) ) return;

  dao::InstitutionAgreementDAO dao;
  dao.insert( c );

  const auto id = c->request()->param( "agreement_id" );
  const auto tp1 = c->request()->param( "transfer_program_id" );
  const auto tp2 = c->request()->param( "transferee_program_id" );
  c->stash( {
    { "template", "institutionAgreements/view.html" },
    { "object", dao.retrieve( id % "_" % tp1 % "_" % tp2 ) }
  } );
}

void InstitutionAgreements::view( Cutelyst::Context* c ) const
{
  c->setStash( "template", "institutionAgreements/view.html" );
}

void InstitutionAgreements::search( Cutelyst::Context* c ) const
{
  const auto text = c->request()->param( "text", "" );

  if ( text.isEmpty() )
  {
    c->response()->redirect( "/institution/program/relations" );
    return;
  }

  c->stash( {
    { "searchText", text },
    { "template", "institutionAgreements/index.html" }
  } );
}

void InstitutionAgreements::remove( Cutelyst::Context* c )
{
  if ( !validate( c ) ) return;
  dao::InstitutionAgreementDAO().remove( c );
  c->response()->redirect( "/institution/program/relations" );
}

bool InstitutionAgreements::validate( Cutelyst::Context* context ) const
{
  const auto id = context->request()->param( "agreement_id", "" );
  const auto tp1 = context->request()->param( "transfer_program_id", "" );
  const auto tp2 = context->request()->param( "transferee_program_id", "" );

  if ( id.isEmpty() || tp1.isEmpty() || tp2.isEmpty() )
  {
    context->stash()["error_msg"] = "Missing values!";
    return false;
  }

  return true;
}
