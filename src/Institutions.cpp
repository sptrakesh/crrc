#include "Institutions.h"
#include "dao/InstitutionDAO.h"
#include "dao/DesignationDAO.h"
#include "dao/InstitutionDesignationDAO.h"
#include "dao/functions.h"
#include "model/Institution.h"
#include "model/InstitutionDesignation.h"

#include <QtCore/QJsonArray>
#include "model/Designation.h"

namespace crrc
{
  namespace util
  {
    static auto institutionComparator = []( const QVariant& inst1, const QVariant& inst2 ) -> bool
    {
      const auto ptr1 = model::Institution::from( inst1 );
      const auto ptr2 = model::Institution::from( inst2 );
      return *ptr1 < *ptr2;
    };

    QJsonArray institutionsToArray( const QVariantList& list )
    {
      QJsonArray arr;
      for ( const auto& degree : list ) arr << toJson( *model::Institution::from( degree ) );
      return arr;
    }
  }
}

using crrc::Institutions;

void Institutions::index( Cutelyst::Context* c ) const
{
  auto list = dao::InstitutionDAO{}.retrieveAll();
  institutionList( c, list );
}

void Institutions::base( Cutelyst::Context* c ) const
{
  c->response()->setHeader( "Cache-Control", "no-cache, no-store, must-revalidate" );
}


void Institutions::object( Cutelyst::Context* c, const QString& id ) const
{
  c->setStash( "object", dao::InstitutionDAO().retrieve( id.toUInt() ) );
}

void Institutions::create( Cutelyst::Context* c ) const
{
  const auto id = c->request()->param( "id", "" );
  if ( !id.isEmpty() ) object( c, id );

  c->stash( {
    { "template", "institutions/form.html" },
    { "designations", dao::DesignationDAO().retrieveByType( "CAE" ) }
  } );
}

void Institutions::edit( Cutelyst::Context* c ) const
{
  const auto id = c->request()->param( "id", "" );
  const auto name = c->request()->param( "name", "" );
  if ( name.isEmpty() )
  {
    c->stash()["error_msg"] = "Institution name required!";
    return;
  }

  if ( ! canEdit( c ) )
  {
    c->stash()["error_msg"] = "Not authorized";
    return;
  }

  dao::InstitutionDAO dao;
  uint32_t iid = id.toUInt();

  if ( id.isEmpty() ) iid = dao.insert( c );
  else dao.update( c );

  const auto obj = dao.retrieve( iid );

  if ( "PUT" == c->request()->method() )
  {
    const auto ptr = model::Institution::from( obj );
    dao::sendJson( c, ptr ? toJson( *ptr ) : QJsonObject() );
    return;
  }

  c->stash( {
    { "template", "institutions/view.html" },
    { "object", obj }
  } );
}

void Institutions::view( Cutelyst::Context* c ) const
{
  c->setStash( "template", "institutions/view.html" );
}

void Institutions::data( Cutelyst::Context* c ) const
{
  const auto& var = c->stash( "object" );
  const auto ptr = model::Institution::from( var );

  if ( !ptr )
  {
    dao::sendJson( c, QJsonObject() );
    return;
  }

  auto json = toJson( *ptr );

  const auto& list = dao::InstitutionDesignationDAO().retrieve( c, ptr->getId() );
  QJsonArray arr;
  for ( const auto& id : list )
  {
    QJsonObject obj;
    const auto idptr = model::InstitutionDesignation::from( id );
    const auto dptr = model::Designation::from( idptr->getDesignation() );
    obj.insert( "designation", dptr ? toJson( *dptr ) : QJsonObject{} );
    obj.insert( "expiration", static_cast<int>( idptr->getExpiration() ) );
    arr << obj;
  }

  json.insert( "designations", arr );
  dao::sendJson( c, json );
}

void Institutions::search( Cutelyst::Context* c ) const
{
  const auto degree = c->request()->param( "degree" );
  if ( !degree.isEmpty() ) return byDegree( c, degree );

  const auto text = c->request()->param( "text" );
  auto list = text.isEmpty() ? QVariantList{} : dao::InstitutionDAO().search( c );
  c->setStash( "searchText", text );
  institutionList( c, list, "PUT" );
}

void Institutions::remove( Cutelyst::Context* c )
{
  auto id = c->request()->param( "id", "" );
  uint32_t count = 0;

  if ( ! canEdit( c ) )
  {
    c->stash()["error_msg"] = "Not authorized";
    return;
  }

  if ( ! id.isEmpty() )
  {
    count = dao::InstitutionDAO().remove( id.toUInt() );
  }

  if ( "PUT" == c->request()->method() )
  {
    QJsonObject obj;
    obj.insert( "id", id );
    obj.insert( "count", static_cast<int>( count ) );
    obj.insert( "status", count > 0 );
    dao::sendJson( c, obj );
    return;
  }

  c->response()->redirect( "/institutions" );
}

void Institutions::checkUnique( Cutelyst::Context* c )
{
  const auto name = c->request()->param( "name" );
  const auto city = c->request()->param( "city" );
  const auto status = dao::InstitutionDAO().isUnique( name, city );
  const QString string = status ? "true" : "false";

  c->response()->setContentType( "text/plain" );
  c->response()->setContentLength( string.size() );
  c->response()->setBody( string );
}

void Institutions::byDegree( Cutelyst::Context* c, const QString& id ) const
{
  if ( id.isEmpty() )
  {
    if ( "POST" == c->request()->method() )
    {
      dao::sendJson( c, QJsonArray{} );
      return;
    }
    return;
  }

  auto list = dao::InstitutionDAO().byDegree( id.toUInt() );
  institutionList( c, list, "PUT" );
}

bool Institutions::canEdit( Cutelyst::Context* c ) const
{
  auto id = c->request()->param( "id", "" );

  const auto flag = ( dao::isGlobalAdmin( c ) ||
      ( ! id.isEmpty() && dao::institutionId( c ) == id.toUInt() ) );
  return flag;
}

void Institutions::institutionList( Cutelyst::Context* c, QVariantList& list,
  const QString& method ) const
{
  qSort( list.begin(), list.end(), util::institutionComparator );

  if ( method == c->request()->method() )
  {
    const auto& arr = util::institutionsToArray( list );
    dao::sendJson( c, arr );
    return;
  }

  c->stash( {
    { "institutions", list },
    { "template", "institutions/index.html" }
  } );
}
