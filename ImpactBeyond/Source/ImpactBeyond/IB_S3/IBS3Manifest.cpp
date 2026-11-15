#include "IBS3Manifest.h"

UIBS3WorldManifest::UIBS3WorldManifest()
{
    auto Add=[this](const TCHAR* City,const TCHAR* Region,double Lat,double Lon,int32 Seed)
    {
        FIBS3City C; C.CityId=City; C.RegionId=Region; C.LatitudeDegrees=Lat; C.LongitudeDegrees=Lon; C.Seed=Seed;
        Cities.Add(C);
    };
    Add(TEXT("CITY_CENTRAL"),TEXT("earth.city.primary"),0,0,1101);
    Add(TEXT("CITY_PORT"),TEXT("earth.city.port"),0,90,1102);
    Add(TEXT("CITY_COAST"),TEXT("earth.city.coast"),0,179.5,1103);
    Add(TEXT("CITY_MOUNTAIN"),TEXT("earth.city.mountain"),75,-90,1104);
    Cities[0].BuildingHeightScale=2.2; Cities[1].BuildingHeightScale=0.6;
    Cities[2].BuildingHeightScale=0.9; Cities[3].BuildingHeightScale=1.3;
}

bool UIBS3WorldManifest::Validate(FString& Error) const
{
    if(BodyId.IsNone() || !FMath::IsFinite(RadiusCm) || RadiusCm<100000 || Cities.IsEmpty() ||
        !FMath::IsFinite(GravityCmS2) || GravityCmS2<=0 || !FMath::IsFinite(BrakeCmS2) || BrakeCmS2<=0 ||
        !FMath::IsFinite(TraversalSpeedCmS) || TraversalSpeedCmS<=0 || !FMath::IsFinite(InitialLoadP95Seconds) || InitialLoadP95Seconds<0 ||
        !FMath::IsFinite(PrefetchMarginCm) || PrefetchMarginCm<0 || !FMath::IsFinite(PhysicsBubbleCm) || PhysicsBubbleCm<1000)
    { Error=TEXT("Invalid planet, speed, gravity, braking, or load-budget data"); return false; }
    TSet<FName> CityIds,RegionIds;
    for(const FIBS3City& C:Cities)
    {
        if(C.CityId.IsNone() || C.RegionId.IsNone() || CityIds.Contains(C.CityId) || RegionIds.Contains(C.RegionId) ||
            !FMath::IsFinite(C.LatitudeDegrees) || !FMath::IsFinite(C.LongitudeDegrees) || FMath::Abs(C.LatitudeDegrees)>90 ||
            !FMath::IsFinite(C.FootprintRadiusCm) || C.FootprintRadiusCm<30000 || !FMath::IsFinite(C.SurfaceOffsetCm) || C.SurfaceOffsetCm<0 ||
            !FMath::IsFinite(C.BuildingHeightScale) || C.BuildingHeightScale<=0)
        { Error=TEXT("City IDs/regions must be unique and coordinates/footprints valid"); return false; }
        CityIds.Add(C.CityId); RegionIds.Add(C.RegionId);
    }
    return true;
}
