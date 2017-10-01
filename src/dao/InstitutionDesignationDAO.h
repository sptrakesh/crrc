#pragma once

#include <Cutelyst/context.h>

namespace crrc
{
  namespace dao
  {
    struct InstitutionDesignationDAO
    {
      QVariantList retrieve( uint32_t institutionId ) const;
      QString save( Cutelyst::Context* context ) const;
      QString remove( Cutelyst::Context* context ) const;
      uint32_t remove( uint32_t institutionId ) const;

    private:
      QString update( Cutelyst::Context* context ) const;
      QString insert( Cutelyst::Context* context ) const;
    };
  }
}
