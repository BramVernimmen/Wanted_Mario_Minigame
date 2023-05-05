#pragma once

#include "Vector2f.h"

class Texture;
class Head
{
public:
	enum class TypeOfHead
	{
		luigi = 0,
		mario = 1,
		yoshi = 2,
		wario = 3
	};
	Head(TypeOfHead type, const Rectf& boundingBox, bool isMoving);
	~Head();

	bool Update(float elapsedSec, const Point2f& lookingPoint);
	void Draw() const;
	bool IsCorrectHead(TypeOfHead posterType) const;

private:
	static Texture* m_pHeadTexture;
	static int m_InstanceCounter;
	static Rectf m_BoundingBox;
	static const float m_MaxTime;

	TypeOfHead m_CurrentHead;

	Rectf m_Shape;
	Rectf m_SourceRect;
	Vector2f m_Velocity;

	float m_CurrentTime;
};

