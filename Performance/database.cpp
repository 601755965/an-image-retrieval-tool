#include "database.h"
//#include "stdafx.h"
//#include <Windows.h>
#include <tchar.h>
//#include "resource.h"

ImgDatabase::ImgDatabase()
{
	featDir = "feat";
	imgNum = 0;
	vecNum = 0;
}

void ImgDatabase::InitDatabase(char *sift_dir)
{
	int len;
	//len = strlen(img_dir) + 1;
	//imgDir = new char[len];
	//strcpy(imgDir, img_dir);
	imgNum = 0;
	vecNum = 0;

	//ExtractFeatures(img_dir);
	LoadSiftInfo(sift_dir);

	CvMat *vec = cvCreateMat(vecNum, FEATURE_MAX_D, CV_32FC1);
	struct feature *feat_buf;
	char feat_file[1000];
	char stat_info[2000];
	int n;
	int temp = 0;
	//HWND hWND = AfxGetMainWnd()->m_hWnd;
	for (int i = 0; i < imgNum; i++) {
		sprintf_s(feat_file, "%s\\%s", sift_dir, mappingList[i].feat_file);
		sprintf_s(stat_info, "Importing from %s...", mappingList[i].feat_file);
		//SetDlgItemText(IDC_STAT, _T(stat_info));
		//SetDlgItemText(hWND, IDC_STAT, _T(stat_info));
		n = import_features(feat_file, FEATURE_LOWE, &feat_buf);
		for (int j = 0; j < n; j++) {
			for (int k = 0; k < FEATURE_MAX_D; k++) {
				vec->data.fl[temp++] = feat_buf[j].descr[k];
			}
		}
		delete feat_buf;
	}
	//SetDlgItemText(hWND, IDC_STAT, _T("Constructing visual word tree..."));
	vwTree.constructTree(vec);

	vector<SIFT_T> img_sift;
	SIFT_T sift_tmp;
	for (int i = 0; i < imgNum; i++) {
		sprintf_s(feat_file, "%s\\%s", sift_dir, mappingList[i].feat_file);
		sprintf_s(stat_info, "Mapping img %s...", mappingList[i].img_file);
		//SetDlgItemText(hWND, IDC_STAT, _T(stat_info));

		n = import_features(feat_file, FEATURE_LOWE, &feat_buf);
		for (int j = 0; j < n; j++) {
			for (int k = 0; k < FEATURE_MAX_D; k++) {
				sift_tmp.sift_vec[k] = feat_buf[j].descr[k];
			}
			img_sift.push_back(sift_tmp);
		}
		vwTree.mapTreeWithImg(mappingList[i].img_id, img_sift);
		img_sift.clear();
		delete feat_buf;
	}
	//SetDlgItemText(hWND, IDC_STAT, _T("Calculating weight..."));
	vwTree.calWeight(imgNum);
	SaveDatabase("D:\\workspace\\ConsoleApplication2");
	vwTree.destroyTree();
}

void ImgDatabase::QueryImg(IplImage *img, priority_queue<QUERYRESULT> &res)
{
	//IplImage *img;
	struct feature *feat_buf;
	vector<SIFT_T> img_feat;

	//img = cvLoadImage(img_path, 1);
	int n;
	n = sift_features(img, &feat_buf);
	SIFT_T tmp;
	for (int i = 0; i < n; i++) {
		for (int j = 0; j < FEATURE_MAX_D; j++) {
			tmp.sift_vec[j] = feat_buf[i].descr[j];
		}
		img_feat.push_back(tmp);
	}
	delete feat_buf;
	cvReleaseImage(&img);

	vwTree.queryTree(img_feat, res);
}

void ImgDatabase::SaveDatabase(char *database_dir)
{
	char fullpath[1000];
	sprintf_s(fullpath, "%s\\dbinfo", database_dir);
	FILE *file;
	file = fopen(fullpath, "wb");
	fwrite(&imgNum, sizeof(int), 1, file);

	int str_len;
	str_len = strlen(imgDir)+1;
	fwrite(&str_len, sizeof(int), 1, file);
	fwrite(imgDir, sizeof(char), str_len, file);

	for (int i = 0; i < imgNum; i++) {
		fwrite(&mappingList[i].img_id, sizeof(int), 1, file);
		str_len = strlen(mappingList[i].img_file)+1;
		fwrite(&str_len, sizeof(int), 1, file);
		fwrite(mappingList[i].img_file, sizeof(char), str_len, file);
		str_len = strlen(mappingList[i].feat_file)+1;
		fwrite(&str_len, sizeof(int), 1, file);
		fwrite(mappingList[i].feat_file, sizeof(char), str_len, file);
	}
	fclose(file);

	sprintf_s(fullpath, "%s\\treeinfo", database_dir);
	vwTree.writeTree2File(fullpath);
}

void ImgDatabase::LoadDatabase(char *database_dir)
{
	char fullpath[1000];
	sprintf_s(fullpath, "%s\\dbinfo", database_dir);
	FILE *file;
	file = fopen(fullpath, "rb");
	fread(&imgNum, sizeof(int), 1, file);

	int str_len;
	fread(&str_len, sizeof(int), 1, file);
	imgDir = new char[str_len];
	fread(imgDir, sizeof(char), str_len, file);

	img_mapping mapping_tmp;
	for (int i = 0; i < imgNum; i++) {
		fread(&mapping_tmp.img_id, sizeof(int), 1, file);
		fread(&str_len, sizeof(int), 1, file);
		mapping_tmp.img_file = new char[str_len];
		fread(mapping_tmp.img_file, sizeof(char), str_len, file);
		fread(&str_len, sizeof(int), 1, file);
		mapping_tmp.feat_file = new char[str_len];
		fread(mapping_tmp.feat_file, sizeof(char), str_len, file);
		mappingList.push_back(mapping_tmp);
	}
	fclose(file);

	sprintf_s(fullpath, "%s\\treeinfo", database_dir);
	vwTree.readTreeFromFile(fullpath, imgNum);
}

//void ImgDatabase::ExtractFeatures(char *img_dir)
//{
//	//IplImage *img = NULL;
//	//struct feature *feat_buf;
//
//	//DIR *dir = NULL;
//	//dirent *entry = NULL;
//	//struct _stat stat_buf;
//	//dir = opendir(img_dir);
//	//char imgpath[2000];
//	//char featpath[2000];
//	//int img_id = 0;
//	//img_mapping mapping_tmp;
//	//while((entry = readdir(dir)) != NULL) {
//	//	if (strcmp(entry->d_name, ".") == 0 ||
//	//		strcmp(entry->d_name, "..") == 0)
//	//		continue;
//	//	sprintf_s(imgpath, "%s\\%s", img_dir, entry->d_name);
//	//	
//	//	_stat(imgpath, &stat_buf);
//	//	if (S_ISDIR(stat_buf.st_mode)) {
//	//		continue;
//	//	}
//
//	//	img = cvLoadImage(imgpath, 1);
//	//	if (!img) {
//	//		//TODO: ´íÎó´¦Àí
//	//	}
//	//	int n;
//	//	n = sift_features(img, &feat_buf);
//	//	cvReleaseImage(&img);
//
//	//	vecNum += n;
//	//	imgNum++;
//	//	mapping_tmp.img_id = img_id++;
//	//	mapping_tmp.feat_file = new char[30];
//	//	sprintf_s(mapping_tmp.feat_file, "%03d.feat", mapping_tmp.img_id);
//	//	mapping_tmp.img_file = new char[entry->d_namlen+1];
//	//	strcpy(mapping_tmp.img_file, entry->d_name);
//	//	mappingList.push_back(mapping_tmp);
//
//	//	sprintf_s(featpath, "%s\\%s", featDir, mapping_tmp.feat_file);
//	//	export_features(featpath, feat_buf, n);
//	//	delete feat_buf;
//	//	//delete entry;
//	//}
//	//closedir(dir);
//
//	//SaveSiftInfo(featDir);
//	char cmdline[1000];
//	STARTUPINFO s_info;
//	ZeroMemory(&s_info, sizeof(s_info));
//	s_info.cb = sizeof(STARTUPINFO);
//	PROCESS_INFORMATION p_info;
//	ZeroMemory(&p_info, sizeof(p_info));
//	sprintf_s(cmdline, "ExtractFeature %s", img_dir);
//	CreateProcess(
//		NULL,
//		_T(cmdline),
//		NULL,
//		NULL,
//		FALSE,
//		NORMAL_PRIORITY_CLASS,
//		NULL,
//		NULL,
//		&s_info,
//		&p_info);
//	WaitForSingleObject(p_info.hProcess, INFINITE);
//}

void ImgDatabase::SaveSiftInfo(char *feat_dir)
{
	char fullpath[1000];
	sprintf_s(fullpath, "%s\\siftinfo", feat_dir);
	FILE *file;
	file = fopen(fullpath, "wb");
	fwrite(&imgNum, sizeof(int), 1, file);
	fwrite(&vecNum, sizeof(int), 1, file);
	
	int str_len;
	for (int i = 0; i < imgNum; i++) {
		fwrite(&mappingList[i].img_id, sizeof(int), 1, file);
		str_len = strlen(mappingList[i].img_file)+1;
		fwrite(mappingList[i].img_file, sizeof(char), str_len, file);
		str_len = strlen(mappingList[i].feat_file)+1;
		fwrite(mappingList[i].feat_file, sizeof(char), str_len, file);
	}
	fclose(file);
}

void ImgDatabase::LoadSiftInfo(char *feat_dir)
{
	char fullpath[1000];
	sprintf_s(fullpath, "%s\\siftinfo", feat_dir);
	FILE *file;
	file = fopen(fullpath, "rb");
	fread(&imgNum, sizeof(int), 1, file);
	fread(&vecNum, sizeof(int), 1, file);
	
	int str_len;
	fread(&str_len, sizeof(int), 1, file);
	imgDir = new char[str_len];
	fread(imgDir, sizeof(char), str_len, file);

	img_mapping mapping_tmp;
	for (int i = 0; i < imgNum; i++) {
		fread(&mapping_tmp.img_id, sizeof(int), 1, file);
		fread(&str_len, sizeof(int), 1, file);
		mapping_tmp.img_file = new char[str_len];
		fread(mapping_tmp.img_file, sizeof(char), str_len, file);
		fread(&str_len, sizeof(int), 1, file);
		mapping_tmp.feat_file = new char[str_len];
		fread(mapping_tmp.feat_file, sizeof(char), str_len, file);
		mappingList.push_back(mapping_tmp);
	}
	fclose(file);
}