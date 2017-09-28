#pragma once

#include "model/Degree.h"

namespace crrc
{
  namespace dao
  {
    struct DegreeDAO
    {
      const QVariantList retrieveAll() const;
      const QVariant retrieve( uint32_t id ) const;
      uint32_t insert( Cutelyst::Context* context ) const;
      void update( Cutelyst::Context* context ) const;
      QVariantList search( Cutelyst::Context* context ) const;
      uint32_t remove( uint32_t id ) const;
    };
  }
}