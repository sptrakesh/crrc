#include "RoleDAO.h"
#include "constants.h"

#include <mutex>

#include <QtSql/QtSql>
#include <Cutelyst/Plugins/Utils/sql.h>

namespace crrc
{
  namespace dao
  {
    struct Role
    {
      QVariant id;
      QVariant role;
    };

    static QMap<uint32_t, Role> roles;
    static std::atomic_bool rolesLoaded{ false };
    static std::mutex roleMutex;

    QVariantHash transform( const Role& role )
    {
      QVariantHash record;
      record.insert( "role_id", role.id );
      record.insert( "role", role.role );

      return record;
    }

    QVariantList fromRoles()
    {
      QVariantList list;
      foreach ( Role role, roles )
      {
        list.append( transform( role ) );
      }

      return list;
    }

    Role createRole( QSqlQuery& query )
    {
      Role role;
      role.id = query.value( 0 ).toUInt();
      role.role = query.value( 1 );
      return role;
    }

    void loadRoles()
    {
      if ( rolesLoaded.load() ) return;
      std::lock_guard<std::mutex> lock{ roleMutex };
      if ( !roles.isEmpty() )
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
          const auto role = createRole( query );
          auto id = role.id.toUInt();
          roles.insert( id, std::move( role ) );
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

QVariantHash RoleDAO::retrieve( const QString& id ) const
{
  loadRoles();
  const auto cid = id.toUInt();
  const auto iter = roles.constFind( cid );

  return ( iter != roles.end() ) ? 
    transform( iter.value() ) : QVariantHash();
}

QVariantHash RoleDAO::retrieveByRole( const QString& name ) const
{
  loadRoles();
  for ( const auto role : roles.values() )
  {
    if ( role.role == name ) return transform( role );
  }

  return QVariantHash();
}
