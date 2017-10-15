#include "ProgramDAO.h"
#include "InstitutionDAO.h"
#include "DegreeDAO.h"
#include "DesignationDAO.h"
#include "constants.h"
#include "functions.h"
#include "model/Program.h"

#include <mutex>
#include <unordered_map>

#include <QtCore/QStringBuilder>
#include <QtSql/QtSql>
#include <Cutelyst/Plugins/Utils/sql.h>

using crrc::model::Program;


namespace crrc
{
  namespace dao
  {
    static std::unordered_map<uint32_t, Program::Ptr> programs;
    static std::atomic_bool programsLoaded{ false };
    static std::mutex programMutex;

    QVariantList fromPrograms()
    {
      QVariantList list;
      for ( const auto& iter : programs ) list << asVariant( iter.second.get() );
      return list;
    }

    void loadPrograms()
    {
      if ( programsLoaded.load() ) return;
      std::lock_guard<std::mutex> lock{ programMutex };
      if ( !programs.empty() )
      {
        programsLoaded = true;
        return;
      }

      auto query = CPreparedSqlQueryThreadForDB(
        "select p.program_id, p.title, p.credits, p.institution_id, p.degree_id, p.type, p.designation_id, p.curriculum_code, p.url, i.name from programs p left join institutions i on (i.institution_id = p.institution_id) order by i.name, p.title",
        DATABASE_NAME );

      if ( query.exec() )
      {
        while ( query.next() )
        {
          auto program = Program::create( query );
          programs[program->getId()] = std::move( program );
        }

        programsLoaded = true;
      }
    }

    void bindProgram( Cutelyst::Context* c, QSqlQuery& query )
    {
      query.bindValue( ":title", c->request()->param( "title" ) );

      const auto& credits = c->request()->param( "credits" );
      query.bindValue( ":credits", credits.isEmpty() ? "Unspecified" : credits );

      const auto iid = c->request()->param( "institution", "0" );
      if ( iid == "0" ) query.bindValue( ":institutionId", QVariant() );
      else query.bindValue( ":institutionId", iid );

      const auto did = c->request()->param( "degree", "0" );
      if ( did == "0" ) query.bindValue( ":degreeId", QVariant() );
      else query.bindValue( ":degreeId", did );

      query.bindValue( ":type", c->request()->param( "type" ) );

      const auto deid = c->request()->param( "designation", "0" );
      if ( deid == "0" ) query.bindValue( ":designationId", QVariant() );
      else query.bindValue( ":designationId", deid );

      query.bindValue( ":cc", c->request()->param( "curriculumCode" ) );
      query.bindValue( ":url", c->request()->param( "url" ) );
    }
  }
}

using crrc::dao::ProgramDAO;

QVariantList ProgramDAO::retrieveAll() const
{
  loadPrograms();
  QVariantList list;

  auto query = CPreparedSqlQueryThreadForDB(
    "select p.program_id, i.name, p.title from programs p left join institutions i on (i.institution_id = p.institution_id) order by i.name, p.title",
    DATABASE_NAME );

  if ( query.exec() )
  {
    while ( query.next() ) list << retrieve( query.value( 0 ).toUInt() );
  }

  return list;
}

QVariantList ProgramDAO::retrieveByInstitution( uint32_t institutionId ) const
{
  loadPrograms();
  QVariantList list;

  for ( const auto& iter : programs )
  {
    if ( institutionId == iter.second->getInstitutionId() )
    {
      list << asVariant( iter.second.get() );
    }
  }

  return list;
}


QVariant ProgramDAO::retrieve( const uint32_t id ) const
{
  loadPrograms();
  const auto iter = programs.find( id );

  return ( iter != programs.end() ) ? asVariant( iter->second.get() ) : QVariant();
}

uint32_t ProgramDAO::insert( Cutelyst::Context* context ) const
{
  loadPrograms();
  QSqlQuery query = CPreparedSqlQueryThreadForDB(
    "insert into programs (title, credits, institution_id, degree_id, type, designation_id, curriculum_code, url) values (:title, :credits, :institutionId, :degreeId, :type, :designationId, :cc, :url)",
    crrc::DATABASE_NAME );
  bindProgram( context, query );

  if ( !query.exec() )
  {
    qWarning() << query.lastError().text();
    context->stash()["error_msg"] = query.lastError().text();
    return 0;
  }

  const auto id = query.lastInsertId().toUInt();
  auto program = Program::create( context );
  program->setId( id );
  std::lock_guard<std::mutex> lock{ programMutex };
  programs[id] = std::move( program );
  return id;
}

uint32_t ProgramDAO::update( Cutelyst::Context* context ) const
{
  loadPrograms();
  auto id = context->request()->param( "id" );
  auto query = CPreparedSqlQueryThreadForDB(
    "update programs set title=:title, credits=:credits, institution_id=:institutionId, degree_id=:degreeId, type=:type, designation_id=:designationId, curriculum_code=:cc, url=:url where program_id=:id",
    crrc::DATABASE_NAME );
  bindProgram( context, query );
  query.bindValue( ":id", id.toInt() );

  if ( query.exec() )
  {
    if ( query.numRowsAffected() )
    {
      const auto pid = id.toUInt();
      auto program = Program::create( context );
      program->setId( pid );
      std::lock_guard<std::mutex> lock{ programMutex };
      programs[pid] = std::move( program );
      return query.numRowsAffected();
    } 
  }
  else
  {
    qWarning() << query.lastError().text();
    context->stash()["error_msg"] = query.lastError().text();
  }

  return 0;
}

QVariantList ProgramDAO::search( Cutelyst::Context* context) const
{
  loadPrograms();
  const auto text = context->request()->param( "text", "" );
  const QString clause = "%" % text % "%";

  auto query = CPreparedSqlQueryThreadForDB(
   "select program_id, institution_id from programs where title like :text order by title",
    DATABASE_NAME );

  query.bindValue( ":text", clause );
  QVariantList list;
  const auto admin = isGlobalAdmin( context );
  const auto iid = institutionId( context );

  if ( query.exec() )
  {
    while ( query.next() )
    {
      auto program = retrieve( query.value( 0 ).toUInt() );
      if ( admin || query.value( 1 ).toUInt() == iid ) list << program;
    }
  }
  else
  {
    qWarning() << query.lastError().text();
    context->stash()["error_msg"] = query.lastError().text();
  }

  return list;
}

uint32_t ProgramDAO::remove( uint32_t id ) const
{
  loadPrograms();
  auto query = CPreparedSqlQueryThreadForDB(
    "delete from programs where program_id = :id", DATABASE_NAME );
  query.bindValue( ":id", id );
  if ( query.exec() )
  {
    const auto count = query.numRowsAffected();
    std::lock_guard<std::mutex> lock{ programMutex };
    programs.erase( id );
    return count;
  }

  qWarning() << query.lastError().text();
  return 0;
}