#include "RRUI.h"
#include "TheApp.h"
#include "UIhint.h"
#include "ButtonTableDrag.h"
#include "ScreenFrame.h"
#include "ButtonMap.h"
#include "ButtonEdit.h"
#include "ButtonClose.h"

extern TheApp theApp;


void RRUI::initPlayScreen() {
	UISubj::clear();
	UIhint::init();
	ScreenFrame* pSF = new ScreenFrame("ScreenFrame", NULL, NULL);
	ButtonMap* pBM = new ButtonMap("ButtonMap", NULL, NULL);
	float r = UISubj::buttonsH;
	float x = r + UISubj::buttonsMargin * 3;
	//init table drag group
	ButtonTableDrag::drag_table_spin = ButtonTableDrag::addTableButton("drag_table_spin", x, UISubj::buttonsMargin, r, r, "bottom right", "/dt/ui/buttons/drag_table_spin.bmp", "/dt/ui/buttons/hint_turn_table.bmp");
	ButtonTableDrag::drag_table_xz = ButtonTableDrag::addTableButton("drag_table_xz", x, r + UISubj::buttonsMargin * 3, r, r, "bottom right", "/dt/ui/buttons/drag_table_xz.bmp", "/dt/ui/buttons/hint_drag_table.bmp");
	//drag_table_xz = addTableButton("drag_table_xy", x, y += d, w, h, "", "/dt/ui/buttons/drag_table_xy.bmp", "/dt/ui/buttons/hint_lift_table.bmp");
	//drag_table_sun = addTableButton("drag_table_sun", x, y += d, w, h, "", "/dt/ui/buttons/drag_table_sun.bmp", "/dt/ui/buttons/hint_shift_light.bmp");

	ButtonEdit* pBE = new ButtonEdit();
	ButtonClose* pBC = new ButtonClose();
}
