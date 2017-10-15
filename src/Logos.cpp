#include "Logos.h"
#include "AttachmentController.h"
#include "dao/LogoDAO.h"
#include "model/Logo.h"

#include <QtCore/QStringBuilder>
#include <QtCore/QtDebug>
#include <QtCore/QJsonDocument>

using crrc::Logos;

void Logos::index( Cutelyst::Context *c)
{
  c->response()->body() = "Matched Controller::Logos in Logos.";
}

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
  const auto& logo = AttachmentController<dao::LogoDAO>().edit( c );
  const auto ptr = model::Logo::from( logo );
  QJsonObject json;
  json.insert( "id", static_cast<int>( ptr->getId() ) );
  const auto& bytes = QJsonDocument( json ).toJson();

  c->response()->setContentType( "application/json" );
  c->response()->setContentLength( bytes.size() );
  c->response()->setBody( bytes );
}

void Logos::display( Cutelyst::Context* c ) const
{
  AttachmentController<dao::LogoDAO>().display( c );
}

void Logos::remove( Cutelyst::Context* c )
{
  AttachmentController<dao::LogoDAO>().remove( c, "/" );
}