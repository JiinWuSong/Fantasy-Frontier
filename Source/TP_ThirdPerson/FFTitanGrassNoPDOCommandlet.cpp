#include "FFTitanGrassNoPDOCommandlet.h"

#if WITH_EDITOR
#include "AssetToolsModule.h"
#include "FileHelpers.h"
#include "Factories/MaterialFactoryNew.h"
#include "Factories/MaterialInstanceConstantFactoryNew.h"
#include "MaterialEditingLibrary.h"
#include "Materials/Material.h"
#include "Materials/MaterialAttributeDefinitionMap.h"
#include "Materials/MaterialExpressionAdd.h"
#include "Materials/MaterialExpressionBlendMaterialAttributes.h"
#include "Materials/MaterialExpressionClamp.h"
#include "Materials/MaterialExpressionConstant.h"
#include "Materials/MaterialExpressionConstant3Vector.h"
#include "Materials/MaterialExpressionCustom.h"
#include "Materials/MaterialExpressionLocalPosition.h"
#include "Materials/MaterialExpressionMakeMaterialAttributes.h"
#include "Materials/MaterialExpressionMaterialFunctionCall.h"
#include "Materials/MaterialExpressionMultiply.h"
#include "Materials/MaterialExpressionPerInstanceRandom.h"
#include "Materials/MaterialExpressionRuntimeVirtualTextureOutput.h"
#include "Materials/MaterialExpressionRuntimeVirtualTextureSample.h"
#include "Materials/MaterialExpressionSetMaterialAttributes.h"
#include "Materials/MaterialExpressionTime.h"
#include "Materials/MaterialExpressionVertexNormalWS.h"
#include "Materials/MaterialExpressionWorldPosition.h"
#include "Materials/MaterialFunction.h"
#include "Materials/MaterialInstanceConstant.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Modules/ModuleManager.h"
#include "VT/RuntimeVirtualTexture.h"
#endif

UFFTitanGrassNoPDOCommandlet::UFFTitanGrassNoPDOCommandlet()
{
	LogToConsole = true;
	IsClient = false;
	IsEditor = true;
	IsServer = false;
}

int32 UFFTitanGrassNoPDOCommandlet::Main(const FString& Params)
{
#if WITH_EDITOR
	const TCHAR* SourceParentPath = TEXT("/Game/Environment/Foliage/Materials/M_GrassBlade.M_GrassBlade");
	const TCHAR* OutputFolderPath = TEXT("/Game/FantasyFrontier/Test/Materials");

	UMaterial* SourceParent = LoadObject<UMaterial>(nullptr, SourceParentPath);
	if (!SourceParent)
	{
		UE_LOG(LogTemp, Error, TEXT("FFTitanGrassNoPDOCommandlet: missing source parent %s"), SourceParentPath);
		return 1;
	}

	FAssetToolsModule& AssetToolsModule = FModuleManager::LoadModuleChecked<FAssetToolsModule>(TEXT("AssetTools"));

	auto CreateOrLoadMaterial = [&](const TCHAR* AssetName) -> UMaterial*
	{
		const FString ObjectPath = FString::Printf(TEXT("%s/%s.%s"), OutputFolderPath, AssetName, AssetName);
		if (UMaterial* Existing = LoadObject<UMaterial>(nullptr, *ObjectPath))
		{
			Existing->Modify();
			UMaterialEditingLibrary::DeleteAllMaterialExpressions(Existing);
			return Existing;
		}

		UMaterialFactoryNew* Factory = NewObject<UMaterialFactoryNew>();
		return Cast<UMaterial>(AssetToolsModule.Get().CreateAsset(AssetName, OutputFolderPath, UMaterial::StaticClass(), Factory));
	};

	auto FinalizeMaterial = [](UMaterial* Material)
	{
		if (!Material)
		{
			return;
		}
		UMaterialEditingLibrary::LayoutMaterialExpressions(Material);
		UMaterialEditingLibrary::RecompileMaterial(Material);
		Material->PostEditChange();
		Material->MarkPackageDirty();
	};

	auto AddWorldPositionNode = [](UMaterial* Material, const int32 X, const int32 Y) -> UMaterialExpressionWorldPosition*
	{
		UMaterialExpressionWorldPosition* WorldPosition = Cast<UMaterialExpressionWorldPosition>(
			UMaterialEditingLibrary::CreateMaterialExpression(Material, UMaterialExpressionWorldPosition::StaticClass(), X, Y));
		if (WorldPosition)
		{
			WorldPosition->WorldPositionShaderOffset = WPT_ExcludeAllShaderOffsets;
		}
		return WorldPosition;
	};

	auto ConnectCustomInput = [](UMaterialExpressionCustom* CustomNode, UMaterialExpression* InputExpression, const FName InputName)
	{
		if (!CustomNode || !InputExpression)
		{
			return;
		}

		FCustomInput CustomInput;
		CustomInput.InputName = InputName;
		CustomInput.Input.Expression = InputExpression;
		CustomNode->Inputs.Add(CustomInput);
	};

	enum class EAttrDiagFeature : uint8
	{
		Baseline,
		BaseColor,
		OpacityMask,
		Normals,
		Wind,
		FoliageInteractionBypass,
		RVTGroundTint,
		DepthOffset,
		Combined,
		SafeMaterialAttributes
	};

	auto BuildAttributeDiagnosticMaterial = [&](const TCHAR* AssetName, const EAttrDiagFeature Feature) -> UMaterial*
	{
		UMaterial* Material = CreateOrLoadMaterial(AssetName);
		if (!Material)
		{
			UE_LOG(LogTemp, Error, TEXT("FFTitanGrassNoPDOCommandlet: failed to create/load diagnostic material %s"), AssetName);
			return nullptr;
		}

		Material->Modify();
		Material->PreEditChange(nullptr);
		const bool bSafeMaterialAttributes = Feature == EAttrDiagFeature::SafeMaterialAttributes;
		Material->bUseMaterialAttributes = bSafeMaterialAttributes;
		Material->BlendMode = Feature == EAttrDiagFeature::OpacityMask ? BLEND_Masked : BLEND_Opaque;
		Material->TwoSided = true;
		Material->SetShadingModel(MSM_TwoSidedFoliage);
		Material->bUsedWithInstancedStaticMeshes = true;
		Material->bUsedWithNanite = true;
		Material->OpacityMaskClipValue = 0.3333f;

		UMaterialExpressionWorldPosition* WorldPosition = AddWorldPositionNode(Material, -920, -190);
		UMaterialExpressionLocalPosition* LocalPosition = Cast<UMaterialExpressionLocalPosition>(
			UMaterialEditingLibrary::CreateMaterialExpression(Material, UMaterialExpressionLocalPosition::StaticClass(), -920, -40));
		UMaterialExpressionPerInstanceRandom* InstanceRandom = Cast<UMaterialExpressionPerInstanceRandom>(
			UMaterialEditingLibrary::CreateMaterialExpression(Material, UMaterialExpressionPerInstanceRandom::StaticClass(), -920, 120));
		UMaterialExpressionTime* Time = Cast<UMaterialExpressionTime>(
			UMaterialEditingLibrary::CreateMaterialExpression(Material, UMaterialExpressionTime::StaticClass(), -920, 275));
		UMaterialExpressionCustom* ColorNode = Cast<UMaterialExpressionCustom>(
			UMaterialEditingLibrary::CreateMaterialExpression(Material, UMaterialExpressionCustom::StaticClass(), -560, -145));
		UMaterialExpressionConstant* Roughness = Cast<UMaterialExpressionConstant>(
			UMaterialEditingLibrary::CreateMaterialExpression(Material, UMaterialExpressionConstant::StaticClass(), -250, 160));
		UMaterialExpressionConstant* Specular = Cast<UMaterialExpressionConstant>(
			UMaterialEditingLibrary::CreateMaterialExpression(Material, UMaterialExpressionConstant::StaticClass(), -250, 260));

		if (!WorldPosition || !LocalPosition || !InstanceRandom || !Time || !ColorNode || !Roughness || !Specular)
		{
			UE_LOG(LogTemp, Error, TEXT("FFTitanGrassNoPDOCommandlet: diagnostic material graph allocation failed for %s"), AssetName);
			return nullptr;
		}

		const bool bTitanColor = Feature != EAttrDiagFeature::Baseline;
		const bool bRVTGroundTint = Feature == EAttrDiagFeature::RVTGroundTint
			|| Feature == EAttrDiagFeature::Combined
			|| Feature == EAttrDiagFeature::SafeMaterialAttributes;
		ColorNode->Description = bSafeMaterialAttributes
			? FString::Printf(TEXT("%s safe MakeMaterialAttributes color"), AssetName)
			: FString::Printf(TEXT("%s explicit-output grass color; no MaterialAttributes path"), AssetName);
		ColorNode->OutputType = CMOT_Float3;
		ConnectCustomInput(ColorNode, WorldPosition, TEXT("WorldPos"));
		ConnectCustomInput(ColorNode, LocalPosition, TEXT("LocalPos"));
		ConnectCustomInput(ColorNode, InstanceRandom, TEXT("InstanceRandom"));
		ColorNode->Code = bTitanColor ? (bRVTGroundTint ? TEXT(R"(
float2 p = WorldPos.xy;
float bladeHeight = smoothstep(-4.0, 72.0, LocalPos.z);
float broad = 0.5 + 0.5 * sin(p.x / 47000.0 + p.y / 71000.0 + sin(p.y / 53000.0));
float mid = 0.5 + 0.5 * sin(p.x / 9700.0 - p.y / 13800.0 + InstanceRandom * 6.28318);
float3 titanGround = lerp(float3(0.335, 0.455, 0.150), float3(0.605, 0.640, 0.292), broad * 0.70 + 0.15);
float3 rvtLikeDarkening = lerp(titanGround, titanGround * float3(0.76, 0.84, 0.70), saturate((1.0 - broad) * 0.28));
float3 rootColor = rvtLikeDarkening * float3(0.94, 0.99, 0.90);
float3 tipColor = lerp(rvtLikeDarkening, float3(0.660, 0.640, 0.335), saturate(0.12 + mid * 0.12 + InstanceRandom * 0.035));
return lerp(rootColor, tipColor, bladeHeight * 0.46);
)") : TEXT(R"(
float2 p = WorldPos.xy;
float bladeHeight = smoothstep(-4.0, 72.0, LocalPos.z);
float broad = 0.5 + 0.5 * sin(p.x / 47000.0 + p.y / 71000.0 + sin(p.y / 53000.0));
float mid = 0.5 + 0.5 * sin(p.x / 9700.0 - p.y / 13800.0 + InstanceRandom * 6.28318);
float3 ground = lerp(float3(0.380, 0.515, 0.165), float3(0.620, 0.640, 0.292), broad * 0.68 + 0.18);
float3 rootColor = ground * float3(0.93, 0.98, 0.88);
float3 tipColor = lerp(ground, float3(0.660, 0.635, 0.335), saturate(0.10 + mid * 0.10 + InstanceRandom * 0.03));
return lerp(rootColor, tipColor, bladeHeight * 0.42);
)")) : TEXT("return float3(0.58, 0.66, 0.28);");

		Roughness->R = 0.94f;
		Specular->R = 0.04f;
		UMaterialExpressionMakeMaterialAttributes* MakeAttributes = nullptr;
		if (bSafeMaterialAttributes)
		{
			MakeAttributes = Cast<UMaterialExpressionMakeMaterialAttributes>(
				UMaterialEditingLibrary::CreateMaterialExpression(Material, UMaterialExpressionMakeMaterialAttributes::StaticClass(), 70, -80));
			if (!MakeAttributes)
			{
				UE_LOG(LogTemp, Error, TEXT("FFTitanGrassNoPDOCommandlet: failed to create MakeMaterialAttributes for %s"), AssetName);
				return nullptr;
			}
			MakeAttributes->BaseColor.Connect(0, ColorNode);
			MakeAttributes->SubsurfaceColor.Connect(0, ColorNode);
			MakeAttributes->Roughness.Connect(0, Roughness);
			MakeAttributes->Specular.Connect(0, Specular);
			if (UMaterialEditorOnlyData* EditorOnlyData = Material->GetEditorOnlyData())
			{
				EditorOnlyData->MaterialAttributes.Connect(0, MakeAttributes);
			}
		}
		else
		{
			UMaterialEditingLibrary::ConnectMaterialProperty(ColorNode, TEXT(""), MP_BaseColor);
			UMaterialEditingLibrary::ConnectMaterialProperty(ColorNode, TEXT(""), MP_SubsurfaceColor);
			UMaterialEditingLibrary::ConnectMaterialProperty(Roughness, TEXT(""), MP_Roughness);
			UMaterialEditingLibrary::ConnectMaterialProperty(Specular, TEXT(""), MP_Specular);
		}

		if (Feature == EAttrDiagFeature::OpacityMask
			|| Feature == EAttrDiagFeature::Combined
			|| Feature == EAttrDiagFeature::SafeMaterialAttributes)
		{
			UMaterialExpressionCustom* MaskNode = Cast<UMaterialExpressionCustom>(
				UMaterialEditingLibrary::CreateMaterialExpression(Material, UMaterialExpressionCustom::StaticClass(), -250, 360));
			if (MaskNode)
			{
				MaskNode->Description = TEXT("Explicit full opacity mask; verifies opacity/mask output does not hide grass.");
				MaskNode->OutputType = CMOT_Float1;
				MaskNode->Code = TEXT("return 1.0;");
				if (MakeAttributes)
				{
					MakeAttributes->OpacityMask.Connect(0, MaskNode);
				}
				else
				{
					UMaterialEditingLibrary::ConnectMaterialProperty(MaskNode, TEXT(""), MP_OpacityMask);
				}
			}
		}

		if (Feature == EAttrDiagFeature::Normals
			|| Feature == EAttrDiagFeature::Combined
			|| Feature == EAttrDiagFeature::SafeMaterialAttributes)
		{
			UMaterialExpressionConstant3Vector* NormalNode = Cast<UMaterialExpressionConstant3Vector>(
				UMaterialEditingLibrary::CreateMaterialExpression(Material, UMaterialExpressionConstant3Vector::StaticClass(), -250, 460));
			if (NormalNode)
			{
				NormalNode->Constant = FLinearColor(0.0f, 0.0f, 1.0f);
				if (MakeAttributes)
				{
					MakeAttributes->Normal.Connect(0, NormalNode);
				}
				else
				{
					UMaterialEditingLibrary::ConnectMaterialProperty(NormalNode, TEXT(""), MP_Normal);
				}
			}
		}

		if (Feature == EAttrDiagFeature::Wind
			|| Feature == EAttrDiagFeature::FoliageInteractionBypass
			|| Feature == EAttrDiagFeature::Combined
			|| Feature == EAttrDiagFeature::SafeMaterialAttributes)
		{
			UMaterialExpressionCustom* WindNode = Cast<UMaterialExpressionCustom>(
				UMaterialEditingLibrary::CreateMaterialExpression(Material, UMaterialExpressionCustom::StaticClass(), -560, 210));
			if (WindNode)
			{
				WindNode->Description = Feature == EAttrDiagFeature::FoliageInteractionBypass
					? TEXT("Root-fixed WPO plus explicit zeroed FoliageInteraction contribution")
					: TEXT("Root-fixed cook-safe WPO wind");
				WindNode->OutputType = CMOT_Float3;
				ConnectCustomInput(WindNode, WorldPosition, TEXT("WorldPos"));
				ConnectCustomInput(WindNode, LocalPosition, TEXT("LocalPos"));
				ConnectCustomInput(WindNode, Time, TEXT("Time"));
				ConnectCustomInput(WindNode, InstanceRandom, TEXT("InstanceRandom"));
				WindNode->Code = TEXT(R"(
float rootMask = smoothstep(3.0, 80.0, LocalPos.z);
rootMask *= rootMask;
float randomPhase = InstanceRandom * 6.28318;
float2 baseDir = normalize(float2(0.72, 0.42));
float angle = (InstanceRandom - 0.5) * 0.75;
float s = sin(angle);
float c = cos(angle);
float2 windDir = normalize(float2(baseDir.x * c - baseDir.y * s, baseDir.x * s + baseDir.y * c));
float gust = 0.70 + 0.30 * sin(dot(WorldPos.xy, float2(0.0016, 0.0011)) + Time * 0.55 + randomPhase);
float sway = sin(dot(WorldPos.xy, float2(0.009, 0.013)) + Time * 1.15 + randomPhase);
float flutter = sin(dot(WorldPos.xy, float2(-0.016, 0.006)) + Time * 1.95 + randomPhase * 0.37) * 0.22;
float bend = (sway + flutter) * gust * lerp(2.1, 5.0, InstanceRandom) * rootMask;
return float3(windDir.x * bend, windDir.y * bend, 0.0);
)");
				if (MakeAttributes)
				{
					MakeAttributes->WorldPositionOffset.Connect(0, WindNode);
				}
				else
				{
					UMaterialEditingLibrary::ConnectMaterialProperty(WindNode, TEXT(""), MP_WorldPositionOffset);
				}
			}
		}

		if (Feature == EAttrDiagFeature::DepthOffset || Feature == EAttrDiagFeature::SafeMaterialAttributes)
		{
			UMaterialExpressionConstant* DepthNode = Cast<UMaterialExpressionConstant>(
				UMaterialEditingLibrary::CreateMaterialExpression(Material, UMaterialExpressionConstant::StaticClass(), -250, 560));
			if (DepthNode)
			{
				DepthNode->R = 0.0f;
				if (MakeAttributes)
				{
					MakeAttributes->PixelDepthOffset.Connect(0, DepthNode);
				}
				else
				{
					UMaterialEditingLibrary::ConnectMaterialProperty(DepthNode, TEXT(""), MP_PixelDepthOffset);
				}
			}
		}

		FinalizeMaterial(Material);
		UE_LOG(LogTemp, Display,
			TEXT("FFTitanGrassNoPDOCommandlet: diagnostic=%s attributes=%d wpo=%d mask=%d pdo=%d"),
			*GetPathNameSafe(Material),
			Material->bUseMaterialAttributes ? 1 : 0,
			Material->HasVertexPositionOffsetConnected() ? 1 : 0,
			Material->IsPropertyConnected(MP_OpacityMask) ? 1 : 0,
			Material->HasPixelDepthOffsetConnected() ? 1 : 0);
		return Material;
	};

	struct FAttrDiagMaterial
	{
		const TCHAR* AssetName;
		EAttrDiagFeature Feature;
	};

	const FAttrDiagMaterial AttrDiagnostics[] = {
		{ TEXT("M_FF_TitanGrassAttr_B_NoAttributes"), EAttrDiagFeature::Baseline },
		{ TEXT("M_FF_TitanGrassAttr_C_BaseColor"), EAttrDiagFeature::BaseColor },
		{ TEXT("M_FF_TitanGrassAttr_D_OpacityMask"), EAttrDiagFeature::OpacityMask },
		{ TEXT("M_FF_TitanGrassAttr_E_Normals"), EAttrDiagFeature::Normals },
		{ TEXT("M_FF_TitanGrassAttr_F_Wind"), EAttrDiagFeature::Wind },
		{ TEXT("M_FF_TitanGrassAttr_G_FoliageInteractionBypass"), EAttrDiagFeature::FoliageInteractionBypass },
		{ TEXT("M_FF_TitanGrassAttr_H_RVTGroundTint"), EAttrDiagFeature::RVTGroundTint },
		{ TEXT("M_FF_TitanGrassAttr_I_DepthOffset"), EAttrDiagFeature::DepthOffset },
		{ TEXT("M_FF_TitanGrassAttr_J_CombinedSafe"), EAttrDiagFeature::Combined },
		{ TEXT("M_FF_TitanGrassAttr_K_SafeMaterialAttributes"), EAttrDiagFeature::SafeMaterialAttributes },
	};

	for (const FAttrDiagMaterial& Diagnostic : AttrDiagnostics)
	{
		if (!BuildAttributeDiagnosticMaterial(Diagnostic.AssetName, Diagnostic.Feature))
		{
			return 1;
		}
	}

	TArray<FString> GraphDumpLines;
	auto AddGraphLine = [&GraphDumpLines](const FString& Line)
	{
		GraphDumpLines.Add(Line);
	};

	auto BlendTypeToString = [](const EMaterialAttributeBlend::Type BlendType) -> FString
	{
		switch (BlendType)
		{
		case EMaterialAttributeBlend::Blend:
			return TEXT("Blend");
		case EMaterialAttributeBlend::UseA:
			return TEXT("UseA");
		case EMaterialAttributeBlend::UseB:
			return TEXT("UseB");
		default:
			return FString::Printf(TEXT("Unknown(%d)"), static_cast<int32>(BlendType));
		}
	};

	auto RVTMaterialTypeToString = [](const ERuntimeVirtualTextureMaterialType MaterialType) -> FString
	{
		if (const UEnum* Enum = StaticEnum<ERuntimeVirtualTextureMaterialType>())
		{
			return Enum->GetNameStringByValue(static_cast<int64>(MaterialType));
		}
		return FString::Printf(TEXT("Unknown(%d)"), static_cast<int32>(MaterialType));
	};

	auto DescribeExpression = [&RVTMaterialTypeToString](UMaterialExpression* Expression) -> FString
	{
		if (!Expression)
		{
			return TEXT("<null>");
		}

		TArray<FString> Captions;
		Expression->GetCaption(Captions);
		FString Extra;
		if (const UMaterialExpressionMaterialFunctionCall* FunctionCall = Cast<UMaterialExpressionMaterialFunctionCall>(Expression))
		{
			Extra = FString::Printf(TEXT(" function=%s"), *GetPathNameSafe(FunctionCall->MaterialFunction));
		}
		else if (const UMaterialExpressionRuntimeVirtualTextureSample* RVTSample = Cast<UMaterialExpressionRuntimeVirtualTextureSample>(Expression))
		{
			URuntimeVirtualTexture* RVT = RVTSample->VirtualTexture;
			Extra = FString::Printf(
				TEXT(" vt=%s sampleType=%s vtType=%s samplePacked=%d vtPacked=%d sampleAdaptive=%d vtAdaptive=%d worldOrigin=%d mipMode=%d"),
				*GetPathNameSafe(RVT),
				*RVTMaterialTypeToString(RVTSample->MaterialType),
				RVT ? *RVTMaterialTypeToString(RVT->GetMaterialType()) : TEXT("<none>"),
				RVTSample->bSinglePhysicalSpace ? 1 : 0,
				RVT && RVT->GetSinglePhysicalSpace() ? 1 : 0,
				RVTSample->bAdaptive ? 1 : 0,
				RVT && RVT->GetAdaptivePageTable() ? 1 : 0,
				static_cast<int32>(RVTSample->WorldPositionOriginType),
				static_cast<int32>(RVTSample->MipValueMode));
		}
		return FString::Printf(TEXT("%s name=%s caption=\"%s\" desc=\"%s\"%s"),
			*Expression->GetClass()->GetName(),
			*Expression->GetName(),
			*FString::Join(Captions, TEXT(" | ")),
			*Expression->Desc,
			*Extra);
	};

	auto DescribeInput = [&DescribeExpression](const FExpressionInput& Input) -> FString
	{
		if (!Input.Expression)
		{
			return TEXT("<unconnected>");
		}

		FExpressionInput TracedInput = Input.GetTracedInput();
		UMaterialExpression* SourceExpression = TracedInput.Expression ? TracedInput.Expression : Input.Expression;
		return FString::Printf(TEXT("%s output=%d mask=%d%d%d%d%d"),
			*DescribeExpression(SourceExpression),
			TracedInput.OutputIndex,
			TracedInput.Mask,
			TracedInput.MaskR,
			TracedInput.MaskG,
			TracedInput.MaskB,
			TracedInput.MaskA);
	};

	TFunction<void(const FString&, TConstArrayView<TObjectPtr<UMaterialExpression>>, int32)> DumpExpressions;
	TSet<const UObject*> VisitedFunctionObjects;
	DumpExpressions = [&](const FString& OwnerLabel, TConstArrayView<TObjectPtr<UMaterialExpression>> Expressions, const int32 Depth)
	{
		const FString Indent = FString::ChrN(Depth * 2, TEXT(' '));
		AddGraphLine(FString::Printf(TEXT("%s--- %s expressionCount=%d ---"), *Indent, *OwnerLabel, Expressions.Num()));
		for (int32 Index = 0; Index < Expressions.Num(); ++Index)
		{
			UMaterialExpression* Expression = Expressions[Index];
			if (!Expression)
			{
				continue;
			}

			AddGraphLine(FString::Printf(TEXT("%sexpr[%d] %s resultMaterialAttributes=%d outputCount=%d"),
				*Indent,
				Index,
				*DescribeExpression(Expression),
				Expression->IsResultMaterialAttributes(0) ? 1 : 0,
				Expression->Outputs.Num()));

			TArrayView<FExpressionInput*> Inputs = Expression->GetInputsView();
			for (int32 InputIndex = 0; InputIndex < Inputs.Num(); ++InputIndex)
			{
				FExpressionInput* Input = Inputs[InputIndex];
				if (!Input)
				{
					continue;
				}
				AddGraphLine(FString::Printf(TEXT("%s  input[%d] %s -> %s"),
					*Indent,
					InputIndex,
					*Expression->GetInputName(InputIndex).ToString(),
					*DescribeInput(*Input)));
			}

			if (const UMaterialExpressionSetMaterialAttributes* SetAttributes = Cast<UMaterialExpressionSetMaterialAttributes>(Expression))
			{
				AddGraphLine(FString::Printf(TEXT("%s  SetMaterialAttributes attributeCount=%d inputCount=%d"),
					*Indent,
					SetAttributes->AttributeSetTypes.Num(),
					SetAttributes->Inputs.Num()));
				for (int32 AttributeIndex = 0; AttributeIndex < SetAttributes->AttributeSetTypes.Num(); ++AttributeIndex)
				{
					const FGuid& AttributeId = SetAttributes->AttributeSetTypes[AttributeIndex];
					const FString AttributeName = FMaterialAttributeDefinitionMap::GetAttributeName(AttributeId);
					const FString InputDescription = SetAttributes->Inputs.IsValidIndex(AttributeIndex)
						? DescribeInput(SetAttributes->Inputs[AttributeIndex])
						: TEXT("<missing input>");
					AddGraphLine(FString::Printf(TEXT("%s    attr[%d] %s %s -> %s"),
						*Indent,
						AttributeIndex,
						*AttributeName,
						*AttributeId.ToString(),
						*InputDescription));
				}
			}

			if (const UMaterialExpressionBlendMaterialAttributes* BlendAttributes = Cast<UMaterialExpressionBlendMaterialAttributes>(Expression))
			{
				AddGraphLine(FString::Printf(TEXT("%s  BlendMaterialAttributes pixel=%s vertex=%s A=%s B=%s Alpha=%s"),
					*Indent,
					*BlendTypeToString(BlendAttributes->PixelAttributeBlendType),
					*BlendTypeToString(BlendAttributes->VertexAttributeBlendType),
					*DescribeInput(BlendAttributes->A),
					*DescribeInput(BlendAttributes->B),
					*DescribeInput(BlendAttributes->Alpha)));
			}

			if (const UMaterialExpressionRuntimeVirtualTextureOutput* RVTOutput = Cast<UMaterialExpressionRuntimeVirtualTextureOutput>(Expression))
			{
				AddGraphLine(FString::Printf(TEXT("%s  RuntimeVirtualTextureOutput BaseColor=%s"),
					*Indent,
					*DescribeInput(RVTOutput->BaseColor)));
				AddGraphLine(FString::Printf(TEXT("%s  RuntimeVirtualTextureOutput Specular=%s"),
					*Indent,
					*DescribeInput(RVTOutput->Specular)));
				AddGraphLine(FString::Printf(TEXT("%s  RuntimeVirtualTextureOutput Roughness=%s"),
					*Indent,
					*DescribeInput(RVTOutput->Roughness)));
				AddGraphLine(FString::Printf(TEXT("%s  RuntimeVirtualTextureOutput Normal=%s"),
					*Indent,
					*DescribeInput(RVTOutput->Normal)));
				AddGraphLine(FString::Printf(TEXT("%s  RuntimeVirtualTextureOutput WorldHeight=%s"),
					*Indent,
					*DescribeInput(RVTOutput->WorldHeight)));
				AddGraphLine(FString::Printf(TEXT("%s  RuntimeVirtualTextureOutput Opacity=%s"),
					*Indent,
					*DescribeInput(RVTOutput->Opacity)));
				AddGraphLine(FString::Printf(TEXT("%s  RuntimeVirtualTextureOutput Mask=%s"),
					*Indent,
					*DescribeInput(RVTOutput->Mask)));
				AddGraphLine(FString::Printf(TEXT("%s  RuntimeVirtualTextureOutput Displacement=%s"),
					*Indent,
					*DescribeInput(RVTOutput->Displacement)));
				AddGraphLine(FString::Printf(TEXT("%s  RuntimeVirtualTextureOutput Mask4=%s"),
					*Indent,
					*DescribeInput(RVTOutput->Mask4)));
			}

			if (const UMaterialExpressionMaterialFunctionCall* FunctionCall = Cast<UMaterialExpressionMaterialFunctionCall>(Expression))
			{
				AddGraphLine(FString::Printf(TEXT("%s  FunctionCall function=%s inputs=%d outputs=%d"),
					*Indent,
					*GetPathNameSafe(FunctionCall->MaterialFunction),
					FunctionCall->FunctionInputs.Num(),
					FunctionCall->FunctionOutputs.Num()));
				for (int32 FunctionInputIndex = 0; FunctionInputIndex < FunctionCall->FunctionInputs.Num(); ++FunctionInputIndex)
				{
					const FFunctionExpressionInput& FunctionInput = FunctionCall->FunctionInputs[FunctionInputIndex];
					AddGraphLine(FString::Printf(TEXT("%s    fnInput[%d] id=%s source=%s"),
						*Indent,
						FunctionInputIndex,
						*FunctionInput.ExpressionInputId.ToString(),
						*DescribeInput(FunctionInput.Input)));
				}
				for (int32 FunctionOutputIndex = 0; FunctionOutputIndex < FunctionCall->FunctionOutputs.Num(); ++FunctionOutputIndex)
				{
					const FFunctionExpressionOutput& FunctionOutput = FunctionCall->FunctionOutputs[FunctionOutputIndex];
					AddGraphLine(FString::Printf(TEXT("%s    fnOutput[%d] id=%s name=%s"),
						*Indent,
						FunctionOutputIndex,
						*FunctionOutput.ExpressionOutputId.ToString(),
						*FunctionOutput.Output.OutputName.ToString()));
				}

				if (UMaterialFunction* MaterialFunction = Cast<UMaterialFunction>(FunctionCall->MaterialFunction))
				{
					if (!VisitedFunctionObjects.Contains(MaterialFunction))
					{
						VisitedFunctionObjects.Add(MaterialFunction);
						DumpExpressions(GetPathNameSafe(MaterialFunction), MaterialFunction->GetExpressions(), Depth + 1);
					}
				}
			}
		}
	};

	AddGraphLine(TEXT("Titan grass native material graph dump for MaterialAttributes hiding diagnosis"));
	AddGraphLine(FString::Printf(TEXT("source=%s useMaterialAttributes=%d blend=%d twoSided=%d pdo=%d mask=%d wpo=%d"),
		*GetPathNameSafe(SourceParent),
		SourceParent->bUseMaterialAttributes ? 1 : 0,
		static_cast<int32>(SourceParent->BlendMode),
		SourceParent->TwoSided ? 1 : 0,
		SourceParent->HasPixelDepthOffsetConnected() ? 1 : 0,
		SourceParent->IsPropertyConnected(MP_OpacityMask) ? 1 : 0,
		SourceParent->HasVertexPositionOffsetConnected() ? 1 : 0));
	if (const UMaterialEditorOnlyData* EditorOnlyData = SourceParent->GetEditorOnlyData())
	{
		AddGraphLine(FString::Printf(TEXT("root MaterialAttributes -> %s"), *DescribeInput(EditorOnlyData->MaterialAttributes)));
	}
	DumpExpressions(GetPathNameSafe(SourceParent), SourceParent->GetExpressions(), 0);
	const FString GraphDumpPath = FPaths::Combine(FPaths::ProjectSavedDir(), TEXT("titan_grass_material_attributes_graph_v15.txt"));
	FFileHelper::SaveStringArrayToFile(GraphDumpLines, *GraphDumpPath);
	UE_LOG(LogTemp, Display, TEXT("FFTitanGrassNoPDOCommandlet: graphDump=%s lines=%d"), *GraphDumpPath, GraphDumpLines.Num());

	if (UMaterial* LandscapeMain = LoadObject<UMaterial>(nullptr, TEXT("/Game/Landscape/Materials/M_LandscapeMain.M_LandscapeMain")))
	{
		GraphDumpLines.Reset();
		VisitedFunctionObjects.Reset();
		bool bHasRVTOutput = false;
		for (const TObjectPtr<UMaterialExpression>& Expression : LandscapeMain->GetExpressions())
		{
			if (Cast<UMaterialExpressionRuntimeVirtualTextureOutput>(Expression.Get()))
			{
				bHasRVTOutput = true;
				break;
			}
		}
		AddGraphLine(TEXT("Titan landscape material graph dump for RVT writer diagnosis"));
		AddGraphLine(FString::Printf(TEXT("source=%s useMaterialAttributes=%d hasRVTOutput=%d blend=%d"),
			*GetPathNameSafe(LandscapeMain),
			LandscapeMain->bUseMaterialAttributes ? 1 : 0,
			bHasRVTOutput ? 1 : 0,
			static_cast<int32>(LandscapeMain->BlendMode)));
		DumpExpressions(GetPathNameSafe(LandscapeMain), LandscapeMain->GetExpressions(), 0);
		const FString LandscapeDumpPath = FPaths::Combine(FPaths::ProjectSavedDir(), TEXT("titan_landscape_rvt_writer_graph_v17.txt"));
		FFileHelper::SaveStringArrayToFile(GraphDumpLines, *LandscapeDumpPath);
		UE_LOG(LogTemp, Display, TEXT("FFTitanGrassNoPDOCommandlet: landscapeRvtDump=%s lines=%d"), *LandscapeDumpPath, GraphDumpLines.Num());
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("FFTitanGrassNoPDOCommandlet: failed to load M_LandscapeMain for RVT writer dump"));
	}

	struct FGrassMaterialVariant
	{
		const TCHAR* ParentAssetName;
		const TCHAR* ParentObjectPath;
		const TCHAR* InstanceAssetName;
		const TCHAR* InstanceObjectPath;
		bool bDisconnectPixelDepthOffset;
		bool bDisconnectWorldPositionOffset;
		bool bDisconnectOpacityMask;
		bool bDisconnectRootSetWorldPositionOffset;
		bool bDisconnectRootSetOpacity;
		int32 RootSetWorldPositionBranch;
	};

	const FGrassMaterialVariant Variants[] = {
		{ TEXT("M_TitanGrassBlade_NoPDO"), TEXT("/Game/FantasyFrontier/Test/Materials/M_TitanGrassBlade_NoPDO.M_TitanGrassBlade_NoPDO"), TEXT("MI_TitanGrassBlade_NoPDO"), TEXT("/Game/FantasyFrontier/Test/Materials/MI_TitanGrassBlade_NoPDO.MI_TitanGrassBlade_NoPDO"), true, false, false, false, false, 0 },
		{ TEXT("M_TitanGrassBlade_NoWPO"), TEXT("/Game/FantasyFrontier/Test/Materials/M_TitanGrassBlade_NoWPO.M_TitanGrassBlade_NoWPO"), TEXT("MI_TitanGrassBlade_NoWPO"), TEXT("/Game/FantasyFrontier/Test/Materials/MI_TitanGrassBlade_NoWPO.MI_TitanGrassBlade_NoWPO"), false, true, false, false, false, 0 },
		{ TEXT("M_TitanGrassBlade_NoMask"), TEXT("/Game/FantasyFrontier/Test/Materials/M_TitanGrassBlade_NoMask.M_TitanGrassBlade_NoMask"), TEXT("MI_TitanGrassBlade_NoMask"), TEXT("/Game/FantasyFrontier/Test/Materials/MI_TitanGrassBlade_NoMask.MI_TitanGrassBlade_NoMask"), false, false, true, false, false, 0 },
		{ TEXT("M_TitanGrassBlade_NoWPO_NoMask"), TEXT("/Game/FantasyFrontier/Test/Materials/M_TitanGrassBlade_NoWPO_NoMask.M_TitanGrassBlade_NoWPO_NoMask"), TEXT("MI_TitanGrassBlade_NoWPO_NoMask"), TEXT("/Game/FantasyFrontier/Test/Materials/MI_TitanGrassBlade_NoWPO_NoMask.MI_TitanGrassBlade_NoWPO_NoMask"), false, true, true, false, false, 0 },
		{ TEXT("M_TitanGrassBlade_RootSetNoWPO"), TEXT("/Game/FantasyFrontier/Test/Materials/M_TitanGrassBlade_RootSetNoWPO.M_TitanGrassBlade_RootSetNoWPO"), TEXT("MI_TitanGrassBlade_RootSetNoWPO"), TEXT("/Game/FantasyFrontier/Test/Materials/MI_TitanGrassBlade_RootSetNoWPO.MI_TitanGrassBlade_RootSetNoWPO"), false, false, false, true, false, 0 },
		{ TEXT("M_TitanGrassBlade_RootSetNoOpacity"), TEXT("/Game/FantasyFrontier/Test/Materials/M_TitanGrassBlade_RootSetNoOpacity.M_TitanGrassBlade_RootSetNoOpacity"), TEXT("MI_TitanGrassBlade_RootSetNoOpacity"), TEXT("/Game/FantasyFrontier/Test/Materials/MI_TitanGrassBlade_RootSetNoOpacity.MI_TitanGrassBlade_RootSetNoOpacity"), false, false, false, false, true, 0 },
		{ TEXT("M_TitanGrassBlade_RootSetNoWPO_NoOpacity"), TEXT("/Game/FantasyFrontier/Test/Materials/M_TitanGrassBlade_RootSetNoWPO_NoOpacity.M_TitanGrassBlade_RootSetNoWPO_NoOpacity"), TEXT("MI_TitanGrassBlade_RootSetNoWPO_NoOpacity"), TEXT("/Game/FantasyFrontier/Test/Materials/MI_TitanGrassBlade_RootSetNoWPO_NoOpacity.MI_TitanGrassBlade_RootSetNoWPO_NoOpacity"), false, false, false, true, true, 0 },
		{ TEXT("M_TitanGrassBlade_RootSetWPO_InteractionWind"), TEXT("/Game/FantasyFrontier/Test/Materials/M_TitanGrassBlade_RootSetWPO_InteractionWind.M_TitanGrassBlade_RootSetWPO_InteractionWind"), TEXT("MI_TitanGrassBlade_RootSetWPO_InteractionWind"), TEXT("/Game/FantasyFrontier/Test/Materials/MI_TitanGrassBlade_RootSetWPO_InteractionWind.MI_TitanGrassBlade_RootSetWPO_InteractionWind"), false, false, false, false, false, 1 },
		{ TEXT("M_TitanGrassBlade_RootSetWPO_OffsetOnly"), TEXT("/Game/FantasyFrontier/Test/Materials/M_TitanGrassBlade_RootSetWPO_OffsetOnly.M_TitanGrassBlade_RootSetWPO_OffsetOnly"), TEXT("MI_TitanGrassBlade_RootSetWPO_OffsetOnly"), TEXT("/Game/FantasyFrontier/Test/Materials/MI_TitanGrassBlade_RootSetWPO_OffsetOnly.MI_TitanGrassBlade_RootSetWPO_OffsetOnly"), false, false, false, false, false, 2 },
		{ TEXT("M_TitanGrassBlade_RootSetWPO_FoliageInteractionOnly"), TEXT("/Game/FantasyFrontier/Test/Materials/M_TitanGrassBlade_RootSetWPO_FoliageInteractionOnly.M_TitanGrassBlade_RootSetWPO_FoliageInteractionOnly"), TEXT("MI_TitanGrassBlade_RootSetWPO_FoliageInteractionOnly"), TEXT("/Game/FantasyFrontier/Test/Materials/MI_TitanGrassBlade_RootSetWPO_FoliageInteractionOnly.MI_TitanGrassBlade_RootSetWPO_FoliageInteractionOnly"), false, false, false, false, false, 3 },
		{ TEXT("M_TitanGrassBlade_RootSetWPO_WindOnly"), TEXT("/Game/FantasyFrontier/Test/Materials/M_TitanGrassBlade_RootSetWPO_WindOnly.M_TitanGrassBlade_RootSetWPO_WindOnly"), TEXT("MI_TitanGrassBlade_RootSetWPO_WindOnly"), TEXT("/Game/FantasyFrontier/Test/Materials/MI_TitanGrassBlade_RootSetWPO_WindOnly.MI_TitanGrassBlade_RootSetWPO_WindOnly"), false, false, false, false, false, 4 },
		{ TEXT("M_TitanGrassBlade_RootSetWPO_RVTOffsetScale10"), TEXT("/Game/FantasyFrontier/Test/Materials/M_TitanGrassBlade_RootSetWPO_RVTOffsetScale10.M_TitanGrassBlade_RootSetWPO_RVTOffsetScale10"), TEXT("MI_TitanGrassBlade_RootSetWPO_RVTOffsetScale10"), TEXT("/Game/FantasyFrontier/Test/Materials/MI_TitanGrassBlade_RootSetWPO_RVTOffsetScale10.MI_TitanGrassBlade_RootSetWPO_RVTOffsetScale10"), false, false, false, false, false, 5 },
		{ TEXT("M_TitanGrassBlade_RootSetWPO_RVTOffsetScale01"), TEXT("/Game/FantasyFrontier/Test/Materials/M_TitanGrassBlade_RootSetWPO_RVTOffsetScale01.M_TitanGrassBlade_RootSetWPO_RVTOffsetScale01"), TEXT("MI_TitanGrassBlade_RootSetWPO_RVTOffsetScale01"), TEXT("/Game/FantasyFrontier/Test/Materials/MI_TitanGrassBlade_RootSetWPO_RVTOffsetScale01.MI_TitanGrassBlade_RootSetWPO_RVTOffsetScale01"), false, false, false, false, false, 6 },
		{ TEXT("M_TitanGrassBlade_RootSetWPO_RVTOffsetClamp005"), TEXT("/Game/FantasyFrontier/Test/Materials/M_TitanGrassBlade_RootSetWPO_RVTOffsetClamp005.M_TitanGrassBlade_RootSetWPO_RVTOffsetClamp005"), TEXT("MI_TitanGrassBlade_RootSetWPO_RVTOffsetClamp005"), TEXT("/Game/FantasyFrontier/Test/Materials/MI_TitanGrassBlade_RootSetWPO_RVTOffsetClamp005.MI_TitanGrassBlade_RootSetWPO_RVTOffsetClamp005"), false, false, false, false, false, 7 },
		{ TEXT("M_TitanGrassBlade_RootSetWPO_RVTOffsetClamp010"), TEXT("/Game/FantasyFrontier/Test/Materials/M_TitanGrassBlade_RootSetWPO_RVTOffsetClamp010.M_TitanGrassBlade_RootSetWPO_RVTOffsetClamp010"), TEXT("MI_TitanGrassBlade_RootSetWPO_RVTOffsetClamp010"), TEXT("/Game/FantasyFrontier/Test/Materials/MI_TitanGrassBlade_RootSetWPO_RVTOffsetClamp010.MI_TitanGrassBlade_RootSetWPO_RVTOffsetClamp010"), false, false, false, false, false, 8 },
	};

	auto ClearInput = [](auto& Input)
	{
		Input.Expression = nullptr;
		Input.OutputIndex = 0;
		Input.InputName = NAME_None;
		Input.Mask = 0;
		Input.MaskR = 0;
		Input.MaskG = 0;
		Input.MaskB = 0;
		Input.MaskA = 0;
	};

	auto ClearRootSetMaterialAttributeInput = [&](UMaterial* Material, const TCHAR* TargetInputName) -> int32
	{
		if (!Material)
		{
			return 0;
		}

		UMaterialEditorOnlyData* EditorOnlyData = Material->GetEditorOnlyData();
		if (!EditorOnlyData || !EditorOnlyData->MaterialAttributes.Expression)
		{
			return 0;
		}

		FExpressionInput RootInput = EditorOnlyData->MaterialAttributes.GetTracedInput();
		UMaterialExpressionSetMaterialAttributes* RootSetAttributes = Cast<UMaterialExpressionSetMaterialAttributes>(RootInput.Expression);
		if (!RootSetAttributes)
		{
			return 0;
		}

		int32 ClearedCount = 0;
		TArrayView<FExpressionInput*> Inputs = RootSetAttributes->GetInputsView();
		for (int32 InputIndex = 0; InputIndex < Inputs.Num(); ++InputIndex)
		{
			FExpressionInput* Input = Inputs[InputIndex];
			if (!Input)
			{
				continue;
			}

			const FString InputName = RootSetAttributes->GetInputName(InputIndex).ToString();
			if (InputName.Equals(TargetInputName, ESearchCase::IgnoreCase))
			{
				const bool bWasConnected = Input->Expression != nullptr;
				ClearInput(*Input);
				++ClearedCount;
				UE_LOG(LogTemp, Display, TEXT("FFTitanGrassNoPDOCommandlet: rootSetClear material=%s input=%s wasConnected=%d"),
					*GetPathNameSafe(Material),
					*InputName,
					bWasConnected ? 1 : 0);
			}
		}

		return ClearedCount;
	};

	auto ReplaceRootSetWorldPositionOffsetWithBranch = [&](UMaterial* Material, const int32 BranchMode) -> int32
	{
		if (!Material || BranchMode <= 0)
		{
			return 0;
		}

		UMaterialEditorOnlyData* EditorOnlyData = Material->GetEditorOnlyData();
		if (!EditorOnlyData || !EditorOnlyData->MaterialAttributes.Expression)
		{
			return 0;
		}

		FExpressionInput RootInput = EditorOnlyData->MaterialAttributes.GetTracedInput();
		UMaterialExpressionSetMaterialAttributes* RootSetAttributes = Cast<UMaterialExpressionSetMaterialAttributes>(RootInput.Expression);
		if (!RootSetAttributes)
		{
			return 0;
		}

		TArrayView<FExpressionInput*> Inputs = RootSetAttributes->GetInputsView();
		for (int32 InputIndex = 0; InputIndex < Inputs.Num(); ++InputIndex)
		{
			FExpressionInput* Input = Inputs[InputIndex];
			if (!Input)
			{
				continue;
			}

			const FString InputName = RootSetAttributes->GetInputName(InputIndex).ToString();
			if (!InputName.Equals(TEXT("World Position Offset"), ESearchCase::IgnoreCase))
			{
				continue;
			}

			UMaterialExpressionAdd* RootAdd = Cast<UMaterialExpressionAdd>(Input->Expression);
			if (!RootAdd)
			{
				UE_LOG(LogTemp, Warning, TEXT("FFTitanGrassNoPDOCommandlet: rootSetWPOBranch material=%s mode=%d missingRootAdd=%s"),
					*GetPathNameSafe(Material),
					BranchMode,
					*GetPathNameSafe(Input->Expression));
				return 0;
			}

			if (BranchMode >= 5)
			{
				UMaterialExpressionMultiply* OffsetMultiply = Cast<UMaterialExpressionMultiply>(RootAdd->B.Expression);
				UMaterialExpressionConstant3Vector* OffsetVector = OffsetMultiply ? Cast<UMaterialExpressionConstant3Vector>(OffsetMultiply->B.Expression) : nullptr;
				if (!OffsetMultiply || !OffsetVector || !OffsetMultiply->A.Expression)
				{
					UE_LOG(LogTemp, Warning, TEXT("FFTitanGrassNoPDOCommandlet: rootSetWPOBranch material=%s mode=%d missingOffsetBranch multiply=%s vector=%s sample=%s"),
						*GetPathNameSafe(Material),
						BranchMode,
						*GetPathNameSafe(OffsetMultiply),
						*GetPathNameSafe(OffsetVector),
						OffsetMultiply ? *GetPathNameSafe(OffsetMultiply->A.Expression) : TEXT("<none>"));
					return 0;
				}

				float ZScale = -400.0f;
				const TCHAR* ReplacementName = TEXT("Native RVT offset unchanged");
				if (BranchMode == 5)
				{
					ZScale = -40.0f;
					ReplacementName = TEXT("Native RVT offset scaled to 10 percent");
				}
				else if (BranchMode == 6)
				{
					ZScale = -4.0f;
					ReplacementName = TEXT("Native RVT offset scaled to 1 percent");
				}
				else if (BranchMode == 7 || BranchMode == 8)
				{
					const float ClampMax = BranchMode == 7 ? 0.05f : 0.10f;
					UMaterialExpressionClamp* Clamp = Cast<UMaterialExpressionClamp>(
						UMaterialEditingLibrary::CreateMaterialExpression(Material, UMaterialExpressionClamp::StaticClass(), OffsetMultiply->MaterialExpressionEditorX - 220, OffsetMultiply->MaterialExpressionEditorY - 120));
					if (!Clamp)
					{
						UE_LOG(LogTemp, Warning, TEXT("FFTitanGrassNoPDOCommandlet: rootSetWPOBranch material=%s mode=%d failedClampAllocation"),
							*GetPathNameSafe(Material),
							BranchMode);
						return 0;
					}

					Clamp->Input = OffsetMultiply->A;
					Clamp->ClampMode = CMODE_Clamp;
					Clamp->MinDefault = 0.0f;
					Clamp->MaxDefault = ClampMax;
					OffsetMultiply->A.Expression = Clamp;
					OffsetMultiply->A.OutputIndex = 0;
					OffsetMultiply->A.InputName = NAME_None;
					OffsetMultiply->A.Mask = 0;
					OffsetMultiply->A.MaskR = 0;
					OffsetMultiply->A.MaskG = 0;
					OffsetMultiply->A.MaskB = 0;
					OffsetMultiply->A.MaskA = 0;
					ReplacementName = BranchMode == 7
						? TEXT("Native RVT offset sample clamped to 0..0.05")
						: TEXT("Native RVT offset sample clamped to 0..0.10");
				}
				else
				{
					return 0;
				}

				OffsetVector->Constant = FLinearColor(0.0f, 0.0f, ZScale, 0.0f);
				UE_LOG(LogTemp, Display, TEXT("FFTitanGrassNoPDOCommandlet: rootSetWPOBranch material=%s mode=%d replacement=%s sample=%s zScale=%.3f"),
					*GetPathNameSafe(Material),
					BranchMode,
					ReplacementName,
					*GetPathNameSafe(OffsetMultiply->A.Expression),
					ZScale);
				return 1;
			}

			FExpressionInput Replacement;
			const TCHAR* ReplacementName = TEXT("Unknown");
			if (BranchMode == 1)
			{
				Replacement = RootAdd->A;
				ReplacementName = TEXT("RootAdd.A Interaction+Wind");
			}
			else if (BranchMode == 2)
			{
				Replacement = RootAdd->B;
				ReplacementName = TEXT("RootAdd.B OffsetOnly");
			}
			else
			{
				FExpressionInput InteractionWindInput = RootAdd->A;
				UMaterialExpressionAdd* InteractionWindAdd = Cast<UMaterialExpressionAdd>(InteractionWindInput.Expression);
				if (!InteractionWindAdd)
				{
					UE_LOG(LogTemp, Warning, TEXT("FFTitanGrassNoPDOCommandlet: rootSetWPOBranch material=%s mode=%d missingInteractionWindAdd=%s"),
						*GetPathNameSafe(Material),
						BranchMode,
						*GetPathNameSafe(InteractionWindInput.Expression));
					return 0;
				}

				if (BranchMode == 3)
				{
					Replacement = InteractionWindAdd->A;
					ReplacementName = TEXT("InteractionWindAdd.A FoliageInteraction");
				}
				else if (BranchMode == 4)
				{
					Replacement = InteractionWindAdd->B;
					ReplacementName = TEXT("InteractionWindAdd.B WindMovement");
				}
				else
				{
					return 0;
				}
			}

			if (!Replacement.Expression)
			{
				UE_LOG(LogTemp, Warning, TEXT("FFTitanGrassNoPDOCommandlet: rootSetWPOBranch material=%s mode=%d replacement=%s empty"),
					*GetPathNameSafe(Material),
					BranchMode,
					ReplacementName);
				return 0;
			}

			*Input = Replacement;
			UE_LOG(LogTemp, Display, TEXT("FFTitanGrassNoPDOCommandlet: rootSetWPOBranch material=%s mode=%d replacement=%s expression=%s"),
				*GetPathNameSafe(Material),
				BranchMode,
				ReplacementName,
				*GetPathNameSafe(Replacement.Expression));
			return 1;
		}

		return 0;
	};

	for (const FGrassMaterialVariant& Variant : Variants)
	{
		UMaterial* VariantParent = LoadObject<UMaterial>(nullptr, Variant.ParentObjectPath);
		if (!VariantParent)
		{
			UObject* Duplicated = AssetToolsModule.Get().DuplicateAsset(Variant.ParentAssetName, OutputFolderPath, SourceParent);
			VariantParent = Cast<UMaterial>(Duplicated);
			UE_LOG(LogTemp, Display, TEXT("FFTitanGrassNoPDOCommandlet: duplicated parent=%s"), *GetPathNameSafe(VariantParent));
		}

		if (!VariantParent)
		{
			UE_LOG(LogTemp, Error, TEXT("FFTitanGrassNoPDOCommandlet: failed to create/load %s"), Variant.ParentObjectPath);
			return 1;
		}

		VariantParent->Modify();
		VariantParent->PreEditChange(nullptr);

		UMaterialEditorOnlyData* EditorOnlyData = VariantParent->GetEditorOnlyData();
		const bool bHadPixelDepthOffset = EditorOnlyData && EditorOnlyData->PixelDepthOffset.Expression != nullptr;
		const bool bHadWorldPositionOffset = EditorOnlyData && EditorOnlyData->WorldPositionOffset.Expression != nullptr;
		const bool bHadOpacityMask = EditorOnlyData && EditorOnlyData->OpacityMask.Expression != nullptr;
		if (EditorOnlyData)
		{
			if (Variant.bDisconnectPixelDepthOffset)
			{
				ClearInput(EditorOnlyData->PixelDepthOffset);
			}
			if (Variant.bDisconnectWorldPositionOffset)
			{
				ClearInput(EditorOnlyData->WorldPositionOffset);
			}
			if (Variant.bDisconnectOpacityMask)
			{
				ClearInput(EditorOnlyData->OpacityMask);
			}
		}
		const int32 RootSetWPOCleared = Variant.bDisconnectRootSetWorldPositionOffset
			? ClearRootSetMaterialAttributeInput(VariantParent, TEXT("World Position Offset"))
			: 0;
		const int32 RootSetOpacityCleared = Variant.bDisconnectRootSetOpacity
			? ClearRootSetMaterialAttributeInput(VariantParent, TEXT("Opacity"))
			: 0;
		const int32 RootSetWPOBranched = Variant.RootSetWorldPositionBranch > 0
			? ReplaceRootSetWorldPositionOffsetWithBranch(VariantParent, Variant.RootSetWorldPositionBranch)
			: 0;

		UMaterialEditingLibrary::RecompileMaterial(VariantParent);
		VariantParent->PostEditChange();
		VariantParent->MarkPackageDirty();
		UE_LOG(LogTemp, Display,
			TEXT("FFTitanGrassNoPDOCommandlet: parent=%s hadPDO=%d nowPDO=%d hadWPO=%d nowWPO=%d hadMask=%d nowMask=%d rootSetWPOCleared=%d rootSetOpacityCleared=%d rootSetWPOBranch=%d"),
			*GetPathNameSafe(VariantParent),
			bHadPixelDepthOffset ? 1 : 0,
			VariantParent->HasPixelDepthOffsetConnected() ? 1 : 0,
			bHadWorldPositionOffset ? 1 : 0,
			VariantParent->HasVertexPositionOffsetConnected() ? 1 : 0,
			bHadOpacityMask ? 1 : 0,
			VariantParent->IsPropertyConnected(MP_OpacityMask) ? 1 : 0,
			RootSetWPOCleared,
			RootSetOpacityCleared,
			RootSetWPOBranched);

		UMaterialInstanceConstant* VariantInstance = LoadObject<UMaterialInstanceConstant>(nullptr, Variant.InstanceObjectPath);
		if (!VariantInstance)
		{
			UMaterialInstanceConstantFactoryNew* Factory = NewObject<UMaterialInstanceConstantFactoryNew>();
			UObject* Created = AssetToolsModule.Get().CreateAsset(Variant.InstanceAssetName, OutputFolderPath, UMaterialInstanceConstant::StaticClass(), Factory);
			VariantInstance = Cast<UMaterialInstanceConstant>(Created);
			UE_LOG(LogTemp, Display, TEXT("FFTitanGrassNoPDOCommandlet: created instance=%s"), *GetPathNameSafe(VariantInstance));
		}

		if (!VariantInstance)
		{
			UE_LOG(LogTemp, Error, TEXT("FFTitanGrassNoPDOCommandlet: failed to create/load %s"), Variant.InstanceObjectPath);
			return 1;
		}

		VariantInstance->Modify();
		VariantInstance->SetParentEditorOnly(VariantParent);
		UMaterialEditingLibrary::SetMaterialInstanceScalarParameterValue(VariantInstance, TEXT("UV Pivot Area"), 50.0f);
		UMaterialEditingLibrary::SetMaterialInstanceScalarParameterValue(VariantInstance, TEXT("FI  Max Rotation"), 0.2440000027f);
		UMaterialEditingLibrary::SetMaterialInstanceScalarParameterValue(VariantInstance, TEXT("FI Interaction Size"), 158.8193664551f);
		UMaterialEditingLibrary::SetMaterialInstanceScalarParameterValue(VariantInstance, TEXT("Per Blade Wind Offset"), 0.9891890287f);
		UMaterialEditingLibrary::SetMaterialInstanceScalarParameterValue(VariantInstance, TEXT("Max Rotation Angle"), 0.2000000030f);
		UMaterialEditingLibrary::SetMaterialInstanceScalarParameterValue(VariantInstance, TEXT("Vertex Normal Overide"), 0.0f);
		UMaterialEditingLibrary::SetMaterialInstanceVectorParameterValue(VariantInstance, TEXT("Ground Col"), FLinearColor(0.0297f, 0.0625f, 0.0059f, 0.05f));
		UMaterialEditingLibrary::SetMaterialInstanceVectorParameterValue(VariantInstance, TEXT("Subsurface Col"), FLinearColor(0.0088f, 0.0104f, 0.0016f, 1.0f));
		UMaterialEditingLibrary::SetMaterialInstanceVectorParameterValue(VariantInstance, TEXT("Color Variation"), FLinearColor(0.02f, 0.25f, 0.04f, 0.0f));
		UMaterialEditingLibrary::SetMaterialInstanceVectorParameterValue(VariantInstance, TEXT("RVT Blend"), FLinearColor(2.0f, 2.0f, 0.0f, 0.0f));
		VariantInstance->PostEditChange();
		VariantInstance->MarkPackageDirty();
	}

	const bool bSavedAssets = UEditorLoadingAndSavingUtils::SaveDirtyPackages(true, true);
	UE_LOG(LogTemp, Display, TEXT("FFTitanGrassNoPDOCommandlet: saved=%d"), bSavedAssets ? 1 : 0);
	return bSavedAssets ? 0 : 1;
#else
	UE_LOG(LogTemp, Error, TEXT("FFTitanGrassNoPDOCommandlet can only run in editor builds."));
	return 1;
#endif
}
