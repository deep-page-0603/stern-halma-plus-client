
#include "mcui.h"
#include "mcscene.h"

namespace mocos2d {

ImageView* MCUI::imageView(string path, Vec2 pos, uchar cam, int zorder, Node* parent) {
	auto img = ImageView::create(path);
	img->setPosition(pos);
	img->setCameraMask(cameraFlag(cam));
	if (!parent) parent = MCScene::getInstance()->getRoot();
	parent->addChild(img, zorder);
	return img;
}

Button* MCUI::button(string normalPath, string pressedPath, string disabledPath, Vec2 pos, uchar cam, int code, int tag, Node* parent) {
	auto but = Button::create(normalPath, pressedPath, disabledPath);
	but->setPosition(pos);
	but->setTag(tag);
	but->setCameraMask(cameraFlag(cam));
	if (!parent) parent = MCScene::getInstance()->getRoot();
	parent->addChild(but);
	if (code >= 0) MCScene::getInstance()->addClick(but, code);
	return but;
}

Text* MCUI::text(string title, string font, float size, Vec2 pos, Color3B color, Node* parent) {
	auto txt = Text::create(title, font, size);
	txt->setPosition(pos);
	txt->setTextColor(Color4B(color.r, color.g, color.b, 255));
	if (!parent) parent = MCScene::getInstance()->getRoot();
	parent->addChild(txt);
	return txt;
}

ushort MCUI::cameraFlag(uchar cam) {
	return 1 << cam;
}

}