#pragma once
#include "tools/cli_tool.h"

namespace Halley
{
	class DistanceFieldTool : public CommandLineTool
	{
	public:
		int run(std::vector<std::string> args) override;
	};
}
