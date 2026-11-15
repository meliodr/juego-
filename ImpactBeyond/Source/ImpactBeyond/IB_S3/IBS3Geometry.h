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
    bool finite() const { return std::isfinite(x) && std::isfinite(y) && std::isfinite(z); }
};
inline double wrap(double a) { a=std::fmod(a+Pi,Tau); return (a<0?a+Tau:a)-Pi; }
struct Geo { double lat=0, lon=0, altitude=0; bool valid=true; };
inline V3 position(Geo g, double radius, V3 center={})
{
    // UE axes: geographic north/east/up form an actor's forward/right/up basis.
    // Positive east longitude therefore points toward -world Y at the zero meridian.
    return center+V3{std::cos(g.lat)*std::cos(g.lon),-std::cos(g.lat)*std::sin(g.lon),std::sin(g.lat)}*(radius+g.altitude);
}
inline Geo geographic(V3 p, double radius, V3 center={}, double poleLongitude=0)
{
    const V3 d=p-center; const double r=d.length();
    if(r<1e-6 || !std::isfinite(r) || !std::isfinite(radius) || radius<=0 || !std::isfinite(poleLongitude)) return {0,0,0,false};
    return {std::asin(std::clamp(d.z/r,-1.0,1.0)), std::hypot(d.x,d.y)<r*1e-10 ? wrap(poleLongitude) : std::atan2(-d.y,d.x),r-radius,true};
}
struct Frame
{
    // Geographic components are east/north/up. Actor rotation uses X=north, Y=east, Z=up.
    V3 east, north, up;
    V3 toGlobal(V3 local) const { return east*local.x+north*local.y+up*local.z; }
    V3 toLocal(V3 global) const { return {global.dot(east),global.dot(north),global.dot(up)}; }
};
inline Frame frame(Geo g)
{
    const V3 u=position({g.lat,g.lon,0},1);
    const V3 e={-std::sin(g.lon),-std::cos(g.lon),0};
    return {e,e.cross(u).unit(),u};
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
    if(!std::isfinite(speed) || !std::isfinite(p95) || !std::isfinite(brake) || !std::isfinite(margin) ||
        speed<0 || p95<0 || brake<=0 || margin<0) return INFINITY;
    return speed*p95+(speed/brake)*speed*0.5+margin;
}
inline double safeSpeed(double clearance, double p95, double brake)
{
    if(!std::isfinite(clearance) || !std::isfinite(p95) || !std::isfinite(brake) || clearance<=0 || p95<0 || brake<=0) return 0;
    // Rationalized positive quadratic root: no subtraction of nearly equal huge numbers.
    const double denominator=0.5*std::hypot(p95,std::sqrt(clearance/brake)*std::sqrt(2.0))+0.5*p95;
    return denominator>0 ? clearance/denominator : 0;
}
inline double percentile95(std::vector<double> samples, double conservativeDefault)
{
    if(!std::isfinite(conservativeDefault) || conservativeDefault<0) return INFINITY;
    if(samples.empty()) return conservativeDefault;
    for(double s:samples) if(!std::isfinite(s) || s<0) return conservativeDefault;
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
    if(!g.valid || !std::isfinite(g.lat) || !std::isfinite(g.lon) || std::abs(g.lat)>Pi/2) return -1;
    if(g.lat>=Pi*0.45) return 72;
    if(g.lat<=-Pi*0.45) return 73;
    const int lat=std::clamp(static_cast<int>((g.lat+Pi/2)/Pi*6),0,5);
    const int lon=std::clamp(static_cast<int>((wrap(g.lon)+Pi)/Tau*12),0,11);
    return lat*12+lon;
}

inline double boundedStep(double remaining,double speed,double acceleration,double maxDistance=400,double maxTime=1.0/60.0)
{
    if(!std::isfinite(remaining) || !std::isfinite(speed) || !std::isfinite(acceleration) ||
        !std::isfinite(maxDistance) || !std::isfinite(maxTime) || remaining<=0 || speed<0 || acceleration<0 || maxDistance<=0 || maxTime<=0) return 0;
    // Semi-implicit velocity update: (v+a*dt)*dt must fit the collision sweep budget.
    const double denominator=speed+std::hypot(speed,2*std::sqrt(acceleration)*std::sqrt(maxDistance));
    const double byDistance=denominator>0 ? 2*maxDistance/denominator : maxTime;
    return std::min({remaining,maxTime,byDistance});
}
struct FlightStep { V3 position, velocity; bool valid=false; };
inline FlightStep flightStep(V3 p,V3 velocity,double dt,V3 center={},bool holdAltitude=true)
{
    if(!p.finite() || !velocity.finite() || !center.finite() || !std::isfinite(dt) || dt<0) return {p,{},false};
    const V3 offset=p-center; const double radius=offset.length();
    if(!std::isfinite(radius) || radius<1) return {p,{},false};
    const V3 up=offset*(1/radius); const double radial=holdAltitude?0:velocity.dot(up);
    const V3 tangent=velocity-up*velocity.dot(up); const double tangentialSpeed=tangent.length();
    if(!std::isfinite(tangentialSpeed)) return {p,{},false};
    const double dr=radial*dt;
    if(!std::isfinite(dr) || radius+dr<1) return {p,{},false};
    // Integrate angular rate vt/(r+vr*t), including simultaneous ascent/descent.
    const double ratio=dr/radius;
    const double integratedInverseRadius=std::abs(ratio)<1e-8 ? (dt/radius)*(1-ratio/2+ratio*ratio/3) : std::log1p(ratio)/radial;
    const ArcStep arc=advance(p,tangent,tangentialSpeed*integratedInverseRadius*radius,center);
    const V3 newUp=(arc.position-center).unit();
    return {center+newUp*(radius+dr),transport(tangent,up,newUp)+newUp*radial,true};
}
inline bool entersUnreadyRegion(V3 start,V3 end,V3 center,double radius)
{
    if(!start.finite() || !end.finite() || !center.finite() || !std::isfinite(radius) || radius<=0) return true;
    const V3 a=start-center,d=end-start; const double lengthSquared=d.dot(d);
    if(!std::isfinite(lengthSquared)) return true;
    if(lengthSquared<1e-12) return false;
    const double distanceSquared=a.dot(a),radiusSquared=radius*radius;
    if(!std::isfinite(distanceSquared) || !std::isfinite(radiusSquared)) return true;
    // Allow an already-inside player to retreat monotonically, never trap them behind the gate.
    if(distanceSquared<=radiusSquared) return a.dot(d)<0;
    const double t=std::clamp(-a.dot(d)/lengthSquared,0.0,1.0);
    const V3 closest=a+d*t;
    return closest.dot(closest)<=radiusSquared;
}
inline bool validSavedPose(V3 p,V3 scale,double quaternionSquared,V3 linear,V3 angular)
{
    return p.finite() && scale.finite() && linear.finite() && angular.finite() &&
        scale.x>0 && scale.y>0 && scale.z>0 && std::isfinite(quaternionSquared) && std::abs(quaternionSquared-1)<1e-4;
}
}
