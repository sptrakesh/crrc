#include "AdminApplication.h"
#include "constants.h"

#include <Cutelyst/Plugins/StaticSimple/staticsimple.h>
#include <Cutelyst/Plugins/View/Grantlee/grantleeview.h>
#include <Cutelyst/Plugins/Utils/Sql>
#include <Cutelyst/Plugins/Session/Session>
#include <Cutelyst/Plugins/Authentication/authentication.h>
#include <Cutelyst/Plugins/Authentication/credentialpassword.h>

#include <QtSql>
#include <QDebug>

#include "Root.h"
#include "Login.h"
#include "Contacts.h"
#include "Logos.h"
#include "Institutions.h"
#include "InstitutionDesignations.h"
#include "Degrees.h"
#include "Departments.h"
#include "Programs.h"
#include "Agreements.h"
#include "InstitutionAgreements.h"
#include "Logout.h"

#include "AuthStoreSql.h"

using crrc::AdminApplication;

bool AdminApplication::init()
{
  new Root( this );
  new Login( this );
  new Contacts( this );
  new Logos( this );
  new Institutions( this );
  new InstitutionDesignations( this );
  new Degrees( this );
  new Departments( this );
  new Programs( this );
  new Agreements( this );
  new InstitutionAgreements( this );
  new Cutelyst::StaticSimple( this );
  new Logout( this );

  auto view = new Cutelyst::GrantleeView( this );
  view->setIncludePaths( { pathTo( { "root", "src" } ) } );
  view->setWrapper( "wrapper.html" );

  new Cutelyst::Session( this );

  auto auth = new Cutelyst::Authentication( this );
  auto credential = new Cutelyst::CredentialPassword;
  credential->setPasswordType( Cutelyst::CredentialPassword::Clear );

  auth->addRealm( new AuthStoreSql, credential );

  return true;
}

bool AdminApplication::postFork()
{
  auto db = QSqlDatabase::addDatabase( "QSQLITE",
    Cutelyst::Sql::databaseNameThread( DATABASE_NAME ) );
  db.setDatabaseName( "var/crrc.db" );
  db.setConnectOptions( "foreign_keys = on" );
  if ( !db.open() )
  {
    qCritical() << "Failed to open database:" << db.lastError().text();
    return false;
  }

  QSqlQuery query( db );
  db.exec( "PRAGMA foreign_keys = ON;" );
  return true;
}
