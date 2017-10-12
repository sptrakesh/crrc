#include "Program.h"
#include "Designation.h"
#include "Institution.h"
#include "dao/DegreeDAO.h"
#include "dao/DesignationDAO.h"
#include "dao/InstitutionDAO.h"

using crrc::model::Program;

Program::Ptr Program::create( QSqlQuery& query )
{
  auto ptr = std::make_unique<Program>();
  ptr->id = query.value( 0 ).toUInt();
  ptr->title = query.value( 1 ).toString();
  ptr->credits = query.value( 2 ).toString();
  ptr->institutionId = query.value( 3 ).toUInt();
  ptr->degreeId = query.value( 4 ).toUInt();
  ptr->type = query.value( 5 ).toString();
  ptr->designationId = query.value( 6 ).toUInt();
  ptr->curriculumCode = query.value( 7 ).toString();
  ptr->url = query.value( 8 ).toString();
  return ptr;
}

Program::Ptr Program::create( Cutelyst::Context* context )
{
  auto ptr = std::make_unique<Program>();

  const auto& id = context->request()->param( "id" );
  if ( !id.isEmpty() ) ptr->id = id.toUInt();

  ptr->title = context->request()->param( "title" );
  ptr->credits = context->request()->param( "credits" );
  ptr->type = context->request()->param( "type" );
  ptr->curriculumCode = context->request()->param( "curriculumCode" );
  ptr->url = context->request()->param( "url" );
  ptr->institutionId = context->request()->param( "institution" ).toUInt();
  ptr->degreeId = context->request()->param( "degree" ).toUInt();
  ptr->designationId = context->request()->param( "designation" ).toUInt();

  return ptr;
}

QVariant Program::getInstitution() const
{
  return dao::InstitutionDAO().retrieve( institutionId );
}

QVariant Program::getDegree() const
{
  return dao::DegreeDAO().retrieve( degreeId );
}

QVariant Program::getDesignation() const
{
  return dao::DesignationDAO().retrieve( designationId );
}

QJsonObject crrc::model::toJson( const Program& program, bool compact )
{
  QJsonObject obj;
  obj.insert( "id", static_cast<int>( program.getId() ) );
  obj.insert( "title", program.getTitle() );
  obj.insert( "credits", program.getCredits() );
  obj.insert( "degree", toJson( *( Degree::from( program.getDegree() ) ) ) );

  if ( program.getInstitutionId() )
  {
    obj.insert( "institution", toJson( *( Institution::from( program.getInstitution() ) ), true ) );
  }

  if ( compact ) return obj;

  if ( !program.getType().isEmpty() ) obj.insert( "type", program.getType() );
  if ( !program.getCurriculumCode().isEmpty() ) obj.insert( "curriculumCode", program.getCurriculumCode() );
  if ( !program.getUrl().isEmpty() ) obj.insert( "url", program.getUrl() );

  if ( program.getDesignationId() )
  {
    obj.insert( "designation", toJson( *( Designation::from( program.getDesignation() ) ) ) );
  }

  return obj;
}
