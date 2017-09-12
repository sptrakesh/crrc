#pragma once
#include <QtCore/QVariant>


namespace crrc
{
  namespace dao
  {
    struct RoleDAO
    {
      QVariantList retrieveAll() const;
      QVariantHash retrieve( const QString& id ) const;
      QVariantHash retrieveByRole( const QString& role ) const;
    };
  }
}
