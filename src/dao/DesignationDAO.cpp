#include "DesignationDAO.h"
#include "constants.h"

#include <mutex>
#include <QtSql/QtSql>
#include <Cutelyst/Plugins/Utils/sql.h>

namespace crrc
{
  namespace dao
  {
    struct Designation
    {
      QVariant id;
      QVariant type;
      QVariant title;
    };

    static QMap<uint32_t, Designation> designations;
    static std::atomic_bool designationsLoaded{ false };
    static std::mutex designationMutex;

    QVariantHash transform( const Designation& designation )
    {
      QVariantHash record;
      record.insert( "designation_id", designation.id );
      record.insert( "type", designation.type );
      record.insert( "title", designation.title );
      return record;
    }

    QVariantList fromDesignations()
    {
      QVariantList list;
      foreach ( Designation designation, designations )
      {
        list.append( transform( designation ) );
      }

      return list;
    }

    Designation createDesignation( QSqlQuery& query )
    {
      Designation designation;
      designation.id = query.value( 0 ).toUInt();
      designation.type = query.value( 1 );
      designation.title = query.value( 2 );
      return designation;
    }

    void loadDesignations()
    {
      if ( designationsLoaded.load() ) return;
      std::lock_guard<std::mutex> lock{ designationMutex };
      if ( !designations.isEmpty() )
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
          const auto designation = createDesignation( query );
          auto id = designation.id.toUInt();
          designations.insert( id, std::move( designation ) );
        }

        designationsLoaded = true;
      }
    }
  }
}

using crrc::dao::DesignationDAO;

QVariantList DesignationDAO::retrieveAll() const
{
  loadDesignations();
  return fromDesignations();
}

QVariantHash DesignationDAO::retrieve( const QString& id ) const
{
  loadDesignations();
  const uint32_t cid = id.toUInt();
  const auto iter = designations.constFind( cid );

  return ( iter != designations.end() ) ? transform( iter.value() ) : QVariantHash();
}

QVariantList DesignationDAO::retrieveByType( const QString& type ) const
{
  loadDesignations();
  QVariantList list;

  for ( const auto& designation : designations )
  {
    if ( type == designation.type.toString() ) list << transform( designation );
  }

  return list;
}
