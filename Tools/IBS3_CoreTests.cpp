#include "../ImpactBeyond/Source/ImpactBeyond/IB_S3/IBS3Geometry.h"
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <random>
#include <set>

using namespace IBS3;
int Checks=0;
void check(bool ok,const char* message) { ++Checks; if(!ok) { std::cerr<<"FAIL: "<<message<<'\n'; std::exit(1); } }
void close(double a,double b,double tolerance,const char* message) { check(std::abs(a-b)<=tolerance,message); }
int main()
{
    const V3 Center{1.2e9,-2.4e9,3.6e9};
    std::mt19937 Random(1103); std::uniform_real_distribution<double> Unit(0,1);
    double RoundTripError=0,ReturnError=0,AltitudeError=0,HeadingError=0;
    for(double Radius:{2500000.0,6000000.0,9000000.0})
    {
        for(int i=0;i<10000;++i)
        {
            Geo G{(Unit(Random)-0.5)*Pi,(Unit(Random)-0.5)*Tau,Unit(Random)*100000};
            V3 P=position(G,Radius,Center); Geo Back=geographic(P,Radius,Center,G.lon);
            double Error=(position(Back,Radius,Center)-P).length(); RoundTripError=std::max(RoundTripError,Error);
            check(Back.valid && Error<0.001,"LWC geographical roundtrip <0.001 cm");
            Frame F=frame(G); V3 Local{817.235,-344.567,456.789};
            check((F.toLocal(F.toGlobal(Local))-Local).length()<1e-8,"orthonormal tangent roundtrip");
            close(F.up.dot(F.east),0,1e-12,"east perpendicular to gravity");
            check(F.east.cross(F.north).dot(F.up)>0.99999999,"right handed frame");
        }
        // Both equatorial and meridian loops, including two poles; 60 Hz, no position reset at end.
        for(V3 Direction:{V3{0,1,0},V3{0,0,1}})
        {
            const double Speed=Radius==2500000?27000:Radius*Tau/600;
            const double OrbitRadius=Radius+50000,Total=Tau*OrbitRadius;
            V3 Start=Center+V3{OrbitRadius,0,0},P=Start,Tangent=Direction;
            double Done=0; std::set<int> Cells;
            while(Done<Total)
            {
                const double Step=std::min(Speed/60,Total-Done); const V3 OldUp=(P-Center).unit();
                ArcStep Next=advance(P,Tangent,Step,Center);
                const V3 NewUp=(Next.position-Center).unit();
                V3 Transported=transport(Tangent,OldUp,NewUp).unit();
                HeadingError=std::max(HeadingError,(Transported-Next.tangent).length());
                check((Next.position-P).length()<=Step+1e-5,"continuous arc chord bounded by motion");
                P=Next.position; Tangent=Next.tangent; Done+=Step;
                AltitudeError=std::max(AltitudeError,std::abs((P-Center).length()-OrbitRadius));
                Cells.insert(regionCell(geographic(P,Radius,Center)));
            }
            ReturnError=std::max(ReturnError,(P-Start).length());
            check((P-Start).length()<0.05,"circumnavigation return <0.05 cm");
            check(Tangent.dot(Direction)>0.999999999,"heading conserved after complete loop");
            check(Cells.size()>=6,"at least five region boundary crossings");
            if(Radius==2500000) check(Total/Speed>=480 && Total/Speed<=720,"25 km trial loop 8-12 minutes");
        }
    }
    check(!geographic(Center,2500000,Center).valid,"singular center rejected");
    for(double Lat:{-Pi/2,0.0,Pi/2})
    {
        Geo G{Lat,1.234,0}; V3 P=position(G,2500000,Center); Geo Back=geographic(P,2500000,Center,G.lon);
        close(Back.lon,G.lon,1e-9,"pole preserves longitude gauge");
        check(frame(Back).up.dot((P-Center).unit())>0.999999999,"pole gravity aligned");
    }
    const V3 West=position({0,Pi-1e-5,0},2500000),East=position({0,-Pi+1e-5,0},2500000);
    close(surfaceDistance(West,East,2500000),50,1e-6,"dateline distance is 50 cm, not circumference");
    MapPoint Mini=localMap(West,East,frame({0,Pi-1e-5}),2500000);
    close(Mini.x,50,1e-6,"minimap dateline continuity");
    check(localMap({1,0,0},{-1,0,0},frame({0,0}),1).ambiguous,"antipodal marker does not invent a bearing");
    close(globalMap({0,Pi,0}).x,globalMap({0,-Pi,0}).x,1e-12,"global map seam identical");
    close(prefetch(32000,1.5,8000,30000),142000,1e-7,"320m/s p95 + braking + margin = 1420 m");
    const double Clearance=112000; const double Safe=safeSpeed(Clearance,1.5,8000);
    close(Safe,32000,1e-7,"inverse stopping budget");
    check(std::isinf(prefetch(32000,1.5,0,0)),"zero braking cannot assert safe prefetch");
    close(percentile95({1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,100},1.5),19,0,"nearest rank p95");
    check(wanted(150000,142000,true,40000),"resident hysteresis retains near edge");
    check(!wanted(150000,142000,false,40000),"nonresident hysteresis does not preload late");
    check(safeSpeed(-1,1.5,8000)==0,"unready collider boundary stops approach");
    check(regionCell({Pi/2,0})==regionCell({Pi/2,2}),"polar region independent of longitude");
    std::cout<<std::setprecision(12)<<"PASS "<<Checks<<" assertions\n"
        <<"max_coordinate_roundtrip_cm="<<RoundTripError<<"\nmax_loop_return_cm="<<ReturnError
        <<"\nmax_altitude_drift_cm="<<AltitudeError<<"\nmax_transport_error="<<HeadingError
        <<"\ntrial_25km_500m_alt_270m_s_seconds="<<Tau*2550000/27000
        <<"\nprefetch_320m_s_cm="<<prefetch(32000,1.5,8000,30000)<<"\n";
}
