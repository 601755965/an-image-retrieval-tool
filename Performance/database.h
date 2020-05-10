#include "visualwordtree.h"
#include <dirent.h>

#include "opensift\sift.h"
#include "opensift\imgfeatures.h"
#include "opensift\utils.h"

using namespace std;

struct img_mapping {
	int img_id;
	char *img_file;
	char *feat_file;
};

class ImgDatabase
{
public:
	ImgDatabase();
	void InitDatabase(char *feat_dir);
	void QueryImg(IplImage *img, priority_queue<QUERYRESULT> &res);
	void SaveDatabase(char *dest_path);
	void LoadDatabase(char *src_path);
	//void ExtractFeatures(char *img_dir);
	char *imgDir;
	char *featDir;
	vector<img_mapping> mappingList;
	int imgNum;
private:
	VisualWordTree vwTree;
	int vecNum;
	void SaveSiftInfo(char *feat_dir);
	void LoadSiftInfo(char *feat_dir);
};