#pragma once

#include <memory>
#include <QtSql/QSqlQuery>
#include <Cutelyst/Context>

namespace crrc
{
  namespace model
  {
    class Role : public QObject
    {
      Q_OBJECT
      Q_PROPERTY( QVariant id READ getRoleId )
      Q_PROPERTY( QString role READ getRole )

    public:
      using Ptr = std::unique_ptr<Role>;
      static Ptr create( QSqlQuery& query );
      static Ptr create( Cutelyst::Context* context );

      explicit Role( QObject* parent = nullptr ) : QObject( parent ), id{ 0 } {}
      ~Role() = default;

      uint32_t getId() const { return id; }
      QVariant getRoleId() const { return QVariant{ id }; }

      Role& setId( const uint32_t id )
      {
        this->id = id;
        return *this;
      }

      const QString& getRole() const { return role; }

      operator QString() const;

    private:
      uint32_t id;
      QString role;
    };

    inline QVariant asVariant( const Role* role )
    {
      return QVariant::fromValue<QObject*>( const_cast<Role*>( role ) );
    }
  }
}
