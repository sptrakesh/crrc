#pragma once

#include <Cutelyst/context.h>

namespace crrc
{
  namespace dao
  {
    struct DepartmentDAO
    {
      QVariantList retrieveAll() const;
      QVariantList retrieveByInstitution( uint32_t institutionId ) const;
      QVariantHash retrieve( const QString& id ) const;
      uint32_t insert( Cutelyst::Context* context ) const;
      uint32_t update( Cutelyst::Context* context ) const;
      QString remove( uint32_t id ) const;
    };
  }
}
