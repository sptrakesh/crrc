#include "DepartmentDAO.h"
#include "constants.h"
#include "InstitutionDAO.h"

#include <mutex>
#include <QtCore/QStringBuilder>
#include <QtSql/QtSql>
#include <Cutelyst/Plugins/Utils/sql.h>

namespace crrc
{
  namespace dao
  {
    struct Department
    {
      QVariant id;
      QVariant name;
      QVariant prefix;
      QVariant institutionId;
    };

    static QMap<uint32_t, Department> departments;
    static std::atomic_bool departmentsLoaded{ false };
    static std::mutex departmentMutex;

    QVariantHash transform( const Department& department )
    {
      QVariantHash record;
      record.insert( "department_id", department.id );
      record.insert( "name", department.name );
      record.insert( "prefix", department.prefix );
      record.insert( "institution", InstitutionDAO().retrieve(
        department.institutionId.toString(), InstitutionDAO::Mode::Partial ) );

      return record;
    }

    QVariantList fromDepartments()
    {
      QVariantList list;
      for ( const auto& department : departments ) list.append( transform( department ) );
      return list;
    }

    Department createDepartment( QSqlQuery& query )
    {
      Department department;
      department.id = query.value( 0 ).toUInt();
      department.name = query.value( 1 );
      department.prefix = query.value( 2 );
      department.institutionId = query.value( 3 ).toUInt();
      return department;
    }

    void loadDepartments()
    {
      if ( departmentsLoaded.load() ) return;
      std::lock_guard<std::mutex> lock{ departmentMutex };
      if ( !departments.isEmpty() )
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
          const auto department = createDepartment( query );
          auto id = department.id.toUInt();
          departments.insert( id, std::move( department ) );
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

    Department departmentFromContext( Cutelyst::Context* context )
    {
      Department department;
      department.name = context->request()->param( "name" );
      department.prefix = context->request()->param( "prefix" );
      department.institutionId = context->request()->param( "institution_id" ).toUInt();
      return department;
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

  for ( const auto& department : departments )
  {
    if ( institutionId == department.institutionId.toUInt() ) list << transform( department );
  }

  return list;
}

QVariantHash DepartmentDAO::retrieve( const QString& id ) const
{
  loadDepartments();
  const uint32_t cid = id.toUInt();
  const auto iter = departments.constFind( cid );
  return ( iter != departments.end() ) ? transform( iter.value() ) : QVariantHash();
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
  else
  {
    const auto id = query.lastInsertId().toUInt();
    auto department = departmentFromContext( context );
    department.id = id;
    std::lock_guard<std::mutex> lock{ departmentMutex };
    departments[id] = std::move( department );
    return id;
  }
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
    auto department = departmentFromContext( context );
    department.id = did;
    std::lock_guard<std::mutex> lock{ departmentMutex };
    departments[did] = std::move( department );
  }
  else context->stash()["error_msg"] = query.lastError().text();

  return query.numRowsAffected();
}

QString DepartmentDAO::remove( uint32_t id ) const
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
      departments.remove( id );
      return QString::number( query.numRowsAffected() );
    }
    return QString( "No records matched." );
  }

  return query.lastError().text();
}
