#include "IBPlayReviewMode.h"
#include "IBPlayCharacter.h"
#include "IBPlaySession.h"
#include "IB_S3/IBS3World.h"
#include "IB_S6H/IBS6HBridge.h"
#include "Blueprint/WidgetLayoutLibrary.h"
#include "Blueprint/UserWidget.h"
#include "Camera/CameraComponent.h"
#include "Components/DecalComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Components/LightComponent.h"
#include "Components/LineBatchComponent.h"
#include "Components/PostProcessComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkyLightComponent.h"
#include "Engine/Engine.h"
#include "Engine/GameViewportClient.h"
#include "Engine/SkeletalMesh.h"
#include "Engine/StaticMesh.h"
#include "Engine/Texture.h"
#include "Engine/TextureCube.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"
#include "UObject/UObjectIterator.h"
#include "UObject/UnrealType.h"
namespace
{
const TSet<FString>& ReviewPackages()
{
    static const TSet<FString> Packages=[] {
        TSet<FString> Result;
        const auto Add=[&](const TCHAR* Root,const TCHAR* Leaves) { TArray<FString> Names; FString(Leaves).ParseIntoArray(Names,TEXT(";")); for(const FString& Name:Names) Result.Add(FString(Root)+Name); };
        Add(TEXT("/Game/IB_S3/"),TEXT("SM_S3Planet;SM_S3NearSurface"));
        Add(TEXT("/Game/IB_S25/Materials/"),TEXT("M_Asphalt;M_Paving;M_R2_Soil;M_R2v3_Roof;M_R2v3_Metal;M_Markings"));
        Add(TEXT("/Game/IB_S25/Textures/"),TEXT("T_asphalt_basecolor;T_asphalt_normal_dx;T_asphalt_roughness;T_paving_basecolor;T_paving_normal_dx;T_paving_roughness;T_R2_Soil_basecolor;T_R2_Soil_normal_dx;T_R2_Soil_roughness"));
        Add(TEXT("/Game/IB_Donors/UrbanFacades/ImpactApartments/StaticMeshes/"),TEXT("BayWide;BayDouble;Entry;Cornice"));
        Add(TEXT("/Game/IB_Donors/UrbanFacades/ImpactApartments/Materials/modular_urban_apartments_facade_"),TEXT("glass;objects;plaster;trim_01"));
        Add(TEXT("/Game/IB_Donors/UrbanFacades/ImpactApartments/Textures/modular_urban_apartments_facade_"),TEXT("objects_diff-modular_urban_apartments_facade_objects_alpha;objects_diff-modular_urban_apartments_facade_objects_alpha1;objects_metal-modular_urban_apartments_facade_objects_rough;objects_metal-modular_urban_apartments_facade_objects_rough1;objects_nor_gl;objects_nor_gl1;plaster_diff;plaster_nor_gl;plaster_rough;trim_01_diff;trim_01_nor_gl;trim_01_rough"));
        Add(TEXT("/Game/IB_S25/Vegetation/S25_IslandTree/"),TEXT("StaticMeshes/S25_IslandTree;Materials/island_tree_01;Materials/island_tree_01_branches;Materials/island_tree_01_leaves;Textures/island_tree_01_diff;Textures/island_tree_01_nor_gl;Textures/island_tree_01_rough;Textures/island_tree_01_branches_diff;Textures/island_tree_01_branches_nor_gl;Textures/island_tree_01_branches_rough;Textures/island_tree_01_leaves_diff-island_tree_01_leaves_alpha;Textures/island_tree_01_leaves_nor_gl;Textures/island_tree_01_leaves_rough"));
        Add(TEXT("/Game/Characters/Mannequins/"),TEXT("Meshes/SKM_Manny_Simple;Materials/M_Mannequin;Materials/Manny/MI_Manny_01_New;Materials/Manny/MI_Manny_02_New;Textures/Manny/T_Manny_01_D;Textures/Manny/T_Manny_01_BN;Textures/Manny/T_Manny_01_MRA;Textures/Manny/T_Manny_02_D;Textures/Manny/T_Manny_02_BN;Textures/Manny/T_Manny_02_MRA;Textures/Shared/T_UE_Logo_M"));
        Add(TEXT("/Game/IB_Donors/CityKit/hotel/hotel/Materials/"),TEXT("agua"));
        Add(TEXT("/InterchangeAssets/gltf/Substrate/"),TEXT("M_GLTF"));
        Add(TEXT("/Engine/MapTemplates/Sky/"),TEXT("DaylightAmbientCubemap"));
        return Result;
    }();
    return Packages;
}
bool ReviewAsset(const UObject* Asset,FString& Reason)
{
    if(!Asset) return true;
    const FString Package=Asset->GetOutermost()->GetName();
    // Installed engine resources only; arbitrary /Game or Fab plugin packages fail.
    for(const TCHAR* Root:{TEXT("/Engine/BasicShapes/"),TEXT("/Engine/EngineResources/"),TEXT("/Engine/EngineMaterials/"),TEXT("/Engine/EngineTextures/")}) if(Package.StartsWith(Root)) return true;
    if(ReviewPackages().Contains(Package)) return true;
    Reason=TEXT("Unapproved review asset: ")+Asset->GetPathName(); return false;
}
}
AIBPlayReviewGameMode::AIBPlayReviewGameMode()
{ DefaultPawnClass=AIBPlayCharacter::StaticClass(); PrimaryActorTick.bCanEverTick=true; }
void AIBPlayReviewGameMode::StartPlay()
{
    // Disable drawing before BeginPlay can load the production appearance. Removal
    // commands enter the render queue before rendering is re-enabled below.
    if(GEngine&&GEngine->GameViewport) GEngine->GameViewport->bDisableWorldRendering=true;
    if(!FParse::Param(FCommandLine::Get(),TEXT("IBPlayQA"))||!GetWorld()->GetMapName().Contains(TEXT("Lvl_IB_S3_PlanetLab")))
    { UE_LOG(LogTemp,Error,TEXT("IBReview requires -IBPlayQA and PlanetLab.")); return; }
    for(TActorIterator<AIBS3PlanetWorld> It(GetWorld());It;++It) { It->bProductionVisuals=true; It->SaveSlot=TEXT("IB_Review_Unused"); }
    Super::StartPlay(); // AGameModeBase only: deliberately no production Session.
    for(TActorIterator<AIBPlayCharacter> It(GetWorld());It;++It)
    {
        It->HeroVisual->SetSkeletalMesh(nullptr); It->HeroVisual->EmptyOverrideMaterials(); It->HeroVisual->SetVisibility(false);
        It->Art=nullptr; It->Heroes->EnergyMaterial=nullptr; It->GetMesh()->SetVisibility(true);
    }
    UWidgetLayoutLibrary::RemoveAllWidgets(this);
    FString Reason; bReviewReady=ValidateScene(Reason);
    if(bReviewReady&&GEngine&&GEngine->GameViewport) GEngine->GameViewport->bDisableWorldRendering=false;
    UE_LOG(LogTemp,Display,TEXT("IBReview QA_ONLY %s %s"),bReviewReady?TEXT("SAFE_READY"):TEXT("REJECTED"),*Reason);
}
bool AIBPlayReviewGameMode::ValidateScene(FString& Reason) const
{
    for(TActorIterator<AIBPlaySession> It(GetWorld());It;++It) { Reason=TEXT("Session present"); return false; }
    for(auto& Volume:GetWorld()->GetPostProcessVolumeIterator()) { (void)Volume; Reason=TEXT("Postprocess volume present"); return false; }
    for(TObjectIterator<UUserWidget> It;It;++It) if(It->GetWorld()==GetWorld()&&It->IsInViewport()) { Reason=TEXT("Unexpected UI overlay"); return false; }
    for(TActorIterator<AActor> It(GetWorld());It;++It)
    {
        TInlineComponentArray<USceneComponent*> SceneComponents(*It);
        for(auto* Component:SceneComponents)
        {
            if(Component->IsA<UDecalComponent>()||Component->IsA<UPostProcessComponent>()||Component->IsA<UExponentialHeightFogComponent>()) { Reason=TEXT("Indirect render component present"); return false; }
            if(auto* Light=Cast<ULightComponent>(Component)) if(Light->LightFunctionMaterial||Light->IESTexture) { Reason=TEXT("Light function/profile present"); return false; }
            if(auto* Camera=Cast<UCameraComponent>(Component))
            {
                TArray<FPostProcessSettings> Extra; TArray<float> Weights; Camera->GetExtraPostProcessBlends(Extra,Weights);
                if(!Extra.IsEmpty()||!Camera->PostProcessSettings.WeightedBlendables.Array.IsEmpty()) { Reason=TEXT("Camera blendable present"); return false; }
                for(TFieldIterator<FObjectPropertyBase> P(FPostProcessSettings::StaticStruct());P;++P) if(!ReviewAsset(P->GetObjectPropertyValue_InContainer(&Camera->PostProcessSettings),Reason)) return false;
            }
        }
        TInlineComponentArray<USkyLightComponent*> Lights(*It); for(auto* Light:Lights) if(!ReviewAsset(Light->Cubemap,Reason)) return false;
        TInlineComponentArray<UPrimitiveComponent*> Components(*It);
        for(UPrimitiveComponent* Component:Components)
        {
            // Validate hidden components too; a visibility toggle cannot bypass the guard.
            if(auto* Mesh=Cast<UStaticMeshComponent>(Component)) { if(!ReviewAsset(Mesh->GetStaticMesh(),Reason)) return false; }
            else if(auto* Skeletal=Cast<USkeletalMeshComponent>(Component)) { if(!ReviewAsset(Skeletal->GetSkeletalMeshAsset(),Reason)) return false; }
            else if(Component->ShouldRender()&&!Component->IsA<ULineBatchComponent>()) { Reason=TEXT("Unknown render component: ")+Component->GetClass()->GetName(); return false; }
            TArray<UMaterialInterface*> Materials; Component->GetUsedMaterials(Materials);
            for(UMaterialInterface* Material:Materials)
            {
                for(UMaterialInterface* Parent=Material;Parent;)
                { if(!Parent->IsA<UMaterialInstanceDynamic>()&&!ReviewAsset(Parent,Reason)) return false; auto* Instance=Cast<UMaterialInstance>(Parent); Parent=Instance?Instance->Parent.Get():nullptr; }
                if(Material) { TArray<UTexture*> Textures; Material->GetUsedTextures(Textures); for(UTexture* Texture:Textures) if(!ReviewAsset(Texture,Reason)) return false; }
            }
        }
    }
    return true;
}
void AIBPlayReviewGameMode::Tick(float Dt)
{
    Super::Tick(Dt); if(!bReviewReady) return;
    FString Reason; if(!ValidateScene(Reason)) { bReviewReady=false; if(GEngine&&GEngine->GameViewport) GEngine->GameViewport->bDisableWorldRendering=true; UE_LOG(LogTemp,Error,TEXT("IBReview rejected: %s"),*Reason); }
}
