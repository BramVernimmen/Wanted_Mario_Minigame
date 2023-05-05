#include "pch.h"
#include "Head.h"
#include "Texture.h"
#include "utils.h"

Texture* Head::m_pHeadTexture{ nullptr };
int Head::m_InstanceCounter{ 0 };
Rectf Head::m_BoundingBox{};
const float Head::m_MaxTime{ 1.5f };

Head::Head(TypeOfHead type, const Rectf& boundingBox, bool isMoving)
	: m_Velocity{ Vector2f{0.f,0.f} }
	, m_CurrentHead{type}
	, m_CurrentTime{0.f}
{
	
	if (isMoving == true)
	{
		if (rand() % 2 == 0)
		{
			m_Velocity.x = rand() % 61 + 30.f;
		}
		else
		{
			m_Velocity.x = rand() % 61 - 90.f;

		}
		if (rand() % 2 == 0)
		{
			m_Velocity.y = rand() % 61 + 30.f;
		}
		else
		{
			m_Velocity.y = rand() % 61 - 90.f;
		}
	}

	if (m_InstanceCounter == 0)
	{
		m_pHeadTexture = new Texture("Resources/Heads.png");
		m_BoundingBox = boundingBox;
	}
	++m_InstanceCounter;

	float textureWidth{ m_pHeadTexture->GetWidth() /4};
	float textureHeight{ m_pHeadTexture->GetHeight()};

	m_Shape.left = rand() % int(boundingBox.width - textureWidth) + boundingBox.left ;
	m_Shape.bottom = rand() % int(boundingBox.height - textureHeight) + boundingBox.bottom;
	m_Shape.width = textureWidth;
	m_Shape.height = textureHeight;

	m_SourceRect.width = textureWidth;
	m_SourceRect.height = textureHeight;
	m_SourceRect.bottom = 0.f;

	switch (m_CurrentHead)
	{
	case Head::TypeOfHead::luigi:
		m_SourceRect.left = textureWidth * 0;
		break;
	case Head::TypeOfHead::mario:
		m_SourceRect.left = textureWidth * 1;

		break;
	case Head::TypeOfHead::yoshi:
		m_SourceRect.left = textureWidth * 2;

		break;
	case Head::TypeOfHead::wario:
		m_SourceRect.left = textureWidth * 3;

		break;
	}
}

Head::~Head()
{
	--m_InstanceCounter;
	if (m_InstanceCounter == 0)
	{
		delete m_pHeadTexture;
		m_pHeadTexture = nullptr;
	}
}

bool Head::Update(float elapsedSec, const Point2f& lookingPoint)
{
	m_Shape.left += m_Velocity.x * elapsedSec;
	m_Shape.bottom += m_Velocity.y * elapsedSec;
	if (m_Shape.left <= m_BoundingBox.left)
	{
		m_Shape.left = m_BoundingBox.left;
		m_Velocity.x *= -1;
	}
	else if (m_Shape.left + m_Shape.width >= m_BoundingBox.width)
	{
		m_Shape.left = m_BoundingBox.width - m_Shape.width;
		m_Velocity.x *= -1;
	}
	if (m_Shape.bottom <= m_BoundingBox.bottom)
	{
		m_Shape.bottom = m_BoundingBox.bottom;
		m_Velocity.y *= -1;
	}
	else if (m_Shape.bottom + m_Shape.height >= m_BoundingBox.height)
	{
		m_Shape.bottom = m_BoundingBox.height - m_Shape.height;
		m_Velocity.y *= -1;
	}

	if (utils::IsPointInRect(lookingPoint, m_Shape))
	{
		m_CurrentTime += elapsedSec;
		if (m_CurrentTime >= m_MaxTime)
		{
			m_CurrentTime = 0.f;
			return true;
		}
	}
	else
	{
		m_CurrentTime = 0.f;
	}

	

	return false;
}

void Head::Draw() const
{
	m_pHeadTexture->Draw(m_Shape, m_SourceRect);
}

bool Head::IsCorrectHead(TypeOfHead posterType) const
{
	
	return (m_CurrentHead == posterType);
}
