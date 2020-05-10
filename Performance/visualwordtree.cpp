#include "visualwordtree.h"

//#include <Windows.h>
#include <tchar.h>
//#include "resource.h"

#define K					8
#define MAXD				5
#define MIN_CLUSTER_SIZE	100
//#define PHOTONUM			100
#define BUFFERSIZE 25000

void VisualWordTree::extendNode(CvMat* sift_feats, NODE* current_node)
{
	int feat_num = sift_feats->rows;
	CvMat *clusters = cvCreateMat(feat_num, 1, CV_32SC1);
	CvMat *centers = cvCreateMat(K, 128, CV_32FC1);
	cvSetZero(clusters);
	cvSetZero(centers);

	cvKMeans2(sift_feats, K, clusters, cvTermCriteria(CV_TERMCRIT_EPS,10,1.0), 3, (CvRNG *)0, cv::KMEANS_USE_INITIAL_LABELS, centers);
	vector<int> greg[K];
	for (int j = 0; j < feat_num; j++) {
		greg[clusters->data.i[j]].push_back(j);
	}
	cvReleaseMat(&clusters);

	//HWND hWnd = AfxGetMainWnd()->m_hWnd;
	char stat_info[2000];
	for (int i = 0; i < K; i++) {
		NODE *child = new NODE;
		child->depth = current_node->depth+1;
		current_node->children.push_back(child);

		sprintf_s(stat_info, "Constructing child node %d at depth %d, father depth: %d\n",
			i, child->depth, current_node->depth);
		//SetDlgItemText(hWnd, IDC_STAT, _T(stat_info));

		for (int j = 0; j < 128; j++) {
			child->sift.sift_vec[j] = centers->data.fl[i*128+j];
		}
		if ((current_node->depth == MAXD) || (greg[i].size() < MIN_CLUSTER_SIZE)) {
			child->is_leaf = true;
		}
		else {
			child->is_leaf = false;
			CvMat *child_sift = cvCreateMat(greg[i].size(), 128, CV_32FC1);
			for (int j = 0; j < greg[i].size(); j++) {
				for (int k = 0; k < 128; k++) {
					child_sift->data.fl[j*128+k] = sift_feats->data.fl[greg[i][j]*128+k];
				}
			}
			extendNode(child_sift, child);
			//cvReleaseMat(&child_sift);
		}
		greg[i].clear();
	}

	cvReleaseMat(&centers);
	cvReleaseMat(&sift_feats);
	return;
}

/*destroyNode ��������ĳһ����µ����нڵ�*/
void VisualWordTree::destroyNode(NODE* pCurrent){
	if(pCurrent->is_leaf == true){
		delete pCurrent;
	}
	else{
		for(unsigned int i =0; i<pCurrent->children.size();i++){
			destroyNode((pCurrent->children[i]));
		}
		delete pCurrent;
	}
	return;
}

/*fillId��������ÿ���ڵ�ĵľ����Ӧ��ͼƬid*/
void VisualWordTree::fillId(int id, SIFT_T& sift_ft, NODE* pCurrent){ // ������ɺ���������Ȼ���ÿ���ڵ��ӦͼƬID���
	map<int,int>::iterator iter;
	if(pCurrent->is_leaf == true){
		iter = pCurrent->img_idFreq.find(id);
		if(iter == pCurrent->img_idFreq.end()){				//��û�н���ӳ�����
			pCurrent->img_idFreq.insert(make_pair(id,1));	
		}
		else{												//ĳһͼƬ���ִ�������һ��
			pCurrent->img_idFreq[id] += 1; 
		}	
	}
	else{
		NODE* pNext = pCurrent->children[0];
		double min_dist = siftDist(sift_ft,pCurrent->children[0]->sift);
		double tmp_dist;
		for(unsigned int i = 0;i < pCurrent->children.size();i++){
			if((tmp_dist = siftDist(sift_ft,pCurrent->children[i]->sift))<min_dist){
				min_dist = tmp_dist;
				pNext = pCurrent->children[i];
			}
		}
		fillId(id,sift_ft,pNext);
	}
	return;
}

void VisualWordTree::fillWeight(NODE* pCurrent){
	if(pCurrent->is_leaf == true){
		//printf("filling weight in child node\n");
		//printf("tot img num: %d\n",tot_img);
		//printf("img included num: %d\n",pCurrent->img_idFreq.size());
		if(pCurrent->img_idFreq.size()!=0){
			pCurrent->weight = log(total_photo_num/(pCurrent->img_idFreq.size()));
		}
		else{
			pCurrent->weight = 0.0;
		}
		//printf("weight:%f",pCurrent->weight);
	}
	else{
		//printf("not child node\n");
		for(unsigned int i = 0;i<pCurrent->children.size();i++){
			fillWeight(pCurrent->children[i]);
		}
	}
	return;
}

/*voteImg����ĳһSIFT����ֵ��Ӧ��Ҷ�ڵ��е�ͼƬ����ͶƱ, ͶƱֵΪ��Ҷ�ڵ��Ȩ��*/
void VisualWordTree::voteImg(SIFT_T& sift_ft, NODE* pCurrent, std::map<int,float>& accumulator){
	if(pCurrent->is_leaf == true){
		//printf("find the leaf node to cal the votes\n");
		std::map<int,int>::iterator iterii =pCurrent->img_idFreq.begin();
		std::map<int,float>::iterator iterid;
		while(iterii!=pCurrent->img_idFreq.end()){
			iterid = accumulator.find(iterii->first);
			if(iterid == accumulator.end()){
				//printf("insert new pair\n");
				accumulator.insert(make_pair(iterii->first,iterii->second*pCurrent->weight));
				//printf("weight: %f\n",accumulator[iterii->first]);
			}
			else{
				//printf("accumulate votes\n");
				accumulator[iterii->first] += (iterii->second*pCurrent->weight);
				//printf("weight: %f\n",accumulator[iterii->first]);
			}
			iterii++;
		}
	}
	else{		//�ҵ�Manhattan ���������һ����֧
		NODE* pNext  = (pCurrent->children[0]);
		double min_dist = siftDist(pNext->sift,sift_ft);
		double tmp_dist;
		for(unsigned int i = 1;i < pCurrent->children.size();i++){
			if((tmp_dist = siftDist(pCurrent->children[i]->sift,sift_ft)) < min_dist){
				pNext = pCurrent->children[i];
				min_dist = tmp_dist; 
			}
		}
		voteImg(sift_ft, pNext,accumulator);
	}
	return;
}

/*������ĳһ���ڵ�д���ļ���*/
void VisualWordTree::writeNode2File(NODE* pCurrent){
	char buffer[BUFFERSIZE];
	memset(buffer,0,sizeof(buffer));
	memcpy(&buffer[1],&pCurrent->depth,sizeof(int));
	memcpy(&buffer[5],&pCurrent->weight,sizeof(float));
	map<int,int>::iterator iter;
	for(int m =0;m<128;m++){
		memcpy(&buffer[9+m*4],&pCurrent->sift.sift_vec[m],sizeof(float));
	}
	if(pCurrent->is_leaf==true){
		buffer[0] = 1;
		tot_written_num++;
		printf("ready to write:%d\n",tot_written_num);
		printf("a leaf\n");
		for(int i = 0; i < total_photo_num;i++){
			iter = pCurrent->img_idFreq.find((i+1));	//ͼƬ��1��ʼ���
			if(iter!=pCurrent->img_idFreq.end()){
				memcpy(&buffer[521+i*4],&iter->second,sizeof(int));
			}
		}
		printf("begin to write into file\n");
		out.write(buffer,1+4+4+4*128+4*total_photo_num);
		printf("written this note successfuly!\n");
		return;
	}
	else{
		buffer[0] = 0;
		out.write(buffer,1+4+4+4*128+4*total_photo_num);
		for(unsigned int i =0;i<pCurrent->children.size();i++){
			tot_written_num++;
			printf("ready to write:%d\n",tot_written_num);
			printf("a father\n");
			writeNode2File(pCurrent->children[i]);
		}
		printf("written this note successfuly!\n");
	}
	return;
}

/*���ļ��е�һ���ڵ���Ϣ����,���ؽ���*/
void VisualWordTree::readNodeFromFile(NODE* pCurrent){
	char buffer[BUFFERSIZE];
	if(in.eof()){
		printf("Info in file is not enough to reconstruct the tree!\n");
		return;
	}
	in.read(buffer,1+4+4+4*128+4*total_photo_num);	//���ļ���һ���ڵ�����ݶ���
	memcpy(&pCurrent->depth,&buffer[1],sizeof(int));
	memcpy(&pCurrent->weight,&buffer[5],sizeof(float));
	for(int m = 0; m<128;m++){
		memcpy(&pCurrent->sift.sift_vec[m],&buffer[9+4*m],sizeof(float));
	}
	if(buffer[0] == 0){
		pCurrent->is_leaf = false;
		for(int i = 0;i < K;i++){
			NODE* pNext = new NODE;
			readNodeFromFile(pNext);
			pCurrent->children.push_back(pNext);
		}
	}
	else{
		pCurrent->is_leaf = true;
		pCurrent->img_idFreq.clear();
		for(int i =0;i<total_photo_num;i++){
			int tmp_freq;
			memcpy(&tmp_freq,&buffer[521+i*4],sizeof(int));
			if(tmp_freq != 0){
				pCurrent->img_idFreq.insert(make_pair((i+1),tmp_freq));
			}
		}
	}
	//printf("successfully read a node from file\n");
	return;
}

/*����Ľӿ�*/
/*constructTree ͨ�����ݿ�ͼƬ��SIFT����ֵ,�Լ�sift-ͼƬid�ĵ����Լ�Kmeans�����ཨ��һ����*/
void VisualWordTree::constructTree(CvMat* psift_space){
	pRoot = new NODE;
	pRoot->depth = 1;
	pRoot->is_leaf = false;
	total_feature_num = psift_space->rows;
	printf("constructed root node\n");
	extendNode(psift_space,pRoot);	//������kmeans�ķ�������
	return;
}

/*��ͼƬid������ֵĴ�����¼��Ҷ�ڵ���*/
void VisualWordTree::mapTreeWithImg(int img_id,std::vector<SIFT_T>& sift_list){
	for(unsigned int i = 0;i < sift_list.size();i++){
		fillId(img_id, sift_list[i],pRoot);
	}
}

/*��������Ҷ�ڵ��Ȩ��*/
void VisualWordTree::calWeight(int tot_img){
	total_photo_num = tot_img;
	fillWeight(pRoot);
	//system("PAUSE");
}

/*destroyTree ���ս��ɵ����Ŀռ�*/
void VisualWordTree::destroyTree(){
	destroyNode(pRoot);	
}

/*����һ��ͼ������SIFT��������, ͨ���Խ��ɵ������Ҳ����սڵ�Ȩ�ؽ���ͶƱ�ķ�ʽ�ҵ�Ȩ������ͼ���id*/
void VisualWordTree::queryTree(vector<SIFT_T>& queryList,priority_queue<QUERYRESULT>& resultList){
	map<int,float> accumulator;
	SIFT_T sift_tmp;
	for(unsigned int i = 0; i< queryList.size();i++){
		//printf("voting for the %d sift vec\n",i);
		for(int m = 0; m < 128;m++){
			sift_tmp.sift_vec[m]=queryList[i].sift_vec[m];
		}
		voteImg(sift_tmp,pRoot,accumulator);			//ÿһ��SIFT������ͼƬ����Ȩ��ͶƱ
	}
	//printf("voting completed\n");
	map<int,float>::iterator iter = accumulator.begin();
	//printf("establishing result list\n");
	int counter=0;
	//printf("total entry: %d\n",accumulator.size());
	while(iter != accumulator.end()){
		//printf("%d entries pushed\n",counter);
		//printf("img id: %d\n",iter->first);
		//printf("tot weight gained: %f\n\n",iter->second);
		counter++;
		QUERYRESULT query_result_tmp;
		query_result_tmp.img_id = iter->first;
		query_result_tmp.tot_weight = iter->second;
		resultList.push(query_result_tmp);
		iter++;
	}
	return;
}
 
/*����д���ļ�*/
void VisualWordTree::writeTree2File(char *filename){
	out.open(filename,ios::out|ios::binary|ios::trunc);
	out.seekp(0,ios::beg);
	tot_written_num++;
	printf("ready to write: %d\n",tot_written_num);
	writeNode2File(pRoot);
	printf("Completely write a tree to file!\n");
	out.close();
}

/*�������ļ�����*/
void VisualWordTree::readTreeFromFile(char *filename, int img_num){
	total_photo_num = img_num;
	pRoot = new NODE;
	in.open(filename,ios::in|ios::binary);
	in.seekg(0,ios::beg);
	readNodeFromFile(pRoot);
	in.close();
	return;
}

