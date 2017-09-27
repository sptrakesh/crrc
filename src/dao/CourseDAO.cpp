#include "CourseDAO.h"
#include "DepartmentDAO.h"
#include "constants.h"

#include <mutex>
#include <QtCore/QStringBuilder>
#include <QtSql/QtSql>
#include <Cutelyst/Plugins/Utils/sql.h>


namespace crrc
{
  namespace dao
  {
    struct Course
    {
      QVariant id;
      QVariant identifier;
      QVariant name;
      QVariant credits;
      QVariant description;
      QVariant knowledgeUnit;
      QVariant departmentId;
    };

    static QMap<uint32_t, Course> courses;
    static std::atomic_bool coursesLoaded{ false };
    static std::mutex courseMutex;

    QVariantHash transform( const Course& course )
    {
      QVariantHash record;
      record.insert( "course_id", course.id.toUInt() );
      record.insert( "identifier", course.identifier );
      record.insert( "name", course.name );
      record.insert( "credits", course.credits );
      record.insert( "description", course.description );
      record.insert( "knowledgeUnit", course.knowledgeUnit.toBool() );
      record.insert( "department", DepartmentDAO().retrieve( course.departmentId.toString() ) );

      return record;
    }

    QVariantList fromCourses()
    {
      QVariantList list;
      for ( const auto& course : courses ) list << transform( course );
      return list;
    }

    Course createCourse( QSqlQuery& query )
    {
      Course course;
      course.id = query.value( 0 ).toUInt();
      course.identifier = query.value( 1 );
      course.name = query.value( 2 );
      course.credits = query.value( 3 );
      course.description = query.value( 4 );
      const auto& ku = query.value( 5 ).toUInt();
      course.knowledgeUnit = ku == 1;
      course.departmentId = query.value( 6 ).toUInt();
      return course;
    }

    void loadCourses()
    {
      if ( coursesLoaded.load() ) return;
      std::lock_guard<std::mutex> lock{ courseMutex };
      if ( !courses.isEmpty() )
      {
        coursesLoaded = true;
        return;
      }

      auto query = CPreparedSqlQueryThreadForDB(
        "select course_id, identifier, name, credits, description, knowledge_unit, department_id from courses order by department_id, name",
        DATABASE_NAME );

      if ( query.exec() )
      {
        while ( query.next() )
        {
          const auto course = createCourse( query );
          auto id = course.id.toUInt();
          courses.insert( id, std::move( course ) );
        }

        coursesLoaded = true;
      }
    }

    void bindCourse( Cutelyst::Context* c, QSqlQuery& query )
    {
      query.bindValue( ":ident", c->request()->param( "identifier" ) );
      query.bindValue( ":name", c->request()->param( "name" ) );
      query.bindValue( ":credits", c->request()->param( "credits" ) );
      query.bindValue( ":desc", c->request()->param( "description" ) );
      query.bindValue( ":ku", c->request()->param( "knowledge_unit" ) );
      query.bindValue( ":dept", c->request()->param( "department" ).toUInt() );
    }

    Course courseFromContext( Cutelyst::Context* context )
    {
      Course course;
      course.identifier = context->request()->param( "identifier" );
      course.name = context->request()->param( "name" );
      course.credits = context->request()->param( "credits" );
      course.description = context->request()->param( "description" );
      course.knowledgeUnit = context->request()->param( "knowledge_unit" );
      course.departmentId = context->request()->param( "department" ).toUInt();
      return course;
    }
  }
}

using crrc::dao::CourseDAO;

QVariantList CourseDAO::retrieveAll() const
{
  loadCourses();
  return fromCourses();
}

QVariantList CourseDAO::retrieveByDepartment( uint32_t departmentId ) const
{
  loadCourses();
  QVariantList list;
  for ( const auto& course : courses )
  {
    if ( departmentId == course.departmentId.toUInt() ) list << transform( course );
  }

  return list;
}

QVariantHash CourseDAO::retrieve( const QString& id ) const
{
  loadCourses();
  const uint32_t cid = id.toUInt();
  const auto iter = courses.constFind( cid );
  return ( iter != courses.end() ) ? transform( iter.value() ) : QVariantHash();
}

uint32_t CourseDAO::insert( Cutelyst::Context* context ) const
{
  loadCourses();
  QSqlQuery query = CPreparedSqlQueryThreadForDB(
    "insert into courses (identifier, name, credits, description, knowledge_unit, department_id) values (:ident, :name, :credits, :desc, :ku, :dept)",
    crrc::DATABASE_NAME );
  bindCourse( context, query );

  if ( !query.exec() )
  {
    context->stash()["error_msg"] = query.lastError().text();
    return 0;
  }

  const auto id = query.lastInsertId().toUInt();
  auto course = courseFromContext( context );
  course.id = id;
  std::lock_guard<std::mutex> lock{ courseMutex };
  courses[id] = std::move( course );
  return id;
}

void CourseDAO::update( Cutelyst::Context* context ) const
{
  loadCourses();
  auto id = context->request()->param( "course_id" );
  auto query = CPreparedSqlQueryThreadForDB(
    "update courses set identifier=:ident, name=:name, credits=:credits, description=:desc, knowledge_unit=:ku, department_id=:dept where course_id=:id",
    crrc::DATABASE_NAME );
  bindCourse( context, query );
  query.bindValue( ":id", id.toInt() );

  if ( query.exec() )
  {
    auto course = courseFromContext( context );
    if ( query.numRowsAffected() )
    {
      course.id = id.toUInt();
      std::lock_guard<std::mutex> lock{ courseMutex };
      courses[id.toUInt()] = std::move( course );
    }

    context->stash()["error_msg"] = QString::number( query.numRowsAffected() );
  }
  else context->stash()["error_msg"] = query.lastError().text();
}

QVariantList CourseDAO::search( Cutelyst::Context* context ) const
{
  loadCourses();
  const auto text = context->request()->param( "text", "" );
  const QString clause = "%" % text % "%";

  auto query = CPreparedSqlQueryThreadForDB(
   "select course_id from courses where name like :text order by department_id, name",
    DATABASE_NAME );

  query.bindValue( ":text", clause );
  QVariantList list;

  if ( query.exec() )
  {
    while ( query.next() ) list << retrieve( query.value( 0 ).toString() );
  }
  else
  {
    qWarning() << query.lastError().text();
    context->stash()["error_msg"] = query.lastError().text();
  }

  return list;
}

QString CourseDAO::remove( uint32_t id ) const
{
  loadCourses();
  auto query = CPreparedSqlQueryThreadForDB(
    "delete from courses where course_id = :id", DATABASE_NAME );
  query.bindValue( ":id", id );
  if ( query.exec() )
  {
    if ( query.numRowsAffected() )
    {
      std::lock_guard<std::mutex> lock{ courseMutex };
      courses.remove( id );
    }
    return QString::number( query.numRowsAffected() );
  }

  return query.lastError().text();
}
