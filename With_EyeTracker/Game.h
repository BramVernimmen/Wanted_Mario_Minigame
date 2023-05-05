#pragma once
#include "Tobii.h"
#include <vector>
class Texture;
#include "Head.h"
//class Head;
class Game
{
public:
	explicit Game( const Window& window );
	Game(const Game& other) = delete;
	Game& operator=(const Game& other) = delete;
	Game( Game&& other) = delete;
	Game& operator=(Game&& other) = delete;
	~Game();

	void Update( float elapsedSec );
	void Draw( ) const;

	// Event handling
	void ProcessKeyDownEvent( const SDL_KeyboardEvent& e );
	void ProcessKeyUpEvent( const SDL_KeyboardEvent& e );
	void ProcessMouseMotionEvent( const SDL_MouseMotionEvent& e );
	void ProcessMouseDownEvent( const SDL_MouseButtonEvent& e );
	void ProcessMouseUpEvent( const SDL_MouseButtonEvent& e );

private:
	// DATA MEMBERS
	const Window m_Window;
	Tobii m_Tobii;
	Point2f m_TobiiPos;

	std::vector<Head*> m_pHeads;

	Texture* m_pWanted;
	float m_WantedWidth;
	float m_WantedHeight;
	Rectf m_BoundingBox;
	
	Point2f m_WantedPos;
	Rectf m_SourceWanted;

	Head::TypeOfHead m_PosterHead;

	bool m_HasGameStarted;
	Rectf m_StartButton;
	Texture* m_pStartText;
	float m_MaxStartTimer;

	float m_MaxGameTimer;
	float m_Timer;
	int m_CurrentLevel;
	bool m_LevelWon;

	// FUNCTIONS
	void Initialize( );
	void Cleanup( );
	void ClearBackground( ) const;

	void InitializeLevel();
	void ClearHeads();
};