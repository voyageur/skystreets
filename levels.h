struct lvlpoint {

	float blx;
	float bly;
	float blz;

	float trx;
	float trY;
	float trz;

	int material;
	struct lvlpoint *next;
	struct lvlpoint *prev;
};

struct block {
	float x1, y1, z1;
	float x2, y2, z2;
	float x3, y3, z3;
	float x4, y4, z4;
};

struct texcoord {
	float x1, y1;
	float x2, y2;
	float x3, y3;
	float x4, y4;
};	

class Level {
      public:
	lvlpoint * head;
	lvlpoint *tail;		//linked lists! yay!
	float *vertices;
	float *texcoords;
	int blocks;
	int curBlock;
	int maxFuel;
	int maxOxygen;
	int gravity;
	char next[30];
	char fname[64];
	FILE *data;
	 Level();
	~Level();
	bool initialize(const char *filename);
	bool loadNext();
	void Render( /*float front, float back */ );
	void Add(lvlpoint * newPoint);
	void Delete(lvlpoint * point);
	void Sort();
	void Save();
	void prepare(); //set up arrays for rendering
	void clear();		//empty the lists
      private:
	 lvlpoint * addPoint();

	GLuint lvlList;
};
