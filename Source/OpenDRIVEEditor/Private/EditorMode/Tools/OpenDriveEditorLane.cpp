#include "OpenDriveEditorLane.h"
#include "CoordTranslate.h"
#include "Components/SplineMeshComponent.h"
#include "Materials/MaterialInstanceConstant.h"

namespace OpenDriveLaneDebugHelper
{
	UMaterialInterface* GetInstancedMaterialForMesh(const roadmanager::Lane* Lane)
	{
		switch (Lane->GetLaneType())
		{
		case(roadmanager::Lane::LaneType::LANE_TYPE_DRIVING):
			if (Lane->GetId() > 0)
			{
				return LoadObject<UMaterialInstance>(nullptr, TEXT("/OpenDRIVE/EditorResources/Materials/MI_LeftRoad"));
			}
			return LoadObject<UMaterialInstance>(nullptr, TEXT("/OpenDRIVE/EditorResources/Materials/MI_RightRoad"));
		case(roadmanager::Lane::LaneType::LANE_TYPE_SIDEWALK):
			return LoadObject<UMaterialInstance>(nullptr, TEXT("/OpenDRIVE/EditorResources/Materials/MI_Sidewalk"));
		case(roadmanager::Lane::LaneType::LANE_TYPE_SHOULDER):
			return LoadObject<UMaterialInstance>(nullptr, TEXT("/OpenDRIVE/EditorResources/Materials/MI_Shoulder"));
		case(roadmanager::Lane::LaneType::LANE_TYPE_PARKING):
			return LoadObject<UMaterialInstance>(nullptr, TEXT("/OpenDRIVE/EditorResources/Materials/MI_Parking"));
		case(roadmanager::Lane::LaneType::LANE_TYPE_BIKING):
			return LoadObject<UMaterialInstance>(nullptr, TEXT("/OpenDRIVE/EditorResources/Materials/MI_Biking"));
		case(roadmanager::Lane::LaneType::LANE_TYPE_RESTRICTED):
			return LoadObject<UMaterialInstance>(nullptr, TEXT("/OpenDRIVE/EditorResources/Materials/MI_RestrictedRoad"));
		default:
			return LoadObject<UMaterial>(nullptr, TEXT("/OpenDRIVE/EditorResources/Materials/M_LaneSplineEd"));
		}
	}
}

// Sets default values
AOpenDriveEditorLane::AOpenDriveEditorLane(): Road(nullptr), LaneSection(nullptr), Lane(nullptr)
{
	PrimaryActorTick.bCanEverTick = false;
	bEditable = true;
	LaneMeshPtr = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/EditorLandscapeResources/SplineEditorMesh"));
	BaseMeshSize = LaneMeshPtr->GetBoundingBox().GetSize().Y; // The mesh's width. Used to set our lanes widths correctly.
	USceneComponent* RootComp = CreateDefaultSubobject<USceneComponent>(FName("Root"));
	RootComponent = RootComp;
	RootComponent->SetMobility(EComponentMobility::Static);
	bIsSelectableInEditor = true;
	bIsEditorOnlyActor = true;
}

void AOpenDriveEditorLane::Initialize(roadmanager::Road* RoadIn, roadmanager::LaneSection* LaneSectionIn, roadmanager::Lane* LaneIn, const float Offset, const float Step)
{
	Road = RoadIn;
	LaneSection = LaneSectionIn;
	Lane = LaneIn;
	DrawLane(Step, Offset);
}

void AOpenDriveEditorLane::DrawLane(const double Step, const float Offset)
{
	// Set Odr position
	roadmanager::Position Position;
	
	double LaneLength = LaneSection->GetLength();
	double s = LaneSection->GetS();
	
	Position.Init();
	Position.SetSnapLaneTypes(roadmanager::Lane::LANE_TYPE_ANY);
	RoadDirection = Position.GetDrivingDirectionRelativeRoad();
	
	// Spline component creation
	USplineComponent* LaneSpline = NewObject<USplineComponent>(this);
	LaneSpline->SetupAttachment(RootComponent);
	LaneSpline->RegisterComponent();
	LaneSpline->ClearSplinePoints();
	
	// Start point
	Position.Init();
	Position.SetSnapLaneTypes(roadmanager::Lane::LANE_TYPE_ANY);
	SetLanePoint(LaneSpline, Position, s, Offset);
	
	//Driving direction
	RoadDirection = Position.GetDrivingDirectionRelativeRoad();

	// Add a lane spline point every Step meters
	s+= Step;
	for (s ; s < LaneSection->GetS() + LaneLength; s += Step)
	{
		SetLanePoint(LaneSpline, Position, s, Offset);
	}

	// Final point
	SetLanePoint(LaneSpline, Position, LaneSection->GetS() + LaneLength, Offset);
	if (LaneLength > Step)
	{
		CheckLastTwoPointsDistance(LaneSpline, Step);
	}
	
	SetColoredLaneMeshes(LaneSpline);
}

FString AOpenDriveEditorLane::GetLaneType() const
{
	FString Type;

	switch (Lane->GetLaneType())
	{
	case(roadmanager::Lane::LaneType::LANE_TYPE_DRIVING):
		Type = "Driving road";
		break;
	case(roadmanager::Lane::LaneType::LANE_TYPE_BIKING):
		Type = "Bike path";
		break;
	case(roadmanager::Lane::LaneType::LANE_TYPE_SIDEWALK):
		Type = "Sidewalk lane";
		break;
	case(roadmanager::Lane::LaneType::LANE_TYPE_PARKING):
		Type = "Parking slot(s)";
		break;
	case(roadmanager::Lane::LaneType::LANE_TYPE_BORDER):
		Type = "Border";
		break;
	case(roadmanager::Lane::LaneType::LANE_TYPE_RAIL):
		Type = "Rail";
		break;
	case(roadmanager::Lane::LaneType::LANE_TYPE_TRAM):
		Type = "Tram";
		break;
	case(roadmanager::Lane::LaneType::LANE_TYPE_SHOULDER):
		Type = "Shoulder";
		break;
	case(roadmanager::Lane::LaneType::LANE_TYPE_RESTRICTED):
		Type = "Restricted lane";
		break;
	case(roadmanager::Lane::LaneType::LANE_TYPE_MEDIAN):
		Type = "Median";
		break;
	default:
		Type = "None";
		break;
	}

	return Type;
}

int AOpenDriveEditorLane::GetSuccessorId() const
{
	const roadmanager::RoadLink* Link = Road->GetLink(roadmanager::LinkType::SUCCESSOR);
	return  Link != nullptr ? Link->GetElementId() :  -1;
}

int AOpenDriveEditorLane::GetPredecessorId() const
{
	const roadmanager::RoadLink* Link = Road->GetLink(roadmanager::LinkType::PREDECESSOR);
	return  Link != nullptr ? Link->GetElementId() : -1;
}

void AOpenDriveEditorLane::SetLanePoint(USplineComponent* LaneSpline, roadmanager::Position& Position, const double S, const float Offset) const
{
	Position.SetLanePos(GetRoadId(), GetLaneId(), S, 0.);

	FVector SP = CoordTranslate::OdrToUe::ToLocation(Position);
	SP.Z += Offset;
	LaneSpline->AddSplineWorldPoint(SP);
	const FRotator Rotator = CoordTranslate::OdrToUe::ToRotation(Position);
	LaneSpline->SetRotationAtSplinePoint(LaneSpline->GetNumberOfSplinePoints() - 1, Rotator, ESplineCoordinateSpace::World);

	const float Yscale = ( (LaneSection->GetWidth(Position.GetS(), Lane->GetId()) * 100) / BaseMeshSize) * 0.8f;
	LaneSpline->SetScaleAtSplinePoint(LaneSpline->GetNumberOfSplinePoints() - 1, FVector(1.0f, Yscale, 1.0f));
}

void AOpenDriveEditorLane::CheckLastTwoPointsDistance(USplineComponent* LaneSpline, const float Step)
{
	const float Dist = FVector::Distance(LaneSpline->GetWorldLocationAtSplinePoint(LaneSpline->GetNumberOfSplinePoints() - 2),
		LaneSpline->GetWorldLocationAtSplinePoint(LaneSpline->GetNumberOfSplinePoints() - 1));
	if (Dist / 100 < Step / 3)
	{
		LaneSpline->RemoveSplinePoint(LaneSpline->GetNumberOfSplinePoints() - 2);
	}
}

void AOpenDriveEditorLane::SetColoredLaneMeshes(USplineComponent* LaneSpline)
{	
	for (int i = 0; i < LaneSpline->GetNumberOfSplinePoints() - 1; i++)
	{
		USplineMeshComponent* NewSplineMesh = NewObject<USplineMeshComponent>(this);
		NewSplineMesh->SetupAttachment(RootComponent);
		NewSplineMesh->SetMobility(EComponentMobility::Static);
		NewSplineMesh->SetStaticMesh(LaneMeshPtr);
		NewSplineMesh->SetForwardAxis(ESplineMeshAxis::X);
		
		UMaterialInterface* Material = OpenDriveLaneDebugHelper::GetInstancedMaterialForMesh(Lane);
		UMaterialInstanceConstant* MaterialConst = NewObject<UMaterialInstanceConstant>(this, UMaterialInstanceConstant::StaticClass(), NAME_None, RF_Transient);
		MaterialConst->SetParentEditorOnly(Material);
		MaterialConst->SetScalarParameterValueEditorOnly(TEXT("RoadDirection"), RoadDirection);
		NewSplineMesh->SetMaterial(0, MaterialConst);
		
		FSplineMeshParams SplineMeshParams;
		SplineMeshParams.StartPos = LaneSpline->GetLocationAtSplinePoint(i, ESplineCoordinateSpace::Local);
		SplineMeshParams.StartTangent = LaneSpline->GetTangentAtSplinePoint(i, ESplineCoordinateSpace::Local);
		SplineMeshParams.StartScale = FVector2D(LaneSpline->GetScaleAtSplinePoint(i).Y, LaneSpline->GetScaleAtSplinePoint(i).Z);
		SplineMeshParams.EndPos = LaneSpline->GetLocationAtSplinePoint(i + 1, ESplineCoordinateSpace::Local);
		SplineMeshParams.EndTangent = LaneSpline->GetTangentAtSplinePoint(i + 1, ESplineCoordinateSpace::Local);
		SplineMeshParams.EndScale = FVector2D(LaneSpline->GetScaleAtSplinePoint(i + 1).Y, LaneSpline->GetScaleAtSplinePoint(i + 1).Z);

		// Turn off all collision
		NewSplineMesh->SetCollisionProfileName(UCollisionProfile::NoCollision_ProfileName);
		NewSplineMesh->RegisterComponent();
		NewSplineMesh->SplineParams = SplineMeshParams;

		// Update spline mesh render and register it so it can be visible in editor viewport
		NewSplineMesh->UpdateRenderStateAndCollision();
	}

	// To regain some performance, we can destroy the spline. Now that we have generated the meshes, spline points are no longer needed.
	LaneSpline->DestroyComponent();
}
