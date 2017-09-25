#include "Departments.h"
#include "dao/DepartmentDAO.h"
#include "dao/InstitutionDAO.h"
#include "dao/functions.h"

#include <QtCore/QStringBuilder>
#include <QtCore/QtDebug>
#include <QtCore/QJsonObject>

using crrc::Departments;

void Departments::index( Cutelyst::Context* c )
{
  const auto& list = dao::isGlobalAdmin( c ) ?
    dao::DepartmentDAO().retrieveAll() :
    dao::DepartmentDAO().retrieveByInstitution( dao::institutionId( c ) );

  const auto mode = dao::InstitutionDAO::Mode::Partial;
  QVariantList ilist;
  if ( dao::isGlobalAdmin( c ) ) ilist = dao::InstitutionDAO().retrieveAll( mode );
  else
  {
    ilist << dao::InstitutionDAO().retrieve( QString::number( dao::institutionId( c ) ), mode );
  }

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
  const auto& obj = dao::InstitutionDAO().retrieve( id );
  c->setStash( "object", dao::isGlobalAdmin( c ) ? obj :
    ( id.toUInt() == dao::institutionId( c ) ) ? obj : QVariantHash() );
}

void Departments::list( Cutelyst::Context* c ) const
{
  const auto& obj = c->stash( "object" ).toHash();
  const auto &list = obj.isEmpty() ? QVariantList() :
    dao::DepartmentDAO().retrieveByInstitution( obj.value( "institution_id" ).toUInt() );
  QVariantList ilist{ obj };

  c->stash( {
    { "departments", list },
    { "institutions", ilist },
    { "template", "institutions/departments/index.html" }
  } );
}

void Departments::save( Cutelyst::Context* c ) const
{
  const auto& did = c->request()->param( "department_id" );
  const auto& iid = c->request()->param( "institution_id" );

  dao::DepartmentDAO dao;
  QJsonObject json;

  if ( !dao::isGlobalAdmin( c ) )
  {
    if ( !iid.toUInt() == dao::institutionId( c ) )
    {
      json.insert( "message", "Unauthorized.  User may not manage departments in other institutions." );
      json.insert( "status", false );
      dao::sendJson( c, json );
      return;
    }
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

void Departments::remove( Cutelyst::Context* c ) const
{
  auto id = c->request()->param( "department_id", "" );
  QJsonObject obj;

  if ( id.isEmpty() )
  {
    obj.insert( "status", false );
    dao::sendJson( c, obj );
    return;
  }

  auto const result = dao::DepartmentDAO().remove( id.toUInt() );
  obj.insert( "id", id.toInt() );

  if ( result == "1" )
  {
    obj.insert( "count", result.toInt() );
    obj.insert( "status", true );
  }
  else obj.insert( "status", false );

  const auto& err = c->stash( "error_msg" );
  if ( !err.isNull() ) obj.insert( "message", err.toString() );

  dao::sendJson( c, obj );
}