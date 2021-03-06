#include "Login.h"
#include "dao/functions.h"
#include "dao/UserDAO.h"

#include <QtCore/QCryptographicHash>
#include <QtCore/QJsonObject>
#include <Cutelyst/Plugins/Authentication/authentication.h>

using crrc::Login;

Login::Login( QObject* parent ) : Controller( parent ) {}

Login::~Login() {}

void Login::index( Cutelyst::Context* c )
{
  if ( "PUT" == c->request()->method() ) return service( c );

  auto username = c->request()->param( "username" );
  auto password = c->request()->param( "password" );

  if ( !username.isNull() && !password.isNull() )
  {
    if ( login( c ) )
    {
      const auto& url = c->request()->cookie( "url" );
      c->response()->redirect( ( url.isEmpty() ? "/contacts" : url ) );
      return;
    }

    c->setStash( "error_msg", "Incorrect username or password." );
  }

  c->setStash( "template", "login.html" );
}

bool Login::plainText( Cutelyst::Context* c )
{
  using Cutelyst::Authentication;

  auto username = c->request()->param( "username" );
  auto password = c->request()->param( "password" );
  const auto result = Authentication::authenticate( c, { { "username", username }, { "password", password } } );
  if ( result ) dao::UserDAO().updatePassword( username, password );
  return result;
}

bool Login::hashed( Cutelyst::Context* c )
{
  using Cutelyst::Authentication;

  auto username = c->request()->param( "username" );

  QCryptographicHash hash{ QCryptographicHash::Sha3_512 };
  hash.addData( c->request()->param( "password" ).toLocal8Bit() );
  const auto passwd = hash.result().toHex();
  return Authentication::authenticate( c, { { "username", username }, { "password", passwd } } );
}

bool Login::login( Cutelyst::Context* c )
{
  auto username = c->request()->param( "username" );
  auto password = c->request()->param( "password" );

  if ( !username.isNull() && !password.isNull() )
  {
    return ( hashed( c ) || plainText( c ) );
  }

  return Cutelyst::Authentication::userExists( c );
}

void Login::service( Cutelyst::Context* c )
{
  QJsonObject json;
  json.insert( "username", c->request()->param( "username" ) );
  json.insert( "status", login( c ) );
  dao::sendJson( c, json );
}
