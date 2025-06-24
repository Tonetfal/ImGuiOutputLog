// Author: Antonio Sidenko (Tonetfal), June 2025

#include "ImGui/ImGuiEngineOutputLog.h"

#include "ImGuiModule.h"
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

	bool DrawVerbosities();
	void DrawAllMessages();
	
	void DrawMessage(int32 Index) const;
	void FormatMessage(ImGui::Private::TMessageRef Message) const;

	void ValidateMessages();
	void Clear();

	void TryFilteringMessage(int32 Index);
	void FilterAllMessages();

	int32 MessageToLine(int32 MessageIndex) const;
	static ImU32 VerbosityToColor(EImGuiLogVerbosity Verbosity);
	static const char* VerbosityToString(EImGuiLogVerbosity Verbosity);

public:
	bool IsActive() const;
	void SetActiveState(bool bInIsActive);
	void SetDisplayedElements(int32 Elements);
	void AddDisplayedElements(int32 Elements);
	void RemovedDisplayedElements(int32 Elements);
	EImGuiOutputLogMessageElement GetDisplayedElements() const;

	FImGuiModule& GetImGuiModule() const;

public:
	UImGuiOutputLogBuffer* LogBuffer = nullptr;

	bool bIsActive = false;
	uint8 ActiveElements = EImGuiOutputLogMessageElement::Category;
	bool bElementsDirty = true;
	bool bFiltersDirty = true;

	TArray<ImGui::Private::TMessageRef> Messages;
	TArray<TPair<ImGui::Private::TMessageRef, int32>> MultiLineMessages;
	TArray<int32> FilteredToNormal;
	int32 LinesOfText = 0;
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
	ImGui::Begin("Engine Log", &bIsActive);

	if (!bIsActive)
	{
		SetActiveState(false);
		ImGui::End();
		return false;
	}

	ImGui::SameLine();
	if (Filter.Draw())
	{
		bFiltersDirty = true;
	}

	if (bFiltersDirty)
	{
		FilteredToNormal.Empty();
		LinesOfText = 0;
	}
	
	ImGui::SameLine();
	if (DrawVerbosities())
	{
		bElementsDirty = true;
	}

	ImGui::SameLine();
	if (ImGui::Button("Clear"))
	{
		Clear();
	}

	ImGui::Separator();

	AddNewMessages();
	ValidateMessages();
	
	DrawAllMessages();

	ImGui::End();

	return true;
}

UImGuiEngineOutputLog::~UImGuiEngineOutputLog()
{
	delete Impl;
}

bool UImGuiEngineOutputLog::HasInstance(const UObject* ContextObject)
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

UImGuiEngineOutputLog::ThisClass& UImGuiEngineOutputLog::Get(const UObject* ContextObject)
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

void UImGuiEngineOutputLog::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	Impl = new FImGuiEngineLogImpl;
	Impl->LogBuffer = Cast<UImGuiOutputLogBuffer>(Collection.InitializeDependency(UImGuiOutputLogBuffer::StaticClass()));
}

void UImGuiEngineOutputLog::Deinitialize()
{
	Super::Deinitialize();

	Impl->SetActiveState(false);
}

void UImGuiEngineOutputLog::Tick(float DeltaSeconds)
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

ETickableTickType UImGuiEngineOutputLog::GetTickableTickType() const
{
	return ETickableTickType::Conditional;
}

TStatId UImGuiEngineOutputLog::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(FImGuiEngineLog, STATGROUP_Tickables);
}

bool UImGuiEngineOutputLog::IsTickable() const
{
	return Impl && Impl->bIsActive;
}

bool UImGuiEngineOutputLog::IsTickableWhenPaused() const
{
	return true;
}

bool UImGuiEngineOutputLog::IsTickableInEditor() const
{
	return false;
}

void UImGuiEngineOutputLog::SetActiveState(bool bInIsActive)
{
	Impl->SetActiveState(bInIsActive);
}

bool UImGuiEngineOutputLog::IsActive() const
{
	return Impl->IsActive();
}

void UImGuiEngineOutputLog::SetDisplayedElements(int32 Elements)
{
	Impl->SetDisplayedElements(Elements);
}

void UImGuiEngineOutputLog::AddDisplayedElements(int32 Elements)
{
	Impl->AddDisplayedElements(Elements);
}

void UImGuiEngineOutputLog::RemovedDisplayedElements(int32 Elements)
{
	Impl->RemovedDisplayedElements(Elements);
}

TEnumAsByte<EImGuiOutputLogMessageElement> UImGuiEngineOutputLog::GetDisplayedElements() const
{
	return Impl->GetDisplayedElements();
}

bool FImGuiEngineLogImpl::IsActive() const
{
	return bIsActive;
}

void FImGuiEngineLogImpl::SetActiveState(bool bInIsActive)
{
	bIsActive = bInIsActive;

	const auto* Settings = GetDefault<UImGuiEngineOutputLogSettings>();
	if (Settings->bEnabledInputOnActive)
	{
		GetImGuiModule().SetInputMode(bIsActive);
	}
}

void FImGuiEngineLogImpl::SetDisplayedElements(int32 Elements)
{
	if (ActiveElements != Elements)
	{
		ActiveElements = Elements;
		bElementsDirty = true;
	}
}

void FImGuiEngineLogImpl::AddDisplayedElements(int32 Elements)
{
	if (ActiveElements | Elements)
	{
		ActiveElements |= Elements;
		bElementsDirty = true;
	}
}

void FImGuiEngineLogImpl::RemovedDisplayedElements(int32 Elements)
{
	if (ActiveElements & ~Elements)
	{
		ActiveElements &= ~Elements;
		bElementsDirty = true;
	}
}

EImGuiOutputLogMessageElement FImGuiEngineLogImpl::GetDisplayedElements() const
{
	return static_cast<EImGuiOutputLogMessageElement>(ActiveElements);
}

FImGuiModule& FImGuiEngineLogImpl::GetImGuiModule() const
{
	return *static_cast<FImGuiModule*>(FModuleManager::Get().GetModule("ImGui"));
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
	const int32 Index = Messages.Num();
	auto& Ref = Messages.Add_GetRef(Message);

	const int32 Lines = Ref->LineOffsets.Num();
	for (int32 i = 0; i < Lines; ++i)
	{
		MultiLineMessages.Add({ Ref,i });
	}

	if (!bElementsDirty)
	{
		FormatMessage(Ref);
		if (Filter.IsActive())
		{
			TryFilteringMessage(MultiLineMessages.Num() - Lines);
		}
		else
		{
			LinesOfText += Lines;
		}
	}

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

bool FImGuiEngineLogImpl::DrawVerbosities()
{
	if (ImGui::Button("Verbosities"))
	{
		ImGui::OpenPopup("VerbositiesPopup");
	}

	bool bHasChanged = false;
	if (ImGui::BeginPopup("VerbositiesPopup"))
	{
		ImGui::PushItemFlag(ImGuiItemFlags_AutoClosePopups, false);

		ImGui::SeparatorText("Verbosities");

		bool bShowCategory = ActiveElements & Category;
		bHasChanged |= ImGui::MenuItem("Category", "", &bShowCategory);
		ActiveElements = bShowCategory ? ActiveElements | Category : ActiveElements & ~Category;

		bool bShowVerbosity = ActiveElements & Verbosity;
		bHasChanged |= ImGui::MenuItem("Verbosity", "", &bShowVerbosity);
		ActiveElements = bShowVerbosity ? ActiveElements | Verbosity : ActiveElements & ~Verbosity;
		
		bool bShowTimestamp = ActiveElements & Timestamp;
		bHasChanged |= ImGui::MenuItem("Timestamp", "", &bShowTimestamp);
		ActiveElements = bShowTimestamp ? ActiveElements | Timestamp : ActiveElements & ~Timestamp;

		ImGui::PopItemFlag();
		ImGui::EndPopup();
	}

	return bHasChanged;
}

void FImGuiEngineLogImpl::DrawAllMessages()
{
	if (ImGui::BeginChild("scrolling", ImVec2(0, 0.f), ImGuiChildFlags_None, ImGuiWindowFlags_HorizontalScrollbar))
	{
		if (Filter.IsActive())
		{
			if (bFiltersDirty)
			{
				FilterAllMessages();
			}

			ImGuiListClipper Clipper;
			Clipper.Begin(LinesOfText);

			while (Clipper.Step())
			{
				for (int32 Idx = Clipper.DisplayStart; Idx < Clipper.DisplayEnd; Idx++)
				{
					DrawMessage(FilteredToNormal[Idx]);
					
				}
			}

			Clipper.End();
		}
		else
		{
			ImGuiListClipper Clipper;
			Clipper.Begin(LinesOfText);

			while (Clipper.Step())
			{
				for (int32 Idx = Clipper.DisplayStart; Idx < Clipper.DisplayEnd; Idx++)
				{
					DrawMessage(Idx);
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
}

void FImGuiEngineLogImpl::DrawMessage(int32 Index) const
{
	auto Message = MultiLineMessages[Index].Key;
	const int32 OffsetIndex = MultiLineMessages[Index].Value;

	ImGui::PushStyleColor(ImGuiCol_Text, VerbosityToColor(Message->Verbosity));

	// ImGui::TextUnformatted can't render multi-line texts correctly, so we render each line manually;
	// this is why there's way more code compared to what it could've been

	int32 Offset = OffsetIndex > 0 ? 1 : 0;
	for (int32 i = 0; i < OffsetIndex; ++i)
	{
		Offset += Message->LineOffsets[i];
	}
	
	const int32 LineLength = Message->LineOffsets[OffsetIndex];
	const char* Begin = Message->FormattedText + Offset;
	const char* End = Message->FormattedText + Offset + LineLength - OffsetIndex;
	ImGui::TextUnformatted(Begin, End);

	ImGui::PopStyleColor();
}

void FImGuiEngineLogImpl::FormatMessage(ImGui::Private::TMessageRef Message) const
{
	free(Message->FormattedText);
	
	// The format must NOT introduce any new newlines!

	// Timestamp Verbosity Category Message
	char Fmt[16] = "";
	const char* ToAppend[3] { "", "", "" };
	int32 ToAllocate = Message->TextLen;
	if (ActiveElements & Timestamp)
	{
		static const FString TimestampString = Message->Timestamp.ToString();
		ToAppend[0] = TCHAR_TO_ANSI(*TimestampString);
		ToAllocate += TimestampString.Len() + 1;
		strcat_s(Fmt, "%s ");
	}
	else
	{
		strcat_s(Fmt, "%s");
	}
	
	if (ActiveElements & Verbosity)
	{
		ToAppend[1] = VerbosityToString(Message->Verbosity);
		ToAllocate += ImStrlen(ToAppend[1]) + 1;
		strcat_s(Fmt, "%s ");
	}
	else
	{
		strcat_s(Fmt, "%s");
	}
	
	if (ActiveElements & Category)
	{
		ToAppend[2] = Message->Category;
		ToAllocate += Message->CategoryLen + 1;
		strcat_s(Fmt, "%s ");
	}
	else
	{
		strcat_s(Fmt, "%s");
	}
	
	strcat_s(Fmt, "%s"); // Actual text
	ToAllocate++; // \0
	
	Message->FormattedTextLen = ToAllocate - 1;
	Message->FormattedText = static_cast<char*>(IM_ALLOC(ToAllocate));
	sprintf_s(Message->FormattedText, ToAllocate, Fmt, ToAppend[0], ToAppend[1], ToAppend[2], Message->Text);

	int32 i = 0;
	for (; Message->FormattedText[i] != '\0'; ++i)
	{
		if (Message->FormattedText[i] == '\n')
		{
			break;
		}
	}

	Message->LineOffsets[0] = i;
}

void FImGuiEngineLogImpl::ValidateMessages()
{
	if (bElementsDirty)
	{
		const int32 Num = Messages.Num();
		for (int32 i = 0; i < Num; ++i)
		{
			auto Message = Messages[i];

			FormatMessage(Message);
			if (!bFiltersDirty && Filter.IsActive())
			{
				TryFilteringMessage(MessageToLine(i));
			}
			else
			{
				LinesOfText += Message->LineOffsets.Num();
			}
		}

		bElementsDirty = false;
	}

	if (!Filter.IsActive() && LinesOfText == 0)
	{
		LinesOfText = MultiLineMessages.Num();
	}
}

void FImGuiEngineLogImpl::Clear()
{
	Messages.Empty();
	MultiLineMessages.Empty();
	FilteredToNormal.Empty();
	LinesOfText = 0;
}

void FImGuiEngineLogImpl::TryFilteringMessage(int32 Index)
{
	auto Message = MultiLineMessages[Index].Key;
	Message->bIsFilteredOut = !Filter.PassFilter(Message);

	if (!Message->bIsFilteredOut)
	{
		FilteredToNormal.Add(Index);
		LinesOfText++;
	}
}

void FImGuiEngineLogImpl::FilterAllMessages()
{
	const int32 Num = Messages.Num();
	for (int32 i = 0; i < Num; ++i)
	{
		TryFilteringMessage(MessageToLine(i));
	}
}

int32 FImGuiEngineLogImpl::MessageToLine(int32 MessageIndex) const
{
	int32 Count = 0;
	for (const auto& Message : Messages)
	{
		const int32 Lines = Message->LineOffsets.Num();
		Count += Lines;

		if (Count > MessageIndex)
		{
			return Count - Lines;
		}
	}

	checkNoEntry();
	return INDEX_NONE;
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
	const auto* Settings = GetDefault<UImGuiEngineOutputLogSettings>();
	const FLinearColor* FoundColor = Settings->Colors.Find(Verbosity);
	return FoundColor ? ColorToU32(*FoundColor) : DefaultVerbosityToColor(Verbosity);
}

const char* FImGuiEngineLogImpl::VerbosityToString(EImGuiLogVerbosity Verbosity)
{
	switch (Verbosity)
	{
	case EImGuiLogVerbosity::Verbose: return "Verbose";
	case EImGuiLogVerbosity::VeryVerbose: return "Very Verbose";
	case EImGuiLogVerbosity::Log: return "Log";
	case EImGuiLogVerbosity::Display: return "Display";
	case EImGuiLogVerbosity::Warning: return "Warning";
	case EImGuiLogVerbosity::Error: return "Error";
	case EImGuiLogVerbosity::Fatal: return "Fatal";
	default: return "Invalid";
	}
}

UImGuiEngineOutputLogSettings::UImGuiEngineOutputLogSettings()
{
	for (const auto Verbosity : TEnumRange<EImGuiLogVerbosity>())
	{
		Colors.Add(Verbosity, U32ToColor(DefaultVerbosityToColor(Verbosity)));
	}
}

FName UImGuiEngineOutputLogSettings::GetCategoryName() const
{
	return "Plugins";
}
