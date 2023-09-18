
#pragma once

#include "mocos2d.h"

namespace mocos2d {

enum DialogResult {
	dlg_result_ok, dlg_result_cancel, dlg_result_yes, dlg_result_no, dlg_resultc
};

class MCDialog : public Node {
public:	
	virtual bool init();
	virtual ~MCDialog() {};
	
	virtual void init(int code, bool withBlack, bool fastCancel);
	virtual void show();
	virtual void hide();
	virtual bool touch(Vec2 loc);

	bool isFastCancel();
	int getDialogCode(Button* button);
	int getSceneCode(Button* button);
	bool isShowAnimFinished();

	virtual void onInitialized() = 0;
	virtual void showAnim() {};
	virtual float hideAnim() {return 0;};
	virtual bool isTouched(Vec2 loc) = 0;
	virtual void onClickListener(int code, int tag) {};
	
protected:
	void setAsDialogButton(Button* button, int code);
	void setAsSceneButton(Button* button, DialogResult result);

protected:
	ImageView* _mBlack;
	bool _mFastCancel;
	map<Button*, int> _mSceneButtonMap;
	map<Button*, int> _mDialogButtonMap;
	bool _mShowAnimFinished;
};

}