#include <vector>
#include <queue>
#include <map>
#include <iterator>
#include <fstream>
#include <math.h>

#include <opencv\cv.h>
#include <opencv\cxcore.h>
#include <opencv\highgui.h>

using namespace std;

/*SIFT特征值, 固定为128维double向量*/
struct SIFT_T{
	float sift_vec[128];
};

/*树的节点*/
struct NODE{
	bool is_leaf;
	int depth;						//在树中的深度
	float weight;					//投票权重
	SIFT_T sift;				//该节点特征值
	std::map<int,int> img_idFreq;	//特征值下的图片标志列表
	std::vector<NODE*> children;	//指向子节点指针
};

/*查询结果的表项,含有有限级的比较函数*/
struct QUERYRESULT{
	int img_id;
	float tot_weight;
	friend bool operator<(const QUERYRESULT& a,const QUERYRESULT& b)
    {
        return a.tot_weight<b.tot_weight;		//大值优先
    }
};

/*sift向量的比较函数*/
struct cmp_sift{  
    bool operator()( const SIFT_T& a,const SIFT_T& b ) const{  
        for(int m = 0; m < 128;m++){
			if(a.sift_vec[m] != b.sift_vec[m])
				return false;
		}
		return true;
    }  
};

/*内联函数计算两个SIFT向量的Manhattan距离*/
//inline double siftDist(const SIFT_T& a, const SIFT_T& b){
//	double dist = 0.0;
//	for(int m = 0; m<128;m++){
//		dist += abs(a.sift_vec[m]-b.sift_vec[m]);
//	}
//	return dist;
//}
inline double siftDist(const SIFT_T& a, const SIFT_T& b){
	double dist = 0.0;
	for(int m = 0; m<128;m++){
		dist += (a.sift_vec[m]-b.sift_vec[m])*(a.sift_vec[m]-b.sift_vec[m]);
	}
	return sqrt(dist);
}

/*将b的SIFT向量值赋给a*/
inline void siftCpy(SIFT_T& a ,const SIFT_T& b){
	memcpy(a.sift_vec,b.sift_vec,sizeof(b.sift_vec));
}

class VisualWordTree
{
public:
	void constructTree(CvMat* sift_vecs);
	void mapTreeWithImg(int img_id, vector<SIFT_T>& img_sift);
	void calWeight(int img_num);
	void queryTree(vector<SIFT_T>& sift, priority_queue<QUERYRESULT>& res);
	void destroyTree();
	void writeTree2File(char *filename);
	void readTreeFromFile(char *filename, int img_num);
private:
	int total_feature_num;
	int total_photo_num;
	NODE *pRoot;
	ifstream in;
	ofstream out;
	int tot_written_num;

	void extendNode(CvMat* feat, NODE* current_node);
	void fillId(int img_id, SIFT_T &sift_ft, NODE* current_node);
	void fillWeight(NODE* current_node);
	void destroyNode(NODE* current_node);
	void voteImg(SIFT_T& sift, NODE* current_node, map<int,float>& score);
	void writeNode2File(NODE* current_node);
	void readNodeFromFile(NODE* current_node);
};