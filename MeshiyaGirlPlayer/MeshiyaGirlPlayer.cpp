// MeshiyaGirlPlayer.cpp : アプリケーションのエントリ ポイントを定義します。
//

#include "framework.h"
#include "MeshiyaGirlPlayer.h"

#include "win_dialogue.h"
#include "meshiya.h"
#include "sfml_sprite_player.h"

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    std::wstring wstrPickedFilePath = win_dialogue::SelectOpenFile(L"JSON file", L"*.json;");
    if (!wstrPickedFilePath.empty())
    {
        std::vector<adv::TextDatum> textData;
        std::vector<adv::ImageDatum> imageData;
        bool bRet = meshiya::LoadScenario(wstrPickedFilePath, textData, imageData);
        if (bRet)
        {
            CSfmlSpritePlayer SfmlPlayer;
            bRet = SfmlPlayer.SetFont("font\\yumindb.ttf", true, true);
            if (bRet)
            {
                SfmlPlayer.SetResources(textData, imageData);
                int iRet = SfmlPlayer.Display(L"メシアガール");
            }
        }
    }

    return 0;
}
