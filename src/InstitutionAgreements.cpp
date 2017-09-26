#include "InstitutionAgreements.h"
#include "dao/AgreementDAO.h"
#include "dao/InstitutionAgreementDAO.h"

#include <QtCore/QStringBuilder>
#include <QtCore/QDebug>
#include <QtCore/QJsonDocument>
#include <QtCore/QJsonObject>

using crrc::InstitutionAgreements;

void InstitutionAgreements::index( Cutelyst::Context* c ) const
{
  dao::InstitutionAgreementDAO dao;
  c->setStash( "template", "institutions/agreements/index.html" );
}

void InstitutionAgreements::base( Cutelyst::Context* c ) const
{
  c->response()->setHeader( "Cache-Control", "no-cache, no-store, must-revalidate" );
}


void InstitutionAgreements::object( Cutelyst::Context* c, const QString& id ) const
{
  auto list = dao::InstitutionAgreementDAO().retrieve( id );
  c->setStash( "object", list.first() );
}

void InstitutionAgreements::edit( Cutelyst::Context* c ) const
{
  if ( !validate( c ) ) return;

  const auto id = dao::InstitutionAgreementDAO().insert( c );
  QJsonObject json;
  json.insert( "id", static_cast<int>( id ) );
  const auto& output = QJsonDocument( json ).toJson();

  c->response()->setContentType( "application/json" );
  c->response()->setContentLength( output.size() );
  c->response()->setBody( output );
}

void InstitutionAgreements::remove( Cutelyst::Context* c )
{
  if ( !validate( c ) ) return;
  dao::InstitutionAgreementDAO().remove( c );

  const auto response = QString( "true" );
  c->response()->setContentType( "text/plain" );
  c->response()->setContentLength( response.size() );
  c->response()->setBody( response );
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
