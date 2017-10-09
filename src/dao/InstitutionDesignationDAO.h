#pragma once

#include <Cutelyst/context.h>

namespace crrc
{
  namespace dao
  {
    struct InstitutionDesignationDAO
    {
      QVariantList retrieve( Cutelyst::Context* context, uint32_t institutionId ) const;
      uint32_t save( Cutelyst::Context* context ) const;
      uint32_t remove( Cutelyst::Context* context ) const;
      uint32_t remove( uint32_t institutionId ) const;

    private:
      uint32_t update( Cutelyst::Context* context ) const;
      uint32_t insert( Cutelyst::Context* context ) const;
    };
  }
}
