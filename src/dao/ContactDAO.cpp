#include "ContactDAO.h"
#include "UserDAO.h"
#include "InstitutionDAO.h"
#include "constants.h"

#include <mutex>
#include <QtCore/QDebug>
#include <QtCore/QStringBuilder>
#include <QtSql/QtSql>
#include <Cutelyst/Plugins/Utils/sql.h>
#include "functions.h"
#include "model/User.h"


namespace crrc
{
  namespace dao
  {
    struct Contact
    {
      QVariant id;
      QVariant name;
      QVariant workEmail;
      QVariant homeEmail;
      QVariant otherEmail;
      QVariant workPhone;
      QVariant mobilePhone;
      QVariant homePhone;
      QVariant otherPhone;
      QVariant userId;
      QVariant institutionId;
    };

    static QMap<uint32_t, Contact> contacts;
    static std::atomic_bool contactsLoaded{ false };
    static std::mutex contactMutex;

    QVariantHash transform( const Contact& contact, const ContactDAO::Mode& mode )
    {
      QVariantHash record;
      record.insert( "contact_id", contact.id );
      record.insert( "name", contact.name );
      record.insert( "work_email", contact.workEmail );

      if ( contact.userId.isValid() )
      {
        record.insert( "user", UserDAO().retrieve( contact.userId.toUInt() ) );
      }

      if ( contact.institutionId.isValid() )
      {
        record.insert( "institution", InstitutionDAO().retrieve(
          contact.institutionId.toString(), InstitutionDAO::Mode::Partial ) );
      }

      if ( ContactDAO::Mode::Full == mode )
      {
        record.insert( "home_email", contact.homeEmail );
        record.insert( "other_email", contact.otherEmail );
        record.insert( "work_phone", contact.workPhone );
        record.insert( "mobile_phone", contact.mobilePhone );
        record.insert( "home_phone", contact.homePhone );
        record.insert( "other_phone", contact.otherPhone );
      }

      return record;
    }

    QVariantList fromContacts( const ContactDAO::Mode& mode )
    {
      QVariantList list;
      foreach ( Contact contact, contacts )
      {
        qDebug() << "Adding contact: " << contact.id;
        list.append( transform( contact, mode ) );
      }

      qDebug() << "Loaded contacts: " << list;
      return list;
    }

    Contact createContact( QSqlQuery& query )
    {
      Contact contact;
      contact.id = query.value( 0 ).toUInt();
      contact.name = query.value( 1 );
      contact.workEmail = query.value( 2 );
      contact.homeEmail = query.value( 3 );
      contact.otherEmail = query.value( 4 );
      contact.workPhone = query.value( 5 );
      contact.mobilePhone = query.value( 6 );
      contact.homePhone = query.value( 7 );
      contact.otherPhone = query.value( 8 );
      contact.userId = query.value( 9 ).toUInt();
      contact.institutionId = query.value( 10 ).toUInt();
      qDebug() << "Created contact with id: " << contact.id;
      return contact;
    }

    void loadContacts()
    {
      if ( contactsLoaded.load() ) return;
      std::lock_guard<std::mutex> lock{ contactMutex };
      if ( !contacts.isEmpty() )
      {
        contactsLoaded = true;
        return;
      }

      auto query = CPreparedSqlQueryThreadForDB(
        "select contact_id, name, work_email, home_email, other_email, work_phone, mobile_phone, home_phone, other_phone, user_id, institution_id from contacts order by name",
        DATABASE_NAME );

      if ( query.exec() )
      {
        while ( query.next() )
        {
          const auto contact = createContact( query );
          auto id = contact.id.toUInt();
          contacts.insert( id, std::move( contact ) );
        }

        contactsLoaded = true;
      }
    }

    void bindContact( Cutelyst::Context* c, QSqlQuery& query )
    {
      query.bindValue( ":name", c->request()->param( "name" ) );
      query.bindValue( ":oe", c->request()->param( "workEmail" ) );
      query.bindValue( ":he", c->request()->param( "homeEmail" ) );
      query.bindValue( ":oem", c->request()->param( "otherEmail" ) );
      query.bindValue( ":op", c->request()->param( "workPhone" ) );
      query.bindValue( ":mp", c->request()->param( "mobilePhone" ) );
      query.bindValue( ":hp", c->request()->param( "homePhone" ) );
      query.bindValue( ":oph", c->request()->param( "otherPhone" ) );
      query.bindValue( ":inst", c->request()->param( "institution" ) );

      const auto username = c->request()->param( "username" );
      if ( ! username.isEmpty() )
      {
        const UserDAO dao;
        const auto user = dao.retrieveByUsername( username );
        const auto uptr = qvariant_cast<model::User*>( user );
        const QVariant userId = ( uptr ) ? uptr->getId() : dao.insert( c );
        query.bindValue( ":uid", userId );

        const auto roleId = c->request()->param( "role" );
        if ( ! roleId.isEmpty() && roleId != "-1" )
        {
          dao.updateRole( userId.toUInt(), roleId.toUInt() );
        }
      }
    }

    Contact contactFromContext( Cutelyst::Context* context )
    {
      Contact contact;
      contact.name = context->request()->param( "name" );
      contact.workEmail = context->request()->param( "workEmail" );
      contact.homeEmail = context->request()->param( "homeEmail" );
      contact.otherEmail = context->request()->param( "otherEmail" );
      contact.workPhone = context->request()->param( "workPhone" );
      contact.mobilePhone = context->request()->param( "mobilePhone" );
      contact.homePhone = context->request()->param( "homePhone" );
      contact.otherPhone = context->request()->param( "otherPhone" );
      contact.institutionId = context->request()->param( "institution" );

      const auto username = context->request()->param( "username" );
      if ( !username.isEmpty() )
      {
        const auto user = UserDAO().retrieveByUsername( username );
        const auto uptr = qvariant_cast<model::User*>( user );
        if ( uptr ) contact.userId = uptr->getUserId();
        else qDebug() << "No user with username: " << username;
      }

      return contact;
    }
  }
}

using crrc::dao::ContactDAO;

QVariantList ContactDAO::retrieveAll( const Mode& mode ) const
{
  loadContacts();
  return fromContacts( mode );
}

QVariantHash ContactDAO::retrieve( const QString& id, const Mode& mode ) const
{
  loadContacts();
  const auto cid = id.toUInt();
  const auto iter = contacts.constFind( cid );

  return ( iter != contacts.end() ) ? 
    transform( iter.value(), mode ) : QVariantHash();
}

QVariantHash ContactDAO::retrieveByUser( uint32_t id, const Mode& mode ) const
{
  loadContacts();

  for ( const auto& contact : contacts )
  {
    qDebug() << "Checking contact userId: " << contact.userId.toUInt() << " against specified id: " << id;
    if ( id == contact.userId.toUInt() ) return transform( contact, mode );
  }

  return QVariantHash();
}

QVariantList ContactDAO::retrieveByInstitution( uint32_t id, const Mode& mode ) const
{
  QVariantList list;
  if ( !id ) return list;

  loadContacts();

  for ( const auto& contact : contacts )
  {
    if ( id == contact.institutionId.toUInt() ) list << transform( contact, mode );
  }

  return list;
}

uint32_t ContactDAO::insert( Cutelyst::Context* context ) const
{
  loadContacts();
  QSqlQuery query = CPreparedSqlQueryThreadForDB(
    "insert into contacts (name, work_email, home_email, other_email, work_phone, mobile_phone, home_phone, other_phone, user_id, institution_id) values (:name, :oe, :he, :oem, :op, :mp, :hp, :oph, :uid, :inst)",
    crrc::DATABASE_NAME );
  bindContact( context, query );

  if ( !query.exec() )
  {
    context->stash()["error_msg"] = query.lastError().text();
    return 0;
  }
  else
  {
    const auto id = query.lastInsertId().toUInt();
    auto contact = contactFromContext( context );
    contact.id = id;
    std::lock_guard<std::mutex> lock{ contactMutex };
    contacts[id] = std::move( contact );
    return id;
  }
}

void ContactDAO::update( Cutelyst::Context* context ) const
{
  loadContacts();
  auto id = context->request()->param( "id" );
  auto query = CPreparedSqlQueryThreadForDB(
    "update contacts set name=:name, work_email=:oe, home_email=:he, other_email=:oem, work_phone=:op, mobile_phone=:mp, home_phone=:hp, other_phone=:oph, user_id = :uid, institution_id = :inst where contact_id=:id",
    crrc::DATABASE_NAME );
  bindContact( context, query );
  query.bindValue( ":id", id.toInt() );

  if ( query.exec() )
  {
    auto contact = contactFromContext( context );
    contact.id = id.toUInt();
    std::lock_guard<std::mutex> lock{ contactMutex };
    contacts[id.toUInt()] = std::move( contact );
  }
  else context->stash()["error_msg"] = query.lastError().text();
}

QVariantList ContactDAO::search( Cutelyst::Context* context, const Mode& mode ) const
{
  loadContacts();

  bool ignoreCheck = isGlobalAdmin( context );
  const uint32_t iid = ( ignoreCheck ) ? 0 : institutionId( context );

  const auto text = context->request()->param( "text", "" );
  const QString clause = "%" % text % "%";

  auto query = CPreparedSqlQueryThreadForDB(
   "select contact_id, institution_id from contacts where name like :text order by name",
    DATABASE_NAME );

  query.bindValue( ":text", clause );
  QVariantList list;

  if ( query.exec() )
  {
    while ( query.next() )
    {
      const auto qid = query.value( 1 ).toUInt();
      if ( ignoreCheck || qid == iid )
      {
        auto contact = retrieve( query.value( 0 ).toString(), mode );
        list.append( contact );
      }
    }
  }
  else
  {
    qWarning() << query.lastError().text();
    context->stash()["error_msg"] = query.lastError().text();
  }

  return list;
}

QString ContactDAO::remove( uint32_t id ) const
{
  loadContacts();
  auto query = CPreparedSqlQueryThreadForDB(
    "delete from contacts where contact_id = :id", DATABASE_NAME );
  query.bindValue( ":id", id );
  if ( query.exec() )
  {
    const auto iter = contacts.constFind( id );
    if ( iter != contacts.end() ) UserDAO().remove( iter.value().userId.toUInt() );
    std::lock_guard<std::mutex> lock{ contactMutex };
    contacts.remove( id );
    return "Contact deleted.";
  }
  else return query.lastError().text();
}

void ContactDAO::removeInstitution( uint32_t id ) const
{
  auto query = CPreparedSqlQueryThreadForDB(
    "update contacts set institution_id = null where institution_id = :id", DATABASE_NAME );
  query.bindValue( ":id", id );
  if ( query.exec() )
  {
    std::lock_guard<std::mutex> lock{ contactMutex };

    for ( auto& contact : contacts )
    {
      if ( contact.institutionId == id )
      {
        contact.institutionId = QVariant( QVariant::UInt );
      }
    }
  }
  else qDebug() << query.lastError().text();
}