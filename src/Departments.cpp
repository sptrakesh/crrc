#include "Departments.h"
#include "dao/functions.h"
#include "dao/DepartmentDAO.h"
#include "dao/InstitutionDAO.h"
#include "model/Department.h"

using crrc::Departments;

void Departments::index( Cutelyst::Context* c )
{
  const auto& list = dao::isGlobalAdmin( c ) ?
    dao::DepartmentDAO().retrieveAll() :
    dao::DepartmentDAO().retrieveByInstitution( dao::institutionId( c ) );

  QVariantList ilist;
  if ( dao::isGlobalAdmin( c ) ) ilist = dao::InstitutionDAO().retrieveAll();
  else ilist << dao::InstitutionDAO().retrieve( dao::institutionId( c ) );

  c->stash( {
    { "departments", list },
    { "institutions", ilist },
    { "template", "institutions/departments/index.html" }
  } );
}

void Departments::base( Cutelyst::Context* c ) const
{
  c->response()->setHeader( "Cache-Control", "no-cache, no-store, must-revalidate" );
}

void Departments::object( Cutelyst::Context* c, const QString& id ) const
{
  const auto& obj = dao::DepartmentDAO().retrieve( id.toUInt() );
  c->setStash( "object", dao::isGlobalAdmin( c ) ? obj :
    ( id.toUInt() == dao::institutionId( c ) ) ? obj : QVariant() );
}

void Departments::data( Cutelyst::Context* c ) const
{
  const auto& var = c->stash( "object" );
  const auto ptr = model::Department::from( var );
  dao::sendJson( c, toJson( *ptr ) );
}

void Departments::remove( Cutelyst::Context* c ) const
{
  const auto& var = c->stash( "object" );
  QJsonObject obj;

  if ( var.isNull() )
  {
    obj.insert( "status", false );
    dao::sendJson( c, obj );
    return;
  }

  const auto ptr = model::Department::from( var );
  auto const result = dao::DepartmentDAO().remove( ptr->getId() );
  obj.insert( "id", static_cast<int>( ptr->getId() ) );

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
