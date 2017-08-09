#include "Programs.h"
#include "dao/ProgramDAO.h"
#include "dao/InstitutionDAO.h"
#include "dao/DegreeDAO.h"

using crrc::Programs;

void Programs::index( Cutelyst::Context* c ) const
{
  c->stash( {
    { "programs", dao::ProgramDAO().retrieveAll() },
    { "template", "programs/index.html" }
  } );
}

void Programs::base( Cutelyst::Context* c ) const
{
  c->response()->setHeader( "Cache-Control", "no-cache" );
}


void Programs::object( Cutelyst::Context* c, const QString& id ) const
{
  dao::ProgramDAO dao;
  c->setStash( "object", dao.retrieve( id ) );
}

void Programs::create( Cutelyst::Context* c ) const
{
  c->stash( {
    { "template", "programs/form.html" },
    { "institutions", dao::InstitutionDAO().retrieveAll( dao::InstitutionDAO::Mode::Partial ) },
    { "degrees", dao::DegreeDAO().retrieveAll() }
  } );
}

void Programs::edit( Cutelyst::Context* c ) const
{
  auto id = c->request()->param( "id", "" );
  const auto title = c->request()->param( "title", "" );
  if ( title.isEmpty() )
  {
    c->stash()["error_msg"] = "Program title required!";
    return;
  }

  dao::ProgramDAO dao;
  if ( id.isEmpty() )
  {
    auto cid = dao.insert( c );
    id = QString::number( cid );
  }
  else dao.update( c );

  c->stash( {
    { "template", "programs/view.html" },
    { "object", dao.retrieve( id ) }
  } );
}

void Programs::view( Cutelyst::Context* c ) const
{
  c->setStash( "template", "programs/view.html" );
}

void Programs::search( Cutelyst::Context* c ) const
{
  const auto text = c->request()->param( "text", "" );

  if ( text.isEmpty() )
  {
    c->response()->redirect( "/programs" );
    return;
  }

  dao::ProgramDAO dao;
  c->stash( {
    { "programs", dao.search( c ) },
    { "searchText", text },
    { "template", "programs/index.html" }
  } );
}

void Programs::remove( Cutelyst::Context* c )
{
  auto id = c->request()->param( "id", "" );
  QString statusMsg;

  if ( ! id.isEmpty() )
  {
    dao::ProgramDAO dao;
    statusMsg = dao.remove( id.toUInt() );
  }
  else statusMsg = "No program identifier specified!";

  c->stash()["status_msg"] = statusMsg;
  c->response()->redirect( "/programs" );
}
