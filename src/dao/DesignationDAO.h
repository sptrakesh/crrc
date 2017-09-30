#pragma once

#include <Cutelyst/context.h>

namespace crrc
{
  namespace dao
  {
    struct DesignationDAO
    {
      QVariantList retrieveAll() const;
      QVariant retrieve( const uint32_t id ) const;
      QVariantList retrieveByType( const QString& type ) const;
    };
  }
}
