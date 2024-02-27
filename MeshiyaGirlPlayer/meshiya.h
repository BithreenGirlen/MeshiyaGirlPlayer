#ifndef MESHIYA_H_
#define MESHIYA_H_

#include <string>
#include <vector>

#include "adv.h"

namespace meshiya
{
	bool LoadScenario(const std::wstring& wstrFilePath, std::vector<adv::TextDatum> &voiceData, std::vector<adv::ImageDatum> &imageData);
}

#endif //MESHIYA_SCENARIO_H_
