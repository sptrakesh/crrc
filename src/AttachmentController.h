#pragma once

#include <QtCore/QStringBuilder>
#include <Cutelyst/Controller>

namespace crrc
{
  template <typename DAO>
  struct AttachmentController
  {
    void object( Cutelyst::Context* c, const QString& id ) const
    {
      c->setStash( "object", DAO().retrieve( id ) );
    }

    QVariantHash edit( Cutelyst::Context* c ) const
    {
      auto id = getId( c );

      DAO dao;
      if ( id.isEmpty() )
      {
        auto cid = dao.insert( c );
        id = QString::number( cid );
      } else dao.update( c );

      return dao.retrieve( id );
    }

    void display( Cutelyst::Context* c ) const
    {
      const auto& object = c->stash( "object" ).toHash();
      c->response()->setHeader( "Cache-Control", "must-revalidate" );
      c->response()->setHeader( "ETag", object.value( "checksum" ).toString() );
      c->response()->setHeader( "Last-Modified", object.value( "updated" ).toString() );

      const auto& etag = c->request()->header( "ETag" );
      const auto& ifModified = c->request()->header( "If-Modified-Since" );
      if ( ( !etag.isEmpty() && etag == object.value( "updated" ).toString() ) ||
        ( !ifModified.isEmpty() && ifModified == object.value( "updated" ).toString() ) )
      {
        c->response()->setStatus( Cutelyst::Response::NotModified );
        return;
      }

      const auto bytes = DAO().contents( c, object.value( "id" ).toString() );
      c->response()->setContentType( object.value( "mimetype" ).toString() );
      c->response()->setContentLength( object.value( "filesize" ).toUInt() );
      const QString disposition = "inline; filename=\"" % object.value( "filename" ).toString() % "\"";
      c->response()->setHeader( "Content-Disposition", disposition );
      c->response()->setBody( bytes );
    }

    void remove( Cutelyst::Context* c, const QString& redirectUrl ) const
    {
      const auto id = getId( c );
      QString statusMsg;

      if ( !id.isEmpty() )
      {
        statusMsg = DAO().remove( id.toUInt() );
      }
      else statusMsg = "No agreement identifier specified!";

      c->stash()["status_msg"] = statusMsg;
      if ( ! redirectUrl.isEmpty() ) c->response()->redirect( redirectUrl );
    }

  private:
    QString getId( Cutelyst::Context* context ) const
    {
      const auto id = context->request()->param( "id", "" );
      if ( !id.isEmpty() ) return id;
      const auto& obj = context->stash( "object" );
      if ( obj.isNull() ) return id;
      return obj.toHash().value( "id" ).toString();
    }
  };
}