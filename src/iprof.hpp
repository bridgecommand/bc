// Copyright (c) 2015-2019 Paweł Cichocki
// License: https://opensource.org/licenses/MIT

#pragma once

#ifndef DISABLE_IPROF

#if (defined(_MSC_VER) && (_MSC_VER < 1916)) || defined(EMSCRIPTEN) || defined(CC_TARGET_OS_IPHONE) || defined(__ANDROID__)
#define DISABLE_IPROF_MULTITHREAD
#endif

#ifndef DISABLE_IPROF_OPTIM
#include <stdint.h>
#include <string.h>
#endif

#include <map>
#ifndef DISABLE_IPROF_MULTITHREAD
#include <mutex>
#endif
#include <ostream>
#include <vector>

#include "hitime.hpp"

#ifdef DISABLE_IPROF_MULTITHREAD
// For MSVC: __declspec(thread) doesn't work for things with a constructor
#define iprof_thread_local
#else
#define iprof_thread_local thread_local
#endif

namespace InternalProfiler
{
/// A faster (for storing within a vector) but limited vector<const char *>
#ifndef DISABLE_IPROF_OPTIM
class Tree
{
public:
   typedef uint16_t size_type;
   typedef const char* value_type;

protected:
   static const int MAX_DEPTH = 15;
   // Should make sizeof(RawEntry) == 64 (15*4+2+2) for 32bit systems
   // and compilers will align it to 128 for 64 bit systems.
   value_type tree[MAX_DEPTH];
   size_type fill = 0;
   size_type osize = 0;  ///< size with overflow

public:
   size_type size() const
   {
      return osize;
   }

   size_type capacity() const
   {
      return fill;
   }

   void push_back(value_type node)
   {
      ++osize;
      if (fill >= MAX_DEPTH)
         return;
      tree[fill++] = node;
      return;
   }

   void pop_back()
   {
      if (--osize < MAX_DEPTH)
         --fill;
   }

   const value_type* begin() const { return tree; }
   const value_type* end() const { return tree + fill; }
   const value_type& back() const { return *(tree + fill - 1); }

   bool operator==(const Tree& a) const
   {
      return size() == a.size() && 0 == memcmp(tree, a.tree, sizeof(value_type) * fill);
   }

   bool operator<(const Tree& a) const
   {
      if (size() < a.size())
         return true;
      else if (size() > a.size())
         return false;
      // We are just comparing pointer values here.
      // The perfect correctness of this depends on compiler putting all
      // string literals (const char *) under the same pointer, which is
      // not guaranteed under the specification, thus inline functions
      // might be a problem for this, and might be threated as different
      // functions when looked at from different translation units.
      return memcmp(tree, a.tree, sizeof(value_type) * fill) < 0;
   }
};
#else
typedef std::vector<const char*> Tree;
#endif

struct RawEntry
{
   Tree tree;
   HighResClock::time_point start;
   HighResClock::time_point end;
};

struct Stat
{
   HighResClock::duration totalTime = HighResClock::duration::zero();
   size_t numVisits = 0;
   Stat& operator+=(const Stat& a)
   {
      totalTime += a.totalTime;
      numVisits += a.numVisits;
      return *this;
   }
   Stat& operator-=(const Stat& a)
   {
      totalTime -= a.totalTime;
      numVisits -= a.numVisits;
      return *this;
   }
};

extern iprof_thread_local Tree tree;
extern iprof_thread_local std::vector<RawEntry> entries;
typedef std::map<Tree, Stat> Stats;   // we lack hashes for unordered
extern iprof_thread_local Stats stats;

#ifndef DISABLE_IPROF_MULTITHREAD
extern std::mutex allThreadStatLock;
extern Stats allThreadStats;
#endif

void aggregateEntries();
void addThisThreadEntriesToAllThreadStats();

inline void Begin(const char* node)
{
   tree.push_back(node);
   auto now = HighResClock::now();
   RawEntry re{tree, now, now - HighResClock::duration(1)};
   entries.emplace_back(re);
}
inline void End()
{
   auto s = tree.size();
   auto rei = entries.rbegin();
   while (rei->tree.size() != s)
      ++rei;
   rei->end = HighResClock::now();
   tree.pop_back();
}

struct ScopedMeasure
{
   ScopedMeasure(const char* node) { Begin(node); }
   ~ScopedMeasure() { End(); }
};
}

std::ostream& operator<<(std::ostream& os, const InternalProfiler::Stats& stats);

#ifndef __FUNCTION_NAME__
# ifdef _MSC_VER
#  define __FUNCTION_NAME__ __FUNCTION__
# else
#  define __FUNCTION_NAME__ __func__
# endif
#endif

#define IPROF(n) InternalProfiler::ScopedMeasure InternalProfiler__##__COUNTER__(n)
#define IPROF_FUNC InternalProfiler::ScopedMeasure InternalProfiler__##__COUNTER__(__FUNCTION_NAME__)

#else

# define IPROF(n)
# define IPROF_FUNC

#endif
