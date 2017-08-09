#include "Logout.h"

#include <Cutelyst/Plugins/Authentication/authentication.h>

using crrc::Logout;

Logout::Logout( QObject* parent ) : Controller( parent ) {}

Logout::~Logout() {}

void Logout::index( Cutelyst::Context* c )
{
  Cutelyst::Authentication::logout( c );
  c->response()->redirect( c->uriFor( "/" ) );
}
