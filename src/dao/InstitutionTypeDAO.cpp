#include "InstitutionTypeDAO.h"
#include "constants.h"
#include "model/InstitutionType.h"

#include <QtCore/QLoggingCategory>
#include <QtCore/QStringBuilder>
#include <QtSql/QtSql>
#include <Cutelyst/Plugins/Utils/sql.h>

#include <unordered_map>
#include <mutex>

using crrc::model::InstitutionType;

Q_LOGGING_CATEGORY( INSTITUTION_TYPE_DAO, "crrc.dao.InstitutionTypeDAO" )

namespace crrc
{
  namespace dao
  {
    static std::unordered_map<uint32_t, InstitutionType::Ptr> instTypes;
    static std::atomic_bool instTypesLoaded{ false };
    static std::mutex instTypeMutex;

    QVariantList fromInstitutionTypes()
    {
      QVariantList list;
      for ( const auto& iter : instTypes ) list << asVariant( iter.second.get() );

      return list;
    }

    void loadInstitutionTypes()
    {
      if ( instTypesLoaded.load() ) return;
      std::lock_guard<std::mutex> lock{ instTypeMutex };
      if ( !instTypes.empty() )
      {
        instTypesLoaded = true;
        return;
      }

      auto query = CPreparedSqlQueryThreadForDB(
        "select institution_type_id, name from institution_types order by institution_type_id",
        DATABASE_NAME );

      if ( query.exec() )
      {
        while ( query.next() )
        {
          auto instType = InstitutionType::create( query );
          instTypes[instType->getId()] = std::move( instType );
        }

        instTypesLoaded = true;
      }
      else
      {
        qWarning( INSTITUTION_TYPE_DAO ) << query.lastError().text();
      }
    }
  }
}

using crrc::dao::InstitutionTypeDAO;

const QVariantList InstitutionTypeDAO::retrieveAll() const
{
  loadInstitutionTypes();
  return fromInstitutionTypes();
}

const QVariant InstitutionTypeDAO::retrieve( uint32_t id ) const
{
  loadInstitutionTypes();
  const auto iter = instTypes.find( id );
  return ( iter != instTypes.end() ) ? asVariant( iter->second.get() ) : QVariant();
}
