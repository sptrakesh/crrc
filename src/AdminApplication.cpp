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
#include "Institutions.h"
#include "Degrees.h"
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
  new Institutions( this );
  new Degrees( this );
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
#if defined (_WINDOWS)
  auto db = QSqlDatabase::addDatabase( "QSQLITE",
    Cutelyst::Sql::databaseNameThread( DATABASE_NAME ) );
  db.setDatabaseName( "crrc.db" );
  db.setConnectOptions( "foreign_keys = on" );
  if ( !db.open() )
  {
    qCritical() << "Failed to open database:" << db.lastError().text();
    return false;
  }
#else
  auto db = QSqlDatabase::addDatabase( "QPSQL",
    Cutelyst::Sql::databaseNameThread( DATABASE_NAME ) );
  if ( !db.open() )
  {
    qCritical() << "Failed to open database:" << db.lastError().text();
    return false;
  }
#endif
  return true;
}
