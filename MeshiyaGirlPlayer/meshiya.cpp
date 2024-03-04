
#include <unordered_map>

#include "meshiya.h"
#include "win_filesystem.h"
#include "win_text.h"
#include "win_dialogue.h"

#include "deps/nlohmann/json.hpp"

namespace meshiya
{
	enum TokenDataType
	{
		kText = 1,
		kVoice,
		kStill,
	};

	struct StillData
	{
		int iLayer = 0;
		int iStillId = 0;
	};

	struct MessageData
	{
		std::wstring wstrName;
		std::wstring wstrText;
	};

	struct TokenDatum
	{
		int iType = 0;
		MessageData messageData;
		StillData stillData;
		std::string strVoicePath;
	};

	constexpr char g_szVoiceFolder[] = "audiosVoice\\";
	constexpr char g_szVoiceFileExtension[] = ".mp3";
	constexpr char g_szImageFolder[] = "adventure\\sprite\\still\\";
	constexpr char g_szImageFileExtension[] = ".png";

	std::string g_strAssetFolderPath;

	void ReadScenarioFile(const std::string& strScenarioFile, std::vector<TokenDatum>& tokenData, std::unordered_map<int, std::string>& fileIds, std::string& strError)
	{
		nlohmann::json nlJson;
		try
		{
			nlJson = nlohmann::json::parse(strScenarioFile);

			nlohmann::json& jData = nlJson[5][0][2];
			if (!jData.contains("tokenData") || !jData.contains("loadData"))return;

			nlohmann::json& jsonTokenData = jData["tokenData"];
			size_t nSize = jsonTokenData.size();
			for (size_t i = 0; i < nSize; ++i)
			{
				TokenDatum tokenDatum;
				if (jsonTokenData[i].contains("type"))
				{
					/*テキスト*/
					int iType = jsonTokenData[i]["type"];
					if (iType == 0)
					{
						if (jsonTokenData[i].contains("message"))
						{
							/*
							* 地の文 - "name"要素なし
							* 男台詞 - "name" : "%name"
							* 女台詞(例) - "name" : "マオ"
							*/
							if (jsonTokenData[i].contains("name"))
							{
								std::string str = std::string(jsonTokenData[i]["name"]);
								tokenDatum.messageData.wstrName = win_text::WidenUtf8(str);
							}
							std::string str = std::string(jsonTokenData[i]["message"]);

							tokenDatum.iType = TokenDataType::kText;
							std::wstring wstr = win_text::WidenUtf8(str);
							for (;;)
							{
								size_t nPos = wstr.find(L"▼");
								if (nPos == std::wstring::npos)break;
								wstr.replace(nPos, 1, L"♡");
							}
							tokenDatum.messageData.wstrText = wstr;
							tokenData.emplace_back(tokenDatum);
						}
					}
					else if (iType == 1)
					{
						if (jsonTokenData[i].contains("params"))
						{
							nlohmann::json& jParams = jsonTokenData[i]["params"];
							if (jParams.size() > 3)
							{
								/*音声指定*/
								if (jParams[3].is_string() && jParams[2] == "vo")
								{
									tokenDatum.iType = TokenDataType::kVoice;
									tokenDatum.strVoicePath = std::string(jParams[3]);
									tokenData.emplace_back(tokenDatum);
								}
								/*静画指定*/
								if (jParams[2].is_string() && jParams[2] == "still")
								{
									int iLayer = jParams[1];
									int iStillId = jParams[3];

									tokenDatum.iType = TokenDataType::kStill;
									tokenDatum.stillData.iStillId = iStillId;
									tokenDatum.stillData.iLayer = iLayer;
									tokenData.emplace_back(tokenDatum);
								}
							}
						}
					}
				}

			}

			nlohmann::json jsonLoadData = jData["loadData"];
			nSize = jsonLoadData.size();
			for (size_t i = 0; i < nSize; ++i)
			{
				/*多重記載あり*/
				if (jsonLoadData[i].contains("id") && jsonLoadData[i].contains("name") && jsonLoadData[i].contains("type"))
				{
					int id = jsonLoadData[i]["id"];
					std::string str = std::string(jsonLoadData[i]["name"]);
					fileIds.insert({ id, str });
				}
			}
		}
		catch (nlohmann::json::exception e)
		{
			strError = e.what();
		}
	}

	bool SetAssetFolderPathFromAdvTextPath(const std::wstring& wstrScenarioFiLePath)
	{
		size_t nPos = wstrScenarioFiLePath.rfind(L"assets\\");
		if (nPos == std::wstring::npos)return false;
		g_strAssetFolderPath = win_text::NarrowUtf8(wstrScenarioFiLePath.substr(0, nPos + wcslen(L"assets\\")));
		return true;
	}

	std::string StillUrlToFilePath(const std::string& strRelativePath)
	{
		if (g_strAssetFolderPath.empty())return std::string();

		std::string str = g_strAssetFolderPath;
		str += g_szImageFolder;
		str += win_text::ReplaceStr(strRelativePath, "/", "\\");
		str += g_szImageFileExtension;
		return str;
	}

	std::string VoiceUrlToFilePath(const std::string& strRelativePath)
	{
		if (g_strAssetFolderPath.empty())return std::string();

		std::string str = g_strAssetFolderPath;
		str += g_szVoiceFolder;
		str += win_text::ReplaceStr(strRelativePath, "/", "\\");
		str += g_szVoiceFileExtension;
		return str;
	}

} // namespace meshiya

bool meshiya::LoadScenario(const std::wstring& wstrFilePath, std::vector<adv::TextDatum>& voiceData, std::vector<adv::ImageDatum>& imageData)
{
	std::string strScenarioFile = win_filesystem::LoadFileAsString(wstrFilePath.c_str());
	if (!strScenarioFile.empty())
	{
		bool bRet = SetAssetFolderPathFromAdvTextPath(wstrFilePath);
		if (bRet)
		{
			std::vector<TokenDatum> tokenData;
			std::unordered_map<int, std::string> fileIds;
			std::string strError;
			ReadScenarioFile(strScenarioFile, tokenData, fileIds, strError);
			if (strError.empty())
			{
				std::string strVoicePath;
				for (size_t i = 0; i < tokenData.size(); ++i)
				{
					const TokenDatum& tokenDatum = tokenData.at(i);
					switch (tokenDatum.iType)
					{
					case TokenDataType::kStill:
					{
						adv::ImageDatum imageDatum;
						auto iter = fileIds.find(tokenDatum.stillData.iStillId);
						if (iter != fileIds.end())
						{
							std::string str = StillUrlToFilePath(iter->second);
							int iLayer = 0;
							/*stillData.iLayerの値で制御するのは無理っぽい。*/
							if (iter->second.rfind("_up_") != std::string::npos)
							{
								iLayer = 2;
							}
							else if (iter->second.rfind("_face_") != std::string::npos)
							{
								iLayer = 1;
							}
							imageDatum.strImagePath = str;
							imageDatum.iLayer = iLayer;
							imageData.emplace_back(imageDatum);
						}
					}
					break;
					case TokenDataType::kText:
					{
						adv::TextDatum voiceDatum;
						voiceDatum.wstrText = tokenDatum.messageData.wstrText;
						if (!strVoicePath.empty())
						{
							voiceDatum.strVoicePath = strVoicePath;
							strVoicePath.clear();
						}
						voiceData.emplace_back(voiceDatum);
					}
						break;
					case TokenDataType::kVoice:
						strVoicePath = VoiceUrlToFilePath(tokenDatum.strVoicePath);
						break;
					}
				}
			}
			else
			{
				win_dialogue::ShowMessageBox("Parse error", strError.c_str());
			}
		}

	}
	return !imageData.empty();
}