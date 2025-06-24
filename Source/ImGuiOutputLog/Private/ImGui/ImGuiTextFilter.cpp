// Author: Antonio Sidenko (Tonetfal), June 2025

#include "ImGui/ImGuiTextFilter.h"

#include "imgui_internal.h"

FImGuiTextFilter::FImGuiTextFilter(const char* DefaultFilter)
{
	InputBuf[0] = 0;
	CountGrep = 0;
	if (DefaultFilter)
	{
		ImStrncpy(InputBuf, DefaultFilter, IM_ARRAYSIZE(InputBuf));
		Build();
	}
}

bool FImGuiTextFilter::Draw(const char* Label, const char* Hint, float Width)
{
	if (Width != 0.0f)
	{
		ImGui::SetNextItemWidth(Width);
	}

	const bool bValueChanged = ImGui::InputTextWithHint(Label, Hint, InputBuf, IM_ARRAYSIZE(InputBuf));
	if (bValueChanged)
	{
		Build();
	}

	return bValueChanged;
}

bool FImGuiTextFilter::PassFilter(const char* Text, const char* TextEnd) const
{
	if (Filters.Size == 0)
	{
		return true;
	}

	for (const FTextRange& Filter : Filters)
	{
		if (Filter.Begin == Filter.End)
		{
			continue;
		}

		if (Filter.Begin[0] == '-')
		{
			// Subtract
			if (ImStristr(Text, TextEnd, Filter.Begin + 1, Filter.End))
			{
				return false;
			}
		}
		else
		{
			// Grep
			if (ImStristr(Text, TextEnd, Filter.Begin, Filter.End))
			{
				return true;
			}
		}
	}

	// Implicit * grep
	if (CountGrep == 0)
	{
		return true;
	}

	return false;
}

void FImGuiTextFilter::Clear()
{
	InputBuf[0] = 0;
	Build();
}

bool FImGuiTextFilter::IsActive() const
{
	return !Filters.empty();
}

FImGuiTextFilter::FTextRange::FTextRange(const char* InBegin, const char* InEnd)
	: Begin(InBegin)
	, End(InEnd)
{
}

bool FImGuiTextFilter::FTextRange::IsEmpty() const
{
	return Begin == End;
}

void FImGuiTextFilter::Build()
{
	Filters.resize(0);
	Split(InputBuf, ',', &Filters);

	CountGrep = 0;
	for (FTextRange& Filter : Filters)
	{
		while (Filter.Begin < Filter.End && ImCharIsBlankA(Filter.Begin[0]))
		{
			Filter.Begin++;
		}

		while (Filter.End > Filter.Begin && ImCharIsBlankA(Filter.End[-1]))
		{
			Filter.End--;
		}

		if (Filter.IsEmpty())
		{
			continue;
		}

		if (Filter.Begin[0] != '-')
		{
			CountGrep++;
		}
	}
}

void FImGuiTextFilter::Split(const char* Buffer, char Separator, ImVector<FTextRange>* Out)
{
	Out->resize(0);

	const char* BufferEnd = Buffer + ImStrlen(Buffer);
	const char* CurrentBegin = Buffer;
	const char* CurrentEnd = CurrentBegin;

	while (CurrentEnd < BufferEnd)
	{
		if (*CurrentEnd == Separator)
		{
			Out->push_back(FTextRange(CurrentBegin, CurrentEnd));
			CurrentBegin = CurrentEnd + 1;
		}

		CurrentEnd++;
	}

	if (CurrentBegin != CurrentEnd)
	{
		Out->push_back(FTextRange(CurrentBegin, CurrentEnd));
	}
}
