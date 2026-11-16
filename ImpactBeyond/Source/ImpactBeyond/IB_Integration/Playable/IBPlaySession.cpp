#include "IBPlaySession.h"
#include "IBPlayCharacter.h"
#include "IBPlayArt.h"
#include "IBPlayCombat.h"
#include "IBPlayEncounter.h"
#include "IB_S4/IBS4Threat.h"
#include "IB_S3/IBS3World.h"
#include "IB_S35/IBS35GripComponent.h"
#include "IB_S35/IBS35Object.h"
#include "IB_S35/IBS35ReceiverComponent.h"
#include "IB_S5/IBS5WorldLife.h"
#include "IB_S6H/IBS6HBridge.h"
#include "Animation/AnimSequence.h"
#include "Components/BoxComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/SkeletalMesh.h"
#include "EngineUtils.h"
#include "Kismet/GameplayStatics.h"
#include "Math/RotationMatrix.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"

AIBPlaySession::AIBPlaySession() { PrimaryActorTick.bCanEverTick=true; }
void AIBPlaySession::BeginPlay()
{
    Super::BeginPlay(); SessionId=FGuid::NewGuid();
    if(FParse::Param(FCommandLine::Get(),TEXT("IBPlayQA"))) bPersistence=false;
    for(TActorIterator<AIBS3PlanetWorld> It(GetWorld());It;++It) { Planet=*It; break; }
    Art=LoadObject<UIBPlayArt>(nullptr,TEXT("/Game/IB_Playable/DA_PlayArt.DA_PlayArt"));
    if(bPersistence && UGameplayStatics::DoesSaveGameExist(SaveSlot,0))
    {
        Loaded=Cast<UIBPlaySave>(UGameplayStatics::LoadGameFromSlot(SaveSlot,0));
        if(Loaded && Loaded->Version!=1) Loaded=nullptr;
    }
    const FTransform Identity;
    Life=GetWorld()->SpawnActorDeferred<AIBS5WorldLife>(AIBS5WorldLife::StaticClass(),Identity);
    if(Life)
    {
        Life->SaveSlot=SaveSlot+TEXT("_life"); Life->bEnablePersistence=bPersistence;
        Life->FinishSpawning(Identity);
        if(Planet) Life->ConfigurePlanet(Planet->GetActorLocation(),true);
        if(Art) Life->SetCitizenAppearance(Art->CitizenMesh);
    }
}
void AIBPlaySession::RegisterTarget(AActor* Actor)
{
    if(!IsValid(Actor)) return;
    if(AIBS7TThreatActor* T=Cast<AIBS7TThreatActor>(Actor))
    {
        UIBS35ReceiverComponent* R=Actor->FindComponentByClass<UIBS35ReceiverComponent>();
        if(!R) { R=NewObject<UIBS35ReceiverComponent>(Actor); Actor->AddInstanceComponent(R); R->RegisterComponent(); }
        R->TargetId=T->GetTargetLifeId();
        if(!R->HasCanonicalIngress()) R->BindCanonicalIngress(T,FIBS35CanonicalIngress::CreateWeakLambda(this,[this,T](FIBS35DamageMessage& M,UPrimitiveComponent* Body)
        {
            FHitResult Hit(T,Body,M.Contact,FVector::ZeroVector);
            const bool Accepted=T->AcceptCanonicalImpact(M,Hit);
            if(Accepted&&Life) Life->ReportDisturbance(M.Contact,M.InstigatorActor.Get()==Hero,15,M.AttackId,false);
            return Accepted;
        }));
        return;
    }
    UIBPlayCombat* C=Actor->FindComponentByClass<UIBPlayCombat>();
    if(!C&&!Actor->FindComponentByClass<UIBS4ResponseComponent>()&&!Actor->FindComponentByClass<UIBS35ReceiverComponent>()) return;
    if(!C)
    {
        C=NewObject<UIBPlayCombat>(Actor); Actor->AddInstanceComponent(C); C->RegisterComponent();
    }
    if(!Reporters.Contains(C))
    {
        Reporters.Add(C);
        const bool CivilianHarm=!Actor->IsA<AIBS4Threat>()&&!Actor->IsA<AIBPlayCharacter>()
            && !(Cast<AIBS4PreparedTarget>(Actor)&&Cast<AIBS4PreparedTarget>(Actor)->Kind==EIBS4TargetKind::Robot);
        C->OnCanonicalImpact.AddWeakLambda(this,[this,CivilianHarm](FGuid Cause,AActor* SourceActor,FVector Contact,float Magnitude)
        { if(Life) Life->ReportDisturbance(Contact,SourceActor==Hero,FMath::Max(5.f,Magnitude*.2f),Cause,CivilianHarm); });
    }
    C->InitializeReceiver();
}
bool AIBPlaySession::DispatchS35(const FIBS35DamageMessage& Input,const FHitResult& Hit)
{
    AActor* Target=Hit.GetActor(); if(!IsValid(Target)) return false;
    FIBS35DamageMessage M=Input; M.TargetPart=Hit.GetComponent();
    bool Accepted=false;
    if(AIBS7TThreatActor* T=Cast<AIBS7TThreatActor>(Target))
    {
        // Capture once at this synchronous contact. A message with an existing life is never rewritten.
        if(!M.TargetId.IsValid()) M.TargetId=T->GetTargetLifeId();
        Accepted=T->AcceptCanonicalImpact(M,Hit);
    }
    else
    {
        RegisterTarget(Target);
        if(UIBPlayCombat* C=Target->FindComponentByClass<UIBPlayCombat>())
        { if(!M.TargetId.IsValid()) M.TargetId=C->GetTargetLife(); Accepted=C->DispatchS35(M,Hit); }
    }
    if(Accepted&&Life&&Target->IsA<AIBS7TThreatActor>()) Life->ReportDisturbance(Hit.ImpactPoint,Input.InstigatorActor.Get()==Hero,15,Input.AttackId,false);
    return Accepted;
}
bool AIBPlaySession::DispatchS4(const FIBS4Hit& Input,const FHitResult& Hit)
{
    AActor* Target=Hit.GetActor(); if(!IsValid(Target)) return false;
    bool Accepted=false;
    if(AIBS7TThreatActor* T=Cast<AIBS7TThreatActor>(Target)) Accepted=T->AcceptCanonicalS4Impact(Input,T->GetTargetLifeId());
    else
    {
        RegisterTarget(Target);
        if(UIBPlayCombat* C=Target->FindComponentByClass<UIBPlayCombat>()) Accepted=C->DispatchS4(Input,Hit);
    }
    if(Accepted&&Life&&Target->IsA<AIBS7TThreatActor>())
    {
        const FGuid Cause=UIBPlayCombat::NativeCauseId(Input.AttackId);
        Life->ReportDisturbance(Hit.ImpactPoint,Input.Instigator==Hero,Input.Weapon==EIBS4Weapon::Missile?20:5,Cause,false);
    }
    return Accepted;
}
void AIBPlaySession::SpawnThreat(FName Identity,bool bKaiju,FVector Location,FVector Up)
{
    const FVector Facing=FVector::VectorPlaneProject(Hero->GetActorLocation()-Location,Up).GetSafeNormal();
    const FTransform Transform(FRotationMatrix::MakeFromXZ(Facing,Up).ToQuat(),Location);
    AIBS7TThreatActor* T=GetWorld()->SpawnActorDeferred<AIBS7TThreatActor>(AIBS7TThreatActor::StaticClass(),Transform);
    if(!T) return;
    T->bKaiju=bKaiju; T->bRadialGravity=true; T->PlanetCenter=Planet->GetActorLocation(); T->Target=Hero;
    T->bUseSkeletalParts=true;
    T->bRegionReady=true;
    if(Art)
    {
        USkeletalMesh* Mesh=bKaiju?Art->Kaiju:Art->Robot;
        T->Visual->SetSkeletalMesh(Mesh);
        if(Mesh)
        {
            const FBoxSphereBounds B=Mesh->GetBounds();
            const double Scale=(bKaiju?800.0:400.0)/FMath::Max(1.0,B.BoxExtent.Z*2);
            T->Visual->SetRelativeScale3D(FVector(Scale));
            T->Visual->SetRelativeLocation(FVector(0,0,-(bKaiju?400:200)-B.Origin.Z*Scale+B.BoxExtent.Z*Scale));
        }
        const auto& Parts=bKaiju?Art->KaijuParts:Art->RobotParts;
        T->PartBones=bKaiju?Art->KaijuPartBones:Art->RobotPartBones;
        for(int32 I=0;I<T->Modules.Num();++I) if(Parts.IsValidIndex(I))
        { T->Modules[I]->SetStaticMesh(Parts[I]); T->Modules[I]->SetVisibility(false); }
        const auto& Anim=bKaiju?Art->KaijuAnimations:Art->RobotAnimations;
        if(Anim.Num()>0) T->IdleAnimation=Anim[0];
        if(Anim.Num()>1) T->WalkAnimation=Anim[1];
        if(Anim.Num()>2) T->AttackAnimation=Anim[2];
    }
    T->FinishSpawning(Transform);
    if(Art)
    {
        const auto& Parts=bKaiju?Art->KaijuParts:Art->RobotParts;
        const auto& Bones=bKaiju?Art->KaijuPartBones:Art->RobotPartBones;
        for(int32 I=0;I<(bKaiju?3:2);++I) if(Parts.IsValidIndex(I)&&Bones.IsValidIndex(I))
            T->ConfigurePartArt(static_cast<EIBS7TPart>(I+(bKaiju?2:0)),Parts[I],Bones[I]);
    }
    T->ConfigureIdentity(Identity,SessionId,Planet->GetActorLocation());
    T->CanonicalStrike.BindUObject(this,&AIBPlaySession::DispatchS4);
    if(Loaded) for(const auto& S:Loaded->Threats) if(S.StableId==Identity) { T->RestoreSnapshot(S); break; }
    RegisterTarget(T); Threats.Add(T);
}
void AIBPlaySession::SpawnRegion(FName Region,FVector Center,FVector Up)
{
    if(SpawnedRegions.Contains(Region)) return;
    SpawnedRegions.Add(Region);
    const FVector East=FVector::CrossProduct(FVector::UpVector,Up).GetSafeNormal(0.001,FVector::RightVector);
    const FVector North=FVector::CrossProduct(Up,East).GetSafeNormal();
    const FQuat Rotation=FRotationMatrix::MakeFromXZ(North,Up).ToQuat();
    for(int32 I=0;I<8;++I)
    {
        const FName Id(*FString::Printf(TEXT("%s/cargo/%d"),*Region.ToString(),I));
        const int32 MeshIndex=Art&&!Art->Props.IsEmpty()?I%Art->Props.Num():0;
        FTransform Transform(Rotation,Center+East*(I%4*650-1000)+North*(I<4?1800:-1800)+Up*160);
        const FIBPlayObjectState* Previous=Loaded?Loaded->Objects.FindByPredicate([Id](const FIBPlayObjectState& S){return S.Id==Id;}):nullptr;
        if(Previous&&!Previous->Transform.ContainsNaN()) Transform=Previous->Transform;
        SpawnObject(Id,MeshIndex,Transform,Previous);
    }
    SpawnThreat(FName(*FString::Printf(TEXT("%s/heavy"),*Region.ToString())),false,Center+North*6000+Up*230,Up);
    if(Region.ToString().Contains(TEXT("port")) || Region.ToString().Contains(TEXT("mountain")))
        SpawnThreat(FName(*FString::Printf(TEXT("%s/kaiju"),*Region.ToString())),true,Center-North*7000+Up*430,Up);
    AIBPlayEncounter* Encounter=GetWorld()->SpawnActor<AIBPlayEncounter>();
    if(Encounter)
    {
        if(Art)
        {
            Encounter->OfficerMesh=Art->CitizenMesh;
            if(Art->Heroes.Num()>2) Encounter->RivalMesh=Art->Heroes[2];
            Encounter->TankMesh=Art->Tank; Encounter->DroneMesh=Art->Drone;
            Encounter->RobotBodyMesh=Art->SmallRobot; Encounter->RobotCannonMesh=Art->Cannon;
            Encounter->ConcreteMaterial=Art->Concrete; Encounter->GroundMaterial=Art->Ground;
            Encounter->GlassMaterial=Art->Glass; Encounter->MetalMaterial=Art->Metal;
        }
        Encounter->OfficerIdle=LoadObject<UAnimSequence>(nullptr,TEXT("/Game/Characters/Mannequins/Anims/Unarmed/MM_Idle"));
        Encounter->OfficerWalk=LoadObject<UAnimSequence>(nullptr,TEXT("/Game/Characters/Mannequins/Anims/Unarmed/Walk/MF_Unarmed_Walk_Fwd"));
        Encounter->RivalIdle=Encounter->OfficerIdle;
        if(Encounter->SpawnRegion(Region,Planet->GetActorLocation(),Center+North*3500,Up,Hero))
        {
            for(AIBS4PreparedTarget* Target:Encounter->PreparedTargets) RegisterTarget(Target);
            for(AIBS4Threat* Threat:Encounter->Threats) RegisterTarget(Threat);
            if(Loaded)
            {
                TArray<FIBS4PreparedSnapshot> Relevant;
                for(const auto& State:Loaded->Prepared) if(Encounter->PreparedTargets.ContainsByPredicate([&](const AIBS4PreparedTarget* Target){return Target->ObjectId==State.ObjectId;})) Relevant.Add(State);
                if(!Relevant.IsEmpty()) Encounter->RestorePreparedState(Relevant);
            }
            Encounters.Add(Encounter);
        }
    }
}
void AIBPlaySession::SpawnObject(FName Id,int32 MeshIndex,const FTransform& Transform,const FIBPlayObjectState* Previous)
{
    if(Objects.Contains(Id)) return;
    AIBS35Object* Object=GetWorld()->SpawnActorDeferred<AIBS35Object>(AIBS35Object::StaticClass(),Transform);
    if(!Object) return;
    Object->Profile=FIBS35ObjectProfile::Preset(MeshIndex==0?EIBS35ObjectClass::Car:MeshIndex==1?EIBS35ObjectClass::Container:EIBS35ObjectClass::Rock);
    Object->bRadialGravity=true; Object->PlanetCenter=Planet->GetActorLocation();
    if(Art&&Art->Props.IsValidIndex(MeshIndex)) Object->Appearance->SetStaticMesh(Art->Props[MeshIndex]);
    Object->FinishSpawning(Transform); RegisterTarget(Object); Objects.Add(Id,Object);
    if(Previous)
    {
        Object->Body->SetPhysicsLinearVelocity(Previous->Velocity); Object->Body->SetPhysicsAngularVelocityInRadians(Previous->AngularVelocity);
        if(auto* C=Object->FindComponentByClass<UIBPlayCombat>()) { C->Response->Health=Previous->Health; C->ContactReceiver->Health=Previous->Health; }
    }
}
void AIBPlaySession::RestoreNearbyObjects()
{
    if(!Loaded||!Hero||!Planet) return;
    for(const auto& S:Loaded->Objects)
        if(!Objects.Contains(S.Id)&&FVector::DistSquared(S.Transform.GetLocation(),Hero->GetActorLocation())<FMath::Square(Planet->Manifest->PhysicsBubbleCm))
            SpawnObject(S.Id,S.MeshIndex,S.Transform,&S);
}
void AIBPlaySession::Tick(float Dt)
{
    Super::Tick(Dt);
    if(!Planet||!Planet->IsOperational()) return;
    if(!Hero) Hero=Cast<AIBPlayCharacter>(UGameplayStatics::GetPlayerPawn(this,0));
    if(!Hero||!Life) return;
    if(!bRestoredHero)
    {
        Life->SetObserver(Hero);
        if(Loaded&&Loaded->Hero>=0&&Loaded->Hero<=2) Hero->Heroes->SelectHero(Loaded->Hero);
        if(Loaded&&!Loaded->HeroTransform.ContainsNaN())
        {
            const FVector P=Loaded->HeroTransform.GetLocation();
            Planet->UpdateResidency(P,0);
            if(!Planet->CanTraverse(P,P,96)) return;
            FCollisionQueryParams Query(SCENE_QUERY_STAT(IBPlayRestore),false,Hero);
            if(Planet->CanTraverse(P,P,96)&&!GetWorld()->OverlapBlockingTestByChannel(P,Loaded->HeroTransform.GetRotation(),ECC_Pawn,Hero->GetCapsuleComponent()->GetCollisionShape(),Query))
            {
                Hero->SetActorTransform(Loaded->HeroTransform,false,nullptr,ETeleportType::TeleportPhysics);
                Hero->PreviousUp=Hero->GetActorUpVector();
                Hero->TangentForward=FVector::VectorPlaneProject(Hero->GetActorForwardVector(),Hero->PreviousUp).GetSafeNormal();
                Hero->GetCharacterMovement()->SetMovementMode(MOVE_Flying);
            }
        }
        bRestoredHero=true;
    }
    if(GetWorld()->GetTimeSeconds()<NextUpdate) return;
    NextUpdate=GetWorld()->GetTimeSeconds()+.25;
    RestoreNearbyObjects();
    const FIBS3City* Active=Planet->Manifest->Cities.FindByPredicate([this](const FIBS3City& C){return C.CityId==Planet->ActiveCity;});
    if(Active)
    {
        const FVector Center=Planet->CityPosition(*Active);
        const FVector Up=(Center-Planet->GetActorLocation()).GetSafeNormal();
        const bool Ready=Planet->IsReady(Active->CityId);
        Life->SetRegionContext(Active->RegionId,Center,Up,Ready);
        if(Ready) SpawnRegion(Active->RegionId,Center,Up);
    }
    else Life->SetRegionContext(NAME_None,FVector::ZeroVector,Hero->GetActorUpVector(),false);
    for(auto& Pair:Objects) if(IsValid(Pair.Value)&&!Pair.Value->IsHeld())
    {
        const bool Near=FVector::DistSquared(Pair.Value->GetActorLocation(),Hero->GetActorLocation())<FMath::Square(Planet->Manifest->PhysicsBubbleCm);
        Pair.Value->SetActorHiddenInGame(!Near); Pair.Value->SetActorTickEnabled(Near);
        if(Pair.Value->Body->IsSimulatingPhysics()!=Near) Pair.Value->Body->SetSimulatePhysics(Near);
    }
    for(AIBS7TThreatActor* Threat:Threats) if(IsValid(Threat))
    {
        const bool Near=FVector::DistSquared(Threat->GetActorLocation(),Hero->GetActorLocation())<FMath::Square(Planet->Manifest->PhysicsBubbleCm);
        Threat->SetRegionReady(Near); Threat->SetActorHiddenInGame(!Near);
    }
    for(AIBPlayEncounter* Encounter:Encounters) if(IsValid(Encounter))
        Encounter->SetRegionReady(Active&&Encounter->RegionId==Active->RegionId&&GetWorld()->GetTimeSeconds()>15);
}
bool AIBPlaySession::SaveSession()
{
    if(!bPersistence||!Planet||!Life) return false;
    UIBPlaySave* Save=nullptr; if(!CaptureSnapshot(Save)) return false;
    if(!Planet->SaveState()||!Life->SaveWorld()) return false;
    return UGameplayStatics::SaveGameToSlot(Save,SaveSlot,0);
}
bool AIBPlaySession::CaptureSnapshot(UIBPlaySave*& Out)
{
    Out=nullptr;
    if(!Hero||Hero->Grip->GetHeld()) return false;
    UIBPlaySave* Save=Cast<UIBPlaySave>(UGameplayStatics::CreateSaveGameObject(UIBPlaySave::StaticClass()));
    Save->Hero=Hero->Heroes->ActiveHero(); Save->HeroTransform=Hero->GetActorTransform();
    if(Loaded) { Save->Objects=Loaded->Objects; Save->Threats=Loaded->Threats; Save->Prepared=Loaded->Prepared; }
    for(auto& Pair:Objects) if(IsValid(Pair.Value))
    {
        if(Pair.Value->IsHeld()) return false;
        FIBPlayObjectState S; S.Id=Pair.Key; S.Transform=Pair.Value->GetActorTransform();
        S.MeshIndex=Pair.Value->Profile.ObjectClass==EIBS35ObjectClass::Car?0:Pair.Value->Profile.ObjectClass==EIBS35ObjectClass::Container?1:2;
        S.Velocity=Pair.Value->Body->GetPhysicsLinearVelocity(); S.AngularVelocity=Pair.Value->Body->GetPhysicsAngularVelocityInRadians();
        if(auto* C=Pair.Value->FindComponentByClass<UIBPlayCombat>()) S.Health=C->GetHealth();
        Save->Objects.RemoveAll([&](const FIBPlayObjectState& Old){return Old.Id==S.Id;}); Save->Objects.Add(S);
    }
    for(AIBS7TThreatActor* Threat:Threats) if(IsValid(Threat))
    {
        FIBS7TThreatSnapshot S; if(!Threat->CaptureSnapshot(S)) return false;
        Save->Threats.RemoveAll([&](const FIBS7TThreatSnapshot& Old){return Old.StableId==S.StableId;}); Save->Threats.Add(S);
    }
    for(AIBPlayEncounter* Encounter:Encounters) if(IsValid(Encounter))
    {
        TArray<FIBS4PreparedSnapshot> States; if(!Encounter->CapturePreparedState(States)) return false;
        for(const auto& S:States) { Save->Prepared.RemoveAll([&](const auto& Old){return Old.ObjectId==S.ObjectId;}); Save->Prepared.Add(S); }
    }
    Out=Save; return true;
}
bool AIBPlaySession::RestoreSnapshot(const UIBPlaySave* Snapshot)
{
    if(!Snapshot||Snapshot->Version!=1||!Hero||!Planet||Hero->Grip->GetHeld()||Snapshot->Hero<0||Snapshot->Hero>2||Snapshot->HeroTransform.ContainsNaN()
        ||!Snapshot->HeroTransform.GetScale3D().Equals(FVector::OneVector,.001)||!Snapshot->HeroTransform.GetRotation().IsNormalized()
        ||Snapshot->Objects.Num()>256||Snapshot->Threats.Num()>32||Snapshot->Prepared.Num()>128) return false;
    TSet<FName> Ids;
    for(const auto& S:Snapshot->Objects)
    {
        if(S.Id.IsNone()||Ids.Contains(S.Id)||S.Transform.ContainsNaN()||!S.Transform.GetScale3D().Equals(FVector::OneVector,.001)
            || S.Velocity.ContainsNaN()||S.AngularVelocity.ContainsNaN()||!FMath::IsFinite(S.Health)||S.Health<0||S.Health>100||S.MeshIndex<0||S.MeshIndex>2) return false;
        Ids.Add(S.Id);
    }
    UIBPlaySave* Before=nullptr; if(!CaptureSnapshot(Before)) return false;
    Ids.Reset();
    for(const auto& S:Snapshot->Threats)
    {
        if(S.StableId.IsNone()||Ids.Contains(S.StableId)||S.Version!=1||S.Transform.ContainsNaN()||!FMath::IsFinite(S.Health)||S.Health<0) return false;
        Ids.Add(S.StableId);
        for(AIBS7TThreatActor* T:Threats) if(IsValid(T)&&T->StableId==S.StableId&&!T->CanRestoreSnapshot(S)) return false;
    }
    Ids.Reset();
    for(const auto& S:Snapshot->Prepared)
    {
        if(S.ObjectId.IsNone()||Ids.Contains(S.ObjectId)||S.Version!=1||S.PartTransform.ContainsNaN()||!FMath::IsFinite(S.Health)||S.Health<0||S.Health>100) return false;
        Ids.Add(S.ObjectId);
        for(AIBPlayEncounter* E:Encounters) if(IsValid(E)) for(AIBS4PreparedTarget* T:E->PreparedTargets)
            if(T->ObjectId==S.ObjectId&&!T->CanRestoreSnapshot(S)) return false;
    }
    const FVector RestorePoint=Snapshot->HeroTransform.GetLocation();
    if(!Planet->CanTraverse(RestorePoint,RestorePoint,96)) return false;
    FCollisionQueryParams RestoreQuery(SCENE_QUERY_STAT(IBPlayRestoreSnapshot),false,Hero);
    if(GetWorld()->OverlapBlockingTestByChannel(RestorePoint,Snapshot->HeroTransform.GetRotation(),ECC_Pawn,Hero->GetCapsuleComponent()->GetCollisionShape(),RestoreQuery)) return false;
    Loaded=DuplicateObject<UIBPlaySave>(Snapshot,this);
    Hero->Heroes->SelectHero(Loaded->Hero);
    for(const auto& S:Loaded->Objects) if(TObjectPtr<AIBS35Object>* Entry=Objects.Find(S.Id))
    {
        AIBS35Object* Object=Entry->Get(); if(!IsValid(Object)) continue;
        if(!Object->ClearLaunchForRestore()) return false;
        Object->Body->SetSimulatePhysics(false); Object->SetActorTransform(S.Transform,false,nullptr,ETeleportType::TeleportPhysics);
        Object->Body->SetSimulatePhysics(true); Object->Body->SetPhysicsLinearVelocity(S.Velocity); Object->Body->SetPhysicsAngularVelocityInRadians(S.AngularVelocity);
        if(auto* C=Object->FindComponentByClass<UIBPlayCombat>()) { C->ResetCombat(); C->Response->Health=S.Health; C->ContactReceiver->Health=S.Health; }
    }
    RestoreNearbyObjects();
    for(AIBS7TThreatActor* T:Threats) if(IsValid(T)) for(const auto& S:Loaded->Threats) if(S.StableId==T->StableId)
    { if(!T->RestoreSnapshot(S)) return false; RegisterTarget(T); }
    for(AIBPlayEncounter* E:Encounters) if(IsValid(E))
    {
        TArray<FIBS4PreparedSnapshot> Relevant;
        for(const auto& S:Loaded->Prepared) if(E->PreparedTargets.ContainsByPredicate([&](const AIBS4PreparedTarget* T){return T->ObjectId==S.ObjectId;})) Relevant.Add(S);
        if(!Relevant.IsEmpty()&&!E->RestorePreparedState(Relevant)) return false;
    }
    Hero->SetActorTransform(Loaded->HeroTransform,false,nullptr,ETeleportType::TeleportPhysics);
    Hero->PreviousUp=Hero->GetActorUpVector();
    Hero->TangentForward=FVector::VectorPlaneProject(Hero->GetActorForwardVector(),Hero->PreviousUp).GetSafeNormal();
    Hero->GetCharacterMovement()->StopMovementImmediately();
    Hero->GetCharacterMovement()->SetMovementMode(MOVE_Flying);
    return true;
}
void AIBPlaySession::CycleDayLength()
{
    static const double Minutes[]={18,36,72}; DayLengthIndex=(DayLengthIndex+1)%3;
    if(Life) Life->SetDayLengthMinutes(Minutes[DayLengthIndex]);
    if(Hero) Hero->ShowNotice(FString::Printf(TEXT("Un dia completo dura %.0f minutos."),Minutes[DayLengthIndex]));
}
