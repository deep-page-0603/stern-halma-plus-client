
#pragma once

#include "mocos2d.h"

namespace mocos2d {

class MCUI {
public:
	static ImageView* imageView(string path, Vec2 pos, uchar cam, int zorder, Node* parent = NULL);
	static Button* button(string normalPath, string pressedPath, string disabledPath, Vec2 pos, uchar cam, int code, int tag, Node* parent = NULL);
	static Text* text(string title, string font, float size, Vec2 pos, Color3B color = Color3B::WHITE, Node* parent = NULL);

public:
	static ushort cameraFlag(uchar cam);
};

}