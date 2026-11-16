#pragma once
#include <array>
#include <algorithm>
#include <cmath>
#include <cstdint>
namespace IBS7T {
struct Slot {
    uint32_t ability=0;
    const char* label="";
    const char* icon="PLACEHOLDER_CIRCLE";
    double cooldown=0;
    bool available=true;
};
struct PowerWheelPort {
    virtual ~PowerWheelPort()=default;
    // Selection only. MUST NOT execute a power. False means no selection mutation.
    virtual bool select(uint32_t ability)=0;
};
class PowerWheel {
    std::array<Slot,8> slots{};
    unsigned count=0;
    int hover=-1;
    uint32_t active=0, previous=0;
    bool opened=false;
    double slowScale=1;
public:
    bool configure(const std::array<Slot,8>& values, unsigned n) {
        cancel(); // hero/flight context change always cancels pending confirmation
        active=previous=0;
        if(n<6 || n>8) { count=0; return false; }
        for(unsigned i=0;i<n;++i) {
            if(!values[i].ability || !values[i].label || !values[i].icon || !std::isfinite(values[i].cooldown) || values[i].cooldown<0) { count=0; return false; }
            for(unsigned j=0;j<i;++j) if(values[j].ability==values[i].ability) { count=0; return false; }
        }
        slots=values; count=n; active=previous=0; return true;
    }
    bool open(bool inputAllowed, double partialScale=1) {
        if(opened || !inputAllowed || count<6 || !std::isfinite(partialScale)) return false;
        opened=true; hover=-1; slowScale=std::clamp(partialScale,0.1,1.0); return true;
    }
    // Both mouse displacement from center and stick use normalized x/right, y/up.
    void point(double x,double y, bool stick) {
        if(!opened || !std::isfinite(x) || !std::isfinite(y)) { hover=-1; return; }
        const double radius=std::hypot(x,y), dead=stick?0.25:0.05;
        if(radius<dead) { hover=-1; return; }
        constexpr double pi=3.14159265358979323846;
        double angle=std::atan2(y,x); if(angle<0) angle+=2*pi;
        const double step=2*pi/count;
        if(stick && hover>=0) {
            const double delta=std::abs(std::remainder(angle-hover*step,2*pi));
            if(delta<step/2+0.08) return; // angular hysteresis
        }
        hover=static_cast<int>(std::floor((angle+step/2)/step))%static_cast<int>(count);
    }
    bool confirm(PowerWheelPort& port) {
        const int candidate=opened?hover:-1; cancel();
        if(candidate<0 || !slots[candidate].available || slots[candidate].cooldown>0) return false;
        if(!port.select(slots[candidate].ability)) return false;
        return synchronizeSelection(slots[candidate].ability);
    }
    bool selectPrevious(PowerWheelPort& port) {
        if(opened || !previous) return false;
        for(unsigned i=0;i<count;++i) if(slots[i].ability==previous && slots[i].available && slots[i].cooldown==0) {
            if(!port.select(previous)) return false;
            std::swap(active,previous); return true;
        }
        return false;
    }
    void cancel() { opened=false; hover=-1; slowScale=1; }
    void contextChanged() { cancel(); }
    bool updateCooldown(uint32_t ability,double seconds) {
        if(!std::isfinite(seconds) || seconds<0) return false;
        for(unsigned i=0;i<count;++i) if(slots[i].ability==ability) { slots[i].cooldown=seconds; return true; }
        return false;
    }
    bool updateAvailable(uint32_t ability,bool available) {
        for(unsigned i=0;i<count;++i) if(slots[i].ability==ability) { slots[i].available=available; return true; }
        return false;
    }
    // Providers may already have equipped this power. Refreshing the same selection must not erase history.
    bool synchronizeSelection(uint32_t ability) {
        for(unsigned i=0;i<count;++i) if(slots[i].ability==ability) {
            if(active!=ability) { previous=active; active=ability; }
            return true;
        }
        return false;
    }
    int hovered() const { return hover; }
    uint32_t selected() const { return active; }
    double requestedTimeScale() const { return opened?slowScale:1; }
    const Slot* view(unsigned i) const { return i<count?&slots[i]:nullptr; }
};
}
