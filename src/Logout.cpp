#include "Logout.h"
#include "dao/functions.h"

#include <QtCore/QJsonObject>
#include <Cutelyst/Plugins/Authentication/authentication.h>

using crrc::Logout;

Logout::Logout( QObject* parent ) : Controller( parent ) {}

Logout::~Logout() {}

void Logout::index( Cutelyst::Context* c )
{
  Cutelyst::Authentication::logout( c );
  if ( "POST" == c->request()->method() )
  {
    QJsonObject obj;
    obj.insert( "status", true );
    dao::sendJson( c, obj );
    return;
  }

  c->response()->redirect( c->uriFor( "/" ) );
}
