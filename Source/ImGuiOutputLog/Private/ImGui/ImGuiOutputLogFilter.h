// Author: Antonio Sidenko (Tonetfal), June 2025

#pragma once

#include "imgui.h"
#include "ImGui/ImGuiTextFilter.h"
#include "ImGui/ImGuiOutputLogMessage.h"

struct FImGuiOutputLogFilter
{
public:
	struct FContext
	{
	public:
		~FContext();

	public:
		const char* Label = "";
		const char* Hint = "";
		float Width = 0.0f;

		bool bShowAll = true;
		TArray<TPair<char*, bool>> Categories;
		int32 DeactivatedCategories = 0;

		bool bShowMessages = true;
		bool bShowWarnings = true;
		bool bShowErrors = true;
	};

public:
	FImGuiOutputLogFilter();

	bool Draw();
	bool PassFilter(const ImGui::Private::TMessageRef Message) const;
	void Clear();
	bool IsActive() const;

private:
	bool DrawInputText();
	bool DrawCategoryFilters();

	bool PassFilter_Verbosity(const ImGui::Private::TMessageRef Message) const;
	bool PassFilter_Category(const ImGui::Private::TMessageRef Message) const;
	bool PassFilter_Text(const ImGui::Private::TMessageRef Message) const;

private:
	struct FTextRange
	{
	public:
		FTextRange(const char* InBegin, const char* InEnd);
		bool IsEmpty() const;

	public:
		const char* Begin;
		const char* End;
	};

	void Build();
	static void Split(const char* Buffer, char Separator, ImVector<FTextRange>* Out);

public:
	FContext Context;

private:
	char InputBuf[256];
	ImVector<FTextRange> Filters;
	int CountGrep = 0;

    FImGuiTextFilter CategoryFilter;
};
