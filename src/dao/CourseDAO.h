#pragma once

#include <Cutelyst/context.h>

namespace crrc
{
  namespace dao
  {
    struct CourseDAO
    {
      QVariantList retrieveAll() const;
      QVariantList retrieveByDepartment( uint32_t departmentId ) const;
      QVariantHash retrieve( const QString& id ) const;
      uint32_t insert( Cutelyst::Context* context ) const;
      void update( Cutelyst::Context* context ) const;
      QVariantList search( Cutelyst::Context* context ) const;
      QString remove( uint32_t id ) const;
    };
  }
}