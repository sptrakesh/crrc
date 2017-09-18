#pragma once

#include <Cutelyst/context.h>

namespace crrc
{
  namespace dao
  {
    struct DesignationDAO
    {
      QVariantList retrieveAll() const;
      QVariantHash retrieve( const QString& id ) const;
      QVariantList retrieveByType( const QString& type ) const;
    };
  }
}
