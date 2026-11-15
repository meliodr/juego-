#include "../ImpactBeyond/Source/ImpactBeyond/IB_S3/IBS3Geometry.h"
#include <iostream>
#include <limits>
#include <random>

using namespace IBS3;
int Failures=0, Checks=0;
void review(bool Ok,const char* Message)
{
    ++Checks;
    if(!Ok) { ++Failures; std::cerr<<"FAIL: "<<Message<<'\n'; }
}
int main()
{
    const double NaN=std::numeric_limits<double>::quiet_NaN();
    const Frame Equator=frame({0,0});
    review(Equator.up.cross(Equator.north).dot(Equator.east)>0.999999,
        "UE camera right when facing north must be geographic east");
    review(std::isinf(prefetch(NaN,1.5,8000,30000)),"invalid speed must not silently become zero for prefetch");
    review(safeSpeed(1000,1.5,NaN)==0,"invalid braking must fail closed, not return NaN");
    review(!geographic({2500000,0,0},NaN).valid,"invalid radius must reject geographical conversion");
    review(safeSpeed(100,1e8,8000)>0.999999e-6 && safeSpeed(100,1e8,8000)<1.000001e-6,
        "inverse stopping budget must avoid catastrophic cancellation");
    review(regionCell({NaN,0})==-1 && regionCell({0,0,0,false})==-1,"invalid geography must not cast NaN to an integer cell");
    review(percentile95({1,2,NaN},1.5)==1.5,"invalid load telemetry uses the conservative budget");
    std::mt19937 Random(1511); std::uniform_real_distribution<double> Unit(0,1);
    for(int i=0;i<10000;++i)
    {
        const Frame F=frame({(Unit(Random)-0.5)*Pi,(Unit(Random)-0.5)*Tau});
        review(F.up.cross(F.north).dot(F.east)>0.999999999,"geographic east matches UE local right everywhere");
        const double Clearance=std::pow(10.0,-6+Unit(Random)*15),Load=std::pow(10.0,-3+Unit(Random)*12),Brake=10+Unit(Random)*10000;
        const double Speed=safeSpeed(Clearance,Load,Brake);
        review(std::isfinite(Speed) && Speed>=0 && prefetch(Speed,Load,Brake,0)<=Clearance*(1+1e-12),
            "safe speed stays inside stopping budget over extreme finite scales");
    }
    const V3 Center{1.2e9,-2.4e9,3.6e9}; const double Radius=2500000;
    for(double Speed:{0.0,23999.0,32000.0,1000000.0,100000000.0})
        for(double Acceleration:{0.0,9000.0,1000000.0})
        {
            const double Dt=boundedStep(0.25,Speed,Acceleration);
            review(Dt>0 && Dt<=1.0/60.0 && (Speed+Acceleration*Dt)*Dt<=400.0+1e-8,
                "accelerating substeps stay inside the 4m sweep budget");
        }
    review(boundedStep(NaN,32000,9000)==0,"invalid frame time stops motion");
    review(boundedStep(0.25,32000,9000,NaN)==0,"invalid sweep length stops motion");
    const V3 P=Center+V3{Radius+50000,0,0};
    for(double Radial:{-32000.0,-1000.0,0.0,1000.0,32000.0})
    {
        const V3 Velocity{Radial,32000,5000}; const double Dt=boundedStep(1.0/60,Velocity.length(),9000);
        const FlightStep S=flightStep(P,Velocity,Dt,Center,false);
        review(S.valid && std::abs((S.position-Center).length()-(Radius+50000+Radial*Dt))<1e-5,
            "combined radial/tangent flight preserves requested altitude rate");
        review((S.position-P).length()<=Velocity.length()*Dt+1e-5,"combined ascent curve cannot exceed swept distance budget");
        review(std::abs(S.velocity.length()-Velocity.length())<1e-7,"curvature does not add kinetic energy");
        const FlightStep H=flightStep(P,Velocity,Dt,Center,true);
        review(H.valid && std::abs((H.position-Center).length()-(Radius+50000))<1e-5,
            "hold altitude removes radial drift explicitly");
    }
    const FlightStep First=flightStep(P,{0,32000,0},boundedStep(1.0/60,32000,9000),Center,true);
    const V3 NextUp=(First.position-Center).unit(),OldAcceleration{0,9000,0};
    review(std::abs(OldAcceleration.dot(NextUp))>1,"reproduces false vertical input from stale tangent acceleration");
    review(std::abs(transport(OldAcceleration,{1,0,0},NextUp).dot(NextUp))<1e-8,
        "transporting acceleration removes false vertical input in subsequent substeps");
    double MaxFlightReturn=0,MaxFlightAltitude=0;
    for(V3 Direction:{V3{0,1,0},V3{0,0,1}})
    {
        V3 Location=P,Velocity=Direction*27000;
        const double Duration=Tau*(Radius+50000)/27000; double Elapsed=0;
        while(Elapsed<Duration)
        {
            const double Dt=std::min(boundedStep(1.0/60,Velocity.length(),0),Duration-Elapsed);
            const FlightStep S=flightStep(Location,Velocity,Dt,Center,true);
            review(S.valid,"actual runtime flightStep accepts every numerical loop substep");
            Location=S.position; Velocity=S.velocity; Elapsed+=Dt;
            MaxFlightAltitude=std::max(MaxFlightAltitude,std::abs((Location-Center).length()-(Radius+50000)));
        }
        MaxFlightReturn=std::max(MaxFlightReturn,(Location-P).length());
        review((Location-P).length()<0.05,"runtime flight helper returns after equatorial/polar loop");
    }
    review(entersUnreadyRegion({-100000,0,0},{100000,0,0},{},50000),"both-endpoints-outside trajectory still detects unready city crossing");
    review(!entersUnreadyRegion({-100000,60000,0},{100000,60000,0},{},50000),"distant trajectory remains free");
    review(!entersUnreadyRegion({40000,0,0},{60000,0,0},{},50000),"already-inside pilot can retreat");
    review(entersUnreadyRegion({40000,0,0},{-60000,0,0},{},50000),"inside pilot cannot cross deeper through unready hub");
    review(entersUnreadyRegion({NaN,0,0},{1,0,0},{},50000),"invalid sweep fails closed");
    review(entersUnreadyRegion({-1e300,0,0},{1e300,0,0},{},50000),"overflowing segment math fails closed");
    review(validSavedPose(Center,{4,1.8,1.3},1,{32000,0,0},{}),"valid far-origin moving car state accepted");
    review(!validSavedPose(Center,{4,1.8,1.3},0,{32000,0,0},{}),"zero quaternion rejected despite finite components");
    review(!validSavedPose(Center,{4,0,1.3},1,{32000,0,0},{}),"degenerate collider scale rejected");
    review(!validSavedPose(Center,{4,1.8,1.3},1,{NaN,0,0},{}),"nonfinite saved velocity rejected");
    review(!flightStep(Center,{0,32000,0},1.0/60,Center).valid,"singular center stops flight rather than inventing gravity");
    review(!flightStep(P,{0,1e300,0},1.0/60,Center).valid,"overflowing flight speed stops safely");
    std::cout<<"runtime_flight_helper_max_return_cm="<<MaxFlightReturn<<"\nruntime_flight_helper_max_altitude_drift_cm="<<MaxFlightAltitude<<'\n';
    std::cout<<(Failures?"FAIL ":"PASS ")<<Checks<<" review assertions; failures="<<Failures<<'\n';
    return Failures?1:0;
}
