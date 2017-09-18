#include "Departments.h"
#include "dao/DepartmentDAO.h"

#include <QtCore/QStringBuilder>
#include <QtCore/QtDebug>
#include <QtCore/QJsonDocument>
#include <QtCore/QJsonObject>

using crrc::Departments;

void Departments::index( Cutelyst::Context* c)
{
  c->response()->setBody( QString( "Departments" ) );
}

void Departments::base( Cutelyst::Context* c ) const
{
  c->response()->setHeader( "Cache-Control", "no-cache, no-store, must-revalidate" );
}

void Departments::create( Cutelyst::Context* c ) const
{
  const uint32_t id = dao::DepartmentDAO().insert( c );
  QJsonObject json;
  json.insert( "id", static_cast<int>( id ) );
  const auto& bytes = QJsonDocument( json ).toJson();

  c->response()->setContentType( "application/json" );
  c->response()->setContentLength( bytes.size() );
  c->response()->setBody( bytes );
}

void Departments::checkUnique( Cutelyst::Context* c )
{
  const auto name = c->request()->param( "name" );
  const auto prefix = c->request()->param( "prefix" );
  const auto status = dao::DepartmentDAO().isUnique( name, prefix );
  const QString string = status ? "true" : "false";

  c->response()->setContentType( "text/plain" );
  c->response()->setContentLength( string.size() );
  c->response()->setBody( string );
}
