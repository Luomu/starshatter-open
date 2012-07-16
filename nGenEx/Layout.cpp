/*  Project nGenEx
	Destroyer Studios LLC
	Copyright (C) 1997-2004. All Rights Reserved.

	SUBSYSTEM:    nGenEx.lib
	FILE:         Layout.cpp
	AUTHOR:       John DiCamillo


	OVERVIEW
	========
	Layout Resource class implementation
*/

#include "MemDebug.h"
#include "Layout.h"

// +--------------------------------------------------------------------+

Layout::Layout()
{ }

Layout::~Layout()
{ }

// +--------------------------------------------------------------------+

bool
Layout::DoLayout(ActiveWindow* panel)
{
	if (!panel || panel->GetChildren().size() < 1)
	return false;

	if (cols.size() < 1 || rows.size() < 1)
	return false;

	std::vector<DWORD>   cell_x;
	std::vector<DWORD>   cell_y;

	ScaleWeights();
	CalcCells(panel->Width(), panel->Height(), cell_x, cell_y);

	ListIter<ActiveWindow> iter = panel->GetChildren();
	while (++iter) {
		ActiveWindow* w = iter.value();
		Rect          c = w->GetCells();
		Rect          r;
		Rect          rp = panel->GetRect();

		if (c.x < 0)                     c.x = 0;
		else if (c.x >= (int)cell_x.size())   c.x = cell_x.size() - 1;
		if (c.y < 0)                     c.y = 0;
		else if (c.y >= (int)cell_y.size())   c.y = cell_y.size() - 1;
		if (c.x+c.w  >= (int)cell_x.size())   c.w = cell_x.size() - c.x - 1;
		if (c.y+c.h  >= (int)cell_y.size())   c.h = cell_y.size() - c.y - 1;

		r.x = cell_x[c.x]     + w->GetCellInsets().left;
		r.y = cell_y[c.y]     + w->GetCellInsets().top;
		r.w = cell_x[c.x+c.w] - w->GetCellInsets().right  - r.x;
		r.h = cell_y[c.y+c.h] - w->GetCellInsets().bottom - r.y;

		r.x += panel->X();
		r.y += panel->Y();

		if (w->GetFixedWidth() && w->GetFixedWidth() < r.w)
		r.w = w->GetFixedWidth();

		if (w->GetFixedHeight() && w->GetFixedHeight() < r.h)
		r.h = w->GetFixedHeight();

		if (w->GetID() == 330 || w->GetID() == 125) {
			int y1 = r.y  + r.h;
			int y2 = rp.y + rp.h;
		}

		if (w->GetHidePartial() && (r.x + r.w > rp.x + rp.w)) {
			w->MoveTo(Rect(0,0,0,0));
		}

		else if (w->GetHidePartial() && (r.y + r.h > rp.y + rp.h)) {
			w->MoveTo(Rect(0,0,0,0));
		}

		else {
			w->MoveTo(r);
		}
	}

	return true;   
}

// +--------------------------------------------------------------------+

void
Layout::ScaleWeights()
{
	float total = 0;

	for (auto cwi = col_weights.begin(); cwi != col_weights.end(); ++cwi)
		total += *cwi;

	if (total > 0) {
		for (auto cwi = col_weights.begin(); cwi != col_weights.end(); ++cwi)
			*cwi = *cwi / total;
	}

	total = 0;
	for (auto rwi = row_weights.begin(); rwi != row_weights.end(); ++rwi)
		total += *rwi;

	if (total > 0) {
		for (auto rwi = row_weights.begin(); rwi != row_weights.end(); ++rwi)
			*rwi = *rwi / total;
	}
}

// +--------------------------------------------------------------------+

void
	Layout::CalcCells(DWORD w, DWORD h, std::vector<DWORD>& cell_x, std::vector<DWORD>& cell_y)
{
	DWORD       x     = 0;
	DWORD       y     = 0;
	DWORD       min_x = 0;
	DWORD       min_y = 0;
	DWORD       ext_x = 0;
	DWORD       ext_y = 0;

	for (auto cit = cols.begin(); cit != cols.end(); ++cit)
		min_x += *cit;

	for (auto rit = rows.begin(); rit != rows.end(); ++rit)
		min_y += *rit;

	if (min_x < w)
		ext_x = w - min_x;

	if (min_y < h)
		ext_y = h - min_y;

	cell_x.push_back(x);
	for (auto cit = cols.begin(); cit != cols.end(); ++cit) {
		x += *cit + (DWORD) (ext_x * col_weights[cit - cols.begin()]);
		cell_x.push_back(x);
	}

	cell_y.push_back(y);
	for (auto rit = rows.begin(); rit != rows.end(); ++rit) {
		y += *rit + (DWORD) (ext_y * row_weights[rit - rows.begin()]);
		cell_y.push_back(y);
	}
}

// +--------------------------------------------------------------------+

void
Layout::Clear()
{
	cols.clear();
	rows.clear();

	col_weights.clear();
	row_weights.clear();
}

void
Layout::AddCol(DWORD min_width, float col_factor)
{
	cols.push_back(min_width);
	col_weights.push_back(col_factor);
}

void
Layout::AddRow(DWORD min_height, float row_factor)
{
	rows.push_back(min_height);
	row_weights.push_back(row_factor);
}

void
Layout::SetConstraints(const std::vector<DWORD>& min_x,
const std::vector<DWORD>& min_y,
const std::vector<float>& weight_x,
const std::vector<float>& weight_y)
{
	Clear();

	if (min_x.size() == weight_x.size() && min_y.size() == weight_y.size()) {
		for (auto iter = min_x.begin(); iter != min_x.end(); ++iter)
			cols.push_back(*iter);

		for (auto iter = min_y.begin(); iter != min_y.end(); ++iter)
			rows.push_back(*iter);

		for (auto iter = weight_x.begin(); iter != weight_x.end(); ++iter)
			col_weights.push_back(*iter);

		for (auto iter = weight_y.begin(); iter != weight_y.end(); ++iter)
			row_weights.push_back(*iter);
	}
}

void
Layout::SetConstraints(const std::vector<float>& min_x,
const std::vector<float>& min_y,
const std::vector<float>& weight_x,
const std::vector<float>& weight_y)
{
	Clear();

	if (min_x.size() == weight_x.size() && 
			min_y.size() == weight_y.size()) {

		for (auto iter = min_x.begin(); iter != min_x.end(); ++iter)
			cols.push_back((DWORD) *iter);

		for (auto iter = min_y.begin(); iter != min_y.end(); ++iter)
			rows.push_back((DWORD) *iter);

		for (auto iter = weight_x.begin(); iter != weight_x.end(); ++iter)
			col_weights.push_back(*iter);

		for (auto iter = weight_y.begin(); iter != weight_y.end(); ++iter)
			row_weights.push_back(*iter);
	}
}

void
Layout::SetConstraints(int            ncols,
int            nrows,
const int*     min_x,
const int*     min_y,
const float*   weight_x,
const float*   weight_y)
{
	Clear();

	if (nrows > 0 && ncols > 0) {
		int i = 0;

		for (i = 0; i < ncols; i++) {
			cols.push_back(min_x[i]);
			col_weights.push_back(weight_x[i]);
		}

		for (i = 0; i < nrows; i++) {
			rows.push_back(min_y[i]);
			row_weights.push_back(weight_y[i]);
		}
	}
}
