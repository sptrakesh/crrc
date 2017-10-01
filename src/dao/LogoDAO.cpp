#include "LogoDAO.h"
#include "constants.h"
#include "model/Logo.h"

#include <mutex>
#include <unordered_map>

#include <QtSql/QtSql>
#include <Cutelyst/Upload>
#include <Cutelyst/Plugins/Utils/sql.h>
#include "functions.h"

using crrc::model::Logo;

namespace crrc
{
  namespace dao
  {
    static std::unordered_map<uint32_t, Logo::Ptr> logos;
    static std::atomic_bool logosLoaded{ false };
    static std::mutex logoMutex;

    QVariantList fromLogos()
    {
      QVariantList list;
      for ( const auto& iter : logos ) list << asVariant( iter.second.get() );
      return list;
    }

    void loadLogos()
    {
      if ( logosLoaded.load() ) return;
      std::lock_guard<std::mutex> lock{ logoMutex };
      if ( !logos.empty() )
      {
        logosLoaded = true;
        return;
      }

      auto query = CPreparedSqlQueryThreadForDB(
        "select logo_id, filename, mimetype, filesize, checksum, updated from logos order by filename",
        DATABASE_NAME );

      if ( query.exec() )
      {
        while ( query.next() )
        {
          auto logo = std::make_unique<Logo>();
          logo->populate( query );
          logos[ logo->getId()] = std::move( logo );
        }

        logosLoaded = true;
      }
    }

    QByteArray bindLogo( Cutelyst::Context* c, QSqlQuery& query )
    {
      auto upload = c->request()->upload( "file" );
      if ( !upload ) return QByteArray();

      auto bytes = upload->readAll();
      const QFileInfo file{ upload->filename() };
      query.bindValue( ":filename", file.fileName() );
      query.bindValue( ":mimetype", upload->contentType() );
      query.bindValue( ":filesize", upload->size() );
      query.bindValue( ":image", bytes );
      query.bindValue( ":checksum", checksum( bytes ) );
      query.bindValue( ":updated", httpDate( file.lastModified() ) );

      return bytes;
    }
  }
}

using crrc::dao::LogoDAO;

QVariantList LogoDAO::retrieveAll() const
{
  loadLogos();
  return fromLogos();
}

QVariant LogoDAO::retrieve( const uint32_t id ) const
{
  loadLogos();
  const auto iter = logos.find( id );

  return ( iter != logos.end() ) ? asVariant( iter->second.get() ) : QVariant();
}

QByteArray LogoDAO::contents( Cutelyst::Context* context, const uint32_t id ) const
{
  auto query = CPreparedSqlQueryThreadForDB(
    "select image from logos where logo_id = :id",
    crrc::DATABASE_NAME );
  query.bindValue( ":id", id );

  if ( query.exec() )
  {
    query.first();
    return query.value( 0 ).toByteArray();
  }

  qDebug() << query.lastError().text();
  context->stash()["error_msg"] = query.lastError().text();

  return QByteArray();
}


uint32_t LogoDAO::insert( Cutelyst::Context* context ) const
{
  auto query = CPreparedSqlQueryThreadForDB(
    "insert into logos (filename, mimetype, filesize, image, checksum, updated) values (:filename, :mimetype, :filesize, :image, :checksum, :updated)",
    crrc::DATABASE_NAME );
  auto bytes = bindLogo( context, query );

  if ( !query.exec() )
  {
    context->stash()["error_msg"] = query.lastError().text();
    return 0;
  }

  auto id = query.lastInsertId().toUInt();
  auto logo = std::make_unique<Logo>();
  logo->populate( context, bytes );
  logo->setId( id );
  std::lock_guard<std::mutex> lock{ logoMutex };
  logos[id] = std::move( logo );
  return id;
}

uint32_t LogoDAO::update( Cutelyst::Context* context ) const
{
  const auto id = context->request()->param( "id" ).toUInt();
  const auto doc =  context->request()->upload( "image" );

  auto query = CPreparedSqlQueryThreadForDB( 
    "update logos set mimetype=:mimetype, filesize=:filesize, image=:image, checksum=:checksum, updated=:updated where logo_id=:id",
    crrc::DATABASE_NAME );
  auto bytes = bindLogo( context, query );
  query.bindValue( ":id", id );

  if ( query.exec() )
  {
    if ( query.numRowsAffected() )
    {
      std::lock_guard<std::mutex> lock{ logoMutex };
      auto iter = logos.find( id );
      iter->second->populate( context, bytes );
      return query.numRowsAffected();
    }
  }
  else
  {
    context->stash()["error_msg"] = query.lastError().text();
    qDebug() << query.lastError().text();
  }

  return 0;
}

uint32_t LogoDAO::remove( uint32_t id ) const
{
  auto query = CPreparedSqlQueryThreadForDB(
    "delete from logos where logo_id = :id", DATABASE_NAME );
  query.bindValue( ":id", id );
  if ( query.exec() && query.numRowsAffected() )
  {
    std::lock_guard<std::mutex> lock{ logoMutex };
    logos.erase( id );
    return query.numRowsAffected();
  }

  qDebug() << query.lastError().text();
  return 0;
}
