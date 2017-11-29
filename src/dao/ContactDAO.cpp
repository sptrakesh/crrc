#include "ContactDAO.h"
#include "UserDAO.h"
#include "constants.h"
#include "functions.h"
#include "model/Contact.h"
#include "model/User.h"

#include <mutex>
#include <unordered_map>

#include <QtCore/QDebug>
#include <QtCore/QLoggingCategory>
#include <QtCore/QStringBuilder>
#include <QtSql/QtSql>
#include <Cutelyst/Plugins/Utils/sql.h>

using crrc::model::Contact;

Q_LOGGING_CATEGORY( CONTACT_DAO, "crrc.model.ContactDAO" )

namespace crrc
{
  namespace dao
  {
    static std::unordered_map<uint32_t, Contact::Ptr> contacts;
    static std::atomic_bool contactsLoaded{ false };
    static std::mutex contactMutex;

    QVariantList fromContacts()
    {
      QVariantList list;
      for ( const auto& iter : contacts ) list << asVariant( iter.second.get() );
      return list;
    }

    void loadContacts()
    {
      if ( contactsLoaded.load() ) return;
      std::lock_guard<std::mutex> lock{ contactMutex };
      if ( !contacts.empty() )
      {
        contactsLoaded = true;
        return;
      }

      auto query = CPreparedSqlQueryThreadForDB(
        "select contact_id, name, work_email, home_email, other_email, work_phone, mobile_phone, home_phone, other_phone, user_id, institution_id, title, url from contacts order by name",
        DATABASE_NAME );

      if ( query.exec() )
      {
        while ( query.next() )
        {
          auto contact = Contact::create( query );
          contacts[contact->getId()] = std::move( contact );
        }

        contactsLoaded = true;
      }
      else qWarning( CONTACT_DAO ) << query.lastError().text();
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
      query.bindValue( ":title", c->request()->param( "title" ) );
      query.bindValue( ":url", c->request()->param( "url" ) );

      const auto username = c->request()->param( "username" );
      if ( ! username.isEmpty() )
      {
        const UserDAO dao{};
        const auto user = dao.retrieveByUsername( username );
        const auto uptr = model::User::from( user );
        const QVariant userId = ( uptr ) ? uptr->getId() : dao.insert( c );
        query.bindValue( ":uid", userId );

        const auto roleId = c->request()->param( "role" );
        if ( ! roleId.isEmpty() && roleId != "-1" )
        {
          dao.updateRole( userId.toUInt(), roleId.toUInt() );
        }
      }
    }
  }
}

using crrc::dao::ContactDAO;

QVariantList ContactDAO::retrieveAll() const
{
  loadContacts();
  return fromContacts();
}

QVariant ContactDAO::retrieve( const uint32_t id ) const
{
  loadContacts();
  const auto iter = contacts.find( id );
  return ( iter != contacts.end() ) ? asVariant( iter->second.get() ) : QVariant();
}

QVariant ContactDAO::retrieveByUser( uint32_t id ) const
{
  if ( !id ) return QVariant();
  loadContacts();

  for ( const auto& iter : contacts )
  {
    if ( id == iter.second->getUserId() ) return asVariant( iter.second.get() );
  }

  return QVariant();
}

QVariantList ContactDAO::retrieveByInstitution( uint32_t id ) const
{
  QVariantList list;
  if ( !id ) return list;

  loadContacts();

  for ( const auto& iter : contacts )
  {
    if ( id == iter.second->getInstitutionId() ) list << asVariant( iter.second.get() );
  }

  return list;
}

uint32_t ContactDAO::insert( Cutelyst::Context* context ) const
{
  loadContacts();
  QSqlQuery query = CPreparedSqlQueryThreadForDB(
    R"(
insert into contacts
(name, work_email, home_email, other_email, work_phone, mobile_phone, home_phone, other_phone, user_id, institution_id, title, url)
values
(:name, :oe, :he, :oem, :op, :mp, :hp, :oph, :uid, :inst, :title, :url)
)",
    crrc::DATABASE_NAME );
  bindContact( context, query );

  if ( !query.exec() )
  {
    qWarning( CONTACT_DAO ) << query.lastError().text();
    context->stash()["error_msg"] = query.lastError().text();
    return 0;
  }

  const auto id = query.lastInsertId().toUInt();
  auto contact = Contact::create( context );
  contact->setId( id );
  std::lock_guard<std::mutex> lock{ contactMutex };
  contacts[id] = std::move( contact );
  return id;
}

void ContactDAO::update( Cutelyst::Context* context ) const
{
  loadContacts();
  const auto id = context->request()->param( "id" ).toUInt();
  auto query = CPreparedSqlQueryThreadForDB(
    R"(
update contacts 
  set name=:name, work_email=:oe, home_email=:he,
  other_email=:oem, work_phone=:op, mobile_phone=:mp, home_phone=:hp,
  other_phone=:oph, user_id = :uid, institution_id = :inst, title = :title,
  url = :url
where contact_id=:id
)",
    crrc::DATABASE_NAME );
  bindContact( context, query );
  query.bindValue( ":id", id );

  if ( query.exec() )
  {
    if ( query.numRowsAffected() )
    {
      auto contact = Contact::create( context );
      const auto& role = context->request()->param( "role" );
      const uint32_t roleId = ( role == "-1" ) ? 0 : role.toUInt();
      UserDAO().updateRole( contact->getUserId(), roleId );
      std::lock_guard<std::mutex> lock{ contactMutex };
      contacts[id] = std::move( contact );
    }

    context->stash()["count"] = query.numRowsAffected();
  }
  else
  {
    qWarning( CONTACT_DAO ) << query.lastError().text();
    context->stash()["error_msg"] = query.lastError().text();
  }
}

QVariantList ContactDAO::search( Cutelyst::Context* context ) const
{
  loadContacts();

  const bool ignoreCheck = isGlobalAdmin( context );
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
        list << retrieve( query.value( 0 ).toUInt() );
      }
    }
  }
  else
  {
    qWarning( CONTACT_DAO ) << query.lastError().text();
    context->stash()["error_msg"] = query.lastError().text();
  }

  return list;
}

uint32_t ContactDAO::remove( uint32_t id ) const
{
  loadContacts();
  auto query = CPreparedSqlQueryThreadForDB(
    "delete from contacts where contact_id = :id", DATABASE_NAME );
  query.bindValue( ":id", id );
  if ( query.exec() )
  {
    const auto count = query.numRowsAffected();
    const auto iter = contacts.find( id );
    if ( iter != contacts.end() ) UserDAO().remove( iter->second->getUserId() );
    std::lock_guard<std::mutex> lock{ contactMutex };
    contacts.erase( id );
    return count;
  }

  qWarning( CONTACT_DAO ) << query.lastError().text();
  return 0;
}

void ContactDAO::removeInstitution( uint32_t id ) const
{
  auto query = CPreparedSqlQueryThreadForDB(
    "update contacts set institution_id = null where institution_id = :id", DATABASE_NAME );
  query.bindValue( ":id", id );
  if ( query.exec() && query.numRowsAffected() )
  {
    std::lock_guard<std::mutex> lock{ contactMutex };

    for ( const auto& iter : contacts )
    {
      if ( iter.second->getInstitutionId() == id )
      {
        iter.second.get()->setInstitutionId( 0 );
      }
    }
  }
  else qWarning( CONTACT_DAO ) << query.lastError().text();
}