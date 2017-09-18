#pragma once

#include <Cutelyst/context.h>


namespace crrc
{
  namespace dao
  {
    struct InstitutionDesignationDAO
    {
      QVariantList retrieve( uint32_t institution_id ) const;
      QString save( Cutelyst::Context* context ) const;
      QString remove( Cutelyst::Context* context ) const;
    };
  }
}
