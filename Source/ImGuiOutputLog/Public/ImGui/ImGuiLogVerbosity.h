// Author: Antonio Sidenko (Tonetfal), June 2025

#pragma once

#include "ImGuiLogVerbosity.generated.h"

UENUM()
enum class EImGuiLogVerbosity : uint8
{
	Fatal = ELogVerbosity::Type::Fatal,
	Error,
	Warning,
	Display,
	Log,
	Verbose,
	VeryVerbose
};

ENUM_RANGE_BY_FIRST_AND_LAST(EImGuiLogVerbosity, EImGuiLogVerbosity::Fatal, EImGuiLogVerbosity::VeryVerbose);
