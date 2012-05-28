/*  Project nGenEx
	Destroyer Studios LLC
	Copyright © 1997-2004. All Rights Reserved.

	SUBSYSTEM:    nGenEx.lib
	FILE:         Layout.h
	AUTHOR:       John DiCamillo


	OVERVIEW
	========
	Layout Manager class for ActiveWindow panels
*/

#ifndef Layout_h
#define Layout_h

#include "ActiveWindow.h"

// +--------------------------------------------------------------------+

class Layout
{
public:
	static const char* TYPENAME() { return "Layout"; }

	Layout();
	virtual ~Layout();

	virtual bool   DoLayout(ActiveWindow* panel);

	virtual void   Clear();
	virtual void   AddCol(DWORD min_width,  float col_factor);
	virtual void   AddRow(DWORD min_height, float row_factor);

	virtual void   SetConstraints(const std::vector<DWORD>& min_x,
	const std::vector<DWORD>& min_y,
	const std::vector<float>& weight_x,
	const std::vector<float>& weight_y);

	virtual void   SetConstraints(const std::vector<float>& min_x,
	const std::vector<float>& min_y,
	const std::vector<float>& weight_x,
	const std::vector<float>& weight_y);

	virtual void   SetConstraints(int            ncols,
	int            nrows,
	const int*     min_x,
	const int*     min_y,
	const float*   weight_x,
	const float*   weight_y);


protected:
	virtual void   ScaleWeights();
	virtual void   CalcCells(DWORD w, DWORD h, std::vector<DWORD>& cell_x, std::vector<DWORD>& cell_y);

	std::vector<DWORD>   cols;
	std::vector<DWORD>   rows;
	std::vector<float>   col_weights;
	std::vector<float>   row_weights;
};

#endif Layout_h

