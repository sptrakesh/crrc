#pragma once
#include <QtCore/QVariant>


namespace crrc
{
  namespace dao
  {
    struct RoleDAO
    {
      QVariantList retrieveAll() const;
      QVariant retrieve( const uint32_t id ) const;
      QVariant retrieveByRole( const QString& role ) const;
    };
  }
}
