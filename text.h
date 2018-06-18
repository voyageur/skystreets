
class Text {
      public:
	Text();
	~Text();
	int drawf(const char *fmt, ...);
	void drawNo(int number);
	void init();
      private:
	int fontLists[65];
	int table[128];		//lookup table for the characters
	void buildLists();

};
