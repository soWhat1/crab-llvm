#pragma once

#include <clam/crab/crab_defs.hh>
#include <crab/config.h>
#include <crab/domains/apron_domains.hpp>
#include <crab/domains/elina_domains.hpp>

namespace clam {
using namespace crab::domains;
#ifdef HAVE_APRON
using BASE(oct_domain_t) =
    apron_domain<number_t, dom_varname_t, apron_domain_id_t::APRON_OCT>;
#else
using BASE(oct_domain_t) =
    elina_domain<number_t, dom_varname_t, elina_domain_id_t::ELINA_OCT>;
#endif
using oct_domain_t = RGN_FUN(ARRAY_FUN(BOOL_NUM(BASE(oct_domain_t))));
} // end namespace clam
