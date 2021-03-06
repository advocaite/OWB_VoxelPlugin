// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#include "OWB_WorldGenerator.h"
//#include "HeightMapTerrain.h"

TVoxelSharedRef<FVoxelWorldGeneratorInstance> UOWB_WorldGenerator::GetInstance()
{
	if (ensureMsgf(OpenWorldBakery != NULL, TEXT("OpenWorldBakery was not set up)"))) {
//		return nullptr;
	}

	return MakeVoxelShared<FOWB_VoxelWorldGeneratorInstance>(*this);
}

//
//void UOWB_WorldGenerator::SetLayer(const EOWBMeshBlockTypes& RenderLayer) {
//	Layer = RenderLayer;
//}


///////////////////////////////////////////////////////////////////////////////

FOWB_VoxelWorldGeneratorInstance::FOWB_VoxelWorldGeneratorInstance(const UOWB_WorldGenerator& MyGenerator)
	:OpenWorldBakery(MyGenerator.OpenWorldBakery), Generator(MyGenerator)
{
}

void FOWB_VoxelWorldGeneratorInstance::Init(const FVoxelWorldGeneratorInit& InitStruct)
{
	MaterialConfig = InitStruct.DebugMaterialConfig;
}

int FOWB_VoxelWorldGeneratorInstance::VoxelXToOWBX(const v_flt X) const {
	return FMath::RoundToInt(X + OpenWorldBakery->MapWidth / 2);
}
int FOWB_VoxelWorldGeneratorInstance::VoxelYToOWBY(const v_flt Y) const {
	return FMath::RoundToInt(Y + OpenWorldBakery->MapHeight / 2);
}

double FOWB_VoxelWorldGeneratorInstance::VoxelZToOWBZ(const v_flt Z) const {
	return Z - 2;
}
v_flt FOWB_VoxelWorldGeneratorInstance::OWBZToVoxelZ(const double Z) const {
	return Z + 2;
}

v_flt FOWB_VoxelWorldGeneratorInstance::GetValueImpl(v_flt X, v_flt Y, v_flt Z, int32 LOD, const FVoxelItemStack& Items) const
{
	int iX = VoxelXToOWBX(X);
	int iY = VoxelYToOWBY(Y);
	double iZ = VoxelZToOWBZ(Z);

	if (iX < 1 || iX >= OpenWorldBakery->MapWidth || iY < 1 || iY >= OpenWorldBakery->MapHeight) {
		return 10.0;
	}

	if (Z <= OWBHeightToVoxelHeight(OpenWorldBakery->OceanDeep)) {
		return -10;
	}

	const FOWBSquareMeter& CookedGround = OpenWorldBakery->BakedHeightMap[iX + iY * OpenWorldBakery->MapWidth];
//	const OpenWorldBakery::FSquareMeter& Ground = OpenWorldBakery->Ground(iX, iY);

	double Elevation = CookedGround.HeightByType(Generator.Layer);
	//switch (Generator.Layer)
	//{
	//case EOWBMeshBlockTypes::Ground: Elevation = Ground.GroundElevation;
	//	break;
	//case EOWBMeshBlockTypes::FreshWater: Elevation = OpenWorldBakery->GetWaterHeightAt(Ground);
	//	break;
	////case EOWBMeshBlockTypes::River: Elevation = OpenWorldBakery->GetWaterHeightAt(Ground);
	////	break;
	////case EOWBMeshBlockTypes::Ocean:
	////	break;
	//default:
	//	break;
	//}


	const float HeightInVoxels = OWBHeightToVoxelHeight(Elevation);

	// Positive value -> empty voxel
	// Negative value -> full voxel
	// The voxel value is clamped between -1 and 1. That can result in a bad gradient/normal. To solve that we divide it
	return (iZ - HeightInVoxels)/5;
}

v_flt FOWB_VoxelWorldGeneratorInstance::OWBHeightToVoxelHeight(double GroundElevation) const {
	return GroundElevation / OpenWorldBakery->CellWidth * 2;
}

int MaterialIndex(EOWBGroundSurfaceTypes Surface) {
	int MaterialD = 0;
	switch (Surface)
	{
		case EOWBGroundSurfaceTypes::Unmarked: MaterialD = 0; break;
		case EOWBGroundSurfaceTypes::Swamp: MaterialD = 1; break;
		case EOWBGroundSurfaceTypes::Grass: MaterialD = 2; break;
		case EOWBGroundSurfaceTypes::Bush: MaterialD = 3; break;
		case EOWBGroundSurfaceTypes::RockWall: MaterialD = 4; break;
		case EOWBGroundSurfaceTypes::RockFlat: MaterialD = 5; break;
		case EOWBGroundSurfaceTypes::Forest: MaterialD = 6; break;
		case EOWBGroundSurfaceTypes::LakeShore: MaterialD = 7; break;
		case EOWBGroundSurfaceTypes::LakeShallow: MaterialD = 8; break;
		case EOWBGroundSurfaceTypes::LakeBed: MaterialD = 9; break;
		case EOWBGroundSurfaceTypes::SeaShoreSand: MaterialD = 10; break;
		case EOWBGroundSurfaceTypes::SeaShoreRock: MaterialD = 11; break;
		case EOWBGroundSurfaceTypes::SeaBed: MaterialD = 12; break;
		case EOWBGroundSurfaceTypes::SeaShallowSand: MaterialD = 13; break;
		case EOWBGroundSurfaceTypes::SeaShallowRock: MaterialD = 14; break;
		case EOWBGroundSurfaceTypes::RiverShore: MaterialD = 15; break;
		case EOWBGroundSurfaceTypes::RiverShallowSand: MaterialD = 16; break;
		case EOWBGroundSurfaceTypes::RiverShallowRock: MaterialD = 17; break;
		case EOWBGroundSurfaceTypes::RiverBed: MaterialD = 18; break;
		case EOWBGroundSurfaceTypes::SpringHard: MaterialD = 19; break;
		case EOWBGroundSurfaceTypes::SpringEasy: MaterialD = 20; break;
		case EOWBGroundSurfaceTypes::LandSlidAged: MaterialD = 21; break;
		case EOWBGroundSurfaceTypes::LandSlideSmooth: MaterialD = 22; break;
		case EOWBGroundSurfaceTypes::LandSlideRocky: MaterialD = 23; break;
		case EOWBGroundSurfaceTypes::ErrorTerrain: MaterialD = 24; break;
	}
	return MaterialD;
}

FVoxelMaterial FOWB_VoxelWorldGeneratorInstance::GetMaterialImpl(v_flt X, v_flt Y, v_flt Z, int32 LOD, const FVoxelItemStack& Items) const
{
	int iX = VoxelXToOWBX(X);
	int iY = VoxelYToOWBY(Y);
	const FOWBSquareMeter& CookedGround = OpenWorldBakery->BakedHeightMap[iX + iY * OpenWorldBakery->MapWidth];

//	const OpenWorldBakery::FSquareMeter& Ground = OpenWorldBakery->Ground(iX, iY);

	// return FVoxelMaterial::CreateFromColor(FColor::Red);
	if (Generator.Layer == EOWBMeshBlockTypes::Ground) {
		if (MaterialConfig == EVoxelMaterialConfig::RGB)
			return FVoxelMaterial::CreateFromColor(OpenWorldBakery->TerrainVoxelColor(CookedGround));
		else if (MaterialConfig == EVoxelMaterialConfig::SingleIndex)
			return FVoxelMaterial::CreateFromSingleIndex(MaterialIndex(CookedGround.SurfaceType));
		else if (CookedGround.SurfaceType == CookedGround.SurfaceTypeAdditional) {
			return FVoxelMaterial::CreateFromDoubleIndex(MaterialIndex(CookedGround.SurfaceType), 0, 0.0f);
		}
		else {
			return FVoxelMaterial::CreateFromDoubleIndex(MaterialIndex(CookedGround.SurfaceType), MaterialIndex(CookedGround.SurfaceTypeAdditional), 0.3f);
		}
	}
	else {
		FVector2D NormalAsColor = CookedGround.Stream;
		//NormalAsColor.X = 0.1 * FMath::RoundToFloat(NormalAsColor.X * 10);
		NormalAsColor.Y *= -1;
		NormalAsColor = (NormalAsColor + FVector2D(1.0, 1.0)) / 2;

		float Deep = FMath::Clamp(
			(float)sqrt((CookedGround.WaterSurface - CookedGround.GroundSurface) / OpenWorldBakery->CellWidth) / 10
			, 0.0f, 1.0f
		);

		FColor Color;

		Color.R = 255 * NormalAsColor.X;
		Color.G = 255 * NormalAsColor.Y;
		Color.B = 255 * Deep;
		Color.A = 255;
		return FVoxelMaterial::CreateFromColor(Color);
	}
}

TVoxelRange<v_flt> FOWB_VoxelWorldGeneratorInstance::GetValueRangeImpl(const FIntBox& Bounds, int32 LOD, const FVoxelItemStack& Items) const
{
	if (OpenWorldBakery->Chunks.Num() == 0)
		return TVoxelRange<v_flt>::Infinite();

	FIntBox ABounds = Bounds;

	ABounds.Min.X = VoxelXToOWBX(ABounds.Min.X);
	ABounds.Max.X = VoxelXToOWBX(ABounds.Max.X);

	ABounds.Min.Y = VoxelYToOWBY(ABounds.Min.Y);
	ABounds.Max.Y = VoxelYToOWBY(ABounds.Max.Y);

	ABounds.Min.Z = VoxelZToOWBZ(ABounds.Min.Z);
	ABounds.Max.Z = VoxelZToOWBZ(ABounds.Max.Z);

	TVoxelRange<v_flt> Out = { 10.0, 10.0 };

	if (
		ABounds.Min.X > OpenWorldBakery->MapWidth || ABounds.Max.X < 0
		|| ABounds.Min.Y > OpenWorldBakery->MapHeight || ABounds.Max.Y < 0
		|| ABounds.Min.Z > 2*OpenWorldBakery->ChunksLayaut.MaxZVoxelOnMap) {
//		|| ABounds.Min.Z > FMath::Max(OpenWorldBakery->MapWidth, OpenWorldBakery->MapHeight)) {
		return Out;
	}

	//return TVoxelRange<v_flt>::Infinite();

	if (ABounds.Max.Z < OWBHeightToVoxelHeight(OpenWorldBakery->OceanDeep))
		return { -10.0,-10.0 };

	FIntBox ChunkBounds = ABounds;
	ChunkBounds.Min.X /= OpenWorldBakery->ChunksLayaut.ChunkWidth;
	ChunkBounds.Max.X /= OpenWorldBakery->ChunksLayaut.ChunkWidth;
	ChunkBounds.Min.Y /= OpenWorldBakery->ChunksLayaut.ChunkHeight;
	ChunkBounds.Max.Y /= OpenWorldBakery->ChunksLayaut.ChunkHeight;

	ChunkBounds.Min.X = FMath::Clamp(ChunkBounds.Min.X, 0, OpenWorldBakery->ChunksLayaut.XChunks - 1);
	ChunkBounds.Max.X = FMath::Clamp(ChunkBounds.Max.X, 0, OpenWorldBakery->ChunksLayaut.XChunks - 1);
	ChunkBounds.Min.Y = FMath::Clamp(ChunkBounds.Min.Y, 0, OpenWorldBakery->ChunksLayaut.YChunks - 1);
	ChunkBounds.Max.Y = FMath::Clamp(ChunkBounds.Max.Y, 0, OpenWorldBakery->ChunksLayaut.YChunks - 1);

	for (int x = ChunkBounds.Min.X; x <= ChunkBounds.Max.X; x++) {
		for (int y = ChunkBounds.Min.Y; y <= ChunkBounds.Max.Y; y++) {
			const FOWBMeshBlocks_set& ChunkData = OpenWorldBakery->Chunks[x + y * OpenWorldBakery->ChunksLayaut.XChunks];
			if (ChunkData.ChunkContents.Contains(Generator.Layer)) {
				const FOWBMeshChunk& MeshChunk = ChunkData.ChunkContents[Generator.Layer];
				if (MeshChunk.BlocksType == Generator.Layer) {
					if (!(MeshChunk.MinPoint.X >= ABounds.Max.X
						|| MeshChunk.MaxPoint.X <= ABounds.Min.X
						|| MeshChunk.MinPoint.Y >= ABounds.Max.Y
						|| MeshChunk.MaxPoint.Y <= ABounds.Min.Y)) {

						double Min_iZ = VoxelZToOWBZ(ChunkBounds.Min.Z);
						double Max_iZ = VoxelZToOWBZ(ChunkBounds.Max.Z);

						float MaxHeightInVoxels = OWBHeightToVoxelHeight(MeshChunk.MaxPoint.Z);
						float MinHeightInVoxels = OWBHeightToVoxelHeight(MeshChunk.MinPoint.Z);

						float MinVal = (Min_iZ - MaxHeightInVoxels) / 5;
						float MaxVal = (Max_iZ - MinHeightInVoxels) / 5;

						// Ok, this one incide Bounds
						if (Out.Min > MinVal)
							Out.Min = MinVal;
						if (Out.Max < MaxVal)
							Out.Max = MaxVal;
					}
				}
			}
		}
	}

	return Out;
}

FVector FOWB_VoxelWorldGeneratorInstance::GetUpVector(v_flt X, v_flt Y, v_flt Z) const
{
	// Used by spawners
	return FVector::UpVector;
}
