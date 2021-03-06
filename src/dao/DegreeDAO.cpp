#include "DegreeDAO.h"
#include "constants.h"

#include <mutex>
#include <unordered_map>

#include <QtCore/QLoggingCategory>
#include <QtCore/QStringBuilder>
#include <QtSql/QtSql>
#include <Cutelyst/Plugins/Utils/sql.h>

using crrc::model::Degree;

Q_LOGGING_CATEGORY( DEGREE_DAO, "crrc.dao.DegreeDAO" )

namespace crrc
{
  namespace dao
  {
    static std::unordered_map<uint32_t, Degree::Ptr> degrees;
    static std::atomic_bool degreesLoaded{ false };
    static std::mutex degreeMutex;

    QVariantList fromDegrees()
    {
      QVariantList list;
      for ( const auto& iter : degrees ) list << asVariant( iter.second.get() );

      return list;
    }

    void loadDegrees()
    {
      if ( degreesLoaded.load() ) return;
      std::lock_guard<std::mutex> lock{ degreeMutex };
      if ( !degrees.empty() )
      {
        degreesLoaded = true;
        return;
      }

      auto query = CPreparedSqlQueryThreadForDB(
        "select degree_id, title, duration from degrees order by title",
        DATABASE_NAME );

      if ( query.exec() )
      {
        while ( query.next() )
        {
          auto degree = Degree::create( query );
          degrees[degree->getId()] = std::move( degree );
        }

        degreesLoaded = true;
      }
    }

    void bindDegree( Cutelyst::Context* c, QSqlQuery& query )
    {
      query.bindValue( ":title", c->request()->param( "title" ) );
      query.bindValue( ":duration", c->request()->param( "duration" ) );
    }
  }
}

using crrc::dao::DegreeDAO;

const QVariantList DegreeDAO::retrieveAll() const
{
  loadDegrees();
  return fromDegrees();
}

const QVariant DegreeDAO::retrieve( uint32_t id ) const
{
  loadDegrees();
  const auto iter = degrees.find( id );
  return ( iter != degrees.end() ) ? asVariant( iter->second.get() ) : QVariant();
}

uint32_t DegreeDAO::insert( Cutelyst::Context* context ) const
{
  loadDegrees();
  QSqlQuery query = CPreparedSqlQueryThreadForDB(
    "insert into degrees (title, duration) values (:title, :duration)",
    crrc::DATABASE_NAME );
  bindDegree( context, query );

  if ( !query.exec() )
  {
    context->stash()["error_msg"] = query.lastError().text();
    return 0;
  }

  const auto id = query.lastInsertId().toUInt();
  auto degree = Degree::create( context );
  degree->setId( id );
  std::lock_guard<std::mutex> lock{ degreeMutex };
  degrees[id] = std::move( degree );
  return id;
}

void DegreeDAO::update( Cutelyst::Context* context ) const
{
  loadDegrees();
  auto id = context->request()->param( "id" );
  auto query = CPreparedSqlQueryThreadForDB(
    "update degrees set title=:title, duration=:duration where degree_id=:id",
    crrc::DATABASE_NAME );
  bindDegree( context, query );
  query.bindValue( ":id", id.toInt() );

  if ( query.exec() )
  {
    if ( query.numRowsAffected() )
    {
      auto degree = Degree::create( context );
      std::lock_guard<std::mutex> lock{ degreeMutex };
      degrees[id.toUInt()] = std::move( degree );
    }

    context->stash()["count"] = query.numRowsAffected();
  }
  else context->stash()["error_msg"] = query.lastError().text();
}

QVariantList DegreeDAO::search( Cutelyst::Context* context ) const
{
  loadDegrees();
  const auto text = context->request()->param( "text", "" );
  const QString clause = "%" % text % "%";

  auto query = CPreparedSqlQueryThreadForDB(
   "select degree_id from degrees where title like :text order by title",
    DATABASE_NAME );

  query.bindValue( ":text", clause );
  QVariantList list;

  if ( query.exec() )
  {
    while ( query.next() )
    {
      auto degree = retrieve( query.value( 0 ).toUInt() );
      if ( !degree.isNull() ) list << degree;
    }
  }
  else
  {
    qWarning( DEGREE_DAO ) << query.lastError().text();
    context->stash()["error_msg"] = query.lastError().text();
  }

  return list;
}

uint32_t DegreeDAO::remove( uint32_t id ) const
{
  loadDegrees();
  auto query = CPreparedSqlQueryThreadForDB(
    "delete from degrees where degree_id = :id", DATABASE_NAME );
  query.bindValue( ":id", id );
  if ( query.exec() )
  {
    const auto count = query.numRowsAffected();
    std::lock_guard<std::mutex> lock{ degreeMutex };
    degrees.erase( id );
    return count;
  }

  qWarning( DEGREE_DAO ) << query.lastError().text();
  return 0;
}