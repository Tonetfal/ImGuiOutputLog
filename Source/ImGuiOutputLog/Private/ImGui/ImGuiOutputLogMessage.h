// Author: Antonio Sidenko (Tonetfal), June 2025

#pragma once

#include "ImGui/ImGuiLogVerbosity.h"

struct FImGuiOutputLogMessage
{
public:
	~FImGuiOutputLogMessage();

public:
	char* Text = nullptr;
	int32 TextLen = 0;
	TArray<int32> LineOffsets;

	char* Category = nullptr;
	int32 CategoryLen = 0;

	EImGuiLogVerbosity Verbosity = EImGuiLogVerbosity::Log;

	char* FormattedText = nullptr;
	int32 FormattedTextLen = 0;

	FDateTime Timestamp;
	bool bIsFilteredOut = false;
};

namespace ImGui::Private
{
	using TMessage = FImGuiOutputLogMessage;
	using TMessageRef = TSharedRef<TMessage>;
}
