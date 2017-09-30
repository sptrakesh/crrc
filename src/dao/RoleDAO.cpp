#include "RoleDAO.h"
#include "constants.h"
#include "model/Role.h"

#include <mutex>
#include <unordered_map>

#include <QtSql/QtSql>
#include <Cutelyst/Plugins/Utils/sql.h>

using crrc::model::Role;

namespace crrc
{
  namespace dao
  {
    static std::unordered_map<uint32_t, Role::Ptr> roles;
    static std::atomic_bool rolesLoaded{ false };
    static std::mutex roleMutex;

    QVariantList fromRoles()
    {
      QVariantList list;
      for ( const auto& iter : roles ) list << asVariant( iter.second.get() );
      return list;
    }

    void loadRoles()
    {
      if ( rolesLoaded.load() ) return;
      std::lock_guard<std::mutex> lock{ roleMutex };
      if ( !roles.empty() )
      {
        rolesLoaded = true;
        return;
      }

      auto query = CPreparedSqlQueryThreadForDB(
        "select role_id, role from roles order by role", DATABASE_NAME );

      if ( query.exec() )
      {
        while ( query.next() )
        {
          auto role = Role::create( query );
          roles[role->getId()] = std::move( role );
        }

        rolesLoaded = true;
      }
    }
  }
}

using crrc::dao::RoleDAO;

QVariantList RoleDAO::retrieveAll() const
{
  loadRoles();
  return fromRoles();
}

QVariant RoleDAO::retrieve( const uint32_t id ) const
{
  loadRoles();
  const auto iter = roles.find( id );
  return ( iter != roles.end() ) ? asVariant( iter->second.get() ) : QVariant();
}

QVariant RoleDAO::retrieveByRole( const QString& name ) const
{
  loadRoles();
  for ( const auto& iter : roles )
  {
    if ( iter.second->getRole() == name ) return asVariant( iter.second.get() );
  }

  return QVariant();
}
