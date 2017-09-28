#include "ProgramDAO.h"
#include "InstitutionDAO.h"
#include "DegreeDAO.h"
#include "DesignationDAO.h"
#include "constants.h"
#include "functions.h"

#include <mutex>
#include <QtCore/QStringBuilder>
#include <QtSql/QtSql>
#include <Cutelyst/Plugins/Utils/sql.h>


namespace crrc
{
  namespace dao
  {
    struct Program
    {
      QVariant id;
      QVariant title;
      QVariant credits;
      QVariant institutionId;
      QVariant degreeId;
      QVariant type;
      QVariant designationId;
      QVariant curriculumCode;
      QVariant url;
    };

    static QMap<uint32_t, Program> programs;
    static std::atomic_bool programsLoaded{ false };
    static std::mutex programMutex;

    QVariantHash transform( const Program& program, const ProgramDAO::Mode& mode )
    {
      QVariantHash record;
      record.insert( "program_id", program.id );
      record.insert( "title", program.title );

      record.insert( "institution", InstitutionDAO().retrieve(
        program.institutionId.toString(), InstitutionDAO::Mode::Partial ) );

      if ( ProgramDAO::Mode::Full == mode )
      {
        record.insert( "credits", program.credits );

        if ( ! program.degreeId.isNull() )
        {
          record.insert( "degree", DegreeDAO().retrieve( program.degreeId.toUInt() ) );
        }

        record.insert( "type", program.type );

        if ( ! program.designationId.isNull() )
        {
          record.insert( "designation", DesignationDAO().retrieve( program.designationId.toString() ) );
        }

        record.insert( "curriculum_code", program.curriculumCode );
        record.insert( "url", program.url );
      }

      return record;
    }

    QVariantList fromPrograms( const ProgramDAO::Mode& mode )
    {
      QVariantList list;
      for ( const auto& program : programs )
      {
        list.append( transform( program, mode ) );
      }

      return list;
    }

    Program createProgram( QSqlQuery& query )
    {
      Program program;
      program.id = query.value( 0 ).toUInt();
      program.title = query.value( 1 );
      program.credits = query.value( 2 );

      auto id = query.value( 3 );
      program.institutionId = id.isNull() ? QVariant() : id.toUInt();

      id = query.value( 4 );
      program.degreeId = id.isNull() ? QVariant() : id.toUInt();

      program.type = query.value( 5 );

      id = query.value( 6 );
      program.designationId = id.isNull() ? QVariant() : id.toUInt();

      program.curriculumCode = query.value( 7 );
      program.url = query.value( 8 );

      return program;
    }

    void loadPrograms()
    {
      if ( programsLoaded.load() ) return;
      std::lock_guard<std::mutex> lock{ programMutex };
      if ( !programs.isEmpty() )
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
          const auto program = createProgram( query );
          auto id = program.id.toUInt();
          programs.insert( id, std::move( program ) );
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

      query.bindValue( ":cc", c->request()->param( "curriculum_code" ) );
      query.bindValue( ":url", c->request()->param( "url" ) );
    }

    Program programFromContext( Cutelyst::Context* context )
    {
      Program program;
      program.title = context->request()->param( "title" );
      program.credits = context->request()->param( "credits" );

      auto id = context->request()->param( "institution", "0" );
      program.institutionId = ( id != "0" ) ? id.toUInt() : QVariant();

      id = context->request()->param( "degree" );
      program.degreeId = ( id != "0" ) ? id.toUInt() : QVariant();

      program.type = context->request()->param( "type" );

      id = context->request()->param( "designation" );
      program.designationId = ( id != "0" ) ? id.toUInt() : QVariant();

      program.curriculumCode = context->request()->param( "curriculum_code" );
      program.url = context->request()->param( "url" );

      return program;
    }
  }
}

using crrc::dao::ProgramDAO;

QVariantList ProgramDAO::retrieveAll( const Mode& mode ) const
{
  loadPrograms();
  QVariantList list;

  auto query = CPreparedSqlQueryThreadForDB(
    "select p.program_id, i.name, p.title from programs p left join institutions i on (i.institution_id = p.institution_id) order by i.name, p.title",
    DATABASE_NAME );

  if ( query.exec() )
  {
    while ( query.next() ) list << retrieve( query.value( 0 ).toString(), mode );
  }

  return list;
}

QVariantList ProgramDAO::retrieveByInstitution(
  uint32_t institutionId, const Mode& mode ) const
{
  loadPrograms();
  QVariantList list;

  for ( const auto& program : programs )
  {
    if ( ! program.institutionId.isNull() && institutionId == program.institutionId.toUInt() )
    {
      list.append( transform( program, mode ) );
    }
  }

  return list;
}


QVariantHash ProgramDAO::retrieve( const QString& id, const Mode& mode ) const
{
  loadPrograms();
  const auto cid = id.toUInt();
  const auto iter = programs.constFind( cid );

  return ( iter != programs.end() ) ? 
    transform( iter.value(), mode ) : QVariantHash();
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
    context->stash()["error_msg"] = query.lastError().text();
    return 0;
  }

  const auto id = query.lastInsertId().toUInt();
  auto program = programFromContext( context );
  program.id = id;
  std::lock_guard<std::mutex> lock{ programMutex };
  programs[id] = std::move( program );
  return id;
}

void ProgramDAO::update( Cutelyst::Context* context ) const
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
      auto program = programFromContext( context );
      program.id = pid;
      std::lock_guard<std::mutex> lock{ programMutex };
      programs[pid] = std::move( program );
    } 

    context->stash()["error_msg"] = QString::number( query.numRowsAffected() );
  }
  else context->stash()["error_msg"] = query.lastError().text();
}

QVariantList ProgramDAO::search( Cutelyst::Context* context, const Mode& mode ) const
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
      auto program = retrieve( query.value( 0 ).toString(), mode );
      if ( admin || query.value( 1 ).toUInt() == iid ) list.append( program );
    }
  }
  else
  {
    qWarning() << query.lastError().text();
    context->stash()["error_msg"] = query.lastError().text();
  }

  return list;
}

QString ProgramDAO::remove( uint32_t id ) const
{
  loadPrograms();
  auto query = CPreparedSqlQueryThreadForDB(
    "delete from programs where program_id = :id", DATABASE_NAME );
  query.bindValue( ":id", id );
  if ( query.exec() )
  {
    std::lock_guard<std::mutex> lock{ programMutex };
    programs.remove( id );
    return QString::number( query.numRowsAffected() );
  }
  else return query.lastError().text();
}