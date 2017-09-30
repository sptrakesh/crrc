#include "DesignationDAO.h"
#include "constants.h"
#include "model/Designation.h"

#include <mutex>
#include <QtSql/QtSql>
#include <Cutelyst/Plugins/Utils/sql.h>
#include <unordered_map>

using crrc::model::Designation;

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
