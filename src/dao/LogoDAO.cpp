#include "LogoDAO.h"
#include "constants.h"
#include "functions.h"

#include <mutex>
#include <QtSql/QtSql>
#include <Cutelyst/Upload>
#include <Cutelyst/Plugins/Utils/sql.h>


namespace crrc
{
  namespace dao
  {
    struct Logo
    {
      QVariant id;
      QVariant filename;
      QVariant mimetype;
      QVariant filesize;
      QVariant image;
      QVariant checksum;
      QVariant updated;
    };

    static QMap<uint32_t, Logo> logos;
    static std::atomic_bool logosLoaded{ false };
    static std::mutex logoMutex;

    QVariantHash transform( const Logo& logo, const LogoDAO::Mode& mode )
    {
      QVariantHash record;
      record.insert( "id", logo.id );
      record.insert( "filename", logo.filename );

      if ( LogoDAO::Mode::Full == mode )
      {
        record.insert( "mimetype", logo.mimetype );
        record.insert( "filesize", logo.filesize );
        record.insert( "checksum", logo.checksum );
        record.insert( "updated", logo.updated );
        record.insert( "image", logo.image );
      }

      return record;
    }

    QVariantList fromLogos( const LogoDAO::Mode& mode )
    {
      QVariantList list;
      foreach ( Logo logo, logos )
      {
        list.append( transform( logo, mode ) );
      }

      return list;
    }

    Logo createLogo( QSqlQuery& query )
    {
      Logo logo;
      logo.id = query.value( 0 ).toUInt();
      logo.filename = query.value( 1 );
      logo.mimetype = query.value( 2 );
      logo.filesize = query.value( 3 );
      logo.checksum = query.value( 4 );
      logo.updated = query.value( 5 );
      return logo;
    }

    void loadLogos()
    {
      if ( logosLoaded.load() ) return;
      std::lock_guard<std::mutex> lock{ logoMutex };
      if ( !logos.isEmpty() )
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
          const auto logo = createLogo( query );
          auto id = logo.id.toUInt();
          logos.insert( id, std::move( logo ) );
        }

        logosLoaded = true;
      }
    }

    QByteArray bindLogo( Cutelyst::Context* c, QSqlQuery& query )
    {
      auto upload = c->request()->upload( "image" );
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

    Logo logoFromContext( Cutelyst::Context* context, const QByteArray& bytes )
    {
      Logo logo;
      const auto upload = context->request()->upload( "image" );
      const QFileInfo file{ upload->filename() };
      logo.filename = file.fileName();
      logo.mimetype = upload->contentType();
      logo.filesize = upload->size();
      logo.checksum = checksum( bytes );
      logo.updated = httpDate( file.lastModified() );
      return logo;
    }
  }
}

using crrc::dao::LogoDAO;

QVariantList LogoDAO::retrieveAll( const Mode& mode ) const
{
  loadLogos();
  return fromLogos( mode );
}

QVariantHash LogoDAO::retrieve( const QString& id, const Mode& mode ) const
{
  loadLogos();
  uint32_t cid = id.toUInt();
  const auto iter = logos.constFind( cid );

  return ( iter != logos.end() ) ? 
    transform( iter.value(), mode ) : QVariantHash();
}

QByteArray LogoDAO::contents( Cutelyst::Context* context, const QString& id ) const
{
  auto query = CPreparedSqlQueryThreadForDB(
    "select image from logos where logo_id = :id",
    crrc::DATABASE_NAME );
  query.bindValue( ":id", id.toUInt() );

  if ( query.exec() )
  {
    query.first();
    return query.value( 0 ).toByteArray();
  }

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
  auto logo = logoFromContext( context, bytes );
  logo.id = id;
  std::lock_guard<std::mutex> lock{ logoMutex };
  logos[id] = std::move( logo );
  return id;
}

void LogoDAO::update( Cutelyst::Context* context ) const
{
  const auto id = context->request()->param( "id" );
  const auto doc =  context->request()->upload( "image" );

  auto query = CPreparedSqlQueryThreadForDB( 
    "update logos set mimetype=:mimetype, filesize=:filesize, image=:image, checksum=:checksum, updated=:updated where logo_id=:id",
    crrc::DATABASE_NAME );
  auto bytes = bindLogo( context, query );
  query.bindValue( ":id", id.toInt() );

  if ( query.exec() )
  {
    auto logo = logoFromContext( context, bytes );
    auto aid = id.toUInt();
    logo.id = aid;

    auto old = logos.find( aid ).value();
    logo.filename = old.filename;

    std::lock_guard<std::mutex> lock{ logoMutex };
    logos[aid] = std::move( logo );
  }
  else context->stash()["error_msg"] = query.lastError().text();
}

QString LogoDAO::remove( uint32_t id ) const
{
  auto query = CPreparedSqlQueryThreadForDB(
    "delete from logos where logo_id = :id", DATABASE_NAME );
  query.bindValue( ":id", id );
  if ( query.exec() )
  {
    std::lock_guard<std::mutex> lock{ logoMutex };
    logos.remove( id );
    return "Logo deleted.";
  }

  return query.lastError().text();
}
