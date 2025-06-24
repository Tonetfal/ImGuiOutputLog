// Author: Antonio Sidenko (Tonetfal), June 2025

#include "ImGuiOutputLogMessage.h"

FImGuiOutputLogMessage::~FImGuiOutputLogMessage()
{
	free(Text);
	free(Category);
	free(FormattedText);
}
