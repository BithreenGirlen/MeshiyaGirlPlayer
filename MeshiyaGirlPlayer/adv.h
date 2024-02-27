#ifndef ADV_H_
#define ADV_H_

#include <string>

namespace adv
{
	struct TextDatum
	{
		std::wstring wstrText;
		std::string strVoicePath;
	};

	struct ImageDatum
	{
		int iLayer = 0;
		std::string strImagePath;
	};
}

#endif // ADV_H_
