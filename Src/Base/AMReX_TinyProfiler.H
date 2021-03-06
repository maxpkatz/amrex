#ifndef AMREX_TINY_PROFILER_H_
#define AMREX_TINY_PROFILER_H_

#include <string>
#include <deque>
#include <map>
#include <vector>
#include <tuple>
#include <utility>
#include <limits>
#include <iostream>

#include <AMReX_INT.H>
#include <AMReX_REAL.H>

#ifdef AMREX_USE_CUDA
#include "nvToolsExt.h"
#endif

namespace amrex {

//! A simple profiler that returns basic performance information (e.g. min, max, and average running time)
class TinyProfiler
{
public:
    explicit TinyProfiler (std::string funcname) noexcept;
    TinyProfiler (std::string funcname, bool start_, bool useCUPTI=false) noexcept;
    explicit TinyProfiler (const char* funcname) noexcept;
    TinyProfiler (const char* funcname, bool start_, bool useCUPTI=false) noexcept;
    ~TinyProfiler ();

    void start () noexcept;
    void stop () noexcept;
#ifdef AMREX_USE_CUPTI
    void stop (unsigned boxUintID) noexcept;
#endif // AMREX_USE_CUPTI

    static void Initialize () noexcept;
    static void Finalize (bool bFlushing = false) noexcept;

    static void StartRegion (std::string regname) noexcept;
    static void StopRegion (const std::string& regname) noexcept;

    static void PrintCallStack (std::ostream& os);

private:
    struct Stats
    {
        Stats () noexcept : depth(0), n(0L), dtin(0.0), dtex(0.0),
                            usesCUPTI(false), nk(0) { }
        int  depth;     //!< recursive depth
        Long n;         //!< number of calls
        double dtin;    //!< inclusive dt
        double dtex;    //!< exclusive dt
        bool usesCUPTI; //!< uses CUPTI
        Long nk;        //!< number of kernel calls
    };
  
    //! stats across processes
    struct ProcStats
    {
        ProcStats () : nmin(std::numeric_limits<Long>::max()),
                       navg(0L), nmax(0L),
                       dtinmin(std::numeric_limits<double>::max()),
                       dtinavg(0.0), dtinmax(0.0),
                       dtexmin(std::numeric_limits<double>::max()),
                       dtexavg(0.0), dtexmax(0.0),
                       usesCUPTI(false) {}
        Long nmin, navg, nmax;
        double dtinmin, dtinavg, dtinmax;
        double dtexmin, dtexavg, dtexmax;
        bool usesCUPTI;
        std::string fname;
        static bool compex (const ProcStats& lhs, const ProcStats& rhs) {
	    return lhs.dtexmax > rhs.dtexmax;
        }
        static bool compin (const ProcStats& lhs, const ProcStats& rhs) {
            return lhs.dtinmax > rhs.dtinmax;
        }
    };

    std::string fname;
    bool uCUPTI;
    int global_depth;
    std::vector<Stats*> stats;

    static std::vector<std::string> regionstack;
    static std::deque<std::tuple<double,double,std::string*> > ttstack;
    static std::map<std::string,std::map<std::string, Stats> > statsmap;
    static double t_init;
    static int device_synchronize_around_region;

    static void PrintStats (std::map<std::string,Stats>& regstats, double dt_max);
};

class TinyProfileRegion
{
public:
    explicit TinyProfileRegion (std::string a_regname) noexcept;
    explicit TinyProfileRegion (const char* a_regname) noexcept;
    ~TinyProfileRegion ();
private:
    std::string regname;
    TinyProfiler tprof;
};

}
#endif
