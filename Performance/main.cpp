#include "database.h"

ImgDatabase imgdb;
int correct_normal;
int correct_blur;
using namespace std; 
int main()
{
    //imgdb.InitDatabase("D:\\workspace\\ConsoleApplication2\\feat");
    priority_queue<QUERYRESULT> res;
    priority_queue<QUERYRESULT> res1;
    printf("Loading database...");
    imgdb.LoadDatabase("D:\\workspace\\ConsoleApplication2");
    printf("done\n");
    char fullpath[2000];
    IplImage *ori, *blurred;
    QUERYRESULT query_item;
    QUERYRESULT query_final;
    struct feature* feat_buf;
    ori = blurred = NULL;
    int n;
    char str[666];

    correct_normal = correct_blur = 0;

    ori = cvLoadImage("C:\\Users\\HMD\\Desktop\\n0216545600001262.jpg", 1);
    //blurred = cvLoadImage("C:\\Users\\HMD\\Desktop\\n0153282900000005.jpg", 1);
    //cvSmooth(ori, blurred, CV_GAUSSIAN, 5,5,0,0);
    imgdb.QueryImg(ori, res);
    for (int i = 0; i < res.size(); i++) {
        query_item = res.top();
        sprintf_s(str, "%s\\%s", "D:\\workspace\\ConsoleApplication2\\feat", imgdb.mappingList[query_item.img_id].feat_file);
        n = import_features(str, FEATURE_LOWE, &feat_buf);
        query_item = res.top();
        query_item.tot_weight=res.top().tot_weight / n;
        res1.push(query_item);
        res.pop();
    }
     
    for (int i = 0;i < res1.size();i++) {
        query_item = res1.top();
        printf("%s\n", imgdb.mappingList[query_item.img_id].img_file);
        printf("%f\n", query_item.tot_weight);
        
        res1.pop();
    }
    


  //  for (int i = 0 ; i < imgdb.imgNum; i++) {
  //      printf("Processing img %d...", i);
  //      while(!res.empty()) {
  //          res.pop();
  //      }
  //      //sprintf_s(fullpath, "%s\\%s", imgdb.imgDir, imgdb.mappingList[i].img_file);
  //      sprintf_s(fullpath, "%s\\%s", "C:\\Users\\HMD\\Desktop\\pictures", imgdb.mappingList[i].img_file);
		//ori = cvLoadImage(fullpath, 1);
		//blurred = cvLoadImage(fullpath, 1);
  //      cvSmooth(ori, blurred, CV_GAUSSIAN, 5,5,0,0);
  //      //cvShowImage("ori", ori);
  //      //cvShowImage("blur", blurred);
  //      //getchar();

  //      imgdb.QueryImg(ori, res);
  //      query_item = res.top();
  //      if (query_item.img_id == i) {
  //          correct_normal++;
  //      }
  //      printf("Normal done...");
  //      
  //      while(!res.empty()) {
  //          res.pop();
  //      }
  //      imgdb.QueryImg(blurred, res);
  //      query_item = res.top();
  //      if (query_item.img_id == i) {
  //          correct_blur++;
  //      }
  //      printf("Gaussian blur done\n");
  //      //cvReleaseImage(&ori);
  //      //cvReleaseImage(&blurred);
  //      ori = blurred = NULL;
  //  }
  //  printf("Total image number: %d\n", imgdb.imgNum);
  //  printf("Normal correct: %d\n", correct_normal);
  //  printf("Gaussian blure correct: %d\n", correct_blur);
}