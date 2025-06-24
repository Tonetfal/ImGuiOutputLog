// Author: Antonio Sidenko (Tonetfal), June 2025

#pragma once

#include "imgui.h"

struct FImGuiTextFilter
{
public:
	FImGuiTextFilter(const char* DefaultFilter = "");

	bool Draw(const char* Label = "Filter (inc,-exc)", const char* Hint = "", float Width = 0.0f);
	bool PassFilter(const char* Text, const char* TextEnd = nullptr) const;

	void Clear();
	bool IsActive() const;

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

private:
	char InputBuf[256];
	ImVector<FTextRange> Filters;
	int32 CountGrep;
};
