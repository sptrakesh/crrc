#pragma once

#include <Cutelyst/context.h>

namespace crrc
{
  namespace dao
  {
    struct DepartmentDAO
    {
      QVariantList retrieveAll() const;
      QVariantHash retrieve( const QString& id ) const;
      uint32_t insert( Cutelyst::Context* context ) const;
      bool isUnique( const QString& name, const QString& prefix ) const;
    };
  }
}
