#include "DesignationDAO.h"
#include "constants.h"
#include "model/Designation.h"

#include <mutex>
#include <unordered_map>

#include <QtCore/QLoggingCategory>
#include <QtSql/QtSql>
#include <Cutelyst/Plugins/Utils/sql.h>

using crrc::model::Designation;

Q_LOGGING_CATEGORY( DESIGNATION_DAO, "crrc.dao.DesignationDAO" )

namespace crrc
{
  namespace dao
  {
    static std::unordered_map<uint32_t, Designation::Ptr> designations;
    static std::atomic_bool designationsLoaded{ false };
    static std::mutex designationMutex;

    QVariantList fromDesignations()
    {
      QVariantList list;
      for ( const auto& iter : designations )
      {
        list << asVariant( iter.second.get() );
      }

      return list;
    }

    void loadDesignations()
    {
      if ( designationsLoaded.load() ) return;
      std::lock_guard<std::mutex> lock{ designationMutex };
      if ( !designations.empty() )
      {
        designationsLoaded = true;
        return;
      }

      auto query = CPreparedSqlQueryThreadForDB(
        "select designation_id, type, title from designations order by type, title",
        DATABASE_NAME );

      if ( query.exec() )
      {
        while ( query.next() )
        {
          auto designation = Designation::create( query );
          auto id = designation->getId();
          designations[id] = std::move( designation );
        }

        designationsLoaded = true;
      }
    }

    void bindDesignation( Cutelyst::Context* c, QSqlQuery& query )
    {
      query.bindValue( ":title", c->request()->param( "title" ) );
      query.bindValue( ":type", c->request()->param( "type" ) );
    }
  }
}

using crrc::dao::DesignationDAO;

QVariantList DesignationDAO::retrieveAll() const
{
  loadDesignations();
  return fromDesignations();
}

QVariant DesignationDAO::retrieve( const uint32_t id ) const
{
  loadDesignations();
  const auto& iter = designations.find( id );
  return ( iter != designations.end() ) ? asVariant( iter->second.get() ) : QVariant();
}

QVariantList DesignationDAO::retrieveByType( const QString& type ) const
{
  loadDesignations();
  QVariantList list;

  for ( const auto& iter : designations )
  {
    if ( type == iter.second->getType() ) list << asVariant( iter.second.get() );
  }

  return list;
}

uint32_t DesignationDAO::insert( Cutelyst::Context* context ) const
{
  loadDesignations();
  QSqlQuery query = CPreparedSqlQueryThreadForDB(
      "insert into designations (title, type) values (:title, :type)",
      crrc::DATABASE_NAME );
  bindDesignation( context, query );

  if ( !query.exec() )
  {
    qWarning( DESIGNATION_DAO ) << query.lastError().text();
    context->stash()["error_msg"] = query.lastError().text();
    return 0;
  }

  const auto id = query.lastInsertId().toUInt();
  auto designation = Designation::create( context );
  designation->setId( id );
  std::lock_guard<std::mutex> lock{ designationMutex };
  designations[id] = std::move( designation );
  return id;
}

uint32_t DesignationDAO::update( Cutelyst::Context* context ) const
{
  loadDesignations();
  auto id = context->request()->param( "id" );
  auto query = CPreparedSqlQueryThreadForDB(
      "update designations set title=:title, type=:type where designation_id=:id",
      crrc::DATABASE_NAME );
  bindDesignation( context, query );
  query.bindValue( ":id", id.toInt() );

  if ( query.exec() && query.numRowsAffected() )
  {
    const auto did = id.toUInt();
    auto designation = Designation::create( context );
    designation->setId( did );
    std::lock_guard<std::mutex> lock{ designationMutex };
    designations[did] = std::move( designation );
  }
  else
  {
    context->stash()["error_msg"] = query.lastError().text();
    qWarning( DESIGNATION_DAO ) << query.lastError().text();
  }

  return query.numRowsAffected();
}

uint32_t DesignationDAO::remove( uint32_t id ) const
{
  loadDesignations();
  auto query = CPreparedSqlQueryThreadForDB(
      "delete from designations where designation_id = :id", DATABASE_NAME );
  query.bindValue( ":id", id );
  if ( query.exec() && query.numRowsAffected() )
  {
    const auto count = query.numRowsAffected();
    std::lock_guard<std::mutex> lock{ designationMutex };
    designations.erase( id );
    return count;
  }

  qWarning( DESIGNATION_DAO ) << query.lastError().text();
  return 0;
}
