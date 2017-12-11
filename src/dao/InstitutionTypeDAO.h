#pragma once

#include <QtCore/QVariant>

namespace crrc
{
  namespace dao
  {
    struct InstitutionTypeDAO
    {
      const QVariantList retrieveAll() const;
      const QVariant retrieve( uint32_t id ) const;
    };
  }
}
