/* menu.h
 * Menu class, necessary stuff for the menu to work right
 * Licensed under the Open Software License version 2.0
 * coded by ReKleSS [rekless@fastmail.fm]
 */

struct menuEntry {
	char *filename;
	char *name;
	struct menuEntry *next;
}; //linked lists


class Menu {
	public:
		Menu();
		void init();
		virtual ~Menu();
		void runFrame();
	private:
		int selected;
		int elements;
		bool whacked[256];
		menuEntry head;
		void addEntry(const char *name);
};
		
void initMenu();
void mDrawScene();
