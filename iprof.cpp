// Copyright (c) 2015-2019 Paweł Cichocki
// License: https://opensource.org/licenses/MIT

#include "iprof.hpp"

namespace InternalProfiler
{
iprof_thread_local Tree tree;
iprof_thread_local std::vector<RawEntry> entries;
iprof_thread_local Stats stats;

#ifndef DISABLE_IPROF_MULTITHREAD
std::mutex allThreadStatLock;
Stats allThreadStats;
#endif

void aggregateEntries()
{
   std::vector<RawEntry> unfinished;
   if (entries.size() > 4)
      unfinished.reserve(entries.size() >> 1);
   for (auto ei : entries)
   {
      if (ei.start > ei.end)
      {
         unfinished.emplace_back(ei);
         continue;
      }
      Stat& s = stats[ei.tree];
      ++s.numVisits;
      s.totalTime += ei.end - ei.start;
   }
   std::swap(entries, unfinished);
}

#ifndef DISABLE_IPROF_MULTITHREAD
void addThisThreadEntriesToAllThreadStats()
{
   std::lock_guard<std::mutex> bouncer(allThreadStatLock);
   iprof_thread_local static Stats lastStats;
   for (auto& s : lastStats)
      allThreadStats[s.first] -= s.second;
   for (auto& s : stats)
      allThreadStats[s.first] += s.second;
   lastStats = stats;
}
#endif
}

std::ostream& operator<<(std::ostream& os, const InternalProfiler::Stats& stats)
{
   for (auto si : stats)
   {
      for (auto& ti : si.first)
         os << ti << (&ti != &si.first.back() ? "/" : "");
      if (si.first.capacity() < si.first.size())
         os << "/...(" << si.first.size() - si.first.capacity() << ")";
      os << ": " << MICRO_SECS(si.second.totalTime) / float(si.second.numVisits)
         << " (" << MICRO_SECS(si.second.totalTime) << " / " << si.second.numVisits << ")\n";
   }
   return os;
}
