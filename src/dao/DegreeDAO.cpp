#include "DegreeDAO.h"
#include "constants.h"

#include <mutex>
#include <QtCore/QStringBuilder>
#include <QtSql/QtSql>
#include <Cutelyst/Plugins/Utils/sql.h>


namespace crrc
{
  namespace dao
  {
    struct Degree
    {
      QVariant id;
      QVariant title;
      QVariant duration;
    };

    static QMap<uint32_t, Degree> degrees;
    static std::atomic_bool degreesLoaded{ false };
    static std::mutex degreeMutex;

    QVariantHash transform( const Degree& degree )
    {
      QVariantHash record;
      record.insert( "degree_id", degree.id );
      record.insert( "title", degree.title );
      record.insert( "duration", degree.duration );

      return record;
    }

    QVariantList fromDegrees()
    {
      QVariantList list;
      foreach ( Degree degree, degrees )
      {
        list.append( transform( degree ) );
      }

      return list;
    }

    Degree createDegree( QSqlQuery& query )
    {
      Degree degree;
      degree.id = query.value( 0 ).toUInt();
      degree.title = query.value( 1 );
      degree.duration = query.value( 2 );
      return degree;
    }

    void loadDegrees()
    {
      if ( degreesLoaded.load() ) return;
      std::lock_guard<std::mutex> lock{ degreeMutex };
      if ( !degrees.isEmpty() )
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
          const auto degree = createDegree( query );
          auto id = degree.id.toUInt();
          degrees.insert( id, std::move( degree ) );
        }

        degreesLoaded = true;
      }
    }

    void bindDegree( Cutelyst::Context* c, QSqlQuery& query )
    {
      query.bindValue( ":title", c->request()->param( "title" ) );
      query.bindValue( ":duration", c->request()->param( "duration" ) );
    }

    Degree degreeFromContext( Cutelyst::Context* context )
    {
      Degree degree;
      degree.title = context->request()->param( "title" );
      degree.duration = context->request()->param( "duration" );
      return degree;
    }
  }
}

using crrc::dao::DegreeDAO;

QVariantList DegreeDAO::retrieveAll() const
{
  loadDegrees();
  return fromDegrees();
}

QVariantHash DegreeDAO::retrieve( const QString& id ) const
{
  loadDegrees();
  const uint32_t cid = id.toUInt();
  const auto iter = degrees.constFind( cid );
  return ( iter != degrees.end() ) ? transform( iter.value() ) : QVariantHash();
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
  else
  {
    const auto id = query.lastInsertId().toUInt();
    auto degree = degreeFromContext( context );
    degree.id = id;
    std::lock_guard<std::mutex> lock{ degreeMutex };
    degrees[id] = std::move( degree );
    return id;
  }
}

void DegreeDAO::update( Cutelyst::Context* context ) const
{
  loadDegrees();
  auto id = context->request()->param( "degree_id" );
  auto query = CPreparedSqlQueryThreadForDB(
    "update degrees set title=:title, duration=:duration where degree_id=:id",
    crrc::DATABASE_NAME );
  bindDegree( context, query );
  query.bindValue( ":id", id.toInt() );

  if ( query.exec() )
  {
    auto degree = degreeFromContext( context );
    degree.id = id.toUInt();
    std::lock_guard<std::mutex> lock{ degreeMutex };
    degrees[id.toUInt()] = std::move( degree );
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
      auto degree = retrieve( query.value( 0 ).toString() );
      list.append( degree );
    }
  }
  else
  {
    qWarning() << query.lastError().text();
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
    std::lock_guard<std::mutex> lock{ degreeMutex };
    degrees.remove( id );
    return query.numRowsAffected();
  }

  qDebug() << query.lastError().text();
  return 0;
}