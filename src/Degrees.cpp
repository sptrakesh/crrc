#include "Degrees.h"
#include "dao/DegreeDAO.h"

using crrc::Degrees;

void Degrees::index( Cutelyst::Context* c ) const
{
  dao::DegreeDAO dao;
  c->setStash( "degrees", dao.retrieveAll() );
  c->setStash( "template", "degrees/index.html" );
}

void Degrees::base( Cutelyst::Context* c ) const
{
  c->response()->setHeader( "Cache-Control", "no-cache, no-store, must-revalidate" );
}


void Degrees::object( Cutelyst::Context* c, const QString& id ) const
{
  dao::DegreeDAO dao;
  c->setStash( "object", dao.retrieve( id ) );
}

void Degrees::create( Cutelyst::Context* c ) const
{
  c->setStash( "template", "degrees/form.html" );
}

void Degrees::edit( Cutelyst::Context* c ) const
{
  auto id = c->request()->param( "id", "" );
  const auto title = c->request()->param( "title", "" );
  if ( title.isEmpty() )
  {
    c->stash()["error_msg"] = "Degree title required!";
    return;
  }

  dao::DegreeDAO dao;
  if ( id.isEmpty() )
  {
    auto cid = dao.insert( c );
    id = QString::number( cid );
  }
  else dao.update( c );

  c->stash( {
    { "template", "degrees/view.html" },
    { "object", dao.retrieve( id ) }
  } );
}

void Degrees::view( Cutelyst::Context* c ) const
{
  c->setStash( "template", "degrees/view.html" );
}

void Degrees::search( Cutelyst::Context* c ) const
{
  const auto text = c->request()->param( "text", "" );

  if ( text.isEmpty() )
  {
    c->response()->redirect( "/degrees" );
    return;
  }

  dao::DegreeDAO dao;
  c->stash( {
    { "degrees", dao.search( c ) },
    { "searchText", text },
    { "template", "degrees/index.html" }
  } );
}

void Degrees::remove( Cutelyst::Context* c )
{
  auto id = c->request()->param( "id", "" );
  QString statusMsg;

  if ( ! id.isEmpty() )
  {
    dao::DegreeDAO dao;
    statusMsg = dao.remove( id.toUInt() );
  }
  else statusMsg = "No degree identifier specified!";

  c->stash()["status_msg"] = statusMsg;
  c->response()->redirect( "/degrees" );
}