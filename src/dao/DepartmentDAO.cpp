#include "DepartmentDAO.h"
#include "constants.h"
#include "InstitutionDAO.h"
#include "model/Department.h"

#include <mutex>
#include <unordered_map>

#include <QtCore/QStringBuilder>
#include <QtSql/QtSql>
#include <Cutelyst/Plugins/Utils/sql.h>

using crrc::model::Department;

namespace crrc
{
  namespace dao
  {
    static std::unordered_map<uint32_t, Department::Ptr> departments;
    static std::atomic_bool departmentsLoaded{ false };
    static std::mutex departmentMutex;

    QVariantList fromDepartments()
    {
      QVariantList list;
      for ( const auto& iter : departments ) list << asVariant( iter.second.get() );
      return list;
    }

    void loadDepartments()
    {
      if ( departmentsLoaded.load() ) return;
      std::lock_guard<std::mutex> lock{ departmentMutex };
      if ( !departments.empty() )
      {
        departmentsLoaded = true;
        return;
      }

      auto query = CPreparedSqlQueryThreadForDB(
        "select department_id, name, prefix, institution_id from departments order by institution_id, name",
        DATABASE_NAME );

      if ( query.exec() )
      {
        while ( query.next() )
        {
          auto department = Department::create( query );
          departments[department->getId()] = std::move( department );
        }

        departmentsLoaded = true;
      }
    }

    void bindDepartment( Cutelyst::Context* c, QSqlQuery& query )
    {
      query.bindValue( ":name", c->request()->param( "name" ) );
      query.bindValue( ":prefix", c->request()->param( "prefix" ) );
      query.bindValue( ":inst", c->request()->param( "institution_id" ) );
    }
  }
}

using crrc::dao::DepartmentDAO;

QVariantList DepartmentDAO::retrieveAll() const
{
  loadDepartments();
  return fromDepartments();
}

QVariantList DepartmentDAO::retrieveByInstitution( uint32_t institutionId ) const
{
  loadDepartments();
  QVariantList list;

  for ( const auto& iter : departments )
  {
    if ( institutionId == iter.second->getInstitutionId() ) list << asVariant( iter.second.get() );
  }

  return list;
}

QVariant DepartmentDAO::retrieve( const uint32_t id ) const
{
  loadDepartments();
  const auto iter = departments.find( id );
  return ( iter != departments.end() ) ? asVariant( iter->second.get() ) : QVariantHash();
}

uint32_t DepartmentDAO::insert( Cutelyst::Context* context ) const
{
  loadDepartments();
  QSqlQuery query = CPreparedSqlQueryThreadForDB(
    "insert into departments (name, prefix, institution_id) values (:name, :prefix, :inst)",
    crrc::DATABASE_NAME );
  bindDepartment( context, query );

  if ( !query.exec() )
  {
    context->stash()["error_msg"] = query.lastError().text();
    return 0;
  }

  const auto id = query.lastInsertId().toUInt();
  auto department = Department::create( context );
  department->setId( id );
  std::lock_guard<std::mutex> lock{ departmentMutex };
  departments[id] = std::move( department );
  return id;
}

uint32_t DepartmentDAO::update( Cutelyst::Context* context ) const
{
  loadDepartments();
  auto id = context->request()->param( "department_id" );
  auto query = CPreparedSqlQueryThreadForDB(
    "update departments set name=:name, prefix=:prefix, institution_id=:inst where department_id=:id",
    crrc::DATABASE_NAME );
  bindDepartment( context, query );
  query.bindValue( ":id", id.toInt() );

  if ( query.exec() && query.numRowsAffected() )
  {
    const auto did = id.toUInt();
    auto department = Department::create( context );
    department->setId( did );
    std::lock_guard<std::mutex> lock{ departmentMutex };
    departments[did] = std::move( department );
  }
  else
  {
    context->stash()["error_msg"] = query.lastError().text();
    qDebug() << query.lastError().text();
  }

  return query.numRowsAffected();
}

uint32_t DepartmentDAO::remove( uint32_t id ) const
{
  loadDepartments();
  auto query = CPreparedSqlQueryThreadForDB(
    "delete from departments where department_id = :id", DATABASE_NAME );
  query.bindValue( ":id", id );
  if ( query.exec() )
  {
    if ( query.numRowsAffected() )
    {
      std::lock_guard<std::mutex> lock{ departmentMutex };
      departments.erase( id );
      return query.numRowsAffected();
    }
  }

  qDebug() << query.lastError().text();
  return 0;
}
