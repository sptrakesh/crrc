#include "InstitutionDepartments.h"
#include "dao/DepartmentDAO.h"
#include "dao/InstitutionDAO.h"
#include "dao/functions.h"

#include <QtCore/QtDebug>

using crrc::InstitutionDepartments;

void InstitutionDepartments::index( Cutelyst::Context* c )
{
  const auto& list = dao::isGlobalAdmin( c ) ?
    dao::DepartmentDAO().retrieveAll() :
    dao::DepartmentDAO().retrieveByInstitution( dao::institutionId( c ) );

  QVariantList ilist;
  if ( dao::isGlobalAdmin( c ) ) ilist = dao::InstitutionDAO().retrieveAll();
  else
  {
    ilist << dao::InstitutionDAO().retrieve( dao::institutionId( c ) );
  }

  c->stash( {
    { "departments", list },
    { "institutions", ilist },
    { "template", "institutions/departments/index.html" }
  } );
}

void InstitutionDepartments::base( Cutelyst::Context* c ) const
{
  c->response()->setHeader( "Cache-Control", "no-cache, no-store, must-revalidate" );
}

void InstitutionDepartments::object( Cutelyst::Context* c, const QString& id ) const
{
  const auto& obj = dao::InstitutionDAO().retrieve( id.toUInt() );
  c->setStash( "object", dao::isGlobalAdmin( c ) ? obj :
    ( id.toUInt() == dao::institutionId( c ) ) ? obj : QVariant() );
}

void InstitutionDepartments::list( Cutelyst::Context* c ) const
{
  const auto obj = c->stash( "object" );
  const auto ptr = model::Institution::from( c->stash( "object" ) );
  const auto id = ptr ? ptr->getId() : 0;
  const auto& list = ptr ?  dao::DepartmentDAO().retrieveByInstitution( id ) : QVariantList();
  QVariantList ilist{ obj };

  c->stash( {
    { "departments", list },
    { "institutions", ilist },
    { "template", "institutions/departments/index.html" }
  } );
}

void InstitutionDepartments::save( Cutelyst::Context* c ) const
{
  const auto& did = c->request()->param( "id" );
  const auto& iid = c->request()->param( "institutionId" );

  dao::DepartmentDAO dao;
  QJsonObject json;

  if ( ! canEdit( c ) )
  {
    json.insert( "message", "Unauthorized.  User may not manage departments in other institutions." );
    json.insert( "status", false );
    dao::sendJson( c, json );
    return;
  }

  if ( did.isEmpty() || ! did.toUInt() )
  {
    const uint32_t id = dao.insert( c );
    json.insert( "id", static_cast<int>( id ) );
    json.insert( "status", id ? true : false );
  }
  else
  {
    const uint32_t count = dao.update( c );
    json.insert( "id", did.toInt() );
    json.insert( "count", static_cast<int>( count ) );
    json.insert( "status", count ? true : false );
  }

  const auto& error = c->stash( "error_msg" );
  if ( !error.isNull() ) json.insert( "message", error.toString() );

  dao::sendJson( c, json );
}

void InstitutionDepartments::remove( Cutelyst::Context* c ) const
{
  auto id = c->request()->param( "id", "" );
  QJsonObject obj;

  if ( id.isEmpty() )
  {
    obj.insert( "status", false );
    dao::sendJson( c, obj );
    return;
  }

  if ( ! canEdit( c ) )
  {
    obj.insert( "message", "Unauthorized.  User may not manage departments in other institutions." );
    obj.insert( "status", false );
    dao::sendJson( c, obj );
    return;
  }

  auto const result = dao::DepartmentDAO().remove( id.toUInt() );
  obj.insert( "id", id.toInt() );

  if ( result )
  {
    obj.insert( "count", static_cast<int>( result ) );
    obj.insert( "status", true );
  }
  else obj.insert( "status", false );

  const auto& err = c->stash( "error_msg" );
  if ( !err.isNull() ) obj.insert( "message", err.toString() );

  dao::sendJson( c, obj );
}

bool InstitutionDepartments::canEdit( Cutelyst::Context* c ) const
{
  if ( dao::isGlobalAdmin( c ) ) return true;
  const auto iid = c->request()->param( "institution" ).toUInt();
  return ( iid && iid == dao::institutionId( c ) );
}
