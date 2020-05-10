#include <opencv\cv.h>
#include <opencv\cxcore.h>
#include <opencv\highgui.h>

#include "opensift\sift.h"
#include "opensift\imgfeatures.h"
#include "opensift\utils.h"

#include <dirent.h>

#include <stdio.h>
#include <string>
#include <vector>

struct img_mapping {
	int img_id;
	char *img_file;
	char *feat_file;
};

int main(int argc, char *argv[])
{
	DIR *dir;
	dirent *entry;
	char *img_dir = argv[1];
	char *data_dir = "feat";
	char img_file[500];
	char feat_file[500];
	std::vector<img_mapping> mapping_list;


	IplImage *img = NULL;
	struct feature *features;

	int img_id = 0;
	int vec_num = 0;
	int img_num = 0;
	int n;
	img_mapping mapping_tmp;
	dir = opendir(img_dir);
	while ((entry = readdir(dir)) != NULL) {
		if (strcmp(entry->d_name, ".") == 0 ||
			strcmp(entry->d_name, "..") == 0) {
			continue;
		}
		sprintf(img_file, "%s\\%s", img_dir, entry->d_name);
		printf("Processing %s...", entry->d_name);
		img = cvLoadImage(img_file, 1);
		n = sift_features(img, &features);
		printf("Found %d features, ", n);
		cvReleaseImage(&img);

		vec_num += n;
		img_num++;
		printf("%d vectors in total\n", vec_num);
		mapping_tmp.img_id = img_id++;
		mapping_tmp.feat_file = new char[30];
		sprintf(mapping_tmp.feat_file, "%03d.feat", mapping_tmp.img_id);
		mapping_tmp.img_file = new char[entry->d_namlen+1];
		strcpy(mapping_tmp.img_file, entry->d_name);
		mapping_list.push_back(mapping_tmp);

		sprintf(feat_file, "%s\\%s", data_dir, mapping_tmp.feat_file);
		export_features(feat_file, features, n);
		delete features;
	}

	FILE *info = NULL;
	info = fopen("feat\\siftinfo", "wb");
	fwrite(&img_num, sizeof(int), 1, info);
	fwrite(&vec_num, sizeof(int), 1, info);
	
	int str_len;
	str_len = strlen(img_dir)+1;
	fwrite(&str_len, sizeof(int), 1, info);
	fwrite(img_dir, sizeof(char), str_len, info);
	for (int i = 0; i < img_num; i++) {
		fwrite(&mapping_list[i].img_id, sizeof(int), 1, info);
		str_len = strlen(mapping_list[i].img_file)+1;
		//printf("str_len: %d\n", str_len);
		fwrite(&str_len, sizeof(int), 1, info);
		fwrite(mapping_list[i].img_file, sizeof(char), str_len, info);
		str_len = strlen(mapping_list[i].feat_file)+1;
		fwrite(&str_len, sizeof(int), 1, info);
		fwrite(mapping_list[i].feat_file, sizeof(char), str_len, info);
	}
	fclose(info);
	return 0;
}