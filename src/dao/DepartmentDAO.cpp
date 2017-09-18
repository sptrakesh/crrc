#include "DepartmentDAO.h"
#include "constants.h"

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

      return record;
    }

    QVariantList fromDepartments()
    {
      QVariantList list;
      foreach ( Department department, departments )
      {
        list.append( transform( department ) );
      }

      return list;
    }

    Department createDepartment( QSqlQuery& query )
    {
      Department department;
      department.id = query.value( 0 ).toUInt();
      department.name = query.value( 1 );
      department.prefix = query.value( 2 );
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
        "select department_id, name, prefix from departments order by name",
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
    }

    Department departmentFromContext( Cutelyst::Context* context )
    {
      Department department;
      department.name = context->request()->param( "name" );
      department.prefix = context->request()->param( "prefix" );
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
    "insert into departments (name, prefix) values (:name, :prefix)",
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

bool DepartmentDAO::isUnique( const QString& name, const QString& prefix ) const
{
  loadDepartments();
  for ( const auto& department : departments )
  {
    if ( department.name.toString() == name &&
      department.prefix == prefix ) return false;
  }

  return true;
}
