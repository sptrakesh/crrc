#pragma once

#include <memory>
#include <QtSql/QSqlQuery>
#include <Cutelyst/Context>

namespace crrc
{
  namespace model
  {
    class Contact : public QObject
    {
      Q_OBJECT
      Q_PROPERTY( QVariant id READ getContactId )
      Q_PROPERTY( QString name READ getName )
      Q_PROPERTY( QString workEmail READ getWorkEmail )
      Q_PROPERTY( QString homeEmail READ getHomeEmail )
      Q_PROPERTY( QString otherEmail READ getOtherEmail )
      Q_PROPERTY( QString workPhone READ getWorkPhone )
      Q_PROPERTY( QString homePhone READ getHomePhone )
      Q_PROPERTY( QString otherPhone READ getOtherPhone )
      Q_PROPERTY( QVariant user READ getUser )
      Q_PROPERTY( QVariant institution READ getInstitution )

    public:
      using Ptr = std::unique_ptr<Contact>;
      static Ptr create( QSqlQuery& query );
      static Ptr create( Cutelyst::Context* context );

      explicit Contact( QObject* parent = nullptr ) : QObject( parent ) {}
      ~Contact() = default;

      uint32_t getId() const { return id; }
      QVariant getContactId() const { return QVariant{ id }; }

      Contact& setId( uint32_t id )
      {
        this->id = id;
        return *this;
      }

      const QString& getName() const { return name; }
      const QString& getWorkEmail() const { return workEmail; }
      const QString& getHomeEmail() const { return homeEmail; }
      const QString& getOtherEmail() const { return otherEmail; }
      const QString& getWorkPhone() const { return workPhone; }
      const QString& getHomePhone() const { return homePhone; }
      const QString& getOtherPhone() const { return otherPhone; }

      uint32_t getUserId() const { return userId; }
      QVariant getUser() const;

      uint32_t getInstitutionId() const { return institutionId; }
      QVariant getInstitution() const;

      Contact& setInstitutionId( const uint32_t id )
      {
        institutionId = id;
        return *this;
      }

    private:
      uint32_t id = 0;
      QString name;
      QString workEmail;
      QString homeEmail;
      QString otherEmail;
      QString workPhone;
      QString mobilePhone;
      QString homePhone;
      QString otherPhone;
      uint32_t userId = 0;
      uint32_t institutionId = 0;
    };

    inline QVariant asVariant( const Contact* contact )
    {
      return QVariant::fromValue<QObject*>( const_cast<Contact*>( contact ) );
    }
  }
}