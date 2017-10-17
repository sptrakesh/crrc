#include "InstitutionDesignations.h"
#include "dao/InstitutionDAO.h"
#include "dao/InstitutionDesignationDAO.h"
#include "dao/DesignationDAO.h"
#include "dao/functions.h"
#include "model/Designation.h"
#include "model/InstitutionDesignation.h"

#include <QtCore/QDate>
#include <QtCore/QDebug>
#include <QtCore/QSet>
#include <QtCore/QJsonArray>

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
  c->setStash( "object", dao::InstitutionDAO().retrieve( id.toUInt() ) );
}

void InstitutionDesignations::view( Cutelyst::Context* c ) const
{
  const auto ptr = model::Institution::from( c->stash( "object" ) );
  const auto id = ptr ? ptr->getId() : 0;
  const auto& list = dao::InstitutionDesignationDAO().retrieve( c, id );

  if ( "POST" == c->request()->method() )
  {
    QJsonArray arr;
    for ( const auto var : list )
    {
      const auto ptr = model::InstitutionDesignation::from( var );
      arr << toJson( *ptr );
    }
    dao::sendJson( c, arr );
    return;
  }

  c->stash( {
    { "template", "institutions/designations/view.html" },
    { "members", list }
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

  const auto ptr = model::Institution::from( c->stash( "object" ) );
  const auto id = ptr ? ptr->getId() : 0;
  const auto& members = dao::InstitutionDesignationDAO().retrieve( c, id );
  QSet<uint32_t> set;
  for ( const auto& member : members )
  {
    const auto ptr = model::InstitutionDesignation::from( member );
    set << ptr->getDesignationId();
  }

  const auto& designations = dao::DesignationDAO().retrieveAll();
  QVariantList nonMembers;
  for ( const auto item : designations )
  {
    const auto* designation = model::Designation::from( item );
    if ( !set.contains( designation->getId() ) ) nonMembers << item;
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

void InstitutionDesignations::sendJson( Cutelyst::Context* c, const uint32_t result ) const
{
  QJsonObject json;
  json.insert( "status", result ? true : false );
  const auto& error = c->stash( "error_msg" );
  if ( !error.isNull() ) json.insert( "error", error.toString() );
  dao::sendJson( c, json );
}
