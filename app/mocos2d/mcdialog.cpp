
#include "mcdialog.h"
#include "mcscene.h"

namespace mocos2d {

bool MCDialog::init() {
	if (!Node::init()) return false;
	return true;
}

void MCDialog::init(int code, bool withBlack, bool fastCancel) {
	MCScene* scene = MCScene::getInstance();	
	scene->getRoot()->addChild(this, MAX_Z_ORDER);

	setTag(code);
	_mFastCancel = fastCancel;
	_mBlack = NULL;
	_mShowAnimFinished = false;

	if (withBlack) {
		_mBlack = ImageView::create("mocos/black.png");
		_mBlack->setScale(scene->getWidth() / 5.f, scene->getHeight() / 5.f);
		_mBlack->setPosition(Vec2(scene->getWidth() / 2, scene->getHeight() / 2));
		_mBlack->setOpacity(0);
		addChild(_mBlack);
	}
	onInitialized();
	
	if (!scene->getSceneInfo() || (scene->getSceneInfo()->getScreenOrientation() == screen_orientation_landscape)) {
		setCameraMask((ushort) CameraFlag::USER7);
	}
	setVisible(false);
}

bool MCDialog::isFastCancel() {
	return _mFastCancel;
}

void MCDialog::setAsDialogButton(Button* button, int code) {
	_mDialogButtonMap.erase(button);
	_mDialogButtonMap.insert(pair<Button*, int>(button, code));
	MCScene::getInstance()->addClick(button, getTag());
}

void MCDialog::setAsSceneButton(Button* button, DialogResult result) {
	_mDialogButtonMap.erase(button);
	_mSceneButtonMap.insert(pair<Button*, int>(button, result));
	MCScene::getInstance()->addClick(button, getTag());
}

void MCDialog::show() {	
	for (Node* child : MCScene::getInstance()->getRoot()->getChildren()) {
		if (child != this) _eventDispatcher->pauseEventListenersForTarget(child, true);
	}
	setVisible(true);
	if (_mBlack) _mBlack->runAction(FadeTo::create(0.2f, 120));

	runAction(Sequence::createWithTwoActions(
		DelayTime::create(0.25f),
		CallFunc::create(bind([this](){
			_mShowAnimFinished = true;
			}))
		));
	showAnim();
}

bool MCDialog::isShowAnimFinished() {
	return _mShowAnimFinished;
}

void MCDialog::hide() {
	MCScene::getInstance()->eraseClick(this);
	_eventDispatcher->resumeEventListenersForTarget(MCScene::getInstance()->getRoot(), true);
	float delay = hideAnim();
	if (_mBlack) _mBlack->runAction(FadeTo::create(MIN(delay, 0.2f), 0));
	runAction(Sequence::createWithTwoActions(DelayTime::create(delay), RemoveSelf::create()));
}

bool MCDialog::touch(Vec2 loc) {
	if (isTouched(loc)) return true;
	if (isFastCancel()) {
		MCScene::getInstance()->hideDialog();
		MCScene::getInstance()->onDialogResult(getTag(), dlg_result_cancel, -1);		
		return true;
	}
	return false;
}

int MCDialog::getDialogCode(Button* button) {
	map<Button*, int>::iterator iter = _mDialogButtonMap.find(button);
	if (iter == _mDialogButtonMap.end()) return -1;
	return iter->second;
}

int MCDialog::getSceneCode(Button* button) {
	map<Button*, int>::iterator iter = _mSceneButtonMap.find(button);
	if (iter == _mSceneButtonMap.end()) return -1;
	return iter->second;
}

}