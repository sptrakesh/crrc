#include "Departments.h"
#include "dao/functions.h"
#include "dao/DepartmentDAO.h"
#include "dao/InstitutionDAO.h"
#include "model/Department.h"

#include <QtCore/QJsonArray>

namespace crrc
{
  namespace util
  {
    static auto departmentComparator = []( const QVariant& deg1, const QVariant& deg2 ) -> bool
    {
      const auto ptr1 = model::Department::from( deg1 );
      const auto ptr2 = model::Department::from( deg2 );
      return *ptr1 < *ptr2;
    };
  }
}

using crrc::Departments;

void Departments::index( Cutelyst::Context* c )
{
  const auto& list = dao::isGlobalAdmin( c ) ?
    dao::DepartmentDAO().retrieveAll() :
    dao::DepartmentDAO().retrieveByInstitution( dao::institutionId( c ) );

  QVariantList ilist;
  if ( dao::isGlobalAdmin( c ) ) ilist = dao::InstitutionDAO().retrieveAll();
  else ilist << dao::InstitutionDAO().retrieve( dao::institutionId( c ) );
  qSort( list.begin(), list.end(), util::departmentComparator );

  QJsonArray arr;
  for ( const auto& department : list ) arr << toJson( *model::Department::from( department ) );
  dao::sendJson( c, arr );
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
  dao::sendJson( c, ptr ? toJson( *ptr ) : QJsonObject() );
}

