#pragma once

#include <memory>

#include <QtSql/QSqlQuery>
#include <Cutelyst/Context>

namespace crrc
{
  namespace model
  {
    class User : public QObject
    {
      Q_OBJECT
      Q_PROPERTY( QVariant id READ getUserId )
      Q_PROPERTY( QVariant username READ getUsername )
      Q_PROPERTY( QVariant password READ getPassword )
      Q_PROPERTY( QVariant email READ getEmail )
      Q_PROPERTY( QVariant firstName READ getFirstName )
      Q_PROPERTY( QVariant lastName READ getLastName )
      Q_PROPERTY( QVariant middleName READ getMiddleName )
      Q_PROPERTY( QVariant role READ getRole )

    public:
      using Ptr = std::unique_ptr<User>;
      static Ptr create( QSqlQuery& query );
      static Ptr create( Cutelyst::Context* context );

      static const User* from( const QVariant& variant )
      {
        return qvariant_cast<User*>( variant );
      }

      explicit User( QObject* parent = nullptr ) : QObject( parent ),
        id{ 0 }, roleId{ 0 } {}
      ~User() = default;

      uint32_t getId() const { return id; }
      QVariant getUserId() const { return id; }

      User& setId( uint32_t id )
      {
        this->id = id;
        return *this;
      }

      const QString& getUsername() const { return username; }

      const QVariant& getPassword() const { return password; }
      User& setPassword( const QVariant& password )
      {
        this->password = password;
        return *this;
      }

      const QString& getEmail() const { return email; }
      const QString& getFirstName() const { return firstName; }
      const QString& getLastName() const { return lastName; }
      const QString& getMiddleName() const { return middleName; }

      uint32_t getRoleId() const { return roleId; }

      User& setRoleId( const uint32_t roleId )
      {
        this->roleId = roleId;
        return *this;
      }

      const QVariant getRole() const;

      operator QString() const;

    private:
      uint32_t id;
      QString username;
      QVariant password;
      QString email;
      QString firstName;
      QString lastName;
      QString middleName;
      uint32_t roleId;
    };

    inline QVariant asVariant( const User* user )
    {
      return QVariant::fromValue<QObject*>( const_cast<User*>( user ) );
    }
  }
}
