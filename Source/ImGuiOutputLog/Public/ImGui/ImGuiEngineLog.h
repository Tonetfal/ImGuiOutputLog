// Author: Antonio Sidenko (Tonetfal), June 2025

#pragma once

#include "ImGui/ImGuiLogVerbosity.h"
#include "Subsystems/GameInstanceSubsystem.h"

#include "ImGuiEngineLog.generated.h"

UCLASS()
class IMGUIOUTPUTLOG_API UImGuiEngineLog
	: public UGameInstanceSubsystem
	, public FTickableGameObject
{
	GENERATED_BODY()

public:
	enum EMessageElement : uint8
	{
		Category = 1 << 0,
		Timestamp = 1 << 1,
		Verbosity = 1 << 2,
	};

public:
	virtual ~UImGuiEngineLog() override;
	
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
	void SetActiveState(bool bInIsActive);
	
	void SetDisplayedElements(EMessageElement Elements);
	void AddDisplayedElements(EMessageElement Elements);
	void RemovedDisplayedElements(EMessageElement Elements);

public:
	FSimpleMulticastDelegate OnClosedDelegate;

private:
	class FImGuiEngineLogImpl* Impl = nullptr;
	uint32 LastFrameNumberWeTicked = INDEX_NONE;
};

UCLASS(Config="Engine", DefaultConfig)
class IMGUIOUTPUTLOG_API UImGuiEngineLogSettings
	: public UDeveloperSettings
{
	GENERATED_BODY()

public:
	UImGuiEngineLogSettings();

public:
	UPROPERTY(Config, EditAnywhere)
	TMap<EImGuiLogVerbosity, FLinearColor> Colors;
};
