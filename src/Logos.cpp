#include "Logos.h"
#include "AttachmentController.h"
#include "dao/LogoDAO.h"

#include <QtCore/QStringBuilder>
#include <QtCore/QtDebug>

using crrc::Logos;

void Logos::base( Cutelyst::Context* c ) const
{
  c->response()->setHeader( "Cache-Control", "no-cache, no-store, must-revalidate" );
}


void Logos::object( Cutelyst::Context* c, const QString& id ) const
{
  AttachmentController<dao::LogoDAO>().object( c, id );
}

void Logos::create( Cutelyst::Context* c ) const
{
}

void Logos::display( Cutelyst::Context* c ) const
{
  AttachmentController<dao::LogoDAO>().display( c );
}

void Logos::remove( Cutelyst::Context* c )
{
  AttachmentController<dao::LogoDAO>().remove( c, "/" );
}
