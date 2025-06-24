// Author: Antonio Sidenko (Tonetfal), June 2025

#include "ImGui/ImGuiEngineLog.h"

#include "imgui_internal.h"
#include "ImGui/ImGuiOutputLogFilter.h"
#include "ImGui/ImGuiOutputLogMessage.h"
#include "ImGui/ImGuiOutputLogBuffer.h"

class FImGuiEngineLogImpl
{
public:
	FImGuiEngineLogImpl();

	bool Tick();
	
	void AddNewMessages();
	void AddMessage(const ImGui::Private::TMessageRef Message);

	void DrawMessage(const ImGui::Private::TMessageRef Message) const;
	void FormatMessage(ImGui::Private::TMessageRef Message) const;

	void Clear();

	static ImU32 VerbosityToColor(EImGuiLogVerbosity Verbosity);

public:	
	void SetActiveState(bool bInIsActive);
	void SetDisplayedElements(UImGuiEngineLog::EMessageElement Elements);
	void AddDisplayedElements(UImGuiEngineLog::EMessageElement Elements);
	void RemovedDisplayedElements(UImGuiEngineLog::EMessageElement Elements);

public:
	UImGuiOutputLogBuffer* LogBuffer = nullptr;
		
	bool bIsActive = false;
	uint8 ActiveElements = UImGuiEngineLog::EMessageElement::Category;
	bool bElementsDirty = true;

	TArray<ImGui::Private::TMessageRef> Messages;
	FImGuiOutputLogFilter Filter;
	int32 LastOutputLogIndex = INDEX_NONE;
};

FImGuiEngineLogImpl::FImGuiEngineLogImpl()
{
	Filter.Context.Hint = "Search Log";
	Filter.Context.Width = 200.f;
}

bool FImGuiEngineLogImpl::Tick()
{
	bool bIsOpen = true;
	ImGui::Begin("Engine Log", &bIsOpen);

	if (!bIsOpen)
	{
		ImGui::End();
		return false;
	}

	AddNewMessages();

	if (bElementsDirty)
	{
		for (ImGui::Private::TMessageRef Message : Messages)
		{
			FormatMessage(Message);
		}
	}

	ImGui::SameLine();
	const bool bHaveFiltersChanged = Filter.Draw();

	ImGui::SameLine();
	const bool bClear = ImGui::Button("Clear");

	ImGui::Separator();

	if (ImGui::BeginChild("scrolling", ImVec2(0, 0.f), ImGuiChildFlags_None, ImGuiWindowFlags_HorizontalScrollbar))
	{
		if (bClear)
		{
			Clear();
		}

		if (Filter.IsActive())
		{
			// @todo clipper?
			for (ImGui::Private::TMessageRef Message : Messages)
			{
				if (bHaveFiltersChanged)
				{
					Message->bIsFilteredOut = !Filter.PassFilter(Message);
				}

				if (!Message->bIsFilteredOut)
				{
					DrawMessage(Message);
				}
			}
		}
		else
		{
			ImGuiListClipper Clipper;
			Clipper.Begin(Messages.Num());

			while (Clipper.Step())
			{
				for (int32 Idx = Clipper.DisplayStart; Idx < Clipper.DisplayEnd; Idx++)
				{
					DrawMessage(Messages[Idx]);
				}
			}

			Clipper.End();
		}

		// Keep up at the bottom of the scroll region if we were already at the bottom at the beginning of the frame.
		// Using a scrollbar or mouse-wheel will take away from the bottom edge.
		if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
		{
			ImGui::SetScrollHereY(1.0f);
		}
	}

	ImGui::EndChild();
	ImGui::End();

	return true;
}

UImGuiEngineLog::~UImGuiEngineLog()
{
	delete Impl;
}

bool UImGuiEngineLog::HasInstance(const UObject* ContextObject)
{
	check(IsValid(ContextObject));
	
	const UWorld* World = ContextObject->GetWorld();
	if (!IsValid(World))
	{
		return false;
	}
	
	const UGameInstance* GameInstance = World->GetGameInstance();
	if (!IsValid(GameInstance))
	{
		return false;
	}
	
	const auto* ThisSubsystem = GameInstance->GetSubsystem<ThisClass>();
	if (!IsValid(ThisSubsystem))
	{
		return false;
	}
	
	return true;
}

UImGuiEngineLog::ThisClass& UImGuiEngineLog::Get(const UObject* ContextObject)
{
	check(IsValid(ContextObject));
	
	const UWorld* World = ContextObject->GetWorld();
	check(IsValid(World));
	
	const UGameInstance* GameInstance = World->GetGameInstance();
	check(IsValid(GameInstance));
	
	auto* ThisSubsystem = GameInstance->GetSubsystem<ThisClass>();
	check(IsValid(ThisSubsystem));
	
	auto& ThisSubsystemRef = *ThisSubsystem;
	return ThisSubsystemRef;
}

void UImGuiEngineLog::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	Impl = new FImGuiEngineLogImpl;
	Impl->LogBuffer = Cast<UImGuiOutputLogBuffer>(Collection.InitializeDependency(UImGuiOutputLogBuffer::StaticClass()));
}

void UImGuiEngineLog::Deinitialize()
{
	Super::Deinitialize();

	Impl->SetActiveState(false);
}

void UImGuiEngineLog::Tick(float DeltaSeconds)
{
	if (LastFrameNumberWeTicked == GFrameCounter)
	{
		return;
	}

	LastFrameNumberWeTicked = GFrameCounter;

	if (!Impl->Tick())
	{
		OnClosedDelegate.Broadcast();
	}
}

ETickableTickType UImGuiEngineLog::GetTickableTickType() const
{
	return ETickableTickType::Conditional;
}

TStatId UImGuiEngineLog::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(FImGuiEngineLog, STATGROUP_Tickables);
}

bool UImGuiEngineLog::IsTickable() const
{
	return Impl && Impl->bIsActive;
}

bool UImGuiEngineLog::IsTickableWhenPaused() const
{
	return true;
}

bool UImGuiEngineLog::IsTickableInEditor() const
{
	return false;
}

void UImGuiEngineLog::SetActiveState(bool bInIsActive)
{
	Impl->SetActiveState(bInIsActive);
}

void UImGuiEngineLog::SetDisplayedElements(EMessageElement Elements)
{
	Impl->SetDisplayedElements(Elements);
}

void UImGuiEngineLog::AddDisplayedElements(EMessageElement Elements)
{
	Impl->AddDisplayedElements(Elements);
}

void UImGuiEngineLog::RemovedDisplayedElements(EMessageElement Elements)
{
	Impl->RemovedDisplayedElements(Elements);
}

void FImGuiEngineLogImpl::SetActiveState(bool bInIsActive)
{
	bIsActive = bInIsActive;
}

void FImGuiEngineLogImpl::SetDisplayedElements(UImGuiEngineLog::EMessageElement Elements)
{
	if (ActiveElements != Elements)
	{
		ActiveElements = Elements;
		bElementsDirty = true;
	}
}

void FImGuiEngineLogImpl::AddDisplayedElements(UImGuiEngineLog::EMessageElement Elements)
{
	if (ActiveElements | Elements)
	{
		ActiveElements |= Elements;
		bElementsDirty = true;
	}
}

void FImGuiEngineLogImpl::RemovedDisplayedElements(UImGuiEngineLog::EMessageElement Elements)
{
	if (ActiveElements & ~Elements)
	{
		ActiveElements &= ~Elements;
		bElementsDirty = true;
	}
}

void FImGuiEngineLogImpl::AddNewMessages()
{
	const int32 Num = LogBuffer->Messages.Num();
	for (int32 i = LastOutputLogIndex + 1; i < Num; ++i)
	{
		AddMessage(LogBuffer->Messages[i]);
	}

	LastOutputLogIndex = Num - 1;
}

void FImGuiEngineLogImpl::AddMessage(const ImGui::Private::TMessageRef Message)
{
	auto& Ref = Messages.Add_GetRef(Message);
	if (!bElementsDirty)
	{
		FormatMessage(Ref);
	}

	Ref->bIsFilteredOut = !Filter.PassFilter(Ref);

	bool bIsCategoryPresent = false;
	for (const auto& [Category, _] : Filter.Context.Categories)
	{
		if (ImStricmp(Category, Message->Category) == 0)
		{
			bIsCategoryPresent = true;
			break;
		}
	}

	if (!bIsCategoryPresent)
	{
		Filter.Context.Categories.Add({ ImStrdup(Message->Category),Filter.Context.bShowAll });
	}
}

void FImGuiEngineLogImpl::DrawMessage(const ImGui::Private::TMessageRef Message) const
{
	ImGui::PushStyleColor(ImGuiCol_Text, VerbosityToColor(Message->Verbosity));
	ImGui::TextUnformatted(Message->FormattedText, Message->FormattedText + Message->FormattedTextLen);
	ImGui::PopStyleColor();
}

// @todo allow custom formatting, e.g. show/hide timestamp, show/hide category, show/hide verbosity
void FImGuiEngineLogImpl::FormatMessage(ImGui::Private::TMessageRef Message) const
{
	Message->FormattedTextLen = Message->CategoryLen + Message->TextLen + 2;

	const size_t ToAllocate = Message->FormattedTextLen + 1;
	Message->FormattedText = static_cast<char*>(IM_ALLOC(ToAllocate));
	sprintf_s(Message->FormattedText, ToAllocate, "%s: %s", Message->Category, Message->Text);
}

void FImGuiEngineLogImpl::Clear()
{
	Messages.Empty();
}

static ImU32 ColorToU32(const FLinearColor& Color)
{
	return ImColor(Color.R, Color.G, Color.B, 1.f);
}

static FLinearColor U32ToColor(const ImU32& Color)
{
	const ImColor ImColor(Color);
	return FLinearColor(ImColor.Value.x, ImColor.Value.y, ImColor.Value.z, ImColor.Value.w);
}

static ImU32 DefaultVerbosityToColor(EImGuiLogVerbosity Verbosity)
{
	switch (Verbosity)
	{
	case EImGuiLogVerbosity::Verbose:
	case EImGuiLogVerbosity::VeryVerbose: return IM_COL32(186, 186, 186, 255);
	case EImGuiLogVerbosity::Log:
	case EImGuiLogVerbosity::Display: return IM_COL32(240, 240, 240, 255);
	case EImGuiLogVerbosity::Warning: return IM_COL32(243, 175, 0, 255);
	case EImGuiLogVerbosity::Error:
	case EImGuiLogVerbosity::Fatal: return IM_COL32(234, 11, 0, 255);
	default: return IM_COL32(255, 0, 128, 255);
	}
}

ImU32 FImGuiEngineLogImpl::VerbosityToColor(EImGuiLogVerbosity Verbosity)
{
	const auto* Settings = GetDefault<UImGuiEngineLogSettings>();
	const FLinearColor* FoundColor = Settings->Colors.Find(Verbosity);
	return FoundColor ? ColorToU32(*FoundColor) : DefaultVerbosityToColor(Verbosity);
}

UImGuiEngineLogSettings::UImGuiEngineLogSettings()
{
	for (const auto Verbosity : TEnumRange<EImGuiLogVerbosity>())
	{
		Colors.Add(Verbosity, U32ToColor(DefaultVerbosityToColor(Verbosity)));
	}
}
