

#include "sfml_sprite_player.h"

#pragma comment(lib, "opengl32.lib")
#pragma comment(lib, "winmm.lib")

#ifdef  _DEBUG
#pragma comment(lib, "sfml-system-d.lib")
#pragma comment(lib, "sfml-graphics-d.lib")
#pragma comment(lib, "sfml-window-d.lib")
#pragma comment(lib, "sfml-audio-d.lib")
#else
#pragma comment(lib, "sfml-system.lib")
#pragma comment(lib, "sfml-graphics.lib")
#pragma comment(lib, "sfml-window.lib")
#pragma comment(lib, "sfml-audio.lib")
#endif // _DEBUG


CSfmlSpritePlayer::CSfmlSpritePlayer()
{

}

CSfmlSpritePlayer::~CSfmlSpritePlayer()
{

}

bool CSfmlSpritePlayer::SetFont(const std::string& strFilePath, bool bBold, bool bItalic)
{
    bool bRet = m_font.loadFromFile(strFilePath);
    if (!bRet)return false;

	constexpr float fOutLineThickness = 1.2f;

    m_msgText.setFont(m_font);
    m_msgText.setFillColor(sf::Color::Black);
    m_msgText.setStyle((bBold ? sf::Text::Style::Bold : 0) | (bItalic ? sf::Text::Style::Italic : 0));
	m_msgText.setOutlineThickness(fOutLineThickness);
	m_msgText.setOutlineColor(sf::Color::White);

	m_imgText.setFont(m_font);
	m_imgText.setFillColor(sf::Color::Black);
	m_imgText.setStyle((bBold ? sf::Text::Style::Bold : 0) | (bItalic ? sf::Text::Style::Italic : 0));
	m_imgText.setOutlineThickness(fOutLineThickness);
	m_imgText.setOutlineColor(sf::Color::White);
    return true;
}

void CSfmlSpritePlayer::SetResources(const std::vector<adv::TextDatum>& textData, const std::vector<adv::ImageDatum>& imageData)
{
    m_textData = textData;
    m_imageData = imageData;
}

int CSfmlSpritePlayer::Display(const std::wstring& wstrWindowName)
{
	SetupSprite();
	m_sound.setVolume(m_audioVolume);
	UpdateMessageText();
	UpdateSprite();

    m_window = std::make_unique<sf::RenderWindow>(sf::VideoMode(m_BaseWindowSize.x, m_BaseWindowSize.y), wstrWindowName, sf::Style::None);
    m_window->setPosition(sf::Vector2i(0, 0));
	AdjustTextPosition();
    
	sf::Vector2i iMouseStartPos;
	bool bOnWindowMove = false;

	sf::Event event;
	while (m_window->isOpen())
	{
		while (m_window->pollEvent(event))
		{
			switch (event.type)
			{
			case sf::Event::Closed:
				m_window->close();
				break;
			case sf::Event::MouseButtonPressed:
				if (event.mouseButton.button == sf::Mouse::Left)
				{
					iMouseStartPos.x = event.mouseButton.x;
					iMouseStartPos.y = event.mouseButton.y;
				}
				break;
			case sf::Event::MouseButtonReleased:
				if (event.mouseButton.button == sf::Mouse::Left)
				{
					if (bOnWindowMove || sf::Mouse::isButtonPressed(sf::Mouse::Right))
					{
						bOnWindowMove ^= true;
						break;
					}

					int iX = iMouseStartPos.x - event.mouseButton.x;
					int iY = iMouseStartPos.y - event.mouseButton.y;

					if (iX != 0 && iY != 0)
					{
						AddOffset(sf::Vector2i(iX, iY));
					}
				}
				if (event.mouseButton.button == sf::Mouse::Middle)
				{
					ResetScale();
				}
				if (event.mouseButton.button == sf::Mouse::Right)
				{

				}
				break;
			case sf::Event::MouseWheelScrolled:
				if (sf::Mouse::isButtonPressed(sf::Mouse::Left))
				{
					ShiftSprite(event.mouseWheelScroll.delta < 0);
				}
				else if (sf::Mouse::isButtonPressed(sf::Mouse::Right))
				{
					ShiftMessageText(event.mouseWheelScroll.delta < 0);
				}
				else
				{
					RescaleSprite(event.mouseWheelScroll.delta < 0);
				}
				break;
			case sf::Event::KeyReleased:
				switch (event.key.code)
				{
				case sf::Keyboard::Key::C:
					SwitchTextColor();
					break;
				case sf::Keyboard::Key::S:
					SwitchSmoothMode();
					break;
				case sf::Keyboard::Key::T:
					m_bTextHidden ^= true;
					break;
				case sf::Keyboard::Key::Escape:
					m_window->close();
					break;
				case sf::Keyboard::Left:
					ChangeVolume(false);
					break;
				case sf::Keyboard::Right:
					ChangeVolume(true);
					break;
				default:
					break;
				}
				break;
			}
		}

		m_window->clear();

		for (size_t i = 0; i < m_sprites.size(); ++i)
		{
			m_window->draw(m_sprites.at(i), i == 0 ? sf::RenderStates(sf::BlendNone) : sf::RenderStates(sf::BlendAlpha));
		}
		if (!m_bTextHidden)
		{
			m_window->draw(m_msgText);
			m_window->draw(m_imgText);
		}

		m_window->display();

		CheckTimer();

		if (bOnWindowMove)
		{
			int iPosX = sf::Mouse::getPosition().x - m_window->getSize().x / 2;
			int iPosY = sf::Mouse::getPosition().y - m_window->getSize().y / 2;
			m_window->setPosition(sf::Vector2i(iPosX, iPosY));
		}
	}

    return 0;
}
/*塗り重ね枚数設定*/
void CSfmlSpritePlayer::SetupSprite()
{
	size_t nMaxLayer = 0;
	for (const adv::ImageDatum& imageDatum : m_imageData)
	{
		nMaxLayer = nMaxLayer < imageDatum.iLayer ? imageDatum.iLayer : nMaxLayer;
	}
	++nMaxLayer;
	for (size_t i = 0; i < nMaxLayer; ++i)
	{
		sf::Texture texture;
		sf::Sprite sprite;
		m_textures.emplace_back(texture);
		m_sprites.emplace_back(sprite);
	}

	int iInitialLayer = m_imageData.at(0).iLayer;
	bool bRet = m_textures.at(iInitialLayer).loadFromFile(m_imageData.at(0).strImagePath);
	if (bRet)
	{
		m_BaseWindowSize = m_textures.at(iInitialLayer).getSize();
		m_sprites.at(iInitialLayer).setTexture(m_textures.at(iInitialLayer));
		for (size_t i = 1; i < m_imageData.size(); ++i)
		{
			int iLayer = m_imageData.at(i).iLayer;
			if (iLayer <= iInitialLayer)break;
			iInitialLayer = iLayer;

			bRet = m_textures.at(iLayer).loadFromFile(m_imageData.at(i).strImagePath);
			if (bRet)
			{
				m_sprites.at(iLayer).setTexture(m_textures.at(iLayer));
			}
		}
	}

}
/*文字色切り替え*/
void CSfmlSpritePlayer::SwitchTextColor()
{
	m_msgText.setFillColor(m_msgText.getFillColor() == sf::Color::Black ? sf::Color::White : sf::Color::Black);
	m_imgText.setFillColor(m_imgText.getFillColor() == sf::Color::Black ? sf::Color::White : sf::Color::Black);

	m_msgText.setOutlineColor(m_msgText.getFillColor() == sf::Color::Black ? sf::Color::White : sf::Color::Black);
	m_imgText.setOutlineColor(m_imgText.getFillColor() == sf::Color::Black ? sf::Color::White : sf::Color::Black);
}
/*平滑補正切り替え*/
void CSfmlSpritePlayer::SwitchSmoothMode()
{
	/*
	* 境界線が際立つので透過情報によっては相性が悪い。
	* 身体・顔表情までなら問題ないものが多いが、駄目なものも有り。
	*/
	for (size_t i = 0; i < m_textures.size(); ++i)
	{
		bool bToBeSmoothed = m_textures.at(i).isSmooth() ^ true;
		m_textures.at(i).setSmooth(bToBeSmoothed);
	}
}
/*拡縮変更*/
void CSfmlSpritePlayer::RescaleSprite(bool bUpscale)
{
	constexpr float kScalePortion = 0.05f;
	constexpr float kMinScale = 0.5f;
	if (bUpscale)
	{
		m_fSpriteScale += kScalePortion;
	}
	else
	{
		m_fSpriteScale -= kScalePortion;
		if (m_fSpriteScale < kMinScale - 0.01f)m_fSpriteScale = kMinScale;
	}
	ResizeWindow();
}
/*窓枠寸法変更*/
void CSfmlSpritePlayer::ResizeWindow()
{
	if (m_window.get() != nullptr)
	{
		m_window->setSize(sf::Vector2u(static_cast<unsigned int>(m_BaseWindowSize.x * m_fSpriteScale), static_cast<unsigned int>(m_BaseWindowSize.y * m_fSpriteScale)));
	}
	AdjustOffset();
	AdjustTextPosition();
}
/*視点移動*/
void CSfmlSpritePlayer::AddOffset(sf::Vector2i iOffset)
{
	m_iOffset += iOffset;
	AdjustOffset();
}
/*視点移動限界値算出*/
void CSfmlSpritePlayer::AdjustOffset()
{
	int iSpriteWidth = static_cast<int>(m_BaseWindowSize.x * m_fSpriteScale);
	int iSpriteHeight = static_cast<int>(m_BaseWindowSize.y * m_fSpriteScale);

	int iClientWidth = sf::VideoMode::getDesktopMode().width;
	int iClientHeight = sf::VideoMode::getDesktopMode().height;

	int iXOffsetMax = iSpriteWidth > iClientWidth ? static_cast<int>(::floor((iSpriteWidth - iClientWidth) / m_fSpriteScale)) : 0;
	int iYOffsetMax = iSpriteHeight > iClientHeight ? static_cast<int>(::floor((iSpriteHeight - iClientHeight) / m_fSpriteScale)) : 0;

	if (m_iOffset.x < 0)m_iOffset.x = 0;
	if (m_iOffset.y < 0)m_iOffset.y = 0;

	if (m_iOffset.x > iXOffsetMax)m_iOffset.x = iXOffsetMax;
	if (m_iOffset.y > iYOffsetMax)m_iOffset.y = iYOffsetMax;

	UpdateSpriteOrigin();
}
/*原点設定*/
void CSfmlSpritePlayer::UpdateSpriteOrigin()
{
	for (size_t i = 0; i < m_sprites.size(); ++i)
	{
		m_sprites.at(i).setOrigin(sf::Vector2f(m_iOffset));
	}
}
/*拡縮初期化*/
void CSfmlSpritePlayer::ResetScale()
{
	m_fSpriteScale = 1.f;
	m_iOffset = sf::Vector2i{};

	ResizeWindow();

	if (m_window.get() != nullptr)
	{
		m_window->setPosition(sf::Vector2i(0, 0));
	}
}
/*文章表示位置調整*/
void CSfmlSpritePlayer::AdjustTextPosition()
{
	int iSpriteHeight = static_cast<int>(m_BaseWindowSize.y * m_fSpriteScale);
	int iClientHeight = sf::VideoMode::getDesktopMode().height;

	if (iSpriteHeight < iClientHeight)
	{
		m_imgText.setPosition(sf::Vector2f{ 0, m_BaseWindowSize.y - m_imgText.getGlobalBounds().height });
	}
	else
	{
		float fScale = static_cast<float>(iClientHeight) / iSpriteHeight;
		m_imgText.setPosition(sf::Vector2f{ 0, m_BaseWindowSize.y * fScale - m_imgText.getGlobalBounds().height });
	}
}
/*経過時間測定*/
void CSfmlSpritePlayer::CheckTimer()
{
	constexpr float fAutoPlayInterval = 2.f;
	float fSecond = m_clock.getElapsedTime().asSeconds();
	if (m_sound.getStatus() == sf::SoundSource::Stopped && fSecond > fAutoPlayInterval)
	{
		if (m_nTextIndex < m_textData.size() - 1)
		{
			ShiftMessageText(true);
		}
		else
		{
			m_clock.restart();
		}
	}
}
/*画像移行*/
void CSfmlSpritePlayer::ShiftSprite(bool bForward)
{
	if (bForward)
	{
		++m_nImageIndex;
		if (m_nImageIndex >= m_imageData.size())m_nImageIndex = 0;
	}
	else
	{
		--m_nImageIndex;
		if (m_nImageIndex >= m_imageData.size())m_nImageIndex = m_imageData.size() - 1;
	}
	UpdateSprite();
}
/*表示画像更新*/
void CSfmlSpritePlayer::UpdateSprite()
{
	int iLayer = m_imageData.at(m_nImageIndex).iLayer;
	bool bRet = m_textures.at(iLayer).loadFromFile(m_imageData.at(m_nImageIndex).strImagePath);
	if (bRet)
	{
		m_sprites.at(iLayer).setTexture(m_textures.at(iLayer));
	}

	std::wstring wstr = std::to_wstring(m_nImageIndex + 1) + L"/" + std::to_wstring(m_imageData.size());
	m_imgText.setString(wstr);
}
/*文章移行*/
void CSfmlSpritePlayer::ShiftMessageText(bool bForward)
{
	if (bForward)
	{
		++m_nTextIndex;
		if (m_nTextIndex >= m_textData.size())m_nTextIndex = 0;
	}
	else
	{
		--m_nTextIndex;
		if (m_nTextIndex >= m_textData.size())m_nTextIndex = m_textData.size() - 1;
	}
	UpdateMessageText();
}
/*表示文章更新*/
void CSfmlSpritePlayer::UpdateMessageText()
{
	const adv::TextDatum& textDatum = m_textData.at(m_nTextIndex);
	std::wstring wstr = textDatum.wstrText +  L"\r\n " + std::to_wstring(m_nTextIndex + 1) + L"/" + std::to_wstring(m_textData.size());
	m_msgText.setString(wstr);

	if (!textDatum.strVoicePath.empty())
	{
		m_soundBuffer.loadFromFile(textDatum.strVoicePath);
		m_sound.setBuffer(m_soundBuffer);
		m_sound.play();
	}
	m_clock.restart();
}

void CSfmlSpritePlayer::ChangeVolume(bool bLouder)
{
	constexpr float fVolumeMax = 100.f;
	constexpr float fVolumeMin = 0.f;
	constexpr float fVolumePortion = 10.f;

	if (bLouder) m_audioVolume += fVolumePortion;
	else m_audioVolume -= fVolumePortion;

	if (m_audioVolume < fVolumeMin)m_audioVolume = fVolumeMin;
	if (m_audioVolume > fVolumeMax)m_audioVolume = fVolumeMax;

	m_sound.setVolume(m_audioVolume);
}
