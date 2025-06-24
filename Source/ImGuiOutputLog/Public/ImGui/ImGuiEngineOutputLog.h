// Author: Antonio Sidenko (Tonetfal), June 2025

#pragma once

#include "ImGui/ImGuiLogVerbosity.h"
#include "Subsystems/GameInstanceSubsystem.h"

#include "ImGuiEngineOutputLog.generated.h"

UENUM(BlueprintType, meta=(BitFlags, UseEnumValuesAsMaskValuesInEditor="True"))
enum EImGuiOutputLogMessageElement : uint8
{
	None		= 0 UMETA(Hidden),
	Category	= 1 << 0,
	Timestamp	= 1 << 1,
	Verbosity	= 1 << 2,
};

UCLASS(DisplayName="ImGui Engine Output Log")
class IMGUIOUTPUTLOG_API UImGuiEngineOutputLog
	: public UGameInstanceSubsystem
	, public FTickableGameObject
{
	GENERATED_BODY()

public:

public:
	virtual ~UImGuiEngineOutputLog() override;
	
	static bool HasInstance(const UObject* ContextObject);
	static ThisClass& Get(const UObject* ContextObject);

	//~UGameInstanceSubsystem Interface
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	//~End of UGameInstanceSubsystem Interface

	//~FTickableGameObject Interface
	virtual void Tick(float DeltaTime) override;
	virtual ETickableTickType GetTickableTickType() const override;
	virtual TStatId GetStatId() const override;
	virtual bool IsTickable() const override;
	virtual bool IsTickableWhenPaused() const override;
	virtual bool IsTickableInEditor() const override;
	//~End of FTickableGameObject Interface

	UFUNCTION(BlueprintCallable, Category="ImGui|Engine Log")
	void SetActiveState(UPARAM(DisplayName="Is Active") bool bInIsActive);
	
	UFUNCTION(BlueprintPure, Category="ImGui|Engine Log")
	bool IsActive() const;

	UFUNCTION(BlueprintCallable, Category="ImGui|Engine Log",
		meta=(Bitmask, BitmaskEnum="EImGuiOutputLogMessageElement"))
	void SetDisplayedElements(TEnumAsByte<EImGuiOutputLogMessageElement> Elements);
	
	UFUNCTION(BlueprintCallable, Category="ImGui|Engine Log",
		meta=(Bitmask, BitmaskEnum="EImGuiOutputLogMessageElement"))
	void AddDisplayedElements(TEnumAsByte<EImGuiOutputLogMessageElement> Elements);
	
	UFUNCTION(BlueprintCallable, Category="ImGui|Engine Log",
		meta=(Bitmask, BitmaskEnum="EImGuiOutputLogMessageElement"))
	void RemovedDisplayedElements(TEnumAsByte<EImGuiOutputLogMessageElement> Elements);
	
	UFUNCTION(BlueprintPure, Category="ImGui|Engine Log")
	TEnumAsByte<EImGuiOutputLogMessageElement> GetDisplayedElements() const;

public:
	FSimpleMulticastDelegate OnClosedDelegate;

private:
	class FImGuiEngineLogImpl* Impl = nullptr;
	uint32 LastFrameNumberWeTicked = INDEX_NONE;
};

UCLASS(Config="ImGui", DefaultConfig, DisplayName="ImGui: Output Log")
class IMGUIOUTPUTLOG_API UImGuiEngineOutputLogSettings
	: public UDeveloperSettings
{
	GENERATED_BODY()

public:
	UImGuiEngineOutputLogSettings();

	//~UDeveloperSettings Interface
	virtual FName GetCategoryName() const override;
	//~End of UDeveloperSettings Interface

public:
	UPROPERTY(Config, EditAnywhere)
	TMap<EImGuiLogVerbosity, FLinearColor> Colors;
	
	UPROPERTY(Config, EditAnywhere)
	bool bEnabledInputOnActive = true;
};
