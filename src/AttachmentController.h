#pragma once

#include "model/BlobItem.h"

#include <QtCore/QStringBuilder>
#include <Cutelyst/Controller>

namespace crrc
{
  template <typename DAO>
  struct AttachmentController
  {
    void object( Cutelyst::Context* c, const QString& id ) const
    {
      c->setStash( "object", DAO().retrieve( id.toUInt() ) );
    }

    QVariant edit( Cutelyst::Context* c ) const
    {
      auto id = getId( c );

      DAO dao;
      if ( id.isEmpty() )
      {
        auto cid = dao.insert( c );
        id = QString::number( cid );
      } else dao.update( c );

      return dao.retrieve( id.toUInt() );
    }

    void display( Cutelyst::Context* c ) const
    {
      const auto& object = c->stash( "object" );
      const auto ptr = model::BlobItem::from( object );
      c->response()->setHeader( "Cache-Control", "must-revalidate" );
      c->response()->setHeader( "ETag", ptr->getChecksum() );
      c->response()->setHeader( "Last-Modified", ptr->getUpdated() );

      const auto& etag = c->request()->header( "ETag" );
      const auto& ifModified = c->request()->header( "If-Modified-Since" );
      if ( ( !etag.isEmpty() && etag == ptr->getChecksum() ) ||
        ( !ifModified.isEmpty() && ifModified == ptr->getUpdated() ) )
      {
        c->response()->setStatus( Cutelyst::Response::NotModified );
        return;
      }

      const auto bytes = DAO().contents( c, ptr->getId() );
      c->response()->setContentType( ptr->getMimetype() );
      c->response()->setContentLength( ptr->getFilesize() );
      const QString disposition = "inline; filename=\"" % ptr->getFilename() % "\"";
      c->response()->setHeader( "Content-Disposition", disposition );
      c->response()->setBody( bytes );
    }

    void remove( Cutelyst::Context* c, const QString& redirectUrl ) const
    {
      const auto id = getId( c );

      if ( !id.isEmpty() ) DAO().remove( id.toUInt() );
      else c->stash()["error_msg"] = "No agreement identifier specified!";
      if ( ! redirectUrl.isEmpty() ) c->response()->redirect( redirectUrl );
    }

  private:
    QString getId( Cutelyst::Context* context ) const
    {
      const auto id = context->request()->param( "id", "" );
      if ( !id.isEmpty() ) return id;
      const auto& obj = context->stash( "object" );
      if ( obj.isNull() ) return id;

      const auto ptr = model::BlobItem::from( obj );
      return ptr ? QString::number( ptr->getId() ) : QString();
    }
  };
}