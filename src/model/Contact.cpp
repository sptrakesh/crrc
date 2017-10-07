﻿#include "Contact.h"
#include "dao/UserDAO.h"
#include "User.h"

#include <QtCore/QDebug>
#include "dao/InstitutionDAO.h"

using crrc::model::Contact;

Contact::Ptr Contact::create( QSqlQuery& query )
{
  auto ptr = std::make_unique<Contact>();
  ptr->id = query.value( 0 ).toUInt();
  ptr->name = query.value( 1 ).toString();
  ptr->workEmail = query.value( 2 ).toString();
  ptr->homeEmail = query.value( 3 ).toString();
  ptr->otherEmail = query.value( 4 ).toString();
  ptr->workPhone = query.value( 5 ).toString();
  ptr->mobilePhone = query.value( 6 ).toString();
  ptr->homePhone = query.value( 7 ).toString();
  ptr->otherPhone = query.value( 8 ).toString();
  ptr->userId = query.value( 9 ).toUInt();
  ptr->institutionId = query.value( 10 ).toUInt();
  return ptr;
}

Contact::Ptr Contact::create( Cutelyst::Context* context )
{
  auto ptr = std::make_unique<Contact>();

  const auto& id = context->request()->param( "id" );
  if ( !id.isEmpty() ) ptr->id = id.toUInt();
  
  ptr->name = context->request()->param( "name" );
  ptr->workEmail = context->request()->param( "workEmail" );
  ptr->homeEmail = context->request()->param( "homeEmail" );
  ptr->otherEmail = context->request()->param( "otherEmail" );
  ptr->workPhone = context->request()->param( "workPhone" );
  ptr->mobilePhone = context->request()->param( "mobilePhone" );
  ptr->homePhone = context->request()->param( "homePhone" );
  ptr->otherPhone = context->request()->param( "otherPhone" );
  ptr->institutionId = context->request()->param( "institution" ).toUInt();

  const auto username = context->request()->param( "username" );
  if ( !username.isEmpty() )
  {
    const auto user = dao::UserDAO().retrieveByUsername( username );
    auto uptr = const_cast<model::User*>( model::User::from( user ) );
    if ( uptr )
    {
      ptr->userId = uptr->getId();
      const auto role = context->request()->param( "role" );
      uptr->setRoleId( ( role == "-1" ) ? 0 : role.toUInt() );
    }
    else qDebug() << "No user with username: " << username;
  }

  return ptr;
}

QVariant Contact::getUser() const
{
  return ( userId > 0 ) ? dao::UserDAO().retrieve( userId ) : QVariant();
}

QVariant Contact::getInstitution() const
{
  return ( institutionId > 0 ) ? dao::InstitutionDAO().retrieve( institutionId ) : QVariant();
}
