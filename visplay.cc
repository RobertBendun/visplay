#include <cassert>
#include <iostream>
#include <random>
#include <tuple>
#include <chrono>

using namespace std::chrono_literals;
namespace chrono = std::chrono;

#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>

unsigned Window_Width  = 800;
unsigned Window_Height = 600;

namespace
{
	sf::Sound sound;
	float sound_duration;
}

namespace visualizers
{
	struct Bars
	{
		unsigned bars_count = 0;
		unsigned width = Window_Width, height = Window_Height;
		std::vector<sf::RectangleShape> bars{};

		void render(sf::RenderTarget &target)
		{
			if (bars.empty() || Window_Width != width || Window_Height != height) {
				std::tie(width, height) = std::tuple { Window_Width, Window_Height };
				bars.resize(bars_count);

				std::mt19937 rnd(std::random_device{}());
				std::uniform_real_distribution<float> dist;

				auto new_width = float(width) / float(bars_count);
				auto x = 0.f;
				for (auto &bar : bars) {
					bar.setPosition(x, Window_Height);
					bar.setSize({ new_width, -dist(rnd) * height });
					bar.setOutlineThickness(0);
					bar.setFillColor(sf::Color::Red);
					x += new_width;
				}
			}

			for (auto const& bar : bars) {
				target.draw(bar);
			}
		}
	};
}

void handle_events(sf::RenderWindow &window, sf::Event ev)
{
	switch (ev.type) {
	case sf::Event::Closed:
		window.close();
		break;

	case sf::Event::KeyPressed:
		switch (ev.key.code) {
		case sf::Keyboard::Escape:
			window.close();
			break;

		case sf::Keyboard::Up:
		case sf::Keyboard::K:
			sound.setVolume(sound.getVolume() + 0.05);
			break;

		case sf::Keyboard::Down:
		case sf::Keyboard::J:
			sound.setVolume(sound.getVolume() - 0.05);
			break;

		case sf::Keyboard::Left:
		case sf::Keyboard::H:
			sound.setPlayingOffset(sound.getPlayingOffset() + sf::seconds(-1));
			break;

		case sf::Keyboard::Right:
		case sf::Keyboard::L:
			sound.setPlayingOffset(sound.getPlayingOffset() + sf::seconds(1));
			break;

		case sf::Keyboard::Num0:
		case sf::Keyboard::Num1:
		case sf::Keyboard::Num2:
		case sf::Keyboard::Num3:
		case sf::Keyboard::Num4:
		case sf::Keyboard::Num5:
		case sf::Keyboard::Num6:
		case sf::Keyboard::Num7:
		case sf::Keyboard::Num8:
		case sf::Keyboard::Num9:
			{
				auto const offset = unsigned(ev.key.code) - unsigned(sf::Keyboard::Num0);
				sound.setPlayingOffset(sf::seconds(sound_duration * (offset / 10.f)));
			}
			break;

		case sf::Keyboard::Space:
			switch (sound.getStatus()) {
			case sf::SoundSource::Stopped:
			case sf::SoundSource::Paused:
				sound.play();
				break;
			case sf::SoundSource::Playing:
				sound.pause();
				break;
			}
			break;
		default:
			;
		}
		break;

	case sf::Event::Resized:
		std::tie(Window_Width, Window_Height) = std::tuple { window.getSize().x, window.getSize().y };
		window.setView(sf::View(sf::FloatRect { 0, 0, float(Window_Width), float(Window_Height) }));
		break;

	default:
		;
	}
}

int main(int argc, char **argv)
{
	if (argc != 2) {
		std::cerr << "usage: specvis <soundfile>\n";
		return 1;
	}

	sf::SoundBuffer buffer;

	if (!buffer.loadFromFile(argv[1])) {
		std::cerr << "specvis: error: could not load file: " << argv[1] << '\n';
		return 1;
	}

	sf::RenderWindow window(sf::VideoMode(Window_Width, Window_Height), "Specvis - audio spectrum visualiser");
	window.setFramerateLimit(60);

	// Set window position to the center of screen
	auto const desktop = sf::VideoMode::getDesktopMode();
	window.setPosition({
			int(desktop.width / 2 - Window_Width / 2),
			int(desktop.height / 2 - Window_Height / 2)
	});

	visualizers::Bars vis;
	vis.bars_count = 300;

	sound.setBuffer(buffer);
	sound.play();

	sound_duration = buffer.getDuration().asSeconds();
	sf::RectangleShape progress;
	progress.setOutlineColor(sf::Color::Black);
	progress.setFillColor(sf::Color::White);

	while (window.isOpen()) {
		for (sf::Event ev; window.pollEvent(ev); ) {
			handle_events(window, ev);
		}

		window.clear();
		vis.render(window);

		auto const sound_progress = sound.getPlayingOffset().asSeconds() / sound_duration;
		progress.setPosition(0, Window_Height);
		progress.setSize({ sound_progress * Window_Width, -0.01f * Window_Height });
		window.draw(progress);

		window.display();
	}
}
