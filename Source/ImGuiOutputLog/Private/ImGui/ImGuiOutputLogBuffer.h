// Author: Antonio Sidenko (Tonetfal), June 2025

#pragma once

#include "ImGui/ImGuiOutputLogMessage.h"
#include "Subsystems/GameInstanceSubsystem.h"

#include "ImGuiOutputLogBuffer.generated.h"

/**
 *
 */
UCLASS(NotBlueprintType)
class UImGuiOutputLogBuffer
	: public UGameInstanceSubsystem
	, public FOutputDevice
{
	GENERATED_BODY()

public:
	//~UGameInstanceSubsystem Interface
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	//~End of UGameInstanceSubsystem Interface

	//~FOutputDevice Interface
	virtual void Serialize(const TCHAR* Message, ELogVerbosity::Type Verbosity, const FName& Category) override;
	//~End of FOutputDevice Interface

public:
	TArray<ImGui::Private::TMessageRef> Messages;
};
