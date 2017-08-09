#include "Root.h"

#include <Cutelyst/Plugins/Authentication/authentication.h>

using crrc::Root;

Root::Root( QObject* parent ) : Controller( parent ) {}

Root::~Root() {}

void Root::index( Cutelyst::Context* c )
{
  c->setStash( "template", "index.html" );
}

void Root::defaultPage( Cutelyst::Context* c )
{
  c->response()->body() = "Page not found!";
  c->response()->setStatus( 404 );
}

bool Root::Auto( Cutelyst::Context* c )
{
  if ( c->controller() == c->controller( "crrc::Login" ) ) return true;
  if ( Cutelyst::Authentication::userExists( c ) ) return true;

  qDebug( "***Root::Auto User not found, forwarding to /login" );
  c->response()->redirect( c->uriFor( "/login" ) );
  return false;
}
