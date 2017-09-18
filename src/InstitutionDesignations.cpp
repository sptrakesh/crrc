#include "InstitutionDesignations.h"
#include "dao/InstitutionDAO.h"
#include "dao/InstitutionDesignationDAO.h"
#include "dao/DesignationDAO.h"

#include <QtCore/QDate>
#include <QtCore/QDebug>
#include <QtCore/QSet>
#include <QtCore/QJsonDocument>
#include <QtCore/QJsonObject>

using crrc::InstitutionDesignations;

void InstitutionDesignations::index( Cutelyst::Context* c )
{
  c->response()->body() = "JSON services";
}

void InstitutionDesignations::base( Cutelyst::Context* c ) const
{
  c->response()->setHeader( "Cache-Control", "no-cache, no-store, must-revalidate" );
}

void InstitutionDesignations::object( Cutelyst::Context* c, const QString& id ) const
{
  c->setStash( "object", dao::InstitutionDAO().retrieve( id ) );
}

void InstitutionDesignations::view( Cutelyst::Context* c ) const
{
  const auto id = c->stash( "object" ).toHash().value( "institution_id" ).toUInt();
  c->stash( {
    { "template", "institutions/designations/view.html" },
    { "members", dao::InstitutionDesignationDAO().retrieve( id ) }
  } );
}

void InstitutionDesignations::edit( Cutelyst::Context* c ) const
{
  QVariantList years;
  for ( auto i = 0; i < 10; ++i )
  {
    auto dt = QDate::currentDate().addYears( i );
    years << dt.year();
  }

  const auto id = c->stash( "object" ).toHash().value( "institution_id" ).toUInt();
  const auto& members = dao::InstitutionDesignationDAO().retrieve( id );
  QSet<uint32_t> set;
  for ( const auto& member : members )
  {
    set << member.toHash().value( "designation" ).toHash().value( "designation_id" ).toUInt();
  }

  const auto& designations = dao::DesignationDAO().retrieveAll();
  QVariantList nonMembers;
  for ( const auto designation : designations )
  {
    const auto& id = designation.toHash().value( "designation_id" ).toUInt();
    if ( !set.contains( id ) ) nonMembers << designation;
  }

  c->stash( {
    { "template", "institutions/designations/form.html" },
    { "members", members },
    { "designations", nonMembers },
    { "years", years }
  } );
}

void InstitutionDesignations::save( Cutelyst::Context* c ) const
{
  sendJson( c, dao::InstitutionDesignationDAO().save( c ) );
}

void InstitutionDesignations::remove( Cutelyst::Context* c ) const
{
  sendJson( c, dao::InstitutionDesignationDAO().remove( c ) );
}

void InstitutionDesignations::sendJson( Cutelyst::Context* c, const QString& result ) const
{
  QJsonObject json;
  json.insert( "status", ( result.isEmpty() ? true : false ) );
  if ( !result.isEmpty() ) json.insert( "error", result );

  const auto& bytes = QJsonDocument( json ).toJson();

  c->response()->setContentType( "application/json" );
  c->response()->setContentLength( bytes.size() );
  c->response()->setBody( bytes );
}
