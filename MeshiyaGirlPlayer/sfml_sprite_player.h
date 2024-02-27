#ifndef SFML_SPRITE_PLAYER_H_
#define SFML_SPRITE_PLAYER_H_

#include <memory>

#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>

#include "adv.h"

class CSfmlSpritePlayer
{
public:
	CSfmlSpritePlayer();
	~CSfmlSpritePlayer();
	bool SetFont(const std::string& strFilePath, bool bBold, bool bItalic);
	void SetResources(const std::vector<adv::TextDatum>& textData, const std::vector<adv::ImageDatum>& imageData);
	int Display(const std::wstring &wstrWindowName);
private:
	enum Size { kBaseWidth = 1280, kBaseHeight = 700 };

	std::unique_ptr<sf::RenderWindow> m_window;
	sf::Vector2u m_BaseWindowSize = sf::Vector2u{ Size::kBaseWidth, Size::kBaseHeight };

	sf::Font m_font;
	sf::Text m_msgText;

	std::vector<sf::Texture> m_textures;
	std::vector<sf::Sprite> m_sprites;

	sf::SoundBuffer m_soundBuffer;
	sf::Sound m_sound;
	sf::Clock m_clock;
	float m_audioVolume = 50.f;

	size_t m_nTextIndex = 0;
	std::vector<adv::TextDatum> m_textData;
	size_t m_nImageIndex = 0;
	std::vector<adv::ImageDatum> m_imageData;

	bool m_bTextHidden = false;
	float m_fSpriteScale = 1.f;
	sf::Vector2i m_iOffset = sf::Vector2i{};

	void SetupSprite();
	void SwitchSmoothMode();

	void RescaleSprite(bool bUpscale);
	void ResizeWindow();
	void AddOffset(sf::Vector2i iOffset);
	void AdjustOffset();
	void UpdateSpriteOrigin();
	void ResetScale();

	void CheckTimer();
	void ShiftSprite(bool bForward);
	void UpdateSprite();
	void ShiftMessageText(bool bForward);
	void UpdateMessageText();

	void ChangeVolume(bool bLouder);
};

#endif // SFML_SPRITE_PLAYER_H_
