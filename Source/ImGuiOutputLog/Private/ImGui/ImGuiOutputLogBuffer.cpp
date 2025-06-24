// Author: Antonio Sidenko (Tonetfal), June 2025

#include "ImGui/ImGuiOutputLogBuffer.h"

#include "imgui_internal.h"

void UImGuiOutputLogBuffer::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	check(GLog);
	GLog->AddOutputDevice(this);
}

void UImGuiOutputLogBuffer::Deinitialize()
{
	if (GLog)
	{
		GLog->RemoveOutputDevice(this);
	}

	Super::Deinitialize();
}

void UImGuiOutputLogBuffer::Serialize(const TCHAR* Message, ELogVerbosity::Type Verbosity, const FName& Category)
{
	if (Verbosity >= ELogVerbosity::Fatal && Verbosity <= ELogVerbosity::VeryVerbose && Message && Message[0] != '\0')
	{
		auto& LogMessage = Messages.Add_GetRef(MakeShared<ImGui::Private::TMessage>());

		LogMessage->Text = ImStrdup(TCHAR_TO_ANSI(Message));

		int32 PreviousOffset = 0;
		int32 i = 0;
		for (; LogMessage->Text[i] != '\0'; ++i)
		{
			if (LogMessage->Text[i] == '\n')
			{
				LogMessage->LineOffsets.Add(i - PreviousOffset);
				PreviousOffset = i;
			}
		}
		
		LogMessage->TextLen = i;
		LogMessage->LineOffsets.Add(LogMessage->TextLen - PreviousOffset);

		LogMessage->Category = ImStrdup(TCHAR_TO_ANSI(*Category.ToString()));
		LogMessage->CategoryLen = ImStrlen(LogMessage->Category);

		LogMessage->Verbosity = static_cast<EImGuiLogVerbosity>(Verbosity);
		LogMessage->Timestamp = FDateTime::Now();
	}
}
