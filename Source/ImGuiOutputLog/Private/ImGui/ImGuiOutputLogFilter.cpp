// Author: Antonio Sidenko (Tonetfal), June 2025

#include "ImGui/ImGuiOutputLogFilter.h"

#include "imgui_internal.h"

FImGuiOutputLogFilter::FContext::~FContext()
{
	for (const auto& [Category, _]  : Categories)
	{
		free(Category);
	}
}

FImGuiOutputLogFilter::FImGuiOutputLogFilter()
{
	InputBuf[0] = 0;
}

bool FImGuiOutputLogFilter::Draw()
{
	bool bReturnValue = false;
	bReturnValue |= DrawInputText();
	ImGui::SameLine();
	bReturnValue |= DrawCategoryFilters();
	return bReturnValue;
}

bool FImGuiOutputLogFilter::PassFilter(const ImGui::Private::TMessageRef Message) const
{
	if (!PassFilter_Verbosity(Message))
	{
		return false;
	}

	if (!PassFilter_Text(Message))
	{
		return false;
	}

	if (!PassFilter_Category(Message))
	{
		return false;
	}

	return true;
}

void FImGuiOutputLogFilter::Clear()
{
	InputBuf[0] = 0;
	Build();
}

bool FImGuiOutputLogFilter::IsActive() const
{
	return !Filters.empty()
		|| !Context.bShowErrors
		|| !Context.bShowMessages
		|| !Context.bShowWarnings
		|| Context.DeactivatedCategories > 0;
}

bool FImGuiOutputLogFilter::DrawInputText()
{
	if (Context.Width != 0.0f)
	{
		ImGui::SetNextItemWidth(Context.Width);
	}

	const bool bValueChanged = ImGui::InputTextWithHint(Context.Label, Context.Hint, InputBuf, IM_ARRAYSIZE(InputBuf));
	if (bValueChanged)
	{
		Build();
	}

	return bValueChanged;
}

bool FImGuiOutputLogFilter::DrawCategoryFilters()
{
	if (ImGui::Button("Filters"))
	{
		ImGui::OpenPopup("FiltersPopup");
	}

	bool bHasChanged = false;
	if (ImGui::BeginPopup("FiltersPopup"))
	{
		ImGui::PushItemFlag(ImGuiItemFlags_AutoClosePopups, false);

		ImGui::SeparatorText("Filters");
		const bool bPreviousShowAll = Context.bShowAll;
		bHasChanged |= ImGui::MenuItem("Show All", "", &Context.bShowAll);

		if (bPreviousShowAll != Context.bShowAll)
		{
			Context.DeactivatedCategories = Context.bShowAll ? 0 : Context.Categories.Num();

			for (auto& [Category, bIsEnabled] : Context.Categories)
			{
				bIsEnabled = Context.bShowAll;
			}
		}

		if (ImGui::BeginMenu("Categories"))
		{
			CategoryFilter.Draw("", "Start typing to search");

			for (auto& [Category, bIsEnabled] : Context.Categories)
			{
				if (!CategoryFilter.IsActive() || CategoryFilter.PassFilter(Category))
				{
					const bool bPreviousEnabled = bIsEnabled;
					bHasChanged |= ImGui::MenuItem(Category, "", &bIsEnabled);

					if (bIsEnabled != bPreviousEnabled)
					{
						Context.DeactivatedCategories += bIsEnabled ? -1 : +1;
					}
				}
			}

			ImGui::EndMenu();
		}
		else
		{
			CategoryFilter.Clear();
		}

		ImGui::SeparatorText("Verbosity");

		bHasChanged |= ImGui::MenuItem("Messages", "", &Context.bShowMessages);
		bHasChanged |= ImGui::MenuItem("Warnings", "", &Context.bShowWarnings);
		bHasChanged |= ImGui::MenuItem("Errors", "", &Context.bShowErrors);

		ImGui::PopItemFlag();
		ImGui::EndPopup();
	}

	return bHasChanged;
}

bool FImGuiOutputLogFilter::PassFilter_Verbosity(const ImGui::Private::TMessageRef Message) const
{
	switch (Message->Verbosity)
	{
	case EImGuiLogVerbosity::Verbose:
	case EImGuiLogVerbosity::VeryVerbose:
	case EImGuiLogVerbosity::Log:
	case EImGuiLogVerbosity::Display: return Context.bShowMessages;
	case EImGuiLogVerbosity::Warning: return Context.bShowWarnings;
	case EImGuiLogVerbosity::Error:
	case EImGuiLogVerbosity::Fatal: return Context.bShowErrors;
	default: return true;
	}
}

bool FImGuiOutputLogFilter::PassFilter_Category(const ImGui::Private::TMessageRef Message) const
{
	if (Context.DeactivatedCategories == Context.Categories.Num())
	{
		return false;
	}

	for (const auto& [Category, bIsEnabled] : Context.Categories)
	{
		if (!bIsEnabled)
		{
			continue;
		}

		if (ImStristr(Message->Category, Message->Category + Message->CategoryLen, Category, nullptr))
		{
			return true;
		}
	}

	return false;
}

bool FImGuiOutputLogFilter::PassFilter_Text(const ImGui::Private::TMessageRef Message) const
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
			if (ImStristr(Message->FormattedText, Message->FormattedText + Message->FormattedTextLen,
				Filter.Begin + 1, Filter.End))
			{
				return false;
			}
		}
		else
		{
			// Grep
			if (ImStristr(Message->FormattedText, Message->FormattedText + Message->FormattedTextLen,
				Filter.Begin, Filter.End))
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

FImGuiOutputLogFilter::FTextRange::FTextRange(const char* InBegin, const char* InEnd)
	: Begin(InBegin)
	, End(InEnd)
{
}

bool FImGuiOutputLogFilter::FTextRange::IsEmpty() const
{
	return Begin == End;
}

void FImGuiOutputLogFilter::Build()
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

void FImGuiOutputLogFilter::Split(const char* Buffer, char Separator, ImVector<FTextRange>* Out)
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
