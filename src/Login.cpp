#include "Login.h"

#include <Cutelyst/Plugins/Authentication/authentication.h>

using crrc::Login;

Login::Login( QObject* parent ) : Controller( parent ) {}

Login::~Login() {}

void Login::index( Cutelyst::Context* c )
{
  using Cutelyst::Authentication;

  auto username = c->request()->param( "username" );
  auto password = c->request()->param( "password" );

  if ( !username.isNull() && !password.isNull() )
  {
    if ( Authentication::authenticate( c, { { "username", username }, { "password", password } } ) )
    {
      //c->response()->redirect( c->request()->referer() );
      c->response()->redirect( "/contacts" );
      return;
    }
    else
    {
      c->setStash( "error_msg", "Incorrect username or password." );
    }
  }
  else if ( !Authentication::userExists( c ) )
  {
    c->setStash( "error_msg", "Empty username or password." );
  }

  c->setStash( "template", "login.html" );
}
