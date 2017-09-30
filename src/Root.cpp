#include "Root.h"
#include "dao/ContactDAO.h"
#include "dao/UserDAO.h"
#include "model/Contact.h"

#include <QtCore/QtDebug>
#include <QtNetwork/QNetworkCookie>
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

  const auto& user = Cutelyst::Authentication::user( c );
  if ( ! user.isEmpty() )
  {
    const auto uid = user.value( "id" ).toUInt();
    const auto& u = dao::UserDAO().retrieve( uid );
    const auto& contact = dao::ContactDAO().retrieveByUser( uid );
    qDebug() << "Contact: " << contact;
    const auto* cptr = qvariant_cast<model::Contact*>( contact );
    qDebug() << "Contact pointer: " << cptr;
    const auto& institution = cptr ? cptr->getInstitution() : QVariant();
    qDebug() << "Institution: " << institution;

    c->stash({
      { "user", u },
      { "userInstitution", institution },
      { "globalAdminRole", 1u },
      { "institutionAdminRole", 2u }
    } );
    return true;
  }

  qDebug() << "***Root::Auto User not found, forwarding to /login from" <<
    c->request()->uri();
  const auto cookie = QNetworkCookie( "url", c->request()->uri().toEncoded() );
  c->response()->setCookie( cookie );
  c->response()->redirect( c->uriFor( "/login" ) );
  return false;
}
