#include "pch.h"
#include "Game.h"
#include "utils.h"
//#include "Head.h"
#include "Texture.h"

Game::Game( const Window& window ) 
	:m_Window{ window }
{
	Initialize( );
}

Game::~Game( )
{
	Cleanup( );
}

void Game::Initialize( )
{
	m_HasGameStarted = false;
	m_pStartText = new Texture("Start", "Resources/DIN-Light.otf", 100, Color4f{ 0.f,0.f,0.f,1.f });

	m_StartButton.width = 300.f;
	m_StartButton.height = 150.f;
	m_StartButton.left = (m_Window.width - m_StartButton.width) / 2;
	m_StartButton.bottom = (m_Window.height - m_StartButton.height) / 2;

	m_pWanted = new Texture("Resources/Wanted_Posters.png");
	m_WantedWidth = m_pWanted->GetWidth() / 4;
	m_WantedHeight = m_pWanted->GetHeight();

	m_PosterHead = Head::TypeOfHead::wario;

	m_SourceWanted.bottom = 0.f;
	m_SourceWanted.width = m_WantedWidth;
	m_SourceWanted.height = m_WantedHeight;
	m_SourceWanted.left = int(m_PosterHead) * m_WantedWidth; // temp

	m_WantedPos = Point2f{ m_Window.width - m_WantedWidth,(m_Window.height - m_WantedHeight) /2.f};
	

	m_BoundingBox.bottom = 0.f;
	m_BoundingBox.left = 0.f;
	m_BoundingBox.width = m_Window.width - m_WantedWidth;
	m_BoundingBox.height = m_Window.height;
	//m_pHeads.push_back(new Head(Head::TypeOfHead::luigi, m_BoundingBox));

	m_MaxGameTimer = 90.f;
	m_MaxStartTimer = 2.f;
	m_Timer = 0.f;
	m_CurrentLevel = 1;
}

void Game::Cleanup( )
{
	ClearHeads();
	delete m_pWanted;
	m_pWanted = nullptr;
	delete m_pStartText;
	m_pStartText = nullptr;
}

void Game::Update( float elapsedSec )
{
	// Check keyboard state
	//const Uint8 *pStates = SDL_GetKeyboardState( nullptr );
	//if ( pStates[SDL_SCANCODE_RIGHT] )
	//{
	//	std::cout << "Right arrow key is down\n";
	//}
	//if ( pStates[SDL_SCANCODE_LEFT] && pStates[SDL_SCANCODE_UP])
	//{
	//	std::cout << "Left and up arrow keys are down\n";
	//}

	// get tobii pos in range [0,1] where 0 is top or left and 1 is right or bottom
	m_TobiiPos = m_Tobii.GetPoint();
	if (m_TobiiPos.x != -1 && m_TobiiPos.y != -1)
	{
		// convert to screen pixel space
		SDL_Rect displayBounds;
		SDL_GetDisplayBounds(0, &displayBounds);
		m_TobiiPos.x *= displayBounds.w;
		m_TobiiPos.y = (1- m_TobiiPos.y) * displayBounds.h;
		// convert from screen space to window space
		int wPosX, wPosY;
		SDL_GetWindowPosition(m_Window.pSDLWindow, &wPosX, &wPosY);
		m_TobiiPos.x -= wPosX;
		m_TobiiPos.y -= wPosY;
		//std::cout << Vector2f(m_TobiiPos) << '\n';		
	}
	else
	{
		// screen is not being watched, set to pause ...
	}

	if (m_HasGameStarted == true)
	{
		m_Timer -= elapsedSec;
		for (Head* ptr : m_pHeads)
		{
			if (ptr->Update(elapsedSec, m_TobiiPos) == true)
			{
				if (ptr->IsCorrectHead(m_PosterHead) == true)
				{
					//std::cout << "Good Job!" << std::endl;
					m_LevelWon = true;
				}
				else
				{
					//std::cout << "Very Bad Job!" << std::endl;
					m_Timer -= 5.f;
				}
			}
			else
			{
				//std::cout << "Bad Job!" << std::endl;
			}
		}

		if (m_LevelWon == true)
		{
			m_LevelWon = false;
			m_Timer += 5.f;
			++m_CurrentLevel;
			InitializeLevel();
		}

		if (m_Timer < 0.f)
		{
			m_HasGameStarted = false;
			ClearHeads();
			m_CurrentLevel = 1;
		}
	}
	else
	{
		if (utils::IsPointInRect(m_TobiiPos, m_StartButton) == true)
		{
			m_Timer += elapsedSec;
			if (m_Timer >= m_MaxStartTimer)
			{
				m_HasGameStarted = true;
				m_Timer = m_MaxGameTimer;
				InitializeLevel();
			}
		}
		else
		{
			m_Timer = 0.f;
		}
		
	}

}

void Game::Draw( ) const
{
	ClearBackground( );
	if (m_HasGameStarted == true)
	{
		for (Head* ptr : m_pHeads)
		{
			ptr->Draw();
		}
		m_pWanted->Draw(m_WantedPos, m_SourceWanted);

		std::string levelText{ "Level: " + std::to_string(m_CurrentLevel) };
		Texture levelTexture{ Texture(levelText, "Resources/DIN-Light.otf", 25, Color4f{1.f,1.f,1.f,1.f}) };
		levelTexture.Draw(Point2f{m_WantedPos.x, 5.f});

		std::string timerText{ "Time: " + std::to_string(int(m_Timer) / 60) + ":" + std::to_string(int(m_Timer) % 60)};
		Texture timerTexture{ Texture(timerText, "Resources/DIN-Light.otf", 25, Color4f{1.f,1.f,1.f,1.f}) };
		timerTexture.Draw(Point2f{ m_WantedPos.x, m_WantedPos.y + m_WantedHeight + 5.f });
	}
	else
	{
		utils::SetColor(Color4f{ 1.f,1.f,0.f,1.f });
		utils::FillRect(m_StartButton);
		m_pStartText->Draw(m_StartButton);
	}
	utils::SetColor(Color4f{ 1.f,1.f,1.f,1.f });
	utils::DrawPoint(m_TobiiPos, 5);
}

void Game::ProcessKeyDownEvent( const SDL_KeyboardEvent & e )
{
	//std::cout << "KEYDOWN event: " << e.keysym.sym << std::endl;
}

void Game::ProcessKeyUpEvent( const SDL_KeyboardEvent& e )
{
	//std::cout << "KEYUP event: " << e.keysym.sym << std::endl;
	//switch ( e.keysym.sym )
	//{
	//case SDLK_LEFT:
	//	//std::cout << "Left arrow key released\n";
	//	break;
	//case SDLK_RIGHT:
	//	//std::cout << "`Right arrow key released\n";
	//	break;
	//case SDLK_1:
	//case SDLK_KP_1:
	//	//std::cout << "Key 1 released\n";
	//	break;
	//}
}

void Game::ProcessMouseMotionEvent( const SDL_MouseMotionEvent& e )
{
	//std::cout << "MOUSEMOTION event: " << e.x << ", " << e.y << std::endl;
}

void Game::ProcessMouseDownEvent( const SDL_MouseButtonEvent& e )
{
	//std::cout << "MOUSEBUTTONDOWN event: ";
	//switch ( e.button )
	//{
	//case SDL_BUTTON_LEFT:
	//	std::cout << " left button " << std::endl;
	//	break;
	//case SDL_BUTTON_RIGHT:
	//	std::cout << " right button " << std::endl;
	//	break;
	//case SDL_BUTTON_MIDDLE:
	//	std::cout << " middle button " << std::endl;
	//	break;
	//}
}

void Game::ProcessMouseUpEvent( const SDL_MouseButtonEvent& e )
{
	//std::cout << "MOUSEBUTTONUP event: ";
	//switch ( e.button )
	//{
	//case SDL_BUTTON_LEFT:
	//	std::cout << " left button " << std::endl;
	//	break;
	//case SDL_BUTTON_RIGHT:
	//	std::cout << " right button " << std::endl;
	//	break;
	//case SDL_BUTTON_MIDDLE:
	//	std::cout << " middle button " << std::endl;
	//	break;
	//}
}

void Game::ClearBackground( ) const
{
	glClearColor( 0.0f, 0.0f, 0.0f, 1.0f );
	glClear( GL_COLOR_BUFFER_BIT );
}

void Game::InitializeLevel()
{
	if (m_CurrentLevel != 1)
	{
		ClearHeads();
	}

	bool isMoving{ m_CurrentLevel >= 4 };

	Head::TypeOfHead newPosterHead{ Head::TypeOfHead(rand() % 4) };
	m_PosterHead = newPosterHead;
	m_SourceWanted.left = m_WantedWidth * int(m_PosterHead);

	int wantedHeadNr{ rand() % (m_CurrentLevel * 4) };


	for (int i{ 0 }; i < m_CurrentLevel * 4; ++i)
	{
		if (i == wantedHeadNr)
		{
			m_pHeads.push_back(new Head(newPosterHead, m_BoundingBox, isMoving));
		}
		else
		{
			Head::TypeOfHead newHeadType{ Head::TypeOfHead(rand() % 4) };
			while (newHeadType == newPosterHead)
			{
				newHeadType = Head::TypeOfHead(rand() % 4);
			}

			m_pHeads.push_back(new Head(newHeadType, m_BoundingBox, isMoving));
		}
	}

}

void Game::ClearHeads()
{
	for (Head* ptr : m_pHeads)
	{
		delete ptr;
		ptr = nullptr;
	}
	m_pHeads.clear();
}
