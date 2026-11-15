#pragma once

// Engine-independent double-precision geometry shared by runtime and native QA.
// All lengths are centimeters, angles radians. No wraps/teleports in world space.
#include <algorithm>
#include <cmath>
#include <vector>

namespace IBS3
{
constexpr double Pi = 3.14159265358979323846;
constexpr double Tau = 2.0 * Pi;
struct V3
{
    double x = 0, y = 0, z = 0;
    V3 operator+(V3 b) const { return {x+b.x,y+b.y,z+b.z}; }
    V3 operator-(V3 b) const { return {x-b.x,y-b.y,z-b.z}; }
    V3 operator*(double s) const { return {x*s,y*s,z*s}; }
    double dot(V3 b) const { return x*b.x+y*b.y+z*b.z; }
    V3 cross(V3 b) const { return {y*b.z-z*b.y,z*b.x-x*b.z,x*b.y-y*b.x}; }
    double length() const { return std::sqrt(dot(*this)); }
    V3 unit(V3 fallback = {0,0,1}) const { const double n=length(); return n>1e-12 ? *this*(1/n) : fallback; }
};
inline double wrap(double a) { a=std::fmod(a+Pi,Tau); return (a<0?a+Tau:a)-Pi; }
struct Geo { double lat=0, lon=0, altitude=0; bool valid=true; };
inline V3 position(Geo g, double radius, V3 center={})
{
    return center+V3{std::cos(g.lat)*std::cos(g.lon),std::cos(g.lat)*std::sin(g.lon),std::sin(g.lat)}*(radius+g.altitude);
}
inline Geo geographic(V3 p, double radius, V3 center={}, double poleLongitude=0)
{
    const V3 d=p-center; const double r=d.length();
    if(r<1e-6 || !std::isfinite(r) || radius<=0) return {0,poleLongitude,0,false};
    return {std::asin(std::clamp(d.z/r,-1.0,1.0)), std::hypot(d.x,d.y)<r*1e-10 ? wrap(poleLongitude) : std::atan2(d.y,d.x),r-radius,true};
}
struct Frame
{
    V3 east, north, up;
    V3 toGlobal(V3 local) const { return east*local.x+north*local.y+up*local.z; }
    V3 toLocal(V3 global) const { return {global.dot(east),global.dot(north),global.dot(up)}; }
};
inline Frame frame(Geo g)
{
    const V3 u=position({g.lat,g.lon,0},1);
    const V3 e={-std::sin(g.lon),std::cos(g.lon),0};
    return {e,u.cross(e).unit(),u};
}
// Minimum rotation transports a heading continuously over poles/dateline.
inline V3 transport(V3 vector, V3 oldUp, V3 newUp)
{
    oldUp=oldUp.unit(); newUp=newUp.unit(); const V3 k=oldUp.cross(newUp);
    const double c=std::clamp(oldUp.dot(newUp),-1.0,1.0);
    if(c<-0.999999999) // Unreachable in bounded runtime substeps; deterministic antipodal fallback.
    {
        V3 axis=oldUp.cross(std::abs(oldUp.z)<0.9?V3{0,0,1}:V3{1,0,0}).unit();
        return axis*(2*axis.dot(vector))-vector;
    }
    return vector+k.cross(vector)+k.cross(k.cross(vector))*(1/(1+c));
}
struct ArcStep { V3 position, tangent; };
inline ArcStep advance(V3 p, V3 direction, double distance, V3 center={})
{
    const V3 d=p-center; const double r=d.length(); const V3 u=d.unit();
    V3 t=(direction-u*direction.dot(u)).unit(frame(geographic(p,1,center)).east);
    if(r<1e-6) return {p,t};
    const double a=distance/r;
    return {center+(u*std::cos(a)+t*std::sin(a))*r,t*std::cos(a)-u*std::sin(a)};
}
inline double surfaceDistance(V3 a, V3 b, double radius, V3 center={})
{
    a=(a-center).unit(); b=(b-center).unit();
    return radius*std::atan2(a.cross(b).length(),std::clamp(a.dot(b),-1.0,1.0));
}
struct MapPoint { double x=0,y=0; bool ambiguous=false; };
inline MapPoint globalMap(Geo g) { return {(wrap(g.lon)+Pi)/Tau,(Pi/2-g.lat)/Pi,false}; }
inline MapPoint localMap(V3 observer, V3 target, Frame basis, double radius, V3 center={})
{
    const V3 a=(observer-center).unit(), b=(target-center).unit();
    const double angle=std::atan2(a.cross(b).length(),a.dot(b));
    V3 tangent=b-a*a.dot(b); const double n=tangent.length();
    if(n<1e-10) return {0,0,angle>Pi/2}; // Antipode has no unique bearing.
    tangent=tangent*(angle*radius/n);
    return {tangent.dot(basis.east),-tangent.dot(basis.north),false};
}
inline double heading(V3 forward, Frame basis) { return std::atan2(forward.dot(basis.east),forward.dot(basis.north)); }
inline double prefetch(double speed, double p95, double brake, double margin)
{
    speed=std::max(0.0,speed);
    if(brake<=0 || !std::isfinite(speed) || !std::isfinite(p95)) return INFINITY;
    return speed*std::max(0.0,p95)+speed*speed/(2*brake)+std::max(0.0,margin);
}
inline double safeSpeed(double clearance, double p95, double brake)
{
    if(clearance<=0 || brake<=0) return 0;
    const double bt=brake*std::max(0.0,p95);
    return std::sqrt(bt*bt+2*brake*clearance)-bt;
}
inline double percentile95(std::vector<double> samples, double conservativeDefault)
{
    if(samples.empty()) return conservativeDefault;
    std::sort(samples.begin(),samples.end());
    return samples[static_cast<size_t>(std::ceil(samples.size()*0.95))-1];
}
inline bool wanted(double distanceToEdge,double lookahead,bool resident,double hysteresis)
{
    return distanceToEdge<=lookahead+(resident?std::max(0.0,hysteresis):0.0);
}
// Spatial cell keys are deterministic; city IDs and origin object IDs never depend on this grid.
inline int regionCell(Geo g)
{
    if(g.lat>=Pi*0.45) return 72;
    if(g.lat<=-Pi*0.45) return 73;
    const int lat=std::clamp(static_cast<int>((g.lat+Pi/2)/Pi*6),0,5);
    const int lon=std::clamp(static_cast<int>((wrap(g.lon)+Pi)/Tau*12),0,11);
    return lat*12+lon;
}
}
