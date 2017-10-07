#include "Program.h"
#include "dao/InstitutionDAO.h"
#include "dao/DegreeDAO.h"
#include "dao/DesignationDAO.h"

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
