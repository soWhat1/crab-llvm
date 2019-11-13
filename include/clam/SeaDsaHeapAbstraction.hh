#pragma once

#include "clam/config.h"

#ifdef HAVE_SEA_DSA

#include "clam/HeapAbstraction.hh"
#include "llvm/ADT/DenseMap.h"
#include "llvm/ADT/ImmutableSet.h"
#include "llvm/ADT/StringRef.h"

#include <unordered_map>

// forward declarations
namespace sea_dsa {
class GlobalAnalysis;
class Node;
class Cell;
class AllocWrapInfo;
class ShadowMem;
} // namespace sea_dsa

namespace llvm {
class DataLayout;
class CallGraph;
class TargetLibraryInfo;
} // namespace llvm

namespace clam {
/*
 * Wrapper for sea-dsa (https://github.com/seahorn/sea-dsa)
 */
class LegacySeaDsaHeapAbstraction : public HeapAbstraction {

public:
  using typename HeapAbstraction::region_id_t;
  using typename HeapAbstraction::region_t;
  using typename HeapAbstraction::region_vector_t;

private:
  // XXX: We should use sea_dsa::Graph::SetFactory.
  // We copy here the definition of sea_dsa::Graph::SetFactory so
  // that we don't need to include Graph.hh
  using Set = llvm::ImmutableSet<llvm::Type *>;
  using SetFactory = typename Set::Factory;
  using region_bool_t = std::pair<region_t, bool>;
  using callsite_map_t =
      llvm::DenseMap<const llvm::CallInst *, std::vector<region_bool_t>>;

  region_t mkRegion(LegacySeaDsaHeapAbstraction *heap_abs,
                    const sea_dsa::Cell &c, region_info ri);

  region_id_t getId(const sea_dsa::Cell &c);

  // compute and cache the set of read, mod and new nodes of a whole
  // function such that mod nodes are a subset of the read nodes and
  // the new nodes are disjoint from mod nodes.
  void computeReadModNewNodes(const llvm::Function &f);

  // Compute and cache the set of read, mod and new nodes of a
  // callsite such that mod nodes are a subset of the read nodes and
  // the new nodes are disjoint from mod nodes.
  void computeReadModNewNodesFromCallSite(const llvm::CallInst &I,
                                          callsite_map_t &accessed,
                                          callsite_map_t &mods,
                                          callsite_map_t &news);

public:
  LegacySeaDsaHeapAbstraction(const llvm::Module &M, llvm::CallGraph &cg,
                              const llvm::DataLayout &dl,
                              const llvm::TargetLibraryInfo &tli,
                              const sea_dsa::AllocWrapInfo &alloc_wrap_info,
                              bool is_context_sensitive,
                              bool disambiguate_unknown = false,
                              bool disambiguate_ptr_cast = false,
                              bool disambiguate_external = false);

  ~LegacySeaDsaHeapAbstraction();

  virtual region_t getRegion(const llvm::Function &F,
                             const llvm::Value *V) override;

  virtual const llvm::Value *getSingleton(region_id_t region) const override;

  virtual region_vector_t getAccessedRegions(const llvm::Function &F) override;

  virtual region_vector_t getOnlyReadRegions(const llvm::Function &F) override;

  virtual region_vector_t getModifiedRegions(const llvm::Function &F) override;

  virtual region_vector_t getNewRegions(const llvm::Function &F) override;

  virtual region_vector_t getAccessedRegions(const llvm::CallInst &I) override;

  virtual region_vector_t getOnlyReadRegions(const llvm::CallInst &I) override;

  virtual region_vector_t getModifiedRegions(const llvm::CallInst &I) override;

  virtual region_vector_t getNewRegions(const llvm::CallInst &I) override;

  virtual llvm::StringRef getName() const override {
    return "LegacySeaDsaHeapAbstraction";
  }

private:
  const llvm::Module &m_m;
  const llvm::DataLayout &m_dl;
  sea_dsa::GlobalAnalysis *m_dsa;
  SetFactory *m_fac;
  /// map from Node to id
  llvm::DenseMap<const sea_dsa::Node *, region_id_t> m_node_ids;
  /// reverse map
  std::unordered_map<region_id_t, const sea_dsa::Node *> m_rev_node_ids;
  region_id_t m_max_id;
  bool m_disambiguate_unknown;
  bool m_disambiguate_ptr_cast;
  bool m_disambiguate_external;

  llvm::DenseMap<const llvm::Function *, std::vector<region_t>> m_func_accessed;
  llvm::DenseMap<const llvm::Function *, std::vector<region_t>> m_func_mods;
  llvm::DenseMap<const llvm::Function *, std::vector<region_t>> m_func_news;
  llvm::DenseMap<const llvm::CallInst *, std::vector<region_t>>
      m_callsite_accessed;
  llvm::DenseMap<const llvm::CallInst *, std::vector<region_t>> m_callsite_mods;
  llvm::DenseMap<const llvm::CallInst *, std::vector<region_t>> m_callsite_news;
};

class ShadowMemSeaDsaHeapAbstraction : public HeapAbstraction {

public:
  using typename HeapAbstraction::region_id_t;
  using typename HeapAbstraction::region_t;
  using typename HeapAbstraction::region_vector_t;

  ShadowMemSeaDsaHeapAbstraction(const llvm::Module &M, sea_dsa::ShadowMem &mem,
                                 bool disambiguate_unknown = false,
                                 bool disambiguate_ptr_cast = false,
                                 bool disambiguate_external = false);

  ~ShadowMemSeaDsaHeapAbstraction();

  virtual region_t getRegion(const llvm::Function &F,
                             const llvm::Value *V) override;

  virtual const llvm::Value *getSingleton(region_id_t region) const override;

  virtual region_vector_t getAccessedRegions(const llvm::Function &F) override;

  virtual region_vector_t getOnlyReadRegions(const llvm::Function &F) override;

  virtual region_vector_t getModifiedRegions(const llvm::Function &F) override;

  virtual region_vector_t getNewRegions(const llvm::Function &F) override;

  virtual region_vector_t getAccessedRegions(const llvm::CallInst &I) override;

  virtual region_vector_t getOnlyReadRegions(const llvm::CallInst &I) override;

  virtual region_vector_t getModifiedRegions(const llvm::CallInst &I) override;

  virtual region_vector_t getNewRegions(const llvm::CallInst &I) override;

  virtual llvm::StringRef getName() const override {
    return "ShadowMemSeaDsaHeapAbstraction";
  }

private:
  const llvm::Module &m_m;
  sea_dsa::ShadowMem &m_mem;
  bool m_disambiguate_unknown;
  bool m_disambiguate_ptr_cast;
  bool m_disambiguate_external;

  llvm::DenseMap<const llvm::Function *, std::vector<region_t>> m_func_accessed;
  llvm::DenseMap<const llvm::Function *, std::vector<region_t>> m_func_mods;
  llvm::DenseMap<const llvm::Function *, std::vector<region_t>> m_func_news;
  llvm::DenseMap<const llvm::CallInst *, std::vector<region_t>>
      m_callsite_accessed;
  llvm::DenseMap<const llvm::CallInst *, std::vector<region_t>> m_callsite_mods;
  llvm::DenseMap<const llvm::CallInst *, std::vector<region_t>> m_callsite_news;
};

#ifdef HAVE_SHADOW_MEM_SEA_DSA
using SeaDsaHeapAbstraction = ShadowMemSeaDsaHeapAbstraction;
#else
using SeaDsaHeapAbstraction = LegacySeaDsaHeapAbstraction;
#endif

} // end namespace clam
#endif /*HAVE_SEA_DSA*/
